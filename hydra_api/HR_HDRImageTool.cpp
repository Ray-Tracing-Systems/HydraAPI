#include <iostream>
#if defined WIN32
#include <windows.h>
#endif
#include <cstring>
#include <fstream>
#include <algorithm>
#include <cmath>

#include "HR_HDRImage.h"
#include "HR_HDRImageTool.h"
#include "HydraObjectManager.h"

#include "FreeImage.h"
//#pragma comment(lib, "FreeImage.lib")

extern HRObjectManager g_objManager;

std::string  ws2s(const std::wstring& s);
std::wstring s2ws(const std::string& s);

void HR_MyDebugSaveBMP(const wchar_t* fname, const int* pixels, int w, int h);

namespace FreeImageFixes
{
    static FIBITMAP* convertToRGBAF(FIBITMAP* pDib)
    {
        const unsigned width = FreeImage_GetWidth(pDib);
        const unsigned height = FreeImage_GetHeight(pDib);

        auto pNew = FreeImage_AllocateT(FIT_RGBAF, width, height);
        FreeImage_CloneMetadata(pNew, pDib);

        const unsigned src_pitch = FreeImage_GetPitch(pDib);
        const unsigned dst_pitch = FreeImage_GetPitch(pNew);

        const BYTE *src_bits = (BYTE*)FreeImage_GetBits(pDib);
        BYTE* dst_bits = (BYTE*)FreeImage_GetBits(pNew);

        for (unsigned y = 0; y < height; y++)
        {
            const FIRGBF *src_pixel = (FIRGBF*)src_bits;
            FIRGBAF* dst_pixel = (FIRGBAF*)dst_bits;

            for (unsigned x = 0; x < width; x++)
            {
                // Convert pixels directly, while adding a "dummy" alpha of 1.0
                dst_pixel[x].red   = src_pixel[x].red;
                dst_pixel[x].green = src_pixel[x].green;
                dst_pixel[x].blue  = src_pixel[x].blue;
                dst_pixel[x].alpha = 1.0F;

            }
            src_bits += src_pitch;
            dst_bits += dst_pitch;
        }
        return pNew;
    }
}

static void HRUtils_LoadImageFromFileToPairOfFreeImageObjects(const wchar_t* filename, FIBITMAP*& dib, FIBITMAP*& converted,
                                                              FREE_IMAGE_FORMAT* pFif, int& bpp, int& chan)
{
  FREE_IMAGE_FORMAT& fif = (*pFif); // image format

                                    //check the file signature and deduce its format
                                    //if still unknown, try to guess the file format from the file extension
                                    //

#if defined WIN32
  fif          = g_objManager.m_FreeImageDll.m_pFreeImage_GetFileTypeU(filename, 0);
#else
  char filename_s[256];
  wcstombs(filename_s, filename, sizeof(filename_s));
  fif          = FreeImage_GetFileType(filename_s, 0);
#endif

  if (fif      == FIF_UNKNOWN)
  {
#if defined WIN32
    fif        = g_objManager.m_FreeImageDll.m_pFreeImage_GetFIFFromFilenameU(filename);
#else
    fif        = FreeImage_GetFIFFromFilename(filename_s);
#endif
  }

  if (fif      == FIF_UNKNOWN)
  {
    bpp        = 0;
    chan       = 0;
    return;
  }

  //check that the plugin has reading capabilities and load the file
  //
  if (g_objManager.m_FreeImageDll.m_pFreeImage_FIFSupportsReading(fif))
  {
#if defined WIN32
    dib        = g_objManager.m_FreeImageDll.m_pFreeImage_LoadU(fif, filename, 0);
#else
    dib        = FreeImage_Load(fif, filename_s);
#endif
  }
  else
  {
    bpp        = 0;
    chan       = 0;
    return;
  }

  bool invertY = false; //(fif != FIF_BMP);

  if (!dib)
  {
    bpp        = 0;
    chan       = 0;
    return;
  }

  auto type           = g_objManager.m_FreeImageDll.m_pFreeImage_GetImageType(dib);
  auto bitsPerPixel   = g_objManager.m_FreeImageDll.m_pFreeImage_GetBPP(dib);

  if(type        == FIT_BITMAP && bitsPerPixel ==  8)
  {
    converted    = g_objManager.m_FreeImageDll.m_pFreeImage_ConvertTo8Bits(dib);
    bpp          = 1;
    chan         = 1;
    converted    = g_objManager.m_FreeImageDll.m_pFreeImage_ConvertTo32Bits(dib);// //FreeImage_ConvertTo32Bits(dib);
    chan         = 4;
    bpp          = chan;
  }
  else if(type   == FIT_FLOAT || type == FIT_UINT16)
  {
    converted    = g_objManager.m_FreeImageDll.m_pFreeImage_ConvertToFloat(dib);
    bpp          = 4;
    chan         = 1;
  }
//  else if(type == FIT_BITMAP && bitsPerPixel == 24)
//  {
//    converted  = FreeImage_ConvertTo24Bits(dib);
//    chan       = 3;
//    bpp        = chan;
//  }
  else if(type == FIT_BITMAP)
  {
    converted    = g_objManager.m_FreeImageDll.m_pFreeImage_ConvertTo32Bits(dib);
    chan         = 4;
    bpp          = chan;
  }
  else if(type   == FIT_RGBF || type == FIT_RGBAF) 
  {
    //converted = FreeImage_ConvertToRGBAF(dib);
    converted = FreeImageFixes::convertToRGBAF(dib);
    chan = 4;
    bpp = sizeof(float) * chan;
  }
  //else if(type == FIT_RGBAF)
  //{
  //  converted  = FreeImage_ConvertToRGBAF(dib);
  //  chan       = 4;
  //  bpp        = sizeof(float) * chan;
  //}
}


