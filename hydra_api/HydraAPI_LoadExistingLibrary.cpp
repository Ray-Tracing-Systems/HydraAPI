#include "HydraAPI.h"
#include "HydraInternal.h"

#include <memory>
#include <vector>
#include <string>
#include <map>

#include <sstream>
#include <iomanip>

#include "HydraObjectManager.h"

extern HRObjectManager g_objManager;

#include <string>
#include <iostream>
#include <algorithm>

#include "xxhash.h"
#include "HydraXMLHelpers.h"

#include "tiny_obj_loader.h"

#include "LiteMath.h"
using namespace HydraLiteMath;


HRTextureNodeRef _hrTexture2DCreateFromNode(pugi::xml_node a_node)
{
  const wchar_t* a_fileName1 = a_node.attribute(L"name").as_string();
  const wchar_t* a_fileName2 = a_node.attribute(L"path").as_string();
  const std::wstring loc     = g_objManager.GetLoc(a_node);

  HRTextureNodeRef ref;
  ref.id = HR_IDType(g_objManager.scnData.textures.size());

  HRTextureNode texRes;
  texRes.name = std::wstring(a_fileName1);
  texRes.id   = ref.id;
  g_objManager.scnData.textures.push_back(texRes);


  HRTextureNode& texture   = g_objManager.scnData.textures[ref.id];
  texture.m_loadedFromFile = true;

  g_objManager.scnData.textures      [ref.id].update(a_node);
  g_objManager.scnData.m_textureCache[a_fileName2] = ref.id; // remember texture id for given file name

  if (loc != L"")
    texture.pImpl = g_objManager.m_pFactory->CreateTextureInfoFromChunkFile(&texture, loc.c_str(), a_node);

  return ref;
}

HRMaterialRef _hrMaterialCreateFromNode(pugi::xml_node a_node)
{
  const wchar_t* a_objectName = a_node.attribute(L"name").as_string();

  HRMaterialRef ref;
  ref.id = HR_IDType(g_objManager.scnData.materials.size());

  HRMaterial mat;
  mat.name = std::wstring(a_objectName);
  mat.id = ref.id;
  g_objManager.scnData.materials.push_back(mat);
  g_objManager.scnData.materials[ref.id].update(a_node);

  return ref;
}

const std::wstring GetRealFilePathOfDelayedMesh(pugi::xml_node a_node)
{
  const std::wstring dl       = a_node.attribute(L"dl").as_string();
  const std::wstring loc      = g_objManager.GetLoc(a_node);
  const wchar_t* a_objectName = a_node.attribute(L"name").as_string();
  const wchar_t* a_fileName   = (dl == L"1") ? a_node.attribute(L"path").as_string() : loc.c_str();

  if(std::wstring(a_fileName) == L"") //  then get path from "loc" due to we actually copied file to 'data' folder
    return loc;
  else
    return std::wstring(a_fileName);
}


HAPI HRMeshRef _hrMeshCreateFromNode(pugi::xml_node a_node)
{
  const std::wstring filePathStr = GetRealFilePathOfDelayedMesh(a_node);
  const wchar_t* a_fileName      = filePathStr.c_str();
  const wchar_t* a_objectName    = a_node.attribute(L"name").as_string();

  HRMeshRef ref;

  ref.id = a_node.attribute(L"id").as_int(); //HR_IDType(g_objManager.scnData.meshes.size());

  HRMesh mesh;
  mesh.name = std::wstring(a_objectName);
  mesh.id   = ref.id;
  mesh.update(a_node);
  g_objManager.scnData.meshes.push_back(mesh);

  HRMesh* pMesh = &g_objManager.scnData.meshes.back();
  pMesh->pImpl  = g_objManager.m_pFactory->CreateVSGFProxy(a_fileName); // delay mesh load untill it will be needed by RenderDriver::UpdateMesh

  if (pMesh->pImpl == nullptr)
    HrError(L"LoadExistingLibrary, _hrMeshCreateFromNode can't load mesh from location = ", a_fileName);

  return ref;
}

HRLightRef _hrLightCreateFromNode(pugi::xml_node a_node)
{
  const wchar_t* a_objectName = a_node.attribute(L"name").as_string();

  HRLightRef ref;
  ref.id = HR_IDType(g_objManager.scnData.lights.size());

  HRLight light;
  light.name = std::wstring(a_objectName);
  light.id = ref.id;
  g_objManager.scnData.lights.push_back(light);

  g_objManager.scnData.lights[ref.id].update(a_node);
  g_objManager.scnData.lights[ref.id].id = ref.id;

  return ref;
}

HAPI HRCameraRef _hrCameraCreateFromNode(pugi::xml_node a_node)
{
  const wchar_t* a_objectName = a_node.attribute(L"name").as_string();

  HRCameraRef ref;
  ref.id = HR_IDType(g_objManager.scnData.cameras.size());

  HRCamera cam;
  cam.name = std::wstring(a_objectName);
  cam.id = ref.id;
  g_objManager.scnData.cameras.push_back(cam);


  g_objManager.scnData.cameras[ref.id].update(a_node);
  g_objManager.scnData.cameras[ref.id].id = ref.id;

  return ref;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


HAPI void _hrMeshInstanceFromNode(HRSceneInstRef a_pScn, pugi::xml_node a_node)
{
  HRSceneInst* pScn = g_objManager.PtrById(a_pScn);
  if (pScn == nullptr)
  {
    HrError(L"hrMeshInstance: nullptr input");
    return;
  }

  HRSceneInst::Instance model;
  model.meshId = a_node.attribute(L"mesh_id").as_int();

  if (a_node.attribute(L"linst_id") == nullptr)
  {
    model.lightId     = -1;
    model.lightInstId = -1;
  }
  else
  {
    model.lightId     = a_node.attribute(L"light_id").as_int();
    model.lightInstId = a_node.attribute(L"linst_id").as_int();
  }

  if(a_node.attribute(L"rmap_id") == nullptr)
    model.remapListId = -1;
  else
    model.remapListId = a_node.attribute(L"rmap_id").as_int();

  /////////////////////////////////////////////////////////////////////////////////////////////
  const wchar_t* matString = a_node.attribute(L"matrix").as_string();
  std::wstringstream matStream(matString);
  float a_mat[16];
  for (int i = 0; i < 16; i++)
    matStream >> a_mat[i];
  /////////////////////////////////////////////////////////////////////////////////////////////

  memcpy(model.m, a_mat, 16 * sizeof(float));
  pScn->drawList.push_back(model);
}

HAPI void _hrLightInstanceFromNode(HRSceneInstRef a_pScn, pugi::xml_node a_node)
{
  HRSceneInst* pScn = g_objManager.PtrById(a_pScn);
  if (pScn == nullptr)
  {
    HrError(L"hrMeshInstance: nullptr input");
    return;
  }

  HRSceneInst::Instance model;
  model.lightId          = a_node.attribute(L"light_id").as_int();
  model.lightGroupInstId = a_node.attribute(L"lgroup_id").as_int();
  model.meshId           = -1;
  model.remapListId      = -1;

  /////////////////////////////////////////////////////////////////////////////////////////////
  const wchar_t* matString = a_node.attribute(L"matrix").as_string();
  std::wstringstream matStream(matString);
  float a_mat[16];
  for (int i = 0; i < 16; i++)
    matStream >> a_mat[i];
  /////////////////////////////////////////////////////////////////////////////////////////////

  memcpy(model.m, a_mat, 16 * sizeof(float));
  model.node = a_node;
  pScn->drawListLights.push_back(model);
}

HRRenderRef _hrRenderSettingsFromNode(pugi::xml_node a_node)
{
  const wchar_t* a_className = a_node.attribute(L"type").as_string();

  int maxRaysPerPixel = a_node.child(L"maxRaysPerPixel").text().as_int();
  HRRenderRef ref;
  ref.id = HR_IDType(g_objManager.renderSettings.size());

  HRRender settingsTmp;
  settingsTmp.name = a_className;
  settingsTmp.maxRaysPerPixel = maxRaysPerPixel;
  g_objManager.renderSettings.push_back(settingsTmp);

  HRRender& settings = g_objManager.renderSettings[ref.id];

  g_objManager.renderSettings[ref.id].update(a_node); // ???
  g_objManager.renderSettings[ref.id].id = ref.id;

  settings.m_pDriver = std::shared_ptr<IHRRenderDriver>(RenderDriverFactory::Create(a_className));

  return ref;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ws2s(const std::wstring& s);
std::wstring s2ws(const std::string& s);


void _hrFindTargetOrLastState(const wchar_t* a_libPath, int32_t a_stateId,
                              std::wstring& fileName, int& stateId)
{
  // (0) (a_stateId == -1) => find last state in folder
  //
  
  if (a_stateId == -1)
  {
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
    
    std::wstring s1(a_libPath);
    std::string libPath = ws2s(s1);
    
    auto fileList = hr_listfiles(libPath.c_str());
    
    std::sort(fileList.begin(), fileList.end());
    
    for (auto p : fileList)
    {
      const std::string& currFile = p;
      
      if (currFile.find("statex") != std::string::npos)
      {
        fileName    = s2ws(currFile);
        auto first  = currFile.find("statex_") + 7;
        auto last   = currFile.find(".xml");
        auto strNew = currFile.substr(first, last - first);
        stateId     = atoi(strNew.c_str());
      }
    }
#elif defined WIN32
    auto fileList = hr_listfiles(a_libPath);

    //for (auto p : std_fs::directory_iterator(a_libPath))
    for (auto p : fileList)
    {
      //std::cout << p << std::endl;
      //const std::wstring& currFile = p.path().wstring();
      const std::wstring& currFile = p;

      if (currFile.find(L"statex") != std::wstring::npos)
      {
        fileName = currFile;
        auto first  = fileName.find(L"statex_") + 7;
        auto last   = fileName.find(L".xml");
        auto strNew = fileName.substr(first, last - first);
        auto strStd = ws2s(strNew);
        stateId     = atoi(strStd.c_str());
      }
    }
#endif
  }
}

int32_t _hrSceneLibraryLoad(const wchar_t* a_libPath, int a_stateId, const std::wstring& a_stateFileName)
{
  // (0) (a_stateId == -1) => find last state in folder
  //
  std::wstring fileName = a_stateFileName;
  int stateId           = a_stateId;
  
  if(fileName == L"")
  {
    _hrFindTargetOrLastState(a_libPath, -1,
                             fileName, stateId);
  }
  else
    fileName = std::wstring(a_libPath) + L"/" + a_stateFileName;
  
	if(fileName == L"")
	{
    HrError(L"[_hrSceneLibraryLoad]: can't find existing library at: ", a_libPath);
	  return 0;
  }

  // stateId--; // #NOTE: uncomment this if ypu need to chenge current state?

  // (1) open last state.xml
  //
  g_objManager.scnData.clear();
  g_objManager.scnInst.clear();

  if (g_objManager.m_attachMode)
    HrPrint(HR_SEVERITY_INFO, L"HydraAPI, loading xml ... ");

  auto loadResult = g_objManager.scnData.m_xmlDoc.load_file(fileName.c_str());

  if (!loadResult)
  {
    HrError(L"[_hrSceneLibraryLoad]: pugixml load: ", loadResult.description());
    return 0;
  }

  if (g_objManager.m_attachMode)
    HrPrint(HR_SEVERITY_INFO, L"HydraAPI, initialising virtual buffer");

  g_objManager.scnData.init_existing(g_objManager.m_attachMode, g_objManager.m_pVBSysMutex, g_objManager.m_lastInitInfo.vbSize);

  // (2) set change id to curr value
  //
  g_objManager.scnData.changeId   = stateId;
  g_objManager.scnData.m_commitId = stateId;

  // (3) load textures
  //
  g_objManager.scnData.textures.reserve(HRSceneData::TEXTURES_RESERVE);
  g_objManager.scnData.meshes.reserve(HRSceneData::MESHES_RESERVE);
  g_objManager.scnData.lights.reserve(HRSceneData::LIGHTS_RESERVE);
  g_objManager.scnData.materials.reserve(HRSceneData::MATERIAL_RESERVE);
  g_objManager.scnData.cameras.reserve(HRSceneData::CAMERAS_RESERVE);

  if (g_objManager.m_attachMode)
    HrPrint(HR_SEVERITY_INFO, L"HydraAPI, before system mutex lock");

  if(g_objManager.m_attachMode && g_objManager.m_pVBSysMutex != nullptr)
    hr_lock_system_mutex(g_objManager.m_pVBSysMutex, VB_LOCK_WAIT_TIME_MS); // need to lock here because _hrMeshCreateFromNode may load data from virtual buffer
  
  if (g_objManager.m_attachMode)
    HrPrint(HR_SEVERITY_INFO, L"HydraAPI, loading objects from xml ... ");

  for (pugi::xml_node node = g_objManager.scnData.m_texturesLib.first_child(); node != nullptr; node = node.next_sibling())
    if(node.attribute(L"id") != nullptr)
      _hrTexture2DCreateFromNode(node);

  // (4) load materials
  //
  for (pugi::xml_node node = g_objManager.scnData.m_materialsLib.first_child(); node != nullptr; node = node.next_sibling())
    if(node.attribute(L"id") != nullptr)
      _hrMaterialCreateFromNode(node);

  // (5) load geom
  //
  for (pugi::xml_node node = g_objManager.scnData.m_geometryLib.first_child(); node != nullptr; node = node.next_sibling())
    if(node.attribute(L"id") != nullptr)
      _hrMeshCreateFromNode(node);

  std::sort(
          g_objManager.scnData.meshes.begin(), g_objManager.scnData.meshes.end(),
          [&](auto a, auto b) { return a.id < b.id; });

  // (6) load lights
  //
  for (pugi::xml_node node = g_objManager.scnData.m_lightsLib.first_child(); node != nullptr; node = node.next_sibling())
    _hrLightCreateFromNode(node);

  // (7) load camera
  //
  for (pugi::xml_node node = g_objManager.scnData.m_cameraLib.first_child(); node != nullptr; node = node.next_sibling())
    _hrCameraCreateFromNode(node);

  g_objManager.scnInst.resize(0);
  
  if(g_objManager.m_attachMode && g_objManager.m_pVBSysMutex != nullptr)
    hr_unlock_system_mutex(g_objManager.m_pVBSysMutex);

  if (g_objManager.m_attachMode)
    HrPrint(HR_SEVERITY_INFO, L"HydraAPI, generating instances");

  // (8) load instanced objects (i.e. scenes)
  //
  for (pugi::xml_node node = g_objManager.scnData.m_sceneNode.first_child(); node != nullptr; node = node.next_sibling())
  {

    HRSceneInstRef a_pScn;
    a_pScn.id = HR_IDType(g_objManager.scnInst.size());

    HRSceneInst scn;
    scn.name = node.attribute(L"name").value();
    scn.id = a_pScn.id;

    g_objManager.scnInst.push_back(scn);

    /*g_objManager.scnInst.push_back(HRSceneInst());

    HRSceneInstRef a_pScn;
    a_pScn.id = HR_IDType(g_objManager.scnInst.size()-1);*/

    for (pugi::xml_node nodeInst = node.first_child(); nodeInst != nullptr; nodeInst = nodeInst.next_sibling())
    {
      if (std::wstring(nodeInst.name()) == L"instance")
        _hrMeshInstanceFromNode(a_pScn, nodeInst);
      else if (std::wstring(nodeInst.name()) == L"instance_light")
        _hrLightInstanceFromNode(a_pScn, nodeInst);
    }
    
    g_objManager.scnInst[a_pScn.id].driverDirtyFlag = true; // driver need to Update this scene
    g_objManager.scnInst[a_pScn.id].update(node);
  }

  // (9) load render settings
  //
  for(pugi::xml_node renderSettings : g_objManager.scnData.m_settingsNode.children())
    _hrRenderSettingsFromNode(renderSettings);

  // (10) load empty chunks to have correct chunk id for new objects if we are not in 'attach mode'
  //
  if(!g_objManager.m_attachMode)
  {
    size_t chunks = size_t(g_objManager.scnData.m_geometryLib.attribute(L"total_chunks").as_llong());
    g_objManager.scnData.m_vbCache.ResizeAndAllocEmptyChunks(chunks);
  }
  
  return 1;
}

void fixTextureIds(pugi::xml_node a_node, const std::wstring &a_libPath, const std::unordered_map<int32_t, int32_t> &texIdUpdates,
                   bool mergeDependencies = false)
{
  if (std::wstring(a_node.name()) == L"texture")
  {
    int32_t id = a_node.attribute(L"id").as_int();

    if (mergeDependencies)
    {
      auto texRef = HRUtils::MergeOneTextureIntoLibrary(a_libPath.c_str(), nullptr, id);

      a_node.attribute(L"id").set_value(texRef.id);
    }
    else
    {
      a_node.attribute(L"id").set_value(texIdUpdates.at(id));
    }

  }
  else
  {
    for (pugi::xml_node child = a_node.first_child(); child != nullptr; child = child.next_sibling())
      fixTextureIds(child, a_libPath, texIdUpdates, mergeDependencies);
  }
}


HRMaterialRef _hrMaterialMergeFromNode(pugi::xml_node a_node, const std::wstring &a_libPath, const std::unordered_map<int32_t, int32_t>& texIdUpdates,//int32_t numTexturesPreMerge,
                                       int32_t numMaterialsPreMerge, bool mergeDependencies = false, bool forceMerge = false)
{
  const wchar_t* a_objectName = a_node.attribute(L"name").as_string();

  std::wstring matType = a_node.attribute(L"type").as_string();
  int notBlend = matType.compare(std::wstring(L"hydra_blend"));
  HRMaterialRef ref;

  /*auto isLightMat = (a_node.attribute(L"light_id") != nullptr);

  if(isLightMat && !forceMerge)
    return HRMaterialRef();*/

  if(notBlend)
  {
    ref = hrMaterialCreate(a_objectName);

    hrMaterialOpen(ref, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(ref);

      for (pugi::xml_node node = a_node.first_child(); node != nullptr; node = node.next_sibling())
      {
        matNode.append_copy(node);
      }
      fixTextureIds(matNode, a_libPath, texIdUpdates, mergeDependencies);
    }
    hrMaterialClose(ref);
  }
  else
  {
    HRMaterialRef matA;
    HRMaterialRef matB;

    if (mergeDependencies)
    {
      matA = HRUtils::MergeOneMaterialIntoLibrary(a_libPath.c_str(), nullptr, a_node.attribute(L"node_top").as_int());
      matB = HRUtils::MergeOneMaterialIntoLibrary(a_libPath.c_str(), nullptr, a_node.attribute(L"node_bottom").as_int());
    }
    else
    {
      matA.id = a_node.attribute(L"node_top").as_int() + numMaterialsPreMerge;
      matB.id = a_node.attribute(L"node_bottom").as_int() + numMaterialsPreMerge;
    }

    ref = hrMaterialCreateBlend(a_objectName, matA, matB);

    hrMaterialOpen(ref, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(ref);

      for (pugi::xml_node node = a_node.first_child(); node != nullptr; node = node.next_sibling())
      {
        matNode.append_copy(node);
      }
      fixTextureIds(matNode, a_libPath, texIdUpdates, mergeDependencies);

    }
    hrMaterialClose(ref);
  }

  return ref;
}

HRTextureNodeRef _hrTexture2DMergeFromNode(pugi::xml_node a_node, const std::wstring &a_libPath)
{
  const std::wstring dl       = a_node.attribute(L"dl").as_string();
  const std::wstring loc      = a_node.attribute(L"loc").as_string();
  const std::wstring a_objectName = a_node.attribute(L"name").as_string();

  std::wstring a_fileName;
  if(dl == L"1")
  {
    a_fileName = a_node.attribute(L"path").as_string();
  }
  else
  {
    std::wstringstream ss;
    ss << a_libPath << L"/" << loc;
    a_fileName = ss.str();
  }

  HRTextureNodeRef ref = hrTexture2DCreateFromFile(a_fileName.c_str());

  return ref;
}

HRLightRef _hrLightMergeFromNode(pugi::xml_node a_node, const std::wstring &a_libPath, const std::unordered_map<int32_t, int32_t> &texIdUpdates,
                                 bool mergeDependencies = false)
{
  const wchar_t* a_objectName = a_node.attribute(L"name").as_string();
  const wchar_t* a_lightType = a_node.attribute(L"type").as_string();
  const wchar_t* a_lightShape = a_node.attribute(L"shape").as_string();
  const wchar_t* a_lightDistribution = a_node.attribute(L"distribution").as_string();
  int a_lightVisibility = a_node.attribute(L"visible").as_int();

  HRLightRef ref = hrLightCreate(a_objectName);

  hrLightOpen(ref, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(ref);

    lightNode.force_attribute(L"type").set_value(a_lightType);
    lightNode.force_attribute(L"shape").set_value(a_lightShape);
    lightNode.force_attribute(L"distribution").set_value(a_lightDistribution);
    lightNode.force_attribute(L"visible").set_value(a_lightVisibility);

    for (pugi::xml_node node = a_node.first_child(); node != nullptr; node = node.next_sibling())
    {
      lightNode.append_copy(node);
    }
    fixTextureIds(lightNode, a_libPath, texIdUpdates, mergeDependencies);

  }
  hrLightClose(ref);



  return ref;
}


HRMeshRef _hrMeshMergeFromNode(pugi::xml_node a_node, const std::wstring &a_libPath, int32_t numMaterialsPreMerge,
                               bool mergeDependencies = false, bool forceMerge = false)
{
  const std::wstring dl       = a_node.attribute(L"dl").as_string();
  const std::wstring loc      = a_node.attribute(L"loc").as_string();
  const std::wstring a_objectName = a_node.attribute(L"name").as_string();

  auto isLightMesh = (a_node.attribute(L"light_id") != nullptr);

  //std::cout << "mesh : " << ws2s(a_objectName) << std::endl;

  if(isLightMesh && !forceMerge)
    return HRMeshRef();

  std::wstring a_fileName;
  if(dl == L"1")
  {
    a_fileName = a_node.attribute(L"path").as_string();
  }
  else
  {
    std::wstringstream ss;
    ss << a_libPath << L"/" << loc;
    a_fileName = ss.str();
  }

  HRMeshRef ref = hrMeshCreateFromFileDL(a_fileName.c_str());

  //std::cout << "mesh : " << ws2s(a_fileName) << std::endl;

  hrMeshOpen(ref, HR_TRIANGLE_IND3, HR_OPEN_EXISTING);
  {
    auto meshInfo = hrMeshGetInfo(ref);

    const int triNum = meshInfo.indicesNum / 3;

    auto matindices = (int32_t*)hrMeshGetPrimitiveAttribPointer(ref, L"mind");

    std::unordered_set<int> matIdsToMerge;

    for (auto i = 0; i < triNum; ++i)
    {
      if (mergeDependencies)
      {
        matIdsToMerge.insert(matindices[i]);
      }
      else
      {
        matindices[i] += numMaterialsPreMerge;
      }
    }

    if (mergeDependencies)
    {
      std::unordered_map<int, int> matIdsOldToNew;
      for (auto matId : matIdsToMerge)
      {
        matIdsOldToNew[matId] = HRUtils::MergeOneMaterialIntoLibrary(a_libPath.c_str(), nullptr, matId).id;
      }

      for (auto i = 0; i < triNum; ++i)
      {
        matindices[i] = matIdsOldToNew[matindices[i]];
      }
    }

  }
  hrMeshClose(ref);

  return ref;
}

void _hrInstanceMergeFromNode(HRSceneInstRef a_scn, pugi::xml_node a_node, int32_t numMeshesPreMerge,
                              const std::vector<std::vector<int> > &remap_lists, bool mergeLights = false,
                              int32_t numLightsPreMerge = 0)
{
  std::wstring nodeName = a_node.name();

  float matrix[16];
  HydraXMLHelpers::ReadMatrix4x4(a_node, L"matrix", matrix);

  if(mergeLights && nodeName == std::wstring(L"instance_light"))
  {
    int light_id = a_node.attribute(L"light_id").as_int();
    //int lgroup_id = a_node.attribute(L"lgroup_id").as_int();

    HRLightRef ref;
    ref.id = light_id + numLightsPreMerge;

    hrLightInstance(a_scn, ref, matrix);
  }
  else if(nodeName == std::wstring(L"instance"))
  {
    int mesh_id = a_node.attribute(L"mesh_id").as_int();
    int rmap_id = -1;
    if(a_node.attribute(L"rmap_id") != nullptr)
      rmap_id = a_node.attribute(L"rmap_id").as_int();

    bool isLightMesh = (a_node.attribute(L"light_id") != nullptr);

    if(!isLightMesh)
    {
      HRMeshRef ref;
      ref.id = mesh_id + numMeshesPreMerge;

      if (rmap_id == -1)
        hrMeshInstance(a_scn, ref, matrix);
      else
        hrMeshInstance(a_scn, ref, matrix, &remap_lists.at((unsigned long) rmap_id)[0],
                       int32_t(remap_lists.at((unsigned long) rmap_id).size()));
    }
  }

}


HRSceneInstRef HRUtils::MergeLibraryIntoLibrary(const wchar_t* a_libPath, bool mergeLights, bool copyScene,
                                                const wchar_t* a_stateFileName, MergeInfo* pInfo)
{
  std::wstring fileName(a_stateFileName);
  int stateId = 0;

  if(fileName == L"")
  {
    _hrFindTargetOrLastState(a_libPath, -1,
                             fileName, stateId);
  }
  else
    fileName = std::wstring(a_libPath) + L"/" + a_stateFileName;

  HRSceneInstRef mergedScn;

  if(fileName.empty())
  {
    HrError(L"MergeLibraryIntoLibrary, can't find existing library at: ", a_libPath);
    return mergedScn;
  }

  pugi::xml_document docToMerge;

  auto loadResult = docToMerge.load_file(fileName.c_str());

  if (!loadResult)
  {
    HrError(L"MergeLibraryIntoLibrary, pugixml load: ", loadResult.description());
    return mergedScn;
  }

  auto numTexturesPreMerge  = int32_t(g_objManager.scnData.textures.size());
  auto numMaterialsPreMerge = int32_t(g_objManager.scnData.materials.size());
  auto numMeshesPreMerge    = int32_t(g_objManager.scnData.meshes.size());
  auto numLightsPreMerge    = int32_t(g_objManager.scnData.lights.size());

  if(pInfo != nullptr)
  {
    pInfo->texturesRange[0] = numTexturesPreMerge;
    pInfo->materialRange[0] = numMaterialsPreMerge;
    pInfo->meshRange    [0] = numMeshesPreMerge;
    pInfo->lightsRange  [0] = numLightsPreMerge;
  }

  std::unordered_map<int32_t, int32_t> texIdUpdates;

  for (pugi::xml_node node = docToMerge.child(L"textures_lib").first_child(); node != nullptr; node = node.next_sibling())
  {
    auto ref = _hrTexture2DMergeFromNode(node, std::wstring(a_libPath));
    //bool isNewTex = (g_objManager.scnData.textures.size() - numTexturesPreMerge) > 0;
    auto srcSceneId = node.attribute(L"id").as_int();

    texIdUpdates[srcSceneId] = ref.id;
  }

  //int32_t newTexturesMerged = int32_t(g_objManager.scnData.textures.size()) - numTexturesPreMerge;

  for (pugi::xml_node node = docToMerge.child(L"materials_lib").first_child(); node != nullptr; node = node.next_sibling())
    _hrMaterialMergeFromNode(node, std::wstring(a_libPath), texIdUpdates, numMaterialsPreMerge);

  //int32_t newMaterialsMerged = int32_t(g_objManager.scnData.materials.size()) - numMaterialsPreMerge;


  for (pugi::xml_node node = docToMerge.child(L"geometry_lib").first_child(); node != nullptr; node = node.next_sibling())
    _hrMeshMergeFromNode(node, std::wstring(a_libPath), numMaterialsPreMerge);


  //int32_t newMeshesMerged = int32_t(g_objManager.scnData.meshes.size()) - numMeshesPreMerge;

  if(mergeLights)
  {
    for (pugi::xml_node node = docToMerge.child(L"lights_lib").first_child(); node != nullptr; node = node.next_sibling())
      _hrLightMergeFromNode(node, std::wstring(a_libPath), texIdUpdates);
  }

  if(copyScene)
  {
    mergedScn = hrSceneCreate(a_libPath);

    hrSceneOpen(mergedScn, HR_WRITE_DISCARD);

    std::vector< std::vector <int> > remap_lists;
    pugi::xml_node sceneToMergeNode = docToMerge.child(L"scenes").first_child();
    for (pugi::xml_node node = sceneToMergeNode.first_child(); node != nullptr; node = node.next_sibling())
    {
      if(node.name() == std::wstring(L"remap_lists"))
      {
        remap_lists = HydraXMLHelpers::ReadRemapLists(node);

        for(auto& list : remap_lists)
        {
          std::for_each(list.begin(), list.end(), [=](int &x) { x += numMaterialsPreMerge ; });
         // list += numMaterialsPreMerge;
          /*for(auto i = 0; i < list.size(); ++i)
          {
            if(i % 2 != 0)
              list.at(i) += numMaterialsPreMerge;
          }*/
        }
      }
      else
      {
        _hrInstanceMergeFromNode(mergedScn, node, numMeshesPreMerge, remap_lists, mergeLights, numLightsPreMerge);
      }
    }

    hrSceneClose(mergedScn);
  }

  auto numTexturesPostMerge  = int32_t(g_objManager.scnData.textures.size());
  auto numMaterialsPostMerge = int32_t(g_objManager.scnData.materials.size());
  auto numMeshesPostMerge    = int32_t(g_objManager.scnData.meshes.size());
  auto numLightsPostMerge    = int32_t(g_objManager.scnData.lights.size());

  if(pInfo != nullptr)
  {
    pInfo->texturesRange[1] = numTexturesPostMerge;
    pInfo->materialRange[1] = numMaterialsPostMerge;
    pInfo->meshRange    [1] = numMeshesPostMerge;
    pInfo->lightsRange  [1] = numLightsPostMerge;
  }

  return mergedScn;
}

HRTextureNodeRef HRUtils::MergeOneTextureIntoLibrary(const wchar_t* a_libPath, const wchar_t* a_texName, int a_texId)
{
  std::wstring fileName;
  int stateId = 0;
  HRTextureNodeRef ref;

  _hrFindTargetOrLastState(a_libPath, -1, fileName, stateId);

  if (fileName.empty())
  {
    HrError(L"MergeOneTextureIntoLibrary, can't find existing library at: ", a_libPath);
    return ref;
  }

  pugi::xml_document docToMerge;

  auto loadResult = docToMerge.load_file(fileName.c_str());

  if (!loadResult)
  {
    HrError(L"MergeOneTextureIntoLibrary, pugixml load: ", loadResult.description());
    return ref;
  }

  //auto numTexturesPreMerge = int32_t(g_objManager.scnData.textures.size());

  for (pugi::xml_node node = docToMerge.child(L"textures_lib").first_child(); node != nullptr; node = node.next_sibling())
  {
    std::wstring texName = node.attribute(L"name").as_string();
    if ((a_texName != nullptr && texName == std::wstring(a_texName))
          || node.attribute(L"id").as_int() == a_texId)
    {
      ref = _hrTexture2DMergeFromNode(node, std::wstring(a_libPath));
      break;
    }
  }

  return ref;
}


HRMaterialRef HRUtils::MergeOneMaterialIntoLibrary(const wchar_t* a_libPath, const wchar_t* a_matName, int a_matId)
{
  std::wstring fileName;
  int stateId = 0;
  HRMaterialRef ref;

  _hrFindTargetOrLastState(a_libPath, -1, fileName, stateId);

  if (fileName.empty())
  {
    HrError(L"MergeOneMaterialIntoLibrary, can't find existing library at: ", a_libPath);
    return ref;
  }

  pugi::xml_document docToMerge;

  auto loadResult = docToMerge.load_file(fileName.c_str());

  if (!loadResult)
  {
    HrError(L"MergeOneMaterialIntoLibrary, pugixml load: ", loadResult.description());
    return ref;
  }

  //auto numTexturesPreMerge = int32_t(g_objManager.scnData.textures.size());
  auto numMaterialsPreMerge = int32_t(g_objManager.scnData.materials.size());

  std::unordered_map<int32_t, int32_t> texIdsUpdate;

  for (pugi::xml_node node = docToMerge.child(L"materials_lib").first_child(); node != nullptr; node = node.next_sibling())
  {
    std::wstring matName = node.attribute(L"name").as_string();
    if ((a_matName != nullptr && matName == std::wstring(a_matName))
          || node.attribute(L"id").as_int() == a_matId)
    {
      ref = _hrMaterialMergeFromNode(node, std::wstring(a_libPath), texIdsUpdate, numMaterialsPreMerge, true, true);
      break;
    }
  }

  return ref;
}

HRMeshRef HRUtils::MergeOneMeshIntoLibrary(const wchar_t* a_libPath, const wchar_t* a_meshName)
{
  std::wstring fileName;
  int stateId = 0;
  HRMeshRef ref;

  _hrFindTargetOrLastState(a_libPath, -1, fileName, stateId);

  if (fileName.empty())
  {
    HrError(L"MergeOneMeshIntoLibrary, can't find existing library at: ", a_libPath);
    return ref;
  }

  pugi::xml_document docToMerge;

  auto loadResult = docToMerge.load_file(fileName.c_str());

  if (!loadResult)
  {
    HrError(L"MergeOneMeshIntoLibrary, pugixml load: ", loadResult.description());
    return ref;
  }

  auto numMaterialsPreMerge = int32_t(g_objManager.scnData.materials.size());


  for (pugi::xml_node node = docToMerge.child(L"geometry_lib").first_child(); node != nullptr; node = node.next_sibling())
  {
    std::wstring meshName = node.attribute(L"name").as_string();
    if (meshName == std::wstring(a_meshName))
    {
      ref = _hrMeshMergeFromNode(node, std::wstring(a_libPath), numMaterialsPreMerge, true, true);
      break;
    }
  }
  return ref;
}


HRLightRef HRUtils::MergeOneLightIntoLibrary(const wchar_t* a_libPath, const wchar_t* a_lightName)
{
  std::wstring fileName;
  int stateId = 0;
  HRLightRef ref;

  _hrFindTargetOrLastState(a_libPath, -1, fileName, stateId);

  if (fileName.empty())
  {
    HrError(L"MergeOneLightIntoLibrary, can't find existing library at: ", a_libPath);
    return ref;
  }

  pugi::xml_document docToMerge;

  auto loadResult = docToMerge.load_file(fileName.c_str());

  if (!loadResult)
  {
    HrError(L"MergeOneLightIntoLibrary, pugixml load: ", loadResult.description());
    return ref;
  }

  //auto numTexturesPreMerge = int32_t(g_objManager.scnData.textures.size());

  std::unordered_map<int32_t, int32_t> texIdsUpdate;

  for (pugi::xml_node node = docToMerge.child(L"lights_lib").first_child(); node != nullptr; node = node.next_sibling())
  {
    std::wstring lightName = node.attribute(L"name").as_string();
    if (lightName == std::wstring(a_lightName))
    {
      ref = _hrLightMergeFromNode(node, std::wstring(a_libPath), texIdsUpdate, true);
      break;
    }
  }
  return ref;
}

BBox HRUtils::InstanceSceneIntoScene(HRSceneInstRef a_scnFrom, HRSceneInstRef a_scnTo, float a_mat[16],
                                     bool origin, const int32_t* remapListOverride, int32_t remapListSize)
{
  HRSceneInst *pScn = g_objManager.PtrById(a_scnFrom);
  HRSceneInst *pScn2 = g_objManager.PtrById(a_scnTo);
  bool overrideRemapLists = (remapListSize != 0) && (remapListOverride != nullptr);
  if (pScn == nullptr || pScn2 == nullptr)
  {
    HrError(L"HRUtils::InstanceSceneIntoScene: one of the scenes is nullptr");
    return BBox();
  }

  if (pScn->opened)
  {
    hrSceneClose(a_scnFrom);
  }
  if (pScn2->opened)
  {
    hrSceneClose(a_scnTo);
  }

 /* std::cout << "pScn->m_bbox of loaded scene: " << pScn->m_bbox.x_min << " " << pScn->m_bbox.x_max << " " <<
            pScn->m_bbox.y_min << " " << pScn->m_bbox.y_max << " " <<
            pScn->m_bbox.z_min << " " << pScn->m_bbox.z_max << std::endl;

  std::cout << "matrix: ";
  for(int i = 0; i < 16; ++i)
  {
    std::cout << a_mat[i] << " ";
  }
  std::cout << std::endl;
*/
  BBox bbox(HRUtils::transformBBox(pScn->m_bbox, a_mat));
/*
  std::cout << "bbox of transformed scene: " << bbox.x_min << " " << bbox.x_max << " "
                                             << bbox.y_min << " " << bbox.y_max << " "
                                             << bbox.z_min << " " << bbox.z_max << std::endl;
*/
  hrSceneOpen(a_scnFrom, HR_OPEN_READ_ONLY);

  std::vector<HRSceneInst::Instance> backupListMeshes(pScn->drawList);
  std::vector<HRSceneInst::Instance> backupListLights(pScn->drawListLights);
  std::vector<std::vector<int32_t> > backupListRemapLists;
  if(!overrideRemapLists)
    backupListRemapLists = pScn->m_remapList;

  hrSceneClose(a_scnFrom);

  HR_OPEN_MODE mode;

  if(a_scnFrom.id == a_scnTo.id)
    mode = HR_WRITE_DISCARD;
  else
    mode = HR_OPEN_EXISTING;

  hrSceneOpen(a_scnTo, mode);

  pScn2->instancedScenesCounter++;

  for (auto light : backupListLights)
  {
    HRLightRef tmp;
    tmp.id = light.lightId;
    hrLightInstance(a_scnTo, tmp, light.m);
  }

  for (auto mesh : backupListMeshes)
  {
    if (mesh.lightId < 0)
    {
      HRMeshRef tmp;
      tmp.id = mesh.meshId;
      float currentMatrix[16];
      memcpy(currentMatrix, mesh.m, 16 * sizeof(float));

      float4x4 m1(currentMatrix);
      float4x4 m2(a_mat);
      float4x4 mRes;

      if (origin)
        mRes = mul(m1, m2);
      else
        mRes = mul(m2, m1);

      int remapListId = mesh.remapListId;

      if(overrideRemapLists)
        hrMeshInstance(a_scnTo, tmp, mRes.L(), remapListOverride, remapListSize);
      else if(remapListId == -1)
        hrMeshInstance(a_scnTo, tmp, mRes.L());
      else
        hrMeshInstance(a_scnTo, tmp, mRes.L(), &backupListRemapLists.at((unsigned long)remapListId)[0],
                       int32_t(backupListRemapLists.at((unsigned long)remapListId).size()));


      pScn2->drawList.back().scene_id  = a_scnFrom.id; //actual source scene id
      pScn2->drawList.back().scene_sid = pScn2->instancedScenesCounter; //sequential "scene instance" id
    }
  }

  hrSceneClose(a_scnTo);
  return bbox;
}

MergeInfo HRUtils::LoadMultipleShapesFromObj(const wchar_t* a_objectName, bool a_copyToLocalFolder){
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;

  MergeInfo res_mergeinfo;

  bool res = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, ws2s(a_objectName).c_str());

  for (size_t s = 0; s < shapes.size(); s++) {
    // The number of indices
    int indices_number = shapes[s].mesh.indices.size();

    // Vertices, Normals, Texture coordinates, Indices
    float *verts = new float[indices_number * 4];
    float *norms = new float[indices_number * 4];
    float *tex_s = new float[indices_number * 2];
    int *indxs = new int[indices_number];

    bool has_normals = true;

    size_t index_offset = 0;
    size_t vertices_num = shapes[s].mesh.num_face_vertices.size();
    for (size_t f = 0; f < vertices_num; f++) {
      int fv = shapes[0].mesh.num_face_vertices[f];
      // Loop over vertices in the face.
      for (size_t v = 0; v < fv; v++) {
        // Current index
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
        // Setting the actual index (we duplicate the vertices so that one vertex corresponds to only one index)
        indxs[index_offset + v] = index_offset + v;
        // Setting vertices
        verts[4 * (index_offset + v) + 0] = attrib.vertices[3 * idx.vertex_index + 0];
        verts[4 * (index_offset + v) + 1] = attrib.vertices[3 * idx.vertex_index + 1];
        verts[4 * (index_offset + v) + 2] = attrib.vertices[3 * idx.vertex_index + 2];
        verts[4 * (index_offset + v) + 3] = 1.0;
        // Setting normals
        if (idx.normal_index != -1) {
          norms[4 * (index_offset + v) + 0] = attrib.normals[3 * idx.normal_index + 0];
          norms[4 * (index_offset + v) + 1] = attrib.normals[3 * idx.normal_index + 1];
          norms[4 * (index_offset + v) + 2] = attrib.normals[3 * idx.normal_index + 2];
          norms[4 * (index_offset + v) + 3] = 0.0;
        } else {
          has_normals = false;
        }
        // Setting texture coordinates
        if (idx.texcoord_index != -1) {
          tex_s[2 * (index_offset + v) + 0] = attrib.texcoords[2 * idx.texcoord_index + 0];
          tex_s[2 * (index_offset + v) + 1] = attrib.texcoords[2 * idx.texcoord_index + 1];
        } else {
          tex_s[2 * (index_offset + v) + 0] = 0.0;
          tex_s[2 * (index_offset + v) + 1] = 0.0;
        }
      }
      index_offset += fv;
    }


    HRMeshRef ref = hrMeshCreate(a_objectName);

    hrMeshOpen(ref, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(ref, L"pos", verts);

      if (has_normals)
        hrMeshVertexAttribPointer4f(ref, L"norm", norms);
      else
        hrMeshVertexAttribPointer4f(ref, L"norm", nullptr);

      hrMeshVertexAttribPointer2f(ref, L"texcoord", tex_s);

      hrMeshMaterialId(ref, 0);

      hrMeshAppendTriangles3(ref, indices_number, indxs);
    }
    hrMeshClose(ref);

    if(s == 0)
      res_mergeinfo.meshRange[0] = ref.id;
    if(s == shapes.size() - 1)
      res_mergeinfo.meshRange[1] = ref.id + 1;
  }

  return res_mergeinfo;
}

