#include "HydraVSGFExport.h"

#include <fstream>
#include <sstream>
#include <cassert>

std::vector<HRBatchInfo> FormMatDrawListRLE(const std::vector<uint32_t>& matIndices);

HydraGeomData::HydraGeomData()
{
  m_ownMemory = false;

  m_data      = nullptr;
  m_positions = nullptr;
  m_normals   = nullptr;
  m_tangents  = nullptr;
  m_texcoords = nullptr;

  m_triVertIndices     = nullptr;
  m_triMaterialIndices = nullptr;
  m_materialNames      = nullptr;

  m_matNamesTotalStringSize = 0;
  fileSizeInBytes = 0;
  verticesNum     = 0;
  indicesNum      = 0;
  materialsNum    = 0;
  flags           = 0;
}

HydraGeomData::~HydraGeomData()
{
  freeMemIfNeeded();
}

void HydraGeomData::freeMemIfNeeded()
{
  if(m_ownMemory)
    delete [] m_data;
  m_data = nullptr;
}

uint32_t     HydraGeomData::getVerticesNumber()             const { return verticesNum; }
const float* HydraGeomData::getVertexPositionsFloat4Array() const { return (const float*)m_positions; }
const float* HydraGeomData::getVertexNormalsFloat4Array()   const { return (const float*)m_normals;   }
const float* HydraGeomData::getVertexTangentsFloat4Array()  const { return (const float*)m_tangents;  }
const float* HydraGeomData::getVertexTexcoordFloat2Array()  const { return (const float*)m_texcoords; }

uint32_t HydraGeomData::getIndicesNumber()                        const { return indicesNum; }
const uint32_t* HydraGeomData::getTriangleVertexIndicesArray()    const { return m_triVertIndices; }
const uint32_t* HydraGeomData::getTriangleMaterialIndicesArray()  const { return m_triMaterialIndices; }

const float* HydraGeomData::getVertexLightmapTexcoordFloat2Array()  const { return nullptr; }
const float* HydraGeomData::getVertexSphericalHarmonicCoeffs()  const {return nullptr; }

void HydraGeomData::setData(uint32_t a_vertNum, const float* a_pos, const float* a_norm, const float* a_tangent, const float* a_texCoord,
                            uint32_t a_indicesNum, const uint32_t* a_triVertIndices, const uint32_t* a_triMatIndices)
{
  verticesNum = a_vertNum;
  indicesNum  = a_indicesNum;

  m_positions = a_pos;
  m_normals   = a_norm;
  m_tangents  = a_tangent;

  m_texcoords = a_texCoord;

  m_triVertIndices     = a_triVertIndices;
  m_triMaterialIndices = a_triMatIndices;
}

size_t HydraGeomData::sizeInBytes()
{
  const size_t szInBytes = size_t(sizeof(float))*(verticesNum*4*3  + verticesNum*2) +
                           size_t(sizeof(int))*(indicesNum + indicesNum/3);

  return szInBytes + size_t(sizeof(Header));
}

void HydraGeomData::write(std::ostream& a_out)
{
  if(m_tangents != nullptr)
    flags |= HAS_TANGENT;

  if(m_normals == nullptr)
    flags |= HAS_NO_NORMALS;

  fileSizeInBytes = verticesNum*( sizeof(float)*4 + sizeof(float)*2 );
  if(!(flags & HAS_NO_NORMALS))
    fileSizeInBytes += verticesNum*sizeof(float)*4;
  if(flags & HAS_TANGENT)
    fileSizeInBytes += verticesNum*sizeof(float)*4;

  fileSizeInBytes += (indicesNum / 3) * 4 * sizeof(uint32_t); // 3*num_triangles + num_triangles = 4*num_triangles
  materialsNum = 0;

  // write data
  //
  a_out.write((const char*)&fileSizeInBytes, sizeof(uint64_t));
  a_out.write((const char*)&verticesNum, sizeof(uint32_t));
  a_out.write((const char*)&indicesNum, sizeof(uint32_t));
  a_out.write((const char*)&materialsNum, sizeof(uint32_t));
  a_out.write((const char*)&flags, sizeof(uint32_t));

  a_out.write((const char*)m_positions, sizeof(float)*4*verticesNum);

  if(!(flags & HAS_NO_NORMALS))
    a_out.write((const char*)m_normals, sizeof(float)*4*verticesNum);

  if(flags & HAS_TANGENT)
    a_out.write((const char*)m_tangents, sizeof(float)*4*verticesNum);

  a_out.write((const char*)m_texcoords, sizeof(float)*2*verticesNum);

  a_out.write((const char*)m_triVertIndices, sizeof(uint32_t)*indicesNum);
  a_out.write((const char*)m_triMaterialIndices, sizeof(uint32_t)*(indicesNum / 3));
  
}

