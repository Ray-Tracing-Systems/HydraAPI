#include <iostream>
#if defined WIN32
#include <windows.h>
#endif
#include <cstring>
#include <fstream>
#include <math.h>

#include "HR_HDRImage.h"
#include "HR_HDRImageTool.h"
#include "HydraObjectManager.h"

#include "FreeImage.h"
#pragma comment(lib, "FreeImage.lib")

bool HR_SaveLDRImageToFile(const wchar_t* a_fileName, int w, int h, int32_t* data);
bool HR_SaveHDRImageToFileHDR(const wchar_t* a_fileName, int w, int h, const float* a_data, const float a_scale = 1.0f);

extern HRObjectManager g_objManager;

std::string  ws2s(const std::wstring& s);
std::wstring s2ws(const std::string& s);

void HR_MyDebugSaveBMP(const wchar_t* fname, const int* pixels, int w, int h);

int HRUtils_LoadImageFromFileToPairOfFreeImageObjects(const wchar_t* a_filename, FIBITMAP*& dib, FIBITMAP*& converted, FREE_IMAGE_FORMAT* pFif);
bool HRUtils_GetImageDataFromFreeImageObject(FIBITMAP* converted, char* data);

namespace HydraRender
{

  static inline float clamp(float u, float a, float b) { return fminf(fmaxf(a, u), b); }
  static inline uint32_t RealColorToUint32(float x, float y, float z, float w)
  {
    float  r = clamp(x*255.0f, 0.0f, 255.0f);
    float  g = clamp(y*255.0f, 0.0f, 255.0f);
    float  b = clamp(z*255.0f, 0.0f, 255.0f);
    float  a = clamp(w*255.0f, 0.0f, 255.0f);

    unsigned char red   = (unsigned char)r;
    unsigned char green = (unsigned char)g;
    unsigned char blue  = (unsigned char)b;
    unsigned char alpha = (unsigned char)a;

    return red | (green << 8) | (blue << 16) | (alpha << 24);
  }

