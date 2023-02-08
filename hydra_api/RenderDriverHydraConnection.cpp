#include "HydraRenderDriverAPI.h"

#include <vector>
#include <string>
#include <sstream>
#include <filesystem>
#include <fstream>

//#include "HydraInternal.h" // #TODO: this is only for hr_mkdir and hr_cleardir. Remove this further

#include "HydraLegacyUtils.h"

#include "HydraXMLHelpers.h"

#include "HR_HDRImage.h"
#include "HydraInternal.h"
#include "HydraObjectManager.h"

#include <mutex>
#include <future>
#include <thread>

#include "ssemath.h"


#ifdef WIN32
#include "../utils/clew/clew.h"
#else
#define CL_TARGET_OPENCL_VERSION 100
#include <CL/cl.h>
#endif

#pragma warning(disable:4996) // for wcscpy to be ok

static constexpr bool MODERN_DRIVER_DEBUG = false;

using HydraRender::HDRImage4f;

struct RD_HydraConnection : public IHRRenderDriver
{
  RD_HydraConnection();

  ~RD_HydraConnection() override
  {
    ClearAll();
  }

  void GetRenderDriverName(std::wstring &name) override { name = std::wstring(L"HydraModern");};
  void              ClearAll() override;
  HRDriverAllocInfo AllocAll(HRDriverAllocInfo a_info) override;

  bool UpdateImage(int32_t a_texId, int32_t w, int32_t h, int32_t bpp, int32_t chan, const void* a_data, pugi::xml_node a_texNode) override;
  bool UpdateMaterial(int32_t a_matId, pugi::xml_node a_materialNode) override;

  bool UpdateLight(int32_t a_lightIdId, pugi::xml_node a_lightNode) override;
  bool UpdateMesh(int32_t a_meshId, pugi::xml_node a_meshNode, const HRMeshDriverInput& a_input, const HRBatchInfo* a_batchList, int32_t listSize) override;

  bool UpdateImageFromFile(int32_t a_texId, const wchar_t* a_fileName, pugi::xml_node a_texNode) override;
  bool UpdateMeshFromFile(int32_t a_meshId, pugi::xml_node a_meshNode, const wchar_t* a_fileName) override;

  bool UpdateCamera(pugi::xml_node a_camNode) override;
  bool UpdateSettings(pugi::xml_node a_settingsNode) override;

  /////////////////////////////////////////////////////////////////////////////////////////////

  void BeginScene(pugi::xml_node a_sceneNode) override;
  void EndScene() override;
  void InstanceMeshes(int32_t a_mesh_id, const float* a_matrices, int32_t a_instNum, const int* a_lightInstId, const int* a_remapId, const int* a_realInstId) override;
  void InstanceLights(int32_t a_light_id, const float* a_matrix, pugi::xml_node* a_custAttrArray, int32_t a_instNum, int32_t a_lightGroupId) override;

  void Draw() override;

  HRRenderUpdateInfo HaveUpdateNow(int a_maxRaysPerPixel) override;

  void GetFrameBufferHDR(int32_t w, int32_t h, float*   a_out, const wchar_t* a_layerName) override;
  void GetFrameBufferLDR(int32_t w, int32_t h, int32_t* a_out)                             override;

  void GetFrameBufferLineHDR(int32_t a_xBegin, int32_t a_xEnd, int32_t y, float* a_out, const wchar_t* a_layerName) override;
  void GetFrameBufferLineLDR(int32_t a_xBegin, int32_t a_xEnd, int32_t y, int32_t* a_out)                           override;

  void GetGBufferLine(int32_t a_lineNumber, HRGBufferPixel* a_lineData, int32_t a_startX, int32_t a_endX, const std::unordered_set<int32_t>& a_shadowCatchers) override;
  
  void    LockFrameBufferUpdate()   override;
  void    UnlockFrameBufferUpdate() override;
  
  // info and devices
  //
  const HRRenderDeviceInfoListElem* DeviceList() const override;
  bool EnableDevice(int32_t id, bool a_enable) override;

  void ExecuteCommand(const wchar_t* a_cmd, wchar_t* a_out) override;
  void EndFlush() override;

  void SetLogDir(const wchar_t* a_logDir, bool a_hideCmd) override;

protected:

  std::wstring m_libPath;
  std::wstring m_libFileState;
  std::wstring m_logFolder;
  std::string  m_logFolderS;
  std::string  m_lastSharedImageName;
  
  int          m_instancesNum; //// current instance num in the scene; (m_instancesNum == 0) => empty scene!
  int          m_oldCounter;
  float        m_oldSPP;
  int          m_clewInitRes;
  bool         m_dontRun;

  void InitBothDeviceList();
  void CreateAndClearSharedImage();
  void RunAllHydraHeads();
  void RunSingleHydraHead(const char* a_cmdLine);

  std::vector<int> GetCurrDeviceList();
  std::string MakeRenderCommandLine(const std::string& hydraImageName) const;
  int GetCurrSharedImageLayersNum();

  RenderProcessRunParams GetCurrRunProcessParams();

  HRDriverAllocInfo m_currAllocInfo;

  IHydraNetPluginAPI*                  m_pConnection;
  IHRSharedAccumImage*                 m_pSharedImage;

  std::vector<HydraRenderDevice>          m_devList;
  std::vector<HRRenderDeviceInfoListElem> m_devList2;

  float m_progressVal;
  RenderProcessRunParams m_params;

  bool m_enableMLT = false;
  bool m_hideCmd   = false;
  bool m_needGbuff = false;

  struct RenderPresets
  {
    int  maxrays;
    bool allocImageB;
    bool allocImageBOnGPU;
    int   mmltThreads;
    float mmltMultBrightness;
    
    float mlt_med_threshold;
    bool  mlt_med_enable;
  } m_presets;

  bool m_firstUpdate;

  int m_width;
  int m_height;
  int m_channels;

