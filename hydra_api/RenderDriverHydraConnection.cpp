#include "HydraRenderDriverAPI.h"

#include <vector>
#include <string>
#include <sstream>
#include <fstream>

//#include "HydraInternal.h" // #TODO: this is only for hr_mkdir and hr_cleardir. Remove this further
#include "RenderDriverHydraLegacyStuff.h"

#include "HydraXMLHelpers.h"

#include "HR_HDRImage.h"
#include "HydraInternal.h"

#include <mutex>
#include <future>
#include <thread>

#include "ssemath.h"

#pragma warning(disable:4996) // for wcscpy to be ok

static constexpr bool MODERN_DRIVER_DEBUG = false;

using HydraRender::HDRImage4f;

struct RD_HydraConnection : public IHRRenderDriver
{
  RD_HydraConnection() : m_pConnection(nullptr), m_pSharedImage(nullptr), m_progressVal(0.0f), m_firstUpdate(true), m_width(0), m_height(0), m_avgBrightness(0.0f), m_avgBCounter(0),
                         m_enableMedianFilter(false), m_medianthreshold(0.4f), m_stopThreadImmediately(false), haveUpdateFromMT(false), m_threadIsRun(false), m_threadFinished(false), hadFinalUpdate(false)
  {
    m_msg = L"";
    InitBothDeviceList();

    m_oldCounter = 0;
    m_oldSPP     = 0.0f;
    //#TODO: init m_currAllocInfo
    //#TODO: init m_presets
    //#TODO: init m_lastServerReply

    HydraSSE::exp2_init();
    HydraSSE::log2_init();
  }

  ~RD_HydraConnection()
  {
    ClearAll();
  }

  void              ClearAll();
  HRDriverAllocInfo AllocAll(HRDriverAllocInfo a_info);

  void GetLastErrorW(wchar_t a_msg[256]) override;

  bool UpdateImage(int32_t a_texId, int32_t w, int32_t h, int32_t bpp, const void* a_data, pugi::xml_node a_texNode) override;
  bool UpdateMaterial(int32_t a_matId, pugi::xml_node a_materialNode) override;

  bool UpdateLight(int32_t a_lightIdId, pugi::xml_node a_lightNode) override;
  bool UpdateMesh(int32_t a_meshId, pugi::xml_node a_meshNode, const HRMeshDriverInput& a_input, const HRBatchInfo* a_batchList, int32_t listSize) override;

  bool UpdateImageFromFile(int32_t a_texId, const wchar_t* a_fileName, pugi::xml_node a_texNode) override;
  bool UpdateMeshFromFile(int32_t a_meshId, pugi::xml_node a_meshNode, const wchar_t* a_fileName) override;

  bool UpdateCamera(pugi::xml_node a_camNode) override;
  bool UpdateSettings(pugi::xml_node a_settingsNode) override;

  /////////////////////////////////////////////////////////////////////////////////////////////

  void BeginScene();
  void EndScene();
  void InstanceMeshes(int32_t a_mesh_id, const float* a_matrices, int32_t a_instNum, const int* a_lightInstId) override;
  void InstanceLights(int32_t a_light_id, const float* a_matrix, pugi::xml_node* a_custAttrArray, int32_t a_instNum, int32_t a_lightGroupId) override;

  void Draw();

  HRRenderUpdateInfo HaveUpdateNow(int a_maxRaysPerPixel) override;

  void GetFrameBufferHDR(int32_t w, int32_t h, float*   a_out, const wchar_t* a_layerName) override;
  void GetFrameBufferLDR(int32_t w, int32_t h, int32_t* a_out)                             override;

  void GetFrameBufferLineHDR(int32_t a_xBegin, int32_t a_xEnd, int32_t y, float* a_out, const wchar_t* a_layerName) override;
  void GetFrameBufferLineLDR(int32_t a_xBegin, int32_t a_xEnd, int32_t y, int32_t* a_out)                           override;

  void GetGBufferLine(int32_t a_lineNumber, HRGBufferPixel* a_lineData, int32_t a_startX, int32_t a_endX) override;