static bool HRUtils_GetImageDataFromFreeImageObject(FIBITMAP* converted, int chan, char* data)
{
  auto bits         = g_objManager.m_FreeImageDll.m_pFreeImage_GetBits(converted);
  auto width        = g_objManager.m_FreeImageDll.m_pFreeImage_GetWidth(converted);
  auto height       = g_objManager.m_FreeImageDll.m_pFreeImage_GetHeight(converted);
  auto bitsPerPixel = g_objManager.m_FreeImageDll.m_pFreeImage_GetBPP(converted);
  auto type         = g_objManager.m_FreeImageDll.m_pFreeImage_GetImageType(converted);

  if (bits == nullptr || width == 0 || height == 0)
    return false;

  if(type == FIT_FLOAT) 
  {
    auto fbits = (float*)bits;
    auto fdata = (float*)data;

    #pragma omp parallel for
    for (int i = 0; i < width*height; ++i)
    {
      for(int j = 0; j < chan; ++j)
        fdata[chan * i + j] = fbits[chan * i + j];
    }
  }
  else if (type == FIT_RGBF || type == FIT_RGBAF)
  {
    auto fbits = (float*)bits;
    auto fdata = (float*)data;

    #pragma omp parallel for
    for (int i = 0; i < width * height; ++i)
    {
      const int channel3 = 3 * i;
      const int channel4 = 4 * i;

      fdata[channel4 + 0] = fbits[channel3 + 0];
      fdata[channel4 + 1] = fbits[channel3 + 1];
      fdata[channel4 + 2] = fbits[channel3 + 2];
      fdata[channel4 + 3] = 1.0F;
    }
  }
  else if (type == FIT_BITMAP)
  {
    #pragma omp parallel for
    for (int i = 0; i < width*height; ++i)
    {
      for(int j = 0; j < chan; ++j)
        data[chan * i + j] = bits[chan * i + j];

      if(chan >= 3) // swap red and blue because freeimage
        std::swap(data[chan * i], data[chan * i + 2]);
    }

//    for (unsigned int y = 0; y<height; y++)
//    {
//      int lineOffset1 = y*width;
//      int lineOffset2 = y*width;
//      //if (invertY)
//      //lineOffset2 = (height - y - 1)*width;
//
//      for (unsigned int x = 0; x<width; x++)
//      {
//        int offset1 = lineOffset1 + x;
//        int offset2 = lineOffset2 + x;
//
//        data[4 * offset1 + 0] = bits[4 * offset2 + 2];
//        data[4 * offset1 + 1] = bits[4 * offset2 + 1];
//        data[4 * offset1 + 2] = bits[4 * offset2 + 0];
//        data[4 * offset1 + 3] = bits[4 * offset2 + 3];
//      }
//    }
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
  auto pos = fileName.find_last_of(L'.');
  if (pos == std::wstring::npos)
  {
    HrPrint(HR_SEVERITY_ERROR, "CutFileExt, can not guess file extension");
    return L"";
  }
  return fileName.substr(pos, fileName.size());
}

std::wstring CutFileName(const std::wstring& fileName)
{
  auto pos = fileName.find_last_of(L'/');

  if (pos == std::wstring::npos)
    pos = fileName.find_last_of(L'\\');

  if (pos == std::wstring::npos)
  {
    HrPrint(HR_SEVERITY_ERROR, "CutFileExt, can not guess file extension");
    return L"";
  }
  return fileName.substr(pos+1, fileName.size());
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
  {
    HrPrint(HR_SEVERITY_ERROR, "LoadImageFromFile, can not open file");
    return false;
  }

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

  HrPrint(HR_SEVERITY_ERROR, L"InternalImageTool::LoadImageFromFile, unsupported file extension ", fileExt.c_str());
  return false;
}

bool InternalImageTool::LoadImageFromFile(const wchar_t* a_fileName, int& w, int& h, int& bpp, int& chan,
                                          std::vector<unsigned char>& a_data)
{
  const std::wstring fileExt = CutFileExt(a_fileName);

#ifdef WIN32
  std::ifstream fin(a_fileName, std::ios::binary);
#else
  std::string   s2 = ws2s(a_fileName);
  std::ifstream fin(s2.c_str(), std::ios::binary);
#endif

  if (!fin.is_open())
  {
    HrPrint(HR_SEVERITY_ERROR, "LoadImageFromFile, can not open file");
    return false;
  }

  int wh[2] = { 0,0 };
  fin.read((char*)wh, sizeof(int) * 2);
  w = wh[0];
  h = wh[1];

  if (fileExt == L".image4f")
  {
    chan = 4;
    bpp = sizeof(float) * chan;
    a_data.resize(w * h * chan * sizeof(float));
    fin.read((char*)a_data.data(), a_data.size());
    fin.close();
    return true;
  }
  else if (fileExt == L".image4b" || fileExt == L".image4ub")
  {
    chan = 4;
    bpp  = chan;
    a_data.resize(w * h * chan);
    fin.read((char*)a_data.data(), a_data.size());
    fin.close();
    return true;
  }
  else if (fileExt == L".image1b")
  {
    chan = 1;
    bpp  = 1;
    a_data.resize(w * h);
    fin.read((char*)a_data.data(), a_data.size());
    fin.close();
    return true;
  }
  else if (fileExt == L".image1f")
  {
    chan = 1;
    bpp  = sizeof(float);
    a_data.resize(w * h * bpp);
    fin.read((char*)a_data.data(), a_data.size());
    fin.close();
    return true;
  }

  HrPrint(HR_SEVERITY_ERROR, L"InternalImageTool::LoadImageFromFile, unsupported file extension ", fileExt.c_str());
  return false;
}

bool InternalImageTool::LoadImageFromFile(const wchar_t* a_fileName, int& w, int& h, int& chan, std::vector<float>& a_data)
{
  std::vector<unsigned char> tempData;
  int bpp = 0;
  LoadImageFromFile(a_fileName,w, h, bpp, chan, tempData);

  a_data.resize(w * h * chan);
  memcpy(a_data.data(), tempData.data(), tempData.size() * sizeof(tempData[0]));
  return true;
}

void InternalImageTool::SaveHDRImageToFileHDR(const wchar_t* a_fileName, int w, int h, int chan, const float* a_data)
{
#ifdef WIN32
  std::ofstream fout(a_fileName, std::ios::binary);
#else
  std::string   s2 = ws2s(a_fileName);
  std::ofstream fout(s2.c_str(), std::ios::binary);
#endif
  int wh[2] = { w,h };
  fout.write((const char*)wh, sizeof(int) * 2);
  fout.write((const char*)a_data, sizeof(float) * size_t(chan * w * h));
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

  void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message)
  {
    std::cout << "\n***\n";
    std::cout << message;
    std::cout << "\n***\n";
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void SaveHDRImageToFileHDR(const std::string& a_fileName, int w, int h, int chan, const float* a_data)
  {
    const std::wstring fileNameW = s2ws(a_fileName);
    g_objManager.m_pImgTool->SaveHDRImageToFileHDR(fileNameW.c_str(), w, h, chan, a_data);
  }

  void SaveHDRImageToFileHDR(const std::wstring& a_fileName, int w, int h, int chan, const float* a_data)
  {
    g_objManager.m_pImgTool->SaveHDRImageToFileHDR(a_fileName.c_str(), w, h, chan, a_data);
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

    for (int i = 0; i < image.width()*image.height(); i++)
    {
      float4 data = in_buff[i];
      data.x      = LinearToSRGB(data.x);
      data.y      = LinearToSRGB(data.y);
      data.z      = LinearToSRGB(data.z);
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
  void LoadImageFromFile(const std::wstring& a_fileName, std::vector<float>& data, int& w, int& h, int& chan) // loads both LDR and HDR images(!)
  {
    g_objManager.m_pImgTool->LoadImageFromFile(a_fileName.c_str(), w, h, chan, data);
  }

  /**
  \brief force convert loaded image to HDR (i.e. float4, 16 bytes per pixel)
  */
  void LoadImageFromFile(const std::string& a_fileName, std::vector<float>& data, int& w, int& h, int& chan) // loads both LDR and HDR images(!)
  {
    const std::wstring fileNameW = s2ws(a_fileName);
    LoadImageFromFile(fileNameW, data, w, h, chan);
  }

  /**
  \brief force convert loaded image to HDR (i.e. float4, 16 bytes per pixel)
  */
  void LoadImageFromFile(const std::wstring& a_fileName, HDRImage4f& image)
  {
    std::vector<float> data;
    int w = 0, h = 0;
    int chan;
    LoadImageFromFile(a_fileName, data, w, h, chan);
    image = HDRImage4f(w, h, &data[0]);
  }

  /**
  \brief force convert loaded image to HDR (i.e. float4, 16 bytes per pixel)
  */
  void LoadImageFromFile(const std::string& a_fileName, HDRImage4f& image)
  {
    std::vector<float> data;
    int w = 0, h = 0;
    int chan;
    LoadImageFromFile(a_fileName, data, w, h, chan);
    image = HDRImage4f(w, h, &data[0]);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  /**
  \brief load LDR image from file
  */
  bool LoadLDRImageFromFile(const char* a_fileName, int* pW, int* pH, std::vector<int32_t>& a_data)
  {
    g_objManager.m_FreeImageDll.m_pFreeImage_SetOutputMessage(FreeImageErrorHandlerHydraInternal);

    FREE_IMAGE_FORMAT fif = FIF_PNG; // image format

    fif = g_objManager.m_FreeImageDll.m_pFreeImage_GetFileType(a_fileName, 0);

    if (fif == FIF_UNKNOWN)
      fif = g_objManager.m_FreeImageDll.m_pFreeImage_GetFIFFromFilename(a_fileName);

    FIBITMAP* dib = nullptr;
    if (g_objManager.m_FreeImageDll.m_pFreeImage_FIFSupportsReading(fif))
      dib = g_objManager.m_FreeImageDll.m_pFreeImage_Load(fif, a_fileName, 0);
    else
    {
      std::cout << "LoadLDRImageFromFile() : FreeImage_FIFSupportsReading/FreeImage_Load failed!" << std::endl;
      return false;
    }

    FIBITMAP* converted = g_objManager.m_FreeImageDll.m_pFreeImage_ConvertTo32Bits(dib);
    BYTE* bits          = g_objManager.m_FreeImageDll.m_pFreeImage_GetBits(converted);
    auto width          = g_objManager.m_FreeImageDll.m_pFreeImage_GetWidth(converted);
    auto height         = g_objManager.m_FreeImageDll.m_pFreeImage_GetHeight(converted);
    auto bitsPerPixel   = g_objManager.m_FreeImageDll.m_pFreeImage_GetBPP(converted);


    if (width == 0 || height == 0)
    {
      std::cerr << "Seems that 'FreeImage_ConvertTo32Bits' has failed " << std::endl;
      return false;
    }

    a_data.resize(width*height);
    BYTE* data = (BYTE*)a_data.data();

#pragma omp parallel for
    for (int y = 0; y < height; ++y)
    {
      const int lineOffset1 = y * width;
      const int lineOffset2 = y * width;

      for (int x = 0; x < width; ++x)
      {
        const int offset1 = lineOffset1 + x;
        const int offset2 = lineOffset2 + x;

        data[4 * offset1 + 0] = bits[4 * offset2 + 2];
        data[4 * offset1 + 1] = bits[4 * offset2 + 1];
        data[4 * offset1 + 2] = bits[4 * offset2 + 0];
        data[4 * offset1 + 3] = bits[4 * offset2 + 3];
      }
    }

    g_objManager.m_FreeImageDll.m_pFreeImage_Unload(dib);
    g_objManager.m_FreeImageDll.m_pFreeImage_Unload(converted);

    (*pW) = width;
    (*pH) = height;
    return true;
  }


  bool LoadHDRImageFromFile(const char* a_fileName, int* pW, int* pH, int* pChan, std::vector<float>& a_data)
  {
    g_objManager.m_FreeImageDll.m_pFreeImage_SetOutputMessage(FreeImageErrorHandlerHydraInternal);

    FREE_IMAGE_FORMAT fif = FIF_EXR; // image format

    fif = g_objManager.m_FreeImageDll.m_pFreeImage_GetFileType(a_fileName, 0);

    if (fif == FIF_UNKNOWN)
      fif = g_objManager.m_FreeImageDll.m_pFreeImage_GetFIFFromFilename(a_fileName);

    FIBITMAP* dib = nullptr;

    if (g_objManager.m_FreeImageDll.m_pFreeImage_FIFSupportsReading(fif))
      dib = g_objManager.m_FreeImageDll.m_pFreeImage_Load(fif, a_fileName, 0);
    else
    {
      std::cout << "LoadHDRImageFromFile() : FreeImage_FIFSupportsReading/FreeImage_Load failed!" << std::endl;
      return false;
    }    

    auto imageType    = g_objManager.m_FreeImageDll.m_pFreeImage_GetImageType(dib);
    BYTE* bits        = g_objManager.m_FreeImageDll.m_pFreeImage_GetBits(dib);
    auto width        = g_objManager.m_FreeImageDll.m_pFreeImage_GetWidth(dib);
    auto height       = g_objManager.m_FreeImageDll.m_pFreeImage_GetHeight(dib);
    auto bitsPerPixel = g_objManager.m_FreeImageDll.m_pFreeImage_GetBPP(dib);

    int channels = 0;
    if (imageType == FIT_RGBAF)
      channels = 4;
    else if (imageType == FIT_FLOAT)
      channels = 1;
    else
    {
      std::cout << "LoadHDRImageFromFile() : Unsupported channels number in image (must be 1 or 4)" << std::endl;
      return false;
    }

    a_data.resize(width * height * channels);
    float* fbits = (float*)bits;

#pragma omp parallel for
    for (int i = 0; i < width * height; i++)
    {
      for (int j = 0; j < channels; ++j)
      {
        a_data[channels * i + j] = fbits[channels * i + j];
      }
    }

    g_objManager.m_FreeImageDll.m_pFreeImage_Unload(dib);

    (*pW)    = width;
    (*pH)    = height;
    (*pChan) = channels;
    return true;
  }


  float MSE_RGB_LDR(const std::vector<int32_t>& image1, const std::vector<int32_t>& image2)
  {
    if(image1.size() != image2.size())
      return std::numeric_limits<float>::infinity();

    double accum = 0.0;

    for(size_t i = 0; i < image1.size(); ++i)
    {
      const int pxData1 = image1[i];
      const int pxData2 = image2[i];
      const int r1 = (pxData1 & 0x00FF0000) >> 16;
      const int g1 = (pxData1 & 0x0000FF00) >> 8;
      const int b1 = (pxData1 & 0x000000FF);

      const int r2 = (pxData2 & 0x00FF0000) >> 16;
      const int g2 = (pxData2 & 0x0000FF00) >> 8;
      const int b2 = (pxData2 & 0x000000FF);

      accum += double( (r1-r2)*(r1-r2) + (b1-b2)*(b1-b2) + (g1-g2)*(g1-g2) );
    }

    return float(accum/double(image1.size()));
  }

  float MSE_HDR(const std::vector<float>& image1, const std::vector<float>& image2, const int channels)
  {
    if (image1.size() != image2.size())
      return std::numeric_limits<float>::infinity();

    double accum = 0.0;
    size_t SIZE = image1.size() / channels;

    for (size_t i = 0; i < SIZE; ++i)
    {
    
      for (size_t k = 0; k < channels; ++k)
      {
        const float px1 = image1[i * channels + k];
        const float px2 = image2[i * channels + k];

        accum += double(px1 - px2) * double(px1 - px2);
      }
    }

    return float(accum / double(SIZE));
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
                           int& w, int& h, int& bpp, int &chan, std::vector<unsigned char>& a_data) override;
    
    bool LoadImageFromFile(const wchar_t* a_fileName, 
                           int& w, int& h, int &chan, std::vector<float>& a_data) override;

    void SaveHDRImageToFileHDR(const wchar_t* a_fileName, int w, int h, int chan, const float* a_data) override;
    void SaveLDRImageToFileLDR(const wchar_t* a_fileName, int w, int h, const int*   a_data) override;

    void Save16BitMonoImageTo16BitPNG(const wchar_t* a_fileName, int w, int h, const unsigned short* a_data) override;
    void SaveMonoHDRImageToFileHDR(const wchar_t* a_fileName, int w, int h, const float* a_data) override;

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

  bool FreeImageTool::LoadImageFromFile(const wchar_t* a_fileName, int& w, int& h, int& bpp, std::vector<int>& a_data)
  {
    const std::wstring fileExt = CutFileExt(a_fileName);

    if (fileExt == L".image4f" || fileExt == L".image1i" || fileExt == L".image1ui" || fileExt == L".image4b" || fileExt == L".image4ub")
    {
      return m_pInternal->LoadImageFromFile(a_fileName, w, h, bpp, a_data);
    }

    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    FIBITMAP *dib(NULL), *converted(NULL);

    int chan = 0;
    HRUtils_LoadImageFromFileToPairOfFreeImageObjects(a_fileName, dib, converted, &fif, bpp, chan);
    if (bpp == 0)
    {
      HrError(L"FreeImage failed to load image: ", a_fileName);
      return false;
    }

    w = g_objManager.m_FreeImageDll.m_pFreeImage_GetWidth(converted);
    h = g_objManager.m_FreeImageDll.m_pFreeImage_GetHeight(converted);
    
    if (w == 0 || h == 0)
    {
      HrError(L"FreeImage failed for undefined reason, file : ", a_fileName);
      g_objManager.m_FreeImageDll.m_pFreeImage_Unload(converted);
      g_objManager.m_FreeImageDll.m_pFreeImage_Unload(dib);
      return false;
    }
   
    a_data.resize(w * h * bpp / sizeof(int));
   
    HRUtils_GetImageDataFromFreeImageObject(converted, chan, (char*)a_data.data());
    
    g_objManager.m_FreeImageDll.m_pFreeImage_Unload(converted);
    g_objManager.m_FreeImageDll.m_pFreeImage_Unload(dib);

    return true;
  }

  bool FreeImageTool::LoadImageFromFile(const wchar_t* a_fileName, int& w, int& h, int& bpp, int &chan, std::vector<unsigned char>& a_data)
  {
    const std::wstring fileExt = CutFileExt(a_fileName);

    const std::vector<std::wstring> hydraExtensions = {L".image4f", L".image4b", L".image4ub", L".image1i", L".image1ui",
                                                       L".image1ub", L".image1f"};
    auto found = std::find(hydraExtensions.begin(), hydraExtensions.end(), fileExt);
    if (found != hydraExtensions.end())
    {
      return m_pInternal->LoadImageFromFile(a_fileName, w, h, bpp, chan, a_data);
    }

    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    FIBITMAP *dib(NULL), *converted(NULL);

    HRUtils_LoadImageFromFileToPairOfFreeImageObjects(a_fileName, dib, converted, &fif, bpp ,chan);
    if (bpp == 0)
    {
      HrError(L"FreeImage failed to load image: ", a_fileName);
      return false;
    }

    w = g_objManager.m_FreeImageDll.m_pFreeImage_GetWidth(converted);
    h = g_objManager.m_FreeImageDll.m_pFreeImage_GetHeight(converted);

    if (w == 0 || h == 0)
    {
      HrError(L"FreeImage failed for undefined reason, file : ", a_fileName);
      g_objManager.m_FreeImageDll.m_pFreeImage_Unload(converted);
      g_objManager.m_FreeImageDll.m_pFreeImage_Unload(dib);
      return false;
    }

    a_data.resize(w * h * bpp);

    HRUtils_GetImageDataFromFreeImageObject(converted, chan, (char*)a_data.data());

    g_objManager.m_FreeImageDll.m_pFreeImage_Unload(converted);
    g_objManager.m_FreeImageDll.m_pFreeImage_Unload(dib);

    return true;
  }


  bool FreeImageTool::LoadImageFromFile(const wchar_t* a_fileName, int& w, int& h, int &chan, std::vector<float>& a_data)
  {
    const std::wstring fileExt = CutFileExt(a_fileName);

    const std::vector<std::wstring> hydraExtensions = {L".image4f", L".image4b", L".image4ub", L".image1i", L".image1ui",
                                                       L".image1ub", L".image1f"};
    auto found = std::find(hydraExtensions.begin(), hydraExtensions.end(), fileExt);
    int bpp = 0;
    if (found != hydraExtensions.end())
    {
      return m_pInternal->LoadImageFromFile(a_fileName, w, h, chan, a_data);
    }

    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    FIBITMAP *dib(NULL), *converted(NULL);

    HRUtils_LoadImageFromFileToPairOfFreeImageObjects(a_fileName, dib, converted, &fif, bpp ,chan);
    if (bpp == 0)
    {
      HrError(L"FreeImage failed to load image: ", a_fileName);
      return false;
    }

    w = g_objManager.m_FreeImageDll.m_pFreeImage_GetWidth(converted);
    h = g_objManager.m_FreeImageDll.m_pFreeImage_GetHeight(converted);

    if (w == 0 || h == 0)
    {
      HrError(L"FreeImage failed for undefined reason, file : ", a_fileName);
      g_objManager.m_FreeImageDll.m_pFreeImage_Unload(converted);
      g_objManager.m_FreeImageDll.m_pFreeImage_Unload(dib);
      return false;
    }

    a_data.resize(w * h * chan);

    HRUtils_GetImageDataFromFreeImageObject(converted, chan, (char*)a_data.data());

    g_objManager.m_FreeImageDll.m_pFreeImage_Unload(converted);
    g_objManager.m_FreeImageDll.m_pFreeImage_Unload(dib);

    return true;
  }

  void FreeImageTool::SaveHDRImageToFileHDR(const wchar_t* a_fileName, int w, int h, int chan, const float* a_data)
  {
    const std::wstring fileExt = CutFileExt(a_fileName);

    if (fileExt == L".image4f" || fileExt == L".image1f")
      m_pInternal->SaveHDRImageToFileHDR(a_fileName, w, h, chan, a_data);
    else
    {
      auto type = FIT_RGBAF;
      if(chan == 1)
        type = FIT_FLOAT;


      //FIBITMAP *dib = g_objManager.m_FreeImageDll.m_pFreeImage_ConvertFromRawBitsEx(FALSE, (BYTE*)a_data, type, w, h, sizeof(float) * chan * w, chan * sizeof(float) * 8, FI_RGBA_BLUE_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_RED_MASK, FALSE);
      FIBITMAP *dib   = g_objManager.m_FreeImageDll.m_pFreeImage_ConvertFromRawBits  (       (BYTE*)a_data,       w, h, sizeof(float) * chan * w, chan * sizeof(float) * 8, FI_RGBA_BLUE_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_RED_MASK, FALSE);

      g_objManager.m_FreeImageDll.m_pFreeImage_SetOutputMessage(FreeImageErrorHandlerHydraInternal);

      auto imageType   = FIF_HDR;
      if      (fileExt == L".exr"  || fileExt == L".EXR")  imageType      = FIF_EXR;
      else if (fileExt == L".tiff" || fileExt == L".TIFF") imageType      = FIF_TIFF;

      #if defined WIN32
      if (!g_objManager.m_FreeImageDll.m_pFreeImage_SaveU(imageType, dib, a_fileName, 0))
      #else
      char filename_s[512];
      wcstombs(filename_s, a_fileName, sizeof(filename_s));
      if (!FreeImage_Save(imageType, dib, filename_s))
      #endif
      {
        g_objManager.m_FreeImageDll.m_pFreeImage_Unload(dib);
        HrError(L"SaveImageToFile(): FreeImage_Save error: ", a_fileName);
        return;
      }

      g_objManager.m_FreeImageDll.m_pFreeImage_Unload(dib);
    }
  }

  void FreeImageTool::SaveLDRImageToFileLDR(const wchar_t* a_fileName, int w, int h, const int* a_data)
  {
    const std::wstring fileExt = CutFileExt(a_fileName);
    if (fileExt == L".image1i" || fileExt == L".image1ui" || fileExt == L".image4b" || fileExt == L".image4ub")
      m_pInternal->SaveLDRImageToFileLDR(a_fileName, w, h, a_data);
    else
    {
      //BYTE* bits = (BYTE*)a_data;
      //for (int i = 0; i<w*h; i++)
      //  bits[4 * i + 3] = 255;
      // FIBITMAP *dib = FreeImage_ConvertFromRawBits((BYTE*)a_data, w, h, 4 * w, 32, FI_RGBA_BLUE_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_RED_MASK, FALSE);

      FIBITMAP* dib = g_objManager.m_FreeImageDll.m_pFreeImage_Allocate(w, h, 32, 0, 0, 0);
      BYTE* bits    = g_objManager.m_FreeImageDll.m_pFreeImage_GetBits(dib);
      //memcpy(bits, data, w*h*sizeof(int32_t));
      const BYTE* data2 = (const BYTE*)a_data;
      for (int i = 0; i<w*h; i++)
      {
        bits[4 * i + 0] = data2[4 * i + 2];
        bits[4 * i + 1] = data2[4 * i + 1];
        bits[4 * i + 2] = data2[4 * i + 0];
        bits[4 * i + 3] = 255; // data2[4 * i + 3]; // 255 to kill alpha channel
      }

      g_objManager.m_FreeImageDll.m_pFreeImage_SetOutputMessage(FreeImageErrorHandlerHydraInternal);

      auto imageFileFormat = FIF_PNG;

      std::wstring fileName(a_fileName);
      if (fileName.size() > 4)
      {
        std::wstring resolution = fileName.substr(fileName.size() - 4, 4);

        if (resolution.find(L".bmp") != std::wstring::npos || resolution.find(L".BMP") != std::wstring::npos)
          imageFileFormat = FIF_BMP;
      }
      #if defined WIN32
      if (!g_objManager.m_FreeImageDll.m_pFreeImage_SaveU(imageFileFormat, dib, a_fileName, 0))
      #else
      char filename_s[512];
      wcstombs(filename_s, a_fileName, sizeof(filename_s));
      if (!FreeImage_Save(imageFileFormat, dib, filename_s))
      #endif
      {
        g_objManager.m_FreeImageDll.m_pFreeImage_Unload(dib);
        HrError(L"SaveImageToFile(): FreeImage_Save error on ", a_fileName);
        return;
      }

      g_objManager.m_FreeImageDll.m_pFreeImage_Unload(dib);
    } // else 

  } // end function


  void FreeImageTool::Save16BitMonoImageTo16BitPNG(const wchar_t* a_fileName, int w, int h, const unsigned short* a_data)
  {
    FIBITMAP* image = g_objManager.m_FreeImageDll.m_pFreeImage_AllocateT(FIT_UINT16, w, h, 8, 0, 0, 0);
    auto bits       = g_objManager.m_FreeImageDll.m_pFreeImage_GetBits(image);

    memcpy(bits, a_data, w*h*sizeof(unsigned short));

    #if defined WIN32
    if (!g_objManager.m_FreeImageDll.m_pFreeImage_SaveU(FIF_PNG, image, a_fileName, 0))
    #else
    char filename_s[512];
    wcstombs(filename_s, a_fileName, sizeof(filename_s));
    if (!FreeImage_Save(FIF_PNG, image, filename_s))
    #endif
    {
      g_objManager.m_FreeImageDll.m_pFreeImage_Unload(image);
      HrError(L"SaveImageToFile(): FreeImage_Save error on ", a_fileName);
      return;
    }

    g_objManager.m_FreeImageDll.m_pFreeImage_Unload(image);
  }

  void FreeImageTool::SaveMonoHDRImageToFileHDR(const wchar_t* a_fileName, int w, int h, const float* a_data)
  {
    const std::wstring fileExt = CutFileExt(a_fileName);
    FIBITMAP* image            = g_objManager.m_FreeImageDll.m_pFreeImage_AllocateT(FIT_FLOAT, w, h, 8, 0, 0, 0);
    auto bits                  = g_objManager.m_FreeImageDll.m_pFreeImage_GetBits(image);

    memcpy(bits, a_data, w * h * sizeof(float));

    auto imageType = FIF_HDR;
    if (fileExt == L".exr" || fileExt == L".EXR")
      imageType = FIF_EXR;
    else if (fileExt == L".tiff" || fileExt == L".TIFF")
      imageType = FIF_TIFF;

#if defined WIN32
    if (!g_objManager.m_FreeImageDll.m_pFreeImage_SaveU(imageType, image, a_fileName, 0))
#else
    char filename_s[512];
    wcstombs(filename_s, a_fileName, sizeof(filename_s));
    if (!FreeImage_Save(imageType, image, filename_s))
#endif
    {
      g_objManager.m_FreeImageDll.m_pFreeImage_Unload(image);
      HrError(L"SaveMonoHDRImageToFileHDR(): FreeImage_Save error on ", a_fileName);
      return;
    }
    g_objManager.m_FreeImageDll.m_pFreeImage_Unload(image);
  }

};