// void mymemcpy(void* a_dst, void* a_src, int a_size)
// {
//   char* dst = (char*)a_dst;
//   char* src = (char*)a_src;
// 
//   for (int i = 0; i < a_size; i++)
//     dst[i] = src[i];
// }

void HydraGeomData::writeToMemory(char* a_dataToWrite)
{
  if (m_normals == nullptr)
    flags |= HAS_NO_NORMALS;

  if (m_tangents != nullptr)
    flags |= HAS_TANGENT;

  fileSizeInBytes = verticesNum*(sizeof(float) * 4 + sizeof(float) * 2);

  if (!(flags & HAS_NO_NORMALS))
    fileSizeInBytes += verticesNum*sizeof(float) * 4;

  if (flags & HAS_TANGENT)
    fileSizeInBytes += verticesNum*sizeof(float) * 4;

  fileSizeInBytes += (indicesNum / 3) * 4 * sizeof(uint32_t); // 3*num_triangles + num_triangles = 4*num_triangles
  fileSizeInBytes += sizeof(Header);
  materialsNum = 0;

  // write data
  //
  char* ptr = a_dataToWrite;

  Header header;

  header.fileSizeInBytes = fileSizeInBytes;
  header.verticesNum     = verticesNum;
  header.indicesNum      = indicesNum;
  header.materialsNum    = materialsNum;
  header.flags           = flags;

  memcpy(ptr, &header,     sizeof(Header));              ptr += sizeof(Header);
  memcpy(ptr, m_positions, sizeof(float)*4*verticesNum); ptr += sizeof(float) * 4 * verticesNum;

  if(!(flags & HAS_NO_NORMALS))
  {
    memcpy(ptr, m_normals, sizeof(float) * 4 * verticesNum);
    ptr += sizeof(float) * 4 * verticesNum;
  }

  if (flags & HAS_TANGENT)
  {
    memcpy(ptr, m_tangents, sizeof(float) * 4 * verticesNum);
    ptr += sizeof(float) * 4 * verticesNum;
  }

  memcpy(ptr, m_texcoords, sizeof(float) * 2 * verticesNum); ptr += sizeof(float) * 2 * verticesNum;

  const int mindNum = (indicesNum / 3);

  memcpy(ptr, m_triVertIndices, sizeof(uint32_t)*indicesNum);   ptr += sizeof(uint32_t)*indicesNum;
  memcpy(ptr, m_triMaterialIndices, sizeof(uint32_t)*mindNum);  ptr += sizeof(uint32_t)*mindNum;
}


void HydraGeomData::write(const std::string& a_fileName)
{
  std::ofstream fout(a_fileName.c_str(), std::ios::out | std::ios::binary);
  write(fout);
  fout.flush();
  fout.close();
}


inline const int readInt32(const unsigned char* ptr) // THIS IS CORRECT BOTH FOR X86 AND PPC !!!
{
  const unsigned char b0 = ptr[0];
  const unsigned char b1 = ptr[1];
  const unsigned char b2 = ptr[2];
  const unsigned char b3 = ptr[3];

  return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
}


inline const uint64_t readInt64(const unsigned char* ptr) // THIS IS CORRECT BOTH FOR X86 AND PPC !!!
{
  const unsigned char b0 = ptr[0];
  const unsigned char b1 = ptr[1];
  const unsigned char b2 = ptr[2];
  const unsigned char b3 = ptr[3];
  const unsigned char b4 = ptr[4];
  const unsigned char b5 = ptr[5];
  const unsigned char b6 = ptr[6];
  const unsigned char b7 = ptr[7];

  return  (uint64_t(b7) << 56) | (uint64_t(b6) << 48) | (uint64_t(b5) << 40) | (uint64_t(b4) << 32) | (uint64_t(b3) << 24) | (uint64_t(b2) << 16) | (uint64_t(b1) << 8) | uint64_t(b0);
}

