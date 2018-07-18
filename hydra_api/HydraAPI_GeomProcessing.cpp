//
// Created by vsan on 23.05.18.
//
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
#include <complex>
#include <set>
#include <algorithm>

#include "LiteMath.h"
using namespace HydraLiteMath;

#include "HydraObjectManager.h"
#include "HydraVSGFExport.h"
#include "HydraXMLHelpers.h"
#include "HydraTextureUtils.h"

extern std::wstring      g_lastError;
extern std::wstring      g_lastErrorCallerPlace;
extern HR_ERROR_CALLBACK g_pErrorCallback;
extern HRObjectManager   g_objManager;


void doDisplacement(HRMesh *pMesh, const pugi::xml_node &displaceXMLNode, std::vector<uint3> &triangleList);


bool meshHasDisplacementMat(HRMeshRef a_mesh, pugi::xml_node &displaceXMLNode)
{
  HRMesh *pMesh = g_objManager.PtrById(a_mesh);
  if (pMesh == nullptr)
  {
    HrError(L"meshHasDisplacementMat: nullptr input");
    return false;
  }

  HRMesh::InputTriMesh &mesh = pMesh->m_input;

  std::set<int32_t> uniqueMatIndices;
  for(auto mI : mesh.matIndices)
  {
    auto ins = uniqueMatIndices.insert(mI);
    if (ins.second)
    {
      HRMaterialRef tmpRef;
      tmpRef.id = mI;
      auto mat = g_objManager.PtrById(tmpRef);
      if (mat != nullptr)
      {
        auto d_node = mat->xml_node_next().child(L"displacement");

        if (d_node.attribute(L"type").as_string() == std::wstring(L"true_displacement"))
        {
          displaceXMLNode = d_node;
          return true;
        }
      }
    }
  }

  return false;
}

void hrMeshDisplace(HRMeshRef a_mesh)
{
  HRMesh *pMesh = g_objManager.PtrById(a_mesh);
  if (pMesh == nullptr)
  {
    HrError(L"hrMeshDisplace: nullptr input");
    return;
  }

  HRMesh::InputTriMesh &mesh = pMesh->m_input;

  const int vertexCount = int(mesh.verticesPos.size() / 4);
  const int triangleCount = int(mesh.triIndices.size() / 3);

  std::set<int32_t> uniqueMatIndices;
  std::unordered_map<int32_t, pugi::xml_node> matsWithDisplacement;
  std::unordered_map<int, std::pair<pugi::xml_node, std::vector<uint3> > > dMatToTriangles;
  for(unsigned int i = 0; i < mesh.matIndices.size(); ++i)
  {
    int mI = mesh.matIndices.at(i);
    auto ins = uniqueMatIndices.insert(mI);
    if(ins.second)
    {
      HRMaterialRef tmpRef;
      tmpRef.id = mI;
      auto mat = g_objManager.PtrById(tmpRef);
      if(mat != nullptr)
      {
        auto d_node = mat->xml_node_next().child(L"displacement");

        if (d_node != nullptr &&
            std::wstring(d_node.attribute(L"type").as_string()) == std::wstring(L"true_displacement"))
        {
          matsWithDisplacement[mI] = d_node;
        }
      }
    }
    auto mat = matsWithDisplacement.find(mI);
    if(mat != matsWithDisplacement.end())
    {
      uint3 triangle = uint3(mesh.triIndices.at(i * 3 + 0),
                             mesh.triIndices.at(i * 3 + 1),
                             mesh.triIndices.at(i * 3 + 2));

      if (dMatToTriangles.find(mI) == dMatToTriangles.end())
      {
        std::vector<uint3> tmp;
        tmp.push_back(triangle);
        dMatToTriangles[mI] = std::pair<pugi::xml_node, std::vector<uint3> >(mat->second, tmp);
      } else
      {
        dMatToTriangles[mI].second.push_back(triangle);
      }
    }
  }

  /*for(auto& dTris : dMatToTriangles)
  {
    std::cout << "id : " << dTris.first << " triangles : " << dTris.second.second.size() <<std::endl;
  }*/

  //#pragma omp parallel for
  for(auto& dTris : dMatToTriangles)
  {
    doDisplacement(pMesh, dTris.second.first, dTris.second.second);
  }


  hrMeshComputeNormals(a_mesh, mesh.triIndices.size(), false);
}