  float m_avgBrightness;
  int   m_avgBCounter;

  bool  m_enableMedianFilter;
  float m_medianthreshold;

  bool m_stopThreadImmediately;
  bool m_threadIsRun;
  bool m_threadFinished;
  bool haveUpdateFromMT;
  bool hadFinalUpdate;

  // MLT/MMLT asynchronous frame buffer update
  //
  HydraRender::HDRImage4f m_colorMLTFinalImage;
  std::future<int>        m_mltFrameBufferUpdateThread;
  bool                    m_mltFrameBufferUpdate_ExitNow;
  int                     m_lastMaxRaysPerPixel;
  std::atomic<bool>       m_colorImageIsLocked;

  int MLT_FrameBufferUpdateLoop();

};

static inline float contribFunc(float colorX, float colorY, float colorZ)
{
  return fmax(0.33334f*(colorX + colorY + colorZ), 0.0f);
}

using namespace cvex;

int RD_HydraConnection::MLT_FrameBufferUpdateLoop()
{
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  m_colorImageIsLocked = false;
  size_t iter = 0;

  const float* indirect = m_pSharedImage->ImageData(0);
  const float* direct   = m_pSharedImage->ImageData(1);

  while(!m_mltFrameBufferUpdate_ExitNow)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    if(m_mltFrameBufferUpdate_ExitNow)
      break;

    if(!HaveUpdateNow(m_lastMaxRaysPerPixel).haveUpdateFB)
      continue;

    double summ[3];
    std::tie(summ[0], summ[1], summ[2]) = HydraRender::ColorSummImage4f(indirect, m_width, m_height);
    double avgDiv       = 1.0/double(m_width*m_height);
	  float avgBrightness = contribFunc(float(avgDiv*summ[0]), float(avgDiv*summ[1]), float(avgDiv*summ[2]));
    
    // calc normalisation constant
    //
    const float normC   = m_pSharedImage->Header()->avgImageB / fmax(avgBrightness, 1e-30f) * m_presets.mmltMultBrightness;
    const float normDL  = 1.0f/float(m_pSharedImage->Header()->spp);
  
    const cvex::vfloat4 normDL4 = {normDL, normDL, normDL, 0.0f};
    const cvex::vfloat4 normC4  = {normC,  normC,  normC,  0.0f};
  
    const size_t imagesize = m_colorMLTFinalImage.width()*m_colorMLTFinalImage.height()*4;
    float* dataOut = m_colorMLTFinalImage.data();
  
    while(m_colorImageIsLocked); // wait for use to free framebuffer
    m_colorImageIsLocked = true;
    
    if(m_presets.mlt_med_enable)
    {
      for(size_t i=0; i<imagesize; i+=4)
      {
        const cvex::vfloat4 RGB0 = cvex::load_u(indirect + i)*normC4;
        cvex::store(dataOut + i, RGB0);
      }
      
      m_colorMLTFinalImage.medianFilterInPlace(m_presets.mlt_med_threshold);
      
      for(size_t i=0; i<imagesize; i+=4)
      {
        const cvex::vfloat4 RGB0 = cvex::load(dataOut+i) + cvex::load_u(direct+i)*normDL4;
        cvex::store(dataOut + i, RGB0);
      }
    }
    else
    {
      for(size_t i=0; i<imagesize; i+=4)
      {
        const cvex::vfloat4 RGB0 = cvex::load_u(direct+i)*normDL4 + cvex::load_u(indirect+i)*normC4;
        cvex::store(dataOut + i, RGB0);
      }
    }
    m_colorImageIsLocked = false;
    
    iter++;
  }

  std::cout << "exit from MLT_FrameBufferUpdateLoop" << std::endl;
  std::cout.flush();

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PlatformDevPair
{
  PlatformDevPair(cl_device_id a_dev, cl_platform_id a_platform) : dev(a_dev), platform(a_platform) {}

  cl_device_id   dev;
  cl_platform_id platform;
};

static std::string cutSpaces(const std::string& a_rhs)
{
  int pos = 0;
  for (int i = 0; i < a_rhs.size(); i++)
  {
    if (a_rhs[i] != ' ')
      break;
    pos++;
  }

  return a_rhs.substr(pos, a_rhs.size() - pos);
}

static std::vector<PlatformDevPair> listAllOpenCLDevices()
{
  const int MAXPLATFORMS = 4;
  const int MAXDEVICES_PER_PLATFORM = 8;

  cl_platform_id platforms[MAXPLATFORMS];
  cl_device_id   devices[MAXDEVICES_PER_PLATFORM];

  cl_uint factPlatroms = 0;
  cl_uint factDevs = 0;

  std::vector<PlatformDevPair> result;

  clGetPlatformIDs(MAXPLATFORMS, platforms, &factPlatroms);

  for (size_t i = 0; i < factPlatroms; i++)
  {
    clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, MAXDEVICES_PER_PLATFORM, devices, &factDevs);
    for (cl_uint j = 0; j < factDevs; j++)
      result.push_back(PlatformDevPair(devices[j], platforms[i]));
  }

  return result;

}

