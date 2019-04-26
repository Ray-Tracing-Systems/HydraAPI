//
// Created by frol on 05.04.19.
//

#include "HydraVSGFCompress.h"
#include <fstream>

#include "HydraRenderDriverAPI.h"
std::vector<HRBatchInfo> FormMatDrawListRLE(const std::vector<uint32_t>& matIndices);

#include "../corto/corto.h"
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

using HydraLiteMath::float4;
using HydraLiteMath::float2;

void HR_ComputeTangentSpaceSimple(const int     vertexCount, const int     triangleCount, const uint32_t* triIndices,
                                  const float4* verticesPos, const float4* verticesNorm, const float2* vertTexCoord,
                                  float4* verticesTang);

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

  HR_ComputeTangentSpaceSimple(a_data.getVerticesNumber(), a_data.getIndicesNumber()/3, a_data.getTriangleVertexIndicesArray(),
                               (float4*)positions, (float4*)normals, (float2*)a_data.getVertexTexcoordFloat2Array(),
                               (float4*)tangents);
}


#include "HydraObjectManager.h"   // #TODO: remove this
#include "vfloat4_x64.h"

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

size_t HR_SaveVSGFCompressed(const HydraGeomData& data, const wchar_t* a_outfileName, const char* a_customData, const int a_customDataSize)
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
  const auto& bbox        = CalcBoundingBox4f(data.getVertexPositionsFloat4Array(), data.getVerticesNumber());

  //std::cout << "bbox.x_min\t" << bbox.x_min << std::endl;
  //std::cout << "bbox.x_max\t" << bbox.x_max << std::endl;
  
  //const std::string xmlNodeData = "";

  HydraHeaderC h2;

  h2.compressedSizeInBytes = 0;
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
  h2.compressedSizeInBytes = compressedBytes;
  fout.seekp(sizeof(HydraGeomData::Header), ios_base::beg);
  fout.write((char*)&h2, sizeof(HydraHeaderC));

  return totalFileSize;
}

size_t HR_SaveVSGFCompressed(const void* vsgfData, size_t a_vsgfSize, const wchar_t* a_outfileName, const char* a_customData, const int a_dataSize)
{
  HydraGeomData::Header* pHeader = (HydraGeomData::Header*)vsgfData;

  const VSGFOffsets offsets = CalcOffsets(pHeader->verticesNum, pHeader->indicesNum);

  char* p = (char*)vsgfData;

  HydraGeomData data;

  data.setData(uint32_t(pHeader->verticesNum), (float*)   (p + offsets.offsetPos),    (float*)(p + offsets.offsetNorm),
               (float*)   (p + offsets.offsetTang),   (float*)(p + offsets.offsetTexc),
               uint32_t(pHeader->indicesNum),  (uint32_t*)(p + offsets.offsetInd), (uint32_t*)(p + offsets.offsetMind));

  return HR_SaveVSGFCompressed(data, a_outfileName, a_customData, a_dataSize);
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

  HR_LoadVSGFCompressedBothHeaders(fin,
                                   batchList, h1, h2);

  
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

  ReadCompressed(data, fin, h2.compressedSizeInBytes);
  

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
  HydraGeomData data = HR_LoadVSGFCompressedData(a_path.c_str(), dataBuffer);
  std::ofstream fout;
  hr_ofstream_open(fout, a_newPath.c_str());
  data.write(fout);
}