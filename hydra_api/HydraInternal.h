#pragma once

/**
\file
\brief This file is about implementation of different subtypes.

*/

#include "HydraAPI.h"
#include "HydraRenderDriverAPI.h"

#include <string>
#include <vector>
#include <iostream>
#include <memory>

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
#include <experimental/filesystem>
#include <cstring>
#elif defined WIN32
#include <filesystem>
#endif

namespace std_fs = std::experimental::filesystem; // change to std::filesystem for C++17

/**
\brief Abstract HydraObjectImplementation

 Each HRObject have only 2 places of keeping data. 
 
 The first  one is the XML (for small data) - all objects are mostly represented in XML nodes.
 
 The second one is the VirtualBuffer (for big data) - infinite linear memory space that stored on disk and cached in shmem with some strategy.  

 IHRObject is responsible for second (big) data and for checking attributes and node in first (small) data API got xml parameters from user. 
 
 For some objects with predefined parameters IHRObject subtype can store a copies of some XML attributes or nodes.
 For example, it is convinient to store bitmap texture width and height locally in class as integer variables. And e.t.c.

 IHRObject subtype can also store some additional variables that are strictly internal and not visible to _ANY_ other part of the system. 

*/

struct IHRObject 
{
  IHRObject() {}
  virtual ~IHRObject(){}

  virtual size_t DataSizeInBytes() const { return 0; }                        ///< The size of the second part (big data in virtual buffer) in bytes.
  virtual void   DataSerialize(void* p, size_t a_sizeInBytes) {}              ///< ???
  virtual void   DataDeserialize(const void* pFrom, size_t a_sizeInBytes) {}  ///< ???

protected:

  std::wstring m_name;

};

struct IHRMesh : public IHRObject ///< Not empty Data (reimplement DataSerialize/DataDeserialize)
{
  IHRMesh() {}
  virtual ~IHRMesh() {}

  virtual uint64_t chunkId() const { return uint64_t(-1); }
  virtual uint64_t offset(const wchar_t* a_arrayname) const { return uint64_t(-1); }
  virtual uint64_t vertNum() const { return 0; }
  virtual uint64_t indNum() const { return 0; }

  virtual const std::vector<HRBatchInfo>& MList() const = 0;

};

struct HRLight;

struct IHRLight : public IHRObject
{
  IHRLight(){}
  virtual ~IHRLight() {}

protected:

};

struct IHRMat : public IHRObject
{
  IHRMat(){}
  virtual ~IHRMat() {}
};

struct IHRCam : public IHRObject
{
  IHRCam(){}
  virtual ~IHRCam(){}
};

struct IHRTextureNode : public IHRObject ///< Not empty Data (reimplement DataSerialize/DataDeserialize)
{
  IHRTextureNode(){}
  virtual ~IHRTextureNode(){}

