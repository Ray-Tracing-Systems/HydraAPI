//
// Created by frol on 05.04.19.
//

#include "HydraVSGFCompress.h"
#include <fstream>

#include "HydraRenderDriverAPI.h"
std::vector<HRBatchInfo> FormMatDrawListRLE(const std::vector<uint32_t>& matIndices);

#include "../utils/corto/corto.h"
#include "LiteMath.h"

#ifdef WIN32
#undef min
#undef max
#endif

size_t WriteCompressed(const HydraGeomData& a_data, std::ostream& a_out, const std::vector<HRBatchInfo>& a_batchesList)
{
  // convert positions to float3 due to corto does not understand float4 input for positions by default
  //

  const float* positions4 = a_data.getVertexPositionsFloat4Array();
  const float* normals4   = a_data.getVertexNormalsFloat4Array();

  std::vector<float> positions3(a_data.getVerticesNumber()*3);
  std::vector<float> normals3  (a_data.getVerticesNumber()*3);

  for(int i=0;i<a_data.getVerticesNumber();i++)
  {
    positions3[i*3+0] = positions4[i*4+0];
    positions3[i*3+1] = positions4[i*4+1];
    positions3[i*3+2] = positions4[i*4+2];

    normals3[i*3+0]   = normals4[i*4+0];
    normals3[i*3+1]   = normals4[i*4+1];
    normals3[i*3+2]   = normals4[i*4+2];
  }

  const int uv_bits   = 12;
  const int norm_bits = 10;
  //auto* normalAttrs      = new crt::NormalAttr(norm_bits);

  crt::Encoder encoder(a_data.getVerticesNumber(), a_data.getIndicesNumber()/3);
  {
    encoder.addPositions(positions3.data(), a_data.getTriangleVertexIndicesArray()); // vertex_quantization_step
    encoder.addUvs(a_data.getVertexTexcoordFloat2Array(), pow(2, -uv_bits));
    encoder.addNormals(normals3.data(), norm_bits, crt::NormalAttr::ESTIMATED);

    for(auto& batch : a_batchesList) // preserve groups to save material indices per triangle
      encoder.addGroup(batch.triEnd);
  }
  encoder.encode();

  //delete normalAttrs; normalAttrs = nullptr;

  a_out.write((char*)encoder.stream.data(), encoder.stream.size());

  return encoder.stream.size();
}

using LiteMath::float4;
using LiteMath::float2;



void ReadCompressed(HydraGeomData& a_data, std::istream& a_input, size_t a_compressedSize)
{
  std::vector<char> dataTemp(a_compressedSize);
  a_input.read(dataTemp.data(), a_compressedSize);

  std::vector<float> positions3(a_data.getVerticesNumber()*3);
  std::vector<float> normals3  (a_data.getVerticesNumber()*3);

  crt::Decoder decoder(a_compressedSize, (const unsigned char*)dataTemp.data());
  {
    decoder.setPositions(positions3.data());
    decoder.setIndex((uint32_t*)a_data.getTriangleVertexIndicesArray());
    decoder.setUvs(const_cast<float*>(a_data.getVertexTexcoordFloat2Array()));
    decoder.setNormals(normals3.data());
  }
  decoder.decode();

    for(int i = 0; i < a_data.getIndicesNumber(); i += 3)
    {
      auto idx1 = a_data.getTriangleVertexIndicesArray()[i + 0];
      auto idx2 = a_data.getTriangleVertexIndicesArray()[i + 1];
      auto idx3 = a_data.getTriangleVertexIndicesArray()[i + 2];
      if(idx1 < 0 || idx1 >= a_data.getVerticesNumber() ||
         idx2 < 0 || idx2 >= a_data.getVerticesNumber() ||
         idx3 < 0 || idx3 >= a_data.getVerticesNumber())
      {
        auto arr = (uint32_t*) a_data.getTriangleVertexIndicesArray();
        arr[i + 0] = 0;
        arr[i + 1] = 0;
        arr[i + 2] = 0;
      }
    }

  float* positions = const_cast<float*>(a_data.getVertexPositionsFloat4Array());
  float* normals   = const_cast<float*>(a_data.getVertexNormalsFloat4Array());
  float* tangents  = const_cast<float*>(a_data.getVertexTangentsFloat4Array());

  for(int i=0;i<a_data.getVerticesNumber();i++)
  {
    positions[i*4+0] = positions3[i*3+0];
    positions[i*4+1] = positions3[i*3+1];
    positions[i*4+2] = positions3[i*3+2];
    positions[i*4+3] = 1.0f;

    normals[i*4+0]   = normals3[i*3+0];
    normals[i*4+1]   = normals3[i*3+1];
    normals[i*4+2]   = normals3[i*3+2];
    normals[i*4+3]   = 0.0f;
  }
  
}


