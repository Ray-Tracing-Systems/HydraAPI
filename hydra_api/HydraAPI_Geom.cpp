#include "HydraAPI.h"
#include "HydraInternal.h"
#include "HydraInternalCommon.h"

#include <memory>
#include <vector>
#include <array>
#include <string>
#include <map>

#include <fstream>
#include <sstream>
#include <iomanip>
#include <complex>

#include "LiteMath.h"
using namespace HydraLiteMath;

#include "HydraObjectManager.h"
#include "HydraVSGFExport.h"

extern std::wstring      g_lastError;
extern std::wstring      g_lastErrorCallerPlace;
extern HR_ERROR_CALLBACK g_pErrorCallback;
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

void FillXMLFromVSGFInfo(pugi::xml_node nodeXml, VSGFChunkInfo info, std::wstring a_fileName, bool dlLoad)
{
  std::wstring location = a_fileName;

  nodeXml.attribute(L"bytesize").set_value(info.dataBytes);
  if(!dlLoad) 
    g_objManager.SetLoc(nodeXml, location.c_str());
  nodeXml.attribute(L"offset").set_value(L"0");
  nodeXml.attribute(L"vertNum").set_value(info.vertNum);
  nodeXml.attribute(L"triNum").set_value(info.indNum / 3);

  ////
  //
  pugi::xml_node positionArrayNode = nodeXml.append_child(L"positions");
  {
    positionArrayNode.append_attribute(L"type").set_value(L"array4f");
    positionArrayNode.append_attribute(L"bytesize").set_value(info.vertNum * sizeof(float) * 4);
    positionArrayNode.append_attribute(L"offset").set_value(info.offsetPos);
  }

  pugi::xml_node normalsArrayNode = nodeXml.append_child(L"normals");
  {
    normalsArrayNode.append_attribute(L"type").set_value(L"array4f");
    normalsArrayNode.append_attribute(L"bytesize").set_value(info.vertNum * sizeof(float) * 4);
    normalsArrayNode.append_attribute(L"offset").set_value(info.offsetNorm);
  }
  
  pugi::xml_node tangentsArrayNode = nodeXml.append_child(L"tangents");
  {
    tangentsArrayNode.append_attribute(L"type").set_value(L"array4f");
    tangentsArrayNode.append_attribute(L"bytesize").set_value(info.vertNum * sizeof(float) * 4);
    tangentsArrayNode.append_attribute(L"offset").set_value(info.offsetTang);
  }

  pugi::xml_node texcoordArrayNode = nodeXml.append_child(L"texcoords");
  {
    texcoordArrayNode.append_attribute(L"type").set_value(L"array2f");
    texcoordArrayNode.append_attribute(L"bytesize").set_value(info.vertNum * sizeof(float) * 2);
    texcoordArrayNode.append_attribute(L"offset").set_value(info.offsetTexc);
  }

  pugi::xml_node indicesArrayNode = nodeXml.append_child(L"indices");
  {
    indicesArrayNode.append_attribute(L"type").set_value(L"array1i");
    indicesArrayNode.append_attribute(L"bytesize").set_value(info.indNum * sizeof(int));
    indicesArrayNode.append_attribute(L"offset").set_value(info.offsetInd);
  }

  pugi::xml_node mindicesArrayNode = nodeXml.append_child(L"matindices");
  {
    mindicesArrayNode.append_attribute(L"type").set_value(L"array1i");
    mindicesArrayNode.append_attribute(L"bytesize").set_value(info.indNum * sizeof(int) / 3);
    mindicesArrayNode.append_attribute(L"offset").set_value(info.offsetMInd);
  }
}

void ReadInfoFromVSGFHeaderToXML(const wchar_t* a_fileName, pugi::xml_node nodeXml, bool dlLoad)
{
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
  std::wstring s1(a_fileName);
  std::string  s2(s1.begin(), s1.end());
  std::ifstream fin(s2.c_str(), std::ios::binary);
#elif defined WIN32
  std::ifstream fin(a_fileName, std::ios::binary);
#endif
  HydraGeomData::Header header;
  fin.read((char*)&header, sizeof(HydraGeomData::Header));

  ////
  //
  VSGFChunkInfo info;

  info.vertNum    = header.verticesNum;
  info.indNum     = header.indicesNum;
  info.dataBytes  = header.fileSizeInBytes;

  info.offsetPos  = sizeof(HydraGeomData::Header);
  info.offsetNorm = info.offsetPos  + info.vertNum*sizeof(float4);
  info.offsetTang = -1; // info.offsetNorm + info.vertNum * sizeof(float4); // #TODO: fix deffered mesh load
  info.offsetTexc = info.offsetNorm + info.vertNum*sizeof(float4);
  info.offsetInd  = info.offsetTexc + info.vertNum*sizeof(float2);
  info.offsetMInd = info.offsetInd  + info.indNum*sizeof(int);

  FillXMLFromVSGFInfo(nodeXml, info, a_fileName, dlLoad);
}

