#pragma once

#ifdef WIN32
#include <windows.h>
#endif

#include <cstdint>
#include <fstream>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <math.h>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct HydraRenderDevice
{
  std::wstring name;
  std::wstring driverName;
  int          id;
  bool         isCPU;
};

struct RenderProcessRunParams
{
  RenderProcessRunParams() : debug(false), normalPriorityCPU(false), showConsole(true), compileShaders(false), enableMLT(false) {}

  bool debug;
  bool normalPriorityCPU;
  bool showConsole;
  bool compileShaders;
  bool enableMLT;

  std::string customExePath;
  std::string customExeArgs;
  std::string customLogFold;
};

struct IHydraNetPluginAPI
{
  IHydraNetPluginAPI(){}
  virtual ~IHydraNetPluginAPI(){}

  virtual bool hasConnection() const = 0;

  /**
  \brief run several hydra processes (for each device id listed in activeDevices list)
  \param a_params
  \param a_devList     -- all device list
  \param activeDevices -- list of active device id (that may not be equal to id in 'a_devList' array)
  */
  virtual void runAllRenderProcesses(RenderProcessRunParams a_params, const std::vector<HydraRenderDevice>& a_devList, const std::vector<int>& activeDevices) = 0;
  virtual void stopAllRenderProcesses() = 0;
};

struct SharedBufferDataInfo
{
  int width;
  int height;
  int read;
  int written;
};


bool isTargetDevIdACPU(int a_devId, const std::vector<HydraRenderDevice>& a_devList);
bool isTargetDevIdAHydraCPU(int a_devId, const std::vector<HydraRenderDevice>& a_devList);

IHydraNetPluginAPI* CreateHydraServerConnection(int renderWidth, int renderHeight, bool inMatEditor);

std::string  ws2s(const std::wstring& s);
std::wstring s2ws(const std::string& s);
bool isFileExist(const char *fileName);
