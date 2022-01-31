#include "HydraInternal.h"
#include "HydraObjectManager.h"

#include "HydraVSGFExport.h"
#include "HydraVSGFCompress.h"
#include "HydraXMLHelpers.h"


#include <sstream>
#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>

extern HRObjectManager g_objManager;

#ifdef WIN32
#undef min
#undef max
#endif



std::vector< LiteMath::float4> getVerticesFromBBox(const BBox &a_bbox)
{
  std::vector< LiteMath::float4> verts;

  verts.emplace_back(LiteMath::float4(a_bbox.x_min, a_bbox.y_min, a_bbox.z_min, 1.0f));
  verts.emplace_back(LiteMath::float4(a_bbox.x_min, a_bbox.y_min, a_bbox.z_max, 1.0f));
  verts.emplace_back(LiteMath::float4(a_bbox.x_min, a_bbox.y_max, a_bbox.z_max, 1.0f));
  verts.emplace_back(LiteMath::float4(a_bbox.x_min, a_bbox.y_max, a_bbox.z_min, 1.0f));

  verts.emplace_back(LiteMath::float4(a_bbox.x_max, a_bbox.y_min, a_bbox.z_min, 1.0f));
  verts.emplace_back(LiteMath::float4(a_bbox.x_max, a_bbox.y_min, a_bbox.z_max, 1.0f));
  verts.emplace_back(LiteMath::float4(a_bbox.x_max, a_bbox.y_max, a_bbox.z_max, 1.0f));
  verts.emplace_back(LiteMath::float4(a_bbox.x_max, a_bbox.y_max, a_bbox.z_min, 1.0f));

  return verts;
}

BBox createBBoxFromFloat4V(const std::vector<LiteMath::float4> &a_verts)
{
  BBox box;

  box.x_min = a_verts[0].x;
  box.x_max = a_verts[0].x;
  box.y_min = a_verts[0].y;
  box.y_max = a_verts[0].y;
  box.z_min = a_verts[0].z;
  box.z_max = a_verts[0].z;

  for (int i = 1; i < a_verts.size(); ++i)
  {
    float x = a_verts[i].x;
    float y = a_verts[i].y;
    float z = a_verts[i].z;

    box.x_min = x < box.x_min ? x : box.x_min;
    box.x_max = x > box.x_max ? x : box.x_max;

    box.y_min = y < box.y_min ? y : box.y_min;
    box.y_max = y > box.y_max ? y : box.y_max;

    box.z_min = z < box.z_min ? z : box.z_min;
    box.z_max = z > box.z_max ? z : box.z_max;
  }

  return box;
}

BBox createBBoxFromFloatV(const std::vector<float> &a_verts, int stride)
{
  BBox box;

  box.x_min = a_verts[0];
  box.x_max = a_verts[0];
  box.y_min = a_verts[1];
  box.y_max = a_verts[1];
  box.z_min = a_verts[2];
  box.z_max = a_verts[2];

  for (int i = 4; i < a_verts.size(); i += stride)
  {
    float x = a_verts[i + 0];
    float y = a_verts[i + 1];
    float z = a_verts[i + 2];

    box.x_min = x < box.x_min ? x : box.x_min;
    box.x_max = x > box.x_max ? x : box.x_max;

    box.y_min = y < box.y_min ? y : box.y_min;
    box.y_max = y > box.y_max ? y : box.y_max;

    box.z_min = z < box.z_min ? z : box.z_min;
    box.z_max = z > box.z_max ? z : box.z_max;
  }

  return box;
}

BBox HRUtils::transformBBox(const BBox &a_bbox, const float m[16])
{
  LiteMath::float4x4 mat(m);

  return ::transformBBox(a_bbox, mat);
}

BBox transformBBox(const BBox &a_bbox, const LiteMath::float4x4 &m)
{
  auto verts = getVerticesFromBBox(a_bbox);

  for(auto& v : verts)
    v = m*v;

  return createBBoxFromFloat4V(verts);
}

