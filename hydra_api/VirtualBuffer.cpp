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

#include <cmath>

static constexpr bool gDebugMode     = true;
static constexpr bool gCopyCollector = false;

bool SharedVirtualBufferIsEnabled() { return (gDebugMode == false); }

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


bool VirtualBuffer::Init(uint64_t a_sizeInBytes, const char* a_shmemName, std::vector<int>* a_pTempBuffer, HRSystemMutex* a_mutex)
{
  m_pVBMutex = a_mutex;
  
  if (a_sizeInBytes % 1024 != 0)
  {
    HrError(L"VirtualBuffer::FATAL ERROR: bad virtual buffer size");
    return false;
  }

#ifdef WIN32
  unsigned long long totalMem;
  GetPhysicallyInstalledSystemMemory(&totalMem);
  totalMem *= size_t(1024);
  
  if (totalMem / 4 < a_sizeInBytes)
    a_sizeInBytes = totalMem / 4;
#endif
  
  
  m_totalSize   = a_sizeInBytes;
  
  if(a_sizeInBytes > 4096)                                        // don't init table if single page wa allocated, dummy virtual buffer.
    a_sizeInBytes += (VB_CHUNK_TABLE_OFFS + VB_CHUNK_TABLE_SIZE); // alloc memory for both virtual buffer and chunks table

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

    if (m_data == nullptr)
    {
      CloseHandle(m_fileHandle); m_fileHandle = NULL;
      HrError(L"VirtualBuffer::FATAL ERROR: shmem file can not be maped");
      return false;
    }
  }

#else

  shmemName = std::string(a_shmemName);
  if (gDebugMode)
  {
    m_data = malloc(size_t(a_sizeInBytes));
    memset(m_data, 0, size_t(a_sizeInBytes));
  }
  else
  {
    m_fileDescriptor = shm_open(a_shmemName, O_CREAT | O_RDWR | O_TRUNC, 0777);
    if(ftruncate(m_fileDescriptor, a_sizeInBytes) == -1)
      HrError(L"VirtualBuffer::FATAL ERROR: shmem file can not be resized (ftruncate error)");

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
  
  if(a_sizeInBytes > 4096) // don't init table if single page was allocated only, dummy virtual buffer.
  {
    m_chunkTable = (int64_t *) (((char *) m_data) + m_totalSize + VB_CHUNK_TABLE_OFFS);
    memset(m_chunkTable, 0, VB_CHUNK_TABLE_SIZE);
  }
  
  Clear();
  m_pTempBuffer = a_pTempBuffer;
  m_owner       = true;
  return true;
}