  virtual uint64_t chunkId() const { return uint64_t(-1); }
  virtual uint32_t width()   const { return 0; }
  virtual uint32_t height()  const { return 0; }
  virtual uint32_t bpp()     const { return 0; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct IHRSceneData : public IHRObject
{
  IHRSceneData(){}
  virtual ~IHRSceneData(){}
};

struct IHRSceneInst : public IHRObject
{
  IHRSceneInst(){}
  virtual ~IHRSceneInst(){}
};

struct IHRRender : public IHRObject
{
  IHRRender(){}
  virtual ~IHRRender(){}
};

struct HRLight;
struct HRMaterial;
struct HRTextureNode;
struct HRMesh;

struct IHydraFactory
{
  IHydraFactory() {}
  virtual ~IHydraFactory() {}

  virtual std::shared_ptr<IHRTextureNode> CreateTexture2DFromFile(HRTextureNode* pSysObj, const std::wstring& a_fileName)              = 0;
  virtual std::shared_ptr<IHRTextureNode> CreateTexture2DFromMemory(HRTextureNode* pSysObj, int w, int h, int bpp, const void* a_data) = 0;
  virtual std::shared_ptr<IHRTextureNode> CreateTextureInfoFromChunkFile(HRTextureNode* pSysObj, const wchar_t* a_chunkFileName)       = 0;

  virtual std::shared_ptr<IHRMesh>        CreateVSGFFromSimpleInputMesh(HRMesh* pSysObj)                                               = 0;
  virtual std::shared_ptr<IHRMesh>        CreateVSGFFromFile(HRMesh* pSysObj, const std::wstring& a_fileName)                          = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum HR_LIGHT_SHAPE_TYPE {
  HR_LIGHT_SHAPE_POINT     = 1,
  HR_LIGHT_SHAPE_RECTANGLE = 2,
  HR_LIGHT_SHAPE_DISC      = 3,
  HR_LIGHT_SHAPE_SPHERE    = 4,
  HR_LIGHT_SHAPE_SKY_DOME  = 5,
};

enum HR_LIGHT_DISTR_TYPE {
  HR_LIGHT_DISTR_OMNI    = 1,
  HR_LIGHT_DISTR_LAMBERT = 2,
  HR_LIGHT_DISTR_SPOT    = 3,
  HR_LIGHT_DISTR_IES     = 4,
};

struct HRLightContainerPoint : public IHRLight
{
  HRLightContainerPoint(HRLight* a_pSysObject);
  virtual ~HRLightContainerPoint() {}
};

struct HRLightContainerArea : public IHRLight
{
  HRLightContainerArea(HRLight* a_pSysObject);
  virtual ~HRLightContainerArea() {}
};


struct HRLightContainerSky : public IHRLight
{
  HRLightContainerSky(HRLight* a_pSysObject);
  virtual ~HRLightContainerSky() {}
};


/// Camera projection type

enum HR_CAMERA_TYPES {
  HR_CAMERA_PERSPECTIVE = 0, ///<  Perspective projection
  HR_CAMERA_ORTHO       = 1, ///<  Orthographic projection
  HR_CAMERA_SPHERICAL   = 2, ///<  Spherical panoramma projection
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


struct VirtualBuffer;

enum CHUNK_TYPE { CHUNK_TYPE_UNKNOWN  = 0,
                  CHUNK_TYPE_ARRAY1F  = 1, 
                  CHUNK_TYPE_ARRAY1UI = 2, 
                  CHUNK_TYPE_ARRAY2F  = 3, 
                  CHUNK_TYPE_ARRAY4F  = 4,
                  CHUNK_TYPE_IMAGE4UB = 5,
                  CHUNK_TYPE_IMAGE4F  = 6,
                  CHUNK_TYPE_IMAGE4HF = 7,
                  CHUNK_TYPE_VSGF     = 8,
};


/**

\brief This is like a smarp pointer, well, may be not so smart ... just a pointer to chunk )

*/
struct ChunkPointer
{
  ChunkPointer()                     : localAddress(-1), sizeInBytes(0), id(0), inUse(true), pVB(nullptr), useCounter(0), type(CHUNK_TYPE_UNKNOWN) {}
  ChunkPointer(VirtualBuffer* a_pVB) : localAddress(-1), sizeInBytes(0), id(0), inUse(true), pVB(a_pVB), useCounter(0), type(CHUNK_TYPE_UNKNOWN) {}

  void* GetMemoryNow();

  //void SwapToMemory();
  void SwapToDisk();
  bool InMemory() const { return (localAddress != uint64_t(-1)); }

  uint64_t localAddress; ///< an offset in shmem to this chunk
  uint64_t sizeInBytes;
  uint64_t id;
  uint32_t useCounter;
  
  CHUNK_TYPE type;
  bool       inUse;

protected:

  VirtualBuffer* pVB;
};


#ifdef WIN32
  #include <windows.h>
#endif

/**
\brief Infinite linear memory space that stored on disk and cached in shmem with some strategy (copying collector currently ... ).
       The VirtualBuffer is an allocator or a pool. It is infinite and addressed with uint64_t;

*/

struct VirtualBuffer
{
  VirtualBuffer() : m_data(nullptr), m_dataHalfCurr(nullptr), m_dataHalfFree(nullptr), 
                    m_currTop(0), m_currSize(0), m_totalSize(0), m_totalSizeAllocated(0)
  {
  #ifdef WIN32
    m_fileHandle = 0;
  #else
    // some thing from unix ... 
  #endif
  }

  bool Init(uint64_t a_sizeInBytes, const char* a_shmemName);
  void Destroy();
  void Clear();
  void FlushToDisc();

  size_t size() const { return m_allChunks.size(); }
  size_t AllocChunk(uint64_t a_dataSizeInBytes, uint64_t a_objId); ///< 

  void   ResizeAndAllocEmptyChunks(uint64_t a_ckunksNum);

  // 
  //
  inline ChunkPointer  chunk_at(size_t a_id) const { return m_allChunks[a_id]; }
  inline ChunkPointer& chunk_at(size_t a_id)       { return m_allChunks[a_id]; }

  inline uint64_t GetCacheSizeInBytes() const { return m_currSize; }

protected:

  friend struct ChunkPointer;

  char* AllocInCacheNow(uint64_t a_sizeInBytes);
  void* AllocInCache(uint64_t a_sizeInBytes); ///< Always alloc aligned 16 byte memory;
  void  RunCopyingCollector();

  inline uint64_t maxAccumulatedSize() const { return m_currSize / 2; }

  void* m_data;

  char* m_dataHalfCurr;
  char* m_dataHalfFree;

  uint64_t m_currTop;
  uint64_t m_currSize;
  uint64_t m_totalSize;
  uint64_t m_totalSizeAllocated;

#ifdef WIN32
  HANDLE m_fileHandle;
#else
  int m_fileDescriptor;
  std::string shmemName;
#endif

  std::vector<ChunkPointer> m_allChunks;
  std::vector<size_t>       m_chunksIdInMemory;
};

std::wstring ChunkName(const ChunkPointer& a_chunk);

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
  int  hr_mkdir(const char* a_folder);
  int  hr_cleardir(const char* a_folder);
#elif defined WIN32
  int  hr_mkdir(const wchar_t* a_folder);
  void hr_cleardir(const wchar_t* a_folder);
#endif

//#TODO: hr_mutex ? (or do that via connection class)
//#TODO: hr_shmem ?

struct HRSceneInst;
struct IHRRenderDriver;

void HR_DriverUpdate(HRSceneInst& scn, IHRRenderDriver* a_pDriver);
void HR_DriverDraw(HRSceneInst& scn, IHRRenderDriver* a_pDriver);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MESSAGE_SIZE 1024

struct HRSharedBufferHeader
{
  int   width;     // 
  int   height;    // 
  int   depth;     // count update times; if does not change in time, seems no connection;
  float spp;       // 

  int64_t totalByteSize;
  int     messageSendOffset;
  int     messageRcvOffset;

  int   imageDataOffset; // up to 4 image layers currently
  int   counterRcv;
  int   counterSnd;
  int   dummy3;
};

struct IHRSharedAccumImage
{
  IHRSharedAccumImage() {}
  virtual ~IHRSharedAccumImage() {}

  virtual bool Create(int w, int h, int d, const char* name, char errMsg[256]) = 0; 
  virtual bool Attach(const char* name, char errMsg[256])                      = 0;

  virtual void Clear() = 0;
                 
  virtual bool Lock(int a_miliseconds) = 0;
  virtual void Unlock() = 0;

  virtual HRSharedBufferHeader* Header()  = 0;

  virtual char*   MessageSendData()       = 0;
  virtual char*   MessageRcvData()        = 0;
  virtual float*  ImageData(int layerNum) = 0; /// guarantee that returned pointer is aligned16 !!!
};

IHRSharedAccumImage* CreateImageAccum();
