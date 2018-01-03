#include "HydraInternal.h"
#include "HydraObjectManager.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

static const bool gDebugMode = false;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline uint64_t blocksST(uint64_t elems, int threadsPerBlock)
{
  if (elems % threadsPerBlock == 0 && elems >= threadsPerBlock)
    return elems / threadsPerBlock;
  else
    return (elems / threadsPerBlock) + 1;
}

inline uint64_t roundBlocks(uint64_t elems, int threadsPerBlock)
{
  if (elems < threadsPerBlock)
    return (uint64_t)threadsPerBlock;
  else
    return blocksST(elems, threadsPerBlock) * threadsPerBlock;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool VirtualBuffer::Init(uint64_t a_sizeInBytes, const char* a_shmemName)
{
  if (a_sizeInBytes % 1024 != 0)
  {
    HrError(L"VirtualBuffer::FATAL ERROR: bad virtual buffer size");
    return false;
  }

  m_totalSize = a_sizeInBytes;

#ifdef WIN32

  DWORD imageSizeL = a_sizeInBytes & 0x00000000FFFFFFFF;
  DWORD imageSizeH = (a_sizeInBytes & 0xFFFFFFFF00000000) >> 32;

  if (gDebugMode)
  {
    m_data = malloc(size_t(a_sizeInBytes));
  }
  else
  {
    m_fileHandle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, imageSizeH, imageSizeL, a_shmemName);
    
    if (m_fileHandle == NULL)
    {
      HrError(L"VirtualBuffer::FATAL ERROR: shmem file can not be created");
      return false;
    }

    m_data = MapViewOfFile(m_fileHandle, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0);
  }


  if (m_data == nullptr)
  {
    CloseHandle(m_fileHandle); m_fileHandle = NULL;
    HrError(L"VirtualBuffer::FATAL ERROR: shmem file can not be maped");
    return false;
  }

#else

  shmemName = std::string(a_shmemName);
  if (gDebugMode)
  {
    m_data = malloc(size_t(a_sizeInBytes));
  }
  else
  {
    m_fileDescriptor = shm_open(a_shmemName, O_CREAT | O_RDWR, 0777);
    ftruncate(m_fileDescriptor, a_sizeInBytes);

    m_data = mmap(nullptr, a_sizeInBytes, PROT_READ | PROT_WRITE, MAP_SHARED, m_fileDescriptor, 0);
    if(m_data == MAP_FAILED)
    {
      HrError(L"VirtualBuffer::FATAL ERROR: shmem file can not be created");
      close(m_fileDescriptor);
      return false;
    }
  }
  if (m_data == nullptr)
  {
    HrError(L"VirtualBuffer::FATAL ERROR: shmem file can not be mapped");
    return false;
  }
#endif

  Clear();

  return true;
}

void VirtualBuffer::Destroy()
{
  if (m_data == nullptr)
    return;

  if (gDebugMode)
    free(m_data);

#ifdef WIN32
  if (!gDebugMode)
  {
    UnmapViewOfFile(m_data);    m_data       = nullptr;
    CloseHandle(m_fileHandle);  m_fileHandle = NULL;
  }
#else
  munmap(m_data, m_totalSizeAllocated);
  shm_unlink(shmemName.c_str());
  close(m_fileDescriptor);
#endif
}

void VirtualBuffer::Clear()
{
  m_dataHalfCurr = (char*)m_data;
  m_dataHalfFree = m_dataHalfCurr + m_totalSize/2;

  m_currTop  = 0;
  m_currSize = m_totalSize / 2;
  m_totalSizeAllocated = 0;

  m_allChunks.clear();
  m_chunksIdInMemory.clear();
}

char* VirtualBuffer::AllocInCacheNow(uint64_t a_sizeInBytes)
{
  char* objectMem = m_dataHalfCurr + m_currTop;
  m_currTop            += a_sizeInBytes;
  m_totalSizeAllocated += a_sizeInBytes;
  return objectMem;
}

void* VirtualBuffer::AllocInCache(uint64_t a_sizeInBytes)
{
  uint64_t freeSize = m_currSize - m_currTop;

  if (a_sizeInBytes < freeSize) // alloc 
  {
    return AllocInCacheNow(a_sizeInBytes);
  }
  else
  {
    uint64_t maxAllowedSize = m_currSize - maxAccumulatedSize();

    if (a_sizeInBytes < maxAllowedSize) // swap old objects to disc and put a new object to free memory 
    {
      RunCopyingCollector();
      return AllocInCacheNow(a_sizeInBytes);
    }
    else // this object is too big. We can not allocate memory here. Need to store it on disk.
    {
      std::cerr << "VirtualBuffer::AllocInCache : the object is too big!" << std::endl;
      return nullptr;
    }
  }

}

void VirtualBuffer::ResizeAndAllocEmptyChunks(uint64_t a_ckunksNum)
{
  m_allChunks.resize(a_ckunksNum);

  for (size_t i = 0; i < m_allChunks.size(); i++)
  {
    m_allChunks[i].id           = uint64_t(i);
    m_allChunks[i].localAddress = uint64_t(-1);
    m_allChunks[i].sizeInBytes  = 0;
    m_allChunks[i].useCounter   = 0;
    m_allChunks[i].inUse        = false;
  }

}

size_t VirtualBuffer::AllocChunk(uint64_t a_dataSizeInBytes, uint64_t a_objId)
{
  ChunkPointer result(this);
  result.id = m_allChunks.size();

  void* memory = AllocInCache(a_dataSizeInBytes);
  if (memory == nullptr) 
  {
    result.inUse      = true;
    result.useCounter = 0;
    return size_t(-1);
  }

  float startCounter   = fmaxf(100.0f/fmax(log(float(a_dataSizeInBytes)), 1.0f), 0.0f); // we want small objects to be mostly in cache

  result.localAddress  = ((char*)memory) - m_dataHalfCurr;
  result.sizeInBytes   = a_dataSizeInBytes;
  result.useCounter    = (uint32_t)(startCounter*100.0f);
  result.inUse         = true;

  m_chunksIdInMemory.push_back(result.id);
  m_allChunks.push_back(result);

  return result.id;
}

void VirtualBuffer::RunCopyingCollector()
{
  // (1)  we must decide wich objects we can handle in memory
  // sort currChunksInMemory by m_allChunks[i].useCounter
  //

  struct CompareIds
  {
    CompareIds(VirtualBuffer* a_pVB) : pVB(a_pVB) { }
    bool operator()(size_t a, size_t b) const { return pVB->chunk_at(a).useCounter > pVB->chunk_at(b).useCounter; }
    VirtualBuffer* pVB;
  };

  std::sort(m_chunksIdInMemory.begin(), m_chunksIdInMemory.end(), CompareIds(this));

  std::vector<size_t> currChunksInMemory = m_chunksIdInMemory;
  m_chunksIdInMemory.clear();

  // (2) sweep throught currChunksInMemory to copy half of them (i.e. half of memory they used) to secondary location
  //

  size_t top = 0;
  size_t curAccumulatedMemory = 0;
  size_t maxAccumulatedMemory = maxAccumulatedSize()/2; // half of buffer size div by 4

  size_t i = 0;
  for (; i < currChunksInMemory.size(); i++)
  {
    size_t id = currChunksInMemory[i];
    uint64_t totalSizeInBytes = m_allChunks[id].sizeInBytes;

    curAccumulatedMemory += totalSizeInBytes;
     
    if (curAccumulatedMemory > maxAccumulatedMemory)
      break;

    memcpy(m_dataHalfFree + top, 
           m_dataHalfCurr + m_allChunks[id].localAddress, totalSizeInBytes);  // copy chunk from m_dataHalfCurr to m_dataHalfFree

    m_allChunks[id].localAddress = top;                                       // alternate it's address because now this chunk is in different location;
    m_chunksIdInMemory.push_back(id);
    top += totalSizeInBytes;
  }

  // (3) swap other chunks to disk
  //
  for (; i < currChunksInMemory.size(); i++)
  {
    size_t id = currChunksInMemory[i];
    m_allChunks[id].SwapToDisk();
    m_allChunks[id].localAddress = uint64_t(-1);
  }


  // (4) swap pointers
  //
  char* temp     = m_dataHalfCurr;
  m_dataHalfCurr = m_dataHalfFree;
  m_dataHalfFree = temp;
  m_currTop      = top;
}

void VirtualBuffer::FlushToDisc()
{
  for (size_t i = 0; i < m_chunksIdInMemory.size(); i++)
  {
    size_t id = m_chunksIdInMemory[i];
    m_allChunks[id].SwapToDisk();
  }

  // m_chunksIdInMemory.clear();
  // m_currTop = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void* ChunkPointer::GetMemoryNow()
{
  if (InMemory())
  {
    return pVB->m_dataHalfCurr + localAddress;
  }
  else
  {
    return nullptr; // #TODO: Swap chunk to memory and get fucking pointer
  }
}

extern HRObjectManager g_objManager;

std::wstring ChunkName(const ChunkPointer& a_chunk)
{
  std::wstringstream namestream;
  namestream << g_objManager.scnlib().m_path.c_str() << L"/data/chunk_" << std::setfill(L"0"[0]) << std::setw(5) << a_chunk.id;

  switch (a_chunk.type)
  {
  case CHUNK_TYPE_UNKNOWN:   namestream << L".bin";      break;
  case CHUNK_TYPE_ARRAY1F:   namestream << L".array1f";  break;
  case CHUNK_TYPE_ARRAY1UI:  namestream << L".array1ui"; break;
  case CHUNK_TYPE_ARRAY2F:   namestream << L".array2f";  break;
  case CHUNK_TYPE_ARRAY4F:   namestream << L".array4f";  break;
  case CHUNK_TYPE_IMAGE4UB:  namestream << L".image4ub"; break;
  case CHUNK_TYPE_IMAGE4F:   namestream << L".image4f";  break;
  case CHUNK_TYPE_IMAGE4HF:  namestream << L".image4hf"; break;
  case CHUNK_TYPE_VSGF:      namestream << L".vsgf";     break;
  default:                   namestream << L".bin";      break;
  };

  return namestream.str();
}


void ChunkPointer::SwapToDisk()
{
  if (!inUse || !InMemory())
  {
    //localAddress = -1;
    return;
  }

  const std::wstring name = ChunkName(*this);
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
  std::wstring s1(name);
  std::string  s2(s1.begin(), s1.end());
  std::ofstream fout(s2.c_str(), std::ios::binary);
#elif defined WIN32
  std::ofstream fout(name.c_str(), std::ios::binary);
#endif
  fout.write(pVB->m_dataHalfCurr + localAddress, sizeInBytes);
  fout.close();

  // localAddress = -1;
}