  void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message)
  {
    std::cout << "\n***\n";
    std::cout << message;
    std::cout << "\n***\n";
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void SaveHDRImageToFileHDR(const std::string& a_fileName, int w, int h, const float* a_data)
  {
    struct float3 { float x, y, z; };
    struct float4 { float x, y, z, w; };

    const float4* data = (const float4*)a_data;

    std::vector<float3> tempData(w*h);
    for (int i = 0; i < w*h; i++)
    {
      float4 src = data[i];
      float3 dst;
      dst.x = src.x;
      dst.y = src.y;
      dst.z = src.z;
      tempData[i] = dst;
    }

    FIBITMAP* dib = FreeImage_AllocateT(FIT_RGBF, w, h);

    BYTE* bits = FreeImage_GetBits(dib);
    memcpy(bits, &tempData[0], sizeof(float3)*w*h);

    FreeImage_SetOutputMessage(FreeImageErrorHandler);

    if (!FreeImage_Save(FIF_HDR, dib, a_fileName.c_str()))
      std::cerr << "SaveImageToFile(): FreeImage_Save error " << std::endl;

    FreeImage_Unload(dib);
  }

  void SaveImageToFile(const std::string& a_fileName, int w, int h, unsigned int* data)
  {
    FIBITMAP* dib = FreeImage_Allocate(w, h, 32);

    BYTE* bits = FreeImage_GetBits(dib);
    BYTE* data2 = (BYTE*)data;
    for (int i = 0; i < w*h; i++)
    {
      bits[4 * i + 0] = data2[4 * i + 2];
      bits[4 * i + 1] = data2[4 * i + 1];
      bits[4 * i + 2] = data2[4 * i + 0];
      bits[4 * i + 3] = data2[4 * i + 3];
    }

    if (!FreeImage_Save(FIF_PNG, dib, a_fileName.c_str()))
      std::cerr << "SaveImageToFile(): FreeImage_Save error on " << a_fileName.c_str() << std::endl;

    FreeImage_Unload(dib);
  }


  void SaveImageToFile(const std::string& a_fileName, const HDRImage4f& image, const float a_gamma)
  {
    std::vector<unsigned int> ldrImageData(image.width()*image.height());

    struct float4 { float x, y, z, w; };
    const float4* in_buff = (const float4*)image.data();

    const float invGamma = 1.0f / a_gamma;

    for (int i = 0; i < image.width()*image.height(); i++)
    {
      float4 data = in_buff[i];
      data.x = powf(data.x, invGamma);
      data.y = powf(data.y, invGamma);
      data.z = powf(data.z, invGamma);
      data.w = 1.0f;
      ldrImageData[i] = RealColorToUint32(data.x, data.y, data.z, data.w);
    }

    SaveImageToFile(a_fileName, image.width(), image.height(), &ldrImageData[0]);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void SaveHDRImageToFileHDR(const std::wstring& a_fileName, int w, int h, const float* a_data)
  {
    HR_SaveHDRImageToFileHDR(a_fileName.c_str(), w, h, a_data, 1.0f);
  }

  void SaveImageToFile(const std::wstring& a_fileName, int w, int h, const unsigned int* data)
  {
    HR_SaveLDRImageToFile(a_fileName.c_str(), w, h, (int32_t*)data);
  }


  void SaveImageToFile(const std::wstring& a_fileName, const HDRImage4f& image, const float a_gamma)
  {
    std::vector<unsigned int> ldrImageData(image.width()*image.height());

    struct float4 { float x, y, z, w; };
    const float4* in_buff = (const float4*)image.data();

    const float invGamma = 1.0f / a_gamma;

    for (int i = 0; i < image.width()*image.height(); i++)
    {
      float4 data = in_buff[i];
      data.x      = powf(data.x, invGamma);
      data.y      = powf(data.y, invGamma);
      data.z      = powf(data.z, invGamma);
      data.w      = 1.0f;
      ldrImageData[i] = RealColorToUint32(data.x, data.y, data.z, data.w);
    }

    SaveImageToFile(a_fileName, image.width(), image.height(), &ldrImageData[0]);
  }


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void LoadImageFromFile(const std::string& a_fileName, std::vector<float>& data, int& w, int& h) // loads both LDR and HDR images(!)
  {
    const char* filename = a_fileName.c_str();

    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN; // image format
    FIBITMAP *dib(NULL), *converted(NULL);
    BYTE* bits(NULL);                    // pointer to the image data
    unsigned int width(0), height(0);    //image width and height


    //check the file signature and deduce its format
    //if still unknown, try to guess the file format from the file extension
    //
    fif = FreeImage_GetFileType(filename, 0);
    if (fif == FIF_UNKNOWN)
      fif = FreeImage_GetFIFFromFilename(filename);

    if (fif == FIF_UNKNOWN)
    {
      std::cerr << "FreeImage failed to guess file image format: " << a_fileName.c_str() << std::endl;
      return;
    }

    //check that the plugin has reading capabilities and load the file
    //
    if (FreeImage_FIFSupportsReading(fif))
      dib = FreeImage_Load(fif, filename);
    else
    {
      std::cerr << "FreeImage does not support file image format: " << a_fileName.c_str() << std::endl;
      return;
    }

    bool invertY = false; //(fif != FIF_BMP);

    if (!dib)
    {
      std::cerr << "FreeImage failed to load image: " << a_fileName.c_str() << std::endl;
      return;
    }

    unsigned int bitsPerPixel = FreeImage_GetBPP(dib);

    int bytesPerPixel = 4;

    converted = FreeImage_ConvertToRGBF(dib);
    bytesPerPixel = 16;

    bits   = FreeImage_GetBits(converted);
    width  = FreeImage_GetWidth(converted);
    height = FreeImage_GetHeight(converted);

    const float* fbits = (const float*)bits;

    data.resize(width*height * 4);

    for (unsigned int i = 0; i < width*height; i++)
    {
      data[4 * i + 0] = fbits[3 * i + 0];
      data[4 * i + 1] = fbits[3 * i + 1];
      data[4 * i + 2] = fbits[3 * i + 2];
      data[4 * i + 3] = 0.0f;
    }

    w = width;
    h = height;

    FreeImage_Unload(dib);
    FreeImage_Unload(converted);

  }


  void LoadImageFromFile(const std::string& a_fileName, HDRImage4f& image)
  {
    std::vector<float> data;
    int w = 0, h = 0;

    LoadImageFromFile(a_fileName, data, w, h);
    image = HDRImage4f(w, h, &data[0]);
  }

  
  void LoadImageFromFile(const std::wstring& a_fileName, std::vector<float>& data, int& w, int& h) // loads both LDR and HDR images(!) 
  {
    const wchar_t* filename = a_fileName.c_str();

    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN; // image format
    FIBITMAP *dib(NULL), *converted(NULL);
    BYTE* bits(NULL);                    // pointer to the image data
    unsigned int width(0), height(0);    //image width and height


    //check the file signature and deduce its format
    //if still unknown, try to guess the file format from the file extension
    //
    #if defined WIN32
    fif = FreeImage_GetFileTypeU(filename, 0);
    #else
    char filename_s[256];
    wcstombs(filename_s, filename, sizeof(filename_s));
    fif = FreeImage_GetFileType(filename_s, 0);
    #endif
    
    if (fif == FIF_UNKNOWN)
    {
    #if defined WIN32
      fif = FreeImage_GetFIFFromFilenameU(filename);
    #else
      fif = FreeImage_GetFIFFromFilename(filename_s);
    #endif
    }
    
    if (fif == FIF_UNKNOWN)
    {
      std::cerr << "FreeImage failed to guess file image format: " << a_fileName.c_str() << std::endl;
      return;
    }

    //check that the plugin has reading capabilities and load the file
    //
    if (FreeImage_FIFSupportsReading(fif))
    {
    #if defined WIN32
      dib = FreeImage_LoadU(fif, filename);
    #else
      dib = FreeImage_Load(fif, filename_s);
    #endif
    }
    else
    {
      std::cerr << "FreeImage does not support file image format: " << a_fileName.c_str() << std::endl;
      return;
    }

    bool invertY = false; //(fif != FIF_BMP);

    if (!dib)
    {
      std::cerr << "FreeImage failed to load image: " << a_fileName.c_str() << std::endl;
      return;
    }

    unsigned int bitsPerPixel = FreeImage_GetBPP(dib);

    int bytesPerPixel = 4;

    converted     = FreeImage_ConvertToRGBF(dib);
    bytesPerPixel = 16;

    bits   = FreeImage_GetBits(converted);
    width  = FreeImage_GetWidth(converted);
    height = FreeImage_GetHeight(converted);

    const float* fbits = (const float*)bits;

    data.resize(width*height * 4);

    for (unsigned int i = 0; i < width*height; i++)
    {
      data[4 * i + 0] = fbits[3 * i + 0];
      data[4 * i + 1] = fbits[3 * i + 1];
      data[4 * i + 2] = fbits[3 * i + 2];
      data[4 * i + 3] = 0.0f;
    }

    w = width;
    h = height;


    FreeImage_Unload(dib);
    FreeImage_Unload(converted);
  }


  void LoadImageFromFile(const std::wstring& a_fileName, HDRImage4f& image)
  {
    std::vector<float> data;
    int w = 0, h = 0;

    LoadImageFromFile(a_fileName, data, w, h);
    image = HDRImage4f(w, h, &data[0]);
  }


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  class InternalImageTool : public IHRImageTool
  {
  public:

    InternalImageTool() {}

    bool LoadImageFromFile(const wchar_t* a_fileName,
                           int& w, int& h, int& bpp, std::vector<int>& a_data) override;

    void SaveHDRImageToFileHDR(const wchar_t* a_fileName, int w, int h, const float* a_data) override;
    void SaveLDRImageToFileLDR(const wchar_t* a_fileName, int w, int h, const int*   a_data) override;
  };


  class FreeImageTool : public IHRImageTool
  {
  public:

    FreeImageTool() : m_pInternal(std::make_unique<InternalImageTool>()) {}

    bool LoadImageFromFile(const wchar_t* a_fileName, 
                           int& w, int& h, int& bpp, std::vector<int>& a_data) override; 
    
    void SaveHDRImageToFileHDR(const wchar_t* a_fileName, int w, int h, const float* a_data) override;
    void SaveLDRImageToFileLDR(const wchar_t* a_fileName, int w, int h, const int*   a_data) override;

  private:

    std::unique_ptr<InternalImageTool> m_pInternal;

  };


  std::wstring CutFileExt(const std::wstring fileName)
  {
    auto pos = fileName.find_last_of(L".");
    if (pos == std::wstring::npos)
    {
      HrPrint(HR_SEVERITY_ERROR, "CutFileExt, can not guess file extension");
      return false;
    }
    return fileName.substr(pos, fileName.size());
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  bool InternalImageTool::LoadImageFromFile(const wchar_t* a_fileName,
                                            int& w, int& h, int& bpp, std::vector<int>& a_data)
  {
    const std::wstring fileExt = CutFileExt(a_fileName);  

    #ifdef WIN32
    std::ifstream fin(a_fileName, std::ios::binary);
    #else
    std::string   s2 = ws2s(fileName);
    std::ifstream fin(s2.c_str(), std::ios::binary);
    #endif

    if (!fin.is_open())
      return false;

    int wh[2] = {0,0};
    fin.read((char*)wh, sizeof(int) * 2);
    w = wh[0];
    h = wh[1];

    if (fileExt == L".image4f")
    {
      bpp = 16;
      a_data.resize(w*h*4);
      fin.read((char*)a_data.data(), a_data.size() * sizeof(int));
      fin.close();
      return true;
    }
    else if (fileExt == L".image1i" || fileExt == L".image1ui" || fileExt == L".image4b" || fileExt == L".image4ub")
    {
      bpp = 4;
      a_data.resize(w*h);
      fin.read((char*)a_data.data(), a_data.size() * sizeof(int));
      fin.close();
      return true;
    } 
    
    //#TODO: add ppm and bmp loaders here ... 

    std::string tempExt = ws2s(fileExt);
    HrPrint(HR_SEVERITY_ERROR, "InternalImageTool::LoadImageFromFile, unsupported file extension ", tempExt.c_str());
    return false;
  }

  void InternalImageTool::SaveHDRImageToFileHDR(const wchar_t* a_fileName, int w, int h, const float* a_data)
  {
    #ifdef WIN32
    std::ofstream fout(a_fileName, std::ios::binary);
    #else
    std::string   s2 = ws2s(fileName);
    std::ofstream fout(s2.c_str(), std::ios::binary);
    #endif
    int wh[2] = { w,h };
    fout.write((const char*)wh,     sizeof(int) * 2);
    fout.write((const char*)a_data, sizeof(float) * size_t(4*w*h));
    fout.close();
  }

  void InternalImageTool::SaveLDRImageToFileLDR(const wchar_t* a_fileName, int w, int h, const int* a_data)
  {
    const std::wstring fileExt = CutFileExt(a_fileName);

    if (fileExt == L".bmp" || fileExt == L".BMP")
    {
      HR_MyDebugSaveBMP(a_fileName, a_data, w, h);
      return;
    }

    //#TODO: implement ppm writer here ... 

    #ifdef WIN32
    std::ofstream fout(a_fileName, std::ios::binary);
    #else
    std::string   s2 = ws2s(fileName);
    std::ofstream fout(s2.c_str(), std::ios::binary);
    #endif
    int wh[2] = { w,h };
    fout.write((const char*)wh,     sizeof(int) * 2);
    fout.write((const char*)a_data, sizeof(float) * size_t(w*h));
    fout.close();
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  bool FreeImageTool::LoadImageFromFile(const wchar_t* a_fileName, 
                                        int& w, int& h, int& bpp, std::vector<int>& a_data)
  {

    /*
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
    
    */

    return false;
  }


};
