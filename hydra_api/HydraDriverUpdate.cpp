#include "HydraObjectManager.h"
#include <unordered_set>
#include <map>

#include <fstream>

#pragma warning(disable:4996)

#if defined(WIN32)
#include "FreeImage.h"
#pragma comment(lib, "FreeImage.lib")
#else
#include <FreeImage.h>
#include <algorithm>

#endif

#include "HydraVSGFExport.h"

extern HRObjectManager g_objManager;

struct ChangeList
{
  ChangeList() {}
  ChangeList(ChangeList&& a_list) : meshUsed(std::move(a_list.meshUsed)), matUsed(std::move(a_list.matUsed)), 
                                    lightUsed(std::move(a_list.lightUsed)), texturesUsed(std::move(a_list.texturesUsed)),
                                    drawSeq(std::move(a_list.drawSeq))
  {
    
  }

  ChangeList& operator=(ChangeList&& a_list)
  {
    meshUsed         = std::move(a_list.meshUsed);
    matUsed          = std::move(a_list.matUsed);
    lightUsed        = std::move(a_list.lightUsed);
    texturesUsed     = std::move(a_list.texturesUsed);
    drawSeq          = std::move(a_list.drawSeq);
    return *this;
  }

  std::unordered_set<int32_t> meshUsed;
  std::unordered_set<int32_t> matUsed;
  std::unordered_set<int32_t> lightUsed;
  std::unordered_set<int32_t> texturesUsed;

  struct InstancesInfo
  {
    std::vector<float>    matrices;
    std::vector<int32_t>  linstid;
  };

  std::unordered_map<int32_t, InstancesInfo > drawSeq;

};


void ScanXmlNodeRecursiveAndAppendTexture(pugi::xml_node a_node, std::unordered_set<int32_t>& a_outSet)
{
  if (std::wstring(a_node.name()) == L"texture")
  {
    int32_t id = a_node.attribute(L"id").as_int();
    a_outSet.insert(id);
  }
  else
  {
    for (pugi::xml_node child = a_node.first_child(); child != nullptr; child = child.next_sibling())
      ScanXmlNodeRecursiveAndAppendTexture(child, a_outSet);
  }
}

void AddUsedMaterialChildrenRecursive(ChangeList& objects, int32_t matId)
{
	if (matId >= g_objManager.scnData.materials.size())
		return;

  HRMaterial& mat = g_objManager.scnData.materials[matId];
  auto matNode = mat.xml_node_immediate();
  auto matType = std::wstring(matNode.attribute(L"type").as_string());

  if (matType.compare(std::wstring(L"hydra_blend")) == 0)
  {
    auto subMatId1 = matNode.attribute(L"node_top").as_int();
    auto subMatId2 = matNode.attribute(L"node_bottom").as_int();

    objects.matUsed.insert(subMatId1);
    objects.matUsed.insert(subMatId2);

    AddUsedMaterialChildrenRecursive(objects, subMatId1);
    AddUsedMaterialChildrenRecursive(objects, subMatId2);
  }
}

