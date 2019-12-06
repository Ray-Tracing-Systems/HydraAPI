#include "HydraObjectManager.h"
#include <unordered_set>
#include <map>

#include <fstream>

#pragma warning(disable:4996)

#if defined(WIN32)
#include "FreeImage.h"
#pragma comment(lib, "FreeImage.lib")
#undef min
#undef max
#else
#include <FreeImage.h>
#endif

#include <algorithm>

#include "HydraVSGFExport.h"
#include "RenderDriverOpenGL3_Utility.h"

#include <chrono>

extern HRObjectManager g_objManager;
extern HR_INFO_CALLBACK  g_pInfoCallback;

using resolution_dict = std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t> >;

struct ChangeList
{
  ChangeList() = default;
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
    std::vector<int32_t>  remapid;
    std::vector<int32_t>  instIdReal;
  };

  std::unordered_map<int32_t, InstancesInfo > drawSeq;

};

void ScanXmlNodeRecursiveAndAppendTexture(pugi::xml_node a_node, std::unordered_set<int32_t>& a_outSet)
{
  if (std::wstring(a_node.name()) == L"texture")
  {
    int32_t id = a_node.attribute(L"id").as_int();
    a_outSet.insert(id);
    
    // list all bitmaps which we depend of
    //
    if (std::wstring(a_node.attribute(L"type").as_string()) == L"texref_proc")
    {
      for (pugi::xml_node arg : a_node.children())
      {
        if (std::wstring(arg.name()) == L"arg" && std::wstring(arg.attribute(L"type").as_string()) == L"sampler2D")
        {
          int size = arg.attribute(L"size").as_int();
          if(size <= 1)
            a_outSet.insert(arg.attribute(L"val").as_int());
          else
          {
            const wchar_t* value = arg.attribute(L"val").as_string();
            std::wstringstream strIn(value);
            for (int i = 0; i < size; i++)
            {
              int texId = 0;
              strIn >> texId;
              a_outSet.insert(texId);
            }
          }
        }
      }
    }
  }
  
  for (pugi::xml_node child = a_node.first_child(); child != nullptr; child = child.next_sibling())
    ScanXmlNodeRecursiveAndAppendTexture(child, a_outSet);
  
}

void AddUsedMaterialChildrenRecursive(ChangeList& objects, int32_t matId)
{
	if (matId >= g_objManager.scnData.materials.size())
		return;

  HRMaterial& mat = g_objManager.scnData.materials[matId];
  auto matNode = mat.xml_node_immediate();
  auto matType = std::wstring(matNode.attribute(L"type").as_string());

  if (matType == L"hydra_blend")
  {
    auto subMatId1 = matNode.attribute(L"node_top").as_int();
    auto subMatId2 = matNode.attribute(L"node_bottom").as_int();

    objects.matUsed.insert(subMatId1);
    objects.matUsed.insert(subMatId2);

    AddUsedMaterialChildrenRecursive(objects, subMatId1);
    AddUsedMaterialChildrenRecursive(objects, subMatId2);
  }
  else if (matType == L"layer_material")
  {
    auto layers = matNode.child(L"layers");
    for (auto node = layers.first_child(); node != nullptr; node = node.next_sibling())
    {
      uint32_t mat_idx   = node.attribute(L"material_id").as_uint();
      objects.matUsed.insert(mat_idx);
      AddUsedMaterialChildrenRecursive(objects, mat_idx);
    }
  }
}