void convertLittleBigEndian(unsigned int* a_buffer, int a_size)
{
  unsigned char* bbuffer = (unsigned char*)a_buffer;

  for(int i=0;i<a_size;i++)
    a_buffer[i] = readInt32(bbuffer + i*4);
}


// #include <iostream>

void HydraGeomData::read(std::istream& a_input)
{
  freeMemIfNeeded();
  m_ownMemory = true;

  Header info;

  unsigned char temp1[sizeof(Header)];
  a_input.read((char*)&temp1[0], sizeof(Header));

  info.fileSizeInBytes = readInt64(&temp1[0]);
  info.verticesNum     = readInt32(&temp1[8]);
  info.indicesNum      = readInt32(&temp1[12]);
  info.materialsNum    = readInt32(&temp1[16]);
  info.flags           = readInt32(&temp1[20]);

  // std::cout << "HydraGeomData import:" << std::endl;
  // std::cout << "info.fileSizeInBytes = " << info.fileSizeInBytes << std::endl;
  // std::cout << "info.verticesNum     = " << info.verticesNum << std::endl;
  // std::cout << "info.indicesNum      = " << info.indicesNum << std::endl;
  // std::cout << "info.materialsNum    = " << info.materialsNum << std::endl;
  // std::cout << "info.flags           = " << info.flags << std::endl;

  fileSizeInBytes = info.fileSizeInBytes;
  verticesNum     = info.verticesNum;
  indicesNum      = info.indicesNum;
  materialsNum    = info.materialsNum;
  flags           = info.flags;

  m_data = new char [info.fileSizeInBytes];
  a_input.read(m_data, info.fileSizeInBytes);

  // std::cout << "[HydraGeomData] data was read" << std::endl;

  char* ptr = m_data;

  m_positions = (float*)ptr; ptr += sizeof(float)*4*verticesNum;

  if(!(flags & HAS_NO_NORMALS))
  {
    m_normals = (float *) ptr;
    ptr += sizeof(float) * 4 * verticesNum;
  }

  if(flags & HAS_TANGENT)
  {
    m_tangents = (float*)ptr;
    ptr += sizeof(float)*4*verticesNum;
  }

  m_texcoords   = (float*)ptr; ptr += sizeof(float)*2*verticesNum;

  m_triVertIndices     = (uint32_t*)ptr; ptr += sizeof(uint32_t)*indicesNum;
  m_triMaterialIndices = (uint32_t*)ptr; ptr += sizeof(uint32_t)*(indicesNum / 3);

  convertLittleBigEndian((unsigned int*)m_positions, verticesNum*4);
  if (! (flags & HAS_NO_NORMALS))
    convertLittleBigEndian((unsigned int*)m_normals, verticesNum*4);
  convertLittleBigEndian((unsigned int*)m_texcoords, verticesNum*2);
  convertLittleBigEndian((unsigned int*)m_triVertIndices, indicesNum);
  convertLittleBigEndian((unsigned int*)m_triMaterialIndices, (indicesNum/3));
}

void HydraGeomData::read(const std::string& a_fileName)
{
  std::ifstream fin(a_fileName.c_str(), std::ios::binary);

  if (!fin.is_open())
    return;

  read(fin);

  fin.close();
}

void HydraGeomData::read(const std::wstring& a_fileName)
{

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
  std::wstring s1(a_fileName);
  std::string  s2(s1.begin(), s1.end());
  std::ifstream fin(s2.c_str(), std::ios::binary);
#elif defined WIN32
  std::ifstream fin(a_fileName.c_str(), std::ios::binary);
#endif

  if (!fin.is_open())
    return;

  read(fin);

  fin.close();
}

#include "../corto/corto.h"
#include "LiteMath.h"