#include "HydraObjectManager.h"   // #TODO: remove this

BBox CalcBoundingBox4f(const float* in_array, const uint32_t a_vertexNumber)
{
  BBox res;

  cvex::vfloat4 boxMax = {res.x_max, res.x_max, res.x_max, 0};
  cvex::vfloat4 boxMin = {res.x_min, res.x_min, res.x_min, 0};

  for(uint32_t i=0; i < a_vertexNumber; i++) // #TODO: can opt via loop vectorisation
  {
    const cvex::vfloat4 vpos = cvex::load_u(in_array + i*4);
    boxMin = cvex::min(boxMin, vpos);
    boxMax = cvex::max(boxMax, vpos);
  }

  float boxMin2[4], boxMax2[4];
  {
    cvex::store_u(boxMin2, boxMin);
    cvex::store_u(boxMax2, boxMax);
  }

  res.x_min = boxMin2[0];
  res.y_min = boxMin2[1];
  res.z_min = boxMin2[2];

  res.x_max = boxMax2[0];
  res.y_max = boxMax2[1];
  res.z_max = boxMax2[2];

  return res;
}

size_t HR_SaveVSGFCompressed(const HydraGeomData& data, const wchar_t* a_outfileName, const char* a_customData, const int a_customDataSize, bool a_placeToOrigin)
{
  // (1) open file
  //
  std::ofstream fout;
  hr_ofstream_open(fout, a_outfileName);

  // (2) put vsgf header
  //
  const auto h1 = data.getHeader();
  fout.write((char*)&h1, sizeof(HydraGeomData::Header));

  // (3) put header2 (bbox, compressed file size in bytes that we have to overwrite later, and other)
  //
  const uint32_t* pInd    = (const uint32_t*)data.getTriangleMaterialIndicesArray();
  const auto      indSize = data.getIndicesNumber()/3;

  const auto& matDrawList = FormMatDrawListRLE(std::vector<uint32_t>(pInd, pInd+indSize));
  auto bbox               = CalcBoundingBox4f(data.getVertexPositionsFloat4Array(), data.getVerticesNumber());

  if (a_placeToOrigin)
  {
    const float centerX = 0.5f*(bbox.x_min + bbox.x_max);
    const float centerY = 0.5f*(bbox.y_min + bbox.y_max);
    const float centerZ = 0.5f*(bbox.z_min + bbox.z_max);

    bbox.x_min -= centerX;
    bbox.y_min -= centerY;
    bbox.z_min -= centerZ;

    bbox.x_max -= centerX;
    bbox.y_max -= centerY;
    bbox.z_max -= centerZ;

    const int vNum = data.getVerticesNumber();
    float4* vertices = (float4*)data.getVertexPositionsFloat4Array();

    for (int i = 0; i < vNum; i++)
    {
      vertices[i].x -= centerX;
      vertices[i].y -= centerY;
      vertices[i].z -= centerZ;
    }
  }

  //std::cout << "bbox.x_min\t" << bbox.x_min << std::endl;
  //std::cout << "bbox.x_max\t" << bbox.x_max << std::endl;
  
  //const std::string xmlNodeData = "";

  HydraHeaderC h2;

  h2.geometrySizeInBytes = 0;
  h2.batchListArraySize    = matDrawList.size();
  h2.boxMin[0]             = bbox.x_min;
  h2.boxMin[1]             = bbox.y_min;
  h2.boxMin[2]             = bbox.z_min;
  h2.boxMax[0]             = bbox.x_max;
  h2.boxMax[1]             = bbox.y_max;
  h2.boxMax[2]             = bbox.z_max;
  h2.customDataOffset      = 0;
  h2.customDataSize        = 0;

  fout.write((char*)&h2, sizeof(HydraHeaderC));

  // (4) put material batch list
  //
  fout.write((char*)matDrawList.data(), matDrawList.size()*sizeof(HRBatchInfo));

  // (5) #TODO: put xml data of mesh in file ...
  //

  // (6) compress mesh via corto lib
  //
  auto compressedBytes = WriteCompressed(data, fout, matDrawList);

  if(a_customDataSize != 0)
    fout.write(a_customData, a_customDataSize);

  size_t totalFileSize = sizeof(HydraGeomData::Header)  +
                         sizeof(HydraHeaderC) +
                         matDrawList.size()*sizeof(HRBatchInfo) + compressedBytes;

  h2.customDataOffset      = uint64_t(totalFileSize);
  h2.customDataSize        = a_customDataSize;

  totalFileSize += a_customDataSize;

  // write true file size to 'h2.compressedSizeInBytes' in file
  //
  h2.geometrySizeInBytes = compressedBytes;
  fout.seekp(sizeof(HydraGeomData::Header), ios_base::beg);
  fout.write((char*)&h2, sizeof(HydraHeaderC));

  return totalFileSize;
}

