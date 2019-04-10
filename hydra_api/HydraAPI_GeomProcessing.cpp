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
#include <random>

using std::isnan;
using std::isinf;

#include "LiteMath.h"
using namespace HydraLiteMath;

#include "HydraObjectManager.h"
#include "HydraVSGFExport.h"
#include "HydraXMLHelpers.h"
#include "HydraTextureUtils.h"

#include "../mikktspace/mikktspace.h"

extern std::wstring      g_lastError;
extern std::wstring      g_lastErrorCallerPlace;
extern HR_ERROR_CALLBACK g_pErrorCallback;
extern HRObjectManager   g_objManager;


void HR_ComputeTangentSpaceSimple(const int     vertexCount, const int     triangleCount, const uint32_t* triIndices,
                                  const float4* verticesPos, const float4* verticesNorm, const float2* vertTexCoord,
                                  float4* verticesTang)
{
  
  float4 *tan1 = new float4[vertexCount * 2];
  float4 *tan2 = tan1 + vertexCount;
  memset(tan1, 0, vertexCount * sizeof(float4) * 2);
  
  const float epsDiv = 1.0e25f;
  
  for (auto a = 0; a < triangleCount; a++)
  {
    auto i1 = triIndices[3 * a + 0];
    auto i2 = triIndices[3 * a + 1];
    auto i3 = triIndices[3 * a + 2];
    
    const float4& v1 = verticesPos[i1];
    const float4& v2 = verticesPos[i2];
    const float4& v3 = verticesPos[i3];
    
    const float2& w1 = vertTexCoord[i1];
    const float2& w2 = vertTexCoord[i2];
    const float2& w3 = vertTexCoord[i3];
    
    const float x1 = v2.x - v1.x;
    const float x2 = v3.x - v1.x;
    const float y1 = v2.y - v1.y;
    const float y2 = v3.y - v1.y;
    const float z1 = v2.z - v1.z;
    const float z2 = v3.z - v1.z;
    
    const float s1 = w2.x - w1.x;
    const float s2 = w3.x - w1.x;
    const float t1 = w2.y - w1.y;
    const float t2 = w3.y - w1.y;
    
    const float denom = (s1 * t2 - s2 * t1);
    const float r     = (fabs(denom) > 1e-35f) ? (1.0f / denom) : 0.0f;
    
    const float4 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r, 1);
    const float4 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r, 1);
    
    tan1[i1] += sdir;
    tan1[i2] += sdir;
    tan1[i3] += sdir;
    
    tan2[i1] += tdir;
    tan2[i2] += tdir;
    tan2[i3] += tdir;
  }
  
  for (long a = 0; a < vertexCount; a++)
  {
    const float4& n = verticesNorm[a];
    const float4& t = tan1[a];
    
    const float3 n1 = to_float3(n);
    const float3 t1 = to_float3(t);
    
    // Gram-Schmidt orthogonalization
    verticesTang[a] = to_float4(normalize(t1 - n1 * dot(n1, t1)), 0.0f); // #NOTE: overlow here is ok!
    
    verticesTang[a].x = isnan(verticesTang[a].x) || isinf(verticesTang[a].x) ? 0 : verticesTang[a].x;
    verticesTang[a].y = isnan(verticesTang[a].y) || isinf(verticesTang[a].y) ? 0 : verticesTang[a].y;
    verticesTang[a].z = isnan(verticesTang[a].z) || isinf(verticesTang[a].z) ? 0 : verticesTang[a].z;
    
    // Calculate handedness
    verticesTang[a].w = (dot(cross(n1, t1), to_float3(tan2[a])) < 0.0f) ? -1.0f : 1.0f;
  }
  
  delete [] tan1;
}



