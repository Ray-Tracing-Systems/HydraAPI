//
// Created by hikawa on 06.07.17.
//

//#include <unordered_map>

#include "pybind11/include/pybind11/pybind11.h"
#include "pybind11/include/pybind11/stl.h"
#include <pybind11/numpy.h>
//#include "pybind11/include/pybind11/stl_bind.h"
#include "HydraAPI.h"
#include "HydraXMLHelpers.h"


namespace py = pybind11;

//PYBIND11_MAKE_OPAQUE(std::vector<float>);
//PYBIND11_MAKE_OPAQUE(std::vector<int>);

std::unordered_map<std::wstring, std::vector<float>> g_vertexAttribf;
std::unordered_map<std::wstring, std::vector<int>> g_vertexAttribi;


struct HRGBufferPixelPy
{
    HRGBufferPixelPy(const HRGBufferPixel &pixel)
    {
      depth = pixel.depth;

      norm.push_back(pixel.norm[0]);
      norm.push_back(pixel.norm[1]);
      norm.push_back(pixel.norm[2]);

      texc.push_back(pixel.texc[0]);
      texc.push_back(pixel.texc[1]);

      rgba.push_back(pixel.rgba[0]);
      rgba.push_back(pixel.rgba[1]);
      rgba.push_back(pixel.rgba[2]);
      rgba.push_back(pixel.rgba[3]);

      shadow = pixel.shadow;
      coverage = pixel.coverage;

      matId = pixel.matId;
      objId = pixel.objId;
      instId = pixel.instId;
    }
    float   depth;
    std::vector<float> norm;
    std::vector<float> texc;
    std::vector<float> rgba;
    float   shadow;
    float   coverage;
    int32_t matId;
    int32_t objId;
    int32_t instId;
};

struct MergeInfoPy
{
  MergeInfoPy(const MergeInfo &info)
  {
    meshRange.push_back(info.meshRange[0]);
    meshRange.push_back(info.meshRange[1]);

    texturesRange.push_back(info.texturesRange[0]);
    texturesRange.push_back(info.texturesRange[1]);

    materialRange.push_back(info.materialRange[0]);
    materialRange.push_back(info.materialRange[1]);

    lightsRange.push_back(info.lightsRange[0]);
    lightsRange.push_back(info.lightsRange[1]);
  }
  MergeInfoPy(){};
  std::vector<int32_t> meshRange;
  std::vector<int32_t> texturesRange;
  std::vector<int32_t> materialRange;
  std::vector<int32_t> lightsRange;
};

void hrMeshOpenPy(HRMeshRef a_pMesh, HR_PRIM_TYPE a_type, HR_OPEN_MODE a_mode)
{
  py::object gc = py::module::import("gc");
  py::object disable = gc.attr("disable");

  g_vertexAttribf.clear();
  g_vertexAttribi.clear();
  hrMeshOpen(a_pMesh, a_type, a_mode);
}


void hrMeshClosePy(HRMeshRef a_pMesh)
{
  py::object gc = py::module::import("gc");
  py::object enable = gc.attr("enable");

  g_vertexAttribf.clear();
  g_vertexAttribi.clear();
  hrMeshClose(a_pMesh);
}

void hrMeshAppendTriangles3Py(HRMeshRef a_pMesh, int indNum, std::vector<int> &indices)
{
  hrMeshAppendTriangles3(a_pMesh, indNum, &indices[0]);
}

void hrMeshVertexAttribPointer4fPy(HRMeshRef pMesh, const wchar_t* a_name, std::vector<float>& pointer, int a_stride = 0)
{
  g_vertexAttribf[std::wstring(a_name)] = pointer;
  hrMeshVertexAttribPointer4f(pMesh, a_name, &g_vertexAttribf[std::wstring(a_name)][0], a_stride);
}

void hrMeshVertexAttribPointer3fPy(HRMeshRef pMesh, const wchar_t* a_name, std::vector<float>& pointer, int a_stride = 0)
{
  g_vertexAttribf[std::wstring(a_name)] = pointer;
  hrMeshVertexAttribPointer3f(pMesh, a_name, &g_vertexAttribf[std::wstring(a_name)][0], a_stride);
}

