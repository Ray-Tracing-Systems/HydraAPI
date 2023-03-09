#include "HydraInternal.h"
#include "HydraObjectManager.h"

#include <fstream>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cwchar>

#pragma warning(disable:4996)

#if defined(WIN32)
  #include "FreeImage.h"
  //#pragma comment(lib, "FreeImage.lib")
#else
  #include <FreeImage.h>
#endif

extern HRObjectManager g_objManager;

size_t IHRTextureNode::DataSizeInBytes() const
{
  return size_t(width()*height())*size_t(bpp());
}

bool IHRTextureNode::ReadDataFromChunkTo(std::vector<int>& a_dataContainer)
{
  auto chunk = g_objManager.scnData.m_vbCache.chunk_at(chunkId());

  auto sizeInInts = DataSizeInBytes() / sizeof(int) + 1;
  if (a_dataContainer.size() < sizeInInts)
    a_dataContainer.resize(sizeInInts);

  // copy from memory
  //
  if (chunk.InMemory())
  {
    char* data = (char*)chunk.GetMemoryNow();
    if (data != nullptr)
    {
      memcpy(a_dataContainer.data(), data + sizeof(int) * 2, DataSizeInBytes());
      return true;
    }
  }

  // if fail then try to load from file
  //
  std::wstring locPath = g_objManager.scnData.m_path + std::wstring(L"/") + ChunkName(chunk);

  InternalImageTool chunkLoader;

  int w, h, bpp;
  return chunkLoader.LoadImageFromFile(locPath.c_str(),
                                       w, h, bpp, a_dataContainer);
}