float smoothing_coeff(uint32_t valence)
{
  return (4.0f - 2.0f * cosf(float(2.0f * 3.14159265358979323846f) / valence )) / 9.0f;
}

std::vector<uint32_t> find_vertex_neighbours(int vertex_index, const HRMesh::InputTriMesh& mesh)
{
  std::set<uint32_t> neighbours;
  for(int i = 0; i < mesh.triIndices.size(); i += 3)
  {
    uint32_t indA = mesh.triIndices[i + 0];
    uint32_t indB = mesh.triIndices[i + 1];
    uint32_t indC = mesh.triIndices[i + 2];

    if(vertex_index == indA)
    {
      neighbours.insert(indB);
      neighbours.insert(indB);
      neighbours.insert(indC);
    }
    else if(vertex_index == indB)
    {
      neighbours.insert(indA);
      neighbours.insert(indC);
    }
    else if(vertex_index == indC)
    {
      neighbours.insert(indA);
      neighbours.insert(indB);
    }
  }
  std::vector<uint32_t> res;
  res.assign(neighbours.begin(), neighbours.end());

  return res;
}

float4 vertex_attrib_by_index_f4(const std::string &attrib_name, uint32_t vertex_index, const HRMesh::InputTriMesh& mesh)
{
  float4 res;
  if(attrib_name == "pos")
  {
    res = float4(mesh.verticesPos.at(vertex_index * 4 + 0), mesh.verticesPos.at(vertex_index * 4 + 1),
                 mesh.verticesPos.at(vertex_index * 4 + 2), mesh.verticesPos.at(vertex_index * 4 + 3));
  }
  else if(attrib_name == "normal")
  {
    res = float4(mesh.verticesNorm.at(vertex_index * 4 + 0), mesh.verticesNorm.at(vertex_index * 4 + 1),
                 mesh.verticesNorm.at(vertex_index * 4 + 2), mesh.verticesNorm.at(vertex_index * 4 + 3));
  }
  else if(attrib_name == "tangent")
  {
    res = float4(mesh.verticesTangent.at(vertex_index * 4 + 0), mesh.verticesTangent.at(vertex_index * 4 + 1),
                 mesh.verticesTangent.at(vertex_index * 4 + 2), mesh.verticesTangent.at(vertex_index * 4 + 3));
  }

  return res;
}

void update_vertex_attrib_by_index_f4(float4 new_val, uint32_t vertex_index, std::vector <float> &attrib_vec)
{
  attrib_vec.at(vertex_index * 4 + 0) = new_val.x;
  attrib_vec.at(vertex_index * 4 + 1) = new_val.y;
  attrib_vec.at(vertex_index * 4 + 2) = new_val.z;
  attrib_vec.at(vertex_index * 4 + 3) = new_val.w;
}

float2 vertex_attrib_by_index_f2(const std::string &attrib_name, uint32_t vertex_index, const HRMesh::InputTriMesh& mesh)
{
  float2 res;
  if(attrib_name == "uv")
  {
    res = float2(mesh.verticesTexCoord.at(vertex_index * 2 + 0), mesh.verticesTexCoord.at(vertex_index * 2 + 1));
  }

  return res;
}

void update_vertex_attrib_by_index_f2(float2 new_val, uint32_t vertex_index, std::vector <float> &attrib_vec)
{
  attrib_vec.at(vertex_index * 2 + 0) = new_val.x;
  attrib_vec.at(vertex_index * 2 + 1) = new_val.y;
}