void hrMeshVertexAttribPointer2fPy(HRMeshRef pMesh, const wchar_t* a_name, std::vector<float> &pointer, int a_stride = 0)
{
  g_vertexAttribf[std::wstring(a_name)] = pointer;
  hrMeshVertexAttribPointer2f(pMesh, a_name, &g_vertexAttribf[std::wstring(a_name)][0], a_stride);
}

void hrMeshVertexAttribPointer1fPy(HRMeshRef pMesh, const wchar_t* a_name, std::vector<float> &pointer, int a_stride = 0)
{
  g_vertexAttribf[std::wstring(a_name)] = pointer;
  hrMeshVertexAttribPointer1f(pMesh, a_name, &g_vertexAttribf[std::wstring(a_name)][0], a_stride);
}

void hrMeshPrimitiveAttribPointer1iPy(HRMeshRef pMesh, const wchar_t* a_name, std::vector<int> &pointer, int a_stride = 0)
{
  g_vertexAttribi[std::wstring(a_name)] = pointer;
  hrMeshPrimitiveAttribPointer1i(pMesh, a_name, &g_vertexAttribi[std::wstring(a_name)][0], a_stride);
}


void hrMeshVertexAttribPointer1fNumPy(HRMeshRef pMesh, const wchar_t* a_name, py::array_t<float> &arr, int a_stride = 0)
{
  auto myArr = arr.unchecked<1>();
  hrMeshVertexAttribPointer1f(pMesh, a_name, myArr.data(0), a_stride);
}

void hrMeshVertexAttribPointer2fNumPy(HRMeshRef pMesh, const wchar_t* a_name, py::array_t<float> &arr, int a_stride = 0)
{
  auto myArr = arr.unchecked<1>();
  hrMeshVertexAttribPointer2f(pMesh, a_name, myArr.data(0), a_stride);
}

void hrMeshVertexAttribPointer3fNumPy(HRMeshRef pMesh, const wchar_t* a_name, py::array_t<float> &arr, int a_stride = 0)
{
  auto myArr = arr.unchecked<1>();
  hrMeshVertexAttribPointer3f(pMesh, a_name, myArr.data(0), a_stride);
}

void hrMeshVertexAttribPointer4fNumPy(HRMeshRef pMesh, const wchar_t* a_name, py::array_t<float> &arr, int a_stride = 0)
{
  auto myArr = arr.unchecked<1>();
  hrMeshVertexAttribPointer4f(pMesh, a_name, myArr.data(0), a_stride);
}

void hrMeshPrimitiveAttribPointer1iNumPy(HRMeshRef pMesh, const wchar_t* a_name, py::array_t<int> &arr, int a_stride = 0)
{
  auto myArr = arr.unchecked<1>();
  hrMeshPrimitiveAttribPointer1i(pMesh, a_name, myArr.data(0), a_stride);
}


void hrMeshAppendTriangles3NumPy(HRMeshRef a_pMesh, int indNum, py::array_t<int32_t> &arr)
{
  auto myArr = arr.unchecked<1>();
  hrMeshAppendTriangles3(a_pMesh, indNum, myArr.data(0));
}

bool hrRenderGetFrameBufferLDR1iNumPy(const HRRenderRef a_pRender, int w, int h, py::array_t<int32_t> &imgData)
{
  auto myArr = imgData.mutable_unchecked<1>();
  return hrRenderGetFrameBufferLDR1i(a_pRender, w, h, myArr.mutable_data(0));
}


void hrMeshInstancePy(HRSceneInstRef a_pScn, HRMeshRef a_pMesh, py::array_t<float> &a_mat)
{

  auto mat = a_mat.mutable_unchecked<1>();

  hrMeshInstance(a_pScn, a_pMesh, mat.mutable_data(0), nullptr, 0);
}

void hrMeshInstancePyRemap(HRSceneInstRef a_pScn, HRMeshRef a_pMesh, py::array_t<float> &a_mat,
                      py::array_t<int32_t> *a_mmListm = nullptr, int32_t a_mmListSize = 0)
{

  auto mat = a_mat.mutable_unchecked<1>();

  if(a_mmListm == nullptr || a_mmListSize == 0)
  {
    hrMeshInstance(a_pScn, a_pMesh, mat.mutable_data(0), nullptr, 0);
  }
  else
  {
    auto mmList = a_mmListm->unchecked<1>();
    hrMeshInstance(a_pScn, a_pMesh, mat.mutable_data(0), mmList.data(0), a_mmListSize);
  }
}

