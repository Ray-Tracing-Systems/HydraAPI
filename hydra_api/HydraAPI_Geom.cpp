#include "HydraAPI.h"
#include "HydraInternal.h"

#include <memory>
#include <vector>
#include <array>
#include <string>
#include <map>

#include <fstream>
#include <sstream>
#include <iomanip>
#include <set>
#include <algorithm>

#include "LiteMath.h"
using namespace LiteMath;

#include "HydraObjectManager.h"
#include "HydraVSGFExport.h"
#include "HydraXMLHelpers.h"
#include "HydraTextureUtils.h"
#include "HydraLegacyUtils.h"

#include "HydraVSGFCompress.h"
#include "HydraXMLVerify.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"


extern std::wstring      g_lastError;
extern std::wstring      g_lastErrorCallerPlace;
extern HRObjectManager   g_objManager;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct VSGFChunkInfo
{
  uint64_t vertNum;
  uint64_t indNum;
  uint64_t dataBytes;

  uint64_t offsetPos;
  uint64_t offsetNorm;
  uint64_t offsetTexc;
  uint64_t offsetTang;
  uint64_t offsetInd;
  uint64_t offsetMInd;
};

void FillXMLFromMeshImpl(pugi::xml_node nodeXml, std::shared_ptr<IHRMesh> a_pImpl, bool dlLoad)
{
  ////
  //
  size_t chunkId = a_pImpl->chunkId();
  std::wstring location = L"unknown";
  ChunkPointer chunk;

  if(chunkId > g_objManager.scnData.m_vbCache.size())
  {
    chunk.sizeInBytes = 0;
  }
  else
  {
    chunk = g_objManager.scnData.m_vbCache.chunk_at(chunkId);
    location = ChunkName(chunk);
  }

  VSGFChunkInfo info;

  info.vertNum    = a_pImpl->vertNum();
  info.indNum     = a_pImpl->indNum();
  info.offsetPos  = a_pImpl->offset(L"pos");
  info.offsetNorm = a_pImpl->offset(L"norm");
  info.offsetTexc = a_pImpl->offset(L"texc");
  info.offsetTang = a_pImpl->offset(L"tan");
  info.offsetInd  = a_pImpl->offset(L"ind");
  info.offsetMInd = a_pImpl->offset(L"mind");
  info.dataBytes  = chunk.sizeInBytes;

  clear_node_childs(nodeXml);

  nodeXml.attribute(L"bytesize").set_value(info.dataBytes);
  if(!dlLoad) 
    g_objManager.SetLoc(nodeXml, location);
  nodeXml.attribute(L"offset").set_value(L"0");
  nodeXml.attribute(L"vertNum").set_value(info.vertNum);
  nodeXml.attribute(L"triNum").set_value(info.indNum / 3);

  HydraXMLHelpers::WriteBBox(nodeXml, a_pImpl->getBBox());

  // (1) fill common attributes
  //
  pugi::xml_node positionArrayNode = nodeXml.append_child(L"positions");
  {
    positionArrayNode.append_attribute(L"type").set_value(L"array4f");
    positionArrayNode.append_attribute(L"bytesize").set_value(info.vertNum * sizeof(float) * 4);
    positionArrayNode.append_attribute(L"offset").set_value(info.offsetPos);
    positionArrayNode.append_attribute(L"apply").set_value(L"vertex");
  }

  pugi::xml_node normalsArrayNode = nodeXml.append_child(L"normals");
  {
    normalsArrayNode.append_attribute(L"type").set_value(L"array4f");
    normalsArrayNode.append_attribute(L"bytesize").set_value(info.vertNum * sizeof(float) * 4);
    normalsArrayNode.append_attribute(L"offset").set_value(info.offsetNorm);
    normalsArrayNode.append_attribute(L"apply").set_value(L"vertex");
  }
  
  pugi::xml_node tangentsArrayNode = nodeXml.append_child(L"tangents");
  {
    tangentsArrayNode.append_attribute(L"type").set_value(L"array4f");
    tangentsArrayNode.append_attribute(L"bytesize").set_value(info.vertNum * sizeof(float) * 4);
    tangentsArrayNode.append_attribute(L"offset").set_value(info.offsetTang);
    tangentsArrayNode.append_attribute(L"apply").set_value(L"vertex");
  }

  pugi::xml_node texcoordArrayNode = nodeXml.append_child(L"texcoords");
  {
    texcoordArrayNode.append_attribute(L"type").set_value(L"array2f");
    texcoordArrayNode.append_attribute(L"bytesize").set_value(info.vertNum * sizeof(float) * 2);
    texcoordArrayNode.append_attribute(L"offset").set_value(info.offsetTexc);
    texcoordArrayNode.append_attribute(L"apply").set_value(L"vertex");
  }

  pugi::xml_node indicesArrayNode = nodeXml.append_child(L"indices");
  {
    indicesArrayNode.append_attribute(L"type").set_value(L"array1i");
    indicesArrayNode.append_attribute(L"bytesize").set_value(info.indNum * sizeof(int));
    indicesArrayNode.append_attribute(L"offset").set_value(info.offsetInd);
    indicesArrayNode.append_attribute(L"apply").set_value(L"tlist");
  }

  pugi::xml_node mindicesArrayNode = nodeXml.append_child(L"matindices");
  {
    mindicesArrayNode.append_attribute(L"type").set_value(L"array1i");
    mindicesArrayNode.append_attribute(L"bytesize").set_value(info.indNum * sizeof(int) / 3);
    mindicesArrayNode.append_attribute(L"offset").set_value(info.offsetMInd);
    mindicesArrayNode.append_attribute(L"apply").set_value(L"primitive");
  }

  // (2) fill custom attributes
  //
  for (const auto& arr : a_pImpl->GetOffsAndSizeForAttrs())
  {
    pugi::xml_node arrayNode = nodeXml.append_child(arr.first.c_str());
    const std::wstring& name = std::get<0>(arr.second);

    arrayNode.append_attribute(L"type").set_value(name.c_str());
    arrayNode.append_attribute(L"bytesize").set_value(std::get<2>(arr.second));
    arrayNode.append_attribute(L"offset").set_value(std::get<1>(arr.second));

    if(std::get<3>(arr.second) == 1)
      arrayNode.append_attribute(L"apply").set_value(L"primitive");
    else
      arrayNode.append_attribute(L"apply").set_value(L"vertex");
  }

}


HAPI HRMeshRef hrMeshCreate(const wchar_t* a_objectName)
{
  HRMeshRef ref;
  ref.id = HR_IDType(g_objManager.scnData.meshes.size());

  HRMesh mesh;
  mesh.name = std::wstring(a_objectName);
  mesh.id = ref.id;
  g_objManager.scnData.meshes.push_back(mesh);

  pugi::xml_node nodeXml = g_objManager.geom_lib_append_child();

  std::wstring idStr = ToWString(ref.id);
  std::wstring name2 = std::wstring(L"mesh#") + idStr;

  if (a_objectName == nullptr || std::wstring(a_objectName) == L"")
    a_objectName = name2.c_str();

	nodeXml.append_attribute(L"id").set_value(idStr.c_str());
  nodeXml.append_attribute(L"name").set_value(a_objectName);
  nodeXml.append_attribute(L"type").set_value(L"vsgf");
  nodeXml.append_attribute(L"bytesize").set_value(L"0");
  nodeXml.append_attribute(L"loc").set_value(L"unknown");
  nodeXml.append_attribute(L"offset").set_value(L"0");
  nodeXml.append_attribute(L"vertNum").set_value(L"0");
  nodeXml.append_attribute(L"triNum").set_value(L"0");
  nodeXml.append_attribute(L"dl").set_value(L"0");
  nodeXml.append_attribute(L"path").set_value(L"");

  g_objManager.scnData.meshes[ref.id].update(nodeXml);
  g_objManager.scnData.meshes[ref.id].id = ref.id;

  return ref;
}

/**** Some utilities for checking empty values and formatting wstrings ****/

inline std::wstring fthree2ws(const float float3_array[3])
{
  std::ostringstream strs;
  strs << float3_array[0] << " " << float3_array[1] << " " << float3_array[2];
  std::wstring str = s2ws(strs.str());
  return str;
}

inline bool f3filled(const float float3_array[3])
{
  for(int i = 0; i < 3; ++i){
    if(float3_array[i] > 0)
      return true;
  }
  return false;
}

