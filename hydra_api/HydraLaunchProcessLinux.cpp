//
// Created by vsan on 21.01.18.
//

#include <unistd.h>
#include <spawn.h>
#include <signal.h>
#include "HydraLegacyUtils.h"

struct HydraProcessLauncher : IHydraNetPluginAPI
{
  HydraProcessLauncher(const char* imageFileName, int width, int height, const char* connectionType, const std::vector<int>& a_devIdList, std::ostream* m_pLog = nullptr);
  ~HydraProcessLauncher();

  bool hasConnection() const override;

  void runAllRenderProcesses(RenderProcessRunParams a_params, const std::vector<HydaRenderDevice>& a_devList) override;
  void stopAllRenderProcesses() override;

protected:

  bool m_hydraServerStarted;
  std::ostream* m_pLog;


  std::vector<int>   m_mdDeviceList;
  std::vector<pid_t> m_mdProcessList;

  std::string m_connectionType;
  std::string m_imageFileName;

  int m_width;
  int m_height;
};


static std::ofstream g_logMain;

IHydraNetPluginAPI* CreateHydraServerConnection(int renderWidth, int renderHeight, bool inMatEditor, const std::vector<int>& a_devList)
{
  static int m_matRenderTimes = 0;

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
    pImpl  = new HydraProcessLauncher(imageName.c_str(), renderWidth, renderHeight, "main", a_devList, logPtr);
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



HydraProcessLauncher::HydraProcessLauncher(const char* imageFileName, int width, int height, const char* connectionType, const std::vector<int>& a_devIdList, std::ostream* a_pLog) :
        m_imageFileName(imageFileName), m_connectionType(connectionType), m_width(width), m_height(height), m_hydraServerStarted(false), m_pLog(a_pLog)
{
  m_mdDeviceList.clear();
  m_mdProcessList.clear();

  m_mdDeviceList = a_devIdList;
}

HydraProcessLauncher::~HydraProcessLauncher()
{
  stopAllRenderProcesses();
}


bool HydraProcessLauncher::hasConnection() const
{
  return true;
}


void HydraProcessLauncher::runAllRenderProcesses(RenderProcessRunParams a_params, const std::vector<HydaRenderDevice>& a_devList)
{

  bool a_debug             = a_params.debug;

  const char* imageFileName = m_imageFileName.c_str();

  int width = m_width;
  int height = m_height;

  //m_mdProcessList.resize(a_devList.size());

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
        ss << "-logdir \"" << a_params.customLogFold.c_str() << "\" ";


      std::string basicCmd = ss.str();

      m_hydraServerStarted = true;
      std::ofstream fout(hydraPath + "zcmd.txt");

      for (size_t i = 0; i < m_mdDeviceList.size(); i++)
      {
        int devId = m_mdDeviceList[i];

        ss.str(std::string());
        ss << " -cl_device_id " << devId;

        std::string cmdFull = basicCmd + ss.str();
        std::string hydraExe(hydraPath + "hydra");

        std::vector<char> hydraExe_cstr(hydraExe.c_str(), hydraExe.c_str() + hydraExe.size() + 1);
        std::vector<char> cmdFull_cstr(cmdFull.c_str(), cmdFull.c_str() + cmdFull.size() + 1);


        char *cmd[] = {&hydraExe_cstr[0], &cmdFull_cstr[0], NULL};

        if (!a_debug)
        {
         /* pid_t pid;
          int status;

          status = posix_spawn(&pid, "/home/vsan/test/a.out", NULL, NULL, cmd, environ);
          (*m_pLog) << status << "PID : " << pid << std::endl;*/

          auto pid = fork();

          switch (pid)
          {
            case -1:
              (*m_pLog) << "error forking hydraAPI" << std::endl;
              break;
            case 0: //child process
              (*m_pLog) << "before executing Hydra Core" << std::endl;
              execv(hydraExe.c_str(), cmd);
              (*m_pLog) << "error launching or executing Hydra Core" << std::endl;
              exit(1);
            default:
              m_mdProcessList.push_back(pid);
              fout << cmdFull.c_str() << std::endl;
              break;
          }

        }
      }

      fout.close();

    }
  }
}

void HydraProcessLauncher::stopAllRenderProcesses()
{
  if (m_hydraServerStarted)
  {
    for (auto i = 0; i < m_mdProcessList.size(); i++)
    {
      if (m_mdProcessList[i] <= 0)
        continue;

      kill(m_mdProcessList[i], SIGKILL);

    }
  }
}