void hrLightInstancePy(HRSceneInstRef pScn, HRLightRef pLight, py::array_t<float> &m)
{
  auto mat = m.mutable_unchecked<1>();
  hrLightInstance(pScn, pLight, mat.mutable_data(0));
}

void WriteMatrix4x4Py(pugi::xml_node a_node, const wchar_t* a_attrib_name, py::array_t<float> &a_mat)
{
  auto mat = a_mat.mutable_unchecked<1>();

  HydraXMLHelpers::WriteMatrix4x4(a_node, a_attrib_name, mat.mutable_data(0));
}

HRUtils::BBox InstanceSceneIntoScenePy(HRSceneInstRef a_scnFrom, HRSceneInstRef a_scnTo, py::array_t<float> &a_mat, bool origin = true)
{
  auto mat = a_mat.mutable_unchecked<1>();

  return HRUtils::InstanceSceneIntoScene(a_scnFrom, a_scnTo,  mat.mutable_data(0), origin, nullptr, 0);
}

HRUtils::BBox InstanceSceneIntoSceneRemapOverridePy(HRSceneInstRef a_scnFrom, HRSceneInstRef a_scnTo, py::array_t<float> &a_mat,
                                           bool origin = true, py::array_t<int32_t> *remap_override = nullptr)
{
  auto mat = a_mat.mutable_unchecked<1>();
  auto remap_list = remap_override->unchecked<1>();

  return HRUtils::InstanceSceneIntoScene(a_scnFrom, a_scnTo,  mat.mutable_data(0), origin, remap_list.data(0),
                                  int32_t(remap_list.size()));
}

std::vector<HRGBufferPixelPy> hrRenderGetGBufferLinePy(HRRenderRef a_pRender, int w, int y)
{
  std::vector<HRGBufferPixel> buffer_cpp(w);
  hrRenderGetGBufferLine(a_pRender, y, &buffer_cpp[0], 0, w);
  std::vector<HRGBufferPixelPy> buffer_py;
  buffer_py.insert(buffer_py.end(), buffer_cpp.begin(), buffer_cpp.end());
  return buffer_py;
}

std::vector<float> getRandomPointsOnMeshPy(HRMeshRef mesh_ref, uint32_t n_points,
                                           bool tri_area_weighted = true, uint32_t seed = 0u)
{
  std::vector<float> output;
  if(n_points > 0)
  {
    output.resize(n_points * 3);
    HRUtils::getRandomPointsOnMesh(mesh_ref, output.data(), n_points, tri_area_weighted, seed);

  }

  return output;
}

HRSceneInstRef MergeLibraryIntoLibraryPy(const wchar_t* a_libPath, bool mergeLights = false, bool copyScene = false,
                                         const wchar_t* a_stateFileName = L"", MergeInfoPy pInfo = MergeInfoPy())
{
  MergeInfo info;
  bool is_null = false;
  if(pInfo.lightsRange.size() >= 2)
  {
    info.lightsRange[0] = pInfo.lightsRange[0];
    info.lightsRange[1] = pInfo.lightsRange[1];

  }
  else
    is_null = true;

  if(pInfo.materialRange.size() >= 2)
  {
    info.materialRange[0] = pInfo.materialRange[0];
    info.materialRange[1] = pInfo.materialRange[1];
  }
  else
    is_null = true;

  if(pInfo.meshRange.size() >= 2)
  {
    info.meshRange[0] = pInfo.meshRange[0];
    info.meshRange[1] = pInfo.meshRange[1];
  }
  else
    is_null = true;

  if(pInfo.texturesRange.size() >= 2)
  {
    info.texturesRange[0] = pInfo.texturesRange[0];
    info.texturesRange[1] = pInfo.texturesRange[1];
  }
  else
    is_null = true;

  if(!is_null)
    return MergeLibraryIntoLibrary(a_libPath, mergeLights, copyScene, a_stateFileName, &info);
  else
    return MergeLibraryIntoLibrary(a_libPath, mergeLights, copyScene, a_stateFileName, nullptr);
}