struct vertex_cache_eq
{
    bool operator()(const vertex_cache & u, const vertex_cache & v) const
    {
      return (fabsf(u.pos.x - v.pos.x) < 1e-6) && (fabsf(u.pos.y - v.pos.y) < 1e-6) && (fabsf(u.pos.z - v.pos.z) < 1e-6) && //pos
             (fabsf(u.normal.x - v.normal.x) < 1e-3) && (fabsf(u.normal.y - v.normal.y) < 1e-3) && (fabsf(u.normal.z - v.normal.z) < 1e-3) && //norm
             (fabsf(u.uv.x - v.uv.x) < 1e-5) && (fabsf(u.uv.y - v.uv.y) < 1e-5) &&
             (fabsf(u.tangent.x - v.tangent.x) < 1e-3) && (fabsf(u.tangent.y - v.tangent.y) < 1e-3) && (fabsf(u.tangent.z - v.tangent.z) < 1e-3) &&
             (fabsf(u.tangent.w - v.tangent.w) < 1e-1); //tangents
    }
};


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
        auto d_node = mat->xml_node_next(HR_OPEN_EXISTING).child(L"displacement");

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

  HRMesh::InputTriMesh &mesh = pMesh->m_input;

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
        auto d_node = mat->xml_node_next(HR_OPEN_EXISTING).child(L"displacement");

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

  HRMesh::InputTriMesh &mesh = pMesh->m_input;

  const auto vertexCount = int(mesh.verticesPos.size() / 4);
  const auto triangleCount = int(mesh.triIndices.size() / 3);

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
        auto d_node = mat->xml_node_next(HR_OPEN_EXISTING).child(L"displacement");

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


  hrMeshComputeNormals(a_mesh, int(mesh.triIndices.size()), false);
}

float smoothing_coeff(uint32_t valence)
{
  return (4.0f - 2.0f * cosf((2.0f * 3.14159265358979323846f) / valence )) / 9.0f;
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

    const auto old_vertex_count = uint32_t(mesh.verticesPos.size() / 4);
    const auto old_tri_count = uint32_t(mesh.triIndices.size() / 3);

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

      uint32_t indP = uint32_t(mesh.verticesPos.size() / 4);
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

    for (uint32_t ii = 0; ii < pos_new.size(); ++ii)
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

//  const auto vertexCount = int(mesh.verticesPos.size() / 4);
//  const auto triangleCount = int(mesh.triIndices.size() / 3);

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

    uint32_t indP = uint32_t(mesh.verticesPos.size() / 4);
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
}

