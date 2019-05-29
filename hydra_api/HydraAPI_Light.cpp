#include "HydraAPI.h"
#include "HydraInternal.h"

#include <memory>
#include <vector>
#include <string>
#include <map>

#include <sstream>
#include <iomanip>
#include "HydraObjectManager.h"
#include "LiteMath.h"
#include "HydraXMLHelpers.h"

extern std::wstring      g_lastError;
extern std::wstring      g_lastErrorCallerPlace;
extern HR_ERROR_CALLBACK g_pErrorCallback;
extern HRObjectManager   g_objManager;

using HydraLiteMath::float2;
using HydraLiteMath::float3;
using HydraLiteMath::float4;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////    Light    /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HAPI HRLightRef hrLightCreate(const wchar_t* a_objectName)
{
  HRLightRef ref;
  ref.id = HR_IDType(g_objManager.scnData.lights.size());

  std::wstring nameGenerated;
  if (a_objectName == nullptr) // create internal name for material
  {
    std::wstringstream strOut;
    strOut << L"light#" << ref.id;
    nameGenerated = strOut.str();
    a_objectName = nameGenerated.c_str();
  }

  HRLight light;
  light.name = std::wstring(a_objectName);
  light.id = ref.id;
  g_objManager.scnData.lights.push_back(light);


  pugi::xml_node nodeXml = g_objManager.lights_lib_append_child();

	nodeXml.append_attribute(L"id").set_value(ref.id);
  nodeXml.append_attribute(L"name").set_value(light.name.c_str());
  nodeXml.append_attribute(L"type").set_value(L"hydra_light");
  nodeXml.append_attribute(L"shape").set_value(L"point");
  nodeXml.append_attribute(L"distribution").set_value(L"omni");
	nodeXml.append_attribute(L"visible").set_value(L"1");

  g_objManager.scnData.lights[ref.id].update(nodeXml);
  g_objManager.scnData.lights[ref.id].id = ref.id;

  return ref;
}

HAPI void hrLightOpen(HRLightRef a_pLight, HR_OPEN_MODE a_openMode)
{
  HRLight* pLight = g_objManager.PtrById(a_pLight);
  if (pLight == nullptr)
  {
    HrError(L"hrLightOpen, nullptr input ");
    return;
  }

  if (pLight->opened)
  {
    HrError(L"hrLightOpen, double open material, with id = ", pLight->id);
    return;
  }

  pLight->openMode = a_openMode;
  pLight->opened   = true;

  pugi::xml_node nodeXml = pLight->xml_node();

  if (a_openMode == HR_WRITE_DISCARD)
  {
    clear_node_childs(nodeXml);
    nodeXml.attribute(L"name").set_value(pLight->name.c_str());
    nodeXml.attribute(L"type").set_value(L"hydra_light");
    nodeXml.attribute(L"shape").set_value(L"point");
    nodeXml.attribute(L"distribution").set_value(L"omni");
    nodeXml.attribute(L"id").set_value(pLight->id);
  }
  else if (a_openMode == HR_OPEN_EXISTING)
  {

  }
  else
  {
    HrError(L"hrLightOpen, bad open mode ");
  }

}

HAPI pugi::xml_node hrLightParamNode(HRLightRef a_lightRef)
{
  HRLight* pLight = g_objManager.PtrById(a_lightRef);
  if (pLight == nullptr)
  {
    HrError(L"hrLightParamNode, nullptr input ");
    return pugi::xml_node();
  }

  if (!pLight->opened)
  {
    HrError(L"hrLightParamNode, light is not opened, light id = ", pLight->id);
    return pugi::xml_node();
  }

  return pLight->xml_node();
}


