//
// Created by vsan on 21.01.18.
//

#include <vector>
#include <string>
#include <cstring>
#include "HydraInternal.h"
#include <fcntl.h>           /* For O_* constants */
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>

#include <iostream>

struct SharedAccumImageLinux : public IHRSharedAccumImage
{
  SharedAccumImageLinux();
  ~SharedAccumImageLinux();

  bool   Create(int w, int h, int d, const char* name, char errMsg[256]) override;
  bool   Attach(const char* name, char errMsg[256]) override;

  void Clear() override;

  bool   Lock(int a_miliseconds) override;
  void   Unlock() override;

  HRSharedBufferHeader* Header() override;
  char*   MessageSendData() override;
  char*   MessageRcvData() override;
  float*  ImageData(int layerNum) override;

private:

  void Free();
  void AttachTo(char* memory);

  int m_buffDescriptor;
  sem_t* m_mutex;

  char*  m_memory;

  char*  m_msgSend;
  char*  m_msgRcv;
  float* m_images;

  uint64_t totalSize;

  std::string m_mutexName;
  std::string m_shmemName;
  bool m_ownThisResource;
};

SharedAccumImageLinux::SharedAccumImageLinux() : m_buffDescriptor(0), m_mutex(NULL), m_memory(nullptr), m_msgSend(nullptr), m_msgRcv(nullptr), m_images(nullptr), m_ownThisResource(false)
{

}

SharedAccumImageLinux::~SharedAccumImageLinux()
{
  Free();
}

void SharedAccumImageLinux::Free()
{
  sem_close(m_mutex);
  if(m_ownThisResource)
    sem_unlink(m_mutexName.c_str());
  
  m_mutex = NULL;

  if (m_memory != nullptr)
    munmap(m_memory, totalSize);
  m_memory = nullptr;

  if (m_buffDescriptor !=  -1)
    close(m_buffDescriptor);
  m_buffDescriptor = -1;
  
  if(m_ownThisResource)
    shm_unlink(m_shmemName.c_str());

  m_msgSend = nullptr;
  m_msgRcv  = nullptr;
  m_images  = nullptr;
  m_ownThisResource = false;
}


bool SharedAccumImageLinux::Create(int a_width, int a_height, int a_depth, const char* a_name, char a_errMsg[256])
{
  memset(a_errMsg, 0, 256);

  if (a_width == 0 || a_height == 0 || a_depth == 0)
  {
    Free();
    if(a_errMsg != nullptr)
      strcpy(a_errMsg, "");
    return true;
  }
  else // #TODO: if new width != old width ...
  {
    if (m_memory != nullptr)
    {
      auto* pHeader = Header();
      if (pHeader->width == a_width && pHeader->height == a_height && pHeader->depth == a_depth && m_shmemName == a_name)
        return true;
    }


    m_shmemName = a_name;
    m_mutexName = std::string(a_name) + "_mutex";
    totalSize   = uint64_t(sizeof(HRSharedBufferHeader)) + uint64_t(MESSAGE_SIZE * 2) + uint64_t(a_width*a_height)*size_t(a_depth*sizeof(float)*4) + uint64_t(1024);

    Free();

    m_mutex = sem_open (m_mutexName.c_str(), O_CREAT | O_EXCL, 0775, 1); //0775  | O_EXCL

    if (m_mutex == NULL)
    {
      perror("sem_open");
      strcpy(a_errMsg, "FAILED to create mutex (shared_mutex_init)");
      return false;
    }

    m_buffDescriptor = shm_open(m_shmemName.c_str(), O_CREAT | O_RDWR, 0775);
    int truncate_res = ftruncate(m_buffDescriptor, totalSize);

    if(m_buffDescriptor == -1 || truncate_res == -1)
    {
      strcpy(a_errMsg, "FAILED to alloc shared memory (shm_open, ftruncate)");
      Free();
      return false;
    }

    m_memory = (char*)mmap(nullptr, totalSize + 1, PROT_READ | PROT_WRITE, MAP_SHARED, m_buffDescriptor, 0);

    if(m_memory == MAP_FAILED)
    {
      strcpy(a_errMsg, "FAILED to map shared memory (mmap)");
      Free();
      return false;
    }

    //const int totalSize2 = totalSize / 4;
    //int* memi            = (int*)m_memory;
    //#pragma omp parallel for
    //for (int64_t i = 0; i < totalSize2; i++)
    //  memi[i] = 0;

    memset(m_memory, 0, size_t(totalSize));

    auto* pHeader = Header();

    pHeader->width      = a_width;
    pHeader->height     = a_height;
    pHeader->depth      = a_depth;
    pHeader->spp        = 0.0f;
    pHeader->counterRcv = 0;
    pHeader->counterSnd = 0;

    pHeader->totalByteSize      = totalSize;
    pHeader->messageSendOffset  = sizeof(HRSharedBufferHeader);
    pHeader->messageRcvOffset   = pHeader->messageSendOffset + MESSAGE_SIZE;
    pHeader->imageDataOffset    = pHeader->messageSendOffset + MESSAGE_SIZE*2;

    // now find offset for imageDataOffset to make resulting pointer is aligned(16) !!!
    //
    char* pData = m_memory + pHeader->imageDataOffset;
    auto intptr = reinterpret_cast<std::uintptr_t>(pData);

    while (intptr % 16 != 0)
    {
      pData++;
      intptr = reinterpret_cast<std::uintptr_t>(pData);
    };

    pHeader->imageDataOffset = (pData - m_memory);
    //
    // \\

    AttachTo(m_memory);
  }
  
  m_ownThisResource = true;
  strcpy(a_errMsg, "");
  return true;
}

