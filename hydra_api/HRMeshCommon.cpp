#include "HydraInternal.h"
#include "HydraObjectManager.h"

#include "HydraVSGFExport.h"

#include <sstream>
#include <unordered_set>
#include <algorithm>

extern HRObjectManager g_objManager;

struct MeshVSGF : public IHRMesh
{
  MeshVSGF(size_t a_sz, size_t a_chId) : m_sizeInBytes(a_sz), m_chunkId(a_chId), m_vertNum(0), m_indNum(0) { m_matDrawList.reserve(100); }

  uint64_t chunkId() const { return uint64_t(m_chunkId); }
  uint64_t offset(const wchar_t* a_arrayname) const;

  uint64_t vertNum() const { return m_vertNum; }
  uint64_t indNum()  const { return m_indNum;  }

  size_t   DataSizeInBytes() const { return m_sizeInBytes; }
  
  const std::vector<HRBatchInfo>& MList() const { return m_matDrawList; }

  size_t   m_sizeInBytes;
  size_t   m_chunkId;

  size_t   m_vertNum;
  size_t   m_indNum;

  std::vector<HRBatchInfo> m_matDrawList;
};


uint64_t MeshVSGF::offset(const wchar_t* a_arrayname) const 
{ 
  uint64_t offset1 = sizeof(HydraGeomData::Header);
  uint64_t offset2 = offset1 + m_vertNum*sizeof(float)*4; // after pos
  uint64_t offset3 = offset2 + m_vertNum*sizeof(float)*4; // after norm
  uint64_t offset4 = offset3 + m_vertNum*sizeof(float)*4; // after tangent
  uint64_t offset5 = offset4 + m_vertNum*sizeof(float)*2; // after texcoord
  uint64_t offset6 = offset5 + m_indNum*sizeof(int);      // after ind


  if (std::wstring(a_arrayname) == L"pos")
  {
    return offset1;
  }
  else if (std::wstring(a_arrayname) == L"norm")
  {
    return offset2;
  }
  else if (std::wstring(a_arrayname) == L"tan")
  {
    return offset3;
  }
  else if (std::wstring(a_arrayname) == L"texc")
  {
    return offset4;
  }
  else if (std::wstring(a_arrayname) == L"ind")
  {
    return offset5;
  }
  else if (std::wstring(a_arrayname) == L"mind")
  {
    return offset6;
  }
  else
  {
    return uint64_t (-1);
  }

}

std::vector<HRBatchInfo> FormMatDrawListRLE(const std::vector<uint32_t>& matIndices)
{
  std::vector<HRBatchInfo> matDrawList;

  if (matIndices.size() == 0)
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
      HRBatchInfo elem;
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

  return matDrawList;
}

std::shared_ptr<IHRMesh> HydraFactoryCommon::CreateVSGFFromSimpleInputMesh(HRMesh* pSysObj)
{
  //if (pSysObj->id == 1)
    //return nullptr;

  const auto& input = pSysObj->m_input;

  if (input.matIndices.size() == 0)
  {
    HrError(L"CreateVSGFFromSimpleInputMesh: input.matIndices.size() == 0");
    return nullptr;
  }

  // put this all to linear memory chunk
  //
  HydraGeomData data;

  const size_t totalVertNumber     = input.verticesPos.size() / 4;
  const size_t totalMeshTriIndices = input.triIndices.size();


  /* sorting triIndices by matIndices */
  
  const uint32_t* triIndices = &input.triIndices[0];
  const uint32_t* matIndices = &input.matIndices[0];

  std::vector<uint32_t> sortedTriIndices;
  std::vector<uint32_t> sortedMatIndices;

  if (g_objManager.m_sortTriIndices)
  {
    std::vector<std::size_t> tmp_vec;
    for (std::size_t i = 0; i != input.matIndices.size(); ++i) { tmp_vec.push_back(i); }

    std::sort(
      tmp_vec.begin(), tmp_vec.end(),
      [&](std::size_t a, std::size_t b) { return input.matIndices[a] < input.matIndices[b]; });

    sortedTriIndices.resize(input.triIndices.size());
    sortedMatIndices.resize(input.matIndices.size());

    for (int i = 0; i < tmp_vec.size(); ++i)
    {
      sortedTriIndices.at(i * 3 + 0) = input.triIndices.at(tmp_vec.at(i) * 3 + 0);
      sortedTriIndices.at(i * 3 + 1) = input.triIndices.at(tmp_vec.at(i) * 3 + 1);
      sortedTriIndices.at(i * 3 + 2) = input.triIndices.at(tmp_vec.at(i) * 3 + 2);

      sortedMatIndices.at(i) = input.matIndices.at(tmp_vec.at(i));
    }
    triIndices = &sortedTriIndices[0];
    matIndices = &sortedMatIndices[0];
  }

  data.setData(uint32_t(totalVertNumber), &input.verticesPos[0], &input.verticesNorm[0], &input.verticesTangent[0], &input.verticesTexCoord[0],
               uint32_t(totalMeshTriIndices), triIndices, matIndices);

  const size_t totalByteSize = data.sizeInBytes();

  const size_t chunkId = g_objManager.scnlib().m_vbCache.AllocChunk(totalByteSize, pSysObj->id);
  auto& chunk          = g_objManager.scnlib().m_vbCache.chunk_at(chunkId);
  chunk.type           = CHUNK_TYPE_VSGF;
   
  if (chunkId == size_t(-1))
  {
    HrError(L"HydraFactoryCommon::CreateVSGFFromSimpleInputMesh, out of memory, failed to allocate large chunk");
    return nullptr;
  }

  char* memory = (char*)chunk.GetMemoryNow();
  if (memory == nullptr)
  {
    HrError(L"HydraFactoryCommon::CreateVSGFFromSimpleInputMesh, out of memory unknown error");
    return nullptr;
  }

  data.writeToMemory(memory);

  std::shared_ptr<MeshVSGF> pMeshImpl = std::make_shared<MeshVSGF>(totalByteSize, chunkId);

  pMeshImpl->m_vertNum     = totalVertNumber;
  pMeshImpl->m_indNum      = totalMeshTriIndices;

  if (g_objManager.m_sortTriIndices)
  {
    pMeshImpl->m_matDrawList = FormMatDrawListRLE(sortedMatIndices);
  }
  else
  {
    pMeshImpl->m_matDrawList = FormMatDrawListRLE(input.matIndices);
  }

  return pMeshImpl;
}

std::shared_ptr<IHRMesh> HydraFactoryCommon::CreateVSGFFromFile(HRMesh* pSysObj, const std::wstring& a_fileName)
{
  // put this all to linear memory chunk
  //
  HydraGeomData data;
  data.read(a_fileName);

  if (data.getVerticesNumber() == 0)
    return nullptr;

  std::shared_ptr<MeshVSGF> pMeshImpl = std::make_shared<MeshVSGF>(data.sizeInBytes(), -1);

  std::vector<uint32_t> mindices(data.getTriangleMaterialIndicesArray(), data.getTriangleMaterialIndicesArray() + data.getIndicesNumber() / 3);

  pMeshImpl->m_vertNum     = data.getVerticesNumber();
  pMeshImpl->m_indNum      = data.getIndicesNumber();
  pMeshImpl->m_matDrawList = FormMatDrawListRLE(mindices);

  return pMeshImpl;
}