inline bool f1filled(const float float1)
{
  return (float1 > 0);
}

inline bool s1filled(const std::string str)
{
  return str.compare("");
}

HAPI HRMeshRef _hrMeshCreateFromObjMerged(const wchar_t* a_objectName, HRModelLoadInfo a_modelInfo)
{
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;

  /*******************************************    Reading the .obj file    ********************************************/
  auto pathS = ws2s(a_objectName);
  // Getting the path to the .obj file
  int strLength = pathS.size() - pathS.substr(pathS.find_last_of("/")).size();
  std::string upToLastSlash = pathS.substr(0, strLength);
  bool res = false;

  // Check for mtl file in the same folder as .obj file in case 'mtlRelativePath' is not provided
  if(a_modelInfo.mtlRelativePath == nullptr)
  {
    res = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, pathS.c_str(), upToLastSlash.c_str());
  }
  else
  {
    res = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, pathS.c_str(), ws2s(a_modelInfo.mtlRelativePath).c_str());
    upToLastSlash = ws2s(a_modelInfo.mtlRelativePath);
  }
  if(!res)
  {
    HrPrint(HR_SEVERITY_ERROR, L"_hrMeshCreateFromObjMerged, failed to load obj file ", a_objectName);
    return HRMeshRef();
  }

  // The number of indices
  std::vector<size_t> shape_indices_number;
  size_t cumulative_indices_number = 0;
  shape_indices_number.push_back(0);
  for (size_t s = 0; s < shapes.size(); s++) {
    shape_indices_number.push_back(shapes[s].mesh.indices.size());
    cumulative_indices_number += shapes[s].mesh.indices.size();
  }

  /*******************************************      Parsing materials      ********************************************/
  bool ifMaterialsProvided = false;
  std::vector<HRMaterialRef> h_materials;
  // Check if there are materials associated with the .obj file
  // If not, set material id to '0' for the whole mesh
  if(a_modelInfo.useMaterial)
    if(materials.size() != 0)
      ifMaterialsProvided = true;
    else
      HrPrint(HR_SEVERITY_ERROR, L"Materials not found for: ", a_objectName);

  if(ifMaterialsProvided) {
    for (int m = 0; m < materials.size(); ++m) {

      /// Checking the material properties
      // Just setting [0.0, 0.0, 0.0] is case of empty variable
      bool ifDiffuseRGB = f3filled(materials.at(m).diffuse);
      bool ifDiffuseTexture = s1filled(materials.at(m).diffuse_texname);
      bool ifSpecularRGB = f3filled(materials.at(m).specular);


      HRMaterialRef current_material = hrMaterialCreate(s2ws(materials.at(m).name).c_str());

      hrMaterialOpen(current_material, HR_WRITE_DISCARD);
      {
        pugi::xml_node matNode = hrMaterialParamNode(current_material);

        if (ifDiffuseRGB || ifDiffuseTexture) {
          pugi::xml_node diff = matNode.append_child(L"diffuse");
          diff.append_attribute(L"brdf_type").set_value(L"lambert");
          auto diffColor = diff.append_child(L"color");
          if (ifDiffuseRGB) {
            diffColor.append_attribute(L"val") = fthree2ws(materials.at(m).diffuse).c_str();
          }
          if (ifDiffuseTexture) {
            diffColor.append_attribute(L"tex_apply_mode").set_value(L"multiply");
            std::wstring wpath = s2ws(upToLastSlash + "/" + materials.at(
                    m).diffuse_texname);//.substr(1, materials.at(m).diffuse_texname.size() - 1));
            HRTextureNodeRef diffuse_texture = hrTexture2DCreateFromFile(wpath.c_str());
            auto texNode = hrTextureBind(diffuse_texture, diffColor);
            texNode.append_attribute(L"matrix");
            float samplerMatrix[16] = {1, 0, 0, 0,
                                       0, 1, 0, 0,
                                       0, 0, 1, 0,
                                       0, 0, 0, 1};
            texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
            texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
            texNode.append_attribute(L"input_gamma").set_value(2.2f);
            texNode.append_attribute(L"input_alpha").set_value(L"rgb");

            HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
          }

        }

        if (ifSpecularRGB) {
          pugi::xml_node refl = matNode.append_child(L"reflectivity");
          refl.append_attribute(L"brdf_type").set_value(L"phong");
          refl.append_child(L"color").append_attribute(L"val") = fthree2ws(materials.at(m).specular).c_str();
          refl.append_child(L"glossiness").append_attribute(L"val") = 1.0f;
          refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
          refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5);

        }
        VERIFY_XML(matNode);
      }
      hrMaterialClose(current_material);

      h_materials.push_back(current_material);
    }
  }

  int mat_indxs_length = int(cumulative_indices_number / 3);
  std::vector<int> mat_ids(mat_indxs_length);
  int mat_counter = 0;
  /*******************************************   Preparing the mesh data   ********************************************/

  // Vertices, Normals, Texture coordinates, Indices
  std::vector<float> verts(cumulative_indices_number * 4);
  std::vector<float> norms(cumulative_indices_number * 4);
  std::vector<float> tex_s(cumulative_indices_number * 2);
  std::vector<int  > indxs(cumulative_indices_number);

  bool has_normals = true;
  bool has_tex = true;

  for (size_t s = 0; s < shapes.size(); s++) {
    size_t index_offset = 0;
    size_t index_shape_offset = 0;
    for(int i = 0; i <= s; ++i){
      index_shape_offset += shape_indices_number[i];
    }
    size_t vertices_num = shapes[s].mesh.num_face_vertices.size();
    for (size_t f = 0; f < vertices_num; f++) {
      // Setting material's index for each polygon
      if(ifMaterialsProvided)
        mat_ids[mat_counter++] = h_materials.at(shapes[s].mesh.material_ids[f]).id;

      int fv = shapes[s].mesh.num_face_vertices[f];
      // Loop over vertices in the face.
      for (size_t v = 0; v < fv; v++) {
        // Current index
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
        // Setting the actual index (we duplicate the vertices so that one vertex corresponds to only one index)
        indxs[index_shape_offset + index_offset + v] = index_shape_offset + index_offset + v;
        // Setting vertices
        verts[4 * (index_shape_offset + index_offset + v) + 0] = attrib.vertices[3 * idx.vertex_index + 0];
        verts[4 * (index_shape_offset + index_offset + v) + 1] = attrib.vertices[3 * idx.vertex_index + 1];
        verts[4 * (index_shape_offset + index_offset + v) + 2] = attrib.vertices[3 * idx.vertex_index + 2];
        verts[4 * (index_shape_offset + index_offset + v) + 3] = 1.0;
        // Setting normals
        if (idx.normal_index != -1) {
          norms[4 * (index_shape_offset + index_offset + v) + 0] = attrib.normals[3 * idx.normal_index + 0];
          norms[4 * (index_shape_offset + index_offset + v) + 1] = attrib.normals[3 * idx.normal_index + 1];
          norms[4 * (index_shape_offset + index_offset + v) + 2] = attrib.normals[3 * idx.normal_index + 2];
          norms[4 * (index_shape_offset + index_offset + v) + 3] = 0.0;
        } else {
          has_normals = false;
        }
        // Setting texture coordinates
        if (idx.texcoord_index != -1) {
          tex_s[2 * (index_shape_offset + index_offset + v) + 0] = attrib.texcoords[2 * idx.texcoord_index + 0];
          tex_s[2 * (index_shape_offset + index_offset + v) + 1] = attrib.texcoords[2 * idx.texcoord_index + 1];
        } else {
          //has_tex = false;
          tex_s[2 * (index_shape_offset + index_offset + v) + 0] = 0.5;
          tex_s[2 * (index_shape_offset + index_offset + v) + 1] = 0.5;
        }
      }
      index_offset += fv;
    }
  }


  /*******************************************   Setting up the buffers   *********************************************/

  HRMeshRef ref = hrMeshCreate(a_objectName);

  hrMeshOpen(ref, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(ref, L"pos", verts.data());

    if (has_normals)
      hrMeshVertexAttribPointer4f(ref, L"norm", norms.data());
    else
      hrMeshVertexAttribPointer4f(ref, L"norm", nullptr);

    //if (has_tex)
      hrMeshVertexAttribPointer2f(ref, L"texcoord", tex_s.data());
    //else
    //  hrMeshVertexAttribPointer2f(ref, L"texcoord", nullptr);

    if(ifMaterialsProvided)
      hrMeshPrimitiveAttribPointer1i(ref, L"mind", mat_ids.data());
    else
      hrMeshMaterialId(ref, 0);

    hrMeshAppendTriangles3(ref, cumulative_indices_number, indxs.data());
  }
  hrMeshClose(ref);

  return ref;
}

