#include "HydraVSGFExport.h"

#include <fstream>
#include <sstream>

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
    free(m_data);
}

uint32_t HydraGeomData::getVerticesNumber() const { return verticesNum; }
const float* HydraGeomData::getVertexPositionsFloat4Array() const { return (const float*)m_positions; }
const float* HydraGeomData::getVertexNormalsFloat4Array()   const { return (const float*)m_normals;   }
const float* HydraGeomData::getVertexTangentsFloat4Array()  const { return (const float*)m_tangents;  }
const float* HydraGeomData::getVertexTexcoordFloat2Array()  const { return (const float*)m_texcoords; }

uint32_t HydraGeomData::getIndicesNumber() const { return indicesNum; }
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

  m_triVertIndices = a_triVertIndices;
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

  fileSizeInBytes = verticesNum*( sizeof(float)*4*2 + sizeof(float)*2 );
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
  a_out.write((const char*)m_normals, sizeof(float)*4*verticesNum);

  if(flags & HAS_TANGENT)
    a_out.write((const char*)m_tangents, sizeof(float)*4*verticesNum);

  a_out.write((const char*)m_texcoords, sizeof(float)*2*verticesNum);

  a_out.write((const char*)m_triVertIndices, sizeof(uint32_t)*indicesNum);
  a_out.write((const char*)m_triMaterialIndices, sizeof(uint32_t)*(indicesNum / 3));

  //a_out.write(strData.c_str(), strData.size());

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

  if (m_tangents != nullptr)
    flags |= HAS_TANGENT;

  fileSizeInBytes = verticesNum*(sizeof(float) * 4 * 2 + sizeof(float) * 2);
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
  memcpy(ptr, m_normals,   sizeof(float)*4*verticesNum); ptr += sizeof(float) * 4 * verticesNum;

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
  m_normals   = (float*)ptr; ptr += sizeof(float)*4*verticesNum;

  if(flags & HAS_TANGENT)
  {
    m_tangents = (float*)ptr;
    ptr += sizeof(float)*4*verticesNum;
  }

  m_texcoords   = (float*)ptr; ptr += sizeof(float)*2*verticesNum;

  m_triVertIndices     = (uint32_t*)ptr; ptr += sizeof(uint32_t)*indicesNum;
  m_triMaterialIndices = (uint32_t*)ptr; ptr += sizeof(uint32_t)*(indicesNum / 3);

  convertLittleBigEndian((unsigned int*)m_positions, verticesNum*4);
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