size_t HR_SaveVSGFUncompressed(const HydraGeomData& data, const wchar_t* a_outfileName, const char* a_customData,
                               const int a_customDataSize, bool a_placeToOrigin)
{
  // (1) open file
  //
  std::ofstream fout;
  hr_ofstream_open(fout, a_outfileName);

  // (2) put vsgf header
  //
  const auto h1 = data.getHeader();
  fout.write((char*)&h1, sizeof(HydraGeomData::Header));

  // (3) write geometry data
  //
  auto geomDataSize = data.writeDataOnly(fout);

  // **********************************************

  // (4) put header2 (bbox, compressed file size in bytes that we have to overwrite later, and other)
  //
  const uint32_t* pInd    = (const uint32_t*)data.getTriangleMaterialIndicesArray();
  const auto      indSize = data.getIndicesNumber()/3;

  const auto& matDrawList = FormMatDrawListRLE(std::vector<uint32_t>(pInd, pInd+indSize));
  auto bbox               = CalcBoundingBox4f(data.getVertexPositionsFloat4Array(), data.getVerticesNumber());

  if (a_placeToOrigin)
  {
    const float centerX = 0.5f*(bbox.x_min + bbox.x_max);
    const float centerY = 0.5f*(bbox.y_min + bbox.y_max);
    const float centerZ = 0.5f*(bbox.z_min + bbox.z_max);

    bbox.x_min -= centerX;
    bbox.y_min -= centerY;
    bbox.z_min -= centerZ;

    bbox.x_max -= centerX;
    bbox.y_max -= centerY;
    bbox.z_max -= centerZ;

    const int vNum = data.getVerticesNumber();
    float4* vertices = (float4*)data.getVertexPositionsFloat4Array();

    for (int i = 0; i < vNum; i++)
    {
      vertices[i].x -= centerX;
      vertices[i].y -= centerY;
      vertices[i].z -= centerZ;
    }
  }

  //std::cout << "bbox.x_min\t" << bbox.x_min << std::endl;
  //std::cout << "bbox.x_max\t" << bbox.x_max << std::endl;

  //const std::string xmlNodeData = "";

  size_t totalFileSize = sizeof(HydraGeomData::Header)  +
                         sizeof(HydraHeaderC) +
                         matDrawList.size()*sizeof(HRBatchInfo) + geomDataSize;

  HydraHeaderC h2{};
  h2.boxMin[0]             = bbox.x_min;
  h2.boxMin[1]             = bbox.y_min;
  h2.boxMin[2]             = bbox.z_min;
  h2.boxMax[0]             = bbox.x_max;
  h2.boxMax[1]             = bbox.y_max;
  h2.boxMax[2]             = bbox.z_max;
  h2.batchListArraySize    = matDrawList.size();
  h2.geometrySizeInBytes   = geomDataSize;
  h2.customDataOffset      = uint64_t(totalFileSize);
  h2.customDataSize        = a_customDataSize;

  fout.write((char*)&h2, sizeof(HydraHeaderC));

  // (5) put material batch list
  //
  fout.write((char*)matDrawList.data(), matDrawList.size()*sizeof(HRBatchInfo));

  // (6) put custom data
  //
  if(a_customDataSize != 0)
    fout.write(a_customData, a_customDataSize);

  totalFileSize += a_customDataSize;

  return totalFileSize;
}

