//
// Created by vsan on 21.01.18.
//

#include <unistd.h>
#include <spawn.h>
#include <csignal>
#include "HydraLegacyUtils.h"

struct HydraProcessLauncher : IHydraNetPluginAPI
{
  HydraProcessLauncher(const char* imageFileName, int width, int height, const char* connectionType, std::ostream* a_pLog = nullptr);
  ~HydraProcessLauncher();

  bool hasConnection() const override;

  void runAllRenderProcesses(RenderProcessRunParams a_params, const std::vector<HydraRenderDevice>& a_devList, const std::vector<int>& a_activeDevices) override;
  void stopAllRenderProcesses() override;

protected:

  bool m_hydraServerStarted;
  std::ostream* m_pLog;
  
  std::vector<pid_t> m_mdProcessList;

  std::string m_connectionType;
  std::string m_imageFileName;

  int m_width;
  int m_height;
};


static std::ofstream g_logMain;

IHydraNetPluginAPI* CreateHydraServerConnection(int renderWidth, int renderHeight, bool inMatEditor)
{
  IHydraNetPluginAPI* pImpl = nullptr;
  long ticks = sysconf(_SC_CLK_TCK);

  std::stringstream ss;
  ss << ticks;

  std::string imageName   = std::string("HydraHDRImage_") + ss.str();
  std::string messageName = std::string("HydraMessageShmem_") + ss.str();
  std::string guiName     = std::string("HydraGuiShmem_") + ss.str();

  std::ostream* logPtr = nullptr;

  if (!inMatEditor)
  {
    if (!g_logMain.is_open())
      g_logMain.open("/home/vsan/test/log.txt");
    logPtr = &g_logMain;
    pImpl  = new HydraProcessLauncher(imageName.c_str(), renderWidth, renderHeight, "main", logPtr);
  }
  else // if in matEditor
  {

  }

  if (pImpl->hasConnection())
    return pImpl;
  else
  {
    delete pImpl;
    return nullptr;
  }

}



HydraProcessLauncher::HydraProcessLauncher(const char* imageFileName, int width, int height, const char* connectionType, std::ostream* a_pLog) :
                                           m_imageFileName(imageFileName), m_connectionType(connectionType), m_width(width), m_height(height), m_hydraServerStarted(false), m_pLog(a_pLog)
{
  m_mdProcessList.clear();
}

HydraProcessLauncher::~HydraProcessLauncher()
{
  stopAllRenderProcesses();
}


bool HydraProcessLauncher::hasConnection() const
{
  return true;
}

#include <thread>

void CreateProcessUnix(const char* exePath, const char* allArgs, const bool a_debug, std::ostream* a_pLog, std::vector<pid_t>& a_mdProcessList)
{
  std::string command = std::string(exePath) + " " + std::string(allArgs) + " &";
  system(command.c_str());
}

void HydraProcessLauncher::runAllRenderProcesses(RenderProcessRunParams a_params, const std::vector<HydraRenderDevice>& a_devList, const std::vector<int>& a_activeDevices)
{
  if (m_connectionType == "main")
  {
    char user_name[L_cuserid];
    cuserid(user_name);

    std::stringstream ss;
    ss << "/home/" << user_name << "/hydra/";

    std::string hydraPath = ss.str();
    if (a_params.customExePath != "")
      hydraPath = a_params.customExePath;

    if (!isFileExist(hydraPath.c_str()))
    {
      m_hydraServerStarted = false;
    }
    else
    {
      ss.str(std::string());
      ss << "-nowindow 1 ";
      ss << a_params.customExeArgs.c_str();
      if(!a_params.customLogFold.empty())
        ss << " -logdir \"" << a_params.customLogFold.c_str() << "\" ";


      std::string basicCmd = ss.str();

      m_hydraServerStarted = true;
      std::ofstream fout(hydraPath + "zcmd.txt");

      for (int devId : a_activeDevices)
      {
        ss.str(std::string());
        ss << " -cl_device_id " << devId;

        std::string cmdFull = basicCmd + ss.str();
        std::string hydraExe(hydraPath + "hydra");
  
        if(!a_params.debug)
          CreateProcessUnix(hydraExe.c_str(), cmdFull.c_str(), a_params.debug, m_pLog, m_mdProcessList);
        fout << cmdFull.c_str() << std::endl;
      }

      fout.close();
    }
  }
}

void HydraProcessLauncher::stopAllRenderProcesses()
{
  if (m_hydraServerStarted)
  {
    for (auto pid : m_mdProcessList)
    {
      if (pid <= 0)
        continue;

      kill(pid, SIGKILL);
    }
  }
}
