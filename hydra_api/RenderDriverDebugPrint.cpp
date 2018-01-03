#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>

#include "HydraRenderDriverAPI.h"
#include "HydraInternal.h" // #TODO: this is only for hr_mkdir and hr_cleardir. Remove this further

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct RD_DebugPrint : public IHRRenderDriver
{
  RD_DebugPrint()
  {
    // m_msg = L"";
  }

  void              ClearAll();
  HRDriverAllocInfo AllocAll(HRDriverAllocInfo a_info);

  void GetLastErrorW(wchar_t a_msg[256]) {}

  bool UpdateImage(int32_t a_texId, int32_t w, int32_t h, int32_t bpp, const void* a_data, pugi::xml_node a_texNode) { return false; }
  bool UpdateMaterial(int32_t a_matId, pugi::xml_node a_materialNode) { return false; }
  bool UpdateLight(int32_t a_lightIdId, pugi::xml_node a_lightNode) { return false; }
  bool UpdateMesh(int32_t a_meshId, pugi::xml_node a_meshNode, const HRMeshDriverInput& a_input, const HRBatchInfo* a_batchList, int32_t listSize);

  bool UpdateImageFromFile(int32_t a_texId, const wchar_t* a_fileName, pugi::xml_node a_texNode) { return false; }
  bool UpdateMeshFromFile(int32_t a_meshId, pugi::xml_node a_meshNode, const wchar_t* a_fileName) { return false; }


  bool UpdateCamera(pugi::xml_node a_camNode) { return false; }
  bool UpdateSettings(pugi::xml_node a_settingsNode) { return false; }

  /////////////////////////////////////////////////////////////////////////////////////////////

  void BeginScene() {}
  void EndScene() {}
  void InstanceMeshes(int32_t a_mesh_id, const float* a_matrices, int32_t a_instNum, const int* a_lightInstId) {}
  void InstanceLights(int32_t a_light_id, const float* a_matrix, pugi::xml_node* a_custAttrArray, int32_t a_instNum, int32_t a_lightGroupId) {}

  void Draw() {}

  HRRenderUpdateInfo HaveUpdateNow(int a_maxRaysPerPixel);

  void GetFrameBufferHDR(int32_t w, int32_t h, float*   a_out, const wchar_t* a_layerName) {}
  void GetFrameBufferLDR(int32_t w, int32_t h, int32_t* a_out) {}

  void GetGBufferLine(int32_t a_lineNumber, HRGBufferPixel* a_lineData, int32_t a_startX, int32_t a_endX) {}

  HRDriverInfo Info();
  const HRRenderDeviceInfoListElem* DeviceList() const override { return nullptr; } //#TODO: implement quering GPU info bu glGetString(GL_VENDOR) and e.t.c.
  void EnableDevice(int32_t id, bool a_enable) {}

protected:

  std::wstring libPath;

};

void RD_DebugPrint::ClearAll() 
{

}

constexpr static wchar_t* const foldername = L"/debugmeshes";

HRDriverAllocInfo RD_DebugPrint::AllocAll(HRDriverAllocInfo a_info) 
{
  libPath = a_info.libraryPath;

  std::wstring debugMeshesFolder = libPath + foldername;

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
  std::string debugMeshesFolderUnix(debugMeshesFolder.begin(), debugMeshesFolder.end());
  hr_mkdir(debugMeshesFolderUnix.c_str());
#elif defined WIN32
  hr_mkdir(debugMeshesFolder.c_str());
#endif

  return a_info;
}

void _hrDebugPrintMesh(const HRMeshDriverInput& a_input, const wchar_t* a_fileName);

bool RD_DebugPrint::UpdateMesh(int32_t a_meshId, pugi::xml_node a_meshNode, const HRMeshDriverInput& a_input, const HRBatchInfo* a_batchList, int32_t listSize)
{
  std::wstring fileName = libPath + foldername + L"/mesh_";
  std::wstringstream namestream;
  namestream << fileName.c_str() << std::setfill(L"0"[0]) << std::setw(5) << a_meshId << L".txt";

  std::wstring finalStr = namestream.str();
  _hrDebugPrintMesh(a_input, finalStr.c_str());
  return true;
}


HRRenderUpdateInfo RD_DebugPrint::HaveUpdateNow(int a_maxRaysPerPixel)
{
  HRRenderUpdateInfo res;
  res.finalUpdate  = true;
  res.haveUpdateFB = true;
  res.progress     = 100.0f;
  return res;
}

HRDriverInfo RD_DebugPrint::Info()
{
  HRDriverInfo info;
  info.createsLightGeometryItself = false;
  info.memTotal = 8* int64_t(1024*1024*1024);
  info.supportHDRFrameBuffer = false;
  info.supportHDRTextures = false;
  info.supportImageLoadFromExternalFormat = false;
  info.supportImageLoadFromInternalFormat = false;
  info.supportLighting = false;
  info.supportMeshLoadFromInternalFormat = false;
  info.supportMultiMaterialInstance = false;
  return info;
}

IHRRenderDriver* CreateDebugPrint_RenderDriver() { return new RD_DebugPrint; }
