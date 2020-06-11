#include "cmesh_vsgf.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>

using namespace cmesh;

struct VSGFOffsets
{
  uint64_t offsetPos ;
  uint64_t offsetNorm;
  uint64_t offsetTang;
  uint64_t offsetTexc;
  uint64_t offsetInd ;
  uint64_t offsetMind;
};

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

  m_header.fileSizeInBytes  = 0;
  m_header.verticesNum      = 0;
  m_header.indicesNum       = 0;
  m_header.materialsNum     = 0;
  m_header.flags            = 0;
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

uint32_t     HydraGeomData::getVerticesNumber()             const { return m_header.verticesNum; }
const float* HydraGeomData::getVertexPositionsFloat4Array() const { return (const float*)m_positions; }
const float* HydraGeomData::getVertexNormalsFloat4Array()   const { return (const float*)m_normals;   }
const float* HydraGeomData::getVertexTangentsFloat4Array()  const { return (const float*)m_tangents;  }
const float* HydraGeomData::getVertexTexcoordFloat2Array()  const { return (const float*)m_texcoords; }

uint32_t HydraGeomData::getIndicesNumber()                        const { return m_header.indicesNum; }
const uint32_t* HydraGeomData::getTriangleVertexIndicesArray()    const { return m_triVertIndices; }
const uint32_t* HydraGeomData::getTriangleMaterialIndicesArray()  const { return m_triMaterialIndices; }


void HydraGeomData::setData(uint32_t a_vertNum, const float* a_pos, const float* a_norm, const float* a_tangent, const float* a_texCoord,
                            uint32_t a_indicesNum, const uint32_t* a_triVertIndices, const uint32_t* a_triMatIndices)
{
  m_header.verticesNum     = a_vertNum;
  m_header.indicesNum      = a_indicesNum;
  m_header.fileSizeInBytes = sizeInBytes();
  m_header.flags           = (a_tangent != nullptr) ? HAS_TANGENT : 0;

  m_positions = a_pos;
  m_normals   = a_norm;
  m_tangents  = a_tangent;

  m_texcoords = a_texCoord;

  m_triVertIndices     = a_triVertIndices;
  m_triMaterialIndices = a_triMatIndices;
}

size_t HydraGeomData::sizeInBytes()
{
  const size_t szInBytes = size_t(sizeof(float))*(m_header.verticesNum*4*3  + m_header.verticesNum*2) +
                           size_t(sizeof(int))*(m_header.indicesNum + m_header.indicesNum/3);

  return szInBytes + size_t(sizeof(Header));
}

void HydraGeomData::write(std::ostream& a_out)
{
  if(m_tangents != nullptr)
    m_header.flags |= HAS_TANGENT;

  if(m_normals == nullptr)
    m_header.flags |= HAS_NO_NORMALS;
  
  m_header.fileSizeInBytes = m_header.verticesNum*( sizeof(float)*4 + sizeof(float)*2 );
  if(!(m_header.flags & HAS_NO_NORMALS))
    m_header.fileSizeInBytes += m_header.verticesNum*sizeof(float)*4;
  if(m_header.flags & HAS_TANGENT)
    m_header.fileSizeInBytes += m_header.verticesNum*sizeof(float)*4;
  
  m_header.fileSizeInBytes += (m_header.indicesNum / 3) * 4 * sizeof(uint32_t); // 3*num_triangles + num_triangles = 4*num_triangles
  m_header.materialsNum = 0;

  // write data
  //
  a_out.write((const char*)&m_header.fileSizeInBytes, sizeof(uint64_t));
  a_out.write((const char*)&m_header.verticesNum,     sizeof(uint32_t));
  a_out.write((const char*)&m_header.indicesNum,      sizeof(uint32_t));
  a_out.write((const char*)&m_header.materialsNum,    sizeof(uint32_t));
  a_out.write((const char*)&m_header.flags,           sizeof(uint32_t));

  a_out.write((const char*)m_positions, sizeof(float)*4*m_header.verticesNum);

  if(!(m_header.flags & HAS_NO_NORMALS))
    a_out.write((const char*)m_normals, sizeof(float)*4*m_header.verticesNum);

  if(m_header.flags & HAS_TANGENT)
    a_out.write((const char*)m_tangents, sizeof(float)*4*m_header.verticesNum);

  a_out.write((const char*)m_texcoords,  sizeof(float)*2*m_header.verticesNum);

  a_out.write((const char*)m_triVertIndices,     sizeof(uint32_t)*m_header.indicesNum);
  a_out.write((const char*)m_triMaterialIndices, sizeof(uint32_t)*(m_header.indicesNum / 3));
}