void displaceCustom(HRMesh *pMesh, const pugi::xml_node &customNode, std::vector<uint3> &triangleList, const HRUtils::BBox &bbox)
{
  HRMesh::InputTriMesh &mesh = pMesh->m_input;

  auto texNode = customNode.child(L"texture");
  pugi::xml_node texLibNode;
  HRTextureNode *texture;
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

void displaceByHeightMapTriPlanar(HRMesh *pMesh, const pugi::xml_node &heightXMLNode, std::vector<uint3> &triangleList)
{
  HRMesh::InputTriMesh &mesh = pMesh->m_input;

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
  int2 texSizes[6];
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

void doDisplacement(HRMesh *pMesh, const pugi::xml_node &displaceXMLNode, std::vector<uint3> &triangleList,
                    const HRUtils::BBox &bbox)
{
  HRMesh::InputTriMesh &mesh = pMesh->m_input;

  auto heightNode = displaceXMLNode.child(L"height_map");

  auto noiseNode = displaceXMLNode.child(L"noise");

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

void InsertFixedMeshesAndInstancesXML(pugi::xml_document &stateToProcess,
                                      std::unordered_map<uint32_t, int32_t> &instToFixedMesh, int32_t sceneId)
{
  auto geolib = stateToProcess.child(L"geometry_lib");
  std::vector <std::pair<int, pugi::xml_node> > tmp_nodes;

  if (geolib != nullptr)
  {
    for (auto& meshMap : instToFixedMesh)
    {
      auto geoLib_ch = g_objManager.scnData.m_geometryLibChanges;

      auto mesh_node = geoLib_ch.find_child_by_attribute(L"id", std::to_wstring(meshMap.second).c_str());
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
        HRUtils::BBox bbox_old = mesh.pImpl->getBBox();
        std::vector<float> verticesPos(mesh.m_input.verticesPos);       ///< float4
        std::vector<float> verticesNorm(mesh.m_input.verticesNorm);      ///< float4
        std::vector<float> verticesTexCoord(mesh.m_input.verticesTexCoord);  ///< float2
        std::vector<float> verticesTangent(mesh.m_input.verticesTangent);   ///< float4
        std::vector<uint32_t> triIndices(mesh.m_input.triIndices);        ///< size of 3*triNum
        std::vector<uint32_t> matIndices(mesh.m_input.matIndices);        ///< size of 1*triNum
        auto mesh_name = mesh.name;
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

  HRMesh::InputTriMesh &mesh = pMesh->m_input;
  //const bool hasNormals  = (pMesh->m_inputPointers.normals != nullptr);

  std::vector<uint32_t> indices_new;
  //std::unordered_map<float3, uint32_t, float3_hash, pos_eq> vertex_hash;
  std::unordered_map<vertex_cache, uint32_t, vertex_cache_hash, vertex_cache_eq> vertex_hash;

  std::vector<float> vertices_new(mesh.verticesPos.size()*2, 0.0f);
  std::vector<float> normals_new(mesh.verticesNorm.size()*2, 0.0f);
  std::vector<float> uv_new(mesh.verticesTexCoord.size()*2, 0.0f);
  std::vector<float> tangents_new(mesh.verticesTangent.size()*2, 0.0f);
  std::vector<int32_t> mid_new;
  mid_new.reserve(mesh.matIndices.size());

  uint32_t index = 0;
  for (auto i = 0u; i < mesh.triIndices.size(); i += 3)
  {
    uint32_t indA = mesh.triIndices[i + 0];
    uint32_t indB = mesh.triIndices[i + 1];
    uint32_t indC = mesh.triIndices[i + 2];

    if (indA == indB || indA == indC || indB == indC)
      continue;

    auto old_mid = mesh.matIndices.at(i / 3);
    mid_new.push_back(old_mid);

    float4 tmp = vertex_attrib_by_index_f4("pos", indA, mesh);
    float3 A(tmp.x, tmp.y, tmp.z);
    tmp = vertex_attrib_by_index_f4("normal", indA, mesh);
    float3 A_normal(tmp.x, tmp.y, tmp.z);
    float2 tmp2 = vertex_attrib_by_index_f2("uv", indA, mesh);
    float2 A_uv(tmp2.x, tmp2.y);
    float4 A_tan = vertex_attrib_by_index_f4("tangent", indA, mesh);

    vertex_cache A_cache;
    A_cache.pos = A;
    A_cache.normal = A_normal;
    A_cache.uv = A_uv;
    A_cache.tangent = A_tan;

    auto it = vertex_hash.find(A_cache);

    if(it != vertex_hash.end())
    {
      indices_new.push_back(it->second);
    }
    else
    {
      vertex_hash[A_cache] = index;
      indices_new.push_back(index);

      vertices_new.at(index * 4 + 0) = A.x;
      vertices_new.at(index * 4 + 1) = A.y;
      vertices_new.at(index * 4 + 2) = A.z;
      vertices_new.at(index * 4 + 3) = 1.0f;

      normals_new.at(index * 4 + 0) = A_normal.x;
      normals_new.at(index * 4 + 1) = A_normal.y;
      normals_new.at(index * 4 + 2) = A_normal.z;
      normals_new.at(index * 4 + 3) = 0.0f;

      uv_new.at(index * 2 + 0) = A_uv.x;
      uv_new.at(index * 2 + 1) = A_uv.y;

      tangents_new.at(index * 4 + 0) = A_tan.x;
      tangents_new.at(index * 4 + 1) = A_tan.y;
      tangents_new.at(index * 4 + 2) = A_tan.z;
      tangents_new.at(index * 4 + 3) = A_tan.w;

      index++;
    }

    tmp = vertex_attrib_by_index_f4("pos", indB, mesh);
    float3 B(tmp.x, tmp.y, tmp.z);
    tmp = vertex_attrib_by_index_f4("normal", indB, mesh);
    float3 B_normal(tmp.x, tmp.y, tmp.z);
    tmp2 = vertex_attrib_by_index_f2("uv", indB, mesh);
    float2 B_uv(tmp2.x, tmp2.y);
    float4 B_tan = vertex_attrib_by_index_f4("tangent", indB, mesh);

    vertex_cache B_cache;
    B_cache.pos = B;
    B_cache.normal = B_normal;
    B_cache.uv = B_uv;
    B_cache.tangent = B_tan;

    it = vertex_hash.find(B_cache);
    if(it != vertex_hash.end())
    {
      indices_new.push_back(it->second);
    }
    else
    {
      vertex_hash[B_cache] = index;
      indices_new.push_back(index);

      vertices_new.at(index * 4 + 0) = B.x;
      vertices_new.at(index * 4 + 1) = B.y;
      vertices_new.at(index * 4 + 2) = B.z;
      vertices_new.at(index * 4 + 3) = 1.0f;

      normals_new.at(index * 4 + 0) = B_normal.x;
      normals_new.at(index * 4 + 1) = B_normal.y;
      normals_new.at(index * 4 + 2) = B_normal.z;
      normals_new.at(index * 4 + 3) = 0.0f;

      uv_new.at(index * 2 + 0) = B_uv.x;
      uv_new.at(index * 2 + 1) = B_uv.y;

      tangents_new.at(index * 4 + 0) = B_tan.x;
      tangents_new.at(index * 4 + 1) = B_tan.y;
      tangents_new.at(index * 4 + 2) = B_tan.z;
      tangents_new.at(index * 4 + 3) = B_tan.w;

      index++;
    }

    tmp = vertex_attrib_by_index_f4("pos", indC, mesh);
    float3 C(tmp.x, tmp.y, tmp.z);
    tmp = vertex_attrib_by_index_f4("normal", indC, mesh);
    float3 C_normal(tmp.x, tmp.y, tmp.z);
    tmp2 = vertex_attrib_by_index_f2("uv", indC, mesh);
    float2 C_uv(tmp2.x, tmp2.y);
    float4 C_tan = vertex_attrib_by_index_f4("tangent", indC, mesh);

    vertex_cache C_cache;
    C_cache.pos = C;
    C_cache.normal = C_normal;
    C_cache.uv = C_uv;
    C_cache.tangent = C_tan;


    it = vertex_hash.find(C_cache);
    if(it != vertex_hash.end())
    {
      indices_new.push_back(it->second);
    }
    else
    {
      vertex_hash[C_cache] = index;
      indices_new.push_back(index);

      vertices_new.at(index * 4 + 0) = C.x;
      vertices_new.at(index * 4 + 1) = C.y;
      vertices_new.at(index * 4 + 2) = C.z;
      vertices_new.at(index * 4 + 3) = 1.0f;

      normals_new.at(index * 4 + 0) = C_normal.x;
      normals_new.at(index * 4 + 1) = C_normal.y;
      normals_new.at(index * 4 + 2) = C_normal.z;
      normals_new.at(index * 4 + 3) = 0.0f;

      uv_new.at(index * 2 + 0) = C_uv.x;
      uv_new.at(index * 2 + 1) = C_uv.y;

      tangents_new.at(index * 4 + 0) = C_tan.x;
      tangents_new.at(index * 4 + 1) = C_tan.y;
      tangents_new.at(index * 4 + 2) = C_tan.z;
      tangents_new.at(index * 4 + 3) = C_tan.w;

      index++;
    }

  }

  vertices_new.resize(index * 4);
  normals_new.resize(index * 4);
  uv_new.resize(index * 2);
  tangents_new.resize(index * 4);

  //std::cout << "vert old : " << mesh.verticesPos.size() << "  vert new :" << vertices_new.size() << std::endl;

  pMesh->m_inputPointers.normals = nullptr;
  pMesh->m_inputPointers.tangents = nullptr;

  mesh.verticesPos.clear();
  mesh.verticesPos.resize(vertices_new.size());
  std::copy(vertices_new.begin(), vertices_new.end(), mesh.verticesPos.begin());

  mesh.verticesNorm.clear();
  mesh.verticesNorm.resize(normals_new.size());
  std::copy(normals_new.begin(), normals_new.end(), mesh.verticesNorm.begin());

  mesh.verticesTexCoord.clear();
  mesh.verticesTexCoord.resize(uv_new.size());
  std::copy(uv_new.begin(), uv_new.end(), mesh.verticesTexCoord.begin());

  mesh.verticesTangent.clear();
  mesh.verticesTangent.resize(tangents_new.size());
  std::copy(tangents_new.begin(), tangents_new.end(), mesh.verticesTangent.begin());

  mesh.triIndices.clear();
  mesh.triIndices.resize(indices_new.size());
  std::copy(indices_new.begin(), indices_new.end(), mesh.triIndices.begin());

  mesh.matIndices.clear();
  mesh.matIndices.resize(mid_new.size());
  std::copy(mid_new.begin(), mid_new.end(), mesh.matIndices.begin());

  indexNum = int(indices_new.size());
}


//***Tangent Space calc******************************************************************

// Return number of primitives in the geometry.
int getNumFaces(const SMikkTSpaceContext *context)
{
  HRMesh *pMesh = static_cast <HRMesh *> (context->m_pUserData);

  return int(pMesh->m_input.triIndices.size()/3);
}

// Return number of vertices in the primitive given by index.
int getNumVerticesOfFace(const SMikkTSpaceContext *context, const int primnum)
{
  return 3;
}

// Write 3-float position of the vertex's point.
void getPosition(const SMikkTSpaceContext *context, float outpos[], const int primnum, const int vtxnum)
{
  HRMesh *pMesh = static_cast <HRMesh *> (context->m_pUserData);
  auto index = pMesh->m_input.triIndices.at(primnum * 3 + vtxnum);

  outpos[0] = pMesh->m_input.verticesPos.at(index * 4 + 0);
  outpos[1] = pMesh->m_input.verticesPos.at(index * 4 + 1);
  outpos[2] = pMesh->m_input.verticesPos.at(index * 4 + 2);
}

// Write 3-float vertex normal.
void getNormal(const SMikkTSpaceContext *context, float outnormal[], const int primnum, const int vtxnum)
{
  HRMesh *pMesh = static_cast <HRMesh *> (context->m_pUserData);
  auto index = pMesh->m_input.triIndices.at(primnum * 3 + vtxnum);

  outnormal[0] = pMesh->m_input.verticesNorm.at(index * 4 + 0);
  outnormal[1] = pMesh->m_input.verticesNorm.at(index * 4 + 1);
  outnormal[2] = pMesh->m_input.verticesNorm.at(index * 4 + 2);
}

// Write 2-float vertex uv.
void getTexCoord(const SMikkTSpaceContext *context, float outuv[], const int primnum, const int vtxnum)
{
  HRMesh *pMesh = static_cast <HRMesh *> (context->m_pUserData);
  auto index = pMesh->m_input.triIndices.at(primnum * 3 + vtxnum);

  outuv[0] = pMesh->m_input.verticesNorm.at(index * 2 + 0);
  outuv[1] = pMesh->m_input.verticesNorm.at(index * 2 + 1);
}

// Compute and set attributes on the geometry vertex. Basic version.
void setTSpaceBasic(const SMikkTSpaceContext *context,
                    const float tangentu[],
                    const float sign,
                    const int primnum,
                    const int vtxnum)
{
  HRMesh *pMesh = static_cast <HRMesh *> (context->m_pUserData);
  auto index = pMesh->m_input.triIndices.at(primnum * 3 + vtxnum);

//  if(sign < 0.0f)
//  {
//    pMesh->m_input.verticesTangent[index * 4 + 0] = -1.0f * tangentu[0];
//    pMesh->m_input.verticesTangent[index * 4 + 1] = -1.0f * tangentu[1];
//    pMesh->m_input.verticesTangent[index * 4 + 2] = -1.0f * tangentu[2];
//  }
//  else
  {
    pMesh->m_input.verticesTangent.at(index * 4 + 0) = tangentu[0];
    pMesh->m_input.verticesTangent.at(index * 4 + 1) = tangentu[1];
    pMesh->m_input.verticesTangent.at(index * 4 + 2) = tangentu[2];
  }
  pMesh->m_input.verticesTangent.at(index * 4 + 3) = sign;
}

void setTSpace(const SMikkTSpaceContext *context,
               const float tangentu[],
               const float tangentv[],
               const float magu,
               const float magv,
               const tbool keep,
               const int primnum,
               const int vtxnum)
{
  HRMesh *pMesh = static_cast <HRMesh *> (context->m_pUserData);
  auto index = pMesh->m_input.triIndices[primnum * 3 + vtxnum];

  pMesh->m_input.verticesTangent[index * 4 + 0] = tangentv[0];
  pMesh->m_input.verticesTangent[index * 4 + 1] = tangentv[1];
  pMesh->m_input.verticesTangent[index * 4 + 2] = tangentv[2];
  pMesh->m_input.verticesTangent[index * 4 + 3] = 1.0f;
}

void runTSpaceCalc(HRMeshRef mesh_ref, bool basic)
{
  HRMesh* pMesh = g_objManager.PtrById(mesh_ref);
  if (pMesh == nullptr)
  {
    HrError(L"runTSpaceCalc: nullptr input");
    return;
  }

  SMikkTSpaceInterface iface;
  iface.m_getNumFaces = getNumFaces;
  iface.m_getNumVerticesOfFace = getNumVerticesOfFace;
  iface.m_getPosition = getPosition;
  iface.m_getNormal = getNormal;
  iface.m_getTexCoord = getTexCoord;
  iface.m_setTSpaceBasic = basic ? setTSpaceBasic : nullptr;
  iface.m_setTSpace = basic ? nullptr : setTSpace;

  SMikkTSpaceContext context;
  context.m_pInterface = &iface;
  context.m_pUserData = pMesh;
  pMesh->m_input.verticesTangent.resize(pMesh->m_input.verticesPos.size());

  genTangSpaceDefault(&context);
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

  HRMesh::InputTriMesh &mesh = pMesh->m_input;

  uint32_t vert_num = mesh.verticesPos.size();
  uint32_t tri_num = mesh.triIndices.size() / 3;

//  std::random_device rand;
  std::mt19937 rng(seed);
  std::uniform_real_distribution<float> select(0.0f, 1.0f);

  std::vector<uint32_t> triangle_indices;
  if(tri_area_weighted)
  {
    float min_area = std::numeric_limits<float>::max();
    std::vector<float> triangle_areas(tri_num, 0.0f);
    for(uint32_t i = 0; i < tri_num; ++i )
    {
      uint32_t idx_A = mesh.triIndices.at(i * 3 + 0);
      uint32_t idx_B = mesh.triIndices.at(i * 3 + 1);
      uint32_t idx_C = mesh.triIndices.at(i * 3 + 2);
      float3 A = make_float3(mesh.verticesPos.at(idx_A * 4 + 0), mesh.verticesPos.at(idx_A * 4 + 1), mesh.verticesPos.at(idx_A * 4 + 2));
      float3 B = make_float3(mesh.verticesPos.at(idx_B * 4 + 0), mesh.verticesPos.at(idx_B * 4 + 1), mesh.verticesPos.at(idx_B * 4 + 2));
      float3 C = make_float3(mesh.verticesPos.at(idx_C * 4 + 0), mesh.verticesPos.at(idx_C * 4 + 1), mesh.verticesPos.at(idx_C * 4 + 2));

      float3 edge1A = normalize(B - A);
      float3 edge2A = normalize(C - A);

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

      for(int j = 0; j <= tmp; ++j)
        triangle_indices.push_back(i);
    }
  }


  for(uint32_t i = 0; i < n_points; ++i)
  {
    uint32_t triangle = 0u;
    if(tri_area_weighted)
      triangle = triangle_indices[uint32_t(triangle_indices.size() * select(rng))];
    else
      triangle = uint32_t(tri_num * select(rng));

    uint32_t idx_A = mesh.triIndices.at(triangle * 3 + 0);
    uint32_t idx_B = mesh.triIndices.at(triangle * 3 + 1);
    uint32_t idx_C = mesh.triIndices.at(triangle * 3 + 2);

    float3 A = make_float3(mesh.verticesPos.at(idx_A * 4 + 0), mesh.verticesPos.at(idx_A * 4 + 1), mesh.verticesPos.at(idx_A * 4 + 2));
    float3 B = make_float3(mesh.verticesPos.at(idx_B * 4 + 0), mesh.verticesPos.at(idx_B * 4 + 1), mesh.verticesPos.at(idx_B * 4 + 2));
    float3 C = make_float3(mesh.verticesPos.at(idx_C * 4 + 0), mesh.verticesPos.at(idx_C * 4 + 1), mesh.verticesPos.at(idx_C * 4 + 2));

    float u = select(rng);
    float v = select(rng);
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



