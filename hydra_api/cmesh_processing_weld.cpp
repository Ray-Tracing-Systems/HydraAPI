#include "cmesh.h"
#include "cmesh_vsgf.h"

#include "LiteMath.h"
using LiteMath::float4;
using LiteMath::float3;
using LiteMath::float2;

#include <cmath>
#include <fstream>
#include <unordered_map>

namespace cmesh
{
  struct vertex_cache
  {
      float3 pos;
      float3 normal;
      float4 tangent;
      float2 uv;

      vertex_cache() = default;
  };

  struct vertex_cache_hash
  {
      std::size_t operator()(const vertex_cache &v) const
      {
        using std::size_t;
        using std::hash;
        return ((hash<int>()(int(v.pos.x * 73856093))) ^
                (hash<int>()(int(v.pos.y * 19349663))) ^
                (hash<int>()(int(v.pos.z * 83492791))) ^
                (hash<int>()(int(v.normal.x * 12929173))) ^
                (hash<int>()(int(v.normal.y * 15484457))) ^
                (hash<int>()(int(v.normal.z * 26430499))) ^
                (hash<int>()(int(v.uv.x * 30025883))) ^
                (hash<int>()(int(v.uv.y * 41855327))) ^
                (hash<int>()(int(v.tangent.x * 50040937))) ^
                (hash<int>()(int(v.tangent.y * 57208453))) ^
                (hash<int>()(int(v.tangent.z * 60352007))) ^
                (hash<int>()(int(v.tangent.w * 67432663))) );
      }
  };

  struct float3_hash
  {
      std::size_t operator()(const float3& k) const
      {
        using std::size_t;
        using std::hash;
        return ((hash<int>()(int(k.x * 73856093))) ^
                (hash<int>()(int(k.y * 19349663))) ^
                (hash<int>()(int(k.z * 83492791))));
      }
  };

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

  float4 vertex_attrib_by_index_f4(const std::string &attrib_name, uint32_t vertex_index, const cmesh::SimpleMesh& mesh)
  {
    float4 res;
    if(attrib_name == "pos")
    {
      res = float4(mesh.vPos4f.at(vertex_index * 4 + 0), mesh.vPos4f.at(vertex_index * 4 + 1),
                   mesh.vPos4f.at(vertex_index * 4 + 2), mesh.vPos4f.at(vertex_index * 4 + 3));
    }
    else if(attrib_name == "normal")
    {
      res = float4(mesh.vNorm4f.at(vertex_index * 4 + 0), mesh.vNorm4f.at(vertex_index * 4 + 1),
                   mesh.vNorm4f.at(vertex_index * 4 + 2), mesh.vNorm4f.at(vertex_index * 4 + 3));
    }
    else if(attrib_name == "tangent")
    {
      res = float4(mesh.vTang4f.at(vertex_index * 4 + 0), mesh.vTang4f.at(vertex_index * 4 + 1),
                   mesh.vTang4f.at(vertex_index * 4 + 2), mesh.vTang4f.at(vertex_index * 4 + 3));
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

  float2 vertex_attrib_by_index_f2(const std::string &attrib_name, uint32_t vertex_index, const cmesh::SimpleMesh& mesh)
  {
    float2 res;
    if(attrib_name == "uv")
    {
      res = float2(mesh.vTexCoord2f.at(vertex_index * 2 + 0), mesh.vTexCoord2f.at(vertex_index * 2 + 1));
    }
  
    return res;
  }

  void update_vertex_attrib_by_index_f2(float2 new_val, uint32_t vertex_index, std::vector <float> &attrib_vec)
  {
    attrib_vec.at(vertex_index * 2 + 0) = new_val.x;
    attrib_vec.at(vertex_index * 2 + 1) = new_val.y;
  }

};



void cmesh::WeldVertices(cmesh::SimpleMesh& mesh, int indexNum)
{
  std::vector<uint32_t> indices_new;
  //std::unordered_map<float3, uint32_t, float3_hash, pos_eq> vertex_hash;
  std::unordered_map<vertex_cache, uint32_t, cmesh::vertex_cache_hash, cmesh::vertex_cache_eq> vertex_hash;

  std::vector<float> vertices_new(mesh.vPos4f.size()*2, 0.0f);
  std::vector<float> normals_new(mesh.vNorm4f.size()*2, 0.0f);
  std::vector<float> uv_new(mesh.vTexCoord2f.size()*2, 0.0f);
  std::vector<float> tangents_new(mesh.vTang4f.size()*2, 0.0f);
  std::vector<int32_t> mid_new;
  mid_new.reserve(mesh.matIndices.size());  

  uint32_t index = 0;
  for (auto i = 0u; i < mesh.indices.size(); i += 3)
  {
    const uint32_t indA = mesh.indices[i + 0];
    const uint32_t indB = mesh.indices[i + 1];
    const uint32_t indC = mesh.indices[i + 2];

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

  mesh.vPos4f.clear();
  mesh.vPos4f.resize(vertices_new.size());
  std::copy(vertices_new.begin(), vertices_new.end(), mesh.vPos4f.begin());

  mesh.vNorm4f.clear();
  mesh.vNorm4f.resize(normals_new.size());
  std::copy(normals_new.begin(), normals_new.end(), mesh.vNorm4f.begin());

  mesh.vTexCoord2f.clear();
  mesh.vTexCoord2f.resize(uv_new.size());
  std::copy(uv_new.begin(), uv_new.end(), mesh.vTexCoord2f.begin());

  mesh.vTang4f.clear();
  mesh.vTang4f.resize(tangents_new.size());
  std::copy(tangents_new.begin(), tangents_new.end(), mesh.vTang4f.begin());

  mesh.indices.clear();
  mesh.indices.resize(indices_new.size());
  std::copy(indices_new.begin(), indices_new.end(), mesh.indices.begin());

  mesh.matIndices.clear();
  mesh.matIndices.resize(mid_new.size());
  std::copy(mid_new.begin(), mid_new.end(), mesh.matIndices.begin());

  indexNum = int(indices_new.size());
}