void HydraGeomData::writeToMemory(char* a_dataToWrite)
{
  if (m_normals == nullptr)
    m_header.flags |= HAS_NO_NORMALS;

  if (m_tangents != nullptr)
    m_header.flags |= HAS_TANGENT;
  
  m_header.fileSizeInBytes = m_header.verticesNum*(sizeof(float) * 4 + sizeof(float) * 2);

  if (!(m_header.flags & HAS_NO_NORMALS))
    m_header.fileSizeInBytes += m_header.verticesNum*sizeof(float) * 4;

  if (m_header.flags & HAS_TANGENT)
    m_header.fileSizeInBytes += m_header.verticesNum*sizeof(float) * 4;
  
  m_header.fileSizeInBytes += (m_header.indicesNum / 3) * 4 * sizeof(uint32_t); // 3*num_triangles + num_triangles = 4*num_triangles
  m_header.fileSizeInBytes += sizeof(Header);
  m_header.materialsNum = 0;

  // write data
  //
  char* ptr     = a_dataToWrite;
  Header header = m_header;

  memcpy(ptr, &header,     sizeof(Header));                       ptr += sizeof(Header);
  memcpy(ptr, m_positions, sizeof(float)*4*m_header.verticesNum); ptr += sizeof(float) * 4 * m_header.verticesNum;

  if(!(m_header.flags & HAS_NO_NORMALS))
  {
    memcpy(ptr, m_normals, sizeof(float) * 4 * m_header.verticesNum);
    ptr += sizeof(float) * 4 * m_header.verticesNum;
  }

  if (m_header.flags & HAS_TANGENT)
  {
    memcpy(ptr, m_tangents, sizeof(float) * 4 * m_header.verticesNum);
    ptr += sizeof(float) * 4 * m_header.verticesNum;
  }

  memcpy(ptr, m_texcoords,          sizeof(float)*2* m_header.verticesNum);      ptr += sizeof(float) * 2 * m_header.verticesNum;
  memcpy(ptr, m_triVertIndices,     sizeof(uint32_t)*m_header.indicesNum);       ptr += sizeof(uint32_t)*m_header.indicesNum;
  memcpy(ptr, m_triMaterialIndices, sizeof(uint32_t)*(m_header.indicesNum / 3)); ptr += sizeof(uint32_t)*(m_header.indicesNum / 3);
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

  m_header = info;
  m_data   = new char [info.fileSizeInBytes];
  a_input.read(m_data, info.fileSizeInBytes);

  // std::cout << "[HydraGeomData] data was read" << std::endl;

  char* ptr = m_data;

  m_positions = (float*)ptr; ptr += sizeof(float)*4*m_header.verticesNum;

  if(!(m_header.flags & HAS_NO_NORMALS))
  {
    m_normals = (float *) ptr;
    ptr += sizeof(float) * 4 * m_header.verticesNum;
  }

  if(m_header.flags & HAS_TANGENT)
  {
    m_tangents = (float*)ptr;
    ptr += sizeof(float)*4*m_header.verticesNum;
  }

  m_texcoords   = (float*)ptr; ptr += sizeof(float)*2*m_header.verticesNum;

  m_triVertIndices     = (uint32_t*)ptr; ptr += sizeof(uint32_t)*m_header.indicesNum;
  m_triMaterialIndices = (uint32_t*)ptr; ptr += sizeof(uint32_t)*(m_header.indicesNum / 3);
  
  // #NOTE: enable if use ppc
  //convertLittleBigEndian((unsigned int*)m_positions, m_header.verticesNum*4);
  //if (!(m_header.flags & HAS_NO_NORMALS))
  //  convertLittleBigEndian((unsigned int*)m_normals, m_header.verticesNum*4);
  //if (m_header.flags & HAS_TANGENT)
  //  convertLittleBigEndian((unsigned int*)m_tangents, m_header.verticesNum*4);
  //convertLittleBigEndian((unsigned int*)m_texcoords, m_header.verticesNum*2);
  //convertLittleBigEndian((unsigned int*)m_triVertIndices, m_header.indicesNum);
  //convertLittleBigEndian((unsigned int*)m_triMaterialIndices, (m_header.indicesNum/3));
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

static VSGFOffsets CalcOffsets(int numVert, int numInd, bool a_haveTangents, bool a_haveNormals)
{
  VSGFOffsets res;

  if(a_haveTangents && a_haveNormals)
  {
    res.offsetPos  = sizeof(HydraGeomData::Header);
    res.offsetNorm = res.offsetPos  + numVert * sizeof(float) * 4; // after pos
    res.offsetTang = res.offsetNorm + numVert * sizeof(float) * 4; // after norm
    res.offsetTexc = res.offsetTang + numVert * sizeof(float) * 4; // after tangent
    res.offsetInd  = res.offsetTexc + numVert * sizeof(float) * 2; // after texcoord
    res.offsetMind = res.offsetInd  + numInd * sizeof(int);        // after ind
  }
  else if (!a_haveTangents && a_haveNormals)
  {
    res.offsetPos  = sizeof(HydraGeomData::Header);
    res.offsetNorm = res.offsetPos  + numVert * sizeof(float) * 4; // after pos
    res.offsetTang = res.offsetNorm; // res.offsetNorm + numVert * sizeof(float) * 4; // after norm
    res.offsetTexc = res.offsetTang + numVert * sizeof(float) * 4; // after tangent
    res.offsetInd  = res.offsetTexc + numVert * sizeof(float) * 2; // after texcoord
    res.offsetMind = res.offsetInd  + numInd * sizeof(int);        // after ind
  }
  else
  {
    res.offsetPos  = sizeof(HydraGeomData::Header);
    res.offsetNorm = res.offsetPos;  // res.offsetPos  + numVert * sizeof(float) * 4; // after pos
    res.offsetTang = res.offsetNorm; // res.offsetNorm + numVert * sizeof(float) * 4; // after norm
    res.offsetTexc = res.offsetTang + numVert * sizeof(float) * 4; // after tangent
    res.offsetInd  = res.offsetTexc + numVert * sizeof(float) * 2; // after texcoord
    res.offsetMind = res.offsetInd  + numInd * sizeof(int);        // after ind
  }

  return res;
}

struct HRMeshDriverInput
{
  HRMeshDriverInput() : vertNum(0), triNum(0), pos4f(nullptr), norm4f(nullptr), texcoord2f(nullptr), tan4f(nullptr), indices(nullptr), triMatIndices(nullptr) {}

  int    vertNum;
  int    triNum;
  const float* pos4f;
  const float* norm4f;
  const float* texcoord2f;
  const float* tan4f;
  const int*   indices;
  const int*   triMatIndices;
  const char*  allData;
};


void _hrDebugPrintMesh(const HRMeshDriverInput& a_input, const char* a_fileName)
{
  std::ofstream fout(a_fileName);

  fout << "vertNum = " << a_input.vertNum << std::endl;
  fout << "triNum  = " << a_input.triNum << std::endl;
  fout << "vpos  = [ " << std::endl;

  for (int i = 0; i < a_input.vertNum; i++)
    fout << "  " << a_input.pos4f[i * 4 + 0] << ", " << a_input.pos4f[i * 4 + 1] << ", " << a_input.pos4f[i * 4 + 2] << ", " << a_input.pos4f[i * 4 + 3] << std::endl;

  fout << "]" << std::endl << std::endl;
  fout << "vnorm  = [ " << std::endl;

  for (int i = 0; i < a_input.vertNum; i++)
    fout << "  " << a_input.norm4f[i * 4 + 0] << ", " << a_input.norm4f[i * 4 + 1] << ", " << a_input.norm4f[i * 4 + 2] << ", " << a_input.norm4f[i * 4 + 3] << std::endl;

  fout << "]" << std::endl << std::endl;
  fout << "vtang  = [ " << std::endl;

  for (int i = 0; i < a_input.vertNum; i++)
    fout << "  " << a_input.tan4f[i * 4 + 0] << ", " << a_input.tan4f[i * 4 + 1] << ", " << a_input.tan4f[i * 4 + 2] << ", " << a_input.tan4f[i * 4 + 3] << std::endl;

  fout << "]" << std::endl << std::endl;
  fout << "vtxcoord  = [ " << std::endl;

  for (int i = 0; i < a_input.vertNum; i++)
    fout << "  " << a_input.texcoord2f[i * 2 + 0] << ", " << a_input.texcoord2f[i * 2 + 1] << std::endl;

  fout << "]" << std::endl << std::endl;
  fout << "indices  = [ " << std::endl;

  for (int i = 0; i < a_input.triNum; i++)
    fout << "  " << a_input.indices[i * 3 + 0] << ", " << a_input.indices[i * 3 + 1] << ", " << a_input.indices[i * 3 + 2] << std::endl;

  fout << "]" << std::endl << std::endl;
  fout << "mindices  = [ " << std::endl;

  for (int i = 0; i < a_input.triNum; i++)
    fout << "  " << a_input.triMatIndices[i] << std::endl;

  fout << "]" << std::endl << std::endl;
  fout.close();
}


void DebugPrintVSGF(const char* a_fileNameIn, const char* a_fileNameOut)
{
  HydraGeomData data;
  data.read(a_fileNameIn);

  if(data.getIndicesNumber() == 0)
  {
    std::cout << "[cmesh::DebugPrintVSGF]: cant open file " << a_fileNameIn << std::endl;
    return;
  }

  HRMeshDriverInput mi;

  mi.triNum        = data.getIndicesNumber() / 3;
  mi.vertNum       = data.getVerticesNumber();
  mi.indices       = (int*)data.getTriangleVertexIndicesArray();
  mi.triMatIndices = (int*)data.getTriangleMaterialIndicesArray();

  mi.pos4f      = (float*)data.getVertexPositionsFloat4Array();
  mi.norm4f     = (float*)data.getVertexNormalsFloat4Array();
  mi.tan4f      = (float*)data.getVertexTangentsFloat4Array();
  mi.texcoord2f = (float*)data.getVertexTexcoordFloat2Array();

  _hrDebugPrintMesh(mi, a_fileNameOut);
}
