#include "HydraAPI.h"
#include "HydraInternal.h"

#include <memory>
#include <vector>
#include <string>
#include <map>

#include <sstream>
#include <fstream>
#include <iomanip>

#include "HydraObjectManager.h"

extern std::wstring      g_lastError;
extern std::wstring      g_lastErrorCallerPlace;
extern HR_ERROR_CALLBACK g_pErrorCallback;
extern HRObjectManager   g_objManager;


#include <FreeImage.h>
#include <cmath>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HAPI bool hrRenderGetFrameBufferHDR4f(const HRRenderRef a_pRender, int w, int h, float* imgData, const wchar_t* a_layerName) // (w,h) is strongly related to viewport size; return true if image was final  
{
  HRRender* pRender = g_objManager.PtrById(a_pRender);

  if (pRender == nullptr)
  {
    HrError(L"hrRenderGetFrameBufferHDR4f, nullptr Render Driver ");
    return false;
  }

  if (pRender->m_pDriver == nullptr)
  {
    HrError(L"hrRenderGetFrameBufferHDR4f, nullptr Render Driver impl ");
    return false;
  }

  pRender->m_pDriver->GetFrameBufferHDR(w, h, imgData, a_layerName);

  return true;
}


HAPI bool hrRenderGetFrameBufferLDR1i(const HRRenderRef a_pRender, int w, int h, int32_t* imgData)
{
  HRRender* pRender = g_objManager.PtrById(a_pRender);

  if (pRender == nullptr)
  {
    HrError(L"hrRenderGetFrameBufferLDR1i, nullptr Render Driver ");
    return false;
  }

  if (pRender->m_pDriver == nullptr)
  {
    HrError(L"hrRenderGetFrameBufferLDR1i, nullptr Render Driver impl ");
    return false;
  }

  pRender->m_pDriver->GetFrameBufferLDR(w, h, imgData);

  return true;
}

HAPI bool hrRenderGetFrameBufferLineHDR4f(const HRRenderRef a_pRender, int a_begin, int a_end, int a_y, float* imgData, const wchar_t* a_layerName)
{
  HRRender* pRender = g_objManager.PtrById(a_pRender);

  if (pRender == nullptr)
  {
    HrError(L"hrRenderGetFrameBufferLineHDR4f, nullptr Render Driver ");
    return false;
  }

  if (pRender->m_pDriver == nullptr)
  {
    HrError(L"hrRenderGetFrameBufferLineHDR4f, nullptr Render Driver impl ");
    return false;
  }

  if (!pRender->m_pDriver->Info().supportGetFrameBufferLine)
  {
    HrError(L"hrRenderGetFrameBufferLineHDR4f is not implemented for general case yet. try different render driver");
    return false;
  }

  pRender->m_pDriver->GetFrameBufferLineHDR(a_begin, a_end, a_y, imgData, a_layerName);
  return true;
}

HAPI bool hrRenderGetFrameBufferLineLDR1i(const HRRenderRef a_pRender, int a_begin, int a_end, int a_y, int32_t* imgData)
{
  HRRender* pRender = g_objManager.PtrById(a_pRender);
  if (pRender == nullptr)
  {
    HrError(L"hrRenderGetFrameBufferLineLDR1i, nullptr Render Driver ");
    return false;
  }

  if (pRender->m_pDriver == nullptr)
  {
    HrError(L"hrRenderGetFrameBufferLineLDR1i, nullptr Render Driver impl ");
    return false;
  }

  if (!pRender->m_pDriver->Info().supportGetFrameBufferLine)
  {
    HrError(L"GetFrameBufferLineLDR1i is not implemented for general case yet. try different render driver");
    return false;
  }

  pRender->m_pDriver->GetFrameBufferLineLDR(a_begin, a_end, a_y, imgData);
  return true;
}