void FindNewObjects(ChangeList& objects, HRSceneInst& scn)
{
  // (1.1) loop through all scene instances to define what meshes used in scene  --> ~ok
  //
  //std::cout << "##FindNewObjects, scnlib.meshUsedByDrv.size() = " << scnlib.meshUsedByDrv.size() << std::endl;

  for (size_t i = 0; i < scn.drawList.size(); i++)
  {
    auto instance = scn.drawList[i];
    if (instance.meshId >= g_objManager.scnData.meshes.size()) //#TODO: ? add log message if need to debug some thiing here
      continue;

    auto& mesh = g_objManager.scnData.meshes[instance.meshId];

    if (scn.meshUsedByDrv.find(instance.meshId) == scn.meshUsedByDrv.end() || mesh.wasChanged)
    {
      objects.meshUsed.insert(instance.meshId);
      mesh.wasChanged = false;
    }
    
    // form draw sequence for each mesh
    //
    auto p = objects.drawSeq.find(instance.meshId);
    if (p == objects.drawSeq.end())
    {
      objects.drawSeq[instance.meshId].matrices = std::vector<float>  (instance.m, instance.m + 16);
      objects.drawSeq[instance.meshId].linstid  = std::vector<int32_t>(&instance.lightInstId, &instance.lightInstId + 1);
    }
    else
    {
      std::vector<float> data(instance.m, instance.m + 16);
      p->second.matrices.insert(p->second.matrices.end(), data.begin(), data.end());
      p->second.linstid.push_back(instance.lightInstId);
    }
  }

  for (size_t i = 0; i < scn.drawListLights.size(); i++)
  {
    auto instance = scn.drawListLights[i];

    auto& light = g_objManager.scnData.lights[instance.lightId];

    if (scn.lightUsedByDrv.find(instance.lightId) == scn.lightUsedByDrv.end() || light.wasChanged)
    {
      objects.lightUsed.insert(instance.lightId);
      light.wasChanged = false;
    }
    
  }

  // (1.2) loop through needed meshed to define what material used in scene      --> ?
  //
  if (g_objManager.scnData.meshes.size() > 0)
  {
    for (auto p = objects.meshUsed.begin(); p != objects.meshUsed.end(); ++p)
    {
      size_t meshId = (*p);
      if (meshId >= g_objManager.scnData.meshes.size()) //#TODO: ? add log message if need to debug some thiing here
        continue;

      HRMesh& mesh = g_objManager.scnData.meshes[meshId];
      auto pImpl = mesh.pImpl;

      if (pImpl != nullptr)
      {
        auto mlist = pImpl->MList();
        for (auto q = mlist.begin(); q != mlist.end(); ++q)
        {
          HRBatchInfo trio = (*q);
          objects.matUsed.insert(trio.matId);

          AddUsedMaterialChildrenRecursive(objects, trio.matId);
        }
      }
    }

  }

  // (1.3) loop through needed materials to define what textures used in scene 
  //
  if (g_objManager.scnData.materials.size() > 0)
  {
    for (auto p = objects.matUsed.begin(); p != objects.matUsed.end(); ++p)
    {
      size_t matId = (*p);
      if (matId >= g_objManager.scnData.materials.size()) //#TODO: ? add log message if need to debug some thiing here
        continue;

      if (matId < g_objManager.scnData.materials.size())
      {
        HRMaterial& mat = g_objManager.scnData.materials[matId];

        // (1.3.1) list all textures for mat, add them to set
        //
        ScanXmlNodeRecursiveAndAppendTexture(mat.xml_node_immediate(), objects.texturesUsed);
      }
      else
        g_objManager.BadMaterialId(int32_t(matId));
    }

    

  }

  // (1.4) loop through all changed lights to define what lights are used in scene --> TEXTURES
  //
  if (g_objManager.scnData.lights.size() > 0)
  {
    for (auto p = objects.lightUsed.begin(); p != objects.lightUsed.end(); ++p)
    {
      size_t lightId = (*p);
      if (lightId >= g_objManager.scnData.lights.size()) //#TODO: ? add log message if need to debug some thiing here
        continue;

      if (lightId < g_objManager.scnData.lights.size())
      {
        HRLight& light = g_objManager.scnData.lights[lightId];
        ScanXmlNodeRecursiveAndAppendTexture(light.xml_node_immediate(), objects.texturesUsed);
      }
    }
  }

}


void InsertChangedIds(std::unordered_set<int32_t>& a_set, const std::unordered_set<int32_t>& a_usedByDRV, pugi::xml_node a_node, const wchar_t* a_childName)
{
  for (pugi::xml_node node = a_node.first_child(); node != nullptr; node = node.next_sibling())
  {
    if (std::wstring(node.name()) != std::wstring(a_childName))
      continue;

    const int32_t id = node.attribute(L"id").as_int();
    if (a_usedByDRV.find(id) != a_usedByDRV.end())
      a_set.insert(id);
  }
}