HAPI HRMeshRef hrMeshCreate(const wchar_t* a_objectName)
{
  HRMesh mesh;
  mesh.name = std::wstring(a_objectName);
  g_objManager.scnlib().meshes.push_back(mesh);

  HRMeshRef ref;
  ref.id = HR_IDType(g_objManager.scnlib().meshes.size() - 1);

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

  g_objManager.scnlib().meshes[ref.id].update_next(nodeXml);
  g_objManager.scnlib().meshes[ref.id].id = ref.id;

  return ref;
}

HAPI HRMeshRef hrMeshCreateFromFileDL(const wchar_t* a_fileName)
{
  if (a_fileName == nullptr || std::wstring(a_fileName) == L"")
    return HRMeshRef();
  
  // (1) this implementation is old, we create the new one later
  // 
  // HRMeshRef ref = hrMeshCreate(a_fileName);
  // pugi::xml_node nodeXml = g_objManager.scnlib().meshes[ref.id].xml_node_immediate();
  // nodeXml.attribute(L"dl").set_value(L"1");
  // nodeXml.attribute(L"path").set_value(a_fileName);
  // 
  // ReadInfoFromVSGFHeaderToXML(a_fileName, nodeXml, true);
  // 
  // HRMesh* pMesh = &g_objManager.scnlib().meshes[ref.id];
  // pMesh->pImpl  = g_objManager.m_pFactory->CreateVSGFFromFile(pMesh, a_fileName);

  // (2) to have this function works, we temporary convert it via common mesh that placed in memory, not really DelayedLoad (!!!)
  //
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

  return ref;
}


