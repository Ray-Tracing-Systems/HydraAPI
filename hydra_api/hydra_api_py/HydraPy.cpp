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

void hrMeshVertexAttribPointer2fPy(HRMeshRef pMesh, const wchar_t* a_name, std::vector<float> &pointer, int a_stride = 0)
{
  g_vertexAttribf[std::wstring(a_name)] = pointer;
  hrMeshVertexAttribPointer2f(pMesh, a_name, &g_vertexAttribf[std::wstring(a_name)][0], a_stride);
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

void hrMeshInstancePyMatOverride(HRSceneInstRef a_pScn, HRMeshRef a_pMesh, py::array_t<float> &a_mat,
                      py::array_t<int32_t> *a_mmListm = nullptr, int32_t a_mmListSize = 0)
{

  auto mmList = a_mmListm->unchecked<1>();
  auto mat = a_mat.mutable_unchecked<1>();

  if(a_mmListm == nullptr || mmList.size() == 0)
    hrMeshInstance(a_pScn, a_pMesh, mat.mutable_data(0), nullptr, a_mmListSize);
  else
    hrMeshInstance(a_pScn, a_pMesh, mat.mutable_data(0), mmList.data(0), a_mmListSize);
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


void TransformAllInstancesPy(HRSceneInstRef a_pScn, py::array_t<float> &a_mat, bool origin = true)
{
  auto mat = a_mat.mutable_unchecked<1>();

  HRUtils::TransformAllInstances(a_pScn,  mat.mutable_data(0), origin);
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

  py::class_<HRGBufferPixel>(m, "HRGBufferPixel")
          .def_readonly("depth", &HRGBufferPixel::depth)
          .def_readonly("norm", &HRGBufferPixel::norm)
          .def_readonly("rgba", &HRGBufferPixel::rgba)
          .def_readonly("matId", &HRGBufferPixel::matId);

  py::class_<HROpenedMeshInfo>(m, "HROpenedMeshInfo")
          .def_readonly("vertNum", &HROpenedMeshInfo::vertNum)
          .def_readonly("indicesNum", &HROpenedMeshInfo::indicesNum);

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

  m.def("hrInit", &hrInit);
  m.def("hrDestroy", &hrDestroy);
  m.def("hrGetLastError", &hrGetLastError);
  m.def("hrErrorCallerPlace", &hrErrorCallerPlace);
  m.def("hrErrorCallback", &hrErrorCallback);
  m.def("hrSceneLibraryInfo", &hrSceneLibraryInfo);
  m.def("hrSceneLibraryOpen", &hrSceneLibraryOpen);
  m.def("hrTexture2DCreateFromFile", &hrTexture2DCreateFromFile, py::arg("a_fileName"), py::arg("w") = -1, py::arg("h") = -1, py::arg("bpp") = -1);
  m.def("hrTexture2DCreateFromFileDL", &hrTexture2DCreateFromFileDL, py::arg("a_fileName"),  py::arg("w") = -1, py::arg("h") = -1, py::arg("bpp") = -1);
  m.def("hrTexture2DUpdateFromFile", &hrTexture2DUpdateFromFile, py::arg("currentRef"), py::arg("a_fileName"), py::arg("w") = -1, py::arg("h") = -1, py::arg("bpp") = -1);
  //m.def("hrTexture2DCreateFromMemory", &hrTexture2DCreateFromMemory);
  //m.def("hrTexture2DUpdateFromMemory", &hrTexture2DUpdateFromMemory);
  //m.def("hrArray1DCreateFromMemory", &hrArray1DCreateFromMemory);
  //m.def("hrTexture2DCreateFromProcHDR", &hrTexture2DCreateFromProcHDR);
  //m.def("hrTexture2DCreateFromProcLDR", &hrTexture2DCreateFromProcLDR);
  //m.def("hrTextureCreateAdvanced", &hrTextureCreateAdvanced);
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
  m.def("hrMeshVertexAttribPointer1f", &hrMeshVertexAttribPointer1f);
  m.def("hrMeshVertexAttribPointer1fNumPy", &hrMeshVertexAttribPointer1fNumPy, py::arg("pMesh"),
        py::arg("a_name"), py::arg("arr").noconvert(), py::arg("a_stride"));
  m.def("hrMeshVertexAttribPointer2f", &hrMeshVertexAttribPointer2fPy);
  m.def("hrMeshVertexAttribPointer2fNumPy", &hrMeshVertexAttribPointer2fNumPy, py::arg("pMesh"),
        py::arg("a_name"), py::arg("arr").noconvert(), py::arg("a_stride"));
  m.def("hrMeshVertexAttribPointer3f", &hrMeshVertexAttribPointer3f);
  m.def("hrMeshVertexAttribPointer3fNumPy", &hrMeshVertexAttribPointer3fNumPy, py::arg("pMesh"),
        py::arg("a_name"), py::arg("arr").noconvert(), py::arg("a_stride"));
  m.def("hrMeshVertexAttribPointer4f", &hrMeshVertexAttribPointer4fPy);
  m.def("hrMeshVertexAttribPointer4fNumPy", &hrMeshVertexAttribPointer4fNumPy, py::arg("pMesh"),
        py::arg("a_name"), py::arg("arr").noconvert(), py::arg("a_stride"));
  m.def("hrMeshPrimitiveAttribPointer1i", &hrMeshPrimitiveAttribPointer1i);
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
  m.def("hrMeshInstanceMatOverride", &hrMeshInstancePyMatOverride, py::arg("a_pScn"), py::arg("a_pMesh"), py::arg("a_mat").noconvert(),
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
  m.def("hrRenderGetGBufferLine", &hrRenderGetGBufferLine);
  m.def("hrRenderCommand", &hrRenderCommand);
  m.def("hrRenderLogDir", &hrRenderLogDir);
  m.def("hrCommit", &hrCommit, py::arg("a_pScn") = HRSceneInstRef(), py::arg("a_pRender") = HRRenderRef(),  py::arg("a_pCam") = HRCameraRef());
  m.def("hrFlush", &hrFlush, py::arg("a_pScn") = HRSceneInstRef(), py::arg("a_pRender") = HRRenderRef(),  py::arg("a_pCam") = HRCameraRef());
  m.def("WriteMatrix4x4", &WriteMatrix4x4Py, py::arg("a_node"), py::arg("a_attrib_name"), py::arg("a_mat").noconvert());
  m.def("TransformAllInstances", &TransformAllInstancesPy, py::arg("a_pScn"), py::arg("a_mat").noconvert(), py::arg("origin") = true);

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