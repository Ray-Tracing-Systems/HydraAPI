#include "HydraLegacyUtils.h"


#ifdef WIN32
#else
#include <stdlib.h>
#include <zconf.h>

#endif

#include "../hydra_api/HydraInternal.h" // for use hr_mkdir

#pragma warning(disable:4996) // for sprintf to be ok

#ifdef WIN32
struct Pixel
{
  unsigned char r, g, b;
};

static void WriteBMP(const wchar_t* fname, Pixel* a_pixelData, int width, int height)
{
  BITMAPFILEHEADER bmfh;
  BITMAPINFOHEADER info;

  memset(&bmfh, 0, sizeof(BITMAPFILEHEADER));
  memset(&info, 0, sizeof(BITMAPINFOHEADER));

  int paddedsize = (width*height)*sizeof(Pixel);

  bmfh.bfType      = 0x4d42;       // 0x4d42 = 'BM'
  bmfh.bfReserved1 = 0;
  bmfh.bfReserved2 = 0;
  bmfh.bfSize      = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + paddedsize;
  bmfh.bfOffBits   = 0x36;

  info.biSize = sizeof(BITMAPINFOHEADER);
  info.biWidth = width;
  info.biHeight = height;
  info.biPlanes = 1;
  info.biBitCount = 24;
  info.biCompression = BI_RGB;
  info.biSizeImage = 0;
  info.biXPelsPerMeter = 0x0ec4;
  info.biYPelsPerMeter = 0x0ec4;
  info.biClrUsed = 0;
  info.biClrImportant = 0;

  std::ofstream out(fname, std::ios::out | std::ios::binary);
  out.write((const char*)&bmfh, sizeof(BITMAPFILEHEADER));
  out.write((const char*)&info, sizeof(BITMAPINFOHEADER));
  out.write((const char*)a_pixelData, paddedsize);
  out.flush();
  out.close();
}
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



struct SharedBufferInfo2
{
  enum LAST_WRITER { WRITER_HYDRA_GUI = 0, WRITER_HYDRA_SERVER = 1 };

  unsigned int xmlBytesize;
  unsigned int lastWriter;
  unsigned int writerCounter;
  unsigned int dummy;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool isFileExist(const char *fileName)
{
  std::ifstream infile(fileName);
  return infile.good();
}

const int GUI_FILE_SIZE = 32768;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool isTargetDevIdACPU(int a_devId, const std::vector<HydaRenderDevice>& a_devList)
{
  bool res = false;

  for (size_t i = 0; i < a_devList.size(); i++)
  {
    if (a_devList[i].id == a_devId)
    {
      res = a_devList[i].isCPU;
      break;
    }
  }

  return res;
}


bool isTargetDevIdAHydraCPU(int a_devId, const std::vector<HydaRenderDevice>& a_devList)
{
  bool res = false;

  for (size_t i = 0; i < a_devList.size(); i++)
  {
    if (a_devList[i].id == a_devId)
    {
      res = (a_devList[i].name == L"Hydra CPU");
      break;
    }
  }

  return res;
}

// std::wstring HydraInstallPathW() { return std::wstring(L"C:\\[Hydra]\\"); }

#ifdef WIN32

std::string ws2s(const std::wstring& s)
{
  int len;
  int slength = (int)s.length() + 1;
  len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);
  char* buf = new char[len];
  WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, buf, len, 0, 0);
  std::string r(buf);
  delete[] buf;
  return r;
}

std::wstring s2ws(const std::string& s)
{
  int len;
  int slength = (int)s.length() + 1;
  len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
  wchar_t* buf = new wchar_t[len];
  MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
  std::wstring r(buf);
  delete[] buf;
  return r;
}

#endif

std::wstring GetAbsolutePath(const std::wstring& a_path)
{
  if (a_path == L"")
    return L"";

  //std::wstring abs_path_str = L"";
  std::wstring path = a_path;

  if (path.size() > 8 && path.substr(0, 8) == L"file:///")
    path = path.substr(8, path.size());

  else if (path.size() > 7 && path.substr(0, 7) == L"file://")
    path = path.substr(7, path.size());

  for (int i = 0; i<path.size(); i++)
  {
    if (path[i] == (L"\\")[0])
      path[i] = (L"/")[0];
  }

#ifdef WIN32
  wchar_t buffer[4096];
  GetFullPathNameW(path.c_str(), 4096, buffer, NULL);
  return std::wstring(buffer);
#else
  char actualpath [PATH_MAX+1];
  char *ptr;

  const std::string tmp(path.begin(), path.end());
  ptr = realpath(tmp.c_str(), actualpath);

  const size_t size = std::strlen(actualpath);
  std::wstring wstr;
  if (size > 0) {
    wstr.resize(size);
    std::mbstowcs(&wstr[0], actualpath, size);
  }

  return wstr;
#endif

}