void smooth_common_vertex_attributes(uint32_t vertex_index, const HRMesh::InputTriMesh& mesh, float4 &pos, float4 &normal,
                                     float4 &tangent, float2 &uv)
{
  auto neighbours = find_vertex_neighbours(vertex_index, mesh);
  uint32_t valence = neighbours.size();

  pos      = vertex_attrib_by_index_f4("pos", vertex_index, mesh);
  normal   = vertex_attrib_by_index_f4("normal", vertex_index, mesh);
  tangent  = vertex_attrib_by_index_f4("tangent", vertex_index, mesh);
  uv       = vertex_attrib_by_index_f2("uv", vertex_index, mesh);

  //only handle ordinary vertices for now
  if(valence == 6)
  {
    float4 pos_;
    float4 norm_;
    float4 tangent_;
    float2 uv_;

    for (const auto &n : neighbours)
    {
      float4 pos_n = vertex_attrib_by_index_f4("pos", n, mesh);
      float4 norm_n = vertex_attrib_by_index_f4("normal", n, mesh);
      float4 tan_n = vertex_attrib_by_index_f4("tangent", n, mesh);
      float2 uv_n = vertex_attrib_by_index_f2("uv", n, mesh);

      pos_ += pos_n;
      norm_ += norm_n;
      tangent_ += tan_n;
      uv_ += uv_n;
    }

    float alpha = smoothing_coeff(valence);

    pos = (1.0f - alpha) * pos + (alpha / valence) * pos_;
    normal = (1.0f - alpha) * normal + (alpha / valence) * norm_;
    tangent = (1.0f - alpha) * tangent + (alpha / valence) * tangent_;
    uv = (1.0f - alpha) * uv + (alpha / valence) * uv_;
  }
}


void addEdge(uint32_t indA, uint32_t indB, uint32_t faceInd, std::unordered_map<uint2, std::vector<uint32_t>, uint2_hash> &edgeToTris)
{
  uint2 edge1(indA, indB);
  uint2 edge2(indB, indA);

  if(edgeToTris.find(edge1) != edgeToTris.end())
  {
    edgeToTris[edge1].push_back(faceInd);
  }
  else if(edgeToTris.find(edge2) != edgeToTris.end())
  {
    edgeToTris[edge2].push_back(faceInd);
  }
  else
  {
    std::vector<uint32_t> tmp = {faceInd};
    edgeToTris[edge1] = tmp;
  }
}