bool SharedAccumImageLinux::Attach(const char* name, char errMsg[256])
{
  memset(errMsg, 0, 256);
  Free();

  const std::string mutexName = std::string(name) + "_mutex";
  m_mutexName = mutexName;
  m_shmemName = std::string(name);

  m_mutex = sem_open(m_mutexName.c_str(), 0);
  if (m_mutex == NULL || m_mutex == SEM_FAILED)
  {
    perror("sem_open");
    strcpy(errMsg, "FAILED to attach semaphore (sem_open)");
    return false;
  }

  m_buffDescriptor = shm_open(m_shmemName.c_str(), O_RDWR, 0);

  if(m_buffDescriptor == -1)
  {
    perror("shm_open");
    strcpy(errMsg, "FAILED to attach shmem (shm_open)");
    Free();
    return false;
  }

  totalSize = 0;

  m_memory = (char*)mmap(nullptr, totalSize + 1, PROT_READ | PROT_WRITE, MAP_SHARED, m_buffDescriptor, 0);

  if(m_memory == MAP_FAILED)
  {
    perror("mmap");
    strcpy(errMsg, "FAILED to map shared memory (mmap)");
    Free();
    return false;
  }

  HRSharedBufferHeader* pHeader = (HRSharedBufferHeader*)m_memory;

  int a_width  = pHeader->width;
  int a_height = pHeader->height;
  int a_depth  = pHeader->depth;

  if (m_memory != nullptr)
    munmap(m_memory, totalSize);

  totalSize = uint64_t(sizeof(HRSharedBufferHeader)) + uint64_t(MESSAGE_SIZE * 2) + uint64_t(a_width*a_height)*uint64_t(a_depth*sizeof(float)*4) + uint64_t(1024);

  m_memory = (char*)mmap(nullptr, totalSize + 1, PROT_READ | PROT_WRITE, MAP_SHARED, m_buffDescriptor, 0);

  if(m_memory == MAP_FAILED)
  {
    perror("mmap");
    strcpy(errMsg, "FAILED to map shared memory (mmap)");
    Free();
    return false;
  }

  AttachTo(m_memory);
  
  m_ownThisResource = false;
  strcpy(errMsg, "");
  return true;
}


void SharedAccumImageLinux::Clear()
{
  auto* pHeader = Header();
  pHeader->spp = 0.0f;
  pHeader->counterRcv = 0;
  pHeader->counterSnd = 0;

  auto pImg = ImageData(0);
  memset(pImg, 0, size_t(pHeader->width*pHeader->height)*size_t(sizeof(float)*4));
}


void SharedAccumImageLinux::AttachTo(char* a_memory)
{
  HRSharedBufferHeader* pHeader = (HRSharedBufferHeader*)m_memory;

  m_msgSend = m_memory + pHeader->messageSendOffset;
  m_msgRcv  = m_memory + pHeader->messageRcvOffset;
  m_images  = (float*)(m_memory + pHeader->imageDataOffset);
}

bool SharedAccumImageLinux::Lock(int a_miliseconds)
{
  struct timespec ts;
  ts.tv_sec = a_miliseconds / 1000;
  ts.tv_nsec = a_miliseconds * 1'000'000 - ts.tv_sec * 1'000'000'000;

  int res = sem_timedwait(m_mutex, &ts);

  if(res == -1)
  {
    perror("sem_timedwait");
    return false;
  }
  else
    return true;

  /*sem_wait(m_mutex);
  perror("sem_wait");*/
}

void SharedAccumImageLinux::Unlock()
{
  sem_post(m_mutex);
}

float* SharedAccumImageLinux::ImageData(int layerId)
{
  HRSharedBufferHeader* pHeader = (HRSharedBufferHeader*)m_memory;
  return m_images + int64_t(pHeader->width*pHeader->height)*int64_t(layerId*4);
}

char* SharedAccumImageLinux::MessageSendData()
{
  return m_msgSend;
}

char* SharedAccumImageLinux::MessageRcvData()
{
  return m_msgRcv;
}

HRSharedBufferHeader* SharedAccumImageLinux::Header()
{
  return (HRSharedBufferHeader*)m_memory;
}


IHRSharedAccumImage* CreateImageAccum()
{
  return new SharedAccumImageLinux();
}