void FindOldObjectsThatWeNeedToUpdate(ChangeList& objects, HRSceneInst& scn)
{
  pugi::xml_node meshesChanges   = g_objManager.scnData.m_geometryLibChanges;
  pugi::xml_node lightsChanges   = g_objManager.scnData.m_lightsLibChanges;
  pugi::xml_node matsChanges     = g_objManager.scnData.m_materialsLibChanges;
  pugi::xml_node texturesChanges = g_objManager.scnData.m_texturesLibChanges;

  InsertChangedIds(objects.meshUsed,     scn.meshUsedByDrv,  meshesChanges, L"mesh");
  InsertChangedIds(objects.lightUsed,    scn.lightUsedByDrv, lightsChanges, L"light");
  InsertChangedIds(objects.matUsed,      scn.matUsedByDrv,   matsChanges, L"material");
  InsertChangedIds(objects.texturesUsed, scn.texturesUsedByDrv, texturesChanges, L"texture");
  InsertChangedIds(objects.texturesUsed, scn.texturesUsedByDrv, texturesChanges, L"texture_advanced");

  // now we must add to change list all textures that are presented in the materials of matUsed 
  // but (!!!) does not present in scnlib.texturesUsedByDrv
  //
  for (auto p = objects.matUsed.begin(); p != objects.matUsed.end(); ++p)
  {
    int matId = (*p);

    if (size_t(matId) < g_objManager.scnData.materials.size() && scn.texturesUsedByDrv.find(matId) == scn.texturesUsedByDrv.end())
    {
      HRMaterial& mat = g_objManager.scnData.materials[matId];

      // (1.3.1) list all textures for mat, add them to set
      //
      ScanXmlNodeRecursiveAndAppendTexture(mat.xml_node_immediate(), objects.texturesUsed);
    }
    else
      g_objManager.BadMaterialId(int32_t(matId));
  }
  
}

/// find objects that we have to Update because they depends of some other objects that we already know we have to Update. 
//
void FindObjectsByDependency(ChangeList& objList, HRSceneInst& scn, IHRRenderDriver* a_pDriver)
{
  if (a_pDriver == nullptr)
    return;

  auto dInfo = a_pDriver->DependencyInfo();

  // if (dInfo.materialDependsOfTexture)
  // {
  //   for (auto p = objList.texturesUsed.begin(); p != objList.texturesUsed.end(); ++p)
  //   {
  // 
  //   }
  // }

  if (dInfo.meshDependsOfMaterial)
  {
    auto& depHashMap = g_objManager.scnData.m_materialToMeshDependency;

    for (auto p = objList.matUsed.begin(); p != objList.matUsed.end(); ++p)
      for (auto meshListIter = depHashMap.find(*p); meshListIter != depHashMap.end() && meshListIter->first == (*p); ++meshListIter)
        objList.meshUsed.insert(meshListIter->second);
  }

  //if (dInfo.lightDependsOfMesh)
  //{
  //  for (auto p = objList.meshUsed.begin(); p != objList.meshUsed.end(); ++p)
  //  {
  //
  //  }
  //}

}