BBox mergeBBoxes(const BBox &A, const BBox &B)
{
  BBox C;
  
  C.x_min = A.x_min < B.x_min ? A.x_min : B.x_min;
  C.x_max = A.x_max > B.x_max ? A.x_max : B.x_max;

  C.y_min = A.y_min < B.y_min ? A.y_min : B.y_min;
  C.y_max = A.y_max > B.y_max ? A.y_max : B.y_max;

  C.z_min = A.z_min < B.z_min ? A.z_min : B.z_min;
  C.z_max = A.z_max > B.z_max ? A.z_max : B.z_max;

  return C;
}

std::string  ws2s(const std::wstring& s);

struct MeshVSGF : public IHRMesh
{
  MeshVSGF(){}
  MeshVSGF(size_t a_sz, size_t a_chId) : m_sizeInBytes(a_sz), m_chunkId(a_chId), m_vertNum(0),
                                         m_indNum(0), m_bbox(BBox()), m_hasTangentOnLoad(true), m_hasNormalsOnLoad(true) { m_matDrawList.reserve(100); }

  uint64_t chunkId() const override { return uint64_t(m_chunkId); }
  uint64_t offset (const wchar_t* a_arrayname) const override;

  uint64_t vertNum() const override { return m_vertNum; }
  uint64_t indNum()  const override { return m_indNum;  }

  size_t DataSizeInBytes() const override { return m_sizeInBytes; }
  size_t EstimatedDataSizeInBytes() const override
  {
    return m_vertNum*(sizeof(float)*4*3 + sizeof(float)*2) +
           m_indNum*sizeof(int) + (m_indNum*sizeof(int))/3 +
           sizeof(HydraGeomData::Header) + 1024;
  }
  
  const void* GetData() const override
  {
    auto chunk = g_objManager.scnData.m_vbCache.chunk_at(m_chunkId);

    if(chunk.InMemory())
    {
      return chunk.GetMemoryNow();
    }
    else
    {
      const std::wstring location = ChunkName(chunk);
      std::ifstream fin;
      hr_ifstream_open(fin, location.c_str());
      g_objManager.m_tempBuffer.resize(DataSizeInBytes()/sizeof(int) + 10); // #TODO: add lock flags for global buffer ... ?? may be ...
      fin.read((char*)g_objManager.m_tempBuffer.data(), DataSizeInBytes());
      fin.close();
      return g_objManager.m_tempBuffer.data();
    }
  }

  BBox getBBox() const override { return m_bbox;}
  
  const std::vector<HRBatchInfo>& MList() const override { return m_matDrawList; }
  std::vector<HRBatchInfo>&       MList() override { return m_matDrawList; }
  
  std::string MaterialNamesList() override
  {
    auto batchList = MList();
    
    std::stringstream strOut;
    for(size_t i=0;i<batchList.size();i++)
    {
      int matId = batchList[i].matId;
    
      if(matId < 0)
      {
        strOut << "undefined;";
        continue;
      }
    
      HRMaterialRef matRef;
      matRef.id = matId;
    
      auto* pMaterial = g_objManager.PtrById(matRef);
      if(pMaterial == nullptr)
      {
        strOut << "undefined;";
        continue;
      }
    
      std::string matName = ws2s(pMaterial->name);
      strOut << matName.c_str() << ";";
    }
    return strOut.str();
  }
  
  const std::unordered_map<std::wstring, std::tuple<std::wstring, size_t, size_t, int> >& GetOffsAndSizeForAttrs() const override { return m_custAttrOffsAndSize; }

  size_t   m_sizeInBytes;
  size_t   m_chunkId;

  size_t   m_vertNum;
  size_t   m_indNum;

  std::vector<HRBatchInfo> m_matDrawList;
  BBox m_bbox;

  bool m_hasTangentOnLoad;
  bool m_hasNormalsOnLoad;

  std::unordered_map<std::wstring, std::tuple<std::wstring, size_t, size_t, int> > m_custAttrOffsAndSize;
};