size_t HydraGeomData::writeCompressed(std::ostream& a_out, const std::vector<HRBatchInfo>& a_batchesList) const
{
  // convert positions to float3 due to corto does not understand float4 input for positions by default
  //
  std::vector<float> positions3(verticesNum*3);
  std::vector<float> normals3  (verticesNum*3);
  
  for(int i=0;i<verticesNum;i++)
  {
    positions3[i*3+0] = m_positions[i*4+0];
    positions3[i*3+1] = m_positions[i*4+1];
    positions3[i*3+2] = m_positions[i*4+2];
  
    normals3[i*3+0]   = m_normals[i*4+0];
    normals3[i*3+1]   = m_normals[i*4+1];
    normals3[i*3+2]   = m_normals[i*4+2];
  }
  
  const int   uv_bits    = 12;
  const int   norm_bits  = 10;
  //auto* normalAttrs      = new crt::NormalAttr(norm_bits);
  
  crt::Encoder encoder(verticesNum, indicesNum/3);
  {
    encoder.addPositions(positions3.data(), m_triVertIndices); // vertex_quantization_step
    encoder.addUvs(m_texcoords, pow(2, -uv_bits));
    encoder.addNormals(normals3.data(), norm_bits, crt::NormalAttr::ESTIMATED);
    
    //normalAttrs->format     = crt::VertexAttribute::FLOAT;
    //normalAttrs->prediction = 0;
    
    for(auto& batch : a_batchesList) // preserve groups to save material indices per triangle
      encoder.addGroup(batch.triEnd);
  }
  encoder.encode();
  
  //delete normalAttrs; normalAttrs = nullptr;
  
  const auto*    compressed_data = encoder.stream.data();
  const uint32_t compressed_size = encoder.stream.size();
  
  a_out.write((char*)compressed_data, compressed_size);
  
  return size_t(compressed_size);
}

using HydraLiteMath::float4;
using HydraLiteMath::float2;

void HR_ComputeTangentSpaceSimple(const int     vertexCount, const int     triangleCount, const uint32_t* triIndices,
                                  const float4* verticesPos, const float4* verticesNorm, const float2* vertTexCoord,
                                  float4* verticesTang);

void HydraGeomData::readCompressed(std::istream& a_input, size_t a_compressedSize)
{
  std::vector<char> dataTemp(a_compressedSize);
  a_input.read(dataTemp.data(), a_compressedSize);

  std::vector<float> positions3(verticesNum*3);
  std::vector<float> normals3  (verticesNum*3);

  crt::Decoder decoder(a_compressedSize, (const unsigned char*)dataTemp.data());
  {
    decoder.setPositions(positions3.data());
    decoder.setIndex((uint32_t*)m_triVertIndices);
    decoder.setUvs(const_cast<float*>(m_texcoords));
    decoder.setNormals(normals3.data());
  }
  decoder.decode();

  float* positions = const_cast<float*>(m_positions);
  float* normals   = const_cast<float*>(m_normals);
  float* tangents  = const_cast<float*>(m_tangents);

  for(int i=0;i<verticesNum;i++)
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
  
  HR_ComputeTangentSpaceSimple(verticesNum, indicesNum/3, m_triVertIndices,
                               (float4*)m_positions, (float4*)m_normals, (float2*)m_texcoords,
                               (float4*)tangents);
  
}


VSGFOffsets CalcOffsets(int numVert, int numInd)
{
  VSGFOffsets res;
  
  res.offsetPos  = sizeof(HydraGeomData::Header);
  res.offsetNorm = res.offsetPos  + numVert*sizeof(float)*4; // after pos
  res.offsetTang = res.offsetNorm + numVert*sizeof(float)*4; // after norm
  res.offsetTexc = res.offsetTang + numVert*sizeof(float)*4; // after tangent
  res.offsetInd  = res.offsetTexc + numVert*sizeof(float)*2; // after texcoord
  res.offsetMind = res.offsetInd  + numInd*sizeof(int);      // after ind
  
  return res;
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

size_t HR_SaveVSGFCompressed(const HydraGeomData& data, const wchar_t* a_outfileName)
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
  
  //const std::string xmlNodeData = "";
  
  HydraGeomData::HeaderC h2;
  
  h2.compressedSizeInBytes = 0;
  h2.batchListArraySize    = matDrawList.size();
  h2.boxMin[0]             = bbox.x_min;
  h2.boxMin[1]             = bbox.y_min;
  h2.boxMin[2]             = bbox.z_min;
  h2.boxMax[0]             = bbox.x_max;
  h2.boxMax[1]             = bbox.y_max;
  h2.boxMax[2]             = bbox.z_max;
  h2.dummy                 = 123456;
  
  fout.write((char*)&h2, sizeof(HydraGeomData::HeaderC));
  
  // (4) put material batch list
  //
  fout.write((char*)matDrawList.data(), matDrawList.size()*sizeof(HRBatchInfo));
  
  // (5) #TODO: put xml data of mesh in file ...
  //
  
  // (6) compress mesh via corto lib
  //
  auto compressedBytes = data.writeCompressed(fout,matDrawList);
  
  size_t totalFileSize = sizeof(HydraGeomData::Header)  +
                         sizeof(HydraGeomData::HeaderC) +
                         matDrawList.size()*sizeof(HRBatchInfo) + compressedBytes;
  
  // write true file size to 'h2.compressedSizeInBytes' in file
  //
  h2.compressedSizeInBytes = compressedBytes;
  fout.seekp(sizeof(HydraGeomData::Header), ios_base::beg);
  fout.write((char*)&h2, sizeof(HydraGeomData::HeaderC));
  
  return totalFileSize;
}