//simplified sqrt3 uniform subdivision
void hrMeshSubdivideSqrt3(HRMeshRef a_mesh, int a_iterations)
{
  HRMesh *pMesh = g_objManager.PtrById(a_mesh);
  if (pMesh == nullptr)
  {
    HrError(L"hrMeshSubdivideSqrt3: nullptr input");
    return;
  }

  HRMesh::InputTriMesh &mesh = pMesh->m_input;

  for(int i = 0; i < a_iterations; ++i)
  {

    const uint32_t old_vertex_count = uint32_t(mesh.verticesPos.size() / 4);
    const uint32_t old_tri_count = uint32_t(mesh.triIndices.size() / 3);

    std::vector<uint32_t> indices;
    indices.reserve(mesh.triIndices.size() * 3);
    std::vector<uint32_t> mat_indices;
    mat_indices.reserve(mesh.triIndices.size() * 3 / 3);

    std::unordered_map<uint2, std::vector<uint32_t>, uint2_hash> edgeToTris;

    uint32_t face_num = 0;
    //insert middle point
    for (int j = 0; j < mesh.triIndices.size(); j += 3)
    {
      uint32_t indA = mesh.triIndices[j + 0];
      uint32_t indB = mesh.triIndices[j + 1];
      uint32_t indC = mesh.triIndices[j + 2];

      addEdge(indA, indB, face_num, edgeToTris);
      addEdge(indB, indC, face_num, edgeToTris);
      addEdge(indC, indA, face_num, edgeToTris);


      float4 A = vertex_attrib_by_index_f4("pos", indA, mesh);
      float4 B = vertex_attrib_by_index_f4("pos", indB, mesh);
      float4 C = vertex_attrib_by_index_f4("pos", indC, mesh);

      float4 ANorm = vertex_attrib_by_index_f4("normal", indA, mesh);
      float4 BNorm = vertex_attrib_by_index_f4("normal", indB, mesh);
      float4 CNorm = vertex_attrib_by_index_f4("normal", indC, mesh);

      float4 ATan = vertex_attrib_by_index_f4("tangent", indA, mesh);
      float4 BTan = vertex_attrib_by_index_f4("tangent", indB, mesh);
      float4 CTan = vertex_attrib_by_index_f4("tangent", indC, mesh);

      float2 Auv = vertex_attrib_by_index_f2("uv", indA, mesh);
      float2 Buv = vertex_attrib_by_index_f2("uv", indB, mesh);
      float2 Cuv = vertex_attrib_by_index_f2("uv", indC, mesh);

      float4 P = (A + B + C) / 3.0f;
      float4 PNorm = (ANorm + BNorm + CNorm) / 3.0f;
      float3 PNorm3 = normalize(make_float3(PNorm.x, PNorm.y, PNorm.z));
      PNorm.x = PNorm3.x;
      PNorm.y = PNorm3.y;
      PNorm.z = PNorm.z;
      float4 PTan = (ATan + BTan + CTan) / 3.0f;
      float2 Puv = (Auv + Buv + Cuv) / 3.0f;

      uint32_t indP = mesh.verticesPos.size() / 4;
      mesh.verticesPos.push_back(P.x);
      mesh.verticesPos.push_back(P.y);
      mesh.verticesPos.push_back(P.z);
      mesh.verticesPos.push_back(P.w);

      mesh.verticesNorm.push_back(PNorm.x);
      mesh.verticesNorm.push_back(PNorm.y);
      mesh.verticesNorm.push_back(PNorm.z);
      mesh.verticesNorm.push_back(PNorm.w);

      mesh.verticesTangent.push_back(PTan.x);
      mesh.verticesTangent.push_back(PTan.y);
      mesh.verticesTangent.push_back(PTan.z);
      mesh.verticesTangent.push_back(PTan.w);

      mesh.verticesTexCoord.push_back(Puv.x);
      mesh.verticesTexCoord.push_back(Puv.y);

      face_num++;
    }

    //flip edges
    for (const auto &edge : edgeToTris)
    {
      if (edge.second.size() == 2)
      {
        uint32_t center1 = (old_vertex_count + edge.second[0]);
        uint32_t center2 = (old_vertex_count + edge.second[1]);
        uint32_t A = edge.first.x;
        uint32_t B = edge.first.y;

        indices.push_back(center1);
        indices.push_back(center2);
        indices.push_back(B);
        mat_indices.push_back(mesh.matIndices[edge.second[0]]);

        indices.push_back(center2);
        indices.push_back(center1);
        indices.push_back(A);
        mat_indices.push_back(mesh.matIndices[edge.second[1]]);
      } else if (edge.second.size() == 1)
      {
        uint32_t center = (old_vertex_count + edge.second[0]);
        uint32_t A = edge.first.x;
        uint32_t B = edge.first.y;

        indices.push_back(center);
        indices.push_back(A);
        indices.push_back(B);
        mat_indices.push_back(mesh.matIndices[edge.second[0]]);
      }
    }

    std::vector<float> pos_new(old_vertex_count * 4, 0.0f);
    std::vector<float> normal_new(old_vertex_count * 4, 0.0f);
    std::vector<float> tangent_new(old_vertex_count * 4, 0.0f);
    std::vector<float> uv_new(old_vertex_count * 2, 0.0f);

    for (uint32_t k = 0; k < old_vertex_count; ++k)
    {
      float4 pos;
      float4 normal;
      float4 tangent;
      float2 uv;

      smooth_common_vertex_attributes(k, mesh, pos, normal, tangent, uv);

      update_vertex_attrib_by_index_f4(pos, k, pos_new);
      update_vertex_attrib_by_index_f4(normal, k, normal_new);
      update_vertex_attrib_by_index_f4(tangent, k, tangent_new);
      update_vertex_attrib_by_index_f2(uv, k, uv_new);
    }

    for (int ii = 0; ii < pos_new.size(); ++ii)
    {
      mesh.verticesPos.at(ii)     = pos_new.at(ii);
      mesh.verticesNorm.at(ii)    = normal_new.at(ii);
      mesh.verticesTangent.at(ii) = tangent_new.at(ii);

      if (ii < pos_new.size() / 2)
        mesh.verticesTexCoord.at(ii) = uv_new.at(ii);
    }

    mesh.triIndices = indices;
    mesh.matIndices = mat_indices;
  }
}

