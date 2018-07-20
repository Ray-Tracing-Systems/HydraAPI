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

extern HRObjectManager g_objManager;

std::string  ws2s(const std::wstring& s);
std::wstring s2ws(const std::string& s);

void HR_MyDebugSaveBMP(const wchar_t* fname, const int* pixels, int w, int h);

static int HRUtils_LoadImageFromFileToPairOfFreeImageObjects(const wchar_t* filename, FIBITMAP*& dib, FIBITMAP*& converted, FREE_IMAGE_FORMAT* pFif)
{
  FREE_IMAGE_FORMAT& fif = (*pFif); // image format

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

static bool HRUtils_GetImageDataFromFreeImageObject(FIBITMAP* converted, char* data)
{
  auto bits = FreeImage_GetBits(converted);
  auto width = FreeImage_GetWidth(converted);
  auto height = FreeImage_GetHeight(converted);
  auto bitsPerPixel = FreeImage_GetBPP(converted);

  if (bits == nullptr || width == 0 || height == 0)
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void FreeImageErrorHandlerHydraInternal(FREE_IMAGE_FORMAT fif, const char *message)
{
  std::wstringstream strOut;
  strOut << L"(FIF = " << fif << L")";
  const std::wstring wstr = std::wstring(L"[FreeImage ") + strOut.str() + std::wstring(L"]: ") + s2ws(message);
  HrError(wstr);
}


static inline float clamp(float u, float a, float b) { float r = fmax(a, u); return fmin(r, b); }

bool HR_SaveHDRImageToFileHDR_WithFreeImage(const wchar_t* a_fileName, int w, int h, const float* a_data, const float a_scale = 1.0f)
{
  struct float3 { float x, y, z; };
  struct float4 { float x, y, z, w; };

  const float4* data = (const float4*)a_data;

  std::vector<float3> tempData(w*h);
  for (int i = 0; i < w*h; i++)
  {
    float4 src = data[i];
    float3 dst = {src.x*a_scale,
                  src.y*a_scale,
                  src.z*a_scale};
    tempData[i] = dst;
  }

  FIBITMAP* dib = FreeImage_AllocateT(FIT_RGBF, w, h);

  BYTE* bits = FreeImage_GetBits(dib);

  memcpy(bits, &tempData[0], sizeof(float3)*w*h);

  FreeImage_SetOutputMessage(FreeImageErrorHandlerHydraInternal);

  #if defined WIN32
  if (!FreeImage_SaveU(FIF_HDR, dib, a_fileName))
  #else
  char filename_s[256];
  wcstombs(filename_s, a_fileName, sizeof(filename_s));
  if (!FreeImage_Save(FIF_HDR, dib, filename_s))
  #endif
  {
    FreeImage_Unload(dib);
    HrError(L"SaveImageToFile(): FreeImage_Save error: ", a_fileName);
    return false;
  }

  FreeImage_Unload(dib);

  return true;
}

void HR_MyDebugSaveBMP(const wchar_t* fname, const int* pixels, int w, int h)
{
  unsigned char file[14] = {
    'B','M',      // magic
    0,0,0,0,      // size in bytes
    0,0,          // app data
    0,0,          // app data
    40 + 14,0,0,0 // start of data offset
  };
  unsigned char info[40] = {
    40,0,0,0,      // info hd size
    0,0,0,0,       // width
    0,0,0,0,       // heigth
    1,0,           // number color planes
    24,0,          // bits per pixel
    0,0,0,0,       // compression is none
    0,0,0,0,       // image bits size
    0x13,0x0B,0,0, // horz resoluition in pixel / m
    0x13,0x0B,0,0, // vert resolutions (0x03C3 = 96 dpi, 0x0B13 = 72 dpi)
    0,0,0,0,       // #colors in pallete
    0,0,0,0,       // #important colors
  };

  int padSize = (4 - (w * 3) % 4) % 4;
  int sizeData = w * h * 3 + h * padSize;
  int sizeAll = sizeData + sizeof(file) + sizeof(info);

  file[2] = (unsigned char)(sizeAll);
  file[3] = (unsigned char)(sizeAll >> 8);
  file[4] = (unsigned char)(sizeAll >> 16);
  file[5] = (unsigned char)(sizeAll >> 24);

  info[4] = (unsigned char)(w);
  info[5] = (unsigned char)(w >> 8);
  info[6] = (unsigned char)(w >> 16);
  info[7] = (unsigned char)(w >> 24);

  info[8] = (unsigned char)(h);
  info[9] = (unsigned char)(h >> 8);
  info[10] = (unsigned char)(h >> 16);
  info[11] = (unsigned char)(h >> 24);

  info[20] = (unsigned char)(sizeData);
  info[21] = (unsigned char)(sizeData >> 8);
  info[22] = (unsigned char)(sizeData >> 16);
  info[23] = (unsigned char)(sizeData >> 24);

  #ifdef WIN32
  std::ofstream stream(fname, std::ios::out | std::ios::binary);
  #else
  std::wstring fnameW(fname);
  std::string  fnameA(fnameW.begin(), fnameW.end());
  std::ofstream stream(fnameA.c_str(), std::ios::out | std::ios::binary);
  #endif

  stream.write((char*)file, sizeof(file));
  stream.write((char*)info, sizeof(info));

  unsigned char pad[3] = { 0,0,0 };

  std::vector<unsigned char> line(3 * w);

  for (int y = 0; y<h; y++)
  {
    for (int x = 0; x<w; x++)
    {
      const int pxData = pixels[y*w + x];

      line[x * 3 + 0] = (pxData & 0x00FF0000) >> 16;
      line[x * 3 + 1] = (pxData & 0x0000FF00) >> 8;
      line[x * 3 + 2] = (pxData & 0x000000FF);
    }

    stream.write((char*)&line[0], line.size());
    stream.write((char*)pad, padSize);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring CutFileExt(const std::wstring& fileName)
{
  auto pos = fileName.find_last_of(L".");
  if (pos == std::wstring::npos)
  {
    HrPrint(HR_SEVERITY_ERROR, "CutFileExt, can not guess file extension");
    return L"";
  }
  return fileName.substr(pos, fileName.size());
}

bool InternalImageTool::LoadImageFromFile(const wchar_t* a_fileName,
                                          int& w, int& h, int& bpp, std::vector<int>& a_data)
{
  const std::wstring fileExt = CutFileExt(a_fileName);

#ifdef WIN32
  std::ifstream fin(a_fileName, std::ios::binary);
#else
  std::string   s2 = ws2s(a_fileName);
  std::ifstream fin(s2.c_str(), std::ios::binary);
#endif

  if (!fin.is_open())
    return false;

  int wh[2] = { 0,0 };
  fin.read((char*)wh, sizeof(int) * 2);
  w = wh[0];
  h = wh[1];

  if (fileExt == L".image4f")
  {
    bpp = 16;
    a_data.resize(w*h * 4);
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

  HrPrint(HR_SEVERITY_ERROR, L"InternalImageTool::LoadImageFromFile, unsupported file extension ", fileExt.c_str());
  return false;
}

bool InternalImageTool::LoadImageFromFile(const wchar_t* a_fileName,
                                          int& w, int& h, std::vector<float>& a_data) //#TODO: the implementation works only for ".image4f" ... this is not strictly correct. 
{
  std::vector<int> tempData;
  int bpp = 0;
  LoadImageFromFile(a_fileName,
                    w, h, bpp, tempData);

  a_data.resize(tempData.size());
  memcpy(a_data.data(), tempData.data(), tempData.size() * sizeof(int));
  return true;
}

void InternalImageTool::SaveHDRImageToFileHDR(const wchar_t* a_fileName, int w, int h, const float* a_data)
{
#ifdef WIN32
  std::ofstream fout(a_fileName, std::ios::binary);
#else
  std::string   s2 = ws2s(a_fileName);
  std::ofstream fout(s2.c_str(), std::ios::binary);
#endif
  int wh[2] = { w,h };
  fout.write((const char*)wh, sizeof(int) * 2);
  fout.write((const char*)a_data, sizeof(float) * size_t(4 * w*h));
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
  std::string   s2 = ws2s(a_fileName);
  std::ofstream fout(s2.c_str(), std::ios::binary);
#endif
  int wh[2] = { w,h };
  fout.write((const char*)wh, sizeof(int) * 2);
  fout.write((const char*)a_data, sizeof(float) * size_t(w*h));
  fout.close();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
    const std::wstring fileNameW = s2ws(a_fileName);
    g_objManager.m_pImgTool->SaveHDRImageToFileHDR(fileNameW.c_str(), w, h, a_data);
  }

  void SaveHDRImageToFileHDR(const std::wstring& a_fileName, int w, int h, const float* a_data)
  {
    g_objManager.m_pImgTool->SaveHDRImageToFileHDR(a_fileName.c_str(), w, h, a_data);
  }

  void SaveImageToFile(const std::string& a_fileName, int w, int h, unsigned int* data)
  {
    const std::wstring fileNameW = s2ws(a_fileName);
    g_objManager.m_pImgTool->SaveLDRImageToFileLDR(fileNameW.c_str(), w, h, (const int*)data);
  }

  void SaveImageToFile(const std::wstring& a_fileName, int w, int h, const unsigned int* data)
  {
    g_objManager.m_pImgTool->SaveLDRImageToFileLDR(a_fileName.c_str(), w, h, (const int*)data);
  }

  void SaveImageToFile(const std::wstring& a_fileName, const HDRImage4f& image, float a_gamma)
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

  void SaveImageToFile(const std::string& a_fileName, const HDRImage4f& image, float a_gamma)
  {
    const std::wstring fileNameW = s2ws(a_fileName);
    SaveImageToFile(fileNameW, image, a_gamma);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /**
  \brief force convert loaded image to HDR (i.e. float4, 16 bytes per pixel)
  */
  void LoadImageFromFile(const std::wstring& a_fileName, std::vector<float>& data, int& w, int& h) // loads both LDR and HDR images(!) 
  {
    g_objManager.m_pImgTool->LoadImageFromFile(a_fileName.c_str(), 
                                               w, h, data);
  }

  /**
  \brief force convert loaded image to HDR (i.e. float4, 16 bytes per pixel)
  */
  void LoadImageFromFile(const std::string& a_fileName, std::vector<float>& data, int& w, int& h) // loads both LDR and HDR images(!)
  {
    const std::wstring fileNameW = s2ws(a_fileName);
    LoadImageFromFile(fileNameW.c_str(), data, w, h);
  }

  /**
  \brief force convert loaded image to HDR (i.e. float4, 16 bytes per pixel)
  */
  void LoadImageFromFile(const std::wstring& a_fileName, HDRImage4f& image)
  {
    std::vector<float> data;
    int w = 0, h = 0;
    LoadImageFromFile(a_fileName, data, w, h);
    image = HDRImage4f(w, h, &data[0]);
  }

  /**
  \brief force convert loaded image to HDR (i.e. float4, 16 bytes per pixel)
  */
  void LoadImageFromFile(const std::string& a_fileName, HDRImage4f& image)
  {
    std::vector<float> data;
    int w = 0, h = 0;
    LoadImageFromFile(a_fileName, data, w, h);
    image = HDRImage4f(w, h, &data[0]);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  class FreeImageTool : public IHRImageTool
  {
  public:

    FreeImageTool() : m_pInternal(std::make_unique<InternalImageTool>()) {}

    bool LoadImageFromFile(const wchar_t* a_fileName, 
                           int& w, int& h, int& bpp, std::vector<int>& a_data) override; 
    
    bool LoadImageFromFile(const wchar_t* a_fileName, 
                           int& w, int& h, std::vector<float>& a_data) override;

    void SaveHDRImageToFileHDR(const wchar_t* a_fileName, int w, int h, const float* a_data) override;
    void SaveLDRImageToFileLDR(const wchar_t* a_fileName, int w, int h, const int*   a_data) override;

  private:

    std::unique_ptr<InternalImageTool> m_pInternal;

  };
  
  class GentooFix_SaveBMP : public FreeImageTool
  {
  public:
    void SaveLDRImageToFileLDR(const wchar_t* a_fileName, int w, int h, const int*   a_data) override
    {
      auto fileExt = CutFileExt(a_fileName);
      if(fileExt == L".bmp" || fileExt == L".BMP")
        HR_MyDebugSaveBMP(a_fileName, a_data, w, h);
      else
        FreeImageTool::SaveLDRImageToFileLDR(a_fileName, w, h, a_data);
    }
  };
  
  std::unique_ptr<IHRImageTool> CreateImageTool()
  {
  #ifdef GENTOO_FIX_SAVE_BMP
    return std::move(std::make_unique<GentooFix_SaveBMP>());
  #else
    return std::move(std::make_unique<FreeImageTool>()); // C++ equals shit, std::unique_ptr equals shit
  #endif
  }

 

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  bool FreeImageTool::LoadImageFromFile(const wchar_t* a_fileName, 
                                        int& w, int& h, int& bpp, std::vector<int>& a_data)
  {
    const std::wstring fileExt = CutFileExt(a_fileName);

    if (fileExt == L".image4f" || fileExt == L".image1i" || fileExt == L".image1ui" || fileExt == L".image4b" || fileExt == L".image4ub")
    {
      return m_pInternal->LoadImageFromFile(a_fileName, 
                                            w, h, bpp, a_data);
    }

    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    FIBITMAP *dib(NULL), *converted(NULL);
    
    int bytesPerPixel = HRUtils_LoadImageFromFileToPairOfFreeImageObjects(a_fileName, dib, converted, &fif);
    if (bytesPerPixel == 0)
    {
      HrError(L"FreeImage failed to load image: ", a_fileName);
      return false;
    }

    w   = FreeImage_GetWidth(converted);
    h   = FreeImage_GetHeight(converted);
    bpp = bytesPerPixel;
    
    if (w == 0 || h == 0)
    {
      HrError(L"FreeImage failed for undefined reason, file : ", a_fileName);
      FreeImage_Unload(converted);
      FreeImage_Unload(dib);
      return false;
    }
   
    a_data.resize(w*h*bpp/sizeof(int));
   
    HRUtils_GetImageDataFromFreeImageObject(converted, (char*)a_data.data());
    
    FreeImage_Unload(converted);
    FreeImage_Unload(dib);

    return true;
  }

  bool FreeImageTool::LoadImageFromFile(const wchar_t* a_fileName,
                                        int& w, int& h, std::vector<float>& a_data)
  {
    const wchar_t* filename = a_fileName;

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
      std::cerr << "FreeImage failed to guess file image format: " << filename << std::endl;
      return false;
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
      std::cerr << "FreeImage does not support file image format: " << filename << std::endl;
      return false;
    }

    bool invertY = false; //(fif != FIF_BMP);

    if (!dib)
    {
      std::cerr << "FreeImage failed to load image: " << filename << std::endl;
      return false;
    }

    converted = FreeImage_ConvertToRGBF(dib);


    bits   = FreeImage_GetBits(converted);
    width  = FreeImage_GetWidth(converted);
    height = FreeImage_GetHeight(converted);

    const float* fbits = (const float*)bits;
    a_data.resize(width*height * 4);

    for (unsigned int i = 0; i < width*height; i++)
    {
      a_data[4 * i + 0] = fbits[3 * i + 0];
      a_data[4 * i + 1] = fbits[3 * i + 1];
      a_data[4 * i + 2] = fbits[3 * i + 2];
      a_data[4 * i + 3] = 0.0f;
    }

    w = width;
    h = height;

    FreeImage_Unload(dib);
    FreeImage_Unload(converted);
    return true;
  }

  void FreeImageTool::SaveHDRImageToFileHDR(const wchar_t* a_fileName, int w, int h, const float* a_data)
  {
    const std::wstring fileExt = CutFileExt(a_fileName);

    if (fileExt == L".image4f")
      m_pInternal->SaveHDRImageToFileHDR(a_fileName, w, h, a_data);
    else
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
      BYTE* bits    = FreeImage_GetBits(dib);

      memcpy(bits, &tempData[0], sizeof(float3)*w*h);

      FreeImage_SetOutputMessage(FreeImageErrorHandlerHydraInternal);

      #if defined WIN32
      if (!FreeImage_SaveU(FIF_HDR, dib, a_fileName))
      #else
      char filename_s[256];
      wcstombs(filename_s, a_fileName, sizeof(filename_s));
      if (!FreeImage_Save(FIF_HDR, dib, filename_s))
      #endif
      {
        FreeImage_Unload(dib);
        HrError(L"SaveImageToFile(): FreeImage_Save error: ", a_fileName);
        return;
      }

      FreeImage_Unload(dib);
    }
  }

  void FreeImageTool::SaveLDRImageToFileLDR(const wchar_t* a_fileName, int w, int h, const int* a_data)
  {
    const std::wstring fileExt = CutFileExt(a_fileName);
    if (fileExt == L".image1i" || fileExt == L".image1ui" || fileExt == L".image4b" || fileExt == L".image4ub")
      m_pInternal->SaveLDRImageToFileLDR(a_fileName, w, h, a_data);
    else
    {
      FIBITMAP* dib = FreeImage_Allocate(w, h, 32);
      BYTE* bits    = FreeImage_GetBits(dib);
      //memcpy(bits, data, w*h*sizeof(int32_t));
      const BYTE* data2 = (const BYTE*)a_data;
      for (int i = 0; i<w*h; i++)
      {
        bits[4 * i + 0] = data2[4 * i + 2];
        bits[4 * i + 1] = data2[4 * i + 1];
        bits[4 * i + 2] = data2[4 * i + 0];
        bits[4 * i + 3] = 255; // data2[4 * i + 3]; // 255 to kill alpha channel
      }

      FreeImage_SetOutputMessage(FreeImageErrorHandlerHydraInternal);

      auto imageFileFormat = FIF_PNG;

      std::wstring fileName(a_fileName);
      if (fileName.size() > 4)
      {
        std::wstring resolution = fileName.substr(fileName.size() - 4, 4);

        if (resolution.find(L".bmp") != std::wstring::npos || resolution.find(L".BMP") != std::wstring::npos)
          imageFileFormat = FIF_BMP;
      }
      #if defined WIN32
      if (!FreeImage_SaveU(imageFileFormat, dib, a_fileName))
      #else
      char filename_s[256];
      wcstombs(filename_s, a_fileName, sizeof(filename_s));
      if (!FreeImage_Save(imageFileFormat, dib, filename_s))
      #endif
      {
        FreeImage_Unload(dib);
        HrError(L"SaveImageToFile(): FreeImage_Save error on ", a_fileName);
        return;
      }

      FreeImage_Unload(dib);
    } // else 

  } // end function

};


