#include "HydraAPI.h"
#include "HydraInternal.h"
#include "HydraInternalCommon.h"

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

#include "LiteMath.h"
using namespace HydraLiteMath;


HRTextureNodeRef _hrTexture2DCreateFromNode(pugi::xml_node a_node)
{
  const wchar_t* a_fileName1 = a_node.attribute(L"name").as_string();
  const wchar_t* a_fileName2 = a_node.attribute(L"path").as_string();
  const std::wstring loc     = g_objManager.GetLoc(a_node);
  const wchar_t* a_chunkPath = loc.c_str(); 

  /////////////////////////////////////////////////////////////////////////////////////////////////
  {
    auto p = g_objManager.scnData.m_textureCache.find(a_fileName2);
    if (p != g_objManager.scnData.m_textureCache.end() && !std::wstring(a_fileName2).empty())
    {
      HRTextureNodeRef ref;
      ref.id = p->second;
      return ref;
    }
  }
  /////////////////////////////////////////////////////////////////////////////////////////////////

  HRTextureNodeRef ref;
  ref.id = HR_IDType(g_objManager.scnData.textures.size());

  HRTextureNode texRes;
  texRes.name = std::wstring(a_fileName1);
  texRes.id   = ref.id;
  g_objManager.scnData.textures.push_back(texRes);


  HRTextureNode& texture   = g_objManager.scnData.textures[ref.id];
  texture.m_loadedFromFile = true;

  g_objManager.scnData.textures      [ref.id].update_this(a_node);
  g_objManager.scnData.m_textureCache[a_fileName2] = ref.id; // remember texture id for given file name

  if (!std::wstring(a_chunkPath).empty())
    texture.pImpl = g_objManager.m_pFactory->CreateTextureInfoFromChunkFile(&texture, a_chunkPath);

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
  g_objManager.scnData.materials[ref.id].update_this(a_node);

  return ref;
}

HAPI HRMeshRef _hrMeshCreateFromNode(pugi::xml_node a_node)
{
  const std::wstring dl       = a_node.attribute(L"dl").as_string();
  const std::wstring loc      = g_objManager.GetLoc(a_node);
  const wchar_t* a_objectName = a_node.attribute(L"name").as_string();
  const wchar_t* a_fileName   = (dl == L"1") ? a_node.attribute(L"path").as_string() : loc.c_str();

  HRMeshRef ref;
  ref.id = HR_IDType(g_objManager.scnData.meshes.size());

  HRMesh mesh;
  mesh.name = std::wstring(a_objectName);
  mesh.id = ref.id;
  g_objManager.scnData.meshes.push_back(mesh);

  g_objManager.scnData.meshes[ref.id].update_this(a_node);
  g_objManager.scnData.meshes[ref.id].id = ref.id;

  HRMesh* pMesh = &g_objManager.scnData.meshes[ref.id];
  pMesh->pImpl  = g_objManager.m_pFactory->CreateVSGFFromFile(pMesh, a_fileName);

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

  g_objManager.scnData.lights[ref.id].update_this(a_node);
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


  g_objManager.scnData.cameras[ref.id].update_this(a_node);
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
  model.multiMaterialId = -1;

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
  model.multiMaterialId  = -1;

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

std::unique_ptr<IHRRenderDriver> CreateRenderFromString(const wchar_t *a_className, const wchar_t *a_options);

HRRenderRef _hrRendeSettingsFromNode(pugi::xml_node a_node)
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

  g_objManager.renderSettings[ref.id].update_this(a_node); // ???
  g_objManager.renderSettings[ref.id].id = ref.id;

  settings.m_pDriver = CreateRenderFromString(a_className, L"");

  return ref;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
  std::vector<std::string> hr_listfiles(const std::string &a_folder);
#elif defined WIN32
  std::vector<std::wstring> hr_listfiles(const wchar_t* a_folder);
#endif

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
    
    auto fileList = hr_listfiles(libPath);
    
    std::sort(fileList.begin(), fileList.end());
    
    for (auto p : fileList)
    {
      const std::string& currFile = p;
      
      if (currFile.find("statex") != std::string::npos)
      {
        fileName = s2ws(currFile);
        stateId++;
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
        stateId++;
      }
    }
#endif
  }
}