  // info and devices
  //
  HRDriverInfo Info();
  const HRRenderDeviceInfoListElem* DeviceList() const;
  void EnableDevice(int32_t id, bool a_enable);

  void ExecuteCommand(const wchar_t* a_cmd, wchar_t* a_out);
  void EndFlush() override;

  void SetLogDir(const wchar_t* a_logDir, bool a_hideCmd) override;

protected:

  std::wstring m_libPath;
  std::wstring m_msg;
  std::wstring m_logFolder;
  std::string  m_logFolderS;
  int          m_instancesNum; //// current instance num in the scene; (m_instancesNum == 0) => empty scene! 
  int          m_oldCounter;
  float        m_oldSPP;

  void InitBothDeviceList();
  void RunAllHydraHeads();

  HRDriverAllocInfo m_currAllocInfo;

  IHydraNetPluginAPI*                  m_pConnection;
  IHRSharedAccumImage*                 m_pSharedImage;

  std::vector<HydaRenderDevice>           m_devList;
  std::vector<HRRenderDeviceInfoListElem> m_devList2;

  float m_progressVal;
  RenderProcessRunParams m_params;

  bool m_enableMLT = false;
  bool m_hideCmd   = false;

  struct RenderPresets
  {
    int  maxrays;
    bool allocImageB;
  } m_presets;

  bool m_firstUpdate;

  int m_width;
  int m_height;

  float m_avgBrightness;
  int   m_avgBCounter;

  bool  m_enableMedianFilter;
  float m_medianthreshold;

