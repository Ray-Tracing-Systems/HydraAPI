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

struct HydaRenderDevice
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

  virtual void runAllRenderProcesses(RenderProcessRunParams a_params, const std::vector<HydaRenderDevice>& a_devList) = 0;
  virtual void stopAllRenderProcesses() = 0;
};

struct SharedBufferDataInfo
{
  int width;
  int height;
  int read;
  int written;
};


bool isTargetDevIdACPU(int a_devId, const std::vector<HydaRenderDevice>& a_devList);
bool isTargetDevIdAHydraCPU(int a_devId, const std::vector<HydaRenderDevice>& a_devList);

IHydraNetPluginAPI* CreateHydraServerConnection(int renderWidth, int renderHeight, bool inMatEditor, const std::vector<int>& a_devList);

std::string  ws2s(const std::wstring& s);
std::wstring s2ws(const std::string& s);
bool isFileExist(const char *fileName);
