#pragma once

/**
\file
\brief This file is about implementation of different subtypes.

*/

#include "HydraAPI.h"
#include "HydraRenderDriverAPI.h"
#include "LiteMath.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <memory>

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
#include <experimental/filesystem>
#include <cstring>
#endif

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

  virtual uint64_t    chunkId() const { return uint64_t(-1); }
  virtual size_t      DataSizeInBytes() const { return 0; }                        ///< The size of the second part (big data in virtual buffer) in bytes.
  virtual const void* GetData() const { return nullptr; }
  virtual bool        ReadDataFromChunkTo(std::vector<int>& a_dataConteiner) { return false; }

protected:

  std::wstring m_name;

};

using namespace HRUtils;

BBox transformBBox(const BBox &a_bbox, const HydraLiteMath::float4x4 &m);
BBox mergeBBoxes(const BBox &A, const BBox &B);
BBox createBBoxFromFloat4V(const std::vector<HydraLiteMath::float4> &a_verts);
BBox createBBoxFromFloatV(const std::vector<float> &a_verts, int stride = 4);
std::vector< HydraLiteMath::float4> getVerticesFromBBox(const BBox &a_bbox);

std::wstring HR_PreprocessMeshes(const wchar_t *state_path);
void hrMeshComputeNormals(HRMeshRef a_mesh, int indexNum, bool useFaceNormals = false);


struct IHRMesh : public IHRObject ///< Not empty Data (reimplement DataSerialize/DataDeserialize)
{
  IHRMesh() {}
  virtual ~IHRMesh() {}

  virtual uint64_t chunkId() const { return uint64_t(-1); }
  virtual uint64_t offset(const wchar_t* a_arrayname) const { return uint64_t(-1); }
  virtual uint64_t vertNum() const { return 0; }
  virtual uint64_t indNum() const { return 0; }

  virtual BBox getBBox() const { return BBox();}

  virtual const std::vector<HRBatchInfo>& MList() const = 0;
  virtual const std::unordered_map<std::wstring, std::tuple<std::wstring, size_t, size_t, int> >& GetOffsAndSizeForAttrs() const = 0;
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

  virtual uint32_t width()   const { return 0; }
  virtual uint32_t height()  const { return 0; }
  virtual uint32_t bpp()     const { return 0; }

  size_t      DataSizeInBytes() const override;
  const void* GetData() const override;
  bool        ReadDataFromChunkTo(std::vector<int>& a_dataConteiner) override;
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

  virtual std::shared_ptr<IHRTextureNode> CreateTexture2DFromFile(HRTextureNode* pSysObj, const std::wstring& a_fileName)                               = 0;
  virtual std::shared_ptr<IHRTextureNode> CreateTexture2DFromMemory(HRTextureNode* pSysObj, int w, int h, int bpp, const void* a_data)                  = 0;
  virtual std::shared_ptr<IHRTextureNode> CreateTextureInfoFromChunkFile(HRTextureNode* pSysObj, const wchar_t* a_chunkFileName, pugi::xml_node a_node) = 0;

  virtual std::shared_ptr<IHRMesh>        CreateVSGFFromSimpleInputMesh(HRMesh* pSysObj)                                               = 0;
  virtual std::shared_ptr<IHRMesh>        CreateVSGFFromFile(HRMesh* pSysObj, const std::wstring& a_fileName, pugi::xml_node a_node)   = 0;
};

int32_t ChunkIdFromFileName(const wchar_t* a_chunkFileName);

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


struct VirtualBuffer;

/**
\brief This is like a smarp pointer, well, may be not so smart ... just a pointer to chunk )
*/
struct ChunkPointer
{
  ChunkPointer()                     : localAddress(-1), sizeInBytes(0), id(0), inUse(true), pVB(nullptr), useCounter(0), type(CHUNK_TYPE_UNKNOWN) {}
  ChunkPointer(VirtualBuffer* a_pVB) : localAddress(-1), sizeInBytes(0), id(0), inUse(true), pVB(a_pVB), useCounter(0), type(CHUNK_TYPE_UNKNOWN) {}

  void* GetMemoryNow();
  const void* GetMemoryNow() const;
  
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

  friend struct VirtualBuffer;
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
  VirtualBuffer() : m_data(nullptr), m_chunkTable(nullptr), m_dataHalfCurr(nullptr), m_dataHalfFree(nullptr),
                    m_currTop(0), m_currSize(0), m_totalSize(0), m_totalSizeAllocated(0), m_pTempBuffer(nullptr), m_owner(false)
  {
  #ifdef WIN32
    m_fileHandle = 0;
  #else
    // some thing from unix ... 
  #endif
  }