  bool m_stopThreadImmediately;
  bool m_threadIsRun;
  bool m_threadFinished;
  bool haveUpdateFromMT;
  bool hadFinalUpdate;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RD_HydraConnection::InitBothDeviceList()
{
  m_devList = InitDeviceList(1);
  m_devList2.resize(m_devList.size());

  // form list in memory ... )
  //
  for (size_t i = 0; i < m_devList2.size(); i++)
  {
    const HydaRenderDevice& devInfo = m_devList[i];
    HRRenderDeviceInfoListElem* pListElem = &m_devList2[i];

    pListElem->id = (int32_t)(i);
    pListElem->isCPU = devInfo.isCPU;
    pListElem->isEnabled = false;

    wcscpy(pListElem->name, devInfo.name.c_str());
    wcscpy(pListElem->driver, devInfo.driverName.c_str());

    if (i != m_devList2.size() - 1)
      pListElem->next = &m_devList2[i + 1];
    else
      pListElem->next = nullptr;
  }
}

void RD_HydraConnection::ClearAll()
{
  delete m_pConnection;
  m_pConnection = nullptr;

  delete m_pSharedImage;
  m_pSharedImage = nullptr;

  m_presets.maxrays     = 2048;
  m_presets.allocImageB = false;

}

HRDriverAllocInfo RD_HydraConnection::AllocAll(HRDriverAllocInfo a_info)
{
  m_libPath       = std::wstring(a_info.libraryPath);
  m_currAllocInfo = a_info;
  return m_currAllocInfo;
}

const HRRenderDeviceInfoListElem* RD_HydraConnection::DeviceList() const
{
  if (m_devList2.size() == 0)
    return nullptr;
  else
    return &m_devList2[0];
}

void RD_HydraConnection::EnableDevice(int32_t id, bool a_enable)
{
  if (id < m_devList2.size())
    m_devList2[id].isEnabled = a_enable;
}

HRDriverInfo RD_HydraConnection::Info()
{
  HRDriverInfo info;

  info.supportHDRFrameBuffer              = true;
  info.supportHDRTextures                 = true;
  info.supportMultiMaterialInstance       = false;

  info.supportImageLoadFromInternalFormat = false;
  info.supportMeshLoadFromInternalFormat  = false;

  info.supportImageLoadFromExternalFormat = true;
  info.supportLighting                    = true;
  info.createsLightGeometryItself         = false;
  info.supportGetFrameBufferLine          = true;

  info.memTotal                           = int64_t(8) * int64_t(1024 * 1024 * 1024); // #TODO: wth i have to do with that ???

  return info;
}

void RD_HydraConnection::GetLastErrorW(wchar_t a_msg[256])
{
  wcscpy(a_msg, m_msg.c_str());
}

void RD_HydraConnection::SetLogDir(const wchar_t* a_logDir, bool a_hideCmd)
{
  m_logFolder  = a_logDir;
  m_logFolderS = ws2s(m_logFolder);

  const std::wstring check = s2ws(m_logFolderS);
  if (m_logFolder != check)
    m_msg = L"[RD_HydraConnection]: warning, bad log dir";

  m_hideCmd = a_hideCmd;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool RD_HydraConnection::UpdateImage(int32_t a_texId, int32_t w, int32_t h, int32_t bpp, const void* a_data, pugi::xml_node a_texNode)
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

  m_enableMLT = (std::wstring(a_settingsNode.child(L"enable_mlt").text().as_string())       == L"1")   || 
                (std::wstring(a_settingsNode.child(L"method_secondary").text().as_string()) == L"mlt") || 
                (std::wstring(a_settingsNode.child(L"method_tertiary").text().as_string())  == L"mlt") ||
                (std::wstring(a_settingsNode.child(L"method_caustic").text().as_string())   == L"mlt");
 
  m_presets.maxrays     = a_settingsNode.child(L"maxRaysPerPixel").text().as_int();
  m_presets.allocImageB = (std::wstring(a_settingsNode.child(L"method_secondary").text().as_string()) == L"lighttracing") || 
                          (std::wstring(a_settingsNode.child(L"method_primary").text().as_string())   == L"lighttracing");

  return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring GetAbsolutePath(const std::wstring& a_path);

void RD_HydraConnection::RunAllHydraHeads()
{
  int width  = m_width;
  int height = m_height;

  ////////////////////////////////////////////////////////////////////////////////////////////////////

  std::vector<int> devList;

  for (size_t i = 0; i < m_devList2.size(); i++)
  {
    if (m_devList2[i].isEnabled)
      devList.push_back(m_devList2[i].id);
  }

  if (m_devList2.size() == 0)
  {
    devList.resize(1);
    devList[0] = -1;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////

  if (m_pSharedImage == nullptr)
    m_pSharedImage = CreateImageAccum();
  else
    m_pSharedImage->Clear();

  char err[256];
  bool shmemImageIsOk = m_pSharedImage->Create(width, height, 1, "hydraimage", err);

  if (!shmemImageIsOk)
  {
    //#TODO: call error callback or do some thing like that
  }

  m_oldSPP     = 0.0f;
  m_oldCounter = 0;

  m_pSharedImage->Header()->spp        = 0.0f;
  m_pSharedImage->Header()->counterRcv = 0;
  m_pSharedImage->Header()->counterSnd = 0;
  strncpy(m_pSharedImage->MessageSendData(), "-layer color -action wait ", 256); // #TODO: (sid, mid) !!!
  m_pSharedImage->Header()->counterSnd++;

  ////////////////////////////////////////////////////////////////////////////////////////////////////

  delete m_pConnection;
  m_pConnection = CreateHydraServerConnection(width, height, false, devList);

  if (m_pConnection != nullptr)
  {
    RenderProcessRunParams params;

    params.compileShaders    = false;
    params.debug             = MODERN_DRIVER_DEBUG;
    params.enableMLT         = m_enableMLT;
    params.normalPriorityCPU = false;
    params.showConsole       = !m_hideCmd;

    const std::wstring libPath = GetAbsolutePath(m_libPath);
    std::string temp = ws2s(libPath);
    std::stringstream auxInput;
    auxInput << "-inputlib \"" << temp.c_str() << "\" -width " << width << " -height " << height << " ";

    if (m_presets.allocImageB)          // this is needed for LT and IBPT
      auxInput << "-alloc_image_b 1 ";  // this is needed for LT and IBPT

    params.customExePath = "C:/[Hydra]/bin2/hydra.exe";
    params.customExeArgs = auxInput.str();
    params.customLogFold = m_logFolderS;

    m_params = params;

    m_pConnection->runAllRenderProcesses(params, m_devList);
  }


}

void RD_HydraConnection::BeginScene()
{
  m_progressVal   = 0.0f;
  m_avgBrightness = 1.0f;
  m_avgBCounter   = 0;

  m_stopThreadImmediately = false;
  haveUpdateFromMT        = false;
  hadFinalUpdate          = false;

  // (3) run hydras
  //
  RunAllHydraHeads();

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

  std::string firstMsg = "-node_t A -sid 0 -layer color -action start";
  strncpy(m_pSharedImage->MessageSendData(), firstMsg.c_str(), 256); // #TODO: (sid, mid) !!!
  m_pSharedImage->Header()->counterSnd++;
}

void RD_HydraConnection::Draw()
{
  // like glFinish();
}

void RD_HydraConnection::InstanceMeshes(int32_t a_meshId, const float* a_matrices, int32_t a_instNum, const int* a_lightInstId)
{
  m_instancesNum += a_instNum;
}

void RD_HydraConnection::InstanceLights(int32_t a_light_id, const float* a_matrix, pugi::xml_node* a_custAttrArray, int32_t a_instNum, int32_t a_lightGroupId)
{

}

HRRenderUpdateInfo RD_HydraConnection::HaveUpdateNow(int a_maxRaysPerPixel)
{
  HRRenderUpdateInfo result;
  result.finalUpdate   = false;
  result.progress      = 0.0f; 
  result.haveUpdateFB  = false;
  result.haveUpdateMSG = false;

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
    m_oldCounter        = pHeader->counterRcv;
    m_oldSPP            = m_pSharedImage->Header()->spp;
  }

  result.finalUpdate = (result.progress >= 100.0f);

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

typedef HRGBufferPixel GBuffer1;

static inline GBuffer1 unpackGBuffer1(const float a_input[4])
{
  GBuffer1 res;

  res.depth = a_input[0];
  res.matId = as_int(a_input[2]);
  decodeNormal(as_int(a_input[1]), res.norm);

  unsigned int rgba = as_int(a_input[3]);
  res.rgba[0] = (rgba & 0x000000FF)*(1.0f / 255.0f);
  res.rgba[1] = ((rgba & 0x0000FF00) >> 8)*(1.0f / 255.0f);
  res.rgba[2] = ((rgba & 0x00FF0000) >> 16)*(1.0f / 255.0f);
  res.rgba[3] = ((rgba & 0xFF000000) >> 24)*(1.0f / 255.0f);

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
  if (data == nullptr)
    return;

  if (m_pSharedImage->Header()->counterRcv == 0 || m_pSharedImage->Header()->spp < 1e-5f)
    return;

  data = data + y*m_width*4;

  const float invSpp = 1.0f / m_pSharedImage->Header()->spp;
  const __m128 mult  = _mm_set_ps1(invSpp);
  auto intptr        = reinterpret_cast<std::uintptr_t>(data);

  // if final update lock image

  if (intptr % 16 == 0)
  {
    for (int i = a_xBegin*4; i < a_xEnd*4; i += 4)
    {
      const __m128 color1 = _mm_load_ps(data + i);
      const __m128 color2 = _mm_mul_ps(mult, color1);
      _mm_store_ps(a_out + i - a_xBegin*4, color2);
    }
  }
  else
  {
    for (int i = a_xBegin * 4; i < a_xEnd * 4; i += 4)
    {
      const __m128 color1 = _mm_load_ps(data + i);
      const __m128 color2 = _mm_mul_ps(mult, color1);
      _mm_storeu_ps(a_out + i - a_xBegin*4, color2);
    }
  }

  // if final update unlock image

}

static inline float clamp(float u, float a, float b) { float r = fmax(a, u); return fmin(r, b); }
static inline int   clamp(int u, int a, int b) { int r = (a > u) ? a : u; return (r < b) ? r : b; }

static inline int RealColorToUint32(const float real_color[4])
{
  float  r = clamp(real_color[0] * 255.0f, 0.0f, 255.0f);
  float  g = clamp(real_color[1] * 255.0f, 0.0f, 255.0f);
  float  b = clamp(real_color[2] * 255.0f, 0.0f, 255.0f);
  float  a = clamp(real_color[3] * 255.0f, 0.0f, 255.0f);

  unsigned char red   = (unsigned char)r;
  unsigned char green = (unsigned char)g;
  unsigned char blue  = (unsigned char)b;
  unsigned char alpha = (unsigned char)a;

  return red | (green << 8) | (blue << 16) | (alpha << 24);
}


void RD_HydraConnection::GetFrameBufferLineLDR(int32_t a_xBegin, int32_t a_xEnd, int32_t y, int32_t* a_out)
{
  if (m_pSharedImage == nullptr)
    return;

  const float* data = m_pSharedImage->ImageData(0); // index depends on a_layerName
  if (data == nullptr)
    return;

  if (m_pSharedImage->Header()->counterRcv == 0 || m_pSharedImage->Header()->spp < 1e-5f)
    return;

  data = data + y * m_width * 4;
  typedef HydraLiteMath::float4 float4;

  const float4* dataHDR = (const float4*)data;

  const float invGamma  = 1.0f / 2.2f;
  const float normConst = 1.0f / m_pSharedImage->Header()->spp;

  // not sse version
  //
  if (!HydraSSE::g_useSSE)
  {
    for (int i = a_xBegin; i < a_xEnd; i++)  // #TODO: use sse and fast pow
    {
      const float4 color = dataHDR[i];
      float color2[4];
      color2[0] = powf(color.x*normConst, invGamma);
      color2[1] = powf(color.y*normConst, invGamma);
      color2[2] = powf(color.z*normConst, invGamma);
      color2[3] = 1.0f;
      a_out[i - a_xBegin] = RealColorToUint32(color2);
    }
  }
  else // sse version
  {
    const __m128 powerf4 = _mm_set_ps1(invGamma);
    const __m128 normc   = _mm_set_ps1(normConst);

    for (int i = a_xBegin; i < a_xEnd; i++)  
      a_out[i - a_xBegin] = HydraSSE::gammaCorr((const float*)(dataHDR + i), normc, powerf4);
  }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RD_HydraConnection::GetFrameBufferHDR(int32_t w, int32_t h, float* a_out, const wchar_t* a_layerName)
{
  #pragma omp parallel for
  for (int y = 0; y < h; y++)
    GetFrameBufferLineHDR(0, w, y, a_out + y * w * 4, a_layerName);
}

void RD_HydraConnection::GetFrameBufferLDR(int32_t w, int32_t h, int32_t* a_out)
{
  #pragma omp parallel for
  for (int y = 0; y < h; y++)
    GetFrameBufferLineLDR(0, w, y, a_out + y*w);
}

void RD_HydraConnection::GetGBufferLine(int32_t a_lineNumber, HRGBufferPixel* a_lineData, int32_t a_startX, int32_t a_endX)
{
  if (a_endX > m_width)
    a_endX = m_width;

  const int32_t lineOffset = (a_lineNumber*m_width + a_startX);
  const int32_t lineSize   = (a_endX - a_startX);

  // for (int32_t x = 0; x < lineSize; x++)
  // {
  //   const float* data = &m_gbuffer[(lineOffset + x) * 4];
  //   a_lineData[x] = unpackGBuffer1(data);
  // }

}

void RD_HydraConnection::ExecuteCommand(const wchar_t* a_cmd, wchar_t* a_out)
{
  std::string inputA = ws2s(a_cmd);

  std::wstringstream inputStream(a_cmd);
  std::wstring name, value;
  inputStream >> name >> value;

  if (name == L"pause")
  {
    if (m_pConnection == nullptr || m_pSharedImage == nullptr)
      return;

    // (1) save imageA
    //

    // (2) finish all render processes
    //
    inputA = "exitnow";
  }
  else if (name == L"resume")
  {
    // (1) create connection and run process
    //
    RunAllHydraHeads();

    // (2) load imageA
    //
    inputA = "start";
  }

  if (m_pConnection == nullptr)
    return;

  std::stringstream sout2;
  sout2 << "-node_t A" << " -sid 0 -layer color -action " << inputA.c_str();
  
  std::string message = sout2.str();
  
  strncpy(m_pSharedImage->MessageSendData(), message.c_str(), 256);
  m_pSharedImage->Header()->counterSnd++;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IHRRenderDriver* CreateHydraConnection_RenderDriver()
{
  return new RD_HydraConnection;
}