std::wstring CutFileName(const std::wstring& fileName);
std::wstring LocalDataPathOfCurrentSceneLibrary();
bool isFileExist(const wchar_t *fileName);

HAPI HRMeshRef hrMeshCreateFromFileDL(const wchar_t* a_fileName, bool a_copyToLocalFolder)
{
  if (a_fileName == nullptr || std::wstring(a_fileName) == L"")
    return HRMeshRef();
  
  if(!isFileExist(a_fileName))
  {
    HrPrint(HR_SEVERITY_ERROR, L"hrMeshCreateFromFileDL, file does not exists: ", a_fileName);
    return HRMeshRef();
  }
 
  HRMeshRef ref = hrMeshCreate(a_fileName);

  HRMesh* pMesh = g_objManager.PtrById(ref);
  if (pMesh == nullptr)
  {
    HrError(L"hrMeshCreateFromFileDL: nullptr created mesh");
    return ref;
  }

  pMesh->pImpl  = g_objManager.m_pFactory->CreateVSGFProxy(a_fileName);
  pMesh->opened = false;

  if (pMesh->pImpl == nullptr)
    return ref;

  auto nodeXml = pMesh->xml_node();
  auto pImpl   = pMesh->pImpl;

  FillXMLFromMeshImpl(nodeXml, pImpl, false);

  nodeXml.force_attribute(L"dl")       = 1;
  nodeXml.force_attribute(L"loc")      = L"unknown";
  nodeXml.force_attribute(L"bytesize") = pImpl->DataSizeInBytes();
  nodeXml.force_attribute(L"path")     = a_fileName;

  pMesh->wasChanged = true;
  pMesh->m_empty    = (nodeXml.attribute(L"bytesize").as_ullong() == 0);
  g_objManager.scnData.m_changeList.meshUsed.insert(pMesh->id);

  if(a_copyToLocalFolder)
  {
    std::wstring fileName1 = CutFileName(a_fileName);
    std::wstring fileName2 = std::wstring(L"data/") + fileName1;

    std::wstring dataFolderPath = LocalDataPathOfCurrentSceneLibrary();
    std::wstring fileName3      = dataFolderPath + fileName1;

    hr_copy_file(a_fileName, fileName3.c_str());
    nodeXml.attribute(L"loc")  = fileName2.c_str();
    nodeXml.attribute(L"path") = L"";
  }

  // (1) to have this function works, we temporary convert it via common mesh that placed in memory, not really DelayedLoad (!!!)
  //
  /*
  HydraGeomData data;
  data.read(a_fileName);

  if (data.getVerticesNumber() == 0)
    return HRMeshRef();

  HRMeshRef ref = hrMeshCreate(a_fileName);

  hrMeshOpen(ref, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(ref, L"pos",      data.getVertexPositionsFloat4Array());
    hrMeshVertexAttribPointer4f(ref, L"norm",     data.getVertexNormalsFloat4Array());
  
    if(data.getVertexTangentsFloat4Array() != nullptr)                                     // for the old format this never happen
      hrMeshVertexAttribPointer4f(ref, L"tangent", data.getVertexTangentsFloat4Array());   // 

    hrMeshVertexAttribPointer2f(ref, L"texcoord", data.getVertexTexcoordFloat2Array());

    hrMeshPrimitiveAttribPointer1i(ref, L"mind", (const int*)data.getTriangleMaterialIndicesArray());
    hrMeshAppendTriangles3(ref, data.getIndicesNumber(), (const int*)data.getTriangleVertexIndicesArray());
  }
  hrMeshClose(ref);
  */
  
  return ref;
}


HAPI HRMeshRef hrMeshCreateFromFile(const wchar_t* a_fileName, HRModelLoadInfo a_modelInfo)
{
  if(!isFileExist(a_fileName))
  {
    HrPrint(HR_SEVERITY_ERROR, L"hrMeshCreateFromFile, file does not exists: ", a_fileName);
    return HRMeshRef();
  }
  
  std::wstring tail = std::wstring(a_fileName).substr(std::wstring(a_fileName).find_last_of(L"."));

  HydraGeomData data;
  std::vector<int> dataBuffer;

  if(tail == L".obj")
    return _hrMeshCreateFromObjMerged(a_fileName, a_modelInfo);
  else if(tail == L".vsgfc")
    data = HR_LoadVSGFCompressedData(a_fileName, dataBuffer);
  else if(tail == L".vsgf")
    data.read(a_fileName);
  else
  {
    HrPrint(HR_SEVERITY_ERROR, L"hrMeshCreateFromFile, unsupported file extension ", tail.c_str());
    return HRMeshRef();
  }

  if (data.getVerticesNumber() == 0)
    return HRMeshRef();

  HRMeshRef ref = hrMeshCreate(a_fileName);

  hrMeshOpen(ref, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(ref, L"pos",      data.getVertexPositionsFloat4Array());
    hrMeshVertexAttribPointer4f(ref, L"norm",     data.getVertexNormalsFloat4Array());

    if(data.getVertexTangentsFloat4Array() != nullptr)                                     // for the old format this never happens
      hrMeshVertexAttribPointer4f(ref, L"tangent", data.getVertexTangentsFloat4Array());   //

    hrMeshVertexAttribPointer2f(ref, L"texcoord", data.getVertexTexcoordFloat2Array());

    hrMeshPrimitiveAttribPointer1i(ref, L"mind", (const int*)data.getTriangleMaterialIndicesArray());
    hrMeshAppendTriangles3(ref, data.getIndicesNumber(), (const int*)data.getTriangleVertexIndicesArray());
  }
  hrMeshClose(ref);

  return ref;
}


HAPI HRMeshRef hrMeshCreateFromFileDL_NoNormals(const wchar_t* a_fileName)
{
  
  if (a_fileName == nullptr || std::wstring(a_fileName) == L"")
    return HRMeshRef();
  
  if(!isFileExist(a_fileName))
  {
    HrPrint(HR_SEVERITY_ERROR, L"hrMeshCreateFromFileDL_NoNormals, file does not exists: ", a_fileName);
    return HRMeshRef();
  }
  
  HydraGeomData data;
  data.read(a_fileName);

  if (data.getVerticesNumber() == 0)
    return HRMeshRef();

  HRMeshRef ref = hrMeshCreate(a_fileName);

  hrMeshOpen(ref, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(ref, L"pos", data.getVertexPositionsFloat4Array());

    if (data.getVertexTangentsFloat4Array() != nullptr)                                    // for the old format this never happen
      hrMeshVertexAttribPointer4f(ref, L"tangent", data.getVertexTangentsFloat4Array());   // 

    hrMeshVertexAttribPointer2f(ref, L"texcoord", data.getVertexTexcoordFloat2Array());

    hrMeshPrimitiveAttribPointer1i(ref, L"mind", (const int*)data.getTriangleMaterialIndicesArray());
    hrMeshAppendTriangles3(ref, data.getIndicesNumber(), (const int*)data.getTriangleVertexIndicesArray());
  }
  hrMeshClose(ref);

  return ref;
}

template<typename T>
static std::vector<T> ReadArrayFromMeshNode(pugi::xml_node meshNode, ChunkPointer a_chunk, const wchar_t* a_arrayName) // pre a_chunk.InMemory()
{
  pugi::xml_node child = meshNode.first_child();
  for (; child != nullptr; child = child.next_sibling())
  {
    if (std::wstring(child.name()) == a_arrayName)
      break;
  }

  const char* data = (const char*)a_chunk.GetMemoryNow();

  const size_t offset = size_t(child.attribute(L"offset").as_ullong());
  const size_t bsize  = size_t(child.attribute(L"bytesize").as_ullong());

  const T* begin = (const T*)(data + offset);
  const T* end   = (const T*)(data + offset + bsize);

  return std::vector<T>(begin, end);
}

const std::wstring GetRealFilePathOfDelayedMesh(pugi::xml_node a_node);
void HR_CopyMeshToInputMeshFromHydraGeomData(const HydraGeomData& data,  HRMesh::InputTriMesh& mesh2);