void AddInstanceToDrawSequence(const HRSceneInst::Instance &instance,
                               std::unordered_map<int32_t, ChangeList::InstancesInfo> &drawSeq, int a_instId)
{

  auto p = drawSeq.find(instance.meshId);
  if (p == drawSeq.end())
  {
    drawSeq[instance.meshId].matrices   = std::vector<float>  (instance.m, instance.m + 16);
    drawSeq[instance.meshId].linstid    = std::vector<int32_t>(&instance.lightInstId, &instance.lightInstId + 1);
    drawSeq[instance.meshId].remapid    = std::vector<int32_t>(&instance.remapListId, &instance.remapListId + 1);
    drawSeq[instance.meshId].instIdReal = std::vector<int32_t>(&a_instId, &a_instId + 1);

    const int RESERVE_SIZE = 100;

    drawSeq[instance.meshId].matrices.reserve(RESERVE_SIZE);
    drawSeq[instance.meshId].linstid.reserve(RESERVE_SIZE);
    drawSeq[instance.meshId].remapid.reserve(RESERVE_SIZE);
    drawSeq[instance.meshId].instIdReal.reserve(RESERVE_SIZE);

  }
  else
  {
    std::vector<float> data(instance.m, instance.m + 16);
    p->second.matrices.insert(p->second.matrices.end(), data.begin(), data.end());
    p->second.linstid.push_back(instance.lightInstId);
    p->second.remapid.push_back(instance.remapListId);
    p->second.instIdReal.push_back(a_instId);
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
    AddInstanceToDrawSequence(instance, objects.drawSeq, int(i));
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
  if (!g_objManager.scnData.meshes.empty())
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
        for(const auto& trio : pImpl->MList())
        {
          objects.matUsed.insert(trio.matId);
          AddUsedMaterialChildrenRecursive(objects, trio.matId);
        }
      }
    }

  }

  // (1.3) loop through needed materials to define what textures used in scene 
  //
  if (!g_objManager.scnData.materials.empty())
  {
    for (auto matId : objects.matUsed)
    {
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
  if (!g_objManager.scnData.lights.empty())
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

  // AddMaterialsFromSceneRemapList
  //
  pugi::xml_node scnRemLists = scn.xml_node_immediate().child(L"remap_lists");

  for (auto remapList : scnRemLists.children())
  {
    const wchar_t* inputStr = remapList.attribute(L"val").as_string();
    const int listSize = remapList.attribute(L"size").as_int();
    std::wstringstream inStrStream(inputStr);

    for (int i = 0; i < listSize; i++)
    {
      if (inStrStream.eof())
        break;

      int matId = 0;
      inStrStream >> matId;

      if (objects.matUsed.find(matId)  == objects.matUsed.end() && // we don't add this object to list yet
          scn.matUsedByDrv.find(matId) == scn.matUsedByDrv.end())  // and it was not added in previous updates
      {
        objects.matUsed.insert(matId);
        AddUsedMaterialChildrenRecursive(objects, matId);
      }
    }
  }

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


void UpdateImageFromFileOrChunk(int32_t a_id, HRTextureNode& img, IHRRenderDriver* a_pDriver) // #TODO: debug and test this
{
  pugi::xml_node node = img.xml_node_immediate();

  bool delayedLoad = (node.attribute(L"dl").as_int() == 1);

  if (delayedLoad && img.m_loadedFromFile) // load external image from file 
  {
    const wchar_t* filename = node.attribute(L"path").as_string();

    int width, height, bpp;
    bool loaded = g_objManager.m_pImgTool->LoadImageFromFile(filename, 
                                                             width, height, bpp, g_objManager.m_tempBuffer);

    if(loaded)
      a_pDriver->UpdateImage(a_id, width, height, bpp, (char*)g_objManager.m_tempBuffer.data(), node);
    else
      a_pDriver->UpdateImage(a_id, 0, 0, 0, nullptr, node);

    if (g_objManager.m_tempBuffer.size() > TEMP_BUFFER_MAX_SIZE_DONT_FREE)
      g_objManager.m_tempBuffer = g_objManager.EmptyBuffer();
  }
  else // load chunk
  {
    auto w           = node.attribute(L"width").as_int();
    auto h           = node.attribute(L"height").as_int();
    auto sizeInBytes = node.attribute(L"bytesize").as_llong();

    if(w == 0 || h == 0 || sizeInBytes == 0)
    {
      HrError(L"UpdateImageFromFileOrChunk: zero or unknown image size/resolution");
      return;
    }

    auto bpp = sizeInBytes / (w*h);

    sizeInBytes += size_t(sizeof(int) * 2);

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
    else
      a_pDriver->UpdateImage(a_id, w, h, bpp, nullptr, node);
  }

}

/////
//
int32_t HR_DriverUpdateTextures(HRSceneInst& scn, ChangeList& objList, IHRRenderDriver* a_pDriver)
{
  if (g_objManager.scnData.textures.empty() || a_pDriver == nullptr)
    return 0;

  a_pDriver->BeginTexturesUpdate();

  int32_t texturesUpdated = 0;

  HRDriverInfo info = a_pDriver->Info();

  std::vector<int32_t> texturesUsed;
  texturesUsed.assign(objList.texturesUsed.begin(), objList.texturesUsed.end());
  std::sort(texturesUsed.begin(), texturesUsed.end());

  for (auto texId : texturesUsed)
  {
    if (texId < 0)
      continue;

    HRTextureNode& texNode = g_objManager.scnData.textures[texId];

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
      if (chunkId != uint64_t(-1) && chunkId < g_objManager.scnData.m_vbCache.size()) // cache may be inactive, so m_vbCache.size() size may be 0
      {
        ChunkPointer chunk = g_objManager.scnData.m_vbCache.chunk_at(chunkId);
        dataPtr = (char*)chunk.GetMemoryNow();
      }
    }

    pugi::xml_node texNodeXML  = texNode.xml_node_immediate();
    uint64_t       dataOffset  = texNodeXML.attribute(L"offset").as_ullong(); //#SAFETY: check dataOffset for too big value ?
    bool           delayedLoad = (texNodeXML.attribute(L"dl").as_int() == 1);
    bool isProc = (texNodeXML.attribute(L"loc").as_string() == std::wstring(L"") && !delayedLoad);

    if (dataPtr == nullptr)
    {

      if (info.supportImageLoadFromExternalFormat && texNode.m_loadedFromFile)
      {
        const wchar_t* path = texNodeXML.attribute(L"path").as_string();
        a_pDriver->UpdateImageFromFile(texId, path, texNodeXML);
      }
      else if (info.supportImageLoadFromInternalFormat && !delayedLoad)
      {
        const std::wstring path = g_objManager.GetLoc(texNodeXML);
        a_pDriver->UpdateImageFromFile(texId, path.c_str(), texNodeXML);
      }
      else if(isProc)
      {
        scn.texturesUsedByDrv.insert(texId);
        a_pDriver->UpdateImage(texId, -1, -1, 4, nullptr, texNodeXML);
      }
      else
        UpdateImageFromFileOrChunk(texId, texNode, a_pDriver);
    }
    else
    {
      scn.texturesUsedByDrv.insert(texId);
      a_pDriver->UpdateImage(texId, w, h, bpp, dataPtr + dataOffset, texNodeXML);
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

  if (g_objManager.scnData.materials.empty())
    return 0;

  // we should update meterials in their id order !!!
  //
  std::vector<int32_t> idsToUpdate; 
  idsToUpdate.reserve(objList.matUsed.size());
  std::copy(objList.matUsed.begin(), objList.matUsed.end(), std::back_inserter(idsToUpdate));
  std::sort(idsToUpdate.begin(), idsToUpdate.end());

  int32_t updatedMaterials = 0;

  for (auto matId : idsToUpdate)
  {
    if (matId < g_objManager.scnData.materials.size())
    {
      pugi::xml_node node = g_objManager.scnData.materials[matId].xml_node_immediate();
      scn.matUsedByDrv.insert(matId);
      a_pDriver->UpdateMaterial(int32_t(matId), node);
      if (std::wstring(L"shadow_catcher") == node.attribute(L"type").as_string())
        g_objManager.scnData.m_shadowCatchers.insert(matId);
      updatedMaterials++;
    }
    else
      g_objManager.BadMaterialId(matId);
  }

  a_pDriver->EndMaterialUpdate();

  return updatedMaterials;
}

int32_t _hr_UtilityDriverUpdateMaterials(HRSceneInst& scn, IHRRenderDriver* a_pDriver)
{
  a_pDriver->BeginMaterialUpdate();

  if (g_objManager.scnData.materials.empty())
    return 0;

  int32_t updatedMaterials = 0;

  for(auto matId : scn.matUsedByDrv)
  {
    if (matId < g_objManager.scnData.materials.size())
    {
      pugi::xml_node node = g_objManager.scnData.materials[matId].xml_node_immediate();
      a_pDriver->UpdateMaterial(matId, node);
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

  if (g_objManager.scnData.lights.empty())
    return 0;

  int32_t updatedLights = 0;

  for (auto id : objList.lightUsed)
  {
    if (id >= 0)
    {
      pugi::xml_node node = g_objManager.scnData.lights[id].xml_node_immediate();

      scn.lightUsedByDrv.insert(id);
      a_pDriver->UpdateLight(int32_t(id), node);
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

  auto chunkId  = mesh.pImpl->chunkId();

  if (chunkId == uint64_t(-1))
    return input;

  ChunkPointer chunk;
  if(chunkId < g_objManager.scnData.m_vbCache.size())
    chunk = g_objManager.scnData.m_vbCache.chunk_at(chunkId);

  char* dataPtr = (char*)chunk.GetMemoryNow();
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
    input.allData       = dataPtr;
  }

  return input;
}

void UpdateMeshFromChunk(int32_t a_id, HRMesh& mesh, const std::vector<HRBatchInfo>& a_batches, IHRRenderDriver* a_pDriver, const wchar_t* path, int64_t a_byteSize)
{
  pugi::xml_node nodeXML    = mesh.xml_node_immediate();
  //uint64_t       dataOffset = nodeXML.attribute(L"offset").as_ullong();

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
  std::wstring s1(path);
  std::string  s2(s1.begin(), s1.end());
  std::ifstream fin(s2.c_str(), std::ios::binary);
#elif defined WIN32
  std::ifstream fin(path, std::ios::binary);
#endif 
  g_objManager.m_tempBuffer.resize(a_byteSize / sizeof(int) + sizeof(int) * 16);
  char* dataPtr = (char*)g_objManager.m_tempBuffer.data();
  fin.read(dataPtr, a_byteSize);

  HRMeshDriverInput input;

  uint64_t offsetPos  = mesh.pImpl->offset(L"pos");
  uint64_t offsetNorm = mesh.pImpl->offset(L"norm");
  uint64_t offsetTexc = mesh.pImpl->offset(L"texc");
  uint64_t offsetTang = mesh.pImpl->offset(L"tan");
  uint64_t offsetInd  = mesh.pImpl->offset(L"ind");
  uint64_t offsetMInd = mesh.pImpl->offset(L"mind");

  input.vertNum       = nodeXML.attribute(L"vertNum").as_int();
  input.triNum        = nodeXML.attribute(L"triNum").as_int();

  input.pos4f         = (float*)(dataPtr + offsetPos);
  input.norm4f        = (float*)(dataPtr + offsetNorm);
  input.tan4f         = (float*)(dataPtr + offsetTang);
  input.texcoord2f    = (float*)(dataPtr + offsetTexc);
  input.indices       = (int*)  (dataPtr + offsetInd);
  input.triMatIndices = (int*)  (dataPtr + offsetMInd);
  input.allData       = dataPtr;

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

  for (auto id : objList.meshUsed)
  {
    HRMesh& mesh            = g_objManager.scnData.meshes[id];
    HRMeshDriverInput input = HR_GetMeshDataPointers(id);
    pugi::xml_node meshNode = mesh.xml_node_immediate();

    const std::wstring delayedLoad = meshNode.attribute(L"dl").as_string();
    const std::wstring locStr      = g_objManager.GetLoc(meshNode);
    const wchar_t* path            = (delayedLoad == L"1") ? meshNode.attribute(L"path").as_string() : locStr.c_str();

    if (mesh.pImpl != nullptr)
    {
      const auto& mlist = mesh.pImpl->MList();

      if (input.pos4f == nullptr)
      {
        if (info.supportMeshLoadFromInternalFormat)
        {
          scn.meshUsedByDrv.insert(id);
          a_pDriver->UpdateMeshFromFile(int32_t(id), meshNode, path);
        }
        else
        {
          scn.meshUsedByDrv.insert(id);

          int64_t byteSize = meshNode.attribute(L"bytesize").as_llong();

          UpdateMeshFromChunk(int32_t(id), mesh, mlist, a_pDriver, path, byteSize);
        }
      }
      else
      {
        scn.meshUsedByDrv.insert(id);
        a_pDriver->UpdateMesh(int32_t(id), meshNode, input, &mlist[0], int32_t(mlist.size()));
      }

      updatedMeshes++;
    }
    
  }

  a_pDriver->EndGeomUpdate();

  return updatedMeshes;
}

int32_t _hr_UtilityDriverUpdateMeshes(HRSceneInst& scn, IHRRenderDriver* a_pDriver)
{
  a_pDriver->BeginGeomUpdate();

  HRDriverInfo info = a_pDriver->Info();


  int32_t updatedMeshes = 0;


  for(auto p : scn.meshUsedByDrv)
  {
    HRMesh& mesh            = g_objManager.scnData.meshes[p];
    HRMeshDriverInput input = HR_GetMeshDataPointers(p);
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
          a_pDriver->UpdateMeshFromFile(p, meshNode, path);
        }
        else
        {
          int64_t byteSize = meshNode.attribute(L"bytesize").as_llong();
          UpdateMeshFromChunk(p, mesh, mlist, a_pDriver, path, byteSize);
        }
      }
      else
      {
        a_pDriver->UpdateMesh(p, meshNode, input, &mlist[0], int32_t(mlist.size()));
      }
      updatedMeshes++;
    }
  }

  a_pDriver->EndGeomUpdate();

  return updatedMeshes;
}

void HR_DriverUpdateCamera(HRSceneInst& scn, IHRRenderDriver* a_pDriver)
{
  if (g_objManager.m_currCamId >= g_objManager.scnData.cameras.size())
    return;

  HRCamera& cam = g_objManager.scnData.cameras[g_objManager.m_currCamId];

  a_pDriver->UpdateCamera(cam.xml_node_immediate());
}

void HR_DriverUpdateSettings(HRSceneInst& scn, IHRRenderDriver* a_pDriver)
{
  if (g_objManager.renderSettings.empty())
    return;


  auto& settings = g_objManager.renderSettings[g_objManager.m_currRenderId];

  a_pDriver->UpdateSettings(settings.xml_node_immediate());
}


void HR_CheckCommitErrors(HRSceneInst& scn, ChangeList& objList)
{
  if (!g_objManager.m_badMaterialId.empty())
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

    const int trisNum     = int(node.child(L"indices").attribute(L"bytesize").as_llong()/(sizeof(int)*3)); // aux per poly shadow ray offsets

    memAmount += (byteSize + trisNum*sizeof(float));
  }

  return memAmount + int64_t(1*1024*1024);
}

int64_t EstimateTexturesMem(const ChangeList& a_objList, std::unordered_map<int32_t, HRTexResInfo>& out_texInfo)
{
  int64_t memAmount = 0;

  for (auto texId : a_objList.texturesUsed)
  {
    if (texId < 0 || texId >= g_objManager.scnData.textures.size())
      continue;

    auto texObj           = g_objManager.scnData.textures[texId];
    pugi::xml_node node   = texObj.xml_node_immediate();

    const size_t byteSize    = node.attribute(L"bytesize").as_llong();
    const int widthOriginal  = node.attribute(L"width").as_int();
    const int heightOriginal = node.attribute(L"height").as_int();

    if (byteSize == 0 || heightOriginal == 0 || widthOriginal == 0)
      continue;

    const size_t elemSize    = byteSize / (widthOriginal*heightOriginal);
    HRTexResInfo texInfo;
    texInfo.id  = texId;
    texInfo.w   = widthOriginal;
    texInfo.h   = heightOriginal;
    texInfo.rw  = widthOriginal;
    texInfo.rh  = heightOriginal;
    texInfo.aw  = widthOriginal;
    texInfo.ah  = heightOriginal;
    texInfo.bpp = int(elemSize);
    texInfo.usedAsBump = false;

    if (node.attribute(L"rwidth") != nullptr && node.attribute(L"rheight") != nullptr)
    {
      const int rwidth  = node.attribute(L"rwidth").as_int();
      const int rheight = node.attribute(L"rheight").as_int();
      texInfo.rw = rwidth;
      texInfo.rh = rheight;
      //memAmount += size_t(rwidth*rheight)*elemSize;
    }
    memAmount += byteSize;

    out_texInfo[texId] = texInfo;
  }

  return memAmount;
}


static void FindAllTexturesForNormalMaps(pugi::xml_node a_node, const std::unordered_map<int, pugi::xml_node >& a_matNodes, 
                                         std::unordered_set<int32_t>& out_texIds)
{

  if (a_node.name() == std::wstring(L"displacement"))
  {
    int32_t texId = a_node.child(L"height_map").child(L"texture").attribute(L"id").as_int();
    if(texId == 0)
      texId = a_node.child(L"normal_map").child(L"texture").attribute(L"id").as_int();

    out_texIds.insert(texId);
  }
  else
  {
    for (auto child : a_node.children())
      FindAllTexturesForNormalMaps(child, a_matNodes, out_texIds);
  }

  if (a_node.name() == std::wstring(L"material") && a_node.attribute(L"type").as_string() == std::wstring(L"hydra_blend"))
  {
    int mid1 = a_node.attribute(L"node_top").as_int();
    int mid2 = a_node.attribute(L"node_bottom").as_int();

    auto p1 = a_matNodes.find(mid1);
    auto p2 = a_matNodes.find(mid2);

    if (p1 != a_matNodes.end())
      FindAllTexturesForNormalMaps(p1->second, a_matNodes, out_texIds);

    if (p2 != a_matNodes.end())
      FindAllTexturesForNormalMaps(p2->second, a_matNodes, out_texIds);
  }

}

int64_t EstimateTexturesMemBump(const ChangeList& a_objList, std::unordered_map<int32_t, HRTexResInfo>& a_outTexInfo)
{
  std::unordered_set<int32_t> texturesUsedForNormalMaps;
  texturesUsedForNormalMaps.reserve(100);

  std::unordered_map<int, pugi::xml_node > matNodesById;
  for (auto mId : a_objList.matUsed)
  {
    if (mId < 0 || mId >= g_objManager.scnData.materials.size())
      continue;

    auto& hmat = g_objManager.scnData.materials[mId];
    pugi::xml_node matNode = hmat.xml_node_immediate();
  
    matNodesById[mId] = hmat.xml_node_immediate();
  }

  for (auto matNodePair : matNodesById)
    FindAllTexturesForNormalMaps(matNodePair.second, matNodesById, texturesUsedForNormalMaps);

  // finally calculate needed memory amount
  //
  int64_t memAmount = 0;

  for (auto texId : texturesUsedForNormalMaps)
  {
    if (texId < 0 || texId >= g_objManager.scnData.textures.size())
      continue;

    auto texObj           = g_objManager.scnData.textures[texId];
    pugi::xml_node node   = texObj.xml_node_immediate();
    const size_t byteSize = node.attribute(L"bytesize").as_llong();

    if (byteSize == 0)
      continue;

    if (node.attribute(L"rwidth") != nullptr && node.attribute(L"rheight") != nullptr)
    {
      const int widthOriginal  = node.attribute(L"width").as_int();
      const int heightOriginal = node.attribute(L"height").as_int();
      //const size_t elemSize    = byteSize / (widthOriginal*heightOriginal);
      const int rwidth  = node.attribute(L"rwidth").as_int();
      const int rheight = node.attribute(L"rheight").as_int();
      a_outTexInfo[texId].rw = rwidth;
      a_outTexInfo[texId].rh = rheight;
    }
  
    memAmount += byteSize;

    a_outTexInfo[texId].usedAsBump = true;
  }

  return memAmount;
}

void EstimateMemHungryLights(const ChangeList& a_objList, bool* pIsHDR, int* pHungryLightsNumber, int* pEnvSize)
{
  size_t  envMemAmount   = 0;
  int32_t hungryLightNum = 0;

  std::unordered_set<std::wstring> processed;

  for (auto lid : a_objList.lightUsed)
  {
    auto objLight       = g_objManager.scnData.lights[lid];
    pugi::xml_node node = objLight.xml_node_immediate();

    if (std::wstring(node.attribute(L"distribution").as_string()) == L"ies" ||
        std::wstring(node.attribute(L"shape").as_string()) == L"mesh")
    {
      const std::wstring path = node.child(L"ies").attribute(L"loc").as_string();

      auto p = processed.find(path);
      if (p == processed.end())
      {
        hungryLightNum++;
        processed.insert(path);
      }
    }
    else if (std::wstring(node.attribute(L"type").as_string()) == L"sky")
    {
      pugi::xml_attribute attrId = node.child(L"intensity").child(L"color").child(L"texture").attribute(L"id");
      if (attrId != nullptr)
      {
        int32_t texId          = attrId.as_int();
        auto texObj            = g_objManager.scnData.textures[texId];
        pugi::xml_node nodeTex = texObj.xml_node_immediate();

        const int32_t width    = nodeTex.attribute(L"width").as_int();
        const int32_t height   = nodeTex.attribute(L"height").as_int();
        const size_t bytesize  = nodeTex.attribute(L"bytesize").as_llong(); 
                               
        const int32_t bpp      = int32_t(bytesize / (width*height));

        if (bpp < 16)
        {
          (*pIsHDR) = false;
          if(envMemAmount < 256 * 256 * 4)
            envMemAmount = 256*256*4;
        }
        else
        {
          (*pIsHDR) = true;
          if(envMemAmount < bytesize)
            envMemAmount = bytesize;
        }
      }
    }
  }

  (*pHungryLightsNumber) = hungryLightNum;
  (*pEnvSize)            = int(envMemAmount);
}

bool g_hydraApiDisableSceneLoadInfo = false;

/////
//
void HR_DriverUpdate(HRSceneInst& scn, IHRRenderDriver* a_pDriver)
{
  if (a_pDriver == nullptr)
    return;

  ChangeList objList = FindChangedObjects(scn, a_pDriver);

  auto p = g_objManager.driverAllocated.find(a_pDriver);
  if (p == g_objManager.driverAllocated.end())
  {
    g_objManager.driverAllocated.insert(a_pDriver);
    HRDriverAllocInfo allocInfo;

    const size_t geomNum  = objList.meshUsed.size();
    const size_t imgNum   = objList.texturesUsed.size();
    const size_t matNum   = objList.matUsed.size();
    const size_t lightNum = objList.lightUsed.size();

    allocInfo.geomNum     = int32_t(geomNum  + geomNum/3  + 100);
    allocInfo.imgNum      = int32_t(imgNum   + imgNum/3   + 100);
    allocInfo.matNum      = int32_t(matNum   + matNum/3   + 100);
    allocInfo.lightNum    = int32_t(lightNum + lightNum/3 + 100);

    std::unordered_map<int32_t, HRTexResInfo> allTexInfo;
    std::vector<HRTexResInfo> imgResInfo;
    allTexInfo.reserve(imgNum);

    const int64_t neededMemT  = EstimateTexturesMem(objList, allTexInfo);
    const int64_t neededMemT2 = EstimateTexturesMemBump(objList, allTexInfo);
    const int64_t neededMemG  = EstimateGeometryMem(objList);

    allocInfo.libraryPath   = g_objManager.scnData.m_path.c_str();
    allocInfo.stateFileName = g_objManager.scnData.m_fileState.c_str();
    
    allocInfo.imgMem      = neededMemT;
    allocInfo.imgMemAux   = neededMemT2;
    allocInfo.geomMem     = neededMemG;

    if (g_objManager.scnData.m_texturesLib.attribute(L"resize_textures").as_int() == 1)
    {
      imgResInfo.resize(allocInfo.imgNum);
      for (auto texInfoPair : allTexInfo)
        imgResInfo[texInfoPair.first] = texInfoPair.second;
      allocInfo.imgResInfoArray = imgResInfo.data();
    }
    else
      allocInfo.imgResInfoArray = nullptr;

    EstimateMemHungryLights(objList, 
                            &allocInfo.envIsHDR, 
                            &allocInfo.lightsWithIESNum, 
                            &allocInfo.envLightTexSize);

    HR_DriverUpdateSettings(scn, a_pDriver);

    allocInfo = a_pDriver->AllocAll(allocInfo);

    if (allocInfo.geomMem < neededMemG)
    {
      std::wstringstream errMsg;
      errMsg << L"RenderDriver can't alloc enough memory for geom, needed = " << neededMemG << L", allocated = " << allocInfo.geomMem;
      std::wstring msg = errMsg.str();
      HrError(msg);
    }

    if (allocInfo.imgMem < neededMemT)
    {
      std::wstringstream errMsg;
      errMsg << L"RenderDriver can't alloc enough memory for textures, needed = " << neededMemT << L", allocated = " << allocInfo.imgMem;
      std::wstring msg = errMsg.str();
      HrError(msg);
    }

  }
  
  g_objManager.m_badMaterialId.clear();
  
  auto timeBeg = std::chrono::system_clock::now();
  
  if(g_objManager.m_attachMode && g_objManager.m_pVBSysMutex != nullptr)
    hr_lock_system_mutex(g_objManager.m_pVBSysMutex, VB_LOCK_WAIT_TIME_MS);          // need to lock here because update may load data from virtual buffer
  
  HR_DriverUpdateCamera(scn, a_pDriver);
  HR_DriverUpdateSettings(scn, a_pDriver);

  if(g_objManager.m_attachMode && !g_hydraApiDisableSceneLoadInfo)
    HrPrint(HR_SEVERITY_INFO, L"HydraAPI, loading textures ... ");
  
  int32_t updatedTextures  = HR_DriverUpdateTextures (scn, objList, a_pDriver);
  int32_t updatedMaterials = HR_DriverUpdateMaterials(scn, objList, a_pDriver);
  
  if(g_objManager.m_attachMode && !g_hydraApiDisableSceneLoadInfo)
    HrPrint(HR_SEVERITY_INFO, L"HydraAPI, loading meshes   ... ");
  
  int32_t updatedMeshes    = HR_DriverUpdateMeshes   (scn, objList, a_pDriver);
  int32_t updatedLights    = HR_DriverUpdateLight    (scn, objList, a_pDriver);

  HR_CheckCommitErrors    (scn, objList);
  
  if(g_objManager.m_attachMode && g_objManager.m_pVBSysMutex != nullptr)
    hr_unlock_system_mutex(g_objManager.m_pVBSysMutex);
  
  if(g_objManager.m_attachMode && !g_hydraApiDisableSceneLoadInfo)
    HrPrint(HR_SEVERITY_INFO, L"HydraAPI, begin scene ");
  
  const auto dInfo = a_pDriver->DependencyInfo();

  const bool haveSomeThingNew      = scn.driverDirtyFlag || (updatedTextures > 0) || (updatedMaterials > 0) || (updatedLights > 0) || (updatedMeshes > 0);
  const bool driverMustUpdateScene = dInfo.needRedrawWhenCameraChanges || haveSomeThingNew;

  if (driverMustUpdateScene)
  {
    // draw/add instances to scene
    //
    a_pDriver->BeginScene(scn.xml_node_immediate());

    for (auto p1 = objList.drawSeq.begin(); p1 != objList.drawSeq.end(); p1++)
    {
      const auto& seq = p1->second;
      a_pDriver->InstanceMeshes(p1->first, &seq.matrices[0], int32_t(seq.matrices.size() / 16), &seq.linstid[0], &seq.remapid[0], &seq.instIdReal[0]);
    }

    for (auto& instance : scn.drawListLights) // #NOTE: this loop can be optimized
      a_pDriver->InstanceLights(instance.lightId, instance.m, &instance.node, 1, instance.lightGroupInstId);

    a_pDriver->EndScene();
  }
  
  auto timeEnd  = std::chrono::system_clock::now();
  auto msPassed = std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeBeg).count();
  
  if(g_objManager.m_attachMode && !g_hydraApiDisableSceneLoadInfo)
    HrPrint(HR_SEVERITY_INFO, L"HydraAPI, end scene; total load time = ", float(msPassed)/1000.0f, " s");
  
  // reset dirty flag; now we don't need to Update the scene to driver untill this flag changes or
  // some new objects will be added/updated
  //rj
  scn.driverDirtyFlag = false; 
}

void HR_DriverDraw(HRSceneInst& scn, IHRRenderDriver* a_pDriver)
{
  a_pDriver->Draw();
}


void _hr_UtilityDriverUpdate(HRSceneInst& scn, IHRRenderDriver* a_pDriver)
{
  if (a_pDriver == nullptr)
    return;

  //ChangeList objList = FindChangedObjects(scn, a_pDriver);


  HRDriverAllocInfo allocInfo;

  const size_t geomNum  = scn.meshUsedByDrv.size();
  const size_t imgNum   = scn.texturesUsedByDrv.size();
  const size_t matNum   = scn.matUsedByDrv.size();
  const size_t lightNum = scn.lightUsedByDrv.size();

  allocInfo.geomNum     = int32_t(geomNum  + geomNum/3  + 100);
  allocInfo.imgNum      = int32_t(imgNum   + imgNum/3   + 100);
  allocInfo.matNum      = int32_t(matNum   + matNum/3   + 100);
  allocInfo.lightNum    = int32_t(lightNum + lightNum/3 + 100);
  
  auto& settings = g_objManager.renderSettings[g_objManager.m_currRenderId];
  auto resources_path = settings.xml_node_immediate().child(L"resources_path").text().as_string();
  
  allocInfo.resourcesPath = resources_path;
  allocInfo.libraryPath   = g_objManager.scnData.m_path.c_str();
  allocInfo.stateFileName = g_objManager.scnData.m_fileState.c_str();
  
  allocInfo = a_pDriver->AllocAll(allocInfo);

  HR_DriverUpdateCamera(scn, a_pDriver);
  HR_DriverUpdateSettings(scn, a_pDriver);

  int32_t updatedMaterials = _hr_UtilityDriverUpdateMaterials(scn, a_pDriver);
  int32_t updatedMeshes    = _hr_UtilityDriverUpdateMeshes   (scn, a_pDriver);


  const auto dInfo = a_pDriver->DependencyInfo();


  ///////////////////////////////

  std::unordered_map<int32_t, ChangeList::InstancesInfo > drawSeq;


  for (size_t i = 0; i < scn.drawList.size(); i++)
  {
    auto instance = scn.drawList[i];
    if (instance.meshId >= g_objManager.scnData.meshes.size())
      continue;

    // form draw sequence for each mesh
    //
    AddInstanceToDrawSequence(instance, drawSeq, int(i));
  }

  ////////////////////////
  a_pDriver->BeginScene(scn.xml_node_immediate());
  {
    // draw/add instances to scene
    for (auto p = drawSeq.begin(); p != drawSeq.end(); p++)
    {
      const auto& seq = p->second;
      a_pDriver->InstanceMeshes(p->first, &seq.matrices[0], int32_t(seq.matrices.size() / 16), &seq.linstid[0], &seq.remapid[0], &seq.instIdReal[0]);
    }
  }
  a_pDriver->EndScene();

}

void CreatePrecompProcTex(pugi::xml_document &doc, resolution_dict &dict)
{
  if (g_objManager.scnData.textures.empty())
    return;

  auto scn = g_objManager.scnInst[g_objManager.m_currSceneId];

  for (auto texIdRes : dict)
  {
    auto texRefId = texIdRes.first;
    auto w = texIdRes.second.first;
    auto h = texIdRes.second.second;

    HRTextureNode &texture = g_objManager.scnData.textures[texIdRes.first];

    int bpp = 4;
    bool isProc = false;
    if (texture.hdrCallback != nullptr)
    {
      bpp = sizeof(float) * 4;
      auto *imageData = new float[w * h * bpp / sizeof(float)];

      texture.hdrCallback(imageData, w, h, texture.customData);

      auto pTextureImpl = g_objManager.m_pFactory->CreateTexture2DFromMemory(&texture, w, h, bpp, imageData);
      texture.pImpl = pTextureImpl;

      delete[] imageData;

      isProc = true;
    }
    else if (texture.ldrCallback != nullptr)
    {
      auto *imageData = new unsigned char[w * h * bpp];

      texture.ldrCallback(imageData, w, h, texture.customData);

      auto pTextureImpl = g_objManager.m_pFactory->CreateTexture2DFromMemory(&texture, w, h, bpp, imageData);
      texture.pImpl = pTextureImpl;

      delete[] imageData;

      isProc = true;
    }

    if (isProc)
    {
      auto texIdStr = ToWString(texRefId);
      auto texNode = doc.child(L"textures_lib").find_child_by_attribute(L"texture", L"id", texIdStr.c_str());
      auto byteSize = size_t(w) * size_t(h) * size_t(bpp);

      ChunkPointer chunk = g_objManager.scnData.m_vbCache.chunk_at(texture.pImpl->chunkId());
      std::wstring location = ChunkName(chunk);

      std::wstring bytesize = ToWString(byteSize);
      g_objManager.SetLoc(texNode, location);
      texNode.force_attribute(L"offset").set_value(L"8");
      texNode.force_attribute(L"bytesize").set_value(bytesize.c_str());
      texNode.force_attribute(L"width") = w;
      texNode.force_attribute(L"height") = h;
    }
  }
}


static std::tuple<int, int> RecommendedTexResolutionFix(int w, int h, int rwidth, int rheight)
{

  if(w == -1 || h == -1)
    return std::tuple<int, int>(rwidth, rheight);
  else if(rwidth >= w || rheight >= h)
    return std::tuple<int, int>(w, h);

  if (rwidth < 256 || rheight < 256)
  {
    const double relation = double(w) / double(h);

    if (rwidth >= rheight)
    {
      rheight = 256;
      rwidth = int(double(rheight)*relation);
    }
    else
    {
      rwidth = 256;
      rheight = int(relation / double(rwidth));
    }

  }
  else if (w % rwidth != 0 || h % rheight != 0)
  {
    int w2 = w, h2 = h;

    while ((w2 >> 1) >= rwidth || (h2 >> 1) >= rheight)
    {
      w2 = w2 >> 1;
      h2 = h2 >> 1;
    }

    rwidth = w2;
    rheight = h2;
  }

  return std::tuple<int, int>(rwidth, rheight);
}


resolution_dict InsertMipLevelInfoIntoXML(pugi::xml_document &stateToProcess, const std::unordered_map<uint32_t, uint32_t> &dict, int a_winWidth, int a_winHeight)
{
  resolution_dict resDict;
  for (std::pair<int32_t, int32_t> elem : dict)
  {
    std::wstringstream tmp;
    tmp << elem.first;
    auto texIdStr = tmp.str();
    auto texNode = stateToProcess.child(L"textures_lib").find_child_by_attribute(L"texture", L"id", texIdStr.c_str());

    if(texNode != nullptr)
    {
      int currW = texNode.attribute(L"width").as_int();
      int currH = texNode.attribute(L"height").as_int();

      auto settingsNode = stateToProcess.child(L"render_lib").child(L"render_settings");

      int32_t newW = MAX_TEXTURE_RESOLUTION;
      int32_t newH = MAX_TEXTURE_RESOLUTION;

     /* if(settingsNode != nullptr)
      {
        newW = std::min(settingsNode.child(L"width").text().as_int(), newW);
        newH = std::min(settingsNode.child(L"height").text().as_int(), newH);
      }*/

      for(int i = 0; i < elem.second; ++i)
      {
        newW /= 2;
        newH /= 2;
      }

      std::tie(newW, newH) = RecommendedTexResolutionFix(currW, currH, newW, newH);

      texNode.force_attribute(L"rwidth").set_value(newW);
      texNode.force_attribute(L"rheight").set_value(newH);

      resDict[elem.first] = std::pair<uint32_t, uint32_t>(newW, newH);
    }
  }

  // next, for all textures that we don't see at all insert renderer screen resolution as recommended
  //
  auto texLib = stateToProcess.child(L"textures_lib");

  for (auto texNode : texLib.children())
  {
    const int32_t id = texNode.attribute(L"id").as_int();
    const auto p     = resDict.find(id);

    if (id > 0 && p == resDict.end())
    {
      int32_t newW = a_winWidth;
      int32_t newH = a_winHeight;

      std::tie(newW, newH) = RecommendedTexResolutionFix(texNode.attribute(L"width").as_int(), texNode.attribute(L"height").as_int(), newW, newH);

      texNode.force_attribute(L"rwidth")  = newW;
      texNode.force_attribute(L"rheight") = newH;
    }
  }

  return resDict;
}


std::wstring SaveFixedStateXML(pugi::xml_document &doc, const std::wstring &oldPath, const std::wstring &suffix)
{
  std::wstringstream ss;
  ss << std::wstring(oldPath.begin(), oldPath.end() - 4) << suffix << L".xml"; //cut ".xml" from initial path and append new suffix
  std::wstring new_state_path = ss.str();
  doc.save_file(new_state_path.c_str(), L"  ");
  return new_state_path;
}


#ifdef WIN32
void HydraDestroyHiddenWindow();
bool HydraCreateHiddenWindow(int width, int height, int a_major, int a_minor, int a_flags);
#endif

std::wstring HR_UtilityDriverStart(const wchar_t* state_path)
{
  std::wstring new_state_path(L"");

  if (state_path == std::wstring(L"") || state_path == nullptr)
  {
    HrError(L"No state for Utility driver at location: ", state_path);
    return new_state_path;
  }

  pugi::xml_document stateToProcess;

  auto loadResult = stateToProcess.load_file(state_path);

  if (!loadResult)
  {
    HrError(L"HR_UtilityDriverStart, pugixml load: ", loadResult.description());
    return new_state_path;
  }

  stateToProcess.child(L"textures_lib").force_attribute(L"resize_textures") = 1;

#ifdef WIN32
  bool windowCreated = HydraCreateHiddenWindow(1024, 1024, 3, 3, 0);
  if (!windowCreated)
  {
    HrError(L"HydraCreateHiddenWindow FAILED!");
    return new_state_path;
  }

  gladLoadGLLoader((GLADloadproc)GetProcAddress);
  if (!gladLoadGL())
  {
    HrError(L"gladLoadGL FAILED!");
    return new_state_path;
  }
#else
  auto offscreen_context = InitGLForUtilityDriver();               //#TODO: refactor this
#endif

  std::unique_ptr<IHRRenderDriver> utilityDriver = CreateRenderFromString(L"opengl3Utility", L"");

  if (utilityDriver != nullptr && g_objManager.m_currSceneId < g_objManager.scnInst.size())
  {
    utilityDriver->SetInfoCallBack(g_pInfoCallback);

    _hr_UtilityDriverUpdate(g_objManager.scnInst[g_objManager.m_currSceneId], utilityDriver.get());

    auto mipLevelsDict = getMipLevelsFromUtilityDriver(utilityDriver.get());
//
//#ifdef IN_DEBUG
//    auto pImgTool = g_objManager.m_pImgTool;
//    auto& imgData = g_objManager.m_tempBuffer;
//    if (imgData.size() < 1024*1024)
//      imgData.resize(1024*1024);
//
//    utilityDriver->GetFrameBufferLDR(1024, 1024, imgData.data());
//    pImgTool->SaveLDRImageToFileLDR(L"tests_images/z_out.png", 1024, 1024, imgData.data());
//
//    if (imgData.size() > TEMP_BUFFER_MAX_SIZE_DONT_FREE) // free temp buffer if it's too large
//      imgData = g_objManager.EmptyBuffer();
//#endif

//    for (auto elem : mipLevelsDict)
//      std::cout << " " << elem.first << ":" << elem.second << std::endl;

#ifdef WIN32
    HydraDestroyHiddenWindow();
#else
    glfwSetWindowShouldClose(offscreen_context, GL_TRUE);         //#TODO: refactor this
#endif

    int winWidthRender  = 1024;
    int winHeightRender = 1024;

    auto renderSettings = stateToProcess.child(L"render_lib").child(L"render_settings");
    if (renderSettings != nullptr)
    {
      winWidthRender  = renderSettings.child(L"width").text().as_int();
      winHeightRender = renderSettings.child(L"height").text().as_int();
    }

    auto resDict = InsertMipLevelInfoIntoXML(stateToProcess, mipLevelsDict, winWidthRender, winHeightRender);
    CreatePrecompProcTex(stateToProcess, resDict);
  }
  return SaveFixedStateXML(stateToProcess, state_path, L"_fixed");
}