void RD_HydraConnection::InitBothDeviceList()
{
#ifdef WIN32
  if (m_clewInitRes == -1)
  {
    m_clewInitRes = clewInit(L"opencl.dll");
    if (m_clewInitRes == -1)
    {
      // HydraRenderDevice cpuDev; #TODO: add CPU "-1" dev when it will work
      m_devList.resize(0);
      m_devList2.resize(0);
      return;
    }
  }
#endif

  // (1) get device list and info
  //
  const std::vector<PlatformDevPair> devList = listAllOpenCLDevices();
  m_devList.resize(devList.size());
  char deviceName[1024];

  for (size_t i = 0; i < devList.size(); i++)
  {
    memset(deviceName, 0, 1024);
    clGetDeviceInfo(devList[i].dev, CL_DEVICE_NAME, 1024, deviceName, nullptr);
    std::string devName2 = cutSpaces(deviceName);
    m_devList[i].name    = s2ws(devName2);

    cl_device_type devType = CL_DEVICE_TYPE_GPU;
    clGetDeviceInfo(devList[i].dev, CL_DEVICE_TYPE, sizeof(cl_device_type), &devType, nullptr);
    m_devList[i].isCPU = (devType == CL_DEVICE_TYPE_CPU);

    memset(deviceName, 0, 1024);
    clGetPlatformInfo(devList[i].platform, CL_PLATFORM_VERSION, 1024, deviceName, nullptr);
    m_devList[i].driverName = s2ws(deviceName);
    m_devList[i].id         = int(i);
  }
 
  // (2) form list in memory ... )
  //
  m_devList2.resize(m_devList.size()); 
  for (size_t i = 0; i < m_devList2.size(); i++)
  {
    const HydraRenderDevice&       devInfo = m_devList[i];
    HRRenderDeviceInfoListElem* pListElem = &m_devList2[i];
  
    pListElem->id        = (int32_t)(i);
    pListElem->isCPU     = devInfo.isCPU;
    pListElem->isEnabled = false;
  
    wcscpy(pListElem->name,   devInfo.name.c_str());
    wcscpy(pListElem->driver, devInfo.driverName.c_str());
  
    if (i != m_devList2.size() - 1)
      pListElem->next = &m_devList2[i + 1];
    else
      pListElem->next = nullptr;
  }
}

IHRRenderDriver* CreateHydraConnection_RenderDriver()
{
  return new RD_HydraConnection;
}


RD_HydraConnection::RD_HydraConnection() : m_pConnection(nullptr), m_pSharedImage(nullptr), m_progressVal(0.0f), m_firstUpdate(true),
                                           m_width(0), m_height(0), m_channels(0), m_avgBrightness(0.0f), m_avgBCounter(0),
                                           m_enableMedianFilter(false), m_medianthreshold(0.4f), m_stopThreadImmediately(false),
                                           haveUpdateFromMT(false), m_threadIsRun(false), m_threadFinished(false), hadFinalUpdate(false),
                                           m_clewInitRes(-1), m_instancesNum(0), m_colorImageIsLocked(false)
{
  InitBothDeviceList();

  m_oldCounter = 0;
  m_oldSPP     = 0.0f;
  m_dontRun    = false;
  //#TODO: init m_presets

  HydraSSE::exp2_init();
  HydraSSE::log2_init();
}

void RD_HydraConnection::ClearAll()
{
  delete m_pConnection;
  m_pConnection = nullptr;

  delete m_pSharedImage;
  m_pSharedImage = nullptr;

  m_presets.maxrays          = 2048;
  m_presets.allocImageB      = false;
  m_presets.allocImageBOnGPU = false;

}

HRDriverAllocInfo RD_HydraConnection::AllocAll(HRDriverAllocInfo a_info)
{
  m_libPath = std::wstring(a_info.libraryPath);
  if(a_info.stateFileName == nullptr)
    m_libFileState = L"";
  else
    m_libFileState = std::wstring(a_info.stateFileName);
  
  m_currAllocInfo = a_info;
  return m_currAllocInfo;
}

const HRRenderDeviceInfoListElem* RD_HydraConnection::DeviceList() const
{
  if (m_devList2.empty())
    return nullptr;
  else
    return m_devList2.data();
}

bool RD_HydraConnection::EnableDevice(int32_t id, bool a_enable)
{
  int devNumTotal = 0;
  const auto* devList = DeviceList();

  while (devList != nullptr)
  {
    devNumTotal++;
    devList = devList->next;
  }

  if (id >= devNumTotal)
    return false;

  if (id < m_devList2.size())
    m_devList2[id].isEnabled = a_enable;

  return true;
}