void hrMeshSubdivide(HRMeshRef a_mesh, int a_iterations)
{
  HRMesh *pMesh = g_objManager.PtrById(a_mesh);
  if (pMesh == nullptr)
  {
    HrError(L"hrMeshSubdivide: nullptr input");
    return;
  }

  HRMesh::InputTriMesh &mesh = pMesh->m_input;

  const int vertexCount = int(mesh.verticesPos.size() / 4);
  const int triangleCount = int(mesh.triIndices.size() / 3);

  std::vector<uint32_t> indices;
  indices.reserve(mesh.triIndices.size() * 3);
  std::vector<uint32_t> mat_indices;
  mat_indices.reserve(mesh.triIndices.size() * 3 / 3);

  int face_num = 0;
  for(int i = 0; i < mesh.triIndices.size(); i += 3)
  {
    uint32_t indA = mesh.triIndices[i + 0];
    uint32_t indB = mesh.triIndices[i + 1];
    uint32_t indC = mesh.triIndices[i + 2];

    float4 A = vertex_attrib_by_index_f4("pos", indA, mesh);
    float4 B = vertex_attrib_by_index_f4("pos", indB, mesh);
    float4 C = vertex_attrib_by_index_f4("pos", indC, mesh);

    float4 ANorm = vertex_attrib_by_index_f4("normal", indA, mesh);
    float4 BNorm = vertex_attrib_by_index_f4("normal", indB, mesh);
    float4 CNorm = vertex_attrib_by_index_f4("normal", indC, mesh);

    float4 ATan = vertex_attrib_by_index_f4("tangent", indA, mesh);
    float4 BTan = vertex_attrib_by_index_f4("tangent", indB, mesh);
    float4 CTan = vertex_attrib_by_index_f4("tangent", indC, mesh);

    float2 Auv = vertex_attrib_by_index_f2("uv", indA, mesh);
    float2 Buv = vertex_attrib_by_index_f2("uv", indB, mesh);
    float2 Cuv = vertex_attrib_by_index_f2("uv", indC, mesh);

    float4 P = (A + B + C) / 3.0f;
    float4 PNorm = (ANorm + BNorm + CNorm) / 3.0f;
    float3 PNorm3 = normalize(make_float3(PNorm.x, PNorm.y, PNorm.z));
    PNorm.x = PNorm3.x; PNorm.y = PNorm3.y; PNorm.z = PNorm.z;
    float4 PTan = (ATan + BTan + CTan) / 3.0f;
    float2 Puv = (Auv + Buv + Cuv) / 3.0f;

    uint32_t indP = mesh.verticesPos.size() / 4;
    mesh.verticesPos.push_back(P.x);
    mesh.verticesPos.push_back(P.y);
    mesh.verticesPos.push_back(P.z);
    mesh.verticesPos.push_back(P.w);

    mesh.verticesNorm.push_back(PNorm.x);
    mesh.verticesNorm.push_back(PNorm.y);
    mesh.verticesNorm.push_back(PNorm.z);
    mesh.verticesNorm.push_back(PNorm.w);

    mesh.verticesTangent.push_back(PTan.x);
    mesh.verticesTangent.push_back(PTan.y);
    mesh.verticesTangent.push_back(PTan.z);
    mesh.verticesTangent.push_back(PTan.w);

    mesh.verticesTexCoord.push_back(Puv.x);
    mesh.verticesTexCoord.push_back(Puv.y);

    indices.push_back(indA); indices.push_back(indB); indices.push_back(indP);
    mat_indices.push_back(mesh.matIndices[face_num]);
    indices.push_back(indB); indices.push_back(indC); indices.push_back(indP);
    mat_indices.push_back(mesh.matIndices[face_num]);
    indices.push_back(indC); indices.push_back(indA); indices.push_back(indP);
    mat_indices.push_back(mesh.matIndices[face_num]);

    face_num++;
  }

  mesh.triIndices = indices;
  mesh.matIndices = mat_indices;
}

