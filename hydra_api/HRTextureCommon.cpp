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
  BitmapLDRNode(uint32_t w, uint32_t h, size_t a_sz, size_t a_chId) : m_width(w), m_height(h), m_sizeInBytes(a_sz), m_chunkId(a_chId), data(nullptr) {}

  uint64_t chunkId() const override { return uint64_t(m_chunkId); }
  uint32_t width()   const override { return m_width; }
  uint32_t height()  const override { return m_height; }
  uint32_t bpp()     const override { return 4; }

  uint32_t m_width;
  uint32_t m_height;
  size_t   m_sizeInBytes;
  size_t   m_chunkId;

  uint32_t* data;
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

int HRUtils_LoadImageFromFileToPairOfFreeImageObjects(const wchar_t* filename, FIBITMAP*& dib, FIBITMAP*& converted, FREE_IMAGE_FORMAT* pFif)
{
  FREE_IMAGE_FORMAT& fif = (*pFif); // image format

  //check the file signature and deduce its format
  //if still unknown, try to guess the file format from the file extension
  //

#if defined WIN32
  fif = FreeImage_GetFileTypeU(filename, 0);
#else
  char filename_s[256];
  size_t len = wcstombs(filename_s, filename, sizeof(filename_s));
  fif = FreeImage_GetFileType(filename_s, 0);
#endif

  if (fif == FIF_UNKNOWN)
#if defined WIN32
    fif = FreeImage_GetFIFFromFilenameU(filename);
#else
    fif = FreeImage_GetFIFFromFilename(filename_s);
#endif
  if (fif == FIF_UNKNOWN)
    return 0;

  //check that the plugin has reading capabilities and load the file
  //
  if (FreeImage_FIFSupportsReading(fif))
#if defined WIN32
    dib = FreeImage_LoadU(fif, filename);
#else
    dib = FreeImage_Load(fif, filename_s);
#endif
  else
    return 0;

  bool invertY = false; //(fif != FIF_BMP);

  if (!dib)
    return 0;

  int bitsPerPixel = FreeImage_GetBPP(dib);

  int bytesPerPixel = 4;
  if (bitsPerPixel <= 32) // bits per pixel
  {
    converted = FreeImage_ConvertTo32Bits(dib);
    bytesPerPixel = 4;
  }
  else
  {
    converted = FreeImage_ConvertToRGBF(dib);
    bytesPerPixel = 16;
  }

  return bytesPerPixel;
}

bool HRUtils_GetImageDataFromFreeImageObject(FIBITMAP* converted, char* data)
{
  auto bits         = FreeImage_GetBits(converted);
  auto width        = FreeImage_GetWidth(converted);
  auto height       = FreeImage_GetHeight(converted);
  auto bitsPerPixel = FreeImage_GetBPP(converted);

  if ((bits == 0) || (width == 0) || (height == 0))
    return false;

  if (bitsPerPixel <= 32)
  {
    // (2.1) if ldr -> create bitmap2DLDR
    //
    for (unsigned int y = 0; y<height; y++)
    {
      int lineOffset1 = y*width;
      int lineOffset2 = y*width;
      //if (invertY)
      //lineOffset2 = (height - y - 1)*width;

      for (unsigned int x = 0; x<width; x++)
      {
        int offset1 = lineOffset1 + x;
        int offset2 = lineOffset2 + x;

        data[4 * offset1 + 0] = bits[4 * offset2 + 2];
        data[4 * offset1 + 1] = bits[4 * offset2 + 1];
        data[4 * offset1 + 2] = bits[4 * offset2 + 0];
        data[4 * offset1 + 3] = bits[4 * offset2 + 3];
      }
    }

  }
  else
  {
    // (2.2) if hdr -> create bitmap2DHDR
    //
    float* fbits = (float*)bits;
    float* fdata = (float*)data;

    for (unsigned int i = 0; i < width*height; i++)
    {
      fdata[4 * i + 0] = fbits[3 * i + 0];
      fdata[4 * i + 1] = fbits[3 * i + 1];
      fdata[4 * i + 2] = fbits[3 * i + 2];
      fdata[4 * i + 3] = 0.0f;
    }

  }

  return true;
}

std::shared_ptr<IHRTextureNode> HydraFactoryCommon::CreateTexture2DFromFile(HRTextureNode* pSysObj, const std::wstring& a_fileName)
{
  // (1) load image from file
  //
  const wchar_t* filename = a_fileName.c_str();

  FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
  FIBITMAP *dib(NULL), *converted(NULL);
  BYTE* bits(NULL);                    // pointer to the image data
  unsigned int width(0), height(0);    //image width and height
 
  int bytesPerPixel = HRUtils_LoadImageFromFileToPairOfFreeImageObjects(a_fileName.c_str(), dib, converted, &fif);
  int bitsPerPixel  = bytesPerPixel * 8;
  
  if (bytesPerPixel == 0)
  {
    HrError(L"FreeImage failed to load image: ", a_fileName.c_str());
    return nullptr;
  }

  bits   = FreeImage_GetBits(converted);
  width  = FreeImage_GetWidth(converted);
  height = FreeImage_GetHeight(converted);

  if ((bits == 0) || (width == 0) || (height == 0))
  {
    HrError(L"FreeImage failed for undefined reason, file : ", a_fileName.c_str());
    FreeImage_Unload(converted);
    FreeImage_Unload(dib);
    return nullptr;
  }

  // (3.*) check totalByteSizeOfTexture
  //
  size_t totalByteSizeOfTexture = (bitsPerPixel <= 32) ? size_t(width)*size_t(height)*size_t(sizeof(char)*4) : size_t(width)*size_t(height)*size_t(sizeof(float) * 4);
  totalByteSizeOfTexture += size_t(2 * sizeof(unsigned int));

  // now put image data directly to cache ... ?
  //
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////
  size_t chunkId = g_objManager.scnData.m_vbCache.AllocChunk(totalByteSizeOfTexture, pSysObj->id);
  auto& chunk    = g_objManager.scnData.m_vbCache.chunk_at(chunkId);
  chunk.type     = (bytesPerPixel <= 4) ? CHUNK_TYPE_IMAGE4UB : CHUNK_TYPE_IMAGE4F;

  //unsigned char* data = new unsigned char[size];
  char* data = (char*)chunk.GetMemoryNow();
  if (data == nullptr)
  {
    FreeImage_Unload(converted);
    FreeImage_Unload(dib);

    std::shared_ptr<BitmapProxy> p = std::make_shared<BitmapProxy>(width, height, totalByteSizeOfTexture, chunkId); //#TODO: return nullptr here?
    p->fileName        = a_fileName;
    p->m_bytesPerPixel = bytesPerPixel;
    return p;
  }
  ////
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  unsigned int* pW = (unsigned int*)data;
  unsigned int* pH = pW + 1;
  data += 2 * sizeof(unsigned int);

  (*pW) = width;
  (*pH) = height;

  HRUtils_GetImageDataFromFreeImageObject(converted, data);

  FreeImage_Unload(converted);
  FreeImage_Unload(dib);

  std::shared_ptr<BitmapLDRNode> p1 = std::make_shared<BitmapLDRNode>(width, height, totalByteSizeOfTexture, chunkId);
  std::shared_ptr<BitmapHDRNode> p2 = std::make_shared<BitmapHDRNode>(width, height, totalByteSizeOfTexture, chunkId);

  std::shared_ptr<IHRTextureNode> p11 = p1;
  std::shared_ptr<IHRTextureNode> p22 = p2;

  //delete [] data;

  return (bitsPerPixel <= 32) ? p11 : p22;
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