PYBIND11_MODULE(hydraPy, m)
{

  //py::bind_vector<std::vector<float>>(m, "VectorFloat");
  //py::bind_vector<std::vector<int>>(m, "VectorInt");

  py::class_<HRMeshRef>(m, "HRMeshRef")
          .def(py::init<>())
          .def_readwrite("id", &HRMeshRef::id);
  py::class_<HRLightRef>(m, "HRLightRef")
          .def(py::init<>())
          .def_readwrite("id", &HRLightRef::id);
  py::class_<HRMaterialRef>(m, "HRMaterialRef")
          .def(py::init<>())
          .def_readwrite("id", &HRMaterialRef::id);
  py::class_<HRCameraRef>(m, "HRCameraRef")
          .def(py::init<>())
          .def_readwrite("id", &HRCameraRef::id);
  py::class_<HRTextureNodeRef>(m, "HRTextureNodeRef")
          .def(py::init<>())
          .def_readonly("id", &HRTextureNodeRef::id);
  py::class_<HRSceneInstRef>(m, "HRSceneInstRef")
          .def(py::init<>())
          .def_readwrite("id", &HRSceneInstRef::id);
  py::class_<HRRenderRef>(m, "HRRenderRef")
          .def(py::init<>())
          .def_readwrite("id", &HRRenderRef::id);

  py::class_<HRSceneLibraryInfo>(m, "HRSceneLibraryInfo")
          .def_readwrite("texturesNum", &HRSceneLibraryInfo::texturesNum)
          .def_readwrite("materialsNum", &HRSceneLibraryInfo::materialsNum)
          .def_readwrite("meshesNum", &HRSceneLibraryInfo::meshesNum)
          .def_readwrite("camerasNum", &HRSceneLibraryInfo::camerasNum)
          .def_readwrite("scenesNum", &HRSceneLibraryInfo::scenesNum)
          .def_readwrite("renderDriversNum", &HRSceneLibraryInfo::renderDriversNum);

  py::class_<HRInitInfo>(m, "HRInitInfo")
      .def(py::init<>())
      .def_readwrite("copyTexturesToLocalFolder", &HRInitInfo::copyTexturesToLocalFolder)
      .def_readwrite("localDataPath", &HRInitInfo::localDataPath)
      .def_readwrite("sortMaterialIndices", &HRInitInfo::sortMaterialIndices)
      .def_readwrite("computeMeshBBoxes", &HRInitInfo::computeMeshBBoxes)
      .def_readwrite("vbSize", &HRInitInfo::vbSize);

  py::class_<MergeInfoPy>(m, "MergeInfo")
      .def(py::init<>())
      .def_readwrite("meshRange", &MergeInfoPy::meshRange)
      .def_readwrite("texturesRange", &MergeInfoPy::texturesRange)
      .def_readwrite("materialRange", &MergeInfoPy::materialRange)
      .def_readwrite("lightsRange", &MergeInfoPy::lightsRange);


  py::class_<HRRenderDeviceInfoListElem>(m, "HRRenderDeviceInfoListElem")
          .def_readonly("name", &HRRenderDeviceInfoListElem::name)
          .def_readonly("driver", &HRRenderDeviceInfoListElem::driver)
          .def_readonly("id", &HRRenderDeviceInfoListElem::id)
          .def_readonly("isCPU", &HRRenderDeviceInfoListElem::isCPU)
          .def_readwrite("isEnabled", &HRRenderDeviceInfoListElem::isEnabled)
          .def_readwrite("next", &HRRenderDeviceInfoListElem::next);

  py::class_<HRRenderUpdateInfo>(m, "HRRenderUpdateInfo")
          .def(py::init<>())
          .def_readwrite("haveUpdateFB", &HRRenderUpdateInfo::haveUpdateFB)
          .def_readwrite("haveUpdateMSG", &HRRenderUpdateInfo::haveUpdateMSG)
          .def_readwrite("finalUpdate", &HRRenderUpdateInfo::finalUpdate)
          .def_readwrite("progress", &HRRenderUpdateInfo::progress)
          .def_readwrite("msg", &HRRenderUpdateInfo::msg);

  /*
   *   float   depth;
  float   norm[3];
  float   texc[2];
  float   rgba[4];
  float   shadow;
  float   coverage;
  int32_t matId;
  int32_t objId;
  int32_t instId;
   */
  py::class_<HRGBufferPixelPy>(m, "HRGBufferPixelPy")
          .def_readonly("depth", &HRGBufferPixelPy::depth)
//          .def_property("norm", &HRGBufferPixel::get_norm, &HRGBufferPixel::set_norm)
//          .def_property("texc", &HRGBufferPixel::get_texc, &HRGBufferPixel::set_texc)
//          .def_property("rgba", &HRGBufferPixel::get_rgba, &HRGBufferPixel::set_rgba)
          .def_readonly("norm", &HRGBufferPixelPy::norm)
          .def_readonly("texc", &HRGBufferPixelPy::texc)
          .def_readonly("rgba", &HRGBufferPixelPy::rgba)
          .def_readonly("shadow", &HRGBufferPixelPy::shadow)
          .def_readonly("coverage", &HRGBufferPixelPy::coverage)
          .def_readonly("matId", &HRGBufferPixelPy::matId)
          .def_readonly("objId", &HRGBufferPixelPy::objId)
          .def_readonly("instId", &HRGBufferPixelPy::instId);

  py::class_<HRUtils::BBox>(m, "BBox")
          .def_readonly("x_min", &HRUtils::BBox::x_min)
          .def_readonly("x_max", &HRUtils::BBox::x_max)
          .def_readonly("y_min", &HRUtils::BBox::y_min)
          .def_readonly("y_max", &HRUtils::BBox::y_max)
          .def_readonly("z_min", &HRUtils::BBox::z_min)
          .def_readonly("z_max", &HRUtils::BBox::z_max);

//  py::class_<HROpenedMeshInfo>(m, "HROpenedMeshInfo")
//          .def_readonly("vertNum", &HROpenedMeshInfo::vertNum)
//          .def_readonly("indicesNum", &HROpenedMeshInfo::indicesNum);

  py::enum_<HR_OPEN_MODE>(m, "HR_OPEN_MODE", py::arithmetic())
          .value("HR_OPEN_EXISTING", HR_OPEN_EXISTING)
          .value("HR_WRITE_DISCARD", HR_WRITE_DISCARD)
          .value("HR_OPEN_READ_ONLY", HR_OPEN_READ_ONLY)
          .export_values();


  py::enum_<HR_PRIM_TYPE>(m, "HR_PRIM_TYPE", py::arithmetic())
          .value("HR_TRIANGLE_LIST", HR_TRIANGLE_LIST)
          .value("HR_TRIANGLE_IND3", HR_TRIANGLE_IND3)
          .value("HR_TRIANGLE_IND9", HR_TRIANGLE_IND9)
          .value("HR_TRIANGLE_IND12", HR_TRIANGLE_IND12)
          .export_values();

  py::enum_<HR_SEVERITY_LEVEL>(m, "HR_SEVERITY_LEVEL", py::arithmetic())
          .value("HR_SEVERITY_DEBUG", HR_SEVERITY_DEBUG)
          .value("HR_SEVERITY_INFO", HR_SEVERITY_INFO)
          .value("HR_SEVERITY_WARNING", HR_SEVERITY_WARNING)
          .value("HR_SEVERITY_ERROR", HR_SEVERITY_ERROR)
          .value("HR_SEVERITY_CRITICAL_ERROR", HR_SEVERITY_CRITICAL_ERROR)
          .export_values();

 // m.def("hrInit", &hrInit);
//  m.def("hrDestroy", &hrDestroy);
  m.def("hrGetLastError", &hrGetLastError);
  m.def("hrErrorCallerPlace", &hrErrorCallerPlace);
  m.def("hrSceneLibraryInfo", &hrSceneLibraryInfo);
  m.def("hrSceneLibraryOpen", &hrSceneLibraryOpen);
  m.def("hrTexture2DCreateFromFile", &hrTexture2DCreateFromFile, py::arg("a_fileName"), py::arg("w") = -1,
        py::arg("h") = -1, py::arg("bpp") = -1);
  m.def("hrTexture2DCreateFromFileDL", &hrTexture2DCreateFromFileDL, py::arg("a_fileName"),  py::arg("w") = -1,
        py::arg("h") = -1, py::arg("bpp") = -1, py::arg("a_copyFileToLocalData") = false);
  m.def("hrTexture2DUpdateFromFile", &hrTexture2DUpdateFromFile, py::arg("currentRef"), py::arg("a_fileName"),
        py::arg("w") = -1, py::arg("h") = -1, py::arg("bpp") = -1);
  //m.def("hrTexture2DCreateFromMemory", &hrTexture2DCreateFromMemory);
  //m.def("hrTexture2DUpdateFromMemory", &hrTexture2DUpdateFromMemory);
  //m.def("hrArray1DCreateFromMemory", &hrArray1DCreateFromMemory);
  //m.def("hrTexture2DCreateFromProcHDR", &hrTexture2DCreateFromProcHDR);
  //m.def("hrTexture2DCreateFromProcLDR", &hrTexture2DCreateFromProcLDR);
  m.def("hrTextureCreateAdvanced", &hrTextureCreateAdvanced);
  m.def("hrTextureNodeOpen", &hrTextureNodeOpen);
  m.def("hrTextureNodeClose", &hrTextureNodeClose);
  m.def("hrTextureBind", &hrTextureBind);
  m.def("hrTextureParamNode", &hrTextureParamNode);
  m.def("hrMaterialCreateBlend", &hrMaterialCreateBlend);
  m.def("hrMaterialCreate", &hrMaterialCreate);
  m.def("hrMaterialOpen", &hrMaterialOpen);
  m.def("hrMaterialParamNode", &hrMaterialParamNode);
  m.def("hrMaterialClose", &hrMaterialClose);
  m.def("hrFindMaterialByName", &hrFindMaterialByName);
  //m.def("hrMaterialCreateBRDFLeaf", &hrMaterialCreateBRDFLeaf);
  m.def("hrLightCreate", &hrLightCreate);
  m.def("hrLightOpen", &hrLightOpen);
  m.def("hrLightClose", &hrLightClose);
  m.def("hrLightParamNode", &hrLightParamNode);
  m.def("hrFindLightByName", &hrFindLightByName);
  m.def("hrCameraCreate", &hrCameraCreate);
  m.def("hrCameraOpen", &hrCameraOpen);
  m.def("hrCameraClose", &hrCameraClose);
  m.def("hrCameraParamNode", &hrCameraParamNode);
  m.def("hrFindCameraByName", &hrFindCameraByName);
  m.def("hrMeshCreate", &hrMeshCreate);
  m.def("hrMeshCreateFromFileDL", &hrMeshCreateFromFileDL);
  m.def("hrMeshOpen", &hrMeshOpenPy);
  m.def("hrMeshClose", &hrMeshClosePy);
  m.def("hrMeshVertexAttribPointer1f", &hrMeshVertexAttribPointer1fPy);
  m.def("hrMeshVertexAttribPointer1fNumPy", &hrMeshVertexAttribPointer1fNumPy, py::arg("pMesh"),
        py::arg("a_name"), py::arg("arr").noconvert(), py::arg("a_stride"));
  m.def("hrMeshVertexAttribPointer2f", &hrMeshVertexAttribPointer2fPy);
  m.def("hrMeshVertexAttribPointer2fNumPy", &hrMeshVertexAttribPointer2fNumPy, py::arg("pMesh"),
        py::arg("a_name"), py::arg("arr").noconvert(), py::arg("a_stride"));
  m.def("hrMeshVertexAttribPointer3f", &hrMeshVertexAttribPointer3fPy);
  m.def("hrMeshVertexAttribPointer3fNumPy", &hrMeshVertexAttribPointer3fNumPy, py::arg("pMesh"),
        py::arg("a_name"), py::arg("arr").noconvert(), py::arg("a_stride"));
  m.def("hrMeshVertexAttribPointer4f", &hrMeshVertexAttribPointer4fPy);
  m.def("hrMeshVertexAttribPointer4fNumPy", &hrMeshVertexAttribPointer4fNumPy, py::arg("pMesh"),
        py::arg("a_name"), py::arg("arr").noconvert(), py::arg("a_stride"));
  m.def("hrMeshPrimitiveAttribPointer1i", &hrMeshPrimitiveAttribPointer1iPy);
  m.def("hrMeshPrimitiveAttribPointer1iNumPy", &hrMeshPrimitiveAttribPointer1iNumPy, py::arg("pMesh"),
        py::arg("a_name"), py::arg("arr").noconvert(), py::arg("a_stride"));

  m.def("hrMeshMaterialId", &hrMeshMaterialId);
  m.def("hrMeshAppendTriangles3", &hrMeshAppendTriangles3Py);
  m.def("hrMeshAppendTriangles3NumPy", &hrMeshAppendTriangles3NumPy, py::arg("a_pMesh"), py::arg("indNum"), py::arg("arr").noconvert());
  m.def("hrMeshGetAttribPointer", &hrMeshGetAttribPointer);
  m.def("hrMeshGetPrimitiveAttribPointer", &hrMeshGetPrimitiveAttribPointer);
  m.def("hrMeshGetInfo", &hrMeshGetInfo);
  m.def("hrMeshParamNode", &hrMeshParamNode);
  m.def("hrSceneCreate", &hrSceneCreate);
  m.def("hrSceneOpen", &hrSceneOpen);
  m.def("hrSceneClose", &hrSceneClose);
  m.def("hrMeshInstance", &hrMeshInstancePy, py::arg("a_pScn"), py::arg("a_pMesh"), py::arg("a_mat").noconvert());
  m.def("hrMeshInstanceRemap", &hrMeshInstancePyRemap, py::arg("a_pScn"), py::arg("a_pMesh"), py::arg("a_mat").noconvert(),
        py::arg("a_mmListm").noconvert() = (py::array_t<int32_t>*)nullptr,  py::arg("a_mmListSize") = 0);

  m.def("hrLightInstance", &hrLightInstancePy);
  m.def("hrRenderCreate", &hrRenderCreate, py::arg("a_className"), py::arg("a_flags") = "");
  m.def("hrRenderOpen", &hrRenderOpen);
  m.def("hrRenderClose", &hrRenderClose);
  m.def("hrRenderParamNode", &hrRenderParamNode);
  m.def("hrRenderHaveUpdate", &hrRenderHaveUpdate);
  m.def("hrRenderEnableDevice", &hrRenderEnableDevice);
  //m.def("hrRenderGetFrameBufferHDR4f", &hrRenderGetFrameBufferHDR4f);
  m.def("hrRenderGetFrameBufferLDR1i", &hrRenderGetFrameBufferLDR1iNumPy);
  m.def("hrRenderGetFrameBufferLDR1i", &hrRenderGetFrameBufferLDR1i);
  m.def("hrRenderSaveFrameBufferLDR", &hrRenderSaveFrameBufferLDR);
  m.def("hrRenderSaveGBufferLayerLDR", &hrRenderSaveGBufferLayerLDR, py::arg("a_pRender"), py::arg("a_outFileName"), py::arg("a_layerName"),
        py::arg("a_palette") = (const int*)nullptr, py::arg("a_paletteSize") = 0);
  m.def("hrRenderSaveFrameBufferHDR", &hrRenderSaveFrameBufferHDR);
  m.def("hrRenderGetGBufferLine", &hrRenderGetGBufferLinePy, py::arg("a_pRender"), py::arg("w"), py::arg("y"));
  m.def("hrRenderCommand", &hrRenderCommand);
  m.def("hrRenderLogDir", &hrRenderLogDir);
  m.def("hrCommit", &hrCommit, py::arg("a_pScn") = HRSceneInstRef(), py::arg("a_pRender") = HRRenderRef(),  py::arg("a_pCam") = HRCameraRef());
  m.def("hrFlush", &hrFlush, py::arg("a_pScn") = HRSceneInstRef(), py::arg("a_pRender") = HRRenderRef(),  py::arg("a_pCam") = HRCameraRef());
  m.def("WriteMatrix4x4", &WriteMatrix4x4Py, py::arg("a_node"), py::arg("a_attrib_name"), py::arg("a_mat").noconvert());

  m.def("GetMaterialNameToIdMap", &HydraXMLHelpers::GetMaterialNameToIdMap);

  m.def("InstanceSceneIntoScene", &InstanceSceneIntoScenePy, py::arg("a_scnFrom"), py::arg("a_scnTo"), py::arg("a_mat").noconvert(),
        py::arg("origin") = true);
  m.def("InstanceSceneIntoSceneRemapOverride", &InstanceSceneIntoSceneRemapOverridePy, py::arg("a_scnFrom"), py::arg("a_scnTo"),
        py::arg("a_mat").noconvert(), py::arg("origin") = true, py::arg("remap_override").noconvert() = (py::array_t<int32_t>*)nullptr);


  m.def("MergeLibraryIntoLibrary", &MergeLibraryIntoLibraryPy, py::arg("a_libPath"), py::arg("mergeLights") = false,
      py::arg("copyScene") = false, py::arg("a_stateFileName") = "", py::arg("pInfo") = MergeInfoPy());
  m.def("MergeOneMaterialIntoLibrary", &HRUtils::MergeOneMaterialIntoLibrary, py::arg("a_libPath"), py::arg("a_matName"), py::arg("a_matId") = -1);
  m.def("MergeOneMeshIntoLibrary", &HRUtils::MergeOneMeshIntoLibrary);
  m.def("MergeOneLightIntoLibrary", &HRUtils::MergeOneLightIntoLibrary);
  m.def("MergeOneTextureIntoLibrary", &HRUtils::MergeOneTextureIntoLibrary, py::arg("a_libPath"), py::arg("a_texName"), py::arg("a_texId") = -1);

  m.def("getRandomPointsOnMesh", &getRandomPointsOnMeshPy, py::arg("mesh_ref"), py::arg("n_points") = 1,
        py::arg("tri_area_weighted") = false, py::arg("seed") = 0u);

  m.def("GetMeshBBox", &HRUtils::GetMeshBBox);


  py::class_<pugi::xml_node>(m, "xml_node")
          .def("force_child", &pugi::xml_node::force_child)
          .def("force_attribute", &pugi::xml_node::force_attribute)
          .def("append_attribute", &pugi::xml_node::append_attribute)
          .def("append_child", py::overload_cast<const wchar_t*>(&pugi::xml_node::append_child))
          .def("attribute", py::overload_cast<const wchar_t*>(&pugi::xml_node::attribute, py::const_))
          .def("child", &pugi::xml_node::child)
          .def("text", &pugi::xml_node::text);

  py::class_<pugi::xml_attribute>(m, "xml_attribute")
          .def("set_value", py::overload_cast<int>(&pugi::xml_attribute::set_value))
          .def("set_value", py::overload_cast<unsigned int>(&pugi::xml_attribute::set_value))
          .def("set_value", py::overload_cast<long>(&pugi::xml_attribute::set_value))
          .def("set_value", py::overload_cast<unsigned long>(&pugi::xml_attribute::set_value))
          .def("set_value", py::overload_cast<double>(&pugi::xml_attribute::set_value))
          .def("set_value", py::overload_cast<float>(&pugi::xml_attribute::set_value))
          .def("set_value", py::overload_cast<bool>(&pugi::xml_attribute::set_value))
          .def("set_value", py::overload_cast<const wchar_t*>(&pugi::xml_attribute::set_value));

  py::class_<pugi::xml_text>(m, "xml_text")
          .def("set", py::overload_cast<int>(&pugi::xml_text::set))
          .def("set", py::overload_cast<unsigned int>(&pugi::xml_text::set))
          .def("set", py::overload_cast<long>(&pugi::xml_text::set))
          .def("set", py::overload_cast<unsigned long>(&pugi::xml_text::set))
          .def("set", py::overload_cast<double>(&pugi::xml_text::set))
          .def("set", py::overload_cast<float>(&pugi::xml_text::set))
          .def("set", py::overload_cast<bool>(&pugi::xml_text::set))
          .def("set", py::overload_cast<const wchar_t*>(&pugi::xml_text::set));

}