void displaceByNoise(HRMesh *pMesh, const pugi::xml_node &noiseXMLNode, std::vector<uint3> &triangleList)
{
  HRMesh::InputTriMesh &mesh = pMesh->m_input;

  float mult = noiseXMLNode.attribute(L"amount").as_float();

  float3 texHeight(0.0f, 0.0f, 0.0f);
  std::set<uint32_t > displaced_indices;
  // #pragma omp parallel for
  for(int i=0;i<triangleList.size();i++)
  {
    const auto& tri = triangleList[i];

    float3 attrib1 = make_float3(vertex_attrib_by_index_f4("pos", tri.x, mesh));
    float3 attrib2 = make_float3(vertex_attrib_by_index_f4("pos", tri.y, mesh));
    float3 attrib3 = make_float3(vertex_attrib_by_index_f4("pos", tri.z, mesh));

    auto ins = displaced_indices.insert(tri.x);
    if(ins.second)
    {
      texHeight.x = sampleNoise(noiseXMLNode, attrib1);
    }

    ins = displaced_indices.insert(tri.y);
    if(ins.second)
    {
      texHeight.y = sampleNoise(noiseXMLNode, attrib2);
    }

    ins = displaced_indices.insert(tri.z);
    if(ins.second)
    {
      texHeight.z = sampleNoise(noiseXMLNode, attrib3);
    }

    mesh.verticesPos.at(tri.x * 4 + 0) += mesh.verticesNorm.at(tri.x * 4 + 0) * mult * texHeight.x;
    mesh.verticesPos.at(tri.x * 4 + 1) += mesh.verticesNorm.at(tri.x * 4 + 1) * mult * texHeight.x;
    mesh.verticesPos.at(tri.x * 4 + 2) += mesh.verticesNorm.at(tri.x * 4 + 2) * mult * texHeight.x;

    mesh.verticesPos.at(tri.y * 4 + 0) += mesh.verticesNorm.at(tri.y * 4 + 0) * mult * texHeight.y;
    mesh.verticesPos.at(tri.y * 4 + 1) += mesh.verticesNorm.at(tri.y * 4 + 1) * mult * texHeight.y;
    mesh.verticesPos.at(tri.y * 4 + 2) += mesh.verticesNorm.at(tri.y * 4 + 2) * mult * texHeight.y;

    mesh.verticesPos.at(tri.z * 4 + 0) += mesh.verticesNorm.at(tri.z * 4 + 0) * mult * texHeight.z;
    mesh.verticesPos.at(tri.z * 4 + 1) += mesh.verticesNorm.at(tri.z * 4 + 1) * mult * texHeight.z;
    mesh.verticesPos.at(tri.z * 4 + 2) += mesh.verticesNorm.at(tri.z * 4 + 2) * mult * texHeight.z;

    texHeight = float3(0.0f, 0.0f, 0.0f);
  }
}

