//
// Created by vsan on 21.01.18.
//

#include "HydraLegacyUtils.h"
#include "HydraObjectManager.h"


bool g_materialProcessStart = false;
PROCESS_INFORMATION g_materialProcessInfo;

struct PluginShmemPipe : public IHydraNetPluginAPI
{
  PluginShmemPipe(const char* imageFileName, const char* messageFileName, const char* guiFile, int width, int height, const char* connectionType = "main",
                  std::ostream* m_pLog = nullptr);

  virtual ~PluginShmemPipe();
  bool hasConnection() const;

  void runAllRenderProcesses(RenderProcessRunParams a_params, const std::vector<HydraRenderDevice>& a_devList, const std::vector<int>& activeDevices, bool a_appendMode = false);
  void stopAllRenderProcesses();

  HANDLE getMtlRenderHProcess() const { return g_materialProcessInfo.hProcess; }

protected:

  void CreateConnectionMainType(const char* imageFileName, const char* messageFileName, const char* guiFile, int width, int height);

  STARTUPINFOA m_hydraStartupInfo;
  PROCESS_INFORMATION m_hydraProcessInfo;
  BOOL m_hydraServerStarted;
  bool m_staticConnection;

  std::vector<PROCESS_INFORMATION> m_mdProcessList;

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

IHydraNetPluginAPI* CreateHydraServerConnection(int renderWidth, int renderHeight, bool inMatEditor)
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
    pImpl  = new PluginShmemPipe(imageName.c_str(), messageName.c_str(), guiName.c_str(), renderWidth, renderHeight, "main", logPtr);
  }
  else // if in matEditor
  {
  

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


PluginShmemPipe::PluginShmemPipe(const char* imageFileName, const char* messageFileName, const char* guiFileName, int width, int height, const char* connectionType, std::ostream* a_pLog) : m_hydraServerStarted(false), m_staticConnection(false), m_pLog(a_pLog)
{
  m_mdProcessList.clear();
  m_connectionType = connectionType;
  CreateConnectionMainType(imageFileName, messageFileName, guiFileName, width, height);
}

void PluginShmemPipe::CreateConnectionMainType(const char* imageFileName, const char* messageFileName, const char* guiFileName, int width, int height)
{
  m_staticConnection = false;

  // remember params
  //
  m_imageFileName = imageFileName;

  m_width  = width;
  m_height = height;
}



void PluginShmemPipe::runAllRenderProcesses(RenderProcessRunParams a_params, const std::vector<HydraRenderDevice>& a_devList, const std::vector<int>& activeDevices, bool a_appendMode)
{
  std::ostream* outp = m_pLog;

  // restore params
  //
  bool a_showCmd           = a_params.showConsole;
  bool a_normalPriorityCPU = a_params.normalPriorityCPU;
  bool a_debug             = a_params.debug;

  const char* imageFileName = m_imageFileName.c_str();

  int width  = m_width;
  int height = m_height;

  m_mdProcessList.resize(activeDevices.size());

  if (m_connectionType == "main")
  {
    ZeroMemory(&m_hydraStartupInfo, sizeof(STARTUPINFOA));
    ZeroMemory(&m_hydraProcessInfo, sizeof(PROCESS_INFORMATION));

    m_hydraStartupInfo.cb = sizeof(STARTUPINFO);
    m_hydraStartupInfo.dwFlags     = STARTF_USESHOWWINDOW; // CREATE_NEW_CONSOLE // DETACHED_PROCESS
    m_hydraStartupInfo.wShowWindow = SW_SHOWMINNOACTIVE;

    std::string hydraPath = "C:/[Hydra]/bin2/hydra.exe";
    if (a_params.customExePath != "")
      hydraPath = a_params.customExePath + "/hydra.exe";

    if (!isFileExist(hydraPath.c_str()))
    {
      m_hydraServerStarted = false;
      HrPrint(HR_SEVERITY_ERROR, L"hydra.exe was not found! (perhaps you have forgotten to install HydraCore to : ", hydraPath.c_str());
    }
    else
    {
      std::stringstream ss;
      ss << "-nowindow 1 ";
      ss << a_params.customExeArgs.c_str();
      if(a_params.customLogFold != "")
        ss << " -logdir \"" << a_params.customLogFold.c_str() << "\" ";

      //if (pImageA != nullptr)
      //pImageA->SendMsg("-node_t A -sid 0 -layer wait -action wait");

      int deviceId  = activeDevices.size() == 0 ? -1 : activeDevices[0];

      std::string basicCmd = ss.str();

      m_hydraServerStarted = true;
      std::ofstream fout;
      if (a_params.customExePath != "")
      {
        fout.close();
        fout.open(a_params.customExePath + "/zcmd.txt");
      }
      else
        fout.open("C:/[Hydra]/zcmd.txt");

      for (size_t i = 0; i < activeDevices.size(); i++)
      {
        DWORD dwCreationFlags = a_showCmd ? 0 : CREATE_NO_WINDOW;

        int devId = activeDevices[i];
        if (isTargetDevIdACPU(devId, a_devList))
        {
          if (isTargetDevIdAHydraCPU(devId, a_devList))
          {
            devId *= -1;
            if (activeDevices.size() != 1 && !a_normalPriorityCPU)
              dwCreationFlags |= BELOW_NORMAL_PRIORITY_CLASS; // is CPU is the only one device, use normal priority, else use background mode
          }
        }

        if(a_showCmd)
          dwCreationFlags |= CREATE_NEW_CONSOLE;

        std::stringstream ss3;
        ss3 << " -cl_device_id " << devId << " ";
        if (a_params.customExePath != "")
          ss3 << "-hydradir " << a_params.customExePath.c_str() << " ";
        const std::string cmdFull = basicCmd + ss3.str();

        ZeroMemory(&m_mdProcessList[i], sizeof(PROCESS_INFORMATION));
        if (!a_debug)
        {
          m_hydraServerStarted = m_hydraServerStarted && CreateProcessA(hydraPath.c_str(), (LPSTR)cmdFull.c_str(), NULL, NULL, FALSE, dwCreationFlags, NULL, NULL, &m_hydraStartupInfo, &m_mdProcessList[i]);
          if (!m_hydraServerStarted && outp != nullptr)
          {
            (*outp) << "[syscall failed]: runAllRenderProcesses->(m_connectionType == 'main')->CreateProcessA " << std::endl;
            HrPrint(HR_SEVERITY_ERROR, L"CreateProcessA have failed (perhaps you have forgottent to install C:/[Hydra]/bin2/hydra.exe)? : ", hydraPath.c_str());
          }
          else if (!m_hydraServerStarted)
            HrPrint(HR_SEVERITY_ERROR, L"CreateProcessA have failed (perhaps you have forgottent to install C:/[Hydra]/bin2/hydra.exe)? : ", hydraPath.c_str());
        }

        fout << cmdFull.c_str() << std::endl;
      }

      fout.close();

    }
  }

}


void PluginShmemPipe::stopAllRenderProcesses()
{
  if (!m_hydraServerStarted)
    return;
  
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

PluginShmemPipe::~PluginShmemPipe()
{
  stopAllRenderProcesses();
}

bool PluginShmemPipe::hasConnection() const
{
  return true;
}