void RD_HydraConnection::SetLogDir(const wchar_t* a_logDir, bool a_hideCmd)
{
  std::filesystem::path logPath(a_logDir);
  if (std::filesystem::exists(logPath))
  {
    m_logFolder  = logPath.wstring();
    m_logFolderS = logPath.string();
    m_hideCmd = a_hideCmd;
  }
  else if (m_pInfoCallBack != nullptr)
  {
      m_pInfoCallBack(L"bad log dir", L"RD_HydraConnection::SetLogDir", HR_SEVERITY_WARNING);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool RD_HydraConnection::UpdateImage(int32_t a_texId, int32_t w, int32_t h, int32_t bpp, int32_t chan, const void* a_data, pugi::xml_node a_texNode)
{
  return true;
}

bool RD_HydraConnection::UpdateImageFromFile(int32_t a_texId, const wchar_t* a_fileName, pugi::xml_node a_texNode)
{
  return true;
}

bool RD_HydraConnection::UpdateMaterial(int32_t a_matId, pugi::xml_node a_materialNode)
{
  return true;
}

bool RD_HydraConnection::UpdateLight(int32_t a_lightIdId, pugi::xml_node a_lightNode)
{
  return true;
}

bool RD_HydraConnection::UpdateMesh(int32_t a_meshId, pugi::xml_node a_meshNode, const HRMeshDriverInput& a_input, const HRBatchInfo* a_batchList, int32_t listSize)
{
  return true;
}

bool RD_HydraConnection::UpdateMeshFromFile(int32_t a_meshId, pugi::xml_node a_meshNode, const wchar_t* a_fileName)
{
  return false;
}

bool RD_HydraConnection::UpdateCamera(pugi::xml_node a_camNode)
{
  return true;
}


bool RD_HydraConnection::UpdateSettings(pugi::xml_node a_settingsNode)
{
  m_width     = a_settingsNode.child(L"width").text().as_int();
  m_height    = a_settingsNode.child(L"height").text().as_int();

  if(a_settingsNode.child(L"framebuffer_channels"))
  {
    m_channels = a_settingsNode.child(L"framebuffer_channels").text().as_int();
  }
  else
  {
    m_channels = 4;
  }
 
  m_needGbuff = (std::wstring(a_settingsNode.child(L"evalgbuffer").text().as_string()) == L"1");

  m_presets.maxrays     = a_settingsNode.child(L"maxRaysPerPixel").text().as_int();

  m_presets.allocImageB = (std::wstring(a_settingsNode.child(L"method_secondary").text().as_string()) == L"lighttracing") || 
                          (std::wstring(a_settingsNode.child(L"method_primary").text().as_string())   == L"lighttracing") || 
                          (std::wstring(a_settingsNode.child(L"method_primary").text().as_string())   == L"IBPT") || 
                          (std::wstring(a_settingsNode.child(L"method_secondary").text().as_string()) == L"IBPT");

  m_presets.allocImageBOnGPU = (std::wstring(a_settingsNode.child(L"forceGPUFrameBuffer").text().as_string()) == L"1");

  std::wstring tmp  = std::wstring(a_settingsNode.child(L"render_exe_dir").text().as_string());
  if(!tmp.empty())
    m_params.customExePath = std::string(tmp.begin(), tmp.end());

  if(a_settingsNode.child(L"dont_run").text().as_int() == 1)
    m_dontRun = true;
  else
    m_dontRun = false;

  m_enableMLT = false;
  if(a_settingsNode.child(L"method_secondary") != nullptr)
  {
    if (std::wstring(a_settingsNode.child(L"method_secondary").text().as_string()) == L"mmlt" ||
        std::wstring(a_settingsNode.child(L"method_secondary").text().as_string()) == L"mlt")
      m_enableMLT = true;
  }

  if (a_settingsNode.child(L"mmlt_threads") != nullptr)
    m_presets.mmltThreads = a_settingsNode.child(L"mmlt_threads").text().as_int();
  else
    m_presets.mmltThreads = 0;

  if (a_settingsNode.child(L"mmlt_multBrightness") != nullptr)
    m_presets.mmltMultBrightness = a_settingsNode.child(L"mmlt_multBrightness").text().as_float();
  else
    m_presets.mmltMultBrightness = 1.0f;
  
  // mlt median filter
  //
  if (a_settingsNode.child(L"mlt_med_threshold") != nullptr)
    m_presets.mlt_med_threshold = a_settingsNode.child(L"mlt_med_threshold").text().as_float();
  else
    m_presets.mlt_med_threshold = 0.4f;
  
  if (a_settingsNode.child(L"mlt_med_enable") != nullptr)
    m_presets.mlt_med_enable = a_settingsNode.child(L"mlt_med_enable").text().as_bool();
  else
    m_presets.mlt_med_enable = false;
  
  return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring GetAbsolutePath(const std::wstring& a_path);

std::vector<int> RD_HydraConnection::GetCurrDeviceList()
{
  std::vector<int> devList;
  
  for (const auto& dev : m_devList2)
  {
    if (dev.isEnabled)
      devList.push_back(dev.id);
  }
  
  if (devList.empty())
  {
    devList.resize(1);
    devList[0] = 0;
    
    if (m_pInfoCallBack != nullptr)
      m_pInfoCallBack(L"No device was selected; will use device 0 by default", L"RD_HydraConnection::GetCurrDeviceList()", HR_SEVERITY_WARNING);
  }
  
  return devList;
}

std::string NewSharedImageName()
{
  auto currtime = std::chrono::system_clock::now();
  auto now_ms   = std::chrono::time_point_cast<std::chrono::milliseconds>(currtime).time_since_epoch().count();
  std::stringstream nameStream;
  nameStream << "hydraimage_" << now_ms;
  return nameStream.str();
}

std::string RD_HydraConnection::MakeRenderCommandLine(const std::string& hydraImageName) const 
{
  const int  width       = m_width;
  const int  height      = m_height;
  const bool needGBuffer = m_needGbuff;
  
  const std::wstring libPath = GetAbsolutePath(m_libPath);
  std::string temp = ws2s(libPath);
  std::stringstream auxInput;
  auxInput << "-inputlib \"" << temp.c_str() << "\" -width " << width << " -height " << height << " ";
  
  if (m_presets.allocImageB)          // this is needed for LT and IBPT
    auxInput << "-alloc_image_b 1 ";  // this is needed for LT and IBPT
  
  if(m_presets.allocImageBOnGPU)
    auxInput << "-cpu_fb 0 ";
  else
    auxInput << "-cpu_fb 1 ";
  
  if (m_enableMLT)
  {
    auxInput << "-enable_mlt 1 ";
    if (m_presets.mmltThreads != 0)
      auxInput << "-mmltthreads " << m_presets.mmltThreads << " ";
  }

  if (needGBuffer)
    auxInput << "-evalgbuffer 1 ";
  
  auxInput << "-sharedimage " << hydraImageName.c_str() << " ";
  
  return auxInput.str();
}

int RD_HydraConnection::GetCurrSharedImageLayersNum()
{
  const bool runMLT      = m_enableMLT;
  const bool needGBuffer = m_needGbuff;

  int layersNum;
  if (runMLT && needGBuffer)
    layersNum = 4;
  else if (needGBuffer)
    layersNum = 3;
  else if (runMLT)
    layersNum = 2;
  else
    layersNum = 1;

  return layersNum;
}

void RD_HydraConnection::CreateAndClearSharedImage()
{
  ////////////////////////////////////////////////////////////////////////////////////////////

  std::string hydraImageName = m_lastSharedImageName;
  const std::vector<int> devList   = GetCurrDeviceList();

  const bool needGBuffer = m_needGbuff;
  const int layersNum    = GetCurrSharedImageLayersNum();

  ////////////////////////////////////////////////////////////////////////////////////////////

  if (m_pSharedImage != nullptr) // if image exists check if it can be reused
  {
    auto *pHeader = m_pSharedImage->Header();
    if (pHeader->width == m_width && pHeader->height == m_height && pHeader->channels == m_channels && pHeader->depth == layersNum)
    {
      m_pSharedImage->Clear();
    }
    else
    {
      delete m_pSharedImage;
      m_pSharedImage = nullptr;
    }
  }

  if (m_pSharedImage == nullptr)
  {
    m_pSharedImage = CreateImageAccum();
    hydraImageName = NewSharedImageName();
  }

  char err[256];
  const bool shmemImageIsOk = m_pSharedImage->Create(m_width, m_height, layersNum, m_channels,
                                                     hydraImageName.c_str(), err);
  
  if (!shmemImageIsOk)
  {
    //#TODO: call error callback or do some thing like that
    return;
  }

  m_oldSPP     = 0.0f;
  m_oldCounter = 0;

  m_pSharedImage->Header()->spp        = 0.0f;
  m_pSharedImage->Header()->counterRcv = 0;
  m_pSharedImage->Header()->counterSnd = 0;
  m_pSharedImage->Header()->gbufferIsEmpty = needGBuffer ? 1 : -1;

  strncpy(m_pSharedImage->MessageSendData(), "-layer color -action wait ", 256); // #TODO: (sid, mid) !!!
  m_pSharedImage->Header()->counterSnd++;

  m_lastSharedImageName = hydraImageName;

  delete m_pConnection;
  m_pConnection = CreateHydraServerConnection(m_width, m_height, false);
}

RenderProcessRunParams RD_HydraConnection::GetCurrRunProcessParams()
{
  RenderProcessRunParams params;

  params.compileShaders    = false;
  params.debug             = MODERN_DRIVER_DEBUG;
  params.enableMLT         = m_enableMLT;
  params.normalPriorityCPU = false;
  params.showConsole       = !m_hideCmd;

  params.customExePath     = m_params.customExePath;
  params.customExeArgs     = MakeRenderCommandLine(m_lastSharedImageName);
  params.customLogFold     = m_logFolderS;

  return params;
}


void RD_HydraConnection::RunAllHydraHeads()
{
  if (m_pConnection == nullptr)
    return;
  
  auto currDevs = GetCurrDeviceList();
  auto params   = GetCurrRunProcessParams();
  m_params      = params;
  
  m_pConnection->runAllRenderProcesses(params, m_devList, currDevs);
}

int ReadDeviceId(const char* a_cmdLine)
{
  std::stringstream strIn(a_cmdLine);
  int devId = 0;  //#TODO: read device id from command line
  std::string name;
  while (!strIn.eof())
  {
    strIn >> name;
    if (name == "-cl_device_id")
    {
      strIn >> devId;
      break;
    }
  }

  return devId;
}

void RD_HydraConnection::RunSingleHydraHead(const char* a_cmdLine)
{
  if (m_pConnection == nullptr)
    return;

  auto params = GetCurrRunProcessParams();

  std::vector<int> devs(1);
  devs[0] = ReadDeviceId(a_cmdLine);

  params.customExeArgs += " ";
  params.customExeArgs += a_cmdLine;

  m_pConnection->runAllRenderProcesses(params, m_devList, devs, true);
}

void RD_HydraConnection::BeginScene(pugi::xml_node a_sceneNode)
{
  m_progressVal   = 0.0f;
  m_avgBrightness = 1.0f;
  m_avgBCounter   = 0;

  m_stopThreadImmediately = false;
  haveUpdateFromMT        = false;
  hadFinalUpdate          = false;

  // (3) run hydras
  //
  if (!m_dontRun)
  {
    CreateAndClearSharedImage();
    RunAllHydraHeads();
  }

  m_firstUpdate  = true;
  m_instancesNum = 0;
}


void RD_HydraConnection::EndScene()
{
  
}

void RD_HydraConnection::EndFlush()
{
  if (m_pSharedImage == nullptr)
    return;

  // sent message to render that it can finally start
  //
  std::string firstMsg = "-node_t A -sid 0 -layer color -action start";
  strncpy(m_pSharedImage->MessageSendData(), firstMsg.c_str(), 256); // #TODO: (sid, mid) !!!
  m_pSharedImage->Header()->counterSnd++;

  // run async framebuffer update loop if MLT is used
  //
  if(m_enableMLT)
  {
    m_colorMLTFinalImage.resize(m_width, m_height);
    const size_t imagesize = m_colorMLTFinalImage.width()*m_colorMLTFinalImage.height()*4;
    float* colorMLTFinal   = m_colorMLTFinalImage.data();
    
    for(size_t i=0;i<imagesize;i+=4)
    {
      colorMLTFinal[i+0] = 0.1f;
      colorMLTFinal[i+1] = 0.0f;
      colorMLTFinal[i+2] = 0.1f;
      colorMLTFinal[i+3] = 0.0f;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // a bug in shared memory connectio, therefore we need this 
    m_colorImageIsLocked           = false;
    m_lastMaxRaysPerPixel          = 1000000;
    m_mltFrameBufferUpdate_ExitNow = false;
    m_mltFrameBufferUpdateThread   = std::async(std::launch::async, &RD_HydraConnection::MLT_FrameBufferUpdateLoop, this);
  }
  else
  {
    m_mltFrameBufferUpdate_ExitNow = true;
  }
 
}

void RD_HydraConnection::Draw()
{
  // like glFinish();
}

void RD_HydraConnection::InstanceMeshes(int32_t a_meshId, const float* a_matrices, int32_t a_instNum, const int* a_lightInstId, const int* a_remapId, const int* a_realInstId)
{
  m_instancesNum += a_instNum;
}

void RD_HydraConnection::InstanceLights(int32_t a_light_id, const float* a_matrix, pugi::xml_node* a_custAttrArray, int32_t a_instNum, int32_t a_lightGroupId)
{

}

HRRenderUpdateInfo RD_HydraConnection::HaveUpdateNow(int a_maxRaysPerPixel)
{
  m_lastMaxRaysPerPixel = a_maxRaysPerPixel;

  HRRenderUpdateInfo result;
  result.finalUpdate   = false;
  result.progress      = 0.0f; 
  result.haveUpdateFB  = false;
  result.haveUpdateMSG = false;

  if (m_instancesNum == 0)
  {
    result.finalUpdate   = true;
    result.progress      = 100.0f;
    result.haveUpdateFB  = true;
    result.haveUpdateMSG = false;
    HrPrint(HR_SEVERITY_ERROR, L"RD_HydraConnection::HaveUpdateNow: no instances in scene!!");
    return result;
  }

  if (m_pSharedImage == nullptr)
    return result;
  
  auto pHeader = m_pSharedImage->Header();

  if(pHeader == nullptr)
    return result;

  if (pHeader->counterRcv == m_oldCounter)
  {
    result.haveUpdateFB  = false;
    result.haveUpdateMSG = false;
  }
  else
  {
    result.haveUpdateFB = fabs(m_oldSPP - m_pSharedImage->Header()->spp) > 1e-5f;
    result.progress     = 100.0f*m_pSharedImage->Header()->spp / float(a_maxRaysPerPixel);
    if(m_enableMLT)
      result.progress *= (2.0f); // MMLT sample indirect light 2 times more than direct light.
    m_oldCounter        = pHeader->counterRcv;
    m_oldSPP            = m_pSharedImage->Header()->spp;
  }

  result.finalUpdate = (result.progress >= 100.0f);
  
  if(!result.finalUpdate && m_enableMLT && m_mltFrameBufferUpdate_ExitNow)
    result.finalUpdate = true;

  if(result.finalUpdate)
  {
    m_mltFrameBufferUpdate_ExitNow = true;
    this->ExecuteCommand(L"exitnow", nullptr);
  }

  // check if processes are still running
  //
  if (m_pConnection != nullptr && !m_pConnection->hasConnection())
  { 
    result.finalUpdate = true;
	  this->ExecuteCommand(L"exitnow", nullptr);
  }  

  return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline int   __float_as_int(float x) { return *((int*)&x); }
static inline float __int_as_float(int x) { return *((float*)&x); }

static inline int   as_int(float x) { return __float_as_int(x); }
static inline float as_float(int x) { return __int_as_float(x); }

static inline void decodeNormal(unsigned int a_data, float a_norm[3])
{
  const float divInv = 1.0f / 32767.0f;

  short a_enc_x, a_enc_y;

  a_enc_x = (short)(a_data & 0x0000FFFF);
  a_enc_y = (short)((int)(a_data & 0xFFFF0000) >> 16);

  float sign = (a_enc_x & 0x0001) ? -1.0f : 1.0f;

  a_norm[0] = (short)(a_enc_x & 0xfffe)*divInv;
  a_norm[1] = (short)(a_enc_y & 0xfffe)*divInv;
  a_norm[2] = sign*sqrt(fmax(1.0f - a_norm[0] * a_norm[0] - a_norm[1] * a_norm[1], 0.0f));
}


static inline HRGBufferPixel UnpackGBuffer(const float a_input[4], const float a_input2[4])
{
  HRGBufferPixel res{};

  res.depth = a_input[0];
  res.matId = as_int(a_input[2]) & 0x00FFFFFF;
  decodeNormal(as_int(a_input[1]), res.norm);

  unsigned int rgba = as_int(a_input[3]);
  res.rgba[0] = (rgba & 0x000000FF)*(1.0f / 255.0f);
  res.rgba[1] = ((rgba & 0x0000FF00) >> 8)*(1.0f / 255.0f);
  res.rgba[2] = ((rgba & 0x00FF0000) >> 16)*(1.0f / 255.0f);
  res.rgba[3] = ((rgba & 0xFF000000) >> 24)*(1.0f / 255.0f);

  res.texc[0] = a_input2[0];
  res.texc[1] = a_input2[1];
  res.objId   = as_int(a_input2[2]);
  res.instId  = as_int(a_input2[3]);

  const int compressedCoverage = (as_int(a_input[2]) & 0xFF000000) >> 24;
  res.coverage = ((float)compressedCoverage)*(1.0f / 255.0f);
  res.shadow   = 0.0f;

  return res;
}


static inline float unpackAlpha(float a_input[4])
{
  const unsigned int rgba = as_int(a_input[3]);
  return ((rgba & 0xFF000000) >> 24)*(1.0f / 255.0f);
}

static float ReadFloatParam(const char* message, const char* paramName)
{
  std::istringstream iss(message);

  float res = 0.0f;

  do
  {
    std::string name, val;
    iss >> name >> val;

    if (name == paramName)
    {
      res = (float)(atof(val.c_str()));
      break;
    }

  } while (iss);

  return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RD_HydraConnection::GetFrameBufferLineHDR(int32_t a_xBegin, int32_t a_xEnd, int32_t y, float* a_out, const wchar_t* a_layerName)
{
  if (m_pSharedImage == nullptr)
    return;

  const float* data = m_pSharedImage->ImageData(0); // index depends on a_layerName
  if(m_enableMLT)
  {
    data = m_colorMLTFinalImage.data();
  }
  if (data == nullptr)
    return;

  if (m_pSharedImage->Header()->counterRcv == 0 || m_pSharedImage->Header()->spp < 1e-5f)
    return;

  const int channels = m_pSharedImage->Header()->channels;
  data = data + y * m_width * channels;

  const float invSpp = m_enableMLT ? 1.0f : 1.0f / m_pSharedImage->Header()->spp;
  const cvex::vfloat4 mult = cvex::splat(invSpp);
  auto intptr        = reinterpret_cast<std::uintptr_t>(data);

  if(m_pSharedImage->Header()->channels == 4)
  {
    if (intptr % 16 == 0)
    {
      for (int i = a_xBegin * 4; i < a_xEnd * 4; i += 4)
      {
        const cvex::vfloat4 color1 = cvex::load(data + i);
        const cvex::vfloat4 color2 = mult * color1;
        cvex::store(a_out + i - a_xBegin * 4, color2);
      }
    }
    else
    {
      for (int i = a_xBegin * 4; i < a_xEnd * 4; i += 4)
      {
        const cvex::vfloat4 color1 = cvex::load(data + i);
        const cvex::vfloat4 color2 = mult * color1;
        cvex::store_u(a_out + i - a_xBegin * 4, color2);
      }
    }
  }
  else
  {
    #pragma omp parallel for
    for (int i = a_xBegin; i < a_xEnd; ++i)
    {
      for(int j = 0; j < channels; ++j)
      {
        a_out[i * channels + j] = data[i * channels + j] * invSpp;
      }
    }
  }

  // if final update unlock image

}

static inline float clamp(float u, float a, float b) { float r = fmax(a, u); return fmin(r, b); }
static inline int   clamp(int u, int a, int b) { int r = (a > u) ? a : u; return (r < b) ? r : b; }


void RD_HydraConnection::GetFrameBufferLineLDR(int32_t a_xBegin, int32_t a_xEnd, int32_t y, int32_t* a_out)
{
  if (m_pSharedImage == nullptr)
    return;

  const float* data = m_pSharedImage->ImageData(0); // index depends on a_layerName
  if(m_enableMLT)
    data = m_colorMLTFinalImage.data();
  if (data == nullptr)
    return;

  if (m_pSharedImage->Header()->counterRcv == 0 || m_pSharedImage->Header()->spp < 1e-5f)
    return;

  int channels = m_pSharedImage->Header()->channels;

  //TODO: check for mono framebuffer with size non-divisible by 4
  data = data + y * m_width * channels;
  typedef LiteMath::float4 float4;

//

  const float invGamma  = 1.0f / 2.2f;
  const float normConst = m_enableMLT ? 1.0f : 1.0f / m_pSharedImage->Header()->spp;

  // not sse version
  //
  if (!HydraSSE::g_useSSE || channels != 4)
  {
    #pragma omp parallel for
    for (int i = a_xBegin; i < a_xEnd; i++)  // #TODO: use sse and fast pow
    {
      float4 color{};
      if(channels == 1)
        color = float4{data[i], data[i], data[i], data[i]};
      else if(channels == 4)
        color = float4{data[i * 4 + 0], data[i * 4 + 1], data[i * 4 + 2], data[i * 4 + 3]};

//        const float4 color = dataHDR[i];
      a_out[i - a_xBegin] = HRUtils::RealColorToUint32(powf(color.x * normConst, invGamma),
                                                       powf(color.y * normConst, invGamma),
                                                       powf(color.z * normConst, invGamma),
                                                       1.0f);
    }
  }
  else // sse version
  {
    if(channels == 4)
    {
      const __m128 powerf4 = _mm_set_ps1(invGamma);
      const __m128 normc = _mm_set_ps1(normConst);

      for (int i = a_xBegin; i < a_xEnd; i++)
        a_out[i - a_xBegin] = HydraSSE::gammaCorr((const float *) (data + i * 4), normc, powerf4);
    }
  }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RD_HydraConnection::LockFrameBufferUpdate()
{
  while(m_colorImageIsLocked); // wait for some fb update thread to set 'm_colorImageIsLocked' to false
  m_colorImageIsLocked = true;
}

void RD_HydraConnection::UnlockFrameBufferUpdate()
{
  m_colorImageIsLocked = false;
}

void RD_HydraConnection::GetFrameBufferHDR(int32_t w, int32_t h, float* a_out, const wchar_t* a_layerName)
{
  const int channels = m_pSharedImage->Header()->channels;
  #pragma omp parallel for
  for (int y = 0; y < h; y++)
    GetFrameBufferLineHDR(0, w, y, a_out + y * w * channels, a_layerName);
}

void RD_HydraConnection::GetFrameBufferLDR(int32_t w, int32_t h, int32_t* a_out)
{
  #pragma omp parallel for
  for (int y = 0; y < h; y++)
    GetFrameBufferLineLDR(0, w, y, a_out + y * w);
}


void RD_HydraConnection::GetGBufferLine(int32_t a_lineNumber, HRGBufferPixel* a_lineData, int32_t a_startX, int32_t a_endX, const std::unordered_set<int32_t>& a_shadowCatchers)
{
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// #TODO: Refactor this
  float* data0 = nullptr;
  float* data1 = nullptr;
  float* data2 = nullptr;
  if (m_pSharedImage == nullptr)
    return;
  
  data0 = m_pSharedImage->ImageData(0);
  if (m_pSharedImage->Header()->depth == 4) // some other process already have computed gbuffer
  {
    data1 = m_pSharedImage->ImageData(2);
    data2 = m_pSharedImage->ImageData(3);
  }
  else if (m_pSharedImage->Header()->depth == 3) // some other process already have computed gbuffer
  {
    data1 = m_pSharedImage->ImageData(1);
    data2 = m_pSharedImage->ImageData(2);
  }
  else
    return;
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// #TODO: Refactor this

  if (a_endX > m_width)
    a_endX = m_width;

  const int32_t lineOffset = (a_lineNumber*m_width + a_startX);
  const int32_t lineSize   = (a_endX - a_startX);

  const float normC = 1.0f / m_pSharedImage->Header()->spp;

  for (int32_t x = 0; x < lineSize; x++)
  {
    const float* data11  = &data1[(lineOffset + x) * 4];
    const float* data22  = &data2[(lineOffset + x) * 4];
    a_lineData[x]        = UnpackGBuffer(data11, data22);                // store main gbuffer data
    a_lineData[x].shadow = 1.0f - data0[(lineOffset + x) * 4 + 3]*normC; // get shadow from the fourthm channel

    //#TODO: move this code outside of API. to 3ds max or utility or else;

    // kill borders alpha for pixels that are neighbours to background 
    //
    //if (a_lineData[x].rgba[3] < 0.85f && a_lineData[x].coverage < 0.85f)
    //{
    //  // set up search window
    //  //
    //  constexpr int WINDOW_SIZE = 2;

    //  int minY = a_lineNumber - WINDOW_SIZE;
    //  int maxY = a_lineNumber + WINDOW_SIZE;

    //  int minX = x - WINDOW_SIZE;
    //  int maxX = x + WINDOW_SIZE;
    //  
    //  if (minY < 0) 
    //    minY = 0;
    //  if (maxY >= m_height)
    //    maxY = m_height - 1;

    //  if (minX < 0)
    //    minX = 0;
    //  if (maxX >= m_width)
    //    maxX = m_width - 1;

    //  // locate background in nearby pixels
    //  //
    //  bool foundBack = false;
    //  for (int y1 = minY; y1 <= maxY; y1++)
    //  {
    //    for (int x1 = minX; x1 <= maxX; x1++)
    //    {
    //      //const int instId = as_int(data2[(y1*m_width + x1) * 4 + 3]);
    //      const int matId = as_int(data1[(y1*m_width + x1) * 4 + 2])& 0x00FFFFFF;
    //      if (matId < 0 || matId >= int(0x00FFFFFF) || a_shadowCatchers.find(matId) != a_shadowCatchers.end())
    //      {
    //        foundBack = true;
    //        goto BREAK_BOTH;
    //      }
    //    }
    //  }
    //  BREAK_BOTH:
    //  
    //  // now finally kill alpha
    //  //
    //  if (foundBack)
    //    a_lineData[x].rgba[3] = 0.0f;
    //}

  }

}

void RD_HydraConnection::ExecuteCommand(const wchar_t* a_cmd, wchar_t* a_out)
{
  std::string inputA = ws2s(a_cmd);

  std::wstringstream inputStream(a_cmd);
  std::wstring name, value;
  inputStream >> name >> value;
  
  bool needToRunProcess  = false;
  bool needToStopProcess = false;

  if (name == L"runhydra" && m_width != 0) // this is special command to run _single_ hydra process
  {
    if(m_pSharedImage == nullptr)
      CreateAndClearSharedImage();
    
    auto* header = m_pSharedImage->Header();
    header->counterSnd++;

    std::stringstream strOut;
    strOut << (inputA.c_str() + 9) << " -boxmode 1"; // << " -mid " << header->counterSnd;
    const std::string cmdArgs = strOut.str();
    RunSingleHydraHead(cmdArgs.c_str());
    return;
  }
  else if (name == L"clearcolor")
  {
    delete m_pSharedImage;
    m_pSharedImage = nullptr;
    return;
  }
  else if (name == L"exitnow")
  {
    needToStopProcess = true;
    m_mltFrameBufferUpdate_ExitNow = true;
  }
  else if (name == L"pause")
  {
    if (m_pConnection == nullptr || m_pSharedImage == nullptr)
      return;

    auto posSpace = inputA.find("pause ");
    std::string path = inputA.substr(posSpace + 6, inputA.size());

    // (1) save imageA
    //
    if(!path.empty())
    {
      std::ofstream fout(path.c_str(), std::ios::binary);
      fout.write((const char*)m_pSharedImage->Header(), sizeof(HRSharedBufferHeader));
    
      const size_t size = size_t(m_pSharedImage->Header()->width*m_pSharedImage->Header()->height)*sizeof(float)*4;
      for(int layer=0; layer < m_pSharedImage->Header()->depth; layer++)
        fout.write((const char*)m_pSharedImage->ImageData(layer), size);
    
      fout.close();
    }

    // (2) finish all render processes
    //
    inputA = "exitnow";
    needToStopProcess = true;
    m_mltFrameBufferUpdate_ExitNow = true;
  }
  else if (name == L"resume")
  {
    // (1) load imageA
    //
    auto posSpace = inputA.find("resume ");
    std::string path = inputA.substr(posSpace + 7, inputA.size());
    if (!path.empty())
    {
      std::ifstream fin(path.c_str(), std::ios::binary);
      if (fin.is_open())
      {
        fin.read((char*)m_pSharedImage->Header(), sizeof(HRSharedBufferHeader));
        const size_t size = size_t(m_pSharedImage->Header()->width*m_pSharedImage->Header()->height) * sizeof(float) * 4;
        for (int layer = 0; layer < m_pSharedImage->Header()->depth; layer++)
          fin.read((char*)m_pSharedImage->ImageData(layer), size);
        fin.close();
      }
    }

    // (2) start rendering
    //
    inputA = "start";
  }
  else if (name == L"continue")
  {
    inputA.replace(inputA.begin(), inputA.begin()+8,"start");
    needToRunProcess = true;
  }

  if (m_pConnection == nullptr)
    return;

  
  if(m_pSharedImage != nullptr)
  {
    auto *header = m_pSharedImage->Header();
  
    std::stringstream sout2;
    sout2 << "-node_t A" << " -sid 0 -layer color -action " << inputA.c_str();
  
    std::string message = sout2.str();
    strncpy(m_pSharedImage->MessageSendData(), message.c_str(), 256);
    header->counterSnd++;
  
    if (needToRunProcess)
    {
      RunAllHydraHeads();  // #NOTE: we don't call CreateAndClearSharedImage();
      strncpy(m_pSharedImage->MessageSendData(), message.c_str(), 256);
      header->counterSnd++;
    }
  }
  
  if(needToStopProcess && m_pConnection != nullptr)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    m_pConnection->stopAllRenderProcesses();
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





