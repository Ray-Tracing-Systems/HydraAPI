#include <iostream>
#if defined WIN32
#include <windows.h>
#endif
#include <cstring>
#include <math.h>

#include "HR_HDRImage.h"
#include "HR_HDRImageTool.h"

#include "FreeImage.h"
#pragma comment(lib, "FreeImage.lib")

bool HR_SaveLDRImageToFile(const wchar_t* a_fileName, int w, int h, int32_t* data);
bool HR_SaveHDRImageToFileHDR(const wchar_t* a_fileName, int w, int h, const float* a_data, const float a_scale = 1.0f);

namespace HydraRender
{

  static inline float clamp(float u, float a, float b) { return fminf(fmaxf(a, u), b); }
  static inline uint32_t RealColorToUint32(float x, float y, float z, float w)
  {
    float  r = clamp(x*255.0f, 0.0f, 255.0f);
    float  g = clamp(y*255.0f, 0.0f, 255.0f);
    float  b = clamp(z*255.0f, 0.0f, 255.0f);
    float  a = clamp(w*255.0f, 0.0f, 255.0f);

    unsigned char red = (unsigned char)r;
    unsigned char green = (unsigned char)g;
    unsigned char blue = (unsigned char)b;
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

};