int32_t _hrSceneLibraryLoad(const wchar_t* a_libPath, int32_t a_stateId)
{
  // (0) (a_stateId == -1) => find last state in folder
  //
  std::wstring fileName = L"";
  int stateId = 0;

  _hrFindTargetOrLastState(a_libPath, a_stateId,
                           fileName, stateId);
  
	if(fileName == L"")
	{
    HrError(L"_hrSceneLibraryLoad, can't find existing library at: ", a_libPath);
	  return -1;
  }

  stateId--;

  // (1) open last state.xml
  //
  g_objManager.scnData.clear();
  g_objManager.scnInst.clear();

  auto loadResult = g_objManager.scnData.m_xmlDoc.load_file(fileName.c_str());

  if (!loadResult)
  {
    HrError(L"_hrSceneLibraryLoad, pugixml load: ", loadResult.description());
    return -1;
  }

  g_objManager.scnData.init_existing(g_objManager.m_emptyVB);

  // (2) set change id to curr value
  //
  g_objManager.scnData.changeId = stateId;
  g_objManager.scnData.m_commitId = stateId;

  // (3) load textures
  //
  g_objManager.scnData.textures.reserve(HRSceneData::TEXTURES_RESERVE);
  g_objManager.scnData.meshes.reserve(HRSceneData::MESHES_RESERVE);
  g_objManager.scnData.lights.reserve(HRSceneData::LIGHTS_RESERVE);
  g_objManager.scnData.materials.reserve(HRSceneData::MATERIAL_RESERVE);
  g_objManager.scnData.cameras.reserve(HRSceneData::CAMERAS_RESERVE);

  for (pugi::xml_node node = g_objManager.scnData.m_texturesLib.first_child(); node != nullptr; node = node.next_sibling())
    _hrTexture2DCreateFromNode(node);

  // (4) load materials
  //
  for (pugi::xml_node node = g_objManager.scnData.m_materialsLib.first_child(); node != nullptr; node = node.next_sibling())
    _hrMaterialCreateFromNode(node);

  // (5) load geom
  //
  for (pugi::xml_node node = g_objManager.scnData.m_geometryLib.first_child(); node != nullptr; node = node.next_sibling())
    _hrMeshCreateFromNode(node);

  // (6) load lights
  //
  for (pugi::xml_node node = g_objManager.scnData.m_lightsLib.first_child(); node != nullptr; node = node.next_sibling())
    _hrLightCreateFromNode(node);

  // (7) load camera
  //
  for (pugi::xml_node node = g_objManager.scnData.m_cameraLib.first_child(); node != nullptr; node = node.next_sibling())
    _hrCameraCreateFromNode(node);

  g_objManager.scnInst.resize(0);
  

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
    g_objManager.scnInst[a_pScn.id].update_this(node);
  }

  // (9) load render settings
  //
  pugi::xml_node renderSettings = g_objManager.scnData.m_settingsNode.first_child();
  _hrRendeSettingsFromNode(renderSettings);


  // (10) load empty chunks to have correct chunk id for new objects
  //
  size_t chunks = size_t(g_objManager.scnData.m_geometryLib.attribute(L"total_chunks").as_llong());
  g_objManager.scnData.m_vbCache.ResizeAndAllocEmptyChunks(chunks);

  return 0;
}

void fixTextureIds(pugi::xml_node a_node, const std::wstring &a_libPath, int32_t increment, bool mergeDependencies = false)
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
      a_node.attribute(L"id").set_value(id + increment);
    }

  }
  else
  {
    for (pugi::xml_node child = a_node.first_child(); child != nullptr; child = child.next_sibling())
      fixTextureIds(child, a_libPath, increment, mergeDependencies);
  }
}


HRMaterialRef _hrMaterialMergeFromNode(pugi::xml_node a_node, const std::wstring &a_libPath, int32_t numTexturesPreMerge, int32_t numMaterialsPreMerge, bool mergeDependencies = false)
{
  const wchar_t* a_objectName = a_node.attribute(L"name").as_string();

  std::wstring matType = a_node.attribute(L"type").as_string();
  int notBlend = matType.compare(std::wstring(L"hydra_blend"));
  HRMaterialRef ref;

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
      fixTextureIds(matNode, a_libPath, numTexturesPreMerge, mergeDependencies);
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
      fixTextureIds(matNode, a_libPath, numTexturesPreMerge, mergeDependencies);

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

HRLightRef _hrLightMergeFromNode(pugi::xml_node a_node, const std::wstring &a_libPath, int32_t numTexturesPreMerge, bool mergeDependencies = false)
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
    fixTextureIds(lightNode, a_libPath, numTexturesPreMerge, mergeDependencies);

  }
  hrLightClose(ref);



  return ref;
}


HRMeshRef _hrMeshMergeFromNode(pugi::xml_node a_node, const std::wstring &a_libPath, int32_t numMaterialsPreMerge, bool mergeDependencies = false)
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

  HRMeshRef ref = hrMeshCreateFromFileDL(a_fileName.c_str());

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

void _hrInstanceMergeFromNode(HRSceneInstRef a_scn, pugi::xml_node a_node, int32_t numMeshesPreMerge, bool mergeLights = false, int32_t numLightsPreMerge = 0)
{
  std::wstring nodeName = a_node.name();

  float matrix[16];
  HydraXMLHelpers::ReadMatrix4x4(a_node, L"matrix", matrix);

  if(mergeLights && nodeName == std::wstring(L"instance_light"))
  {
    int light_id = a_node.attribute(L"light_id").as_int();
    int lgroup_id = a_node.attribute(L"lgroup_id").as_int();

    HRLightRef ref;
    ref.id = light_id + numLightsPreMerge;

    hrLightInstance(a_scn, ref, matrix);
  }
  else if(nodeName == std::wstring(L"instance"))
  {
    int mesh_id = a_node.attribute(L"mesh_id").as_int();
    int mmat_id = a_node.attribute(L"mmat_id").as_int();

    HRMeshRef ref;
    ref.id = mesh_id + numMeshesPreMerge;

    hrMeshInstance(a_scn, ref, matrix);
  }

}