  bool Init(uint64_t a_sizeInBytes, const char* a_shmemName, std::vector<int>* a_pTempBuffer);
  bool Attach(uint64_t a_sizeInBytes, const char* a_shmemName, std::vector<int>* a_pTempBuffer);
  void RestoreChunks();
  
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

  inline uint64_t       SizeInBytes()    const { return m_currSize; }
  
  inline const int64_t* ChunksTablePtr() const { return m_chunkTable; }
  inline int64_t*       ChunksTablePtr()       { return m_chunkTable; }
  
protected:

  friend struct ChunkPointer;
  
  constexpr static size_t VB_CHUNK_TABLE_SIZE = sizeof(int64_t)*99999;
  constexpr static size_t VB_CHUNK_TABLE_OFFS = 1024;
  
  char* AllocInCacheNow(uint64_t a_sizeInBytes);
  void* AllocInCache(uint64_t a_sizeInBytes); ///< Always alloc aligned 16 byte memory;
  void  RunCopyingCollector();
  void  RunCollector(int a_divisor);

  inline uint64_t maxAccumulatedSize() const { return m_currSize / 2; }

  void*    m_data;
  int64_t* m_chunkTable;

  char* m_dataHalfCurr;
  char* m_dataHalfFree;

  uint64_t m_currTop;
  uint64_t m_currSize;
  uint64_t m_totalSize;
  uint64_t m_totalSizeAllocated;

#ifdef WIN32
  void* m_fileHandle;
#else
  int m_fileDescriptor;
  std::string shmemName;
#endif

  std::vector<ChunkPointer> m_allChunks;
  std::vector<size_t>       m_chunksIdInMemory;
  std::vector<int>*         m_pTempBuffer;
  
  bool m_owner;
};

std::wstring ChunkName(const ChunkPointer& a_chunk);


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
  int  hr_mkdir(const char* a_folder);
  int  hr_cleardir(const char* a_folder);
#elif defined WIN32
  int  hr_mkdir(const wchar_t* a_folder);
  void hr_cleardir(const wchar_t* a_folder);
#endif
  
  void hr_copy_file(const wchar_t* a_file1, const wchar_t* a_file2); //#TODO: implement this on Linux!!!


struct HRSystemMutex;
HRSystemMutex* hr_create_system_mutex(const char* a_mutexName);
void hr_free_system_mutex(HRSystemMutex*& a_mutex);
bool hr_lock_system_mutex(HRSystemMutex* a_mutex, int a_msToWait = 1000);
void hr_unlock_system_mutex(HRSystemMutex* a_mutex);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct HRSceneInst;
struct IHRRenderDriver;

std::unique_ptr<IHRRenderDriver> CreateRenderFromString(const wchar_t *a_className, const wchar_t *a_options);

void HR_DriverUpdate(HRSceneInst& scn, IHRRenderDriver* a_pDriver);
void HR_DriverDraw(HRSceneInst& scn, IHRRenderDriver* a_pDriver);

std::wstring HR_UtilityDriverStart(const wchar_t* state_path);
std::wstring SaveFixedStateXML(pugi::xml_document &doc, const std::wstring &oldPath, const std::wstring &suffix);

HRMeshDriverInput HR_GetMeshDataPointers(size_t a_meshId);

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

  int   imageDataOffset; 
  int   counterRcv;
  int   counterSnd;
  int   gbufferIsEmpty;
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct HydraFactoryCommon : public IHydraFactory
{
  HydraFactoryCommon() {}
  virtual ~HydraFactoryCommon() {}

  std::shared_ptr<IHRTextureNode> CreateTexture2DFromFile(HRTextureNode* pSysObj, const std::wstring& a_fileName) override;
  std::shared_ptr<IHRTextureNode> CreateTexture2DFromMemory(HRTextureNode* pSysObj, int w, int h, int bpp, const void* a_data) override;
  std::shared_ptr<IHRTextureNode> CreateTextureInfoFromChunkFile(HRTextureNode* pSysObj, const wchar_t* a_chunkFileName, pugi::xml_node a_node) override;

  std::shared_ptr<IHRMesh>        CreateVSGFFromSimpleInputMesh(HRMesh* pSysObj) override;
  std::shared_ptr<IHRMesh>        CreateVSGFFromFile(HRMesh* pSysObj, const std::wstring& a_fileName, pugi::xml_node) override;
};