void OpenHRMesh(HRMesh* pMesh, pugi::xml_node nodeXml)
{
  pMesh->m_input.clear();
  
  if(pMesh->pImpl == nullptr)
    return;
  
  // form m_input from serialized representation ... 
  //
  ChunkPointer chunk;
  auto chunkId = pMesh->pImpl->chunkId();

  if(chunkId != size_t(-1))
    chunk   = g_objManager.scnData.m_vbCache.chunk_at(chunkId);

  if (chunk.InMemory() && chunkId != size_t(-1))
  {
    // (1) read common mesh attributes
    //
    pMesh->m_input.verticesPos      = ReadArrayFromMeshNode<float>   (nodeXml, chunk, L"positions");
    pMesh->m_input.verticesNorm     = ReadArrayFromMeshNode<float>   (nodeXml, chunk, L"normals");
    pMesh->m_input.verticesTangent  = ReadArrayFromMeshNode<float>   (nodeXml, chunk, L"tangents");
    pMesh->m_input.verticesTexCoord = ReadArrayFromMeshNode<float>   (nodeXml, chunk, L"texcoords");
    pMesh->m_input.triIndices       = ReadArrayFromMeshNode<uint32_t>(nodeXml, chunk, L"indices");
    pMesh->m_input.matIndices       = ReadArrayFromMeshNode<uint32_t>(nodeXml, chunk, L"matindices");

    // (2) #TODO: read custom mesh attributes
    //
  }
  else
  {
    std::wstring location   = GetRealFilePathOfDelayedMesh(nodeXml); // ChunkName(chunk);

    const std::wstring tail = str_tail(location, 6);

    HydraGeomData data;
    if(tail == L".vsgfc")
      data = HR_LoadVSGFCompressedData(location.c_str(), g_objManager.m_tempBuffer);
    else
      data.read(location);

    const int vnum = data.getVerticesNumber();
    const int inum = data.getIndicesNumber();

    if (vnum == 0 || inum == 0)
    {
      HrError(L"OpenHRMesh, can't import existing mesh at loc = ", location.c_str());
      return;
    }

    HR_CopyMeshToInputMeshFromHydraGeomData(data, pMesh->m_input);

    //pMesh->pImpl->MList() = FormMatDrawListRLE(pMesh->m_input.matIndices);

    //// (1) read all mesh attributes
    ////
    //pMesh->m_input.verticesPos      = std::vector<float>(data.getVertexPositionsFloat4Array(), data.getVertexPositionsFloat4Array() + 4 * vnum);
    //pMesh->m_input.verticesNorm     = std::vector<float>(data.getVertexNormalsFloat4Array(),   data.getVertexNormalsFloat4Array() + 4 * vnum);
    //pMesh->m_input.verticesTangent  = std::vector<float>(data.getVertexTangentsFloat4Array(),  data.getVertexTangentsFloat4Array() + 4 * vnum);
    //pMesh->m_input.verticesTexCoord = std::vector<float>(data.getVertexTexcoordFloat2Array(),  data.getVertexTexcoordFloat2Array() + 2 * vnum);
    //
    //pMesh->m_input.triIndices       = std::vector<uint32_t>(data.getTriangleVertexIndicesArray(),   data.getTriangleVertexIndicesArray() + inum);
    //pMesh->m_input.matIndices       = std::vector<uint32_t>(data.getTriangleMaterialIndicesArray(), data.getTriangleMaterialIndicesArray() + inum / 3);
    //
    //// (2) #TODO: read custom mesh attributes
    ////
  }

  // set pointers
  //
  pMesh->m_inputPointers.clear();
  pMesh->m_inputPointers.pos       = pMesh->m_input.verticesPos.data();      pMesh->m_inputPointers.posStride = sizeof(float) * 4;
  pMesh->m_inputPointers.normals   = pMesh->m_input.verticesNorm.data();     pMesh->m_inputPointers.normStride = sizeof(float) * 4;
  pMesh->m_inputPointers.tangents  = pMesh->m_input.verticesTangent.data();  pMesh->m_inputPointers.tangStride = sizeof(float) * 4;
  pMesh->m_inputPointers.texCoords = pMesh->m_input.verticesTexCoord.data();
  pMesh->m_inputPointers.mindices  = (const int*)pMesh->m_input.matIndices.data();

  // #TODO: set custom pointers
  //
}

HAPI void hrMeshOpen(HRMeshRef a_mesh, HR_PRIM_TYPE a_type, HR_OPEN_MODE a_mode)
{
  HRMesh* pMesh = g_objManager.PtrById(a_mesh);
  if (pMesh == nullptr)
  {
    HrError(L"hrMeshOpen: nullptr input");
    return;
  }

  pMesh->opened   = true;
  pMesh->openMode = a_mode;

  pMesh->m_input.clear();
  pMesh->m_inputPointers.clear();

  pMesh->m_allMeshMatId  = -1;
  pugi::xml_node nodeXml = pMesh->xml_node();

  if (a_mode == HR_WRITE_DISCARD)
  {
    nodeXml.force_attribute(L"name").set_value(pMesh->name.c_str());
    nodeXml.force_attribute(L"type").set_value(L"vsgf");
    nodeXml.force_attribute(L"bytesize").set_value(L"0");
    nodeXml.force_attribute(L"loc").set_value(L"unknown");
    nodeXml.force_attribute(L"offset").set_value(L"0");
  }
  else // open existing or read only
  {
    OpenHRMesh(pMesh, nodeXml);
  }

}

HAPI pugi::xml_node hrMeshParamNode(HRMeshRef a_mesh)
{
  HRMesh* pMesh = g_objManager.PtrById(a_mesh);
  if (pMesh == nullptr)
  {
    HrError(L"hrMeshParamNode: nullptr input");
    return pugi::xml_node();
  }

  if(!pMesh->opened)
  {
    HrError(L"hrMeshParamNode: mesh was not opened");
    return pugi::xml_node();
  }

  return pMesh->xml_node();
}