/// collect all id's of objects we need to Update
//
ChangeList FindChangedObjects(HRSceneInst& scn, IHRRenderDriver* a_pDriver)
{
  ChangeList objects;

  objects.meshUsed.reserve(1000);
  objects.matUsed.reserve(1000);
  objects.lightUsed.reserve(1000);
  objects.texturesUsed.reserve(1000);

  FindNewObjects(objects, scn);
  FindOldObjectsThatWeNeedToUpdate(objects, scn);
  FindObjectsByDependency(objects, scn, a_pDriver);

  return objects;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int HRUtils_LoadImageFromFileToPairOfFreeImageObjects(const wchar_t* a_filename, FIBITMAP*& dib, FIBITMAP*& converted, FREE_IMAGE_FORMAT* pFif);
bool HRUtils_GetImageDataFromFreeImageObject(FIBITMAP* converted, char* data);

void UpdateImageFromFileOrChunk(int32_t a_id, HRTextureNode& img, IHRRenderDriver* a_pDriver) // #TODO: debug and test this
{
  pugi::xml_node node = img.xml_node_immediate();

  bool delayedLoad = (node.attribute(L"dl").as_int() == 1);

  if (delayedLoad && img.m_loadedFromFile) // load external image from file 
  {
    const wchar_t* filename = node.attribute(L"path").as_string();

    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    FIBITMAP *dib(NULL), *converted(NULL);
    BYTE* bits(NULL);                    // pointer to the image data
    unsigned int width(0), height(0);    //image width and height

    int bytesPerPixel = HRUtils_LoadImageFromFileToPairOfFreeImageObjects(filename, dib, converted, &fif);
    int bitsPerPixel  = bytesPerPixel * 8;

    if (bytesPerPixel == 0)
    {
      HrError(L"UpdateImageFromFileOrChunk: FreeImage failed to load image: ", filename);
      return;
    }

    bits   = FreeImage_GetBits(converted);
    width  = FreeImage_GetWidth(converted);
    height = FreeImage_GetHeight(converted);

    if ((bits == 0) || (width == 0) || (height == 0))
    {
      HrError(L"UpdateImageFromFileOrChunk: FreeImage failed for undefined reason, file : ", filename);
      FreeImage_Unload(converted);
      FreeImage_Unload(dib);
      return;
    }

    size_t sizeInBytes = bytesPerPixel*width*height;

    g_objManager.m_tempBuffer.resize(sizeInBytes / uint64_t(sizeof(int)) + uint64_t(sizeof(int) * 16));
    char* data = (char*)&g_objManager.m_tempBuffer[0];

    HRUtils_GetImageDataFromFreeImageObject(converted, data);

    FreeImage_Unload(converted);
    FreeImage_Unload(dib);

    a_pDriver->UpdateImage(a_id, width, height, bytesPerPixel, data, node);
  }
  else // load chunk
  {
    if (img.pImpl == nullptr)
      return;

    auto w   = img.pImpl->width();
    auto h   = img.pImpl->height();
    auto bpp = img.pImpl->bpp();

    size_t sizeInBytes = size_t(w*h)*size_t(bpp) + size_t(sizeof(int) * 2);

    g_objManager.m_tempBuffer.resize(sizeInBytes / uint64_t(sizeof(int)) + uint64_t(sizeof(int) * 16));
    char* data = (char*)&g_objManager.m_tempBuffer[0];

    const std::wstring path = g_objManager.GetLoc(node);
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
    std::wstring s1(path);
    std::string  s2(s1.begin(), s1.end());
    std::ifstream fin(s2.c_str(), std::ios::binary);
#elif defined WIN32
    std::ifstream fin(path.c_str(), std::ios::binary);
#endif
    if (fin.is_open())
    {
      fin.read(data, sizeInBytes);

      uint64_t dataOffset = node.attribute(L"offset").as_ullong();

      a_pDriver->UpdateImage(a_id, w, h, bpp, data + dataOffset, node);
      fin.close();
    }
  }

}

/////
//
int32_t HR_DriverUpdateTextures(HRSceneInst& scn, ChangeList& objList, IHRRenderDriver* a_pDriver)
{
  if (g_objManager.scnData.textures.size() == 0 || a_pDriver == nullptr)
    return 0;

  a_pDriver->BeginTexturesUpdate();

  int32_t texturesUpdated = 0;

  HRDriverInfo info = a_pDriver->Info();

  for (auto p = objList.texturesUsed.begin(); p != objList.texturesUsed.end(); ++p)
  {
    HRTextureNode& texNode = g_objManager.scnData.textures[(*p)];

    int32_t w     = 0;
    int32_t h     = 0;
    int32_t bpp   = 4;
    char* dataPtr = nullptr;

    if (texNode.pImpl != nullptr)
    {
      w   = texNode.pImpl->width();
      h   = texNode.pImpl->height();
      bpp = texNode.pImpl->bpp();

      uint64_t chunkId = texNode.pImpl->chunkId();
      if (chunkId != uint64_t(-1))
      {
        ChunkPointer chunk = g_objManager.scnData.m_vbCache.chunk_at(chunkId);
        dataPtr = (char*)chunk.GetMemoryNow();
      }
    }

    pugi::xml_node texNodeXML  = texNode.xml_node_immediate();
    uint64_t       dataOffset  = texNodeXML.attribute(L"offset").as_ullong(); //#SAFETY: check dataOffset for too big value ?
    bool           delayedLoad = (texNodeXML.attribute(L"dl").as_int() == 1);

    if (dataPtr == nullptr)
    {

      if (info.supportImageLoadFromExternalFormat && texNode.m_loadedFromFile)
      {
        const wchar_t* path = texNodeXML.attribute(L"path").as_string();
        a_pDriver->UpdateImageFromFile(int32_t(*p), path, texNodeXML);
      }
      else if (info.supportImageLoadFromInternalFormat && !delayedLoad)
      {
        const std::wstring path = g_objManager.GetLoc(texNodeXML);
        a_pDriver->UpdateImageFromFile(int32_t(*p), path.c_str(), texNodeXML);
      }
      else
        UpdateImageFromFileOrChunk(int32_t(*p), texNode, a_pDriver);
    }
    else
    {
      scn.texturesUsedByDrv.insert(*p);
      a_pDriver->UpdateImage(int32_t(*p), w, h, bpp, dataPtr + dataOffset, texNodeXML);
    }

    texturesUpdated++;
  }

  a_pDriver->EndTexturesUpdate();

  return texturesUpdated;
}

/////
//
int32_t HR_DriverUpdateMaterials(HRSceneInst& scn, ChangeList& objList, IHRRenderDriver* a_pDriver)
{
  a_pDriver->BeginMaterialUpdate();

  if (g_objManager.scnData.materials.size() == 0)
    return 0;

  // we should update meterials in their id order !!!
  //
  std::vector<int32_t> idsToUpdate; 
  idsToUpdate.reserve(objList.matUsed.size());
  std::copy(objList.matUsed.begin(), objList.matUsed.end(), std::back_inserter(idsToUpdate));
  std::sort(idsToUpdate.begin(), idsToUpdate.end());

  int32_t updatedMaterials = 0;

  for (auto p = idsToUpdate.begin(); p != idsToUpdate.end(); ++p)
  {
    int32_t matId = int32_t(*p);
    if (matId < g_objManager.scnData.materials.size())
    {
      pugi::xml_node node = g_objManager.scnData.materials[matId].xml_node_immediate();
      scn.matUsedByDrv.insert(*p);
      a_pDriver->UpdateMaterial(int32_t(*p), node);
      updatedMaterials++;
    }
    else
      g_objManager.BadMaterialId(matId);
  }

  a_pDriver->EndMaterialUpdate();

  return updatedMaterials;
}

/////
//
int32_t HR_DriverUpdateLight(HRSceneInst& scn, ChangeList& objList, IHRRenderDriver* a_pDriver)
{
  a_pDriver->BeginLightsUpdate();

  if (g_objManager.scnData.lights.size() == 0)
    return 0;

  int32_t updatedLights = 0;

  for (auto p = objList.lightUsed.begin(); p != objList.lightUsed.end(); ++p)
  {
    auto id = (*p);
    if (id >= 0)
    {
      pugi::xml_node node = g_objManager.scnData.lights[id].xml_node_immediate();

      scn.lightUsedByDrv.insert(*p);
      a_pDriver->UpdateLight(int32_t(*p), node);
      updatedLights++;
    }
  }

  a_pDriver->EndLightsUpdate();

  return updatedLights;
}


HRMeshDriverInput HR_GetMeshDataPointers(size_t a_meshId)
{
  HRSceneData& scn = g_objManager.scnData;

  HRMeshDriverInput input;
  if (a_meshId >= scn.meshes.size())
    return input;

  HRMesh& mesh = scn.meshes[a_meshId];
  if (mesh.pImpl == nullptr)
    return input;

  input.vertNum = int(mesh.pImpl->vertNum());
  input.triNum  = int(mesh.pImpl->indNum()/3);

  auto chunkId        = mesh.pImpl->chunkId();

  if (chunkId == uint64_t(-1))
    return input;

  ChunkPointer chunk  = g_objManager.scnData.m_vbCache.chunk_at(chunkId);
  char* dataPtr       = (char*)chunk.GetMemoryNow();

  if (dataPtr == nullptr)
  {
     input.pos4f         = nullptr;
     input.norm4f        = nullptr;
     input.texcoord2f    = nullptr;
     input.indices       = nullptr;
     input.triMatIndices = nullptr;
     input.triNum        = 0;
     input.vertNum       = 0;
  }
  else
  {
    uint64_t offsetPos  = mesh.pImpl->offset(L"pos");
    uint64_t offsetNorm = mesh.pImpl->offset(L"norm");
    uint64_t offsetTexc = mesh.pImpl->offset(L"texc");
    uint64_t offsetTang = mesh.pImpl->offset(L"tan");
    uint64_t offsetInd  = mesh.pImpl->offset(L"ind");
    uint64_t offsetMInd = mesh.pImpl->offset(L"mind");

    input.pos4f         = (float*)(dataPtr + offsetPos);
    input.norm4f        = (float*)(dataPtr + offsetNorm);
    input.texcoord2f    = (float*)(dataPtr + offsetTexc);
    input.tan4f         = (float*)(dataPtr + offsetTang);
    input.indices       = (int*)  (dataPtr + offsetInd);
    input.triMatIndices = (int*)  (dataPtr + offsetMInd);
  }

  return input;
}

void UpdateMeshFromChunk(int32_t a_id, HRMesh& mesh, const std::vector<HRBatchInfo>& a_batches, IHRRenderDriver* a_pDriver, const wchar_t* path)
{
  pugi::xml_node nodeXML    = mesh.xml_node_immediate();
  uint64_t       dataOffset = nodeXML.attribute(L"offset").as_ullong();

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
  std::wstring s1(path);
  std::string  s2(s1.begin(), s1.end());
  std::ifstream fin(s2.c_str(), std::ios::binary);
#elif defined WIN32
  std::ifstream fin(path, std::ios::binary);
#endif
  char* dataPtr = nullptr;
  auto chunkId  = mesh.pImpl->chunkId();

  uint64_t offsetFix = 0;

  if (chunkId != uint64_t(-1))
  {
    ChunkPointer chunk = g_objManager.scnData.m_vbCache.chunk_at(chunkId);

    g_objManager.m_tempBuffer.resize(chunk.sizeInBytes / uint64_t(sizeof(int)) + uint64_t(sizeof(int) * 16));
    dataPtr = (char*)&g_objManager.m_tempBuffer[0];

    fin.read(dataPtr, chunk.sizeInBytes);
    offsetFix = 0;
  }
  else
  {
    HydraGeomData::Header header;
    fin.read((char*)&header, sizeof(HydraGeomData::Header));

    g_objManager.m_tempBuffer.resize(header.fileSizeInBytes / uint64_t(sizeof(int)) + uint64_t(sizeof(int) * 16));
    dataPtr = (char*)&g_objManager.m_tempBuffer[0];
    
    fin.read(dataPtr, header.fileSizeInBytes);
    offsetFix = sizeof(HydraGeomData::Header);
  }

  HRMeshDriverInput input;

  uint64_t offsetPos  = mesh.pImpl->offset(L"pos")  - offsetFix;
  uint64_t offsetNorm = mesh.pImpl->offset(L"norm") - offsetFix;
  uint64_t offsetTexc = mesh.pImpl->offset(L"texc") - offsetFix;
  uint64_t offsetTang = mesh.pImpl->offset(L"tan") - offsetFix;
  uint64_t offsetInd  = mesh.pImpl->offset(L"ind")  - offsetFix;
  uint64_t offsetMInd = mesh.pImpl->offset(L"mind") - offsetFix;

  input.vertNum = nodeXML.attribute(L"vertNum").as_int();
  input.triNum  = nodeXML.attribute(L"triNum").as_int();

  input.pos4f         = (float*)(dataPtr + offsetPos);
  input.norm4f        = (float*)(dataPtr + offsetNorm);
  input.tan4f         = (float*)(dataPtr + offsetTang);
  input.texcoord2f    = (float*)(dataPtr + offsetTexc);
  input.indices       = (int*)  (dataPtr + offsetInd);
  input.triMatIndices = (int*)  (dataPtr + offsetMInd);

  a_pDriver->UpdateMesh(a_id, nodeXML, input, &a_batches[0], int32_t(a_batches.size()));

  fin.close();
}


/////
//
int32_t HR_DriverUpdateMeshes(HRSceneInst& scn, ChangeList& objList, IHRRenderDriver* a_pDriver)
{
  a_pDriver->BeginGeomUpdate();

  //static bool wasThere = false;
  HRDriverInfo info = a_pDriver->Info();

  //std::cout << std::endl;
  //std::cout << "##HR_DriverUpdateMeshes: objList.meshUsed.size() = " << objList.meshUsed.size() << std::endl;
  //std::cout << std::endl;

  //auto drawSeq = FormInstDrawSequence(scnlib);

  int32_t updatedMeshes = 0;

  for (auto p = objList.meshUsed.begin(); p != objList.meshUsed.end(); ++p)
  {
    HRMesh& mesh            = g_objManager.scnData.meshes[*p];
    HRMeshDriverInput input = HR_GetMeshDataPointers(*p);
    pugi::xml_node meshNode = mesh.xml_node_immediate();


    const std::wstring delayedLoad = meshNode.attribute(L"dl").as_string();
    const std::wstring locStr      = g_objManager.GetLoc(meshNode);
    const wchar_t* path = (delayedLoad == L"1") ? meshNode.attribute(L"path").as_string() : locStr.c_str();

    if (mesh.pImpl != nullptr)
    {
      const auto& mlist = mesh.pImpl->MList();

      if (input.pos4f == nullptr)
      {
        if (info.supportMeshLoadFromInternalFormat)
        {
          scn.meshUsedByDrv.insert(*p);
          a_pDriver->UpdateMeshFromFile(int32_t(*p), meshNode, path);
        }
        else
        {
          scn.meshUsedByDrv.insert(*p);
          UpdateMeshFromChunk(int32_t(*p), mesh, mlist, a_pDriver, path);
        }
      }
      else
      {
        scn.meshUsedByDrv.insert(*p);
        a_pDriver->UpdateMesh(int32_t(*p), meshNode, input, &mlist[0], int32_t(mlist.size()));
      }

      updatedMeshes++;
    }
    
  }

  a_pDriver->EndGeomUpdate();

  return updatedMeshes;
}

void HR_DriverUpdateCamera(HRSceneInst& scn, ChangeList& objList, IHRRenderDriver* a_pDriver)
{
  if (g_objManager.m_currCamId >= g_objManager.scnData.cameras.size())
    return;

  HRCamera& cam = g_objManager.scnData.cameras[g_objManager.m_currCamId];

  a_pDriver->UpdateCamera(cam.xml_node_immediate());
}

void HR_DriverUpdateSettings(HRSceneInst& scn, ChangeList& objList, IHRRenderDriver* a_pDriver)
{
  if (g_objManager.renderSettings.size() == 0)
    return;

  auto& settings = g_objManager.renderSettings[g_objManager.m_currRenderId];

  a_pDriver->UpdateSettings(settings.xml_node_immediate());
}

void HR_CheckCommitErrors(HRSceneInst& scn, ChangeList& objList)
{
  if (g_objManager.m_badMaterialId.size() != 0)
  {
    std::wstringstream outStr;
    outStr << L"bad material id (10 first): [";
    for (size_t i = 0; i < g_objManager.m_badMaterialId.size() - 1; i++)
      outStr << g_objManager.m_badMaterialId[i] << L", ";
    outStr << g_objManager.m_badMaterialId[g_objManager.m_badMaterialId.size() - 1] << L"]";
    g_objManager.m_badMaterialId.clear();
    std::wstring strErr = outStr.str();
    HrError(L"commit error, ", strErr.c_str());
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void union_sets(ChangeList& in1, const ChangeList& in2)
{
  in1.lightUsed.insert(in2.lightUsed.begin(), in2.lightUsed.end());
  in1.matUsed.insert(in2.matUsed.begin(), in2.matUsed.end());
  in1.meshUsed.insert(in2.meshUsed.begin(), in2.meshUsed.end());
  in1.texturesUsed.insert(in2.texturesUsed.begin(), in2.texturesUsed.end());
}

int64_t EstimateGeometryMem(const ChangeList& a_objList)
{
  int64_t memAmount = 0;

  for (auto texId : a_objList.meshUsed)
  {
    auto meshObj          = g_objManager.scnData.meshes[texId];
    pugi::xml_node node   = meshObj.xml_node_immediate();
    const size_t byteSize = node.attribute(L"bytesize").as_llong();
    memAmount += byteSize;
  }

  return memAmount;
}

int64_t EstimateTexturesMem(const ChangeList& a_objList)
{
  int64_t memAmount = 0;

  for (auto texId : a_objList.texturesUsed)
  {
    auto texObj           = g_objManager.scnData.textures[texId];
    pugi::xml_node node   = texObj.xml_node_immediate();
    const size_t byteSize = node.attribute(L"bytesize").as_llong();
    memAmount += byteSize;
  }

  return memAmount;
}



/////
//
void HR_DriverUpdate(HRSceneInst& scn, IHRRenderDriver* a_pDriver)
{
  if (a_pDriver == nullptr)
    return;

  ChangeList objList = FindChangedObjects(scn, a_pDriver);

  if (scn.meshUsedByDrv.size() == 0) // if this is the first call of HR_DriverUpdateDraw #TODO: refactor this !!!
  {
    HRDriverAllocInfo allocInfo;

    const size_t geomNum  = objList.meshUsed.size();
    const size_t imgNum   = objList.texturesUsed.size();
    const size_t matNum   = objList.matUsed.size();
    const size_t lightNum = objList.lightUsed.size();

    allocInfo.geomNum     = int32_t(geomNum  + geomNum/3  + 100);
    allocInfo.imgNum      = int32_t(imgNum   + imgNum/3   + 100);
    allocInfo.matNum      = int32_t(matNum   + matNum/3   + 100);
    allocInfo.lightNum    = int32_t(lightNum + lightNum/3 + 100);

    const int64_t neededMemT = EstimateTexturesMem(objList);
    const int64_t neededMemG = EstimateGeometryMem(objList);

    allocInfo.imgMem      = (4*neededMemT/3) + size_t(128)*size_t(1024*1024);
    allocInfo.geomMem     = (neededMemG)     + size_t(64)*size_t(1024*1024);
    allocInfo.libraryPath = g_objManager.scnData.m_path.c_str();

    HR_DriverUpdateSettings(scn, objList, a_pDriver);

    allocInfo = a_pDriver->AllocAll(allocInfo);

    if (allocInfo.geomMem < neededMemG)
    {
      std::wstringstream errMsg;
      errMsg << L"RenderDriver can't alloc enough memory for geom, needed = " << neededMemG << L", allocated = " << allocInfo.geomMem;
      std::wstring msg = errMsg.str();
      HrError(msg.c_str());
    }

    if (allocInfo.imgMem < neededMemT)
    {
      std::wstringstream errMsg;
      errMsg << L"RenderDriver can't alloc enough memory for textures, needed = " << neededMemT << L", allocated = " << allocInfo.imgMem;
      std::wstring msg = errMsg.str();
      HrError(msg.c_str());
    }

  }


  g_objManager.m_badMaterialId.clear();

  HR_DriverUpdateCamera(scn, objList, a_pDriver);
  HR_DriverUpdateSettings(scn, objList, a_pDriver); 

  int32_t updatedTextures  = HR_DriverUpdateTextures (scn, objList, a_pDriver);
  int32_t updatedMaterials = HR_DriverUpdateMaterials(scn, objList, a_pDriver);
  int32_t updatedMeshes    = HR_DriverUpdateMeshes   (scn, objList, a_pDriver);
  int32_t updatedLights    = HR_DriverUpdateLight    (scn, objList, a_pDriver);

  HR_CheckCommitErrors    (scn, objList);

  const auto dInfo = a_pDriver->DependencyInfo();

  const bool haveSomeThingNew      = scn.driverDirtyFlag || (updatedTextures > 0) || (updatedMaterials > 0) || (updatedLights > 0) || (updatedMeshes > 0);
  const bool driverMustUpdateScene = dInfo.needRedrawWhenCameraChanges || haveSomeThingNew;

  if (driverMustUpdateScene)
  {
    // draw/add instances to scene
    //
    a_pDriver->BeginScene();

    for (auto p = objList.drawSeq.begin(); p != objList.drawSeq.end(); p++)
    {
      const auto& seq = p->second;
      a_pDriver->InstanceMeshes(p->first, &seq.matrices[0], int32_t(seq.matrices.size() / 16), &seq.linstid[0]);
    }

    // for (size_t i = 0; i < scnlib.drawList.size(); i++) // #NOTE: this loop can be optimized for glDrawElementsInstanced (by grouping together sequence of a single mesh instances)
    // {
    //   auto& instance = scnlib.drawList[i];
    //   a_pDriver->InstanceMeshes(instance.meshId, instance.m, 1, nullptr);
    // }

    for (size_t i = 0; i < scn.drawListLights.size(); i++) // #NOTE: this loop can be optimized
    {
      auto& instance = scn.drawListLights[i];
      a_pDriver->InstanceLights(instance.lightId, instance.m, &instance.node, 1, instance.lightGroupInstId);
    }

    a_pDriver->EndScene();
  }

  // reset dirty flag; now we don't need to Update the scene to driver untill this flag changes or
  // some new objects will be added/updated
  //
  scn.driverDirtyFlag = false; 
}

void HR_DriverDraw(HRSceneInst& scn, IHRRenderDriver* a_pDriver)
{
  a_pDriver->Draw();
}

