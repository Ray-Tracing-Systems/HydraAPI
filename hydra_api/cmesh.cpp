#include "cmesh.h"
#include "HydraVSGFExport.h"

#include <cmath>
#include <fstream>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void cmesh_hapi::SimpleMesh::Resize(int a_vertNum, int a_indNum)
{
  vPos4f.resize(a_vertNum*4);
  vNorm4f.resize(a_vertNum*4);
  vTang4f.resize(a_vertNum*4);
  vTexCoord2f.resize(a_vertNum*2);
  indices.resize(a_indNum);
  matIndices.resize(a_indNum/PolySize()); 
}

void cmesh_hapi::SimpleMesh::Clear()
{
  vPos4f.clear();
  vNorm4f.clear();
  vTexCoord2f.clear();
  vTang4f.clear();
  indices.clear();
  matIndices.clear();
  customArrays.clear();
}

void cmesh_hapi::SimpleMesh::FreeMem()
{
  vPos4f      = std::vector<float, aligned<float,16> >();
  vNorm4f     = std::vector<float, aligned<float,16> >();
  vTang4f     = std::vector<float, aligned<float,16> >();
  vTexCoord2f = std::vector<float>();
  indices     = std::vector<uint32_t>();
  matIndices  = std::vector<uint32_t>();
  customArrays.clear();                       /// its a vector of vectors
}

size_t align_to_16(size_t type_size, size_t N)
{
  size_t requested = type_size * N;
  return (requested % 16) / type_size + N;
}

void cmesh_hapi::SimpleMesh::Reserve(size_t vNum, size_t indNum)
{
  vPos4f.reserve (align_to_16(sizeof(float), vNum * 4 + 10));
  vNorm4f.reserve(align_to_16(sizeof(float), vNum * 4 + 10));
  vTang4f.reserve(align_to_16(sizeof(float), vNum * 4 + 10));
  vTexCoord2f.reserve(align_to_16(sizeof(float), vNum * 2 + 10));
  
  if(indices.capacity() < indNum + 10)
    indices.reserve(align_to_16(sizeof(uint32_t), indNum + 10));
  
  if(matIndices.capacity() < indNum / PolySize() + 10) 
    matIndices.reserve(align_to_16(sizeof(uint32_t), indNum / PolySize() + 10));     
  
  for (auto& arr : customArrays)
  {
    arr.idata.reserve(align_to_16(sizeof(uint32_t), 1*vNum));
    arr.fdata.reserve(align_to_16(sizeof(float), 4*vNum));
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


std::vector<uint32_t> CreateQuadTriIndices(const int a_sizeX, const int a_sizeY)
{
  std::vector<uint32_t> indicesData(a_sizeY*a_sizeX * 6);
  auto* indexBuf = indicesData.data();

  for (int i = 0; i < a_sizeY; i++)
  {
    for (int j = 0; j < a_sizeX; j++)
    {
      *indexBuf++ = (i + 0) * (a_sizeX + 1) + (j + 0);
      *indexBuf++ = (i + 1) * (a_sizeX + 1) + (j + 0);
      *indexBuf++ = (i + 1) * (a_sizeX + 1) + (j + 1);  
      *indexBuf++ = (i + 0) * (a_sizeX + 1) + (j + 0);
      *indexBuf++ = (i + 1) * (a_sizeX + 1) + (j + 1);
      *indexBuf++ = (i + 0) * (a_sizeX + 1) + (j + 1);
    }
  }

  return indicesData;
}

cmesh_hapi::SimpleMesh cmesh_hapi::CreateQuad(const int a_sizeX, const int a_sizeY, const float a_size)
{
  const int vertNumX = a_sizeX + 1;
  const int vertNumY = a_sizeY + 1;

  const int quadsNum = a_sizeX*a_sizeY;
  const int vertNum  = vertNumX*vertNumY;

  cmesh_hapi::SimpleMesh res(vertNum, quadsNum*2*3);

  const float edgeLength  = a_size / float(a_sizeX);
  const float edgeLength2 = sqrtf(2.0f)*edgeLength;

  // put vertices
  //
  const float startX = -0.5f*a_size;
  const float startY = -0.5f*a_size;

  for (int y = 0; y < vertNumY; y++)
  {
    const float ypos = startY + float(y)*edgeLength;
    const int offset = y*vertNumX;

    for (int x = 0; x < vertNumX; x++)
    { 
      const float xpos = startX + float(x)*edgeLength;
      const int i      = offset + x;

      res.vPos4f [i * 4 + 0] = xpos;
      res.vPos4f [i * 4 + 1] = ypos;
      res.vPos4f [i * 4 + 2] = 0.0f;
      res.vPos4f [i * 4 + 3] = 1.0f;

      res.vNorm4f[i * 4 + 0] = 0.0f;
      res.vNorm4f[i * 4 + 1] = 0.0f;
      res.vNorm4f[i * 4 + 2] = 1.0f;
      res.vNorm4f[i * 4 + 3] = 0.0f;

      res.vTexCoord2f[i*2 + 0] = (xpos - startX) / a_size;
      res.vTexCoord2f[i*2 + 1] = (ypos - startY) / a_size;
    }
  }
 
  res.indices = CreateQuadTriIndices(a_sizeX, a_sizeY);

  return res;
}

cmesh_hapi::SimpleMesh cmesh_hapi::LoadMeshFromVSGF(const char* a_fileName)
{
  std::ifstream input(a_fileName, std::ios::binary);
  if(!input.is_open())
    return SimpleMesh();

  HydraGeomData data;
  data.read(input); 

  SimpleMesh res(data.getVerticesNumber(), data.getIndicesNumber());

  memcpy(res.vPos4f.data(),      data.getVertexPositionsFloat4Array(), res.vPos4f.size()*sizeof(float));
  memcpy(res.vNorm4f.data(),     data.getVertexNormalsFloat4Array(),   res.vNorm4f.size()*sizeof(float));
  
  if(data.getVertexTangentsFloat4Array() != nullptr)
    memcpy(res.vTang4f.data(),     data.getVertexTangentsFloat4Array(),  res.vTang4f.size()*sizeof(float));
  else
    memset(res.vTang4f.data(), 0, res.vTang4f.size()*sizeof(float));

  memcpy(res.vTexCoord2f.data(), data.getVertexTexcoordFloat2Array(),  res.vTexCoord2f.size()*sizeof(float));

  memcpy(res.indices.data(),     data.getTriangleVertexIndicesArray(),   res.indices.size()*sizeof(int));
  memcpy(res.matIndices.data(),  data.getTriangleMaterialIndicesArray(), res.matIndices.size()*sizeof(int));

  return res; 
}