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

#include <cmath>
#include <cassert>
#include <random>

using std::isnan;
using std::isinf;

#include "LiteMath.h"
using namespace LiteMath;

#include "HydraObjectManager.h"
#include "HydraVSGFExport.h"
#include "HydraXMLHelpers.h"
#include "HydraTextureUtils.h"

#ifdef WIN32
#undef min
#undef max
#endif

extern std::wstring      g_lastError;
extern std::wstring      g_lastErrorCallerPlace;
extern HRObjectManager   g_objManager;

#ifdef WIN32
#undef min
#undef max
#endif


void doDisplacement(HRMesh *pMesh, const pugi::xml_node &displaceXMLNode, std::vector<uint3> &triangleList,
                    const HRUtils::BBox &bbox);


bool meshHasDisplacementMat(HRMeshRef a_mesh, pugi::xml_node &displaceXMLNode)
{
  HRMesh *pMesh = g_objManager.PtrById(a_mesh);
  if (pMesh == nullptr)
  {
    HrError(L"meshHasDisplacementMat: nullptr input");
    return false;
  }

  auto& mesh = pMesh->m_input;

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
        auto d_node = mat->xml_node().child(L"displacement");

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

bool instanceHasDisplacementMat(HRMeshRef a_meshRef, const std::unordered_map<uint32_t, uint32_t> &remapList,
                                pugi::xml_node &displaceXMLNode, HRMaterialRef &matRef)
{
  HRMesh *pMesh = g_objManager.PtrById(a_meshRef);
  if (pMesh == nullptr)
  {
    HrError(L"meshHasDisplacementMat: nullptr input");
    return false;
  }

  auto& mesh = pMesh->m_input;

  std::set<int32_t> uniqueMatIndices;
  for(auto mI : mesh.matIndices)
  {
    if(remapList.find(mI) != remapList.end())
      mI = remapList.at(mI);
    auto ins = uniqueMatIndices.insert(mI);
    if (ins.second)
    {
      //HRMaterialRef tmpRef;
      matRef.id = mI;
      auto mat = g_objManager.PtrById(matRef);
      if (mat != nullptr)
      {
        auto d_node = mat->xml_node().child(L"displacement");

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

void hrMeshDisplace(HRMeshRef a_mesh, const std::unordered_map<uint32_t, uint32_t> &remapList,
                    pugi::xml_document &stateToProcess, const HRUtils::BBox &bbox)
{
  HRMesh *pMesh = g_objManager.PtrById(a_mesh);
  if (pMesh == nullptr)
  {
    HrError(L"hrMeshDisplace: nullptr input");
    return;
  }

  auto& mesh = pMesh->m_input;

  const auto vertexCount   = mesh.VerticesNum();
  const auto triangleCount = mesh.IndicesNum();

  std::set<int32_t> uniqueMatIndices;
  std::unordered_map<int32_t, pugi::xml_node> matsWithDisplacement;
  std::unordered_map<int, std::pair<pugi::xml_node, std::vector<uint3> > > dMatToTriangles;

  for(unsigned int i = 0; i < mesh.matIndices.size(); ++i)
  {
    int mI = mesh.matIndices.at(i);
    if(remapList.find(mI) != remapList.end())
      mI = remapList.at(mI);

    auto ins = uniqueMatIndices.insert(mI);
    if(ins.second)
    {

      HRMaterialRef tmpRef;
      tmpRef.id = mI;
      auto mat = g_objManager.PtrById(tmpRef);
      if(mat != nullptr)
      {
        auto d_node = mat->xml_node().child(L"displacement");

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
      uint3 triangle = uint3(mesh.indices.at(i * 3 + 0),
                             mesh.indices.at(i * 3 + 1),
                             mesh.indices.at(i * 3 + 2));

      if (dMatToTriangles.find(mI) == dMatToTriangles.end())
      {
        std::vector<uint3> tmp;
        tmp.push_back(triangle);
        dMatToTriangles[mI] = std::pair<pugi::xml_node, std::vector<uint3> >(mat->second, tmp);
      }
      else
      {
        dMatToTriangles[mI].second.push_back(triangle);
      }
    }
  }

  /*for(auto& dTris : dMatToTriangles)
  {
    std::cout << "id : " << dTris.first << " triangles : " << dTris.second.second.size() <<std::endl;
  }*/

  for(auto& dTris : dMatToTriangles)
  {
    doDisplacement(pMesh, dTris.second.first, dTris.second.second, bbox);
  }


  hrMeshComputeNormals(a_mesh, int(mesh.indices.size()), false);
}

float smoothing_coeff(uint32_t valence)
{
  return (4.0f - 2.0f * cosf((2.0f * 3.14159265358979323846f) / valence )) / 9.0f;
}

std::vector<uint32_t> find_vertex_neighbours(int vertex_index, const cmesh_hapi::SimpleMesh& mesh)
{
  std::set<uint32_t> neighbours;
  for(int i = 0; i < mesh.indices.size(); i += 3)
  {
    uint32_t indA = mesh.indices[i + 0];
    uint32_t indB = mesh.indices[i + 1];
    uint32_t indC = mesh.indices[i + 2];

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

void update_vertex_attrib_by_index_f4(float4 new_val, uint32_t vertex_index, std::vector <float> &attrib_vec)
{
  attrib_vec.at(vertex_index * 4 + 0) = new_val.x;
  attrib_vec.at(vertex_index * 4 + 1) = new_val.y;
  attrib_vec.at(vertex_index * 4 + 2) = new_val.z;
  attrib_vec.at(vertex_index * 4 + 3) = new_val.w;
}


void update_vertex_attrib_by_index_f2(float2 new_val, uint32_t vertex_index, std::vector <float> &attrib_vec)
{
  attrib_vec.at(vertex_index * 2 + 0) = new_val.x;
  attrib_vec.at(vertex_index * 2 + 1) = new_val.y;
}

namespace cmesh_hapi
{
  float4 vertex_attrib_by_index_f4(const std::string &attrib_name, uint32_t vertex_index, const cmesh_hapi::SimpleMesh& mesh);
  float2 vertex_attrib_by_index_f2(const std::string &attrib_name, uint32_t vertex_index, const cmesh_hapi::SimpleMesh& mesh);
};

void smooth_common_vertex_attributes(uint32_t vertex_index, const cmesh_hapi::SimpleMesh& mesh, float4 &pos, float4 &normal,
                                     float4 &tangent, float2 &uv)
{
  auto neighbours  = find_vertex_neighbours(vertex_index, mesh);
  uint32_t valence = uint32_t(neighbours.size());

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

struct uint2_hash
{
    std::size_t operator()(const uint2& k) const
    {
      using std::size_t;
      using std::hash;
      return ((hash<unsigned int>()(k.x) ^ (hash<unsigned int>()(k.y) << 1u)) >> 1u);
    }
};


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

  auto& mesh = pMesh->m_input;

  for(int i = 0; i < a_iterations; ++i)
  {

    const auto old_vertex_count = mesh.VerticesNum();
    const auto old_tri_count    = mesh.IndicesNum();

    std::vector<uint32_t> indices;
    indices.reserve(mesh.indices.size() * 3);
    std::vector<uint32_t> mat_indices;
    mat_indices.reserve(mesh.indices.size() * 3 / 3);

    std::unordered_map<uint2, std::vector<uint32_t>, uint2_hash> edgeToTris;

    uint32_t face_num = 0;
    //insert middle point
    for (int j = 0; j < mesh.indices.size(); j += 3)
    {
      uint32_t indA = mesh.indices[j + 0];
      uint32_t indB = mesh.indices[j + 1];
      uint32_t indC = mesh.indices[j + 2];

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
      PNorm.z = PNorm3.z; // ?
      float4 PTan = (ATan + BTan + CTan) / 3.0f;
      float2 Puv  = (Auv + Buv + Cuv) / 3.0f;

      uint32_t indP = uint32_t(mesh.vPos4f.size() / 4);
      mesh.vPos4f.push_back(P.x);
      mesh.vPos4f.push_back(P.y);
      mesh.vPos4f.push_back(P.z);
      mesh.vPos4f.push_back(P.w);

      mesh.vNorm4f.push_back(PNorm.x);
      mesh.vNorm4f.push_back(PNorm.y);
      mesh.vNorm4f.push_back(PNorm.z);
      mesh.vNorm4f.push_back(PNorm.w);

      mesh.vTang4f.push_back(PTan.x);
      mesh.vTang4f.push_back(PTan.y);
      mesh.vTang4f.push_back(PTan.z);
      mesh.vTang4f.push_back(PTan.w);

      mesh.vTexCoord2f.push_back(Puv.x);
      mesh.vTexCoord2f.push_back(Puv.y);

      face_num++;
    }

    //flip edges
    for (const auto &edge : edgeToTris)
    {
      if (edge.second.size() == 2)
      {
        uint32_t center1 = uint32_t(old_vertex_count + edge.second[0]);
        uint32_t center2 = uint32_t(old_vertex_count + edge.second[1]);
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
        uint32_t center = uint32_t(old_vertex_count + edge.second[0]);
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

    for (uint32_t ii = 0; ii < pos_new.size(); ++ii)
    {
      mesh.vPos4f.at(ii)  = pos_new.at(ii);
      mesh.vNorm4f.at(ii) = normal_new.at(ii);
      mesh.vTang4f.at(ii) = tangent_new.at(ii);

      if (ii < pos_new.size() / 2)
        mesh.vTexCoord2f.at(ii) = uv_new.at(ii);
    }

    mesh.indices    = indices;
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

  auto& mesh = pMesh->m_input;

//  const auto vertexCount = int(mesh.verticesPos.size() / 4);
//  const auto triangleCount = int(mesh.triIndices.size() / 3);

  std::vector<uint32_t> indices;
  indices.reserve(mesh.indices.size() * 3);
  std::vector<uint32_t> mat_indices;
  mat_indices.reserve(mesh.indices.size() * 3 / 3);

  int face_num = 0;
  for(int i = 0; i < mesh.indices.size(); i += 3)
  {
    uint32_t indA = mesh.indices[i + 0];
    uint32_t indB = mesh.indices[i + 1];
    uint32_t indC = mesh.indices[i + 2];

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
    PNorm.x = PNorm3.x; PNorm.y = PNorm3.y; PNorm.z = PNorm3.z;         // ?
    float4 PTan = (ATan + BTan + CTan) / 3.0f;
    float2 Puv = (Auv + Buv + Cuv) / 3.0f;

    uint32_t indP = uint32_t(mesh.vPos4f.size() / 4);
    mesh.vPos4f.push_back(P.x);
    mesh.vPos4f.push_back(P.y);
    mesh.vPos4f.push_back(P.z);
    mesh.vPos4f.push_back(P.w);

    mesh.vNorm4f.push_back(PNorm.x);
    mesh.vNorm4f.push_back(PNorm.y);
    mesh.vNorm4f.push_back(PNorm.z);
    mesh.vNorm4f.push_back(PNorm.w);

    mesh.vTang4f.push_back(PTan.x);
    mesh.vTang4f.push_back(PTan.y);
    mesh.vTang4f.push_back(PTan.z);
    mesh.vTang4f.push_back(PTan.w);

    mesh.vTexCoord2f.push_back(Puv.x);
    mesh.vTexCoord2f.push_back(Puv.y);

    indices.push_back(indA); indices.push_back(indB); indices.push_back(indP);
    mat_indices.push_back(mesh.matIndices[face_num]);
    indices.push_back(indB); indices.push_back(indC); indices.push_back(indP);
    mat_indices.push_back(mesh.matIndices[face_num]);
    indices.push_back(indC); indices.push_back(indA); indices.push_back(indP);
    mat_indices.push_back(mesh.matIndices[face_num]);

    face_num++;
  }

  mesh.indices    = indices;
  mesh.matIndices = mat_indices;
}


void displaceByNoise(HRMesh *pMesh, const pugi::xml_node &noiseXMLNode, std::vector<uint3> &triangleList)
{
  /*
  HRMesh::InputTriMesh &mesh = pMesh->m_input;

  float mult = noiseXMLNode.attribute(L"amount").as_float();
  float noise_scale = noiseXMLNode.attribute(L"scale").as_float();

  std::set<uint32_t > displaced_indices;

  float4x4 scale_pos = scale4x4(make_float3(noise_scale, noise_scale, noise_scale));


  for(int i = 0; i < triangleList.size(); i++)
  {
    const auto& tri = triangleList[i];

    float3 attrib1 = mul3x3(scale_pos, make_float3(vertex_attrib_by_index_f4("pos", tri.x, mesh)));
    float3 attrib2 = mul3x3(scale_pos, make_float3(vertex_attrib_by_index_f4("pos", tri.y, mesh)));
    float3 attrib3 = mul3x3(scale_pos, make_float3(vertex_attrib_by_index_f4("pos", tri.z, mesh)));

    float3 offset = float3(10.0f, 10.0f, 10.0f);
    
    float3 texHeight(0.0f, 0.0f, 0.0f);
    auto ins = displaced_indices.insert(tri.x);
    if(ins.second)
    {
      texHeight.x = HRTextureUtils::sampleNoise(noiseXMLNode, attrib1 + offset);
      if (texHeight.x < 0.5) texHeight.x = 0.0f;
    }

    ins = displaced_indices.insert(tri.y);
    if(ins.second)
    {
      texHeight.y = HRTextureUtils::sampleNoise(noiseXMLNode, attrib2 + offset);
      if (texHeight.y < 0.5) texHeight.y = 0.0f;
    }

    ins = displaced_indices.insert(tri.z);
    if(ins.second)
    {
      texHeight.z = HRTextureUtils::sampleNoise(noiseXMLNode, attrib3 + offset);
      if (texHeight.z < 0.5) texHeight.z = 0.0f;
    }

    auto normalX = vertex_attrib_by_index_f4("normal", tri.x, mesh);
    mesh.verticesPos.at(tri.x * 4 + 0) += normalX.x * mult * texHeight.x;
    mesh.verticesPos.at(tri.x * 4 + 1) += normalX.y * mult * texHeight.x;
    mesh.verticesPos.at(tri.x * 4 + 2) += normalX.z * mult * texHeight.x;

    auto normalY = vertex_attrib_by_index_f4("normal", tri.y, mesh);
    mesh.verticesPos.at(tri.y * 4 + 0) += normalY.x * mult * texHeight.y;
    mesh.verticesPos.at(tri.y * 4 + 1) += normalY.y * mult * texHeight.y;
    mesh.verticesPos.at(tri.y * 4 + 2) += normalY.z * mult * texHeight.y;

    auto normalZ = vertex_attrib_by_index_f4("normal", tri.z, mesh);
    mesh.verticesPos.at(tri.z * 4 + 0) += normalZ.x * mult * texHeight.z;
    mesh.verticesPos.at(tri.z * 4 + 1) += normalZ.y * mult * texHeight.z;
    mesh.verticesPos.at(tri.z * 4 + 2) += normalZ.z * mult * texHeight.z;

  }
  */
}

void displaceCustom(HRMesh *pMesh, const pugi::xml_node &customNode, std::vector<uint3> &triangleList, const HRUtils::BBox &bbox)
{
  /*
  HRMesh::InputTriMesh &mesh = pMesh->m_input;

  auto texNode = customNode.child(L"texture");
  pugi::xml_node texLibNode;
  HRTextureNode *texture = nullptr;
  if(texNode != nullptr)
  {
    auto id = texNode.attribute(L"id").as_int();
    texture = &g_objManager.scnData.textures[id];
  }

  std::set<uint32_t > displaced_indices;
  for(int i = 0; i < triangleList.size(); i++)
  {
    const auto& tri = triangleList[i];

    float3 pos = make_float3(vertex_attrib_by_index_f4("pos", tri.x, mesh));
    float3 norm = make_float3(vertex_attrib_by_index_f4("normal", tri.x, mesh));
    float displace_vec[3] = {0.0f, 0.0f, 0.0f};

    auto ins = displaced_indices.insert(tri.x);
    if(ins.second)
    {
	  assert(texture != nullptr);
      texture->displaceCallback((const float*)&pos, (const float*)&norm, bbox, displace_vec, texture->customData,
                               texture->customDataSize);
      mesh.verticesPos.at(tri.x * 4 + 0) += displace_vec[0];
      mesh.verticesPos.at(tri.x * 4 + 1) += displace_vec[1];
      mesh.verticesPos.at(tri.x * 4 + 2) += displace_vec[2];
    }

    pos = make_float3(vertex_attrib_by_index_f4("pos", tri.y, mesh));
    norm = make_float3(vertex_attrib_by_index_f4("normal", tri.y, mesh));
    displace_vec[0] = 0.0f, displace_vec[1] = 0.0f, displace_vec[2] = 0.0f;
    ins = displaced_indices.insert(tri.y);
    if(ins.second)
    {
      texture->displaceCallback((const float *) &pos, (const float *) &norm, bbox, displace_vec, texture->customData,
                               texture->customDataSize);
      mesh.verticesPos.at(tri.y * 4 + 0) += displace_vec[0];
      mesh.verticesPos.at(tri.y * 4 + 1) += displace_vec[1];
      mesh.verticesPos.at(tri.y * 4 + 2) += displace_vec[2];
    }

    pos = make_float3(vertex_attrib_by_index_f4("pos", tri.z, mesh));
    norm = make_float3(vertex_attrib_by_index_f4("normal", tri.z, mesh));
    displace_vec[0] = 0.0f, displace_vec[1] = 0.0f, displace_vec[2] = 0.0f;
    ins = displaced_indices.insert(tri.z);
    if(ins.second)
    {
      texture->displaceCallback((const float *) &pos, (const float *) &norm, bbox, displace_vec, texture->customData,
                               texture->customDataSize);
      mesh.verticesPos.at(tri.z * 4 + 0) += displace_vec[0];
      mesh.verticesPos.at(tri.z * 4 + 1) += displace_vec[1];
      mesh.verticesPos.at(tri.z * 4 + 2) += displace_vec[2];
    }
  }
  */
}

void displaceByHeightMap(HRMesh *pMesh, const pugi::xml_node &heightXMLNode, std::vector<uint3> &triangleList)
{
  auto& mesh = pMesh->m_input;
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
  auto location = std::wstring(texLibNode.attribute(L"loc").as_string());
  float3 texHeight(1.0f, 1.0f, 1.0f);
  float4x4 matrix;
  if(!location.empty())
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
          texHeight.x = HRTextureUtils::sampleHeightMapLDR(imageDataLDR, w, h, uv1, matrix);
        if(isHDR)
          texHeight.x = HRTextureUtils::sampleHeightMapHDR(imageDataHDR, w, h, uv1, matrix);
      }

      ins = displaced_indices.insert(tri.y);
      if(ins.second)
      {
        if(isLDR)
          texHeight.y = HRTextureUtils::sampleHeightMapLDR(imageDataLDR, w, h, uv2, matrix);
        if(isHDR)
          texHeight.y = HRTextureUtils::sampleHeightMapHDR(imageDataHDR, w, h, uv2, matrix);
      }

      ins = displaced_indices.insert(tri.z);
      if(ins.second)
      {
        if(isLDR)
          texHeight.z = HRTextureUtils::sampleHeightMapLDR(imageDataLDR, w, h, uv3, matrix);
        if(isHDR)
          texHeight.z = HRTextureUtils::sampleHeightMapHDR(imageDataHDR, w, h, uv3, matrix);
      }
    }

    mesh.vPos4f.at(tri.x * 4 + 0) += mesh.vNorm4f.at(tri.x * 4 + 0) * mult * texHeight.x;
    mesh.vPos4f.at(tri.x * 4 + 1) += mesh.vNorm4f.at(tri.x * 4 + 1) * mult * texHeight.x;
    mesh.vPos4f.at(tri.x * 4 + 2) += mesh.vNorm4f.at(tri.x * 4 + 2) * mult * texHeight.x;

    mesh.vPos4f.at(tri.y * 4 + 0) += mesh.vNorm4f.at(tri.y * 4 + 0) * mult * texHeight.y;
    mesh.vPos4f.at(tri.y * 4 + 1) += mesh.vNorm4f.at(tri.y * 4 + 1) * mult * texHeight.y;
    mesh.vPos4f.at(tri.y * 4 + 2) += mesh.vNorm4f.at(tri.y * 4 + 2) * mult * texHeight.y;

    mesh.vPos4f.at(tri.z * 4 + 0) += mesh.vNorm4f.at(tri.z * 4 + 0) * mult * texHeight.z;
    mesh.vPos4f.at(tri.z * 4 + 1) += mesh.vNorm4f.at(tri.z * 4 + 1) * mult * texHeight.z;
    mesh.vPos4f.at(tri.z * 4 + 2) += mesh.vNorm4f.at(tri.z * 4 + 2) * mult * texHeight.z;

    texHeight = float3(0.0f, 0.0f, 0.0f);
  }
}

static inline float3 abs_f3(const float3 &u) { return make_float3(fabsf(u.x), fabsf(u.y), fabsf(u.z)); }
static inline float3 abs_f3(const float4 &u) { return make_float3(fabsf(u.x), fabsf(u.y), fabsf(u.z)); }

static inline float3 pow_f3(const float3 &u, const float &exp){return make_float3(powf(u.x, exp), powf(u.y, exp), powf(u.z, exp));}
static inline float3 pow_f3(const float4 &u, const float &exp){return make_float3(powf(u.x, exp), powf(u.y, exp), powf(u.z, exp));}

static inline float3 max_f3_scalar(const float3 &u, const float &v){ return make_float3(fmaxf(u.x, v), fmaxf(u.y, v), fmaxf(u.z, v));}

void displaceByHeightMapTriPlanar(HRMesh *pMesh, const pugi::xml_node &heightXMLNode, std::vector<uint3> &triangleList)
{
  auto& mesh = pMesh->m_input;

  float mult = heightXMLNode.attribute(L"amount").as_float();

  auto texNode = heightXMLNode.child(L"textures_hexaplanar");
  pugi::xml_node texLibNodes[6];
  int texIds[6];
  if(texNode != nullptr)
  {
    auto id = texNode.attribute(L"texX").as_string();
    texIds[0] = texNode.attribute(L"texX").as_int();
    texLibNodes[0] = g_objManager.scnData.m_texturesLib.find_child_by_attribute(L"id", id);

    id = texNode.attribute(L"texX2").as_string();
    texIds[1] = texNode.attribute(L"texX2").as_int();
    texLibNodes[1] = g_objManager.scnData.m_texturesLib.find_child_by_attribute(L"id", id);

    id = texNode.attribute(L"texY").as_string();
    texIds[2] = texNode.attribute(L"texY").as_int();
    texLibNodes[2] = g_objManager.scnData.m_texturesLib.find_child_by_attribute(L"id", id);

    id = texNode.attribute(L"texY2").as_string();
    texIds[3] = texNode.attribute(L"texY2").as_int();
    texLibNodes[3] = g_objManager.scnData.m_texturesLib.find_child_by_attribute(L"id", id);

    id = texNode.attribute(L"texZ").as_string();
    texIds[4] = texNode.attribute(L"texZ").as_int();
    texLibNodes[4] = g_objManager.scnData.m_texturesLib.find_child_by_attribute(L"id", id);

    id = texNode.attribute(L"texZ2").as_string();
    texIds[5] = texNode.attribute(L"texZ2").as_int();
    texLibNodes[5] = g_objManager.scnData.m_texturesLib.find_child_by_attribute(L"id", id);
  }

  bool sampleTexture = false;
  bool isLDR = false;
  bool isHDR = false;

  std::vector<std::vector<int> > imageDataLDR(6, std::vector<int>());
  std::vector<std::vector<float> > imageDataHDR(6, std::vector<float>());
  std::wstring locations[6] = {std::wstring(texLibNodes[0].attribute(L"loc").as_string()),
                               std::wstring(texLibNodes[1].attribute(L"loc").as_string()),
                               std::wstring(texLibNodes[2].attribute(L"loc").as_string()),
                               std::wstring(texLibNodes[3].attribute(L"loc").as_string()),
                               std::wstring(texLibNodes[4].attribute(L"loc").as_string()),
                               std::wstring(texLibNodes[5].attribute(L"loc").as_string())};
  float3 texHeight(1.0f, 1.0f, 1.0f);
  float4x4 matrix;
  int2 texSizes[6] = { int2(0,0), int2(0,0), int2(0,0), 
	                   int2(0,0), int2(0,0), int2(0,0) };

  for(int i = 0; i < 6; ++i)
  {
    auto location = locations[i];
    if (!location.empty())
    {
      int bpp = 0;

      HRTextureNodeRef texRef;
      texRef.id = texIds[i];
      int w = 0;
      int h = 0;
      hrTexture2DGetSize(texRef, &w, &h, &bpp);
      texSizes[i].x = w;
      texSizes[i].y = h;
      if (bpp > 4)
      {
        isHDR = true;
        imageDataHDR.at(i).resize(w * h * 4);
        hrTextureNodeOpen(texRef, HR_OPEN_READ_ONLY);
        {
          hrTexture2DGetDataHDR(texRef, &w, &h, &imageDataHDR.at(i)[0]);
        }
        hrTextureNodeClose(texRef);
      }
      else
      {
        isLDR = true;
        imageDataLDR.at(i).resize(w * h);

        hrTextureNodeOpen(texRef, HR_OPEN_READ_ONLY);
        {
          hrTexture2DGetDataLDR(texRef, &w, &h, &imageDataLDR.at(i)[0]);
        }
        hrTextureNodeClose(texRef);
      }

      float mat[16];
      HydraXMLHelpers::ReadMatrix4x4(texNode, L"matrix", mat);

      matrix = float4x4(mat);

      sampleTexture = true;
      texHeight = float3(0.0f, 0.0f, 0.0f);
    }
  }


  std::set<uint32_t > displaced_indices;
  for(int i = 0; i < triangleList.size(); ++i)
  {
    const auto& tri = triangleList[i];

    float4 norm1 = vertex_attrib_by_index_f4("normal", tri.x, mesh);
    float4 norm2 = vertex_attrib_by_index_f4("normal", tri.y, mesh);
    float4 norm3 = vertex_attrib_by_index_f4("normal", tri.z, mesh);

    float4 pos1 = vertex_attrib_by_index_f4("pos", tri.x, mesh);
    float4 pos2 = vertex_attrib_by_index_f4("pos", tri.y, mesh);
    float4 pos3 = vertex_attrib_by_index_f4("pos", tri.z, mesh);
    float sharpness = 10.0f;

    int2 sampledTexSizes_v1[3];
    int2 sampledTexSizes_v2[3];
    int2 sampledTexSizes_v3[3];

    auto& texX_v1_ldr = norm1.x < 0 ? imageDataLDR[0] : imageDataLDR[1];
    auto& texY_v1_ldr = norm1.y < 0 ? imageDataLDR[2] : imageDataLDR[3];
    auto& texZ_v1_ldr = norm1.z < 0 ? imageDataLDR[4] : imageDataLDR[5];

    auto& texX_v2_ldr = norm2.x < 0 ? imageDataLDR[0] : imageDataLDR[1];
    auto& texY_v2_ldr = norm2.y < 0 ? imageDataLDR[2] : imageDataLDR[3];
    auto& texZ_v2_ldr = norm2.z < 0 ? imageDataLDR[4] : imageDataLDR[5];

    auto& texX_v3_ldr = norm3.x < 0 ? imageDataLDR[0] : imageDataLDR[1];
    auto& texY_v3_ldr = norm3.y < 0 ? imageDataLDR[2] : imageDataLDR[3];
    auto& texZ_v3_ldr = norm3.z < 0 ? imageDataLDR[4] : imageDataLDR[5];

    auto& texX_v1_hdr = norm1.x < 0 ? imageDataHDR[0] : imageDataHDR[1];
    auto& texY_v1_hdr = norm1.y < 0 ? imageDataHDR[2] : imageDataHDR[3];
    auto& texZ_v1_hdr = norm1.z < 0 ? imageDataHDR[4] : imageDataHDR[5];

    auto& texX_v2_hdr = norm2.x < 0 ? imageDataHDR[0] : imageDataHDR[1];
    auto& texY_v2_hdr = norm2.y < 0 ? imageDataHDR[2] : imageDataHDR[3];
    auto& texZ_v2_hdr = norm2.z < 0 ? imageDataHDR[4] : imageDataHDR[5];

    auto& texX_v3_hdr = norm3.x < 0 ? imageDataHDR[0] : imageDataHDR[1];
    auto& texY_v3_hdr = norm3.y < 0 ? imageDataHDR[2] : imageDataHDR[3];
    auto& texZ_v3_hdr = norm3.z < 0 ? imageDataHDR[4] : imageDataHDR[5];


    sampledTexSizes_v1[0] = norm1.x < 0 ? texSizes[0] : texSizes[1];
    sampledTexSizes_v1[1] = norm1.y < 0 ? texSizes[2] : texSizes[3];
    sampledTexSizes_v1[2] = norm1.z < 0 ? texSizes[4] : texSizes[5];

    sampledTexSizes_v2[0] = norm2.x < 0 ? texSizes[0] : texSizes[1];
    sampledTexSizes_v2[1] = norm2.y < 0 ? texSizes[2] : texSizes[3];
    sampledTexSizes_v2[2] = norm2.z < 0 ? texSizes[4] : texSizes[5];

    sampledTexSizes_v3[0] = norm3.x < 0 ? texSizes[0] : texSizes[1];
    sampledTexSizes_v3[1] = norm3.y < 0 ? texSizes[2] : texSizes[3];
    sampledTexSizes_v3[2] = norm3.z < 0 ? texSizes[4] : texSizes[5];


    float3 w_v1 = abs_f3(norm1);
    float3 w_v2 = abs_f3(norm2);
    float3 w_v3 = abs_f3(norm3);

    w_v1 = pow_f3(w_v1, sharpness);
    w_v2 = pow_f3(w_v2, sharpness);
    w_v3 = pow_f3(w_v3, sharpness);

    w_v1 = max_f3_scalar(w_v1, 0.00001f) / dot(w_v1, w_v1);
    w_v2 = max_f3_scalar(w_v2, 0.00001f) / dot(w_v2, w_v2);
    w_v3 = max_f3_scalar(w_v3, 0.00001f) / dot(w_v3, w_v3);

    float b_v1 = (w_v1.x + w_v1.y + w_v1.z);
    float b_v2 = (w_v2.x + w_v2.y + w_v2.z);
    float b_v3 = (w_v3.x + w_v3.y + w_v3.z);
    w_v1 = w_v1 / b_v1;
    w_v2 = w_v2 / b_v2;
    w_v3 = w_v3 / b_v3;

    float tex_scale = 1.0f;

    float2 y_uv_v1 = make_float2(pos1.x * tex_scale, pos1.z * tex_scale);
    float2 x_uv_v1 = make_float2(pos1.z * tex_scale, pos1.y * tex_scale);
    float2 z_uv_v1 = make_float2(pos1.x * tex_scale, pos1.y * tex_scale);

    float2 y_uv_v2 = make_float2(pos2.x * tex_scale, pos2.z * tex_scale);
    float2 x_uv_v2 = make_float2(pos2.z * tex_scale, pos2.y * tex_scale);
    float2 z_uv_v2 = make_float2(pos2.x * tex_scale, pos2.y * tex_scale);

    float2 y_uv_v3 = make_float2(pos3.x * tex_scale, pos3.z * tex_scale);
    float2 x_uv_v3 = make_float2(pos3.z * tex_scale, pos3.y * tex_scale);
    float2 z_uv_v3 = make_float2(pos3.x * tex_scale, pos3.y * tex_scale);

    if(sampleTexture)
    {
      auto ins = displaced_indices.insert(tri.x);
      if(ins.second)
      {
        if(isLDR)
        {
          float texColX = HRTextureUtils::sampleHeightMapLDR(texX_v1_ldr, sampledTexSizes_v1[0].x, sampledTexSizes_v1[0].y, x_uv_v1, matrix);
          float texColY = HRTextureUtils::sampleHeightMapLDR(texY_v1_ldr, sampledTexSizes_v1[1].x, sampledTexSizes_v1[1].y, y_uv_v1, matrix);
          float texColZ = HRTextureUtils::sampleHeightMapLDR(texZ_v1_ldr, sampledTexSizes_v1[2].x, sampledTexSizes_v1[2].y, z_uv_v1, matrix);

          texHeight.x = texColX * w_v1.x + texColY * w_v1.y + texColZ * w_v1.z;
        }
        if(isHDR)
        {
          float texColX = HRTextureUtils::sampleHeightMapHDR(texX_v1_hdr, sampledTexSizes_v1[0].x, sampledTexSizes_v1[0].y, x_uv_v1, matrix);
          float texColY = HRTextureUtils::sampleHeightMapHDR(texY_v1_hdr, sampledTexSizes_v1[1].x, sampledTexSizes_v1[1].y, y_uv_v1, matrix);
          float texColZ = HRTextureUtils::sampleHeightMapHDR(texZ_v1_hdr, sampledTexSizes_v1[2].x, sampledTexSizes_v1[2].y, z_uv_v1, matrix);

          texHeight.x = texColX * w_v1.x + texColY * w_v1.y + texColZ * w_v1.z;
        }
      }

      ins = displaced_indices.insert(tri.y);
      if(ins.second)
      {
        if(isLDR)
        {
          float texColX = HRTextureUtils::sampleHeightMapLDR(texX_v2_ldr, sampledTexSizes_v2[0].x, sampledTexSizes_v2[0].y, x_uv_v2, matrix);
          float texColY = HRTextureUtils::sampleHeightMapLDR(texY_v2_ldr, sampledTexSizes_v2[1].x, sampledTexSizes_v2[1].y, y_uv_v2, matrix);
          float texColZ = HRTextureUtils::sampleHeightMapLDR(texZ_v2_ldr, sampledTexSizes_v2[2].x, sampledTexSizes_v2[2].y, z_uv_v2, matrix);

          texHeight.y = texColX * w_v2.x + texColY * w_v2.y + texColZ * w_v2.z;
        }
        if(isHDR)
        {
          float texColX = HRTextureUtils::sampleHeightMapHDR(texX_v2_hdr, sampledTexSizes_v2[0].x, sampledTexSizes_v2[0].y, x_uv_v2, matrix);
          float texColY = HRTextureUtils::sampleHeightMapHDR(texY_v2_hdr, sampledTexSizes_v2[1].x, sampledTexSizes_v2[1].y, y_uv_v2, matrix);
          float texColZ = HRTextureUtils::sampleHeightMapHDR(texZ_v2_hdr, sampledTexSizes_v2[2].x, sampledTexSizes_v2[2].y, z_uv_v2, matrix);

          texHeight.y = texColX * w_v2.x + texColY * w_v2.y + texColZ * w_v2.z;
        }
      }

      ins = displaced_indices.insert(tri.z);
      if(ins.second)
      {
        if(isLDR)
        {
          float texColX = HRTextureUtils::sampleHeightMapLDR(texX_v3_ldr, sampledTexSizes_v3[0].x, sampledTexSizes_v3[0].y, x_uv_v3, matrix);
          float texColY = HRTextureUtils::sampleHeightMapLDR(texY_v3_ldr, sampledTexSizes_v3[1].x, sampledTexSizes_v3[1].y, y_uv_v3, matrix);
          float texColZ = HRTextureUtils::sampleHeightMapLDR(texZ_v3_ldr, sampledTexSizes_v3[2].x, sampledTexSizes_v3[2].y, z_uv_v3, matrix);

          texHeight.z = texColX * w_v3.x + texColY * w_v3.y + texColZ * w_v3.z;
        }
        if(isHDR)
        {
          float texColX = HRTextureUtils::sampleHeightMapHDR(texX_v3_hdr, sampledTexSizes_v3[0].x, sampledTexSizes_v3[0].y, x_uv_v3, matrix);
          float texColY = HRTextureUtils::sampleHeightMapHDR(texY_v3_hdr, sampledTexSizes_v3[1].x, sampledTexSizes_v3[1].y, y_uv_v3, matrix);
          float texColZ = HRTextureUtils::sampleHeightMapHDR(texZ_v3_hdr, sampledTexSizes_v3[2].x, sampledTexSizes_v3[2].y, z_uv_v3, matrix);

          texHeight.z = texColX * w_v3.x + texColY * w_v3.y + texColZ * w_v3.z;
        }
      }
    }

    mesh.vPos4f.at(tri.x * 4 + 0) += mesh.vNorm4f.at(tri.x * 4 + 0) * mult * texHeight.x;
    mesh.vPos4f.at(tri.x * 4 + 1) += mesh.vNorm4f.at(tri.x * 4 + 1) * mult * texHeight.x;
    mesh.vPos4f.at(tri.x * 4 + 2) += mesh.vNorm4f.at(tri.x * 4 + 2) * mult * texHeight.x;

    mesh.vPos4f.at(tri.y * 4 + 0) += mesh.vNorm4f.at(tri.y * 4 + 0) * mult * texHeight.y;
    mesh.vPos4f.at(tri.y * 4 + 1) += mesh.vNorm4f.at(tri.y * 4 + 1) * mult * texHeight.y;
    mesh.vPos4f.at(tri.y * 4 + 2) += mesh.vNorm4f.at(tri.y * 4 + 2) * mult * texHeight.y;

    mesh.vPos4f.at(tri.z * 4 + 0) += mesh.vNorm4f.at(tri.z * 4 + 0) * mult * texHeight.z;
    mesh.vPos4f.at(tri.z * 4 + 1) += mesh.vNorm4f.at(tri.z * 4 + 1) * mult * texHeight.z;
    mesh.vPos4f.at(tri.z * 4 + 2) += mesh.vNorm4f.at(tri.z * 4 + 2) * mult * texHeight.z;

    texHeight = float3(0.0f, 0.0f, 0.0f);
  }
}

void doDisplacement(HRMesh *pMesh, const pugi::xml_node &displaceXMLNode, std::vector<uint3> &triangleList,
                    const HRUtils::BBox &bbox)
{
  auto& mesh = pMesh->m_input;

  auto heightNode = displaceXMLNode.child(L"height_map");
  auto noiseNode  = displaceXMLNode.child(L"noise");
  auto customNode = displaceXMLNode.child(L"custom");

  auto heightNodeTri = displaceXMLNode.child(L"height_map_triplanar");

  if(heightNode != nullptr)
  {
    displaceByHeightMap(pMesh, heightNode, triangleList);
  }
  else if(heightNodeTri != nullptr)
  {
    displaceByHeightMapTriPlanar(pMesh, heightNodeTri, triangleList);
  }
  else if(noiseNode != nullptr)
  {
    displaceByNoise(pMesh, noiseNode, triangleList);
  }
  else if(customNode != nullptr)
  {
    displaceCustom(pMesh, customNode, triangleList, bbox);
  }
}



// void InsertFixedMeshesInfoIntoXML(pugi::xml_document &stateToProcess, std::unordered_map<uint32_t, uint32_t> &meshTofixedMesh)
// {
//   auto texNode = stateToProcess.child(L"geometry_lib");
//
//   if (texNode != nullptr)
//   {
//     for (auto& meshMap : meshTofixedMesh)
//     {
//       auto geoLib = g_objManager.scnData.m_geometryLibChanges;
//
//       auto mesh_node = geoLib.find_child_by_attribute(L"id", std::to_wstring(meshMap.second).c_str());
//       texNode.append_copy(mesh_node);
//
//       auto sceneNode = stateToProcess.child(L"scenes").child(L"scene");
//       if (sceneNode != nullptr)
//       {
//         for (auto node = sceneNode.child(L"instance"); node != nullptr; node = node.next_sibling())
//         {
//           if(node.attribute(L"mesh_id") != nullptr && node.attribute(L"mesh_id").as_int() == meshMap.first)
//             node.attribute(L"mesh_id").set_value(meshMap.second);
//         }
//       }
//     }
//   }
// }
//

void InsertFixedMeshesAndInstancesXML(pugi::xml_document &stateToProcess,
                                      std::unordered_map<uint32_t, int32_t> &instToFixedMesh, int32_t sceneId)
{
  auto geolib = stateToProcess.child(L"geometry_lib");
  std::vector <std::pair<int, pugi::xml_node> > tmp_nodes;
  if (geolib != nullptr)
  {
    for (auto& meshMap : instToFixedMesh)
    {
      //auto geoLib_ch = g_objManager.scnData.m_geometryLibChanges;
      //auto mesh_node = geoLib_ch.find_child_by_attribute(L"id", std::to_wstring(meshMap.second).c_str());
      if(meshMap.second < 0 || meshMap.second >= g_objManager.scnData.meshes.size())
        continue;
      
      auto mesh_node = g_objManager.scnData.meshes[meshMap.second].xml_node();
      
      std::pair<int, pugi::xml_node> p(meshMap.second, mesh_node);
      tmp_nodes.push_back(p);
      auto sceneNode = stateToProcess.child(L"scenes").find_child_by_attribute(L"scene", L"id", std::to_wstring(sceneId).c_str());
      if (sceneNode != nullptr)
      {
        auto node = sceneNode.find_child_by_attribute(L"instance", L"id", std::to_wstring(meshMap.first).c_str());
        if(node != nullptr)
        {
          node.attribute(L"mesh_id").set_value(meshMap.second);
        }
      }
    }
  }
  std::sort(tmp_nodes.begin(), tmp_nodes.end(),
            [&](auto a, auto b) { return a.first < b.first; });
  
  for (auto& node : tmp_nodes)
    geolib.append_copy(node.second);
}


/*
std::wstring HR_PreprocessMeshes_old(const wchar_t *state_path)
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

      //instanceHasDisplacementMat(mesh_ref, remapList, displaceXMLNode);

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
        hrMeshAppendTriangles3(mesh_ref_new, int(triIndices.size()), (int *) (&triIndices[0]), false);

        int subdivs = displaceXMLNode.attribute(L"subdivs").as_int();

        //std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        hrMeshSubdivideSqrt3(mesh_ref_new, max(subdivs, 1));
        //std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        //std::cout << "Subdivision time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" <<std::endl;

        //begin = std::chrono::steady_clock::now();
        hrMeshDisplace(mesh_ref_new, stateToProcess);
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

*/

void removeDisplacementFromMaterials(const pugi::xml_document &stateToProcess, std::set<int32_t > &materialIDs)
{
  auto matlib = stateToProcess.child(L"materials_lib");
  for (auto &mat_id :materialIDs)
  {
    auto mat_node = matlib.find_child_by_attribute(L"material", L"id", (std::to_wstring(mat_id)).c_str());
    if (mat_node != nullptr)
    {
      mat_node.remove_child(L"displacement");
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

    std::vector<std::unordered_map<uint32_t, uint32_t> > remapLists;
    std::unordered_map<uint32_t, int32_t> instToFixedMesh;
    auto sceneNode = stateToProcess.child(L"scenes").find_child_by_attribute(L"scene", L"id", std::to_wstring(g_objManager.m_currSceneId).c_str());
    if (sceneNode != nullptr && sceneNode.child(L"remap_lists") != nullptr)
    {
      for(auto listNode = sceneNode.child(L"remap_lists").first_child(); listNode != nullptr; listNode = listNode.next_sibling())
      {
        int listSize = listNode.attribute(L"size").as_int();

        std::unordered_map<uint32_t, uint32_t> remapList;
        const wchar_t* listStr = listNode.attribute(L"val").as_string();
        if(listStr != nullptr)
        {
          std::wstringstream inputStream(listStr);
          for(int i = 0; i < listSize; i += 2)
          {
            uint32_t a = 0;
            uint32_t b = 0;

            inputStream >> a;
            inputStream >> b;

            remapList[a] = b;
          }
        }
        remapLists.emplace_back(remapList);
      }
    }

    std::set<int32_t > displacementMatIDs;
    for(int i = 0; i < scn.drawList.size(); ++i)
    {
      auto inst = scn.drawList.at(i);
      HRMeshRef mesh_ref;
      mesh_ref.id = inst.meshId;

      auto inst_id = i;
      auto inst_id_str = std::to_wstring(inst_id);

      std::unordered_map<uint32_t, uint32_t> remap_list;
      if(inst.remapListId >= 0)
        remap_list = remapLists[inst.remapListId];

      pugi::xml_node displaceXMLNode;
      HRMaterialRef matRef;

      hrMeshOpen(mesh_ref, HR_TRIANGLE_IND3, HR_OPEN_READ_ONLY);
      if(instanceHasDisplacementMat(mesh_ref, remap_list, displaceXMLNode, matRef))
      {
        //displacementMats.
        displacementMatIDs.insert(matRef.id);
        HRMesh& mesh = g_objManager.scnData.meshes[inst.meshId];

        auto bbox_old         = mesh.pImpl->getBBox();
        auto verticesPos      = mesh.m_input.vPos4f;      ///< float4
        auto verticesNorm     = mesh.m_input.vNorm4f;     ///< float4
        auto verticesTexCoord = mesh.m_input.vTexCoord2f; ///< float2
        auto verticesTangent  = mesh.m_input.vTang4f;     ///< float4
        auto triIndices       = mesh.m_input.indices;     ///< size of 3*triNum
        auto matIndices       = mesh.m_input.matIndices;  ///< size of 1*triNum
        auto mesh_name        = mesh.name;
        hrMeshClose(mesh_ref);

        std::wstring new_mesh_name = mesh_name.append(L"_fixed_").append(inst_id_str);
        HRMeshRef mesh_ref_new = hrMeshCreate(new_mesh_name.c_str());
        hrMeshOpen(mesh_ref_new, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
        hrMeshVertexAttribPointer4f(mesh_ref_new, L"pos", &verticesPos[0]);
        hrMeshVertexAttribPointer4f(mesh_ref_new, L"norm", &verticesNorm[0]);
        hrMeshVertexAttribPointer2f(mesh_ref_new, L"texcoord", &verticesTexCoord[0]);
        hrMeshPrimitiveAttribPointer1i(mesh_ref_new, L"mind", (int *) (&matIndices[0]));
        hrMeshAppendTriangles3(mesh_ref_new, int(triIndices.size()), (int *) (&triIndices[0]), false);

        int subdivs = displaceXMLNode.attribute(L"subdivs").as_int();

        //std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        hrMeshSubdivideSqrt3(mesh_ref_new, max(subdivs, 0));
        //std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        //std::cout << "Subdivision time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" <<std::endl;

        //begin = std::chrono::steady_clock::now();
        hrMeshDisplace(mesh_ref_new, remap_list, stateToProcess, bbox_old);
        //end = std::chrono::steady_clock::now();
        //std::cout << "Displacement time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" <<std::endl;

        hrMeshClose(mesh_ref_new);

        instToFixedMesh[inst_id] = mesh_ref_new.id;
        anyChanges = true;
      }
      else
      {
        hrMeshClose(mesh_ref);
      }
    }

    if(anyChanges)
    {
      InsertFixedMeshesAndInstancesXML(stateToProcess, instToFixedMesh, g_objManager.m_currSceneId);
      removeDisplacementFromMaterials(stateToProcess, displacementMatIDs);
    }
  }

  if(anyChanges)
    return SaveFixedStateXML(stateToProcess, state_path, L"_meshes");
  else
    return state_path;
}


void hrMeshWeldVertices(HRMeshRef a_mesh, int &indexNum)
{
  HRMesh *pMesh = g_objManager.PtrById(a_mesh);

  if (pMesh == nullptr)
  {
    HrError(L"hrMeshWeldVertices: nullptr input");
    return;
  }

  if (!pMesh->opened)
  {
    HrError(L"hrMeshWeldVertices: mesh is not opened, id = ", a_mesh.id);
    return;
  }

  WeldVertices(pMesh->m_input, uint32_t(pMesh->m_input.IndicesNum()));
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// Mesh sampling

void HRUtils::getRandomPointsOnMesh(HRMeshRef mesh_ref, float *points, uint32_t n_points, bool tri_area_weighted, uint32_t seed)
{
  HRMesh *pMesh = g_objManager.PtrById(mesh_ref);

  if (pMesh == nullptr)
  {
    HrError(L"HRUtils::getRandomPointsOnMesh: nullptr input");
    return;
  }

  if(points == nullptr)
  {
    HrError(L"HRUtils::getRandomPointsOnMesh: points is nullptr");
    return;
  }

  if (!pMesh->opened)
  {
    hrMeshOpen(mesh_ref, HR_TRIANGLE_IND3, HR_OPEN_EXISTING);
  }

  auto& mesh = pMesh->m_input;

  uint32_t vert_num = uint32_t(mesh.VerticesNum());
  uint32_t tri_num  = uint32_t(mesh.TrianglesNum());

  //std::mt19937 rng(seed);
  hr_prng::RandomGen rgen = hr_prng::RandomGenInit(777777);
  std::uniform_real_distribution<float> select(0.0f, 1.0f);

  std::vector<uint32_t> triangle_indices;
  if(tri_area_weighted)
  {
    float min_area = std::numeric_limits<float>::max();
    std::vector<float> triangle_areas(tri_num, 0.0f);
    for(uint32_t i = 0; i < tri_num; ++i )
    {
      const uint32_t idx_A = mesh.indices[i * 3 + 0];
      const uint32_t idx_B = mesh.indices[i * 3 + 1];
      const uint32_t idx_C = mesh.indices[i * 3 + 2];

      const float3 A       = float3(mesh.vPos4f[idx_A * 4 + 0], mesh.vPos4f[idx_A * 4 + 1], mesh.vPos4f[idx_A * 4 + 2]);
      const float3 B       = float3(mesh.vPos4f[idx_B * 4 + 0], mesh.vPos4f[idx_B * 4 + 1], mesh.vPos4f[idx_B * 4 + 2]);
      const float3 C       = float3(mesh.vPos4f[idx_C * 4 + 0], mesh.vPos4f[idx_C * 4 + 1], mesh.vPos4f[idx_C * 4 + 2]);
 
      const float3 edge1A = normalize(B - A);
      const float3 edge2A = normalize(C - A);

      float face_area = 0.5f * sqrtf(powf(edge1A.y * edge2A.z - edge1A.z * edge2A.y, 2) +
                                     powf(edge1A.z * edge2A.x - edge1A.x * edge2A.z, 2) +
                                     powf(edge1A.x * edge2A.y - edge1A.y * edge2A.x, 2));

//      float cos_theta = dot(edge1A, edge2A);
//      float sin_theta = sqrtf(1.0f - cos_theta * cos_theta);
//      float face_area = 0.5f * dot(edge1A, edge1A) * dot(edge2A, edge2A) * sin_theta;

      if(face_area < min_area)
        min_area = face_area;

      triangle_areas[i] = face_area;
    }

    //std::vector<uint32_t> tmp_vec(triangle_areas.size(), 0u);
    for(uint32_t i = 0; i < triangle_areas.size(); ++i)
    {
      auto tmp = static_cast<uint32_t >(std::ceil(triangle_areas[i] / min_area));

      for(uint32_t j = 0; j <= tmp; ++j)
        triangle_indices.push_back(i);
    }
  }


  for(uint32_t i = 0; i < n_points; ++i)
  {
    uint32_t triangle = 0u;
    if(tri_area_weighted)
      triangle = triangle_indices[uint32_t(triangle_indices.size() * hr_prng::rndFloatUniform(rgen, 0.0f, 1.0f))];
    else
      triangle = uint32_t(tri_num * hr_prng::rndFloatUniform(rgen, 0.0f, 1.0f));

    uint32_t idx_A = mesh.indices.at(triangle * 3 + 0);
    uint32_t idx_B = mesh.indices.at(triangle * 3 + 1);
    uint32_t idx_C = mesh.indices.at(triangle * 3 + 2);

    float3 A = make_float3(mesh.vPos4f.at(idx_A * 4 + 0), mesh.vPos4f.at(idx_A * 4 + 1), mesh.vPos4f.at(idx_A * 4 + 2));
    float3 B = make_float3(mesh.vPos4f.at(idx_B * 4 + 0), mesh.vPos4f.at(idx_B * 4 + 1), mesh.vPos4f.at(idx_B * 4 + 2));
    float3 C = make_float3(mesh.vPos4f.at(idx_C * 4 + 0), mesh.vPos4f.at(idx_C * 4 + 1), mesh.vPos4f.at(idx_C * 4 + 2));

    float u = hr_prng::rndFloatUniform(rgen, 0.0f, 1.0f);
    float v = hr_prng::rndFloatUniform(rgen, 0.0f, 1.0f);
    if( u + v > 1.0f)
    {
      u = 1.0f - u;
      v = 1.0f - v;
    }

    float3 pt = u * A + v * B + (1 - (u + v)) * C;

    points[i * 3 + 0] = pt.x;
    points[i * 3 + 1] = pt.y;
    points[i * 3 + 2] = pt.z;
  }
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