void displaceByHeightMap(HRMesh *pMesh, const pugi::xml_node &heightXMLNode, std::vector<uint3> &triangleList)
{
  HRMesh::InputTriMesh &mesh = pMesh->m_input;

  float mult = heightXMLNode.attribute(L"amount").as_float();

  auto texNode = heightXMLNode.child(L"texture");
  pugi::xml_node texLibNode;
  if(texNode != nullptr)
  {
    auto id = texNode.attribute(L"id").as_string();
    texLibNode = g_objManager.scnData.m_texturesLib.find_child_by_attribute(L"id", id);
  }

  int w = 0;
  int h = 0;
  bool sampleTexture = false;
  bool isLDR = false;
  bool isHDR = false;

  std::vector<int> imageDataLDR;
  std::vector<float> imageDataHDR;
  auto location = texLibNode.attribute(L"loc").as_string();
  float3 texHeight(1.0f, 1.0f, 1.0f);
  float4x4 matrix;
  if(location != L"")
  {
    int bpp = 0;

    HRTextureNodeRef texRef;
    texRef.id = texNode.attribute(L"id").as_int();

    hrTexture2DGetSize(texRef, &w, &h, &bpp);
    if(bpp > 4)
    {
      isHDR = true;
      imageDataHDR.resize(w * h * 4);
      hrTextureNodeOpen(texRef, HR_OPEN_READ_ONLY);
      {
        hrTexture2DGetDataHDR(texRef, &w, &h, &imageDataHDR[0]);
      }
      hrTextureNodeClose(texRef);
    }
    else
    {
      isLDR = true;
      imageDataLDR.resize(w * h);

      hrTextureNodeOpen(texRef, HR_OPEN_READ_ONLY);
      {
        hrTexture2DGetDataLDR(texRef, &w, &h, &imageDataLDR[0]);
      }
      hrTextureNodeClose(texRef);
    }

    float mat[16];
    HydraXMLHelpers::ReadMatrix4x4(texNode, L"matrix", mat);

    matrix = float4x4(mat);

    sampleTexture = true;
    texHeight = float3(0.0f, 0.0f, 0.0f);
  }


  std::set<uint32_t > displaced_indices;
  // #pragma omp parallel for
  for(int i=0;i<triangleList.size();i++)
  {
    const auto& tri = triangleList[i];

    float2 uv1 = vertex_attrib_by_index_f2("uv", tri.x, mesh);
    float2 uv2 = vertex_attrib_by_index_f2("uv", tri.y, mesh);
    float2 uv3 = vertex_attrib_by_index_f2("uv", tri.z, mesh);

    if(sampleTexture)
    {
      auto ins = displaced_indices.insert(tri.x);
      if(ins.second)
      {
        if(isLDR)
          texHeight.x = sampleHeightMapLDR(imageDataLDR, w, h, uv1, matrix);
        if(isHDR)
          texHeight.x = sampleHeightMapHDR(imageDataHDR, w, h, uv1, matrix);
      }

      ins = displaced_indices.insert(tri.y);
      if(ins.second)
      {
        if(isLDR)
          texHeight.y = sampleHeightMapLDR(imageDataLDR, w, h, uv2, matrix);
        if(isHDR)
          texHeight.y = sampleHeightMapHDR(imageDataHDR, w, h, uv2, matrix);
      }

      ins = displaced_indices.insert(tri.z);
      if(ins.second)
      {
        if(isLDR)
          texHeight.z = sampleHeightMapLDR(imageDataLDR, w, h, uv3, matrix);
        if(isHDR)
          texHeight.z = sampleHeightMapHDR(imageDataHDR, w, h, uv3, matrix);
      }
    }

    mesh.verticesPos.at(tri.x * 4 + 0) += mesh.verticesNorm.at(tri.x * 4 + 0) * mult * texHeight.x;
    mesh.verticesPos.at(tri.x * 4 + 1) += mesh.verticesNorm.at(tri.x * 4 + 1) * mult * texHeight.x;
    mesh.verticesPos.at(tri.x * 4 + 2) += mesh.verticesNorm.at(tri.x * 4 + 2) * mult * texHeight.x;

    mesh.verticesPos.at(tri.y * 4 + 0) += mesh.verticesNorm.at(tri.y * 4 + 0) * mult * texHeight.y;
    mesh.verticesPos.at(tri.y * 4 + 1) += mesh.verticesNorm.at(tri.y * 4 + 1) * mult * texHeight.y;
    mesh.verticesPos.at(tri.y * 4 + 2) += mesh.verticesNorm.at(tri.y * 4 + 2) * mult * texHeight.y;

    mesh.verticesPos.at(tri.z * 4 + 0) += mesh.verticesNorm.at(tri.z * 4 + 0) * mult * texHeight.z;
    mesh.verticesPos.at(tri.z * 4 + 1) += mesh.verticesNorm.at(tri.z * 4 + 1) * mult * texHeight.z;
    mesh.verticesPos.at(tri.z * 4 + 2) += mesh.verticesNorm.at(tri.z * 4 + 2) * mult * texHeight.z;

    texHeight = float3(0.0f, 0.0f, 0.0f);
  }
}

void doDisplacement(HRMesh *pMesh, const pugi::xml_node &displaceXMLNode, std::vector<uint3> &triangleList)
{
  HRMesh::InputTriMesh &mesh = pMesh->m_input;

  auto heightNode = displaceXMLNode.child(L"height_map");

  auto noiseNode = displaceXMLNode.child(L"noise");

  if(heightNode != nullptr)
  {
    displaceByHeightMap(pMesh, heightNode, triangleList);
  }
  else if(noiseNode != nullptr)
  {
    displaceByNoise(pMesh, noiseNode, triangleList);
  }

}



void InsertFixedMeshesInfoIntoXML(pugi::xml_document &stateToProcess, std::unordered_map<uint32_t, uint32_t> &meshTofixedMesh)
{
  auto texNode = stateToProcess.child(L"geometry_lib");

  if (texNode != nullptr)
  {
    for (auto& meshMap : meshTofixedMesh)
    {
      auto geoLib = g_objManager.scnData.m_geometryLibChanges;

      auto mesh_node = geoLib.find_child_by_attribute(L"id", std::to_wstring(meshMap.second).c_str());
      texNode.append_copy(mesh_node);

      auto sceneNode = stateToProcess.child(L"scenes").child(L"scene");
      if (sceneNode != nullptr)
      {
        for (auto node = sceneNode.child(L"instance"); node != nullptr; node = node.next_sibling())
        {
          if(node.attribute(L"mesh_id") != nullptr && node.attribute(L"mesh_id").as_int() == meshMap.first)
            node.attribute(L"mesh_id").set_value(meshMap.second);
        }
      }
    }
  }

}