const void* IHRTextureNode::GetData() const
{
  if(chunkId() >= g_objManager.scnData.m_vbCache.size())
    return nullptr;
  
  auto chunk = g_objManager.scnData.m_vbCache.chunk_at(chunkId());
  if (!chunk.InMemory())
    return nullptr;

  const char* ptr = (const char*)chunk.GetMemoryNow();
  if (ptr == nullptr)
    return nullptr;

  return (ptr + 2 * sizeof(int));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct BitmapLDRNode : public IHRTextureNode
{
  BitmapLDRNode(uint32_t w, uint32_t h, size_t a_sz, size_t a_chId, uint32_t a_chan) : m_width(w), m_height(h),
  m_sizeInBytes(a_sz), m_chunkId(a_chId), m_bytesPerPixel(a_chan), m_channels(a_chan) {}

  uint64_t chunkId()  const override { return uint64_t(m_chunkId); }
  uint32_t width()    const override { return m_width; }
  uint32_t height()   const override { return m_height; }
  uint32_t bpp()      const override { return m_bytesPerPixel; }
  uint32_t channels() const override { return m_channels; }

  uint32_t m_width;
  uint32_t m_height;
  size_t   m_sizeInBytes;
  size_t   m_chunkId;
  uint32_t m_bytesPerPixel;
  uint32_t m_channels;
};

struct BitmapHDRNode : public IHRTextureNode
{
  BitmapHDRNode(uint32_t w, uint32_t h, size_t a_sz, size_t a_chId, uint32_t a_chan) : m_width(w), m_height(h),
  m_sizeInBytes(a_sz), m_chunkId(a_chId), m_channels(a_chan)
  {
    m_bytesPerPixel = m_channels * 4;
  }

  uint64_t chunkId()  const override { return uint64_t(m_chunkId); }
  uint32_t width()    const override { return m_width; }
  uint32_t height()   const override { return m_height; }
  uint32_t bpp()      const override { return m_bytesPerPixel; }
  uint32_t channels() const override { return m_channels; }

  uint32_t m_width;
  uint32_t m_height;
  size_t   m_sizeInBytes;
  size_t   m_chunkId;
  uint32_t m_bytesPerPixel;
  uint32_t m_channels;
};

struct BitmapProxy : public IHRTextureNode
{
  BitmapProxy(uint32_t w, uint32_t h, size_t a_sz, size_t a_chId, uint32_t a_chan) : m_width(w), m_height(h),
  m_sizeInBytes(a_sz), m_chunkId(a_chId), m_bytesPerPixel(a_chan), m_channels(a_chan) {}

  uint64_t chunkId()  const override { return uint64_t(m_chunkId); }
  uint32_t width()    const override { return m_width; }
  uint32_t height()   const override { return m_height; }
  uint32_t bpp()      const override { return m_bytesPerPixel; }
  uint32_t channels() const override { return m_channels; }

  uint32_t     m_width;
  uint32_t     m_height;
  size_t       m_sizeInBytes;
  size_t       m_chunkId;
  uint32_t     m_bytesPerPixel;
  uint32_t     m_channels;
  std::wstring fileName;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<IHRTextureNode> HydraFactoryCommon::CreateTexture2DFromMemory(HRTextureNode* pSysObj, int width, int height, int bpp, int chan, const void* a_data)
{
  const size_t textureSizeInBytes     = size_t(width)*size_t(height)*size_t(bpp);
  const size_t totalByteSizeOfTexture = textureSizeInBytes + size_t(2 * sizeof(unsigned int));
  
  const size_t chunkId                = g_objManager.scnData.m_vbCache.AllocChunk(totalByteSizeOfTexture, pSysObj->id);

  if (chunkId == size_t(-1))
  {
    HrError(L"HydraFactoryCommon::CreateTexture2DFromMemory, out of memory, failed to allocate large chunk");
    return nullptr;
  }

  auto& chunk         = g_objManager.scnData.m_vbCache.chunk_at(chunkId);
  if(chan == 1 && bpp == 1)
    chunk.type = CHUNK_TYPE_IMAGE1UB;
  else if(chan == 4 && bpp <= 4)
    chunk.type = CHUNK_TYPE_IMAGE4UB;
  else if(chan == 1 && bpp == 4)
    chunk.type = CHUNK_TYPE_IMAGE1F;
  else if(chan == 4 && bpp > 4)
    chunk.type = CHUNK_TYPE_IMAGE4F;

  auto data = (unsigned char*)chunk.GetMemoryNow();

  if (data == nullptr)
  {
    HrError(L"HydraFactoryCommon::CreateTexture2DFromMemory, out of memory unknown error");
    return nullptr;
  }

  auto pW = (unsigned int*)data;
  unsigned int* pH = pW+1;

  (*pW) = width;
  (*pH) = height;

  data += 2 * sizeof(unsigned int);

  memcpy(data, a_data, textureSizeInBytes);

  std::shared_ptr<BitmapLDRNode> p1 = std::make_shared<BitmapLDRNode>(width, height, totalByteSizeOfTexture, chunkId, chan);
  std::shared_ptr<BitmapHDRNode> p2 = std::make_shared<BitmapHDRNode>(width, height, totalByteSizeOfTexture, chunkId, chan);

  std::shared_ptr<IHRTextureNode> p11 = p1;
  std::shared_ptr<IHRTextureNode> p22 = p2;

  return (chunk.type == CHUNK_TYPE_IMAGE1F || chunk.type == CHUNK_TYPE_IMAGE4F) ? p22 : p11;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<IHRTextureNode> CreateTexture2D_WithImageTool(HRTextureNode* pSysObj, const std::wstring& a_fileName)
{
  // load image from file
  //
  const wchar_t* filename = a_fileName.c_str();

  int width, height, bpp, chan;
  std::vector<unsigned char> tmpBuf;
  bool loaded = g_objManager.m_pImgTool->LoadImageFromFile(filename,width, height, bpp, chan, tmpBuf);

  if (!loaded)
    return nullptr;

  const size_t totalByteSizeOfTexture = size_t(width*height)*size_t(bpp) + size_t(2)*sizeof(unsigned int);

  // now put image data directly to cache ... 
  //
  size_t chunkId = g_objManager.scnData.m_vbCache.AllocChunk(totalByteSizeOfTexture, pSysObj->id);
  auto& chunk    = g_objManager.scnData.m_vbCache.chunk_at(chunkId);
  if(chan == 1 && bpp == 1)
    chunk.type = CHUNK_TYPE_IMAGE1UB;
  else if(chan == 4 && bpp <= 4)
    chunk.type = CHUNK_TYPE_IMAGE4UB;
  else if(chan == 1 && bpp == 4)
    chunk.type = CHUNK_TYPE_IMAGE1F;
  else if(chan == 4 && bpp > 4)
    chunk.type = CHUNK_TYPE_IMAGE4F;

  //unsigned char* data = new unsigned char[size];
  char* data = (char*)chunk.GetMemoryNow();
  if (data == nullptr)
  {
    std::shared_ptr<BitmapProxy> p = std::make_shared<BitmapProxy>(width, height, totalByteSizeOfTexture, chunkId, chan);
    p->fileName        = a_fileName;
    p->m_bytesPerPixel = bpp;
    return p;
  }

  unsigned int* pW = (unsigned int*)data;
  unsigned int* pH = pW + 1;
  data += 2 * sizeof(unsigned int);

  memcpy(data, tmpBuf.data(), size_t(width * height) * size_t(bpp));

  tmpBuf = {};

  (*pW) = width;
  (*pH) = height;

  // return resulting object
  //
  std::shared_ptr<BitmapLDRNode> p1 = std::make_shared<BitmapLDRNode>(width, height, totalByteSizeOfTexture, chunkId, chan);
  std::shared_ptr<BitmapHDRNode> p2 = std::make_shared<BitmapHDRNode>(width, height, totalByteSizeOfTexture, chunkId, chan);

  std::shared_ptr<IHRTextureNode> p11 = p1;
  std::shared_ptr<IHRTextureNode> p22 = p2;

  return (chunk.type == CHUNK_TYPE_IMAGE1F || chunk.type == CHUNK_TYPE_IMAGE4F) ? p22 : p11;
}

std::shared_ptr<IHRTextureNode> CreateTexture2DImageUB(HRTextureNode* pSysObj, const std::wstring& a_fileName, int chan)
{
  std::string m_fileName(a_fileName.begin(), a_fileName.end());
  std::ifstream fin(m_fileName.c_str(), std::ios::binary);

  if (!fin.is_open())
    return nullptr;

  unsigned int width = 0;
  unsigned int height = 0;

  fin.read((char*)&width, sizeof(unsigned int));
  fin.read((char*)&height, sizeof(unsigned int));

  size_t totalByteSizeOfTexture = size_t(width) * size_t(height) * size_t(sizeof(char) * chan);
  totalByteSizeOfTexture += size_t(2 * sizeof(unsigned int));

  size_t chunkId = g_objManager.scnData.m_vbCache.AllocChunk(totalByteSizeOfTexture, pSysObj->id);
  auto& chunk    = g_objManager.scnData.m_vbCache.chunk_at(chunkId);
  if(chan == 1)
    chunk.type = CHUNK_TYPE_IMAGE1UB;
  else if (chan == 4)
    chunk.type = CHUNK_TYPE_IMAGE4UB;
  else
    HrError(L"[CreateTexture2DImageUB]: unsupported image channel number", chan);

  char* data = (char*)chunk.GetMemoryNow();
  if (data == nullptr)
  {
    std::shared_ptr<BitmapProxy> p = std::make_shared<BitmapProxy>(width, height, totalByteSizeOfTexture, chunkId, chan); //#TODO: return nullptr here?
    p->fileName        = a_fileName;
    return p;
  }

  unsigned int* pW = (unsigned int*)data;
  unsigned int* pH = pW + 1;
  data += 2 * sizeof(unsigned int);

  (*pW) = width;
  (*pH) = height;

  fin.read(data, size_t(width) * size_t(height) * size_t(sizeof(char) * chan));

  fin.close();

  std::shared_ptr<BitmapLDRNode> p1 = std::make_shared<BitmapLDRNode>(width, height, totalByteSizeOfTexture, chunkId, chan);

  std::shared_ptr<IHRTextureNode> p11 = p1;

  return p11;
}

std::shared_ptr<IHRTextureNode> CreateTexture2DImageF(HRTextureNode* pSysObj, const std::wstring& a_fileName, int chan)
{
  std::string m_fileName(a_fileName.begin(), a_fileName.end());
  std::ifstream fin(m_fileName.c_str(), std::ios::binary);

  if (!fin.is_open())
    return nullptr;

  unsigned int width = 0;
  unsigned int height = 0;

  fin.read((char*)&width, sizeof(unsigned int));
  fin.read((char*)&height, sizeof(unsigned int));

  size_t totalByteSizeOfTexture = size_t(width)*size_t(height)*size_t(sizeof(float) * chan);
  totalByteSizeOfTexture += size_t(2 * sizeof(unsigned int));

  size_t chunkId = g_objManager.scnData.m_vbCache.AllocChunk(totalByteSizeOfTexture, pSysObj->id);
  auto& chunk    = g_objManager.scnData.m_vbCache.chunk_at(chunkId);
  if(chan == 1)
    chunk.type = CHUNK_TYPE_IMAGE1F;
  else if (chan == 4)
    chunk.type = CHUNK_TYPE_IMAGE4F;
  else
    HrError(L"[CreateTexture2DImageF]: unsupported image channel number", chan);

  char* data = (char*)chunk.GetMemoryNow();
  if (data == nullptr)
  {

    std::shared_ptr<BitmapProxy> p = std::make_shared<BitmapProxy>(width, height, totalByteSizeOfTexture, chunkId, chan); //#TODO: return nullptr here?
    p->fileName        = a_fileName;
    return p;
  }

  unsigned int* pW = (unsigned int*)data;
  unsigned int* pH = pW + 1;
  data += 2 * sizeof(unsigned int);

  (*pW) = width;
  (*pH) = height;

  fin.read(data, size_t(width) * size_t(height) * size_t(sizeof(float) * chan));

  fin.close();

  std::shared_ptr<BitmapLDRNode> p1 = std::make_shared<BitmapLDRNode>(width, height, totalByteSizeOfTexture, chunkId, chan);

  std::shared_ptr<IHRTextureNode> p11 = p1;

  return p11;
}

std::shared_ptr<IHRTextureNode> HydraFactoryCommon::CreateTexture2DFromFile(HRTextureNode* pSysObj, const std::wstring& a_fileName)
{
  if(a_fileName.find(L".image4ub") != std::wstring::npos)
  {
    return CreateTexture2DImageUB(pSysObj, a_fileName, 4);
  }
  if(a_fileName.find(L".image1ub") != std::wstring::npos)
  {
    return CreateTexture2DImageUB(pSysObj, a_fileName, 1);
  }
  else if(a_fileName.find(L".image4f") != std::wstring::npos)
  {
    return CreateTexture2DImageF(pSysObj, a_fileName, 4);
  }
  else if(a_fileName.find(L".image1f") != std::wstring::npos)
  {
    return CreateTexture2DImageF(pSysObj, a_fileName, 1);
  }
  else
  {
    return CreateTexture2D_WithImageTool(pSysObj, a_fileName);
  }
}

int32_t ChunkIdFromFileName(const wchar_t* a_chunkFileName)
{
  std::wstring fileName(a_chunkFileName);
  
  const std::wstring chnk_str(L"chunk_");

  auto posBeg = fileName.find(chnk_str);
  auto posEnd = fileName.find_last_of(L'.');
  if (posBeg == std::wstring::npos || posEnd == std::wstring::npos)
    return -1;

  posBeg += chnk_str.size();
  
  const auto strid  = fileName.substr(posBeg, posEnd - posBeg);
  int res = std::stoi(strid);

  return res; 
}

std::shared_ptr<IHRTextureNode> HydraFactoryCommon::CreateTextureInfoFromChunkFile(HRTextureNode* pSysObj, const wchar_t* a_chunkFileName, pugi::xml_node a_node)
{
  // width="64" height="64" bytesize="16384"
  int wh[2];
  wh[0] = a_node.attribute(L"width").as_int();
  wh[1] = a_node.attribute(L"height").as_int();

  auto id = a_node.attribute(L"id").as_int();
  
  if(std::wstring(a_node.attribute(L"type").as_string()) == L"proc")
    return nullptr;
  
  if(wh[0] == 0 || wh[1] == 0)
  {
    HrError(L"HydraFactoryCommon::CreateTextureInfoFromChunkFile, unknown image resolution is not allowed! TexId = ", id);
    return nullptr;
  }

  const size_t bytesize = size_t(a_node.attribute(L"bytesize").as_llong());
  const int bpp         = int( bytesize/(wh[0]*wh[1]) );

  // now put all this to a custom implementations of IHRTextureNode
  //
  struct BitmapInfo : public IHRTextureNode
  {

    BitmapInfo() : m_chunkId(uint64_t(-1)), m_width(0), m_height(0), m_bpp(0), m_channels(0) {}

    uint64_t chunkId()  const override { return m_chunkId; }
    uint32_t width()    const override { return m_width;   }
    uint32_t height()   const override { return m_height;  }
    uint32_t bpp()      const override { return m_bpp;     }
    uint32_t channels() const override { return m_channels;     }

    uint64_t m_chunkId;
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_bpp;
    uint32_t m_channels;
  };

  std::shared_ptr<BitmapInfo> pBitMapInfo = std::make_shared<BitmapInfo>();

  pBitMapInfo->m_chunkId  = ChunkIdFromFileName(a_chunkFileName);
  pBitMapInfo->m_width    = uint32_t(wh[0]);
  pBitMapInfo->m_height   = uint32_t(wh[1]);
  pBitMapInfo->m_bpp      = uint32_t(bpp);

  auto ext = CutFileExt(a_chunkFileName);
  if(ext == L".image4ub" || ext == L".image4f")
    pBitMapInfo->m_channels = 4u;
  else if(ext == L".image1ub" || ext == L".image1f")
    pBitMapInfo->m_channels = 1u;

  return pBitMapInfo;
}


void GetTextureFileInfo(const wchar_t* a_fileName, int32_t* pW, int32_t* pH, size_t* pBytesPP, int32_t* pChan)
{
  FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

  fif = g_objManager.m_FreeImageDll.m_pFreeImage_GetFileTypeU(a_fileName, 0);
  if (fif == FIF_UNKNOWN)
    fif = g_objManager.m_FreeImageDll.m_pFreeImage_GetFIFFromFilenameU(a_fileName);

  if (fif == FIF_UNKNOWN)
  {
    (*pW)       = 0;
    (*pH)       = 0;
    (*pBytesPP) = 0;
    (*pChan)    = 0;
    return;
  }

  //check that the plugin has reading capabilities and load the file
  //

  FIBITMAP* dib = nullptr;

  if (g_objManager.m_FreeImageDll.m_pFreeImage_FIFSupportsReading(fif))
  {
    dib = g_objManager.m_FreeImageDll.m_pFreeImage_LoadU(fif, a_fileName, 0);
  }
  else
  {
    (*pW)       = 0;
    (*pH)       = 0;
    (*pBytesPP) = 0;
    (*pChan)    = 0;
    return;
  }

  if (dib == nullptr)
  {
    (*pW)       = 0;
    (*pH)       = 0;
    (*pBytesPP) = 0;
    (*pChan)    = 0;
    return;
  }

  auto type     = g_objManager.m_FreeImageDll.m_pFreeImage_GetImageType(dib);  
  auto width    = g_objManager.m_FreeImageDll.m_pFreeImage_GetWidth(dib);
  auto height   = g_objManager.m_FreeImageDll.m_pFreeImage_GetHeight(dib);
  auto bpp      = g_objManager.m_FreeImageDll.m_pFreeImage_GetBPP(dib);

  if (type      == FIT_FLOAT)
  {
    (*pChan)    = 1;
    (*pBytesPP) = 4;
  }
  else if (type == FIT_RGBF)
  {
    (*pChan)    = 3;
    (*pBytesPP) = 12;
  }
  else if (type == FIT_RGBAF)
  {
    (*pChan)    = 4;
    (*pBytesPP) = 16;
  }
  else if (type == FIT_BITMAP)
  {
    (*pChan)    = 4;
    (*pBytesPP) = 4;
  }

  //    if (bpp <= 24)
  //    bpp = 32;
  //  else if (bpp < 128)
  //    bpp = 128;

  g_objManager.m_FreeImageDll.m_pFreeImage_Unload(dib);

  (*pW) = width;
  (*pH) = height;
  //  (*pBytesPP) = bpp/8;
}