uint64_t MeshVSGF::offset(const wchar_t* a_arrayname) const 
{

  const VSGFOffsets offsets = CalcOffsets(m_vertNum, m_indNum, m_hasTangentOnLoad, m_hasNormalsOnLoad);
  
  if (std::wstring(a_arrayname) == L"pos")
  {
    return offsets.offsetPos;
  }
  else if (std::wstring(a_arrayname) == L"norm")
  {
    return offsets.offsetNorm;
  }
  else if (std::wstring(a_arrayname) == L"tan")
  {
    return offsets.offsetTang;
  }
  else if (std::wstring(a_arrayname) == L"texc")
  {
    return offsets.offsetTexc;
  }
  else if (std::wstring(a_arrayname) == L"ind")
  {
    return offsets.offsetInd;
  }
  else if (std::wstring(a_arrayname) == L"mind")
  {
    return offsets.offsetMind;
  }
  else
  {
    return uint64_t (-1);
  }

}

std::vector<HRBatchInfo> FormMatDrawListRLE(const std::vector<uint32_t>& matIndices);

void HR_LoadVSGFCompressedBothHeaders(std::ifstream& fin,
                                      std::vector<HRBatchInfo>& a_outBatchList, HydraGeomData::Header& h1, HydraHeaderC& h2);

struct MeshVSGFProxy : public MeshVSGF
{
  MeshVSGFProxy() : m_loaded(false) {}
  MeshVSGFProxy(const wchar_t* a_vsgfPath)
  {
    m_loaded = ReadVSGFHeader(a_vsgfPath);
  }
  
  std::string MaterialNamesList() override { return matNames; }
  std::string matNames;

  bool m_loaded;
  
protected:

  bool ReadVSGFHeader(const wchar_t* a_fileName)
  {
    std::ifstream fin;
    hr_ifstream_open(fin, a_fileName);
    
    if(!fin.is_open())
    {
      HrPrint(HR_SEVERITY_ERROR, L"MeshVSGFProxy::ReadVSGFHeader, can't open file: ", a_fileName);
      return false;
    }
    
    std::wstring fileName(a_fileName);
    std::wstring ext = str_tail(fileName, 6);

    HydraGeomData::Header header{};

    if(ext == L".vsgfc")
    {
      HydraHeaderC h2;
      HR_LoadVSGFCompressedBothHeaders(fin,
                                       m_matDrawList, header, h2);
      m_vertNum     = header.verticesNum;
      m_indNum      = header.indicesNum;
      m_sizeInBytes = header.fileSizeInBytes;
      m_chunkId     = size_t(-1);

      //#TODO: read file names list;
      //
      matNames.resize(h2.customDataSize);
      fin.seekg(h2.customDataOffset, std::ios_base::beg);
      fin.read ((char*)matNames.c_str(), h2.customDataSize);
      fin.close();
      
      m_bbox.x_min = h2.boxMin[0];
      m_bbox.y_min = h2.boxMin[1];
      m_bbox.z_min = h2.boxMin[2];
  
      m_bbox.x_max = h2.boxMax[0];
      m_bbox.y_max = h2.boxMax[1];
      m_bbox.z_max = h2.boxMax[2];
    }
    else
    {
      fin.read((char*)&header, sizeof(HydraGeomData::Header));

      m_vertNum     = header.verticesNum;
      m_indNum      = header.indicesNum;
      m_sizeInBytes = header.fileSizeInBytes;
      m_chunkId     = size_t(-1);

      const bool hasNormalsOnLoad = ((header.flags & HydraGeomData::HAS_NO_NORMALS) == 0);
      const bool hasTangentOnLoad = ((header.flags & HydraGeomData::HAS_TANGENT)    != 0);
      const auto allOffsets       = CalcOffsets(header.verticesNum, header.indicesNum, hasTangentOnLoad, hasNormalsOnLoad);
      const auto matIndOffset     = allOffsets.offsetMind;

      auto cur_pos = fin.tellg();
      fin.seekg(std::ios_base::end, std::ios_base::beg);
      auto pos = fin.tellg();
      bool hasExtraData = pos > m_sizeInBytes;
      fin.seekg(cur_pos, std::ios_base::beg);

      if(pos != m_sizeInBytes) // ext == L".vsgf2"
      {
        HydraHeaderC h2{};
        fin.seekg(header.fileSizeInBytes);
        fin.read((char*)&h2, sizeof(HydraHeaderC));

        m_matDrawList.resize(h2.batchListArraySize);
        fin.read((char*)m_matDrawList.data(), m_matDrawList.size()*sizeof(HRBatchInfo));

        matNames.resize(h2.customDataSize);
        fin.seekg(h2.customDataOffset, std::ios_base::beg);
        fin.read ((char*)matNames.c_str(), h2.customDataSize);
        fin.close();

        m_bbox.x_min = h2.boxMin[0];
        m_bbox.y_min = h2.boxMin[1];
        m_bbox.z_min = h2.boxMin[2];

        m_bbox.x_max = h2.boxMax[0];
        m_bbox.y_max = h2.boxMax[1];
        m_bbox.z_max = h2.boxMax[2];
      }
      else
      {
        std::vector<uint32_t> matIndixes(m_indNum / 3);
        fin.seekg(matIndOffset);
        fin.read((char *) matIndixes.data(), matIndixes.size() * sizeof(int));
        fin.close();

        m_matDrawList = FormMatDrawListRLE(matIndixes);
      }
      //m_bbox;        // don't evaluate this for Proxy Object due to this is long operation
    }

    m_hasNormalsOnLoad = ((header.flags & HydraGeomData::HAS_NO_NORMALS) == 0);
    m_hasTangentOnLoad = ((header.flags & HydraGeomData::HAS_TANGENT)    != 0);
    
    return true;
  }

  const void* GetData() const override  // yes, don;t try to get data of MeshVSGFProxy. Find another option.
  {
    return nullptr;
  }

};

std::vector<HRBatchInfo> FormMatDrawListRLE(const std::vector<uint32_t>& matIndices)
{
  std::vector<HRBatchInfo> matDrawList;

  if (matIndices.empty())
    return matDrawList;

  matDrawList.reserve(matIndices.size() / 4);

  uint32_t tBegin = 0;
  int32_t  tMatId = matIndices[0];

  for (size_t i = 0; i < matIndices.size(); i++)
  {
    uint32_t mid = matIndices[i];

    if (matIndices[tBegin] != mid || (i == matIndices.size() - 1))
    {
      // append current tri sequence withe the same material id
      //
      HRBatchInfo elem = {0,0,0};
      elem.matId    = tMatId;
      elem.triBegin = tBegin;
      elem.triEnd   = uint32_t(i);

      if (i == matIndices.size() - 1)
        elem.triEnd = uint32_t(matIndices.size());

      matDrawList.push_back(elem);

      // save begin and material id for next  tri sequence withe the same material id
      //
      tBegin = elem.triEnd;
      tMatId = mid;
    }
  }

  ////////////////////////////////////////////////////////////////////////////////// fix for last tringle
  size_t last    = matIndices.size() - 1;
  size_t preLast = matIndices.size() - 2;

  if(matIndices.size() == 1 && matDrawList.size() == 0)
  {
    HRBatchInfo elem = {0,0,0};
    elem.matId    = matIndices[0];
    elem.triBegin = 0;
    elem.triEnd   = 1;
    matDrawList.push_back(elem);
  }
  else if (matIndices.size() > 1 && matIndices[last] != matIndices[preLast])
  {
    HRBatchInfo elem = {0,0,0};
    elem.matId    = matIndices[last];
    elem.triBegin = last;
    elem.triEnd   = matIndices.size();
    matDrawList.push_back(elem);
  }
  ////////////////////////////////////////////////////////////////////////////////// fix for last tringle

  return matDrawList;
}