std::wstring HR_PreprocessMeshes(const wchar_t *state_path)
{
  std::wstring new_state_path;

  if (state_path == std::wstring(L"") || state_path == nullptr)
  {
    HrError(L"No state for HR_PreprocessMeshes at location: ", state_path);
    return new_state_path;
  }

  pugi::xml_document stateToProcess;

  auto loadResult = stateToProcess.load_file(state_path);

  if (!loadResult)
  {
    HrError(L"HR_PreprocessMeshes, pugixml load: ", loadResult.description());
    return new_state_path;
  }
  bool anyChanges = false;

  if (g_objManager.m_currSceneId < g_objManager.scnInst.size())
  {
    auto scn = g_objManager.scnInst[g_objManager.m_currSceneId];
    std::unordered_map<uint32_t, uint32_t> meshTofixedMesh;
    for (auto p : scn.meshUsedByDrv)
    {
      HRMeshRef mesh_ref;
      mesh_ref.id = p;

      pugi::xml_node displaceXMLNode;

      hrMeshOpen(mesh_ref, HR_TRIANGLE_IND3, HR_OPEN_READ_ONLY);
      if (meshHasDisplacementMat(mesh_ref, displaceXMLNode))
      {
        HRMesh& mesh = g_objManager.scnData.meshes[p];
        std::vector<float> verticesPos(mesh.m_input.verticesPos);       ///< float4
        std::vector<float> verticesNorm(mesh.m_input.verticesNorm);      ///< float4
        std::vector<float> verticesTexCoord(mesh.m_input.verticesTexCoord);  ///< float2
        std::vector<float> verticesTangent(mesh.m_input.verticesTangent);   ///< float4
        std::vector<uint32_t> triIndices(mesh.m_input.triIndices);        ///< size of 3*triNum
        std::vector<uint32_t> matIndices(mesh.m_input.matIndices);        ///< size of 1*triNum
        auto mesh_name = mesh.name;
        hrMeshClose(mesh_ref);

        HRMeshRef mesh_ref_new = hrMeshCreate(std::wstring(mesh_name + L"_fixed").c_str());
        hrMeshOpen(mesh_ref_new, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
        hrMeshVertexAttribPointer4f(mesh_ref_new, L"pos", &verticesPos[0]);
        hrMeshVertexAttribPointer4f(mesh_ref_new, L"norm", &verticesNorm[0]);
        hrMeshVertexAttribPointer2f(mesh_ref_new, L"texcoord", &verticesTexCoord[0]);
        hrMeshPrimitiveAttribPointer1i(mesh_ref_new, L"mind", (int *) (&matIndices[0]));
        hrMeshAppendTriangles3(mesh_ref_new, int(triIndices.size()), (int *) (&triIndices[0]));

        int subdivs = displaceXMLNode.attribute(L"subdivs").as_int();

        //std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        hrMeshSubdivideSqrt3(mesh_ref_new, max(subdivs, 1));
        //std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        //std::cout << "Subdivision time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" <<std::endl;

        //begin = std::chrono::steady_clock::now();
        hrMeshDisplace(mesh_ref_new);
        //end = std::chrono::steady_clock::now();
        //std::cout << "Displacement time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" <<std::endl;

        hrMeshClose(mesh_ref_new);

        HRMesh *pMesh = g_objManager.PtrById(mesh_ref_new);

        meshTofixedMesh[p] = mesh_ref_new.id;
        anyChanges = true;
      }
      else
      {
        hrMeshClose(mesh_ref);
      }
    }
    if(anyChanges)
      InsertFixedMeshesInfoIntoXML(stateToProcess, meshTofixedMesh);
  }

  if(anyChanges)
    return SaveFixedStateXML(stateToProcess, state_path, L"_meshes");
  else
    return state_path;
}