HRSceneInstRef HRUtils::MergeLibraryIntoLibrary(const wchar_t* a_libPath, bool mergeLights, bool copyScene)
{
  std::wstring fileName;
  int stateId = 0;

  _hrFindTargetOrLastState(a_libPath, -1,
                           fileName, stateId);

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
  auto numMeshesPreMerge  = int32_t(g_objManager.scnData.meshes.size());
  int32_t numLightsPreMerge = 0;

  for (pugi::xml_node node = docToMerge.child(L"textures_lib").first_child(); node != nullptr; node = node.next_sibling())
    _hrTexture2DMergeFromNode(node, std::wstring(a_libPath));

  for (pugi::xml_node node = docToMerge.child(L"materials_lib").first_child(); node != nullptr; node = node.next_sibling())
    _hrMaterialMergeFromNode(node, std::wstring(a_libPath), numTexturesPreMerge, numMaterialsPreMerge);

  for (pugi::xml_node node = docToMerge.child(L"geometry_lib").first_child(); node != nullptr; node = node.next_sibling())
    _hrMeshMergeFromNode(node, std::wstring(a_libPath), numMaterialsPreMerge);

  if(mergeLights)
  {
    numLightsPreMerge = int32_t(g_objManager.scnData.lights.size());
    for (pugi::xml_node node = docToMerge.child(L"lights_lib").first_child(); node != nullptr; node = node.next_sibling())
      _hrLightMergeFromNode(node, std::wstring(a_libPath), numTexturesPreMerge);
  }

  if(copyScene)
  {
    mergedScn = hrSceneCreate(L"merged scene");

    hrSceneOpen(mergedScn, HR_WRITE_DISCARD);

    pugi::xml_node sceneToMergeNode = docToMerge.child(L"scenes").first_child();
    for (pugi::xml_node node = sceneToMergeNode.first_child(); node != nullptr; node = node.next_sibling())
    {
      _hrInstanceMergeFromNode(mergedScn, node, numMeshesPreMerge, mergeLights, numLightsPreMerge);
    }

    hrSceneClose(mergedScn);
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

  auto numTexturesPreMerge = int32_t(g_objManager.scnData.textures.size());

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

  auto numTexturesPreMerge = int32_t(g_objManager.scnData.textures.size());
  auto numMaterialsPreMerge = int32_t(g_objManager.scnData.materials.size());

  for (pugi::xml_node node = docToMerge.child(L"materials_lib").first_child(); node != nullptr; node = node.next_sibling())
  {
    std::wstring matName = node.attribute(L"name").as_string();
    if ((a_matName != nullptr && matName == std::wstring(a_matName))
          || node.attribute(L"id").as_int() == a_matId)
    {
      ref = _hrMaterialMergeFromNode(node, std::wstring(a_libPath), numTexturesPreMerge, numMaterialsPreMerge, true);
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
      ref = _hrMeshMergeFromNode(node, std::wstring(a_libPath), numMaterialsPreMerge, true);
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

  auto numTexturesPreMerge = int32_t(g_objManager.scnData.textures.size());

  for (pugi::xml_node node = docToMerge.child(L"lights_lib").first_child(); node != nullptr; node = node.next_sibling())
  {
    std::wstring lightName = node.attribute(L"name").as_string();
    if (lightName == std::wstring(a_lightName))
    {
      ref = _hrLightMergeFromNode(node, std::wstring(a_libPath), numTexturesPreMerge, true);
      break;
    }
  }
  return ref;
}

void HRUtils::InstanceSceneIntoScene(HRSceneInstRef a_scnFrom, HRSceneInstRef a_scnTo, float a_mat[16], bool origin)
{
  HRSceneInst *pScn = g_objManager.PtrById(a_scnFrom);
  HRSceneInst *pScn2 = g_objManager.PtrById(a_scnTo);
  if (pScn == nullptr || pScn2 == nullptr)
  {
    HrError(L"HRUtils::InstanceSceneIntoScene: one of the scenes is nullptr");
    return;
  }

  if (pScn->opened)
  {
    hrSceneClose(a_scnFrom);
  }
  if (pScn2->opened)
  {
    hrSceneClose(a_scnTo);
  }

  hrSceneOpen(a_scnFrom, HR_OPEN_READ_ONLY);

  std::vector<HRSceneInst::Instance> backupListMeshes(pScn->drawList);
  std::vector<HRSceneInst::Instance> backupListLights(pScn->drawListLights);

  hrSceneClose(a_scnFrom);

  HR_OPEN_MODE mode;

  if(a_scnFrom.id == a_scnTo.id)
    mode = HR_WRITE_DISCARD;
  else
    mode = HR_OPEN_EXISTING;

  hrSceneOpen(a_scnTo, mode);
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

      hrMeshInstance(a_scnTo, tmp, mRes.L());
    }
  }

  hrSceneClose(a_scnTo);

}