size_t HR_SaveVSGFCompressed(const void* vsgfData, size_t a_vsgfSize, const wchar_t* a_outfileName, const char* a_customData, const int a_dataSize, bool a_placeToOrigin)
{
  HydraGeomData::Header* pHeader = (HydraGeomData::Header*)vsgfData;

  const VSGFOffsets offsets = CalcOffsets(pHeader->verticesNum, pHeader->indicesNum);

  char* p = (char*)vsgfData;

  HydraGeomData data;

  data.setData(uint32_t(pHeader->verticesNum), (float*)   (p + offsets.offsetPos),    (float*)(p + offsets.offsetNorm),
               (float*)   (p + offsets.offsetTang),   (float*)(p + offsets.offsetTexc),
               uint32_t(pHeader->indicesNum),  (uint32_t*)(p + offsets.offsetInd), (uint32_t*)(p + offsets.offsetMind));

  return HR_SaveVSGFCompressed(data, a_outfileName, a_customData, a_dataSize, a_placeToOrigin);
}


void HR_LoadVSGFCompressedBothHeaders(std::ifstream& fin,
                                      std::vector<HRBatchInfo>& a_outBatchList, HydraGeomData::Header& h1, HydraHeaderC& h2)
{
  fin.read((char*)&h1, sizeof(HydraGeomData::Header));
  fin.read((char*)&h2, sizeof(HydraHeaderC));

  a_outBatchList.resize(h2.batchListArraySize);

  fin.read((char*)a_outBatchList.data(), sizeof(HRBatchInfo)*a_outBatchList.size());
}


HydraGeomData HR_LoadVSGFCompressedData(const wchar_t* a_fileName, std::vector<int>& dataBuffer, std::vector<HRBatchInfo>* pOutbatchList)
{
  std::ifstream fin;
  hr_ifstream_open(fin, a_fileName);

  std::vector<HRBatchInfo> batchList;
  HydraGeomData::Header    h1;
  HydraHeaderC             h2;

  HR_LoadVSGFCompressedBothHeaders(fin, batchList, h1, h2);

  const size_t bufferSize = CalcVSGFSize(h1.verticesNum, h1.indicesNum);
  dataBuffer.resize(bufferSize/sizeof(int) + 1);
  char* p = (char*)dataBuffer.data();

  memcpy(p, &h1, sizeof(HydraGeomData::Header));

  const VSGFOffsets offsets = CalcOffsets(h1.verticesNum, h1.indicesNum);
  
  int* pMaterialsId = (int*)(p + offsets.offsetMind);
  
  HydraGeomData data;
  data.setData(uint32_t(h1.verticesNum), (float*)(p + offsets.offsetPos),  (float*)(p + offsets.offsetNorm),
                                         (float*)(p + offsets.offsetTang), (float*)(p + offsets.offsetTexc),
               uint32_t(h1.indicesNum),  (uint32_t*)(p + offsets.offsetInd), (uint32_t*)(pMaterialsId));

  ReadCompressed(data, fin, h2.geometrySizeInBytes);
  

  for(int batchId = 0; batchId < batchList.size(); batchId++)
  {
    const auto& batch = batchList[batchId];
    for(int i=batch.triBegin; i < batch.triEnd; i++)
      pMaterialsId[i] = batch.matId;
  }

  if(pOutbatchList != nullptr)
    (*pOutbatchList) = batchList;

  return data;
}