size_t HR_SaveVSGFCompressed(const void* vsgfData, size_t a_vsgfSize, const wchar_t* a_outfileName)
{
  HydraGeomData::Header* pHeader = (HydraGeomData::Header*)vsgfData;
  
  const VSGFOffsets offsets = CalcOffsets(pHeader->verticesNum, pHeader->indicesNum);
  
  char* p = (char*)vsgfData;
  
  HydraGeomData data;
  
  data.setData(uint32_t(pHeader->verticesNum), (float*)   (p + offsets.offsetPos),    (float*)(p + offsets.offsetNorm),
                                               (float*)   (p + offsets.offsetTang),   (float*)(p + offsets.offsetTexc),
               uint32_t(pHeader->indicesNum),  (uint32_t*)(p + offsets.offsetInd), (uint32_t*)(p + offsets.offsetMind));
  
  return HR_SaveVSGFCompressed(data, a_outfileName);
}


void HR_LoadVSGFCompressedBothHeaders(std::ifstream& fin,
                                      std::vector<HRBatchInfo>& a_outBatchList, HydraGeomData::Header& h1, HydraGeomData::HeaderC& h2)
{
  fin.read((char*)&h1, sizeof(HydraGeomData::Header));
  fin.read((char*)&h2, sizeof(HydraGeomData::HeaderC));
  
  a_outBatchList.resize(h2.batchListArraySize);
  
  fin.read((char*)a_outBatchList.data(), sizeof(HRBatchInfo)*a_outBatchList.size());
}


HydraGeomData HR_LoadVSGFCompressedData(const wchar_t* a_fileName, std::vector<int>& dataBuffer)
{
  std::ifstream fin;
  hr_ifstream_open(fin, a_fileName);

  std::vector<HRBatchInfo> batchList;
  HydraGeomData::Header    h1;
  HydraGeomData::HeaderC   h2;

  HR_LoadVSGFCompressedBothHeaders(fin,
                                   batchList, h1, h2);

  dataBuffer.resize(h1.fileSizeInBytes / sizeof(int) + 1);
  char* p = (char*)dataBuffer.data();

  memcpy(p, &h1, sizeof(HydraGeomData::Header));

  const VSGFOffsets offsets = CalcOffsets(h1.verticesNum, h1.indicesNum);

  HydraGeomData data;
  data.setData(uint32_t(h1.verticesNum), (float*)   (p + offsets.offsetPos),    (float*)(p + offsets.offsetNorm),
                                         (float*)   (p + offsets.offsetTang),   (float*)(p + offsets.offsetTexc),
               uint32_t(h1.indicesNum),  (uint32_t*)(p + offsets.offsetInd), (uint32_t*)(p + offsets.offsetMind));
  
  data.readCompressed(fin, h2.compressedSizeInBytes);
  
  int* pMaterialsId = (int*)(p + offsets.offsetMind);

  for(int batchId = 0; batchId < batchList.size(); batchId++)
  {
    const auto& batch = batchList[batchId];
    for(int i=batch.triBegin; i < batch.triEnd; i++)
      pMaterialsId[i] = batch.matId;
  }
  
  return data;
}

void _hrCompressMesh(const std::wstring& a_inPath, const std::wstring& a_outPath)
{
  HydraGeomData data;
  data.read(a_inPath.c_str());
  
  HR_SaveVSGFCompressed(data, a_outPath.c_str());
}

void _hrDecompressMesh(const std::wstring& a_path, const std::wstring& a_newPath)
{
  std::vector<int> dataBuffer;
  HydraGeomData data = HR_LoadVSGFCompressedData(a_path.c_str(), dataBuffer);

  std::ofstream fout;
  hr_ofstream_open(fout, a_newPath.c_str());
  data.write(fout);
}