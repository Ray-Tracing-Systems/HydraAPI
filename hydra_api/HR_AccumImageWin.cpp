#pragma once

#include <vector>
#include <string>
#include <cstdint>

#if _MSC_VER >= 1400
#include <intrin.h>
#else
#include <xmmintrin.h>
#endif

#include <cstring>
#include "HydraInternal.h"

#include <windows.h>

struct SharedAccumImageWin32 : public IHRSharedAccumImage
{
  SharedAccumImageWin32();
  ~SharedAccumImageWin32();

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

  HANDLE m_buffHandle;
  HANDLE m_mutex;
  char*  m_memory;

  char*  m_msgSend;
  char*  m_msgRcv;
  float* m_images;

  std::string m_shmemName;
};

SharedAccumImageWin32::SharedAccumImageWin32() : m_buffHandle(NULL), m_mutex(NULL), m_memory(nullptr), m_msgSend(nullptr), m_msgRcv(nullptr), m_images(nullptr)
{

}

SharedAccumImageWin32::~SharedAccumImageWin32()
{
  Free();
}

void SharedAccumImageWin32::Free()
{
  if (m_mutex != INVALID_HANDLE_VALUE && m_mutex != NULL)
    CloseHandle(m_mutex);
  m_mutex = NULL;

  if (m_memory != nullptr)
    UnmapViewOfFile(m_memory);
  m_memory = nullptr;

  if (m_buffHandle != INVALID_HANDLE_VALUE && m_buffHandle != NULL)
    CloseHandle(m_buffHandle);
  m_buffHandle = NULL;

  m_msgSend = nullptr;
  m_msgRcv  = nullptr;
  m_images  = nullptr;
}


bool SharedAccumImageWin32::Create(int a_width, int a_height, int a_depth, const char* a_name, char a_errMsg[256])
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

    Free();
    m_shmemName = a_name;
    
    const int64_t totalSize     = int64_t(sizeof(HRSharedBufferHeader)) + int64_t(MESSAGE_SIZE * 2) + int64_t(a_width*a_height)*int64_t(a_depth*sizeof(float)*4) + int64_t(1024);
    const std::string mutexName = std::string(a_name) + "_mutex";

    m_mutex = CreateMutexA(NULL, FALSE, mutexName.c_str());

    if (m_mutex == NULL || m_mutex == INVALID_HANDLE_VALUE)
    {
      strcpy(a_errMsg, "FAILED to create mutex (CreateMutexA)");
      return false;
    }

    DWORD high = (DWORD)(totalSize >> 32);
    DWORD low  = (DWORD)(totalSize & 0x00000000FFFFFFFF);

    m_buffHandle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, high, low, a_name);

    if (m_buffHandle == NULL || m_buffHandle == INVALID_HANDLE_VALUE)
    {
      strcpy(a_errMsg, "FAILED to alloc shared memory (CreateFileMappingA)");
      Free();
      return false;
    }

    m_memory = (char*)MapViewOfFile(m_buffHandle, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0);
    if(m_memory == nullptr)
    {
      strcpy(a_errMsg, "FAILED to map shared memory (MapViewOfFile)");
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

  strcpy(a_errMsg, "");
  return true;
}

bool SharedAccumImageWin32::Attach(const char* name, char errMsg[256])
{
  memset(errMsg, 0, 256);
  Free();

  const std::string mutexName = std::string(name) + "_mutex";
  
  m_mutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, mutexName.c_str());
  if (m_mutex == NULL || m_mutex == INVALID_HANDLE_VALUE)
  {
    strcpy(errMsg, "FAILED to attach mutex (OpenMutexA)");
    return false;
  }

  m_buffHandle = OpenFileMappingA(FILE_MAP_READ | FILE_MAP_WRITE, 0, name);

  if (m_buffHandle == NULL || m_buffHandle == INVALID_HANDLE_VALUE)
  {
    strcpy(errMsg, "FAILED to attach shmem (OpenFileMappingA)");
    Free();
    return false;
  }

  m_memory = (char*)MapViewOfFile(m_buffHandle, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0);
  if (m_memory == nullptr)
  {
    strcpy(errMsg, "FAILED to map shared memory (MapViewOfFile)");
    Free();
    return false;
  }

  AttachTo(m_memory);

  strcpy(errMsg, "");
  return true;
}


void SharedAccumImageWin32::Clear()
{
  auto* pHeader = Header();
  pHeader->spp = 0.0f;
  pHeader->counterRcv = 0;
  pHeader->counterSnd = 0;

  auto pImg = ImageData(0);
  memset(pImg, 0, size_t(pHeader->width*pHeader->height)*size_t(sizeof(float)*4));
}


void SharedAccumImageWin32::AttachTo(char* a_memory)
{
  HRSharedBufferHeader* pHeader = (HRSharedBufferHeader*)m_memory;

  m_msgSend = m_memory + pHeader->messageSendOffset;
  m_msgRcv  = m_memory + pHeader->messageRcvOffset;
  m_images  = (float*)(m_memory + pHeader->imageDataOffset);
}

bool SharedAccumImageWin32::Lock(int a_miliseconds)
{
  const DWORD res = WaitForSingleObject(m_mutex, a_miliseconds);

  if (res == WAIT_TIMEOUT || res == WAIT_FAILED)
    return false;
  else
    return true;
}

void SharedAccumImageWin32::Unlock()
{
  ReleaseMutex(m_mutex);
}

float* SharedAccumImageWin32::ImageData(int layerId)
{
  HRSharedBufferHeader* pHeader = (HRSharedBufferHeader*)m_memory;
  return m_images + int64_t(pHeader->width*pHeader->height)*int64_t(layerId*4);
}

char* SharedAccumImageWin32::MessageSendData()
{
  return m_msgSend;
}

char* SharedAccumImageWin32::MessageRcvData()
{
  return m_msgRcv;
}

HRSharedBufferHeader* SharedAccumImageWin32::Header()
{
  return (HRSharedBufferHeader*)m_memory;
}


IHRSharedAccumImage* CreateImageAccum()
{
  return new SharedAccumImageWin32();
}