HAPI HRMeshRef hrMeshCreateFromFileDL_NoNormals(const wchar_t* a_fileName)
{
  if (a_fileName == nullptr || std::wstring(a_fileName) == L"")
    return HRMeshRef();
  HydraGeomData data;
  data.read(a_fileName);

  if (data.getVerticesNumber() == 0)
    return HRMeshRef();

  HRMeshRef ref = hrMeshCreate(a_fileName);

  hrMeshOpen(ref, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(ref, L"pos", data.getVertexPositionsFloat4Array());

    if (data.getVertexTangentsFloat4Array() != nullptr)                                     // for the old format this never happen
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

void OpenHRMesh(HRMesh* pMesh, pugi::xml_node nodeXml)
{
  // form m_input from serialized representation ... 
  //
  auto chunkId = pMesh->pImpl->chunkId();
  auto chunk   = g_objManager.scnlib().m_vbCache.chunk_at(chunkId);

  pMesh->m_input.clear();

  if (chunk.InMemory())
  {
    pMesh->m_input.verticesPos      = ReadArrayFromMeshNode<float>   (nodeXml, chunk, L"positions");
    pMesh->m_input.verticesNorm     = ReadArrayFromMeshNode<float>   (nodeXml, chunk, L"normals");
    pMesh->m_input.verticesTangent  = ReadArrayFromMeshNode<float>   (nodeXml, chunk, L"tangents");
    pMesh->m_input.verticesTexCoord = ReadArrayFromMeshNode<float>   (nodeXml, chunk, L"texcoords");
    pMesh->m_input.triIndices       = ReadArrayFromMeshNode<uint32_t>(nodeXml, chunk, L"indices");
    pMesh->m_input.matIndices       = ReadArrayFromMeshNode<uint32_t>(nodeXml, chunk, L"matindices");
  }
  else
  {
    std::wstring location = ChunkName(chunk);

    HydraGeomData data;
    data.read(location);

    const int vnum = data.getVerticesNumber();
    const int inum = data.getIndicesNumber();

    if (vnum == 0)
    {
      HrError(L"OpenHRMesh, can't import existing mesh at loc = ", location.c_str());
      return;
    }

    pMesh->m_input.verticesPos      = std::vector<float>(data.getVertexPositionsFloat4Array(), data.getVertexPositionsFloat4Array() + 4 * vnum);
    pMesh->m_input.verticesNorm     = std::vector<float>(data.getVertexNormalsFloat4Array(),   data.getVertexNormalsFloat4Array() + 4 * vnum);
    pMesh->m_input.verticesTangent  = std::vector<float>(data.getVertexTangentsFloat4Array(),  data.getVertexTangentsFloat4Array() + 4 * vnum);
    pMesh->m_input.verticesTexCoord = std::vector<float>(data.getVertexTexcoordFloat2Array(),  data.getVertexTexcoordFloat2Array() + 2 * vnum);

    pMesh->m_input.triIndices = std::vector<uint32_t>(data.getTriangleVertexIndicesArray(),   data.getTriangleVertexIndicesArray() + inum);
    pMesh->m_input.matIndices = std::vector<uint32_t>(data.getTriangleMaterialIndicesArray(), data.getTriangleMaterialIndicesArray() + inum / 3);
  }

  // set pointers
  //

  pMesh->m_inputPointers.clear();
  pMesh->m_inputPointers.pos       = &pMesh->m_input.verticesPos[0];      pMesh->m_inputPointers.posStride = sizeof(float) * 4;
  pMesh->m_inputPointers.normals   = &pMesh->m_input.verticesNorm[0];     pMesh->m_inputPointers.normStride = sizeof(float) * 4;
  pMesh->m_inputPointers.tangents  = &pMesh->m_input.verticesTangent[0];  pMesh->m_inputPointers.tangStride = sizeof(float) * 4;
  pMesh->m_inputPointers.texCoords = &pMesh->m_input.verticesTexCoord[0];
  pMesh->m_inputPointers.mindices  = (const int*)&pMesh->m_input.matIndices[0];
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
  pugi::xml_node nodeXml = pMesh->xml_node_next(a_mode);

  if (a_mode == HR_WRITE_DISCARD)
  {
    nodeXml.attribute(L"name").set_value(pMesh->name.c_str());
    nodeXml.attribute(L"type").set_value(L"vsgf");
    nodeXml.attribute(L"bytesize").set_value(L"0");
    nodeXml.attribute(L"loc").set_value(L"unknown");
    nodeXml.attribute(L"offset").set_value(L"0");
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

  return pMesh->xml_node_next(pMesh->openMode);
}

HAPI void hrMeshClose(HRMeshRef a_mesh)
{
  HRMesh* pMesh = g_objManager.PtrById(a_mesh);
  if (pMesh == nullptr)
  {
    HrError(L"hrMeshClose: nullptr input");
    return;
  }

  if (pMesh->openMode == HR_OPEN_READ_ONLY)
  {
    pMesh->pImpl  = nullptr;
    pMesh->opened = false;
    return;
  }

  // construct dependency list for material -> mesh
  //
  auto& mindices = pMesh->m_input.matIndices;
  for (size_t i = 0; i < mindices.size(); i++)
    g_objManager.scnlib().m_materialToMeshDependency.emplace(mindices[i], a_mesh.id);

  pMesh->pImpl  = g_objManager.m_pFactory->CreateVSGFFromSimpleInputMesh(pMesh);
  pMesh->opened = false;

  if (pMesh->pImpl == nullptr)
    return;

  auto nodeXml = pMesh->xml_node_next();
  auto pImpl   = pMesh->pImpl;

  ////
  //
  size_t       chunkId  = pImpl->chunkId();
  ChunkPointer chunk    = g_objManager.scnlib().m_vbCache.chunk_at(chunkId);
  std::wstring location = ChunkName(chunk);


  VSGFChunkInfo info;

  info.vertNum    = pImpl->vertNum();
  info.indNum     = pImpl->indNum();
  info.offsetPos  = pImpl->offset(L"pos");
  info.offsetNorm = pImpl->offset(L"norm");
  info.offsetTexc = pImpl->offset(L"texc");
  info.offsetTang = pImpl->offset(L"tan");
  info.offsetInd  = pImpl->offset(L"ind");
  info.offsetMInd = pImpl->offset(L"mind");
  info.dataBytes  = chunk.sizeInBytes;

  FillXMLFromVSGFInfo(nodeXml, info, location, false);

  pMesh->m_input.freeMem();
  pMesh->m_inputPointers.clear();
  pMesh->wasChanged = true;
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
    pMesh->m_inputPointers.pos = a_pointer;
    pMesh->m_inputPointers.posStride = 3;
  }
  else if (std::wstring(a_name) == L"norm" || std::wstring(a_name) == L"normals")
  {
    pMesh->m_inputPointers.normals = a_pointer;
    pMesh->m_inputPointers.normStride = 3;
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
}

HAPI void hrMeshComputeNormals(HRMeshRef a_mesh, const int indexNum, bool useFaceNormals = false);
HAPI void hrMeshComputeTangents(HRMeshRef a_mesh, const int indexNum);

HAPI void hrMeshAppendTriangles3(HRMeshRef a_mesh, int indNum, const int* indices)
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

  if (indices == 0)
  {
    HrPrint(HR_SEVERITY_WARNING, L"hrMeshAppendTriangles3: nullptr input indices", a_mesh.id);
    return;
  }

  // find max vertex id
  //
  int maxVertexId = 0;
  for (int i = 0; i < indNum; i++)
  {
    if (indices[i] > maxVertexId)
      maxVertexId = indices[i];
  }

  const size_t oldVertexNum = pMesh->m_input.verticesPos.size() / 4;   // remember old vertex buffer size
  const size_t oldIndexNum  = pMesh->m_input.triIndices.size();   // remember old vertex buffer size

  // append maxVertexId vertex data
  //
  const float* posPtr  = pMesh->m_inputPointers.pos;
  const float* normPtr = pMesh->m_inputPointers.normals;
  const float* texcPtr = pMesh->m_inputPointers.texCoords;
  const float* tangPtr = pMesh->m_inputPointers.tangents;

	const bool hasNormals  = (normPtr != nullptr);
  const bool hasTangents = (tangPtr != nullptr);

  const size_t newVertexSize = oldVertexNum + maxVertexId;
  const size_t newIndexSize  = oldVertexNum + indNum;

  pMesh->m_input.reserve(newVertexSize, newIndexSize);

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

	// now append triangle indices ...
	//
	for (int i = 0; i < indNum; i++)
		pMesh->m_input.triIndices.push_back(int(oldVertexNum) + indices[i]);

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

    if (!wcscmp(attributeName, L"pos"))
    {
      if (mesh.verticesPos.empty())
        return nullptr;
      else
        return (void*)&(mesh.verticesPos[0]);
    }
    else if (!wcscmp(attributeName, L"norm"))
    {
      if (mesh.verticesNorm.empty())
        return nullptr;
      else
        return (void*)&(mesh.verticesNorm[0]);

    }
    else if (!wcscmp(attributeName, L"uv"))
    {
      if (mesh.verticesTexCoord.empty())
        return nullptr;
      else
        return (void*)&(mesh.verticesTexCoord[0]);
    }
    else
      return nullptr;
  }
  else
  {
    return nullptr; // if mesh is closed you can not change it's data !!!
  }
}


HAPI void* hrMeshGetPrimitiveAttribPointer(HRMeshRef a_mesh, const wchar_t* attributeName)
{
  HRMesh* pMesh = g_objManager.PtrById(a_mesh);
  if (pMesh == nullptr)
    return nullptr;

  if (pMesh->opened)
  {
    HRMesh::InputTriMesh& mesh = pMesh->m_input;

    if (!wcscmp(attributeName, L"mind") && mesh.matIndices.size() != 0)
      return (void*)(&mesh.matIndices[0]);
    else
      return nullptr;
  }
  else
    return nullptr;

}

HAPI HROpenedMeshInfo  hrMeshGetInfo(HRMeshRef a_mesh)
{
  HROpenedMeshInfo info;
  info.indicesNum = 0;
  info.vertNum    = 0;

  HRMesh* pMesh = g_objManager.PtrById(a_mesh);
  if (pMesh == nullptr)
    return info;

  if(!pMesh->opened)
    return info;

  HRMesh::InputTriMesh& mesh = pMesh->m_input;

  info.indicesNum = int32_t(mesh.triIndices.size());
  info.vertNum    = int32_t(mesh.verticesPos.size() / 4);
  return info;
}


HAPI void hrMeshComputeNormals(HRMeshRef a_mesh, const int indexNum, bool useFaceNormals)
{
	HRMesh* pMesh = g_objManager.PtrById(a_mesh);
	if (pMesh == nullptr)
	{
		HrError(L"hrMeshComputeNormals: nullptr input");
		return;
	}

	const HRMesh::InputTriMesh& mesh = pMesh->m_input;

	int faceNum = indexNum / 3;

	//std::vector<float3> faceNormals;
	//faceNormals.reserve(faceNum);

	std::vector<float3> vertexNormals(mesh.triIndices.size(), float3(0.0, 0.0, 0.0));


	for (auto i = 0; i < faceNum; ++i)
	{
		float3 A = float3(mesh.verticesPos.at(4 * mesh.triIndices.at(3*i)), mesh.verticesPos.at(4 * mesh.triIndices.at(3*i) + 1), mesh.verticesPos.at(4 * mesh.triIndices.at(3*i) + 2));
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
      
      float wA = fmax(lenA*fabsf(std::acos(dotA)), 1e-5f);
      float wB = fmax(lenB*fabsf(std::acos(dotB)), 1e-5f);
      float wC = fmax(lenC*fabsf(std::acos(dotC)), 1e-5f);

     // float face_area = std::abs((A.x * (B.y - C.y) + B.x * (C.y - A.y) + C.x * (A.y - B.y)) / 2.0f);

      float3 normalA = face_normal * wA;// *face_area;
      float3 normalB = face_normal * wB;// *face_area;
      float3 normalC = face_normal * wC;// *face_area;

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

	for (int i = 0; i < vertexNormals.size(); ++i)
	{
		float3 N = normalize(vertexNormals.at(i));

		pMesh->m_input.verticesNorm.push_back(N.x);
		pMesh->m_input.verticesNorm.push_back(N.y);
		pMesh->m_input.verticesNorm.push_back(N.z);
		pMesh->m_input.verticesNorm.push_back(1.0f);
	}
}

HAPI void hrMeshComputeTangents(HRMeshRef a_mesh, const int indexNum)
{
  HRMesh* pMesh = g_objManager.PtrById(a_mesh);
  if (pMesh == nullptr)
  {
    HrError(L"hrMeshComputeNormals: nullptr input");
    return;
  }

  HRMesh::InputTriMesh& mesh = pMesh->m_input;

  const int vertexCount   = int(mesh.verticesPos.size()/4);                   // #TODO: not 0-th element, last vertex from prev append!
  const int triangleCount = indexNum / 3;

  float4 *tan1 = new float4[vertexCount * 2];
  float4 *tan2 = tan1 + vertexCount;
  memset(tan1, 0, vertexCount * sizeof(float4) * 2);
                                                                              // #TODO: not 0-th element, last vertex from prev append!
  const float4* verticesPos  = (const float4*)(&mesh.verticesPos[0]);         // #TODO: not 0-th element, last vertex from prev append!
  const float4* verticesNorm = (const float4*)(&mesh.verticesNorm[0]);        // #TODO: not 0-th element, last vertex from prev append!
  const float2* vertTexCoord = (const float2*)(&mesh.verticesTexCoord[0]);    // #TODO: not 0-th element, last vertex from prev append!
 
  for (auto a = 0; a < triangleCount; a++)
  {
    auto i1 = mesh.triIndices[3 * a + 0];
    auto i2 = mesh.triIndices[3 * a + 1];
    auto i3 = mesh.triIndices[3 * a + 2];

    const float4& v1 = verticesPos[i1];
    const float4& v2 = verticesPos[i2];
    const float4& v3 = verticesPos[i3];

    const float2& w1 = vertTexCoord[i1];
    const float2& w2 = vertTexCoord[i2];
    const float2& w3 = vertTexCoord[i3];

    float x1 = v2.x - v1.x;
    float x2 = v3.x - v1.x;
    float y1 = v2.y - v1.y;
    float y2 = v3.y - v1.y;
    float z1 = v2.z - v1.z;
    float z2 = v3.z - v1.z;

    float s1 = w2.x - w1.x;
    float s2 = w3.x - w1.x;
    float t1 = w2.y - w1.y;
    float t2 = w3.y - w1.y;

    float r = 1.0f / (s1 * t2 - s2 * t1);
    float4 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r, 1);
    float4 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r, 1);

    tan1[i1] += sdir;
    tan1[i2] += sdir;
    tan1[i3] += sdir;

    tan2[i1] += tdir;
    tan2[i2] += tdir;
    tan2[i3] += tdir;
  }

  mesh.verticesTangent.resize(vertexCount*4);                 // #TODO: not 0-th element, last vertex from prev append!
  float4* verticesTang = (float4*)(&mesh.verticesTangent[0]); // #TODO: not 0-th element, last vertex from prev append!

  for (long a = 0; a < vertexCount; a++)
  {
    const float4& n = verticesNorm[a];
    const float4& t = tan1[a];

    const float3 n1 = to_float3(n);
    const float3 t1 = to_float3(t);

    // Gram-Schmidt orthogonalization
    verticesTang[a] = to_float4(normalize(t1 - n1 * dot(n1, t1)), 0.0f);

    // Calculate handedness
    verticesTang[a].w = (dot(cross(n1, t1), to_float3(tan2[a])) < 0.0f) ? -1.0f : 1.0f;
  }
  
  delete[] tan1;
  
}
