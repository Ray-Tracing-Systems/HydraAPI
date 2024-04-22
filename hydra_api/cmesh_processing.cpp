#include "cmesh.h"
#include <cstring>

#include "LiteMath.h"
using LiteMath::float4;
using LiteMath::float3;
using LiteMath::float2;

#include <cmath>
#include <fstream>

void cmesh_hapi::ComputeNormals(cmesh_hapi::SimpleMesh& mesh, const int indexNum, bool useFaceNormals)
{
  int faceNum = indexNum / 3;

  std::vector<float3> vertexNormals(mesh.vPos4f.size() / 4, float3(0.0, 0.0, 0.0));

  for (auto i = 0; i < faceNum; ++i)
  {
    float3 A = float3(mesh.vPos4f.at(4 * mesh.indices.at(3*i)),     mesh.vPos4f.at(4 * mesh.indices.at(3*i) + 1),     mesh.vPos4f.at(4 * mesh.indices.at(3*i) + 2));
    float3 B = float3(mesh.vPos4f.at(4 * mesh.indices.at(3*i + 1)), mesh.vPos4f.at(4 * mesh.indices.at(3*i + 1) + 1), mesh.vPos4f.at(4 * mesh.indices.at(3*i + 1) + 2));
    float3 C = float3(mesh.vPos4f.at(4 * mesh.indices.at(3*i + 2)), mesh.vPos4f.at(4 * mesh.indices.at(3*i + 2) + 1), mesh.vPos4f.at(4 * mesh.indices.at(3*i + 2) + 2));
    
    float3 edge1A = normalize(B - A);
    float3 edge2A = normalize(C - A);

    float3 edge1B = normalize(A - B);
    float3 edge2B = normalize(C - B);

    float3 edge1C = normalize(A - C);
    float3 edge2C = normalize(B - C);

    float3 face_normal = normalize(cross(edge1A, edge2A));
   
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

      // float face_area = 0.5f * sqrtf(powf(edge1A.y * edge2A.z - edge1A.z * edge2A.y, 2) +
      //                                powf(edge1A.z * edge2A.x - edge1A.x * edge2A.z, 2) +
      //                                powf(edge1A.x * edge2A.y - edge1A.y * edge2A.x, 2));
      float face_area = 1.0f;

      float3 normalA = face_normal * wA * face_area;
      float3 normalB = face_normal * wB * face_area;
      float3 normalC = face_normal * wC * face_area;

      vertexNormals.at(mesh.indices.at(3 * i + 0)) += normalA;
      vertexNormals.at(mesh.indices.at(3 * i + 1)) += normalB;
      vertexNormals.at(mesh.indices.at(3 * i + 2)) += normalC;
    }
    else
    {
      vertexNormals.at(mesh.indices.at(3 * i + 0)) += face_normal;
      vertexNormals.at(mesh.indices.at(3 * i + 1)) += face_normal;
      vertexNormals.at(mesh.indices.at(3 * i + 2)) += face_normal;
    }
    //faceNormals.push_back(face_normal);
  }

  if(mesh.vNorm4f.size() != mesh.vPos4f.size())
    mesh.vNorm4f.resize(mesh.vPos4f.size());

  for (int i = 0; i < vertexNormals.size(); ++i)
  {
    float3 N = normalize(vertexNormals.at(i));

    mesh.vNorm4f.at(4 * i + 0) = N.x;
    mesh.vNorm4f.at(4 * i + 1) = N.y;
    mesh.vNorm4f.at(4 * i + 2) = N.z;
    mesh.vNorm4f.at(4 * i + 3) = 0.0f;
  }

}

using std::isnan;
using std::isinf;

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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace cmesh_hapi
{
  void MikeyTSpaceCalc(cmesh_hapi::SimpleMesh* pInput, bool basic);
};

void cmesh_hapi::ComputeTangents(cmesh_hapi::SimpleMesh& mesh, int indexNum, TANG_ALGORITHMS a_alg)
{
  mesh.vTang4f.resize(mesh.vPos4f.size());

  if(a_alg == TANG_MIKEY)
  {
    MikeyTSpaceCalc(&mesh, false);
    return;
  }
  else
  {
    const int vertexCount      = int(mesh.vPos4f.size()/4);                   // #TODO: not 0-th element, last vertex from prev append!
    const int triangleCount    = indexNum / 3;
  
    const float4* verticesPos  = (const float4*)(mesh.vPos4f.data());         // #TODO: not 0-th element, last vertex from prev append!
    const float4* verticesNorm = (const float4*)(mesh.vNorm4f.data());        // #TODO: not 0-th element, last vertex from prev append!
    const float2* vertTexCoord = (const float2*)(mesh.vTexCoord2f.data());    // #TODO: not 0-th element, last vertex from prev append!
  
    float4* verticesTang       = (float4*)(mesh.vTang4f.data());              // #TODO: not 0-th element, last vertex from prev append!
  
  
    HR_ComputeTangentSpaceSimple(vertexCount, triangleCount, mesh.indices.data(),
                                 verticesPos, verticesNorm, vertTexCoord,
                                 verticesTang);
  }
}