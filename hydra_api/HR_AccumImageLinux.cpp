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
  ~SharedAccumImageLinux() override;

  bool   Create(int a_width, int a_height, int a_depth, const char* a_name, char a_errMsg[ERR_MSG_SZ]) override;
  bool   Create(int a_width, int a_height, int a_depth, int a_channels, const char* a_name, char a_errMsg[ERR_MSG_SZ]) override;
  bool   Attach(const char* name, char errMsg[ERR_MSG_SZ]) override;

  void   Clear() override;

  bool   Lock(int a_miliseconds) override;
  void   Unlock() override;

  HRSharedBufferHeader* Header() override;
  char*   MessageSendData() override;
  char*   MessageRcvData() override;
  float*  ImageData(int layerId) override;

private:
  bool CreateInternal(int a_width, int a_height, int a_depth, int a_channels, const char* a_name, char a_errMsg[ERR_MSG_SZ]);
  size_t GetImageSize();
  void Free();
  void AttachTo(char* a_memory);

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

SharedAccumImageLinux::SharedAccumImageLinux() : m_buffDescriptor(-1), m_mutex(nullptr), m_memory(nullptr), m_msgSend(nullptr), m_msgRcv(nullptr), m_images(nullptr),
                                                 m_ownThisResource(false), totalSize(0)
{

}

SharedAccumImageLinux::~SharedAccumImageLinux()
{
  Free();
}

void SharedAccumImageLinux::Free()
{
//  if(m_mutex)
//    Unlock();

  sem_close(m_mutex);
  if(m_ownThisResource)
    sem_unlink(m_mutexName.c_str());
  
  m_mutex = nullptr;

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

size_t SharedAccumImageLinux::GetImageSize()
{
  auto* pHeader = Header();

  return uint64_t(sizeof(HRSharedBufferHeader)) + uint64_t(MESSAGE_SIZE * 2) +
         uint64_t(pHeader->width * pHeader->height) * size_t(pHeader->depth * sizeof(float) * pHeader->channels) +
         uint64_t(1024);
}

bool SharedAccumImageLinux::CreateInternal(int a_width, int a_height, int a_depth, int a_channels, const char* a_name, char a_errMsg[ERR_MSG_SZ])
{
  memset(a_errMsg, 0, ERR_MSG_SZ);

  if (a_width == 0 || a_height == 0 || a_depth == 0 || a_channels == 0)
  {
    Free();
    if(a_errMsg != nullptr)
      strncpy(a_errMsg, "", ERR_MSG_SZ);
    return true;
  }
  else
  {
    if (m_memory != nullptr)
    {
      auto* pHeader = Header();
      if (pHeader->width == a_width && pHeader->height == a_height && pHeader->depth == a_depth &&
          pHeader->channels == a_channels && m_shmemName == a_name)
      {
        return true;
      }
    }

    m_shmemName = a_name;
    m_mutexName = std::string(a_name) + "_mutex";
    totalSize   = uint64_t(sizeof(HRSharedBufferHeader)) + uint64_t(MESSAGE_SIZE * 2) +
                  uint64_t(a_width * a_height) * size_t(a_depth * a_channels * sizeof(float)) + uint64_t(1024);

    Free();

    m_mutex = sem_open (m_mutexName.c_str(), O_CREAT | O_EXCL, 0775, 1); //0775  | O_EXCL
    if (m_mutex == nullptr)
    {
      perror("sem_open");
      strncpy(a_errMsg, "FAILED to create mutex (shared_mutex_init)", ERR_MSG_SZ);
      return false;
    }

    m_buffDescriptor = shm_open(m_shmemName.c_str(), O_CREAT | O_RDWR, 0775);
    int truncate_res = ftruncate(m_buffDescriptor, totalSize);

    if(m_buffDescriptor == -1 || truncate_res == -1)
    {
      strncpy(a_errMsg, "FAILED to alloc shared memory (shm_open, ftruncate)", ERR_MSG_SZ);
      Free();
      return false;
    }

    m_memory = (char*)mmap(nullptr, totalSize + 1, PROT_READ | PROT_WRITE, MAP_SHARED, m_buffDescriptor, 0);
    if(m_memory == MAP_FAILED)
    {
      strncpy(a_errMsg, "FAILED to map shared memory (mmap)", ERR_MSG_SZ);
      Free();
      return false;
    }


    memset(m_memory, 0, size_t(totalSize));

    auto* pHeader = Header();

    pHeader->width      = a_width;
    pHeader->height     = a_height;
    pHeader->depth      = a_depth;
    pHeader->channels   = a_channels;
    pHeader->spp        = 0.0f;
    pHeader->counterRcv = 0;
    pHeader->counterSnd = 0;

    pHeader->totalByteSize      = totalSize;
    pHeader->messageSendOffset  = sizeof(HRSharedBufferHeader);
    pHeader->messageRcvOffset   = pHeader->messageSendOffset + MESSAGE_SIZE;
    pHeader->imageDataOffset    = pHeader->messageSendOffset + MESSAGE_SIZE*2;

    // now find offset for imageDataOffset to make resulting pointer aligned(16) !!!
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
  strncpy(a_errMsg, "", ERR_MSG_SZ);
  return true;
}

bool SharedAccumImageLinux::Create(int a_width, int a_height, int a_depth, int a_channels, const char* a_name, char a_errMsg[ERR_MSG_SZ])
{
  return CreateInternal(a_width, a_height, a_depth, a_channels, a_name, a_errMsg);
}

bool SharedAccumImageLinux::Create(int a_width, int a_height, int a_depth, const char* a_name, char a_errMsg[ERR_MSG_SZ])
{
  return CreateInternal(a_width, a_height, a_depth, 4, a_name, a_errMsg);
}

bool SharedAccumImageLinux::Attach(const char* name, char errMsg[ERR_MSG_SZ])
{
  memset(errMsg, 0, ERR_MSG_SZ);
  Free();

  const std::string mutexName = std::string(name) + "_mutex";
  m_mutexName = mutexName;
  m_shmemName = std::string(name);

  std::cout << "SharedAccumImageLinux::Attach: " << m_mutexName << std::endl;
  m_mutex = sem_open(m_mutexName.c_str(), 0);
  if (m_mutex == nullptr || m_mutex == SEM_FAILED)
  {
    perror("sem_open");
    strncpy(errMsg, "FAILED to attach semaphore (sem_open)", ERR_MSG_SZ);
    return false;
  }

  m_buffDescriptor = shm_open(m_shmemName.c_str(), O_RDWR, 0);
  if(m_buffDescriptor == -1)
  {
    perror("shm_open");
    strncpy(errMsg, "FAILED to attach shmem (shm_open)", ERR_MSG_SZ);
    Free();
    return false;
  }

  totalSize = 0;

  m_memory = (char*)mmap(nullptr, totalSize + 1, PROT_READ | PROT_WRITE, MAP_SHARED, m_buffDescriptor, 0);
  if(m_memory == MAP_FAILED)
  {
    perror("mmap");
    strncpy(errMsg, "FAILED to map shared memory (mmap)", ERR_MSG_SZ);
    Free();
    return false;
  }

  if (m_memory != nullptr)
    munmap(m_memory, totalSize);

  totalSize = GetImageSize();

  m_memory = (char*)mmap(nullptr, totalSize + 1, PROT_READ | PROT_WRITE, MAP_SHARED, m_buffDescriptor, 0);

  if(m_memory == MAP_FAILED)
  {
    perror("mmap");
    strncpy(errMsg, "FAILED to map shared memory (mmap)", ERR_MSG_SZ);
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
  memset(pImg, 0, size_t(pHeader->width * pHeader->height) * size_t(pHeader->channels * sizeof(float)));
}


void SharedAccumImageLinux::AttachTo(char* a_memory)
{
  auto* pHeader = (HRSharedBufferHeader*)a_memory;

  m_msgSend = a_memory + pHeader->messageSendOffset;
  m_msgRcv  = a_memory + pHeader->messageRcvOffset;
  m_images  = (float*)(a_memory + pHeader->imageDataOffset);
}

bool SharedAccumImageLinux::Lock(int a_miliseconds)
{
  struct timespec ts{};
  if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
  {
    perror("clock_gettime(SharedAccumImageLinux::Lock)");
    return false;
  }
  long seconds      = a_miliseconds / 1000;
  long milliseconds = a_miliseconds % 1000;

  milliseconds = milliseconds * 1000 * 1000 + ts.tv_nsec;
  long add = milliseconds / (1000 * 1000 * 1000);
  ts.tv_sec += (add + seconds);
  ts.tv_nsec = milliseconds % (1000 * 1000 * 1000);

  // std::cout <<"SharedAccumImageLinux::Lock for " << ts.tv_sec << "seconds; " << ts.tv_nsec << "nanoseconds" << std::endl;

  int res = 0;
  while ((res = sem_timedwait(m_mutex, &ts)) == -1 && errno == EINTR)
    continue;
  if(res == -1)
  {
    if (errno == ETIMEDOUT)
      std::cout << "sem_timedwait(SharedAccumImageLinux::Lock) time out\n";
    else
      perror("sem_timedwait(SharedAccumImageLinux::Lock)");

    return false;
  }
  else
  {
    return true;
  }

  /*sem_wait(m_mutex);
  perror("sem_wait");*/
}

void SharedAccumImageLinux::Unlock()
{
  sem_post(m_mutex);
}

float* SharedAccumImageLinux::ImageData(int layerId)
{
  auto* pHeader = (HRSharedBufferHeader*)m_memory;
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