std::wstring ToWideString(const std::string& rhs)
{
  std::wstring res; res.resize(rhs.size());
  std::copy(rhs.begin(), rhs.end(), res.begin());
  return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void FreeImageErrorHandlerHydraInternal(FREE_IMAGE_FORMAT fif, const char *message)
{
  std::wstringstream strOut;
  strOut << L"(FIF = " << fif << L")";
  const std::wstring wstr = std::wstring(L"[FreeImage ") + strOut.str() + std::wstring(L"]: ") + ToWideString(message);
  HrError(wstr.c_str());
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
    40,0,0,0, // info hd size
    0,0,0,0,  // width
    0,0,0,0,  // heigth
    1,0,      // number color planes
    24,0,     // bits per pixel
    0,0,0,0,  // compression is none
    0,0,0,0,  // image bits size
    0x13,0x0B,0,0, // horz resoluition in pixel / m
    0x13,0x0B,0,0, // vert resolutions (0x03C3 = 96 dpi, 0x0B13 = 72 dpi)
    0,0,0,0,  // #colors in pallete
    0,0,0,0,  // #important colors
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



bool HR_SaveLDRImageToFile(const wchar_t* a_fileName, int w, int h, const int32_t* data)
{
  FIBITMAP* dib = FreeImage_Allocate(w, h, 32);

  BYTE* bits = FreeImage_GetBits(dib);
  //memcpy(bits, data, w*h*sizeof(int32_t));
  const BYTE* data2 = (const BYTE*)data;
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
  size_t len = wcstombs(filename_s, a_fileName, sizeof(filename_s));
  if (!FreeImage_Save(imageFileFormat, dib, filename_s))
#endif
  {
    FreeImage_Unload(dib);
    HrError(L"SaveImageToFile(): FreeImage_Save error on ", a_fileName);
    return false;
  }

  FreeImage_Unload(dib);

  return true;
}

static inline float clamp(float u, float a, float b) { float r = fmax(a, u); return fmin(r, b); }

bool HR_SaveHDRImageToFileLDR(const wchar_t* a_fileName, int w, int h, const float* a_data, const float a_scaleInv, const float a_gamma = 2.2f)
{
  struct float4 { float x, y, z, w; };
  const float4* data = (const float4*)a_data;

  const float gammaPow = 1.0f / a_gamma;
  std::vector<int32_t> imageLDR(w*h);

  const int size = w*h;
  #pragma omp parallel for
  for (int i = 0; i < size; i++)
  {
    float4 color = data[i];

    color.x = powf(clamp(color.x*a_scaleInv, 0.0f, 1.0f), gammaPow);
    color.y = powf(clamp(color.y*a_scaleInv, 0.0f, 1.0f), gammaPow);
    color.z = powf(clamp(color.z*a_scaleInv, 0.0f, 1.0f), gammaPow);
    color.w = 1.0f;

    float  r = clamp(color.x * 255.0f, 0.0f, 255.0f);
    float  g = clamp(color.y * 255.0f, 0.0f, 255.0f);
    float  b = clamp(color.z * 255.0f, 0.0f, 255.0f);
    float  a = 1.0f;

    unsigned char red   = (unsigned char)r;
    unsigned char green = (unsigned char)g;
    unsigned char blue  = (unsigned char)b;
    unsigned char alpha = (unsigned char)a;

    imageLDR[i] = red | (green << 8) | (blue << 16) | (alpha << 24);
  }

  return HR_SaveLDRImageToFile(a_fileName, w, h, &imageLDR[0]);
}

bool HR_SaveHDRImageToFileHDR(const wchar_t* a_fileName, int w, int h, const float* a_data, const float a_scale = 1.0f)
{
  struct float3 { float x, y, z; };
  struct float4 { float x, y, z, w; };

  const float4* data = (const float4*)a_data;

  std::vector<float3> tempData(w*h);
  for (int i = 0; i < w*h; i++)
  {
    float4 src = data[i];
    float3 dst;
    dst.x = src.x*a_scale;
    dst.y = src.y*a_scale;
    dst.z = src.z*a_scale;
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
  size_t len = wcstombs(filename_s, a_fileName, sizeof(filename_s));
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HAPI bool hrRenderSaveFrameBufferLDR(const HRRenderRef a_pRender, const wchar_t* a_outFileName)
{
  HRRender* pRender = g_objManager.PtrById(a_pRender);

  if (pRender == nullptr)
  {
    HrError(L"hrRenderSaveFrameBufferLDR, nullptr Render Driver ");
    return false;
  }

  if (pRender->m_pDriver == nullptr)
  {
    HrError(L"hrRenderSaveFrameBufferLDR, nullptr Render Driver impl ");
    return false;
  }

  pugi::xml_node node = pRender->xml_node_immediate();

  int w = node.child(L"width").text().as_int();
  int h = node.child(L"height").text().as_int();

  if (w <= 0 || h <= 0)
  {
    HrError(L"hrRenderSaveFrameBufferLDR, <width> or <height> xml param of render was not set");
    return false;
  }

  std::vector<int32_t> imgData(w*h);
  pRender->m_pDriver->GetFrameBufferLDR(w, h, &imgData[0]);

  //HR_MyDebugSaveBMP(a_outFileName, &imgData[0], w, h);
  //return true;

  return HR_SaveLDRImageToFile(a_outFileName, w, h, &imgData[0]);
}

HAPI bool hrRenderSaveFrameBufferHDR(const HRRenderRef a_pRender, const wchar_t* a_outFileName)
{
  HRRender* pRender = g_objManager.PtrById(a_pRender);

  if (pRender == nullptr)
  {
    HrError(L"hrRenderSaveFrameBufferHDR, nullptr Render Driver ");
    return false;
  }

  if (pRender->m_pDriver == nullptr)
  {
    HrError(L"hrRenderSaveFrameBufferHDR, nullptr Render Driver impl ");
    return false;
  }

  pugi::xml_node node = pRender->xml_node_immediate();

  int w = node.child(L"width").text().as_int();
  int h = node.child(L"height").text().as_int();

  if (w <= 0 || h <= 0)
  {
    HrError(L"hrRenderSaveFrameBufferHDR, <width> or <height> xml param of render was not set");
    return false;
  }

  std::vector<float> imgData(w*h*4);
  pRender->m_pDriver->GetFrameBufferHDR(w, h, &imgData[0], L"color");

  return HR_SaveHDRImageToFileHDR(a_outFileName, w, h, &imgData[0]);
}

HAPI void hrRenderCommand(const HRRenderRef a_pRender, const wchar_t* a_command, wchar_t* a_answer)
{
  HRRender* pRender = g_objManager.PtrById(a_pRender);
  if (pRender == nullptr)
  {
    HrError(L"[hrRenderCommand]: nullptr Render Driver ");
    return;
  }

  if (pRender->m_pDriver == nullptr)
  {
    HrError(L"[hrRenderCommand]: nullptr Render Driver impl ");
    return;
  }
  
  std::wstring command(a_command);
  if(command == L"start" && g_objManager.scnData.m_fileState!= L"") // just 'start', without additional arguments
  {
    command += std::wstring(L" -statefile ") + g_objManager.scnData.m_fileState;
    a_command = command.c_str();
  }
  
  pRender->m_pDriver->ExecuteCommand(a_command, a_answer);

}

HAPI void hrRenderLogDir(const HRRenderRef a_pRender, const wchar_t* a_logDir, bool a_hrRenderLogDir)
{
  HRRender* pRender = g_objManager.PtrById(a_pRender);
  if (pRender == nullptr)
  {
    HrError(L"[hrRenderLogDir]: nullptr Render Driver ");
    return;
  }

  if (pRender->m_pDriver == nullptr)
  {
    HrError(L"[hrRenderLogDir]: nullptr Render Driver impl ");
    return;
  }

  pRender->m_pDriver->SetLogDir(a_logDir, a_hrRenderLogDir);

}