void _hrCompressMesh(const std::wstring& a_inPath, const std::wstring& a_outPath)
{
  HydraGeomData data;
  data.read(a_inPath.c_str());
  HR_SaveVSGFCompressed(data, a_outPath.c_str(), "", 0);
}

void _hrDecompressMesh(const std::wstring& a_path, const std::wstring& a_newPath)
{
  std::vector<int> dataBuffer;
  auto path_s = ws2s(a_path);
//  auto cMesh = cmesh::LoadMeshFromVSGF(path_s.c_str());
//  cmesh::ComputeTangents(cMesh, cMesh.IndicesNum());
  HydraGeomData data = HR_LoadVSGFCompressedData(a_path.c_str(), dataBuffer);

  cmesh::SimpleMesh cMesh(data.getVerticesNumber(), data.getIndicesNumber());
  memcpy(cMesh.vPos4f.data(), data.getVertexPositionsFloat4Array(), data.getVerticesNumber() * sizeof(float) * 4);
  memcpy(cMesh.vNorm4f.data(), data.getVertexNormalsFloat4Array(), data.getVerticesNumber() * sizeof(float) * 4);
  memcpy(cMesh.vTexCoord2f.data(), data.getVertexTexcoordFloat2Array(), data.getVerticesNumber() * sizeof(float) * 2);
  memcpy(cMesh.indices.data(), data.getTriangleVertexIndicesArray(), data.getIndicesNumber() * sizeof(uint32_t));
  memcpy(cMesh.matIndices.data(), data.getTriangleMaterialIndicesArray(), data.getIndicesNumber() * sizeof(uint32_t) / 3);

  cmesh::ComputeTangents(cMesh, cMesh.IndicesNum());

  std::ifstream fin;
  hr_ifstream_open(fin, a_path.c_str());

  std::vector<HRBatchInfo> batchList;
  HydraGeomData::Header    h1{};
  HydraHeaderC             h2{};

  HR_LoadVSGFCompressedBothHeaders(fin, batchList, h1, h2);

  const size_t bufferSize = CalcVSGFSize(h1.verticesNum, h1.indicesNum);
  dataBuffer.resize(bufferSize/sizeof(int) + 1);
  char* p = (char*)dataBuffer.data();

  memcpy(p, &h1, sizeof(HydraGeomData::Header));

  const VSGFOffsets offsets = CalcOffsets(h1.verticesNum, h1.indicesNum);

  int* pMaterialsId = (int*)(p + offsets.offsetMind);

  data.setData(uint32_t(h1.verticesNum), (float*)(cMesh.vPos4f.data()),  (float*)(cMesh.vNorm4f.data()),
               cMesh.vTang4f.data(), cMesh.vTexCoord2f.data(),
               uint32_t(h1.indicesNum),  cMesh.indices.data(), cMesh.matIndices.data());
//  data.setData(uint32_t(h1.verticesNum), (float*)(p + offsets.offsetPos),  (float*)(p + offsets.offsetNorm),
//               nullptr/*(float*)(p + offsets.offsetTang)*/, (float*)(p + offsets.offsetTexc),
//               uint32_t(h1.indicesNum),  (uint32_t*)(p + offsets.offsetInd), (uint32_t*)(pMaterialsId));
//  {
//    HydraGeomData data2;
//    ReadCompressed(data2, fin, h2.geometrySizeInBytes);
//  }
  fin.seekg(h2.customDataOffset, std::ios_base::beg);

  for(int batchId = 0; batchId < batchList.size(); batchId++)
  {
    const auto& batch = batchList[batchId];
    for(int i=batch.triBegin; i < batch.triEnd; i++)
      pMaterialsId[i] = batch.matId;
  }

  std::vector<char> customData(h2.customDataSize, 0);
  fin.read((char*)customData.data(), h2.customDataSize);
  HR_SaveVSGFUncompressed(data, a_newPath.c_str(), customData.data(), customData.size(), false);

//  std::ofstream fout;
//  hr_ofstream_open(fout, a_newPath.c_str());
//  data.write(fout);
}