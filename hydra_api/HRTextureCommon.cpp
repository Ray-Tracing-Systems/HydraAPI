#include "HydraInternal.h"
#include "HydraObjectManager.h"

#include <fstream>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <wchar.h>

#pragma warning(disable:4996)

#if defined(WIN32)
  #include "FreeImage.h"
  #pragma comment(lib, "FreeImage.lib")
#else
  #include <FreeImage.h>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct BitmapLDRNode : public IHRTextureNode
{
  BitmapLDRNode(uint32_t w, uint32_t h, size_t a_sz, size_t a_chId) : m_width(w), m_height(h), m_sizeInBytes(a_sz), m_chunkId(a_chId) {}

  uint64_t chunkId() const override { return uint64_t(m_chunkId); }
  uint32_t width()   const override { return m_width; }
  uint32_t height()  const override { return m_height; }
  uint32_t bpp()     const override { return 4; }

  uint32_t m_width;
  uint32_t m_height;
  size_t   m_sizeInBytes;
  size_t   m_chunkId;
};

struct BitmapHDRNode : public IHRTextureNode
{
  BitmapHDRNode(uint32_t w, uint32_t h, size_t a_sz, size_t a_chId) : m_width(w), m_height(h), m_sizeInBytes(a_sz), m_chunkId(a_chId) {}

  uint64_t chunkId() const override { return uint64_t(m_chunkId); }
  uint32_t width()   const override { return m_width; }
  uint32_t height()  const override { return m_height; }
  uint32_t bpp()     const override { return 16; }

  uint32_t m_width;
  uint32_t m_height;
  size_t   m_sizeInBytes;
  size_t   m_chunkId;
};

struct BitmapProxy : public IHRTextureNode
{
  BitmapProxy(uint32_t w, uint32_t h, size_t a_sz, size_t a_chId) : m_width(w), m_height(h), m_sizeInBytes(a_sz), m_chunkId(a_chId), m_bytesPerPixel(4) {}

  uint64_t chunkId() const override { return uint64_t(m_chunkId); }
  uint32_t width()   const override { return m_width; }
  uint32_t height()  const override { return m_height; }
  uint32_t bpp()     const override { return 4; }

  uint32_t     m_width;
  uint32_t     m_height;
  size_t       m_sizeInBytes;
  size_t       m_chunkId;
  int          m_bytesPerPixel;
  std::wstring fileName;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern HRObjectManager g_objManager;

std::shared_ptr<IHRTextureNode> HydraFactoryCommon::CreateTexture2DFromMemory(HRTextureNode* pSysObj, int width, int height, int bpp, const void* a_data)
{
  const size_t textureSizeInBytes     = size_t(width)*size_t(height)*size_t(bpp);
  const size_t totalByteSizeOfTexture = textureSizeInBytes + size_t(2 * sizeof(unsigned int));

  const size_t chunkId = g_objManager.scnData.m_vbCache.AllocChunk(totalByteSizeOfTexture, pSysObj->id);
  auto& chunk          = g_objManager.scnData.m_vbCache.chunk_at(chunkId);
  chunk.type           = (bpp <= 4) ? CHUNK_TYPE_IMAGE4UB : CHUNK_TYPE_IMAGE4F;

  if (chunkId == size_t(-1))
  {
    HrError(L"HydraFactoryCommon::CreateTexture2DFromMemory, out of memory, failed to allocate large chunk");
    return nullptr;
  }

  unsigned char* data = (unsigned char*)chunk.GetMemoryNow();
  if (data == nullptr)
  {
    HrError(L"HydraFactoryCommon::CreateTexture2DFromMemory, out of memory unknown error");
    return nullptr;
  }

  unsigned int* pW = (unsigned int*)data;
  unsigned int* pH = pW+1;

  (*pW) = width;
  (*pH) = height;

  data += 2*sizeof(unsigned int);

  memcpy(data, a_data, textureSizeInBytes);

  std::shared_ptr<BitmapLDRNode> p1 = std::make_shared<BitmapLDRNode>(width, height, totalByteSizeOfTexture, chunkId);
  std::shared_ptr<BitmapHDRNode> p2 = std::make_shared<BitmapHDRNode>(width, height, totalByteSizeOfTexture, chunkId);

  std::shared_ptr<IHRTextureNode> p11 = p1;
  std::shared_ptr<IHRTextureNode> p22 = p2;

  return (bpp <= 4) ? p11 : p22;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<IHRTextureNode> CreateTexture2D_WithImageTool(HRTextureNode* pSysObj, const std::wstring& a_fileName)
{
  // load image from file
  //
  const wchar_t* filename = a_fileName.c_str();

  int width, height, bpp;
  bool loaded = g_objManager.m_pImgTool->LoadImageFromFile(filename, 
                                                           width, height, bpp, g_objManager.m_tempBuffer);
  
  if (!loaded)
    return nullptr;

  const size_t totalByteSizeOfTexture = size_t(width*height)*size_t(bpp) + size_t(2)*sizeof(unsigned int);

  // now put image data directly to cache ... 
  //
  size_t chunkId = g_objManager.scnData.m_vbCache.AllocChunk(totalByteSizeOfTexture, pSysObj->id);
  auto& chunk    = g_objManager.scnData.m_vbCache.chunk_at(chunkId);
  chunk.type     = (bpp <= 4) ? CHUNK_TYPE_IMAGE4UB : CHUNK_TYPE_IMAGE4F;

  //unsigned char* data = new unsigned char[size];
  char* data = (char*)chunk.GetMemoryNow();
  if (data == nullptr)
  {
    std::shared_ptr<BitmapProxy> p = std::make_shared<BitmapProxy>(width, height, totalByteSizeOfTexture, chunkId); //#TODO: return nullptr here?
    p->fileName        = a_fileName;
    p->m_bytesPerPixel = bpp;
    return p;
  }

  unsigned int* pW = (unsigned int*)data;
  unsigned int* pH = pW + 1;
  data += 2 * sizeof(unsigned int);

  memcpy(data, g_objManager.m_tempBuffer.data(), size_t(width*height)*size_t(bpp));

  if (g_objManager.m_tempBuffer.size() > TEMP_BUFFER_MAX_SIZE_DONT_FREE)
    g_objManager.m_tempBuffer = g_objManager.EmptyBuffer();

  (*pW) = width;
  (*pH) = height;

  // return resulting object
  //
  std::shared_ptr<BitmapLDRNode> p1 = std::make_shared<BitmapLDRNode>(width, height, totalByteSizeOfTexture, chunkId);
  std::shared_ptr<BitmapHDRNode> p2 = std::make_shared<BitmapHDRNode>(width, height, totalByteSizeOfTexture, chunkId);

  std::shared_ptr<IHRTextureNode> p11 = p1;
  std::shared_ptr<IHRTextureNode> p22 = p2;

  return (bpp == 4) ? p11 : p22;
}

std::shared_ptr<IHRTextureNode> CreateTexture2DImage4UB(HRTextureNode* pSysObj, const std::wstring& a_fileName)
{
  std::string m_fileName(a_fileName.begin(), a_fileName.end());
  std::ifstream fin(m_fileName.c_str(), std::ios::binary);

  if (!fin.is_open())
    return nullptr;

  unsigned int width = 0;
  unsigned int height = 0;

  fin.read((char*)&width, sizeof(unsigned int));
  fin.read((char*)&height, sizeof(unsigned int));

  size_t totalByteSizeOfTexture = size_t(width)*size_t(height)*size_t(sizeof(char)*4);
  totalByteSizeOfTexture += size_t(2 * sizeof(unsigned int));

  size_t chunkId = g_objManager.scnData.m_vbCache.AllocChunk(totalByteSizeOfTexture, pSysObj->id);
  auto& chunk    = g_objManager.scnData.m_vbCache.chunk_at(chunkId);
  chunk.type     = CHUNK_TYPE_IMAGE4UB;

  char* data = (char*)chunk.GetMemoryNow();
  if (data == nullptr)
  {

    std::shared_ptr<BitmapProxy> p = std::make_shared<BitmapProxy>(width, height, totalByteSizeOfTexture, chunkId); //#TODO: return nullptr here?
    p->fileName        = a_fileName;
    p->m_bytesPerPixel = 4;
    return p;
  }

  unsigned int* pW = (unsigned int*)data;
  unsigned int* pH = pW + 1;
  data += 2 * sizeof(unsigned int);

  (*pW) = width;
  (*pH) = height;

  fin.read(data, size_t(width)*size_t(height)*size_t(sizeof(char)*4));

  fin.close();

  std::shared_ptr<BitmapLDRNode> p1 = std::make_shared<BitmapLDRNode>(width, height, totalByteSizeOfTexture, chunkId);

  std::shared_ptr<IHRTextureNode> p11 = p1;

  return p11;
}

std::shared_ptr<IHRTextureNode> CreateTexture2DImage4F(HRTextureNode* pSysObj, const std::wstring& a_fileName)
{
  std::string m_fileName(a_fileName.begin(), a_fileName.end());
  std::ifstream fin(m_fileName.c_str(), std::ios::binary);

  if (!fin.is_open())
    return nullptr;

  unsigned int width = 0;
  unsigned int height = 0;

  fin.read((char*)&width, sizeof(unsigned int));
  fin.read((char*)&height, sizeof(unsigned int));

  size_t totalByteSizeOfTexture = size_t(width)*size_t(height)*size_t(sizeof(float) * 4);
  totalByteSizeOfTexture += size_t(2 * sizeof(unsigned int));

  size_t chunkId = g_objManager.scnData.m_vbCache.AllocChunk(totalByteSizeOfTexture, pSysObj->id);
  auto& chunk    = g_objManager.scnData.m_vbCache.chunk_at(chunkId);
  chunk.type     = CHUNK_TYPE_IMAGE4F;

  char* data = (char*)chunk.GetMemoryNow();
  if (data == nullptr)
  {

    std::shared_ptr<BitmapProxy> p = std::make_shared<BitmapProxy>(width, height, totalByteSizeOfTexture, chunkId); //#TODO: return nullptr here?
    p->fileName        = a_fileName;
    p->m_bytesPerPixel = 16;
    return p;
  }

  unsigned int* pW = (unsigned int*)data;
  unsigned int* pH = pW + 1;
  data += 2 * sizeof(unsigned int);

  (*pW) = width;
  (*pH) = height;

  fin.read(data, size_t(width)*size_t(height)*size_t(sizeof(float)*4));

  fin.close();

  std::shared_ptr<BitmapLDRNode> p1 = std::make_shared<BitmapLDRNode>(width, height, totalByteSizeOfTexture, chunkId);

  std::shared_ptr<IHRTextureNode> p11 = p1;

  return p11;
}

std::shared_ptr<IHRTextureNode> HydraFactoryCommon::CreateTexture2DFromFile(HRTextureNode* pSysObj, const std::wstring& a_fileName)
{
  if(a_fileName.find(L".image4ub") != std::wstring::npos)
  {
    return CreateTexture2DImage4UB(pSysObj, a_fileName);
  }
  else if(a_fileName.find(L".image4f") != std::wstring::npos)
  {
    return CreateTexture2DImage4F(pSysObj, a_fileName);
  }
  else
  {
    return CreateTexture2D_WithImageTool(pSysObj, a_fileName);
  }
}


std::shared_ptr<IHRTextureNode> HydraFactoryCommon::CreateTextureInfoFromChunkFile(HRTextureNode* pSysObj, const wchar_t* a_chunkFileName)
{
  const std::wstring chunkFileName(a_chunkFileName);

  int wh[2] = {0,0};
  int bpp = 4;

  if (chunkFileName.find(L".image4ub") != std::wstring::npos)
    bpp = 4;
  else if(chunkFileName.find(L".image4f") != std::wstring::npos)
    bpp = 16;

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
  std::wstring s1(a_chunkFileName);
  std::string  s2(s1.begin(), s1.end());
  std::ifstream fin(s2.c_str(), std::ios::binary);
#elif defined WIN32
  std::ifstream fin(a_chunkFileName, std::ios::binary);
#endif

  fin.read((char*)wh, sizeof(int) * 2);
  fin.close();

  // now put all this to a custom implementations of IHRTextureNode
  //
  struct BitmapInfo : public IHRTextureNode
  {
    uint64_t chunkId() const { return m_chunkId; }
    uint32_t width()   const { return m_width; }
    uint32_t height()  const { return m_height; }
    uint32_t bpp()     const { return m_bpp; }

    uint64_t m_chunkId;
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_bpp;
  };

  // cut id of chunk and convert it from string to int
  //
  // auto idBeginPos = chunkFileName.find(L"chunk_") + wcslen(L"chunk_");
  // auto idEndPos   = chunkFileName.find(L".image");
  // auto idStrSize  = idEndPos - idBeginPos;
  // 
  // std::wstring id = chunkFileName.substr(idBeginPos, idStrSize);
  // 
  // uint64_t chunkId;
  // std::wstringstream strIn(id.c_str());
  // strIn >> chunkId;

  std::shared_ptr<BitmapInfo> pBitMapIndo = std::make_shared<BitmapInfo>();

  pBitMapIndo->m_chunkId = -1;
  pBitMapIndo->m_width   = uint32_t(wh[0]);
  pBitMapIndo->m_height  = uint32_t(wh[1]);
  pBitMapIndo->m_bpp     = uint32_t(bpp);

  return pBitMapIndo;
}


void GetTextureFileInfo(const wchar_t* a_fileName, int32_t* pW, int32_t* pH, size_t* pByteSize)
{
  FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

#if defined WIN32
  fif = FreeImage_GetFileTypeU(a_fileName, 0);
#else
  char filename_s[256];
  size_t len = wcstombs(filename_s, a_fileName, sizeof(filename_s));
  fif = FreeImage_GetFileType(filename_s, 0);
#endif

  if (fif == FIF_UNKNOWN)
#if defined WIN32
    fif = FreeImage_GetFIFFromFilenameU(a_fileName);
#else
    fif = FreeImage_GetFIFFromFilename(filename_s);
#endif
  if (fif == FIF_UNKNOWN)
  {
    (*pW)        = 0;
    (*pH)        = 0;
    (*pByteSize) = 0;
    return;
  }

  //check that the plugin has reading capabilities and load the file
  //

  FIBITMAP* dib = nullptr;

  if (FreeImage_FIFSupportsReading(fif))
#if defined WIN32
    dib = FreeImage_LoadU(fif, a_fileName);
#else
    dib = FreeImage_Load(fif, filename_s);
#endif
  else
  {
    (*pW) = 0;
    (*pH) = 0;
    (*pByteSize) = 0;
    return;
  }

  if(dib == nullptr)
  {
    (*pW) = 0;
    (*pH) = 0;
    (*pByteSize) = 0;
    return;
  }

  auto width  = FreeImage_GetWidth(dib);
  auto height = FreeImage_GetHeight(dib);
  auto bpp    = FreeImage_GetBPP(dib);
  if (bpp <= 24) 
    bpp = 32;
  else if (bpp < 128) 
    bpp = 128;

  FreeImage_Unload(dib);

  (*pW)        = width;
  (*pH)        = height;
  (*pByteSize) = bpp/8;
}
