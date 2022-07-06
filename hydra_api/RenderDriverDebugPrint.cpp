#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>

#include "HydraRenderDriverAPI.h"
#include "HydraInternal.h"
#include "HydraVSGFExport.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct RD_DebugPrint : public IHRRenderDriver
{
  RD_DebugPrint() = default;

  void GetRenderDriverName(std::wstring &name) override { name = std::wstring(L"DebugPrint");};

  void              ClearAll() override;
  HRDriverAllocInfo AllocAll(HRDriverAllocInfo a_info) override;

  bool UpdateImage(int32_t a_texId, int32_t w, int32_t h, int32_t bpp, int32_t chan, const void* a_data, pugi::xml_node a_texNode) override { return false; }
  bool UpdateMaterial(int32_t a_matId, pugi::xml_node a_materialNode) override { return false; }
  bool UpdateLight(int32_t a_lightIdId, pugi::xml_node a_lightNode) override { return false; }
  bool UpdateMesh(int32_t a_meshId, pugi::xml_node a_meshNode, const HRMeshDriverInput& a_input, const HRBatchInfo* a_batchList, int32_t listSize) override;

  bool UpdateImageFromFile(int32_t a_texId, const wchar_t* a_fileName, pugi::xml_node a_texNode) override { return false; }
  bool UpdateMeshFromFile(int32_t a_meshId, pugi::xml_node a_meshNode, const wchar_t* a_fileName) override { return false; }


  bool UpdateCamera(pugi::xml_node a_camNode) override { return false; }
  bool UpdateSettings(pugi::xml_node a_settingsNode) override { return false; }

  /////////////////////////////////////////////////////////////////////////////////////////////

  void BeginScene(pugi::xml_node a_sceneNode) override {}
  void EndScene() override {}
  void InstanceMeshes(int32_t a_mesh_id, const float* a_matrices, int32_t a_instNum, const int* a_lightInstId, const int* a_remapId, const int* a_realInstId) override {}
  void InstanceLights(int32_t a_light_id, const float* a_matrix, pugi::xml_node* a_custAttrArray, int32_t a_instNum, int32_t a_lightGroupId) override {}

  void Draw() override {}

  HRRenderUpdateInfo HaveUpdateNow(int a_maxRaysPerPixel) override;

  void GetFrameBufferHDR(int32_t w, int32_t h, float*   a_out, const wchar_t* a_layerName) override {}
  void GetFrameBufferLDR(int32_t w, int32_t h, int32_t* a_out) override {}

  void GetGBufferLine(int32_t a_lineNumber, HRGBufferPixel* a_lineData, int32_t a_startX, int32_t a_endX, const std::unordered_set<int32_t>& a_shadowCatchers) override {}

//  HRDriverInfo Info() override;
  const HRRenderDeviceInfoListElem* DeviceList() const override { return nullptr; } //#TODO: implement quering GPU info bu glGetString(GL_VENDOR) and e.t.c.
  bool EnableDevice(int32_t id, bool a_enable) override { return true; }

protected:

  std::wstring libPath;

};

void RD_DebugPrint::ClearAll() 
{

}

constexpr static wchar_t foldername[] = L"/debugmeshes";

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

void _hrDebugPrintMesh(const HRMeshDriverInput& a_input, const wchar_t* a_fileName)
{
  std::ofstream fout;
  hr_ofstream_open(fout, a_fileName);

  fout << "vertNum = " << a_input.vertNum << std::endl;
  fout << "triNum  = " << a_input.triNum << std::endl;

  fout << "vpos  = [ " << std::endl;

  for (int i = 0; i < a_input.vertNum; i++)
    fout << "  " << a_input.pos4f[i * 4 + 0] << ", " << a_input.pos4f[i * 4 + 1] << ", " << a_input.pos4f[i * 4 + 2] << ", " << a_input.pos4f[i * 4 + 3] << std::endl;

  fout << "]" << std::endl << std::endl;


  fout << "vnorm  = [ " << std::endl;

  for (int i = 0; i < a_input.vertNum; i++)
    fout << "  " << a_input.norm4f[i * 4 + 0] << ", " << a_input.norm4f[i * 4 + 1] << ", " << a_input.norm4f[i * 4 + 2] << ", " << a_input.norm4f[i * 4 + 3] << std::endl;

  fout << "]" << std::endl << std::endl;

  fout << "vtang  = [ " << std::endl;

  for (int i = 0; i < a_input.vertNum; i++)
    fout << "  " << a_input.tan4f[i * 4 + 0] << ", " << a_input.tan4f[i * 4 + 1] << ", " << a_input.tan4f[i * 4 + 2] << ", " << a_input.tan4f[i * 4 + 3] << std::endl;

  fout << "]" << std::endl << std::endl;


  fout << "vtxcoord  = [ " << std::endl;

  for (int i = 0; i < a_input.vertNum; i++)
    fout << "  " << a_input.texcoord2f[i * 2 + 0] << ", " << a_input.texcoord2f[i * 2 + 1] << std::endl;

  fout << "]" << std::endl << std::endl;


  fout << "indices  = [ " << std::endl;

  for (int i = 0; i < a_input.triNum; i++)
    fout << "  " << a_input.indices[i * 3 + 0] << ", " << a_input.indices[i * 3 + 1] << ", " << a_input.indices[i * 3 + 2] << std::endl;

  fout << "]" << std::endl << std::endl;

  fout << "mindices  = [ " << std::endl;

  for (int i = 0; i < a_input.triNum; i++)
    fout << "  " << a_input.triMatIndices[i] << std::endl;

  fout << "]" << std::endl << std::endl;

  fout.close();
}


void _hrDebugPrintVSGF(const wchar_t* a_fileNameIn, const wchar_t* a_fileNameOut)
{
  HydraGeomData data;
  data.read(a_fileNameIn);

  HRMeshDriverInput mi;

  mi.triNum = data.getIndicesNumber() / 3;
  mi.vertNum = data.getVerticesNumber();
  mi.indices = (int*)data.getTriangleVertexIndicesArray();
  mi.triMatIndices = (int*)data.getTriangleMaterialIndicesArray();

  mi.pos4f = (float*)data.getVertexPositionsFloat4Array();
  mi.norm4f = (float*)data.getVertexNormalsFloat4Array();
  mi.tan4f = (float*)data.getVertexTangentsFloat4Array();
  mi.texcoord2f = (float*)data.getVertexTexcoordFloat2Array();

  _hrDebugPrintMesh(mi, a_fileNameOut);
}

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


IHRRenderDriver* CreateDebugPrint_RenderDriver()
{
  return new RD_DebugPrint;
}