HAPI void hrMeshClose(HRMeshRef a_mesh, bool a_compress, bool a_placeToOrigin)
{
  HRMesh* pMesh = g_objManager.PtrById(a_mesh);
  if (pMesh == nullptr)
  {
    HrError(L"hrMeshClose: nullptr input");
    return;
  }

  if (pMesh->openMode == HR_OPEN_READ_ONLY)
  {
    //pMesh->pImpl  = nullptr;
    pMesh->opened = false;
    return;
  }

  // construct dependency list for material -> mesh
  //
  
  //auto& mindices       = pMesh->m_input.matIndices;
  //const size_t maxSize = g_objManager.scnData.materials.size();
  //for (size_t i = 0; i < mindices.size(); i++)
  //{
  //  const int32_t matIndex = mindices[i];
  //  if(matIndex >= 0 && matIndex < maxSize)
  //    g_objManager.scnData.m_materialToMeshDependency.emplace(matIndex, a_mesh.id);
  //}
  
  pMesh->pImpl  = g_objManager.m_pFactory->CreateVSGFFromSimpleInputMesh(pMesh, a_compress);
  pMesh->opened = false;

  if (pMesh->pImpl == nullptr)
    return;

  auto nodeXml = pMesh->xml_node();
  auto pImpl   = pMesh->pImpl;

  FillXMLFromMeshImpl(nodeXml, pImpl, false); 

  pMesh->m_input.freeMem();
  pMesh->m_inputPointers.clear();
  pMesh->wasChanged = true;
  pMesh->m_empty    = (nodeXml.attribute(L"bytesize").as_int() == 0);
  pMesh->m_input.m_saveCompressed = a_compress;
  pMesh->m_input.m_placeToOrigin  = a_placeToOrigin;
  g_objManager.scnData.m_changeList.meshUsed.insert(pMesh->id);

  nodeXml.attribute(L"dl") = 0; // if we 'open/close' mesh then it became common, not delayed load object
  if(a_compress)
  {
    std::wstring originalPath = nodeXml.attribute(L"loc").as_string();
    if(str_tail(originalPath, 6) != L".vsgfc") // already compressed format extension
    {
      std::wstring compressedPath = originalPath + L"c";
      nodeXml.attribute(L"loc")   = compressedPath.c_str();
    }
  }
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


HAPI void hrMeshVertexAttribPointer1f(HRMeshRef a_mesh, const wchar_t* a_name, const float* a_pointer, int a_stride)
{
  HRMesh* pMesh = g_objManager.PtrById(a_mesh);
  if (pMesh == nullptr)
  {
    HrError(L"hrMeshVertexAttribPointer1f: nullptr input");
    return;
  }

  HRMesh::InputTriMeshPointers::CustPointer custPointer;

  custPointer.ptype  = HRMesh::InputTriMeshPointers::CUST_POINTER_FLOAT;
  custPointer.stride = 1; // (a_stride <= 0) ? 1 : a_stride;
  custPointer.fdata  = a_pointer;
  custPointer.name   = a_name;

  pMesh->m_inputPointers.customVertPointers.push_back(custPointer);

}

HAPI void hrMeshVertexAttribPointer2f(HRMeshRef a_mesh, const wchar_t* a_name, const float* a_pointer, int a_stride)
{
  HRMesh* pMesh = g_objManager.PtrById(a_mesh);
  if (pMesh == nullptr)
  {
    HrError(L"hrMeshVertexAttribPointer2f: nullptr input");
    return;
  }

  // temporary "dirty" implementation ...
  //
  if (std::wstring(a_name) == L"tex" || std::wstring(a_name) == L"texcoord")
    pMesh->m_inputPointers.texCoords = a_pointer;
  else
  {
    HRMesh::InputTriMeshPointers::CustPointer custPointer;

    custPointer.ptype  = HRMesh::InputTriMeshPointers::CUST_POINTER_FLOAT;
    custPointer.stride = 2; // (a_stride <= 0) ? 2 : a_stride;
    custPointer.fdata  = a_pointer;
    custPointer.name   = a_name;

    pMesh->m_inputPointers.customVertPointers.push_back(custPointer);
  }
}

HAPI void hrMeshVertexAttribPointer3f(HRMeshRef a_mesh, const wchar_t* a_name, const float* a_pointer, int a_stride)
{
  HRMesh* pMesh = g_objManager.PtrById(a_mesh);
  if (pMesh == nullptr)
  {
    HrError(L"hrMeshVertexAttribPointer3f: nullptr input");
    return;
  }

  // temporary "dirty" implementation ...
  //
  if (std::wstring(a_name) == L"pos" || std::wstring(a_name) == L"positions")
  {
    pMesh->m_inputPointers.pos       = a_pointer;
    pMesh->m_inputPointers.posStride = 3;
  }
  else if (std::wstring(a_name) == L"norm" || std::wstring(a_name) == L"normals")
  {
    pMesh->m_inputPointers.normals    = a_pointer;
    pMesh->m_inputPointers.normStride = 3;
  }
  else
  {
    HRMesh::InputTriMeshPointers::CustPointer custPointer;

    custPointer.ptype  = HRMesh::InputTriMeshPointers::CUST_POINTER_FLOAT;
    custPointer.stride = 3; // (a_stride <= 0) ? 2 : a_stride;
    custPointer.fdata  = a_pointer;
    custPointer.name   = a_name;

    pMesh->m_inputPointers.customVertPointers.push_back(custPointer);
  }


}

HAPI void hrMeshVertexAttribPointer4f(HRMeshRef a_mesh, const wchar_t* a_name, const float* a_pointer, int a_stride)
{
  HRMesh* pMesh = g_objManager.PtrById(a_mesh);
  if (pMesh == nullptr)
  {
    HrError(L"hrMeshVertexAttribPointer4f: nullptr input");
    return;
  }

  // temporary "dirty" implementation ...
  //
  if (std::wstring(a_name) == L"pos" || std::wstring(a_name) == L"positions")
  {
    pMesh->m_inputPointers.pos       = a_pointer;
    pMesh->m_inputPointers.posStride = 4;
  }
  else if (std::wstring(a_name) == L"norm" || std::wstring(a_name) == L"normals")
  {
    pMesh->m_inputPointers.normals    = a_pointer;
    pMesh->m_inputPointers.normStride = 4;
  }
  else if (std::wstring(a_name) == L"tan" || std::wstring(a_name) == L"tang" || std::wstring(a_name) == L"tangent")
  {
    pMesh->m_inputPointers.tangents   = a_pointer;
    pMesh->m_inputPointers.tangStride = 4;
  }
  else
  {
    HRMesh::InputTriMeshPointers::CustPointer custPointer;

    custPointer.ptype  = HRMesh::InputTriMeshPointers::CUST_POINTER_FLOAT;
    custPointer.stride = 4; // (a_stride <= 0) ? 2 : a_stride;
    custPointer.fdata  = a_pointer;
    custPointer.name   = a_name;

    pMesh->m_inputPointers.customVertPointers.push_back(custPointer);
  }

}

HAPI void hrMeshPrimitiveAttribPointer1i(HRMeshRef a_mesh, const wchar_t* a_name, const int* a_pointer, const int a_stride)
{
  HRMesh* pMesh = g_objManager.PtrById(a_mesh);
  if (pMesh == nullptr)
  {
    HrError(L"hrMeshPrimitiveAttribPointer1i: nullptr input");
    return;
  }

  if (std::wstring(a_name) == L"mind")
  {
    pMesh->m_inputPointers.mindices = a_pointer;
  }
  else
  {
    HRMesh::InputTriMeshPointers::CustPointer custPointer;

    custPointer.ptype  = HRMesh::InputTriMeshPointers::CUST_POINTER_INT;
    custPointer.stride = 1; // (a_stride <= 0) ? 2 : a_stride;
    custPointer.idata  = a_pointer;
    custPointer.name   = a_name;

    pMesh->m_inputPointers.customPrimPointers.push_back(custPointer);
  }
}

HAPI void hrMeshComputeTangents(HRMeshRef a_mesh, int indexNum);

static void AddCommonAttributesFromPointers(HRMesh* pMesh, int maxVertexId)
{
  const size_t oldVertexNum = pMesh->m_input.verticesPos.size() / 4;   // remember old vertex buffer size

  // append maxVertexId vertex data
  //
  const float* posPtr  = pMesh->m_inputPointers.pos;
  const float* normPtr = pMesh->m_inputPointers.normals;
  const float* texcPtr = pMesh->m_inputPointers.texCoords;
  const float* tangPtr = pMesh->m_inputPointers.tangents;

	const bool hasNormals  = (normPtr != nullptr);
  const bool hasTangents = (tangPtr != nullptr);
  
  pMesh->m_input.verticesPos.reserve(maxVertexId*4);
  pMesh->m_input.verticesTexCoord.reserve(maxVertexId*2);
  
  if (hasNormals)
    pMesh->m_input.verticesNorm.reserve(maxVertexId*4);
  if (hasTangents)
    pMesh->m_input.verticesTangent.reserve(maxVertexId*4);
    
  for (int i = 0; i <= maxVertexId; i++)
  {
    pMesh->m_input.verticesPos.push_back(posPtr[0]);
    pMesh->m_input.verticesPos.push_back(posPtr[1]);
    pMesh->m_input.verticesPos.push_back(posPtr[2]);
    pMesh->m_input.verticesPos.push_back(1.0f);

    posPtr += pMesh->m_inputPointers.posStride;

		if (hasNormals)
		{
      pMesh->m_input.verticesNorm.push_back(normPtr[0]);
      pMesh->m_input.verticesNorm.push_back(normPtr[1]);
      pMesh->m_input.verticesNorm.push_back(normPtr[2]);
      pMesh->m_input.verticesNorm.push_back(0.0f);

      normPtr += pMesh->m_inputPointers.normStride;
		}

    if (hasTangents)
    {
      pMesh->m_input.verticesTangent.push_back(tangPtr[0]);
      pMesh->m_input.verticesTangent.push_back(tangPtr[1]);
      pMesh->m_input.verticesTangent.push_back(tangPtr[2]);
      pMesh->m_input.verticesTangent.push_back(0.0f);

      tangPtr += pMesh->m_inputPointers.tangStride;
    }
		
    if (texcPtr != nullptr)
    {
      pMesh->m_input.verticesTexCoord.push_back(texcPtr[0]);
      pMesh->m_input.verticesTexCoord.push_back(texcPtr[1]);
      texcPtr += 2;
    }
    else
    {
      pMesh->m_input.verticesTexCoord.push_back(0.0f);
      pMesh->m_input.verticesTexCoord.push_back(0.0f);
    }

  }
}

static void AddCustomAttributesFromPointers(HRMesh* pMesh, int maxVertexId)
{
  // vertex custom attributes
  //
  for (auto ptrs : pMesh->m_inputPointers.customVertPointers)
  {
    {
      HRMesh::InputTriMesh::CustArray arr;
      arr.name  = ptrs.name;
      arr.depth = (ptrs.stride == 3) ? 4 : ptrs.stride;
      arr.apply = 0; // per vertex
      pMesh->m_input.customArrays.push_back(arr);
    }

    auto& custArray = pMesh->m_input.customArrays[pMesh->m_input.customArrays.size() - 1];


    switch (ptrs.ptype)
    {
    case HRMesh::InputTriMeshPointers::CUST_POINTER_FLOAT:

      if (ptrs.stride == 3)
      {
        const float* input = ptrs.fdata;
        const int stride   = ptrs.stride;

        for (int i = 0; i <= maxVertexId; i++)
        {
          custArray.fdata.push_back(input[3 * i + 0]);
          custArray.fdata.push_back(input[3 * i + 1]);
          custArray.fdata.push_back(input[3 * i + 2]);
          custArray.fdata.push_back(0.0f);
        }
      }
      else
      {
        const float* input = ptrs.fdata;
        const int stride   = ptrs.stride;

        for (int i = 0; i <= maxVertexId; i++)
          for (int k = 0; k < stride; k++)
            custArray.fdata.push_back(input[stride*i + k]);
      }
      break;
    
    // we don't have CUST_POINTER_INT per vertex attributes because we can not interpolate them !
    default:
      break;
    }
  }

  // primitive custom attributes
  //
  /************************************************** NOT YET TESTED !!! **************************************************
  for (auto ptrs : pMesh->m_inputPointers.customPrimPointers)
  {
    {
      HRMesh::InputTriMesh::CustArray arr;
      arr.name = ptrs.name;
      pMesh->m_input.customArrays.push_back(arr);
    }

    auto& custArray = pMesh->m_input.customArrays[pMesh->m_input.customArrays.size() - 1];

    custArray.depth = (ptrs.stride == 3) ? 4 : ptrs.stride;
    custArray.apply = 1; // per primitive

    switch (ptrs.ptype)
    {
    case HRMesh::InputTriMeshPointers::CUST_POINTER_FLOAT:

      if (ptrs.stride == 3)
      {
        const float* input = ptrs.fdata;
        const int stride = ptrs.stride;

        for (int i = 0; i <= maxVertexId; i++)
        {
          custArray.fdata.push_back(input[3 * i + 0]);
          custArray.fdata.push_back(input[3 * i + 1]);
          custArray.fdata.push_back(input[3 * i + 2]);
          custArray.fdata.push_back(0.0f);
        }
      }
      else
      {
        const float* input = ptrs.fdata;
        const int stride = ptrs.stride;

        for (int i = 0; i <= maxVertexId; i++)
          for (int k = 0; k < stride; k++)
            custArray.fdata.push_back(input[stride*i + k]);
      }
      break;

    case HRMesh::InputTriMeshPointers::CUST_POINTER_INT:

      if (ptrs.stride == 3)
      {
        const int* input = ptrs.idata;
        const int stride = ptrs.stride;

        for (int i = 0; i <= maxVertexId; i++)
        {
          custArray.idata.push_back(input[3 * i + 0]);
          custArray.idata.push_back(input[3 * i + 1]);
          custArray.idata.push_back(input[3 * i + 2]);
          custArray.idata.push_back(0.0f);
        }
      }
      else
      {
        const int* input = ptrs.idata;
        const int stride = ptrs.stride;

        for (int i = 0; i <= maxVertexId; i++)
          for (int k = 0; k < stride; k++)
            custArray.idata.push_back(input[stride*i + k]);
      }
      break;
    
    default:
      break;
    }
  }

  */


}


HAPI void hrMeshAppendTriangles3(HRMeshRef a_mesh, int indNum, const int* indices, bool weld_vertices)
{
  HRMesh* pMesh = g_objManager.PtrById(a_mesh);

  if (pMesh == nullptr)
  {
    HrError(L"hrMeshAppendTriangles3: nullptr input");
    return;
  }

  if (!pMesh->opened)
  {
    HrError(L"hrMeshAppendTriangles3: mesh is not opened, id = ", a_mesh.id);
    return;
  }

  const int* matIndices = pMesh->m_inputPointers.mindices;

  if (matIndices != nullptr)
    pMesh->m_allMeshMatId = -1;

  if (indices == nullptr)
  {
    HrPrint(HR_SEVERITY_WARNING, L"hrMeshAppendTriangles3: nullptr input indices", a_mesh.id);
    return;
  }

  
  // append triangle indices and find maxVertexId
  //
  const size_t oldVertexNum  = pMesh->m_input.verticesPos.size() / 4;   // remember old vertex buffer size
  
  if(pMesh->m_input.triIndices.capacity() < pMesh->m_input.triIndices.size() + size_t(indNum))
  {
    pMesh->m_input.triIndices.reserve(pMesh->m_input.triIndices.size() + size_t(indNum) + 100);
    pMesh->m_input.matIndices.reserve(pMesh->m_input.triIndices.capacity()/3 + 100 );
  }
  
  int maxVertexId = 0;
  for (int i = 0; i < indNum; i++)
  {
    const int currIndex = indices[i];
    if (currIndex > maxVertexId)
      maxVertexId = currIndex;
    pMesh->m_input.triIndices.push_back(int(oldVertexNum) + currIndex);
  }
  
  // append maxVertexId vertex data
  //
  const size_t newVertexSize = oldVertexNum + maxVertexId;
  const size_t newIndexSize  = oldVertexNum + indNum;

  pMesh->m_input.reserve(newVertexSize, newIndexSize);

  AddCommonAttributesFromPointers(pMesh, maxVertexId);
  AddCustomAttributesFromPointers(pMesh, maxVertexId);

  const bool hasNormals  = (pMesh->m_inputPointers.normals != nullptr);
  const bool hasTangents = (pMesh->m_inputPointers.tangents != nullptr);

  if (!hasNormals)
    hrMeshComputeNormals(a_mesh, indNum); //specify 3rd parameter as "true" to use facenormals

  if(!hasTangents)
    hrMeshComputeTangents(a_mesh, indNum);

  // append per triangle material id
  //
  if (matIndices != nullptr)
  {
    for (int i = 0; i < (indNum / 3); i++)
      pMesh->m_input.matIndices.push_back(matIndices[i]);
  }
  else
  {
    int matId = pMesh->m_allMeshMatId;
    if (matId < 0)
      matId = 0;

    for (int i = 0; i < (indNum / 3); i++)
      pMesh->m_input.matIndices.push_back(matId);
  }
  
  //
  if(weld_vertices)
    hrMeshWeldVertices(a_mesh, indNum);
}

HAPI void hrMeshMaterialId(HRMeshRef a_mesh, int a_matId)
{
  HRMesh* pMesh = g_objManager.PtrById(a_mesh);
  if (pMesh == nullptr)
  {
    HrError(L"hrMeshMaterialId: nullptr input");
    return;
  }

  if (!pMesh->opened)
  {
    HrError(L"hrMeshMaterialId: mesh is not opened, id = ", a_mesh.id);
    return;
  }

  pMesh->m_allMeshMatId = a_matId;
  for (size_t i = 0; i < pMesh->m_input.matIndices.size(); i++)
    pMesh->m_input.matIndices[i] = pMesh->m_allMeshMatId;
}


HAPI void* hrMeshGetAttribPointer(HRMeshRef a_mesh, const wchar_t* attributeName)
{
	HRMesh* pMesh = g_objManager.PtrById(a_mesh);
  if (pMesh == nullptr)
    return nullptr;
  
  
  if (pMesh->opened)
  {
    HRMesh::InputTriMesh& mesh = pMesh->m_input;
  
    //std::cout << "mesh.verticesPos.size() = " << mesh.verticesPos.size() << std::endl;
    
    if (!wcscmp(attributeName, L"pos"))
      return mesh.verticesPos.data();
    else if (!wcscmp(attributeName, L"norm"))
      return mesh.verticesNorm.data();
    else if (!wcscmp(attributeName, L"uv") || !wcscmp(attributeName, L"texcoord"))
      return mesh.verticesTexCoord.data();
    else if (!wcscmp(attributeName, L"tang"))
      return mesh.verticesTangent.data();
    else
      return nullptr;
  }
  else
  {
    return nullptr; // if mesh is closed you can not change it's data !!!
  }
}

HAPI const void*  hrMeshGetAttribConstPointer(HRMeshRef a_mesh, const wchar_t* attributeName)
{
  HRMesh* pMesh = g_objManager.PtrById(a_mesh);
  if (pMesh == nullptr)
    return nullptr;
  
  if (pMesh->opened)
    return nullptr;
  
  auto pImpl = pMesh->pImpl;
  if(pImpl == nullptr)
    return nullptr;
  
  const char* basicPointer = (const char*)pMesh->pImpl->GetData();
  
  auto meshInfo = hrMeshGetInfo(a_mesh);
  
  auto offsets  = CalcOffsets(meshInfo.vertNum, meshInfo.indicesNum, true , true);
  
  if (!wcscmp(attributeName, L"pos"))
  {
    return (basicPointer + offsets.offsetPos);
  }
  else if (!wcscmp(attributeName, L"norm"))
  {
    return (basicPointer + offsets.offsetNorm);
  }
  else if (!wcscmp(attributeName, L"uv") || !wcscmp(attributeName, L"texcoord"))
  {
    return (basicPointer + offsets.offsetTexc);
  }
  else if (!wcscmp(attributeName, L"tang"))
  {
    return (basicPointer + offsets.offsetTang);
  }
  else if (!wcscmp(attributeName, L"ind"))
  {
    return (basicPointer + offsets.offsetInd);
  }
  else if (!wcscmp(attributeName, L"mind"))
  {
    return (basicPointer + offsets.offsetMind);
  }
  
  return nullptr;
}


HAPI void* hrMeshGetPrimitiveAttribPointer(HRMeshRef a_mesh, const wchar_t* attributeName)
{
  HRMesh* pMesh = g_objManager.PtrById(a_mesh);
  if (pMesh == nullptr)
    return nullptr;

  if (pMesh->opened)
  {
    HRMesh::InputTriMesh& mesh = pMesh->m_input;

    if (!wcscmp(attributeName, L"mind"))
      return mesh.matIndices.data();
    else if (!wcscmp(attributeName, L"tind"))
      return mesh.triIndices.data();
    else
      return nullptr;
  }
  else
    return nullptr;

}

std::wstring s2ws(const std::string& s);

HAPI HRMeshInfo  hrMeshGetInfo(HRMeshRef a_mesh)
{
  HRMeshInfo info;

  HRMesh* pMesh = g_objManager.PtrById(a_mesh);
  if (pMesh == nullptr)
    return info;

  HRMesh::InputTriMesh& mesh = pMesh->m_input;
  
  auto pImpl     = pMesh->pImpl;
  auto& mig_data = pMesh->mig_data;

  if(pImpl != nullptr)
  {
    mig_data.batches     = pImpl->MList();
    info.batchesList     = mig_data.batches.data();
    info.batchesListSize = int32_t(mig_data.batches.size());
    
    // create wchar_t** pointers ...
    //
    mig_data.mlist        = s2ws(pImpl->MaterialNamesList());
  
    mig_data.mptrs.resize(0);
    mig_data.mptrs.push_back(mig_data.mlist.data());
    
    for(size_t i=0;i<mig_data.mlist.size();i++)
    {
      if(mig_data.mlist[i] == L";"[0])
      {
        mig_data.mlist[i] = 0; // "\0"
        
        if(i != mig_data.mlist.size()-1)
          mig_data.mptrs.push_back(mig_data.mlist.data() + i + 1);
      }
    }
    
    info.matNamesList     = mig_data.mptrs.data();
    info.matNamesListSize = int32_t(mig_data.mptrs.size());
  
    auto box = pImpl->getBBox();
    info.boxMin[0] = box.x_min;
    info.boxMin[1] = box.y_min;
    info.boxMin[2] = box.z_min;
  
    info.boxMax[0] = box.x_max;
    info.boxMax[1] = box.y_max;
    info.boxMax[2] = box.z_max;
  
    info.indicesNum = int32_t(pImpl->indNum());
    info.vertNum    = int32_t(pImpl->vertNum());
  }
  else
  {
    for(int i=0;i<3;i++)
    {
      info.boxMin[i] = 0;
      info.boxMax[i] = 0;
    }
  }
  
 
  return info;
}

void ComputeVertexNormals(HRMesh::InputTriMesh& mesh, const int indexNum, bool useFaceNormals)
{
  int faceNum = indexNum / 3;

  //std::vector<float3> faceNormals;
  //faceNormals.reserve(faceNum);

  std::vector<float3> vertexNormals(mesh.verticesPos.size() / 4, float3(0.0, 0.0, 0.0));


  for (auto i = 0; i < faceNum; ++i)
  {
    float3 A = float3(mesh.verticesPos.at(4 * mesh.triIndices.at(3*i)),     mesh.verticesPos.at(4 * mesh.triIndices.at(3*i) + 1),     mesh.verticesPos.at(4 * mesh.triIndices.at(3*i) + 2));
    float3 B = float3(mesh.verticesPos.at(4 * mesh.triIndices.at(3*i + 1)), mesh.verticesPos.at(4 * mesh.triIndices.at(3*i + 1) + 1), mesh.verticesPos.at(4 * mesh.triIndices.at(3*i + 1) + 2));
    float3 C = float3(mesh.verticesPos.at(4 * mesh.triIndices.at(3*i + 2)), mesh.verticesPos.at(4 * mesh.triIndices.at(3*i + 2) + 1), mesh.verticesPos.at(4 * mesh.triIndices.at(3*i + 2) + 2));
    
    float3 edge1A = normalize(B - A);
    float3 edge2A = normalize(C - A);

    float3 edge1B = normalize(A - B);
    float3 edge2B = normalize(C - B);

    float3 edge1C = normalize(A - C);
    float3 edge2C = normalize(B - C);

/*
    float3 edge1A = normalize(A - B);
    float3 edge2A = normalize(A - C);

    float3 edge1B = normalize(B - A);
    float3 edge2B = normalize(B - C);

    float3 edge1C = normalize(C - A);
    float3 edge2C = normalize(C - B);
    */

    float3 face_normal = normalize(cross(edge1A, edge2A));
    /*
    vertexNormals.at(mesh.triIndices.at(3 * i)) += face_normal;
    vertexNormals.at(mesh.triIndices.at(3 * i + 1)) += face_normal;
    vertexNormals.at(mesh.triIndices.at(3 * i + 2)) += face_normal;
    */
    if(!useFaceNormals)
    {
      float dotA = dot(edge1A, edge2A);
      float dotB = dot(edge1B, edge2B);
      float dotC = dot(edge1C, edge2C);

      const float lenA = length(cross(edge1A, edge2A));
      const float lenB = length(cross(edge1B, edge2B));
      const float lenC = length(cross(edge1C, edge2C));
      
      float wA = fmaxf(lenA*fabsf(std::acos(dotA)), 1e-5f);
      float wB = fmaxf(lenB*fabsf(std::acos(dotB)), 1e-5f);
      float wC = fmaxf(lenC*fabsf(std::acos(dotC)), 1e-5f);

//      float face_area = 0.5f * sqrtf(powf(edge1A.y * edge2A.z - edge1A.z * edge2A.y, 2) +
//                                     powf(edge1A.z * edge2A.x - edge1A.x * edge2A.z, 2) +
//                                     powf(edge1A.x * edge2A.y - edge1A.y * edge2A.x, 2));
      float face_area = 1.0f;

      float3 normalA = face_normal * wA * face_area;
      float3 normalB = face_normal * wB * face_area;
      float3 normalC = face_normal * wC * face_area;

      vertexNormals.at(mesh.triIndices.at(3 * i + 0)) += normalA;
      vertexNormals.at(mesh.triIndices.at(3 * i + 1)) += normalB;
      vertexNormals.at(mesh.triIndices.at(3 * i + 2)) += normalC;
    }
    else
    {
      vertexNormals.at(mesh.triIndices.at(3 * i + 0)) += face_normal;
      vertexNormals.at(mesh.triIndices.at(3 * i + 1)) += face_normal;
      vertexNormals.at(mesh.triIndices.at(3 * i + 2)) += face_normal;
    }
    //faceNormals.push_back(face_normal);
  }

  if(mesh.verticesNorm.size() != mesh.verticesPos.size())
    mesh.verticesNorm.resize(mesh.verticesPos.size());

  for (int i = 0; i < vertexNormals.size(); ++i)
  {
    float3 N = normalize(vertexNormals.at(i));

    mesh.verticesNorm.at(4 * i + 0) = N.x;
    mesh.verticesNorm.at(4 * i + 1) = N.y;
    mesh.verticesNorm.at(4 * i + 2) = N.z;
    mesh.verticesNorm.at(4 * i + 3) = 0.0f;
  }

}

void hrMeshComputeNormals(HRMeshRef a_mesh, const int indexNum, bool useFaceNormals)
{
	HRMesh* pMesh = g_objManager.PtrById(a_mesh);
	if (pMesh == nullptr)
	{
		HrError(L"hrMeshComputeNormals: nullptr input");
		return;
	}

	HRMesh::InputTriMesh& mesh = pMesh->m_input;

  ComputeVertexNormals(mesh, indexNum, useFaceNormals);
}

void HR_ComputeTangentSpaceSimple(const int     vertexCount, const int     triangleCount, const uint32_t* triIndices,
                                  const float4* verticesPos, const float4* verticesNorm, const float2* vertTexCoord,
                                  float4* verticesTang);

void ComputeVertexTangents(HRMesh::InputTriMesh& mesh, int indexNum)
{
  const int vertexCount      = int(mesh.verticesPos.size()/4);                   // #TODO: not 0-th element, last vertex from prev append!
  const int triangleCount    = indexNum / 3;

  const float4* verticesPos  = (const float4*)(mesh.verticesPos.data());         // #TODO: not 0-th element, last vertex from prev append!
  const float4* verticesNorm = (const float4*)(mesh.verticesNorm.data());        // #TODO: not 0-th element, last vertex from prev append!
  const float2* vertTexCoord = (const float2*)(mesh.verticesTexCoord.data());    // #TODO: not 0-th element, last vertex from prev append!

  float4* verticesTang       = (float4*)(mesh.verticesTangent.data());           // #TODO: not 0-th element, last vertex from prev append!

  HR_ComputeTangentSpaceSimple(vertexCount, triangleCount, mesh.triIndices.data(),
                               verticesPos, verticesNorm, vertTexCoord,
                               verticesTang);
}

void MikeyTSpaceCalc(HRMesh::InputTriMesh* pInput, bool basic);

HAPI void hrMeshComputeTangents(HRMeshRef a_mesh, int indexNum)
{
  HRMesh* pMesh = g_objManager.PtrById(a_mesh);
  if (pMesh == nullptr)
  {
    HrError(L"hrMeshComputeNormals: nullptr input");
    return;
  }

  if (!pMesh->opened)
  {
    HrError(L"hrMeshComputeTangents assume nesh is opened!");
    return;
  }

  HRMesh::InputTriMesh& mesh = pMesh->m_input;
  const int vertexCount      = int(pMesh->m_input.verticesPos.size()/4);
  mesh.verticesTangent.resize(vertexCount*4); // #TODO: not 0-th element, last vertex from prev append!

  MikeyTSpaceCalc(&mesh, false);     // mikktspace implementation
  //ComputeVertexTangents(mesh, indexNum); // simple algotithm
}



void _hrConvertOldVSGFMesh(const std::wstring& a_path, const std::wstring& a_newPath)
{
  // (1) to have this function works, we temporary convert it via common mesh that placed in memory, not really DelayedLoad (!!!)
  //
  HydraGeomData data;
  data.read(a_path);
  if (data.getVerticesNumber() == 0)
    return;

  HRMeshRef ref = hrMeshCreate(a_path.c_str());
  hrMeshOpen(ref, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(ref, L"pos",      data.getVertexPositionsFloat4Array());
    hrMeshVertexAttribPointer4f(ref, L"norm",     data.getVertexNormalsFloat4Array());
    if(data.getVertexTangentsFloat4Array() != nullptr)                                     // for the old format this never happen
      hrMeshVertexAttribPointer4f(ref, L"tangent", data.getVertexTangentsFloat4Array());   //
    hrMeshVertexAttribPointer2f(ref, L"texcoord", data.getVertexTexcoordFloat2Array());
    hrMeshPrimitiveAttribPointer1i(ref, L"mind", (const int*)data.getTriangleMaterialIndicesArray());
    hrMeshAppendTriangles3(ref, data.getIndicesNumber(), (const int*)data.getTriangleVertexIndicesArray());
  }
  hrMeshClose(ref);

  HRMesh* pMesh               = g_objManager.PtrById(ref);
  pugi::xml_node node         = pMesh->xml_node();
  std::wstring newFilePath    = g_objManager.scnData.m_path + L"/" + node.attribute(L"loc").as_string();

  hrFlush();

  hr_copy_file(newFilePath.c_str(), a_newPath.c_str());
  
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


std::string  ws2s(const std::wstring& s);
std::wstring s2ws(const std::string& s);


HAPI void hrMeshSaveVSGF(HRMeshRef a_meshRef, const wchar_t* a_fileName)
{
  std::wstring inFileName(a_fileName);
  std::wstring tail = str_tail(inFileName, 5);
  if(tail != L".vsgf")
  {
    HrError(L"hrMeshSaveVSGF: bad file tail. Must be '.vsgf', but in fact it is ", tail.c_str());
    return;
  }

  HRMesh* pMesh = g_objManager.PtrById(a_meshRef);
  if (pMesh == nullptr)
  {
    HrError(L"hrMeshSaveVSGF: nullptr mesh input");
    return;
  }

  if (pMesh->opened)
  {
    HrError(L"hrMeshSaveVSGF: mesh is opened. Close it please before save. meshId = ", a_meshRef.id);
    return;
  }

  if(pMesh->pImpl->DataSizeInBytes() == 0 || pMesh->pImpl->GetData() == 0)
  {
    HrError(L"hrMeshSaveVSGF: mesh data in not avaliable; meshId = ", a_meshRef.id);
    return;
  }

  std::ofstream fout;
  hr_ofstream_open(fout, a_fileName);
  fout.write((const char*)pMesh->pImpl->GetData(), pMesh->pImpl->DataSizeInBytes());
  fout.close();
}


void PrintMaterialListNames(std::ostream& strOut, HRMesh* pMesh)
{
  auto batchList = pMesh->pImpl->MList();

  for(size_t i=0;i<batchList.size();i++)
  {
    int matId = batchList[i].matId;

    if(matId < 0)
    {
      strOut << "undefined;";
      continue;
    }

    HRMaterialRef matRef;
    matRef.id = matId;

    auto* pMaterial = g_objManager.PtrById(matRef);
    if(pMaterial == nullptr)
    {
      strOut << "undefined;";
      continue;
    }

    std::string matName = ws2s(pMaterial->xml_node().attribute(L"name").as_string());
    strOut << matName.c_str() << ";";
  }
}

HAPI void hrMeshSaveVSGFCompressed(HRMeshRef a_meshRef, const wchar_t* a_fileName)
{
  std::wstring inFileName(a_fileName);
  std::wstring tail = str_tail(inFileName, 6);
  if(tail != L".vsgfc")
  {
    HrError(L"hrMeshSaveVSGFCompressed: bad file tail. Must be '.vsgfc', but in fact it is ", tail.c_str());
    return;
  }

  HRMesh* pMesh = g_objManager.PtrById(a_meshRef);
  if (pMesh == nullptr)
  {
    HrError(L"hrMeshSaveVSGFCompressed: nullptr mesh input");
    return;
  }

  if (pMesh->opened)
  {
    HrError(L"hrMeshSaveVSGFCompressed: mesh is opened. Close it please before save; meshId = ", a_meshRef.id);
    return;
  }

  if(pMesh->pImpl == nullptr)
  {
    HrError(L"hrMeshSaveVSGFCompressed: nullptr impl, can't save; meshId = ", a_meshRef.id);
    return;
  }

  std::stringstream strOut;
  PrintMaterialListNames(strOut, pMesh);
  std::string matnames = strOut.str();

  if(pMesh->pImpl->DataSizeInBytes() == 0 || pMesh->pImpl->GetData() == 0)
  {
    HrError(L"hrMeshSaveVSGFCompressed: mesh data in not avaliable; meshId = ", a_meshRef.id);
    return;
  }

  HR_SaveVSGFCompressed(pMesh->pImpl->GetData(), pMesh->pImpl->DataSizeInBytes(), a_fileName, matnames.c_str(), int(matnames.size()));
}

BBox HRUtils::GetMeshBBox(HRMeshRef a_mesh)
{
  HRMesh* pMesh = g_objManager.PtrById(a_mesh);

  return pMesh->pImpl->getBBox();
}