std::shared_ptr<IHRMesh> HydraFactoryCommon::CreateVSGFFromSimpleInputMesh(HRMesh* pSysObj, bool a_saveCompressed)
{
  const auto& input = pSysObj->m_input;

  if (input.matIndices.empty())
  {
    HrError(L"CreateVSGFFromSimpleInputMesh: input.matIndices.size() == 0");
    return nullptr;
  }

  // put this all to linear memory chunk
  //
  HydraGeomData data;

  const size_t totalVertNumber     = input.VerticesNum();
  const size_t totalMeshTriIndices = input.IndicesNum();

  // sorting triIndices by matIndices
  
  const uint32_t* triIndices = input.indices.data();
  const uint32_t* matIndices = input.matIndices.data();

  std::vector<uint32_t> sortedTriIndices;
  std::vector<uint32_t> sortedMatIndices;

  if (g_objManager.m_sortTriIndices)
  {
    std::vector<std::size_t> tmp_vec;
    for (std::size_t i = 0; i != input.matIndices.size(); ++i) { tmp_vec.push_back(i); }

    std::sort(
      tmp_vec.begin(), tmp_vec.end(),
      [&](std::size_t a, std::size_t b) { return input.matIndices[a] < input.matIndices[b]; });

    sortedTriIndices.resize(input.indices.size());
    sortedMatIndices.resize(input.matIndices.size());

    for (int i = 0; i < tmp_vec.size(); ++i)
    {
      sortedTriIndices.at(i * 3 + 0) = input.indices.at(tmp_vec.at(i) * 3 + 0);
      sortedTriIndices.at(i * 3 + 1) = input.indices.at(tmp_vec.at(i) * 3 + 1);
      sortedTriIndices.at(i * 3 + 2) = input.indices.at(tmp_vec.at(i) * 3 + 2);

      sortedMatIndices.at(i) = input.matIndices.at(tmp_vec.at(i));
    }
    triIndices = &sortedTriIndices[0];
    matIndices = &sortedMatIndices[0];
  }


  // (1) common mesh attributes
  //
  data.setData(uint32_t(totalVertNumber), input.vPos4f.data(), input.vNorm4f.data(), input.vTang4f.data(), input.vTexCoord2f.data(),
               uint32_t(totalMeshTriIndices), triIndices, matIndices);

  const size_t totalByteSizeCommon = data.sizeInBytes();

  // (2) custom mesh attributes
  //
  size_t totalByteSizeCustom = 0;

  for (auto& arr : pSysObj->m_input.customArrays)
  {
    if (!arr.fdata.empty())
      totalByteSizeCustom += arr.fdata.size() * sizeof(float);
    else
      totalByteSizeCustom += arr.idata.size() * sizeof(int);
  }

  const size_t totalByteSize = totalByteSizeCommon + totalByteSizeCustom;
  const size_t chunkId       = g_objManager.scnData.m_vbCache.AllocChunk(totalByteSize, pSysObj->id);
   
  if (chunkId == size_t(-1))
  {
    HrError(L"HydraFactoryCommon::CreateVSGFFromSimpleInputMesh, out of memory, failed to allocate large chunk");
    return nullptr;
  }

  auto& chunk          = g_objManager.scnData.m_vbCache.chunk_at(chunkId);
  chunk.type           = CHUNK_TYPE_VSGF;
  chunk.saveCompressed = a_saveCompressed;  // we need to remember this info inside chunk to enable compressed mesh save
  chunk.sysObjectId    = pSysObj->id;       // and plug xml description inside a file also;
  
  char* memory = (char*)chunk.GetMemoryNow();

  if (memory == nullptr)
  {
    HrError(L"HydraFactoryCommon::CreateVSGFFromSimpleInputMesh, out of memory, unknown error");
    return nullptr;
  }
  
  std::shared_ptr<MeshVSGF> pMeshImpl = std::make_shared<MeshVSGF>(totalByteSize, chunkId);

  // (1) common mesh attributes
  //
  data.writeToMemory(memory);

  // (2) custom mesh attributes
  //
  size_t currOffset = totalByteSizeCommon;
  for (const auto& arr : pSysObj->m_input.customArrays)
  {
    std::wstring type = L"";

    size_t currSize = 0;
    if (!arr.fdata.empty())
    {
      currSize = arr.fdata.size() * sizeof(float);
      memcpy(memory + currOffset, &arr.fdata[0], currSize);

      if (arr.depth == 4)
        type = L"array4f";
      else if(arr.depth == 2)
        type = L"array2f";
      else
        type = L"array1f";
    }
    else
    {
      currSize = arr.idata.size() * sizeof(int);
      memcpy(memory + currOffset, &arr.idata[0], currSize);

      if (arr.depth == 4)
        type = L"array4i";
      else if (arr.depth == 2)
        type = L"array2i";
      else
        type = L"array1i";
    }

    pMeshImpl->m_custAttrOffsAndSize[arr.name] = std::tuple<std::wstring, size_t, size_t, int>(type, currOffset, currSize, arr.apply);
    currOffset += currSize;
  }

  pMeshImpl->m_vertNum = totalVertNumber;
  pMeshImpl->m_indNum  = totalMeshTriIndices;

  //compute bbox
  if (g_objManager.m_computeBBoxes)
  {
    BBox box;
    box.x_min = std::numeric_limits<float>::max();
    box.x_max = std::numeric_limits<float>::lowest();
    box.y_min = std::numeric_limits<float>::max();
    box.y_max = std::numeric_limits<float>::lowest();
    box.z_min = std::numeric_limits<float>::max();
    box.z_max = std::numeric_limits<float>::lowest();
    for (int i = 0; i < input.vPos4f.size(); i += 4)
    {
      float x = input.vPos4f[i + 0];
      float y = input.vPos4f[i + 1];
      float z = input.vPos4f[i + 2];

      box.x_min = x < box.x_min ? x : box.x_min;
      box.x_max = x > box.x_max ? x : box.x_max;

      box.y_min = y < box.y_min ? y : box.y_min;
      box.y_max = y > box.y_max ? y : box.y_max;

      box.z_min = z < box.z_min ? z : box.z_min;
      box.z_max = z > box.z_max ? z : box.z_max;
    }
    pMeshImpl->m_bbox = box;
  }


  if (g_objManager.m_sortTriIndices)
    pMeshImpl->m_matDrawList = FormMatDrawListRLE(sortedMatIndices);
  else
    pMeshImpl->m_matDrawList = FormMatDrawListRLE(input.matIndices);

  return pMeshImpl;
}