bool VirtualBuffer::Attach(uint64_t a_sizeInBytes, const char* a_shmemName, std::vector<int>* a_pTempBuffer)
{
  if (a_sizeInBytes % 1024 != 0)
  {
    HrError(L"VirtualBuffer::FATAL ERROR: bad virtual buffer size");
    return false;
  }
  
  m_totalSize = a_sizeInBytes;
  if(a_sizeInBytes > 4096)                                        // don't init table if single page wa allocated, dummy virtual buffer.
    a_sizeInBytes += (VB_CHUNK_TABLE_OFFS + VB_CHUNK_TABLE_SIZE); // alloc memory for both virtual buffer and chunks table

#ifdef WIN32

  m_fileHandle = OpenFileMappingA(FILE_MAP_READ | FILE_MAP_WRITE, 0, a_shmemName);
  if (m_fileHandle == NULL)
  {
    HrError(L"VirtualBuffer::FATAL ERROR: shmem file can not attach (OpenFileMappingA)");
    return false;
  }

  m_data = MapViewOfFile(m_fileHandle, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0);
  if (m_data == nullptr)
  {
    CloseHandle(m_fileHandle); m_fileHandle = NULL;
    HrError(L"VirtualBuffer::FATAL ERROR: shmem file can not be maped (MapViewOfFile)");
    return false;
  }

#else
  
  shmemName = std::string(a_shmemName);
  if (gDebugMode)
    return false;
  else
  {
    m_fileDescriptor = shm_open(a_shmemName, O_RDONLY, 0777);
    if(m_fileDescriptor == -1)
    {
      HrError(L"VirtualBuffer::FATAL ERROR: can not attach to shmem file (shm_open)");
      return false;
    }
    
    m_data = mmap(nullptr, a_sizeInBytes, PROT_READ, MAP_SHARED, m_fileDescriptor, 0);
    if(m_data == MAP_FAILED)
    {
      HrError(L"VirtualBuffer::FATAL ERROR: can not attach to shmem file (mmap)");
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
  
  if(a_sizeInBytes > 4096) // don't init table if single page was allocated only, dummy virtual buffer.
  {
    m_chunkTable = (int64_t *) (((char *) m_data) + m_totalSize + VB_CHUNK_TABLE_OFFS);
  }
  
  Clear();
  m_pTempBuffer = a_pTempBuffer;
  m_owner       = false;
  return true;
}

void VirtualBuffer::RestoreChunks()
{
  const int64_t* table = ChunksTablePtr();
  if(table == nullptr)
    return;

  // (1) scan table to find last chunk
  //
  const auto maxChunks = (VB_CHUNK_TABLE_SIZE)/sizeof(int64_t) - 2;

  int chunksNum = 0, chunkOffset = 1;
  do {

    chunksNum++;
    chunkOffset = table[chunksNum];

  } while(chunksNum < maxChunks && chunkOffset != 0);

  // (2) restore chunk pointers (their localAddresses)
  //
  m_allChunks.resize(chunksNum);
  
  for(size_t j=0;j<m_allChunks.size()-1;j++)
  {
    m_allChunks[j].id           = j;
    m_allChunks[j].localAddress = uint64_t(table[j]);
    m_allChunks[j].sizeInBytes  = table[j+1] - table[j];
    
    if(m_allChunks[j].localAddress == uint64_t(-1))
      m_allChunks[j].pVB = nullptr;
    else
      m_allChunks[j].pVB = this;
  }

  auto last = m_allChunks.size()-1;
  
  m_allChunks[last].id           = last;
  m_allChunks[last].localAddress = uint64_t(table[last]);
  m_allChunks[last].sizeInBytes  = 0;
  if(m_allChunks[last].localAddress == uint64_t(-1))
    m_allChunks[last].pVB = nullptr;
  else
    m_allChunks[last].pVB = this;
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
  if (!gDebugMode)
  {
    munmap(m_data, m_totalSizeAllocated);
    //if(m_owner)
    shm_unlink(shmemName.c_str());
    close(m_fileDescriptor);
  }
#endif

  m_data = nullptr;
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
  const uint64_t freeSize = gCopyCollector ? (m_currSize - m_currTop) : (m_totalSize - m_currTop);

  if (a_sizeInBytes < freeSize) // alloc 
  {
    return AllocInCacheNow(a_sizeInBytes);
  }
  else if (gCopyCollector)
  {
    const uint64_t maxAllowedSize = m_currSize - maxAccumulatedSize();
    
    if (a_sizeInBytes < maxAllowedSize) // swap old objects to disc and put a new object to free memory 
    {
      if(m_pVBMutex!=nullptr)
        hr_lock_system_mutex(m_pVBMutex, VB_LOCK_WAIT_TIME_MS);
      
      RunCopyingCollector();
      
      int64_t* chunkTable = ChunksTablePtr(); // we need to clear table right after collector works, because old data is invalid!
      if(chunkTable != nullptr)               // we can do better of cource here, but clearing should be enought to work corretcly
      {
        for(size_t i=0;i<size();i++)
          chunkTable[i] = int64_t(-1);
      }

      if(m_pVBMutex!=nullptr)
        hr_unlock_system_mutex(m_pVBMutex);
      return AllocInCacheNow(a_sizeInBytes);
    }
    else // this object is too big. We can not allocate memory here. Need to store it on disk.
    {
      std::cerr << "VirtualBuffer::AllocInCache : the object is too big! gCopyCollector = " << int(gCopyCollector) << std::endl;
      return nullptr;
    }
  }
  else
  { 
    constexpr int div = 8;
    const uint64_t maxAllowedSize = (m_totalSize*(div-1))/div - 1024;
    
    if (a_sizeInBytes < maxAllowedSize) // swap old objects to disc and put a new object to free memory 
    {
      if(m_pVBMutex!=nullptr)
        hr_lock_system_mutex(m_pVBMutex, VB_LOCK_WAIT_TIME_MS);
      
      RunCollector(div);
  
      int64_t* chunkTable = ChunksTablePtr(); // we need to clear table right after collector works, because old data is invalid!
      if(chunkTable != nullptr)               // we can do better of cource here, but clearing should be enought to work corretcly
      {
        for(size_t i=0;i<size();i++)
          chunkTable[i] = int64_t(-1);
      }
      
      if(m_pVBMutex!=nullptr)
        hr_unlock_system_mutex(m_pVBMutex);

      return AllocInCacheNow(a_sizeInBytes);
    }
    else // this object is too big. We can not allocate memory here. Need to store it on disk.
    {
      std::cerr << "VirtualBuffer::AllocInCache : the object is too big! gCopyCollector = " << int(gCopyCollector) << std::endl;
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

void VirtualBuffer::RunCollector(int a_divisor)
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

  // (2) sweep throught currChunksInMemory to copy 1/8 of the buffer size to tempBuffer
  //
  size_t top = 0;
  size_t curAccumulatedMemory = 0;
  size_t maxAccumulatedMemory = m_totalSize / a_divisor;

  if (m_pTempBuffer->size() < maxAccumulatedMemory)
    m_pTempBuffer->resize( maxAccumulatedMemory/sizeof(int) + 1);

  char* dataTemp = (char*)m_pTempBuffer->data();

  size_t i = 0;
  for (; i < currChunksInMemory.size(); i++)
  {
    size_t id = currChunksInMemory[i];
    uint64_t totalSizeInBytes = m_allChunks[id].sizeInBytes;

    curAccumulatedMemory += totalSizeInBytes; 
    if (curAccumulatedMemory > maxAccumulatedMemory)
      break;
    
    memcpy(dataTemp + top, m_dataHalfCurr + m_allChunks[id].localAddress, totalSizeInBytes);  // copy chunk from m_dataHalfCurr to m_dataHalfFree
    
    m_allChunks[id].localAddress = top; // alternate it's address because now this chunk is in different location;
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

  // (4) copy chunks from tempBuffer to 
  //
  memcpy(m_dataHalfCurr, dataTemp, top);

  if (m_pTempBuffer->size() > maxAccumulatedMemory) // free unneccesary memory of tempBuffer it it was allocated previously too much
    (*m_pTempBuffer) = std::vector<int>();

  m_currTop = top;
}

void VirtualBuffer::FlushToDisc()
{
  for (size_t id : m_chunksIdInMemory)
    m_allChunks[id].SwapToDisk();
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
    return nullptr; // #TODO: Swap chunk to memory (m_tempBuffer) and get fucking pointer
  }
}

const void* ChunkPointer::GetMemoryNow() const
{
  if (InMemory())
  {
    return pVB->m_dataHalfCurr + localAddress;
  }
  else
  {
    return nullptr; // #TODO: Swap chunk to memory (m_tempBuffer) and get fucking pointer
  }
}

extern HRObjectManager g_objManager;

std::wstring LocalDataPathOfCurrentSceneLibrary()
{
  return g_objManager.scnData.m_path + L"/data/";
}

std::wstring ChunkName(const ChunkPointer& a_chunk)
{
  std::wstringstream namestream;
  namestream << g_objManager.scnData.m_path.c_str() << L"/data/chunk_" << std::setfill(L"0"[0]) << std::setw(5) << a_chunk.id;

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

  if (wasSaved)
    return;

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
  wasSaved = true;
}