HAPI void hrLightClose(HRLightRef a_pLight)
{
  HRLight* pLight = g_objManager.PtrById(a_pLight);
  if (pLight == nullptr)
  {
    HrError(L"hrLightClose, nullptr input ");
    return;
  }

  if (!pLight->opened)
  {
    HrError(L"hrLightClose, double close light, with id = ", pLight->id);
    return;
  }

  // if have ies files, copy them to local data folder and alter 
  //
  auto lightNode = pLight->xml_node();

  if (lightNode.child(L"ies") != nullptr)
  {
    const wchar_t* iesFilePath = lightNode.child(L"ies").attribute(L"data").as_string();
    if (iesFilePath != nullptr && std::wstring(iesFilePath) != L"")
    {
      std::wstringstream fileNameOut;
      fileNameOut << L"data/ies_" << a_pLight.id << ".ies";

      std::wstring newFileName = fileNameOut.str();
      std::wstring fullPath    = g_objManager.scnData.m_path + std::wstring(L"/") + newFileName;

      auto p = g_objManager.scnData.m_iesCache.find(iesFilePath); //#TODO: add some test for check m_iesCache in work!!!
      if (p == g_objManager.scnData.m_iesCache.end())
      {
        hr_copy_file(iesFilePath, fullPath.c_str());
        g_objManager.scnData.m_iesCache[iesFilePath] = newFileName;
      }
      else
      {
        newFileName = p->second;
      }
     
      lightNode.child(L"ies").force_attribute(L"loc").set_value(newFileName.c_str());
    }
  }

  pLight->opened     = false;
  pLight->wasChanged = true;
  g_objManager.scnData.m_changeList.lightUsed.insert(pLight->id);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct HR_SimpleMesh
{
  HR_SimpleMesh() = default;

  HR_SimpleMesh(HR_SimpleMesh&& a_in)      = default;
  HR_SimpleMesh(const HR_SimpleMesh& a_in) = default;
 
  HR_SimpleMesh& operator=(HR_SimpleMesh&& a_in)      = default;
  HR_SimpleMesh& operator=(const HR_SimpleMesh& a_in) = default;

  std::vector<float>      vPos;
  std::vector<float>      vNorm;
  std::vector<float>      vTexCoord;
  std::vector<uint32_t>   triIndices;
  std::vector<int>        matIndices;
};

static HR_SimpleMesh CreateSphereMeshForLight(int a_matId, float radius, int numberSlices)
{
  HR_SimpleMesh sphere;

  int i, j;

  int numberParallels = numberSlices;
  int numberVertices = (numberParallels + 1) * (numberSlices + 1);
  int numberIndices = numberParallels * numberSlices * 3;

  float angleStep = (2.0f * 3.14159265358979323846f) / ((float)numberSlices);

  sphere.vPos.resize(numberVertices * 4);
  sphere.vNorm.resize(numberVertices * 4);
  sphere.vTexCoord.resize(numberVertices * 2);

  sphere.triIndices.resize(numberIndices);
  sphere.matIndices.resize(numberIndices / 3);

  for (size_t k = 0; k < sphere.matIndices.size(); k++)
    sphere.matIndices[k] = a_matId;

  for (i = 0; i < numberParallels + 1; i++)
  {
    for (j = 0; j < numberSlices + 1; j++)
    {
      int vertexIndex = (i * (numberSlices + 1) + j) * 4;
      int normalIndex = (i * (numberSlices + 1) + j) * 4;
      int texCoordsIndex = (i * (numberSlices + 1) + j) * 2;

      sphere.vPos[vertexIndex + 0] = radius * sinf(angleStep * (float)i) * sinf(angleStep * (float)j);
      sphere.vPos[vertexIndex + 1] = radius * cosf(angleStep * (float)i);
      sphere.vPos[vertexIndex + 2] = radius * sinf(angleStep * (float)i) * cosf(angleStep * (float)j);
      sphere.vPos[vertexIndex + 3] = 1.0f;

      sphere.vNorm[normalIndex + 0] = sphere.vPos[vertexIndex + 0] / radius;
      sphere.vNorm[normalIndex + 1] = sphere.vPos[vertexIndex + 1] / radius;
      sphere.vNorm[normalIndex + 2] = sphere.vPos[vertexIndex + 2] / radius;
      sphere.vNorm[normalIndex + 3] = 1.0f;

      sphere.vTexCoord[texCoordsIndex + 0] = (float)j / (float)numberSlices;
      sphere.vTexCoord[texCoordsIndex + 1] = (1.0f - (float)i) / (float)(numberParallels - 1);
    }
  }

  auto* indexBuf = &sphere.triIndices[0];

  for (i = 0; i < numberParallels; i++)
  {
    for (j = 0; j < numberSlices; j++)
    {
      *indexBuf++ = i * (numberSlices + 1) + j;
      *indexBuf++ = (i + 1) * (numberSlices + 1) + j;
      *indexBuf++ = (i + 1) * (numberSlices + 1) + (j + 1);

      *indexBuf++ = i * (numberSlices + 1) + j;
      *indexBuf++ = (i + 1) * (numberSlices + 1) + (j + 1);
      *indexBuf++ = i * (numberSlices + 1) + (j + 1);

      int diff = int(indexBuf - &sphere.triIndices[0]);
      if (diff >= numberIndices)
        break;
    }

    int diff = int(indexBuf - &sphere.triIndices[0]);
    if (diff >= numberIndices)
      break;
  }

  //int diff = indexBuf - &sphere.triIndices[0];

  return sphere;
}

static HR_SimpleMesh CreateRectMeshForLight(int a_matId, float2 size)
{
  const int numVert    = 4;
  const int numIndices = 6;

  HR_SimpleMesh mesh;

  mesh.vPos.resize(numVert*4);
  mesh.vNorm.resize(numVert*4);
  mesh.vTexCoord.resize(numVert*2);
  mesh.triIndices.resize(numIndices);
  mesh.matIndices.resize(numIndices/3);

  float4* vertPos  = (float4*)&mesh.vPos[0];
  float4* vertNorm = (float4*)&mesh.vNorm[0];
  float2* vertTexc = (float2*)&mesh.vTexCoord[0];

  auto* indices  = &mesh.triIndices[0];
  int* mindices  = &mesh.matIndices[0];

  const float4 norm = float4(0, -1, 0, 0);

  vertPos[0]  = float4(-size.x, 0, -size.y, 1); vertNorm[0] = norm;
  vertPos[1]  = float4(-size.x, 0, size.y, 1);  vertNorm[1] = norm;
  vertPos[2]  = float4(size.x,  0, size.y, 1);  vertNorm[2] = norm;
  vertPos[3]  = float4(size.x,  0, -size.y, 1); vertNorm[3] = norm;

  vertTexc[0] = float2(0, 0);
  vertTexc[1] = float2(0, 1);
  vertTexc[2] = float2(1, 1);
  vertTexc[3] = float2(1, 0);

  indices[0] = 0; indices[1] = 1; indices[2] = 2;
  indices[3] = 2; indices[4] = 3; indices[5] = 0;

  mindices[0] = a_matId;
  mindices[1] = a_matId;

  return mesh;
}

static HR_SimpleMesh CreateDiskMeshForLight(int a_matId, float a_radius)
{

  const int numVertOld    = 128;
  const int numIndicesOld = numVertOld * 3;

  const int numVert    = numVertOld+1;
  const int numIndices = numVert * 3;

  const int LAST_VERT = numVertOld;

  HR_SimpleMesh mesh;

  mesh.vPos.resize(numVert*4);
  mesh.vNorm.resize(numVert*4);
  mesh.vTexCoord.resize(numVert*2);
  mesh.triIndices.resize(numIndices);
  mesh.matIndices.resize(numIndices/3);

  float4* vertPos  = (float4*)&mesh.vPos[0];
  float4* vertNorm = (float4*)&mesh.vNorm[0];
  float2* vertTexc = (float2*)&mesh.vTexCoord[0];

  auto* indices    = &mesh.triIndices[0];
  int* mindices    = &mesh.matIndices[0];

  for (int i = 0; i < numIndices / 3; i++)
    mindices[i] = a_matId;

  const float4 norm = float4(0, -1, 0, 0);

  // disk center
  //
  vertPos [LAST_VERT] = float4(0, 0, 0, 1);
  vertNorm[LAST_VERT] = norm;
  vertTexc[LAST_VERT] = float2(0, 0);

  // disk perimeter
  //
  const float R  = a_radius;
  const float PI = 3.14159265358979323846f;

  for (int i = 0; i < numVertOld; i++)
  {
    float angle = 2.0f*PI*float(i) / float(numVertOld);

    float2 pos;
    pos.x = cosf(angle);
    pos.y = sinf(angle);

    vertPos [i] = float4(pos.x*R, 0, pos.y*R, 1);
    vertNorm[i] = norm;
  }

  for (int i = 0; i < numVertOld; i++)
  {
    indices[i * 3 + 0] = LAST_VERT;
    indices[i * 3 + 1] = i;
    indices[i * 3 + 2] = i + 1;
  }

  indices[numIndicesOld + 0] = LAST_VERT;
  indices[numIndicesOld + 1] = numVertOld - 1;
  indices[numIndicesOld + 2] = 0;
 
  return mesh;
}

static HR_SimpleMesh CreateCylinderMeshForLight(int a_matId, float a_radius, float a_height, float a_angle, int a_numberSlices)
{
  const float DEG_TO_RAD     = float(3.14159265358979323846f) / 180.0f;
  const float partOfCircle   = a_angle / 360.0f;

  const int numberSliceZ = a_numberSlices;
  int numberSliceX = int(float(numberSliceZ)*partOfCircle);
  if (numberSliceX < 6) numberSliceX = 6;

  const int numVert    = (numberSliceZ+1)*(numberSliceX+1);
  const int numIndices = numberSliceZ*numberSliceX*6;

  HR_SimpleMesh mesh;

  mesh.vPos.resize(numVert * 4);
  mesh.vNorm.resize(numVert * 4);
  mesh.vTexCoord.resize(numVert * 2);
  mesh.triIndices.resize(numIndices);
  mesh.matIndices.resize(numIndices / 3);

  float4* vertPos  = (float4*)&mesh.vPos[0];
  float4* vertNorm = (float4*)&mesh.vNorm[0];
  float2* vertTexc = (float2*)&mesh.vTexCoord[0];

  auto* indices = &mesh.triIndices[0];
  int* mindices = &mesh.matIndices[0];

  for (int i = 0; i < numIndices / 3; i++)
    mindices[i] = a_matId;

  const float sizeZ = a_height;
  //const float sizeX = a_radius; // *2.0f*float(M_PI);

  for (int vertX = 0; vertX < numberSliceX+1; vertX++)
  {
    const int pitchZ = numberSliceZ + 1;
    for (int vertZ = 0; vertZ < numberSliceZ+1; vertZ++)
    {
      const float tx = ( float(vertX) / float(numberSliceX) );
      const float tz = ( float(vertZ) / float(numberSliceZ) );

      const float angle = tx*a_angle*DEG_TO_RAD;

      const float posX = a_radius*sin(angle);
      const float posY = a_radius*cos(angle);
      const float posZ = (tz - 0.5f)*sizeZ;

      vertPos [vertX*pitchZ + vertZ] = float4(posX, posY, posZ, 1.0f);

      const float3 vnorm = normalize(to_float3(vertPos[vertX*pitchZ + vertZ]) - float3(0,0, posZ));

      vertNorm[vertX*pitchZ + vertZ] = to_float4(vnorm, 0.0f);
      vertTexc[vertX*pitchZ + vertZ] = float2(tz, tx);
    }
  }

  for (int quadX = 0; quadX < numberSliceX; quadX++)
  {
    const int pitchZ = numberSliceZ + 1;
    for (int quadZ = 0; quadZ < numberSliceZ; quadZ++)
    {
      const int quadOffset = (quadX*numberSliceZ + quadZ)*6;

      indices[quadOffset + 0] = (quadX + 0)*pitchZ + quadZ + 0;
      indices[quadOffset + 1] = (quadX + 1)*pitchZ + quadZ + 0;
      indices[quadOffset + 2] = (quadX + 1)*pitchZ + quadZ + 1;

      indices[quadOffset + 3] = (quadX + 0)*pitchZ + quadZ + 0;
      indices[quadOffset + 4] = (quadX + 1)*pitchZ + quadZ + 1;
      indices[quadOffset + 5] = (quadX + 0)*pitchZ + quadZ + 1;
    }
  }

  return mesh;
}

HRMaterialRef HR_UpdateLightMaterial(pugi::xml_node a_lightNode, const std::wstring& lightIdS)
{
  const float3  clr           = HydraXMLHelpers::ReadLightIntensity(a_lightNode);
  const int32_t lightId       = a_lightNode.attribute(L"id").as_int();
  const std::wstring matName  = std::wstring(a_lightNode.attribute(L"name").as_string()) + L"_material";

  pugi::xml_node materials = g_objManager.scnData.m_materialsLib;                              // #TODO: accelerate linear search
  pugi::xml_node matNode   = materials.find_child_by_attribute(L"light_id", lightIdS.c_str()); // #TODO: accelerate linear search

  HRMaterialRef emissiveMtl;
  if (matNode == nullptr)
  {
    emissiveMtl = hrMaterialCreate(matName.c_str());
    hrMaterialOpen(emissiveMtl, HR_WRITE_DISCARD);
    {
      auto emNode = hrMaterialParamNode(emissiveMtl);
      if(a_lightNode.child(L"sky_portal") != nullptr)
        emNode.attribute(L"type") = L"sky_portal_mtl";
      auto emissColor = emNode.append_child(L"emission").append_child(L"color");
      auto valueAttr  = emissColor.append_attribute(L"val");
      HydraXMLHelpers::WriteFloat3(valueAttr, clr);

      emNode.force_attribute(L"light_id").set_value(lightId);          // reference from material to related light
      emNode.force_attribute(L"visible").set_value(a_lightNode.attribute(L"visible").as_int());
    }
    hrMaterialClose(emissiveMtl);
  }
  else
  {
    emissiveMtl.id = matNode.attribute(L"id").as_int();
    hrMaterialOpen(emissiveMtl, HR_OPEN_EXISTING);
    {
      auto emNode     = hrMaterialParamNode(emissiveMtl);

      if (a_lightNode.child(L"sky_portal") != nullptr)
        emNode.attribute(L"type") = L"sky_portal_mtl";
      else
        emNode.attribute(L"type") = L"hydra_material";

      auto emissColor = emNode.child(L"emission").child(L"color");
      auto valueAttr  = emissColor.attribute(L"val");
      HydraXMLHelpers::WriteFloat3(valueAttr, clr);
    }
    hrMaterialClose(emissiveMtl);
  }

  return emissiveMtl;
}

HRMeshRef HR_UpdateLightMesh(const std::wstring& a_meshName, const HR_SimpleMesh& lmesh, const std::wstring& lightIdS)
{
  pugi::xml_node geomlib  = g_objManager.scnData.m_geometryLib;                             // #TODO: accelerate linear search
  pugi::xml_node geomNode = geomlib.find_child_by_attribute(L"light_id", lightIdS.c_str()); // #TODO: accelerate linear search

  HRMeshRef lightMesh;
  
  if (geomNode == nullptr)
    lightMesh = hrMeshCreate(a_meshName.c_str());
  else
    lightMesh.id = geomNode.attribute(L"id").as_int();

  hrMeshOpen(lightMesh, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    pugi::xml_node  meshNode = hrMeshParamNode(lightMesh);
    meshNode.force_attribute(L"light_id") = lightIdS.c_str();

    hrMeshVertexAttribPointer4f   (lightMesh, L"pos",      &lmesh.vPos[0]);
    hrMeshVertexAttribPointer4f   (lightMesh, L"norm",     &lmesh.vNorm[0]);
    hrMeshVertexAttribPointer2f   (lightMesh, L"texcoord", &lmesh.vTexCoord[0]);
    hrMeshPrimitiveAttribPointer1i(lightMesh, L"mind",     &lmesh.matIndices[0]);

    hrMeshAppendTriangles3(lightMesh, int(lmesh.triIndices.size()), (const int*)&lmesh.triIndices[0]);
  }
  hrMeshClose(lightMesh);

  return lightMesh;
}

static inline bool HR_LightHaveShape(const std::wstring& a_shape)
{
  if (a_shape == L"rect"   || a_shape == L"disk" ||
      a_shape == L"sphere" || a_shape == L"cylinder" || a_shape == L"mesh")
    return true;
  else
    return false;
}

void OpenHRMesh(HRMesh* pMesh, pugi::xml_node nodeXml);

bool HR_UpdateLightGeomAndMaterial(pugi::xml_node a_lightNode, const std::wstring& a_shape)
{
  //const float3  clr           = HydraXMLHelpers::ReadLightIntensity(a_lightNode);
  //const int32_t lightId       = a_lightNode.attribute(L"id").as_int();
  const std::wstring lightIdS = a_lightNode.attribute(L"id").as_string();

  // update light material (1)
  //
  HRMaterialRef emissiveMtl = HR_UpdateLightMaterial(a_lightNode, lightIdS);

  a_lightNode.force_attribute(L"mat_id").set_value(emissiveMtl.id);  // reference from light to it's material 

  // update light mesh (2)
  //
  HR_SimpleMesh lmesh;
  {
    if (a_shape == L"rect")
    {
      const float2 size = HydraXMLHelpers::ReadRectLightSize(a_lightNode);
      lmesh = CreateRectMeshForLight(emissiveMtl.id, size);
    }
    else if (a_shape == L"disk")
    {
      const float radius = HydraXMLHelpers::ReadSphereOrDiskLightRadius(a_lightNode);
      lmesh = CreateDiskMeshForLight(emissiveMtl.id, radius);
    }
    else if (a_shape == L"sphere")
    {
      const float radius = HydraXMLHelpers::ReadSphereOrDiskLightRadius(a_lightNode);
      lmesh = CreateSphereMeshForLight(emissiveMtl.id, radius, 50);
    }
    else if (a_shape == L"cylinder")
    {
      const float radius = HydraXMLHelpers::ReadSphereOrDiskLightRadius(a_lightNode);
      const float height = a_lightNode.child(L"size").attribute(L"height").as_float();

      float angle = a_lightNode.child(L"size").attribute(L"angle").as_float();
      if (a_lightNode.child(L"size").attribute(L"angle") == nullptr)
        angle = 360.0f;

      lmesh = CreateCylinderMeshForLight(emissiveMtl.id, radius, height, angle, 50);
    }
    else if (a_shape == L"mesh")
    {
      int meshId = -1;
      if (a_lightNode.child(L"mesh").attribute(L"id") != nullptr)
        meshId = a_lightNode.child(L"mesh").attribute(L"id").as_int();

      if (meshId >= 0)
      {
        HRMesh& mesh           = g_objManager.scnData.meshes[meshId];
        pugi::xml_node nodeXMl = mesh.xml_node();
        OpenHRMesh(&mesh, nodeXMl);

        lmesh.vPos       = mesh.m_input.verticesPos;
        lmesh.vNorm      = mesh.m_input.verticesNorm;
        lmesh.vTexCoord  = mesh.m_input.verticesTexCoord;
        lmesh.triIndices = mesh.m_input.triIndices;

        lmesh.matIndices.resize(mesh.m_input.matIndices.size());
        for (size_t i = 0; i < lmesh.matIndices.size(); i++)
          lmesh.matIndices[i] = emissiveMtl.id;

        mesh.m_input.freeMem();
      }
      else
      {
        HrError(L"HR_UpdateLightGeomAndMaterial, bad input mesh for light mesh shape");
        return false;
      }
    }
    else
    {
      //Error(L"HR_InsertLightGeom, unknown shape ", a_shape.c_str());
      return false;
    }
  }

  const std::wstring meshName = std::wstring(a_lightNode.attribute(L"name").as_string()) + L"_lightmesh";

  HRMeshRef lightMesh = HR_UpdateLightMesh(meshName, lmesh, lightIdS);

  a_lightNode.force_attribute(L"mesh_id").set_value(lightMesh.id); // reference from light to mesh

  return true;
}


void HR_UpdateLightsGeometryAndMaterial(pugi::xml_node a_lightLib, pugi::xml_node a_sceneInstances)
{
  // iterate lights lib to fill lightsHash for ALL (!) lights (there can be lights that were instanced but were not changed in the last commit !!!)
  //
  std::unordered_map<int32_t, pugi::xml_node> lightsHash;  // #TODO: form global hashes before each commit operation. Opt. this crap.
 
  // iterate lights libChange to form lighNodes and lightsHash; Override (!!!) references in lightsHash with new ones that were changed in the last commit.
  //
  std::vector<pugi::xml_node> lighNodes; lighNodes.reserve(1000);
  //for (pugi::xml_node lightNode = a_lightLibChanges.first_child(); lightNode != nullptr; lightNode = lightNode.next_sibling())
  for(auto& light : g_objManager.scnData.lights)
  {
    const int32_t lightId = light.id; //int32_t lightId = lightNode.attribute(L"id").as_int();
    const auto lightNode  = light.xml_node();
    if(light.wasChanged)
      lighNodes.push_back(lightNode);
  }

  // insert light mesh
  //
  for(pugi::xml_node lightNode : lighNodes)
  {
    // if (lightNode.attribute(L"mesh_id") != nullptr) // we already add geometry for this light in past
    //   continue;

    // #TODO: optimize this. need to ckeck that we are going to change light geometry 
    // if we don't need to cgange geometry, continue
    // need to compare geometry nodes from LibChanges and oldLib
    //
		if (lightNode.attribute(L"visible") != nullptr && (lightNode.attribute(L"visible").as_int() == 0)) // don't add geom for invisible lights
		{
      //// sky portals should not be invisiable because their visibility is implemented via spetial material and flags
      //
			bool isSkyPortal = (lightNode.child(L"sky_portal").attribute(L"val").as_int() == 1);
			if(!isSkyPortal)
			  continue;
		}

    //const int32_t     lightId   = lightNode.attribute(L"id").as_int();
    const std::wstring lshape   = lightNode.attribute(L"shape").as_string();

    HR_UpdateLightGeomAndMaterial(lightNode, lshape);
  }

  // list all light_instances
  //
  struct LightInstance
  {
    LightInstance() : matrixStr(L""), meshId(-1), lightId(-1), lightInstId(-1){}
    LightInstance(const wchar_t* a_mat, int32_t a_meshId, int32_t a_lightId, int32_t a_lightInstId) : 
                  matrixStr(a_mat), meshId(a_meshId), lightId(a_lightId), lightInstId(a_lightInstId) { }

    std::wstring matrixStr;
    int32_t      meshId;
    int32_t      lightId;
    int32_t      lightInstId;
  };

  std::vector<LightInstance> instToAdd; 
  instToAdd.reserve(100);

  int32_t nextInstId  = 0;
  std::unordered_map<int32_t, pugi::xml_node> linstHash;
 
  for (pugi::xml_node inst = a_sceneInstances.first_child(); inst != nullptr; inst = inst.next_sibling())
  {
    if (std::wstring(inst.name()) == L"instance_light")
    {
      const wchar_t* matrixStr = inst.attribute(L"matrix").as_string();
      const int32_t  lightId   = inst.attribute(L"light_id").as_int();
      const int32_t  instId    = inst.attribute(L"id").as_int();


      const pugi::xml_node lightNode = g_objManager.scnData.lights[lightId].xml_node();
      const int32_t meshId           = lightNode.attribute(L"mesh_id").as_int();                                            
      const std::wstring lshape      = lightNode.attribute(L"shape").as_string();

      const bool invisiable = lightNode.attribute(L"visible") != nullptr && (lightNode.attribute(L"visible").as_int() == 0);
      bool isSkyPortal      = (lightNode.child(L"sky_portal").attribute(L"val").as_int() == 1);

      //// sky portals should not be invisiable because their visibility is implemented via spetial material and flags
      //
      if((HR_LightHaveShape(lshape) && !invisiable) || isSkyPortal)
        instToAdd.push_back(LightInstance(matrixStr, meshId, lightId, instId));
    }
    else
    {
      if (inst.attribute(L"linst_id") != nullptr)
      {
        const int32_t linstId = inst.attribute(L"linst_id").as_int();
        linstHash[linstId]    = inst;
      }

      nextInstId++;
    }
  }

  // insert light mesh instance
  //
  HRSceneInstRef scnref;
  scnref.id         = g_objManager.m_currSceneId;
  HRSceneInst* pScn = g_objManager.PtrById(scnref);

  pugi::xml_node sceneNode = pScn->xml_node();

  for (auto& instData : instToAdd)
  {
    if (linstHash.find(instData.lightInstId) != linstHash.end()) // for this light instance we already insert geometry instance in the past
      continue;

    pugi::xml_node nodeXML = sceneNode.append_child(L"instance");

    nodeXML.append_attribute(L"id").set_value(nextInstId);
    nodeXML.append_attribute(L"mesh_id").set_value(instData.meshId);
    nodeXML.append_attribute(L"rmap_id").set_value(-1);
    nodeXML.append_attribute(L"matrix").set_value(instData.matrixStr.c_str());
    nodeXML.append_attribute(L"light_id").set_value(instData.lightId);
    nodeXML.append_attribute(L"linst_id").set_value(instData.lightInstId);

    // add instances to pScn->drawList also for correct update call (!!!)
    //
    HRSceneInst::Instance model;
    model.meshId          = instData.meshId;
    model.lightId         = instData.lightId;
    model.lightInstId     = instData.lightInstId;
    model.remapListId = -1;

    HydraXMLHelpers::ReadMatrix4x4(nodeXML, L"matrix", model.m);
    pScn->drawList.push_back(model);

    nextInstId++;
  }

}


HAPI HRLightRef hrFindLightByName(const wchar_t *a_lightName)
{
  HRLightRef light;

  if(a_lightName != nullptr)
  {
    for (auto lgt : g_objManager.scnData.lights)
    {
      if (lgt.name == std::wstring(a_lightName))
      {
        light.id = lgt.id;
        break;
      }
    }
  }

  if(light.id == -1)
  {
    std::wstringstream ss;
    ss << L"hrLightFindByName: can't find light \"" << a_lightName << "\"";
    HrError(ss.str());
  }

  return light;
}