std::shared_ptr<IHRMesh> HydraFactoryCommon::CreateVSGFFromFile(HRMesh* pSysObj, const std::wstring& a_fileName, pugi::xml_node a_node)
{
  const int64_t totalByteSize = a_node.attribute(L"bytesize").as_llong();

  if(totalByteSize <= 0)
    return nullptr;

  std::shared_ptr<MeshVSGF> pMeshImpl = std::make_shared<MeshVSGF>(totalByteSize, -1);
  pMeshImpl->m_vertNum     = a_node.attribute(L"vertNum").as_int();
  pMeshImpl->m_indNum      = a_node.attribute(L"triNum").as_int()*3;
  pMeshImpl->m_chunkId     = ChunkIdFromFileName(a_fileName.c_str());
  
  pugi::xml_node mindicesNode = a_node.child(L"matindices");
  if(mindicesNode == nullptr)
    mindicesNode = a_node.child(L"mindices");

  const int64_t moffset = mindicesNode.attribute(L"offset").as_llong();
  const int64_t msize   = mindicesNode.attribute(L"bytesize").as_llong();

  std::vector<uint32_t> mindices(a_node.attribute(L"triNum").as_int());

  size_t totalChunks = g_objManager.scnData.m_vbCache.size();
  
  ChunkPointer chunk;
  if(g_objManager.m_attachMode && pMeshImpl->m_chunkId >= 0 && pMeshImpl->m_chunkId < totalChunks)
    chunk = g_objManager.scnData.m_vbCache.chunk_at(pMeshImpl->m_chunkId);
  
  if(chunk.InMemory())
  {
    const char* data = (const char*)chunk.GetMemoryNow();
    memcpy(mindices.data(), data + moffset, msize);
  }
  else
  {
  #ifdef WIN32
    std::ifstream fin(a_fileName.c_str(), std::ios::binary);
  #else
    std::string fileIn(a_fileName.begin(), a_fileName.end());
    std::ifstream fin(fileIn.c_str(), std::ios::binary);
  #endif
    fin.seekg(moffset);
    fin.read((char*)mindices.data(), msize);
    fin.close();
  }
  
  pMeshImpl->m_matDrawList = FormMatDrawListRLE(mindices);

  BBox bbox;
  HydraXMLHelpers::ReadBBox(a_node, bbox);
  pMeshImpl->m_bbox = bbox;

  return pMeshImpl;
}

std::shared_ptr<IHRMesh> HydraFactoryCommon::CreateVSGFProxy(const wchar_t* a_fileName)
{
  std::shared_ptr<MeshVSGFProxy> pImpl = std::make_shared<MeshVSGFProxy>(a_fileName);
  if(!pImpl->m_loaded)
    return nullptr;
  return pImpl;
}