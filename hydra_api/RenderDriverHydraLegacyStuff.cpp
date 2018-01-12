#include "RenderDriverHydraLegacyStuff.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <math.h>

#include "../hydra_api/HydraInternal.h" // for use hr_mkdir

#pragma warning(disable:4996) // for sprintf to be ok

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


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool g_materialProcessStart = false;
PROCESS_INFORMATION g_materialProcessInfo;

struct PluginShmemPipe : public IHydraNetPluginAPI
{
  PluginShmemPipe(const char* imageFileName, const char* messageFileName, const char* guiFile, int width, int height,
                  const char* connectionType = "main",
                  const std::vector<int>& a_devIdList = std::vector<int>(),
                  std::ostream* m_pLog = nullptr);

  virtual ~PluginShmemPipe();
  bool hasConnection() const;

  void runAllRenderProcesses(RenderProcessRunParams a_params, const std::vector<HydaRenderDevice>& a_devList);
  void stopAllRenderProcesses();

  HANDLE getMtlRenderHProcess() const { return g_materialProcessInfo.hProcess; }

protected:

  void CreateConnectionMainType(const char* imageFileName, const char* messageFileName, const char* guiFile, int width, int height);

  STARTUPINFOA m_hydraStartupInfo;
  PROCESS_INFORMATION m_hydraProcessInfo;
  BOOL m_hydraServerStarted;
  bool m_staticConnection;

  // when multi-device mode is used
  bool m_multiDevMode;
  std::vector<PROCESS_INFORMATION> m_mdProcessList;
  std::vector<int>                 m_mdDeviceList;
  // \\

  unsigned int m_lastImageType;

  enum { MESSAGE_BUFF_SIZE = 1024 };


  // params that we have to remember after connection created
  //
  std::string m_connectionType;
  std::string m_imageFileName;

  int m_width;
  int m_height;

  std::ostream* m_pLog;
};

static IHydraNetPluginAPI* g_pMaterialRenderConnect = nullptr;
static std::ofstream g_logMain;
static std::ofstream g_logMtl;

IHydraNetPluginAPI* CreateHydraServerConnection(int renderWidth, int renderHeight, bool inMatEditor, const std::vector<int>& a_devList)
{
  static int m_matRenderTimes = 0;

  IHydraNetPluginAPI* pImpl = nullptr;
  DWORD ticks = GetTickCount();

  std::stringstream ss;
  ss << ticks;

  std::string imageName   = std::string("HydraHDRImage_") + ss.str();
  std::string messageName = std::string("HydraMessageShmem_") + ss.str();
  std::string guiName     = std::string("HydraGuiShmem_") + ss.str();

  std::ostream* logPtr = nullptr;

  if (!inMatEditor)
  {
    if (!g_logMain.is_open())
      g_logMain.open("C:/[Hydra]/logs/plugin_log.txt");
    logPtr = &g_logMain;
    pImpl  = new PluginShmemPipe(imageName.c_str(), messageName.c_str(), guiName.c_str(), renderWidth, renderHeight, "main", a_devList, logPtr);
  }
  else // if in matEditor
  {
    if (!g_logMtl.is_open())
      g_logMtl.open("C:/[Hydra]/logs/material_log.txt");
    logPtr = &g_logMtl;

    if ((m_matRenderTimes != 0) && (m_matRenderTimes % 40 == 0)) // restart mtl render process each 32 render to prevent mem leaks
    {
      //hydraRender_mk3::plugin_log.Print("restart material render");
      if (g_pMaterialRenderConnect != nullptr)
        g_pMaterialRenderConnect->stopAllRenderProcesses();

      g_materialProcessStart = false;
    }

    pImpl = g_pMaterialRenderConnect;

    if (pImpl == nullptr)
    {
      g_pMaterialRenderConnect = new PluginShmemPipe(imageName.c_str(), messageName.c_str(), guiName.c_str(), 512, 512, "material", a_devList, logPtr);
      pImpl = g_pMaterialRenderConnect;
    }

    m_matRenderTimes++;
  }

  if (pImpl->hasConnection())
    return pImpl;
  else
  {
    delete pImpl;
    return nullptr;
  }

}

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PluginShmemPipe ///
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PluginShmemPipe::PluginShmemPipe(const char* imageFileName, const char* messageFileName, const char* guiFileName, int width, int height, const char* connectionType, const std::vector<int>& a_devIdList, std::ostream* a_pLog) : m_hydraServerStarted(false), m_staticConnection(false), m_pLog(a_pLog)
{
  m_mdDeviceList.clear();
  m_mdProcessList.clear();

  m_connectionType = connectionType;
  m_mdDeviceList = a_devIdList;

  if (m_connectionType == "main")
  {
    m_multiDevMode = true; // to enable specular filter
    m_mdProcessList.resize(m_mdDeviceList.size());
    CreateConnectionMainType(imageFileName, messageFileName, guiFileName, width, height);
  }
  else
  {
    m_multiDevMode = false;
  }

}

void PluginShmemPipe::CreateConnectionMainType(const char* imageFileName, const char* messageFileName, const char* guiFileName, int width, int height)
{
  m_staticConnection = false;

  // remember params
  //
  m_imageFileName   = imageFileName;

  m_width = width;
  m_height = height;
}



void PluginShmemPipe::runAllRenderProcesses(RenderProcessRunParams a_params, const std::vector<HydaRenderDevice>& a_devList)
{
  std::ostream* outp = m_pLog;

  // restore params
  //
  bool a_showCmd           = a_params.showConsole;
  bool a_normalPriorityCPU = a_params.normalPriorityCPU;
  bool a_debug             = a_params.debug;

  const char* imageFileName = m_imageFileName.c_str();

  int width = m_width;
  int height = m_height;

  m_mdProcessList.resize(a_devList.size());

  if (m_connectionType == "main")
  {
    ZeroMemory(&m_hydraStartupInfo, sizeof(STARTUPINFOA));
    ZeroMemory(&m_hydraProcessInfo, sizeof(PROCESS_INFORMATION));

    m_hydraStartupInfo.cb = sizeof(STARTUPINFO);
    m_hydraStartupInfo.dwFlags     = STARTF_USESHOWWINDOW; // CREATE_NEW_CONSOLE // DETACHED_PROCESS
    m_hydraStartupInfo.wShowWindow = SW_SHOWMINNOACTIVE;

    const char* hydraPath = "C:\\[Hydra]\\bin\\hydra.exe";
    if (a_params.customExePath != "")
      hydraPath = a_params.customExePath.c_str();

    if (!isFileExist(hydraPath))
    {
      m_hydraServerStarted = false;
    }
    else
    {
      std::stringstream ss;
      ss << "-nowindow 1 -cpufb 1 ";
      ss << a_params.customExeArgs.c_str();
      if(a_params.customLogFold != "")
        ss << "-logdir \"" << a_params.customLogFold.c_str() << "\" ";

      //if (pImageA != nullptr)
        //pImageA->SendMsg("-node_t A -sid 0 -layer wait -action wait");

      int deviceId = m_mdDeviceList.size() == 0 ? -1 : m_mdDeviceList[0];
      int engineType = 1;
      int liteMode = false;

      std::string basicCmd = ss.str();

      m_hydraServerStarted = true;
      std::ofstream fout("C:\\[Hydra]\\pluginFiles\\zcmd.txt");

      for (size_t i = 0; i < m_mdDeviceList.size(); i++)
      {
        DWORD dwCreationFlags = a_showCmd ? 0 : CREATE_NO_WINDOW;

        int devId = m_mdDeviceList[i];
        if (isTargetDevIdACPU(devId, a_devList))
        {
          if (isTargetDevIdAHydraCPU(devId, a_devList))
          {
            devId *= -1;
            if (m_mdDeviceList.size() != 1 && !a_normalPriorityCPU)
              dwCreationFlags |= BELOW_NORMAL_PRIORITY_CLASS; // is CPU is the only one device, use normal priority, else use background mode
          }
        }

        if(a_showCmd)
          dwCreationFlags |= CREATE_NEW_CONSOLE;

        std::stringstream ss3;

        if (engineType == 1)
          ss3 << " -cl_device_id " << devId << " -cl_lite_core " << liteMode;
        else
          ss3 << " -cl_device_id " << devId;

        std::string cmdFull = basicCmd + ss3.str();

        ZeroMemory(&m_mdProcessList[i], sizeof(PROCESS_INFORMATION));
        if (!a_debug)
        {
          m_hydraServerStarted = m_hydraServerStarted && CreateProcessA(hydraPath, (LPSTR)cmdFull.c_str(), NULL, NULL, FALSE, dwCreationFlags, NULL, NULL, &m_hydraStartupInfo, &m_mdProcessList[i]);
          if (!m_hydraServerStarted && outp != nullptr)
          {
            (*outp) << "[syscall failed]: runAllRenderProcesses->(m_connectionType == 'main')->CreateProcessA " << std::endl;
          }
        }

        fout << cmdFull.c_str() << std::endl;
      }

      fout.close();

    }
  }

}


void PluginShmemPipe::stopAllRenderProcesses()
{

  if (m_multiDevMode && m_hydraServerStarted)
  {
    for (auto i = 0; i < m_mdProcessList.size(); i++)
    {
      if (m_mdProcessList[i].hProcess == 0 || m_mdProcessList[i].hProcess == INVALID_HANDLE_VALUE)
        continue;

      DWORD exitCode;
      GetExitCodeProcess(m_mdProcessList[i].hProcess, &exitCode);

      if (exitCode == STILL_ACTIVE)
        Sleep(100);

      GetExitCodeProcess(m_mdProcessList[i].hProcess, &exitCode);

      if (exitCode == STILL_ACTIVE)
        TerminateProcess(m_mdProcessList[i].hProcess, NO_ERROR);
    }
  }
  else if (m_hydraServerStarted && m_hydraProcessInfo.hProcess != 0 && m_hydraProcessInfo.hProcess != INVALID_HANDLE_VALUE)
  {
    // ask process for exit
    //
    DWORD exitCode;
    GetExitCodeProcess(m_hydraProcessInfo.hProcess, &exitCode);

    if (exitCode == STILL_ACTIVE)
      Sleep(100);

    GetExitCodeProcess(m_hydraProcessInfo.hProcess, &exitCode);

    if (exitCode == STILL_ACTIVE)
      TerminateProcess(m_hydraProcessInfo.hProcess, NO_ERROR);
  }


}

PluginShmemPipe::~PluginShmemPipe()
{
  stopAllRenderProcesses();
}

bool PluginShmemPipe::hasConnection() const
{
  return true;
}

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
  boost::filesystem::path bst_path(path);
  boost::filesystem::path abs_path = system_complete(bst_path);
  return abs_path.string();
#endif

}

