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


HRTextureNodeRef _hrTexture2DCreateFromNode(pugi::xml_node a_node)
{
  const wchar_t* a_fileName1 = a_node.attribute(L"name").as_string();
  const wchar_t* a_fileName2 = a_node.attribute(L"path").as_string();
  const std::wstring loc     = g_objManager.GetLoc(a_node);
  const wchar_t* a_chunkPath = loc.c_str(); 

  /////////////////////////////////////////////////////////////////////////////////////////////////
  {
    auto p = g_objManager.scnData.m_textureCache.find(a_fileName2);
    if (p != g_objManager.scnData.m_textureCache.end() && std::wstring(a_fileName2) != L"")
    {
      HRTextureNodeRef ref;
      ref.id = p->second;
      return ref;
    }
  }
  /////////////////////////////////////////////////////////////////////////////////////////////////

  HRTextureNode texRes;
  texRes.name = std::wstring(a_fileName1);
  texRes.id   = g_objManager.scnData.textures.size();
  g_objManager.scnData.textures.push_back(texRes);

  HRTextureNodeRef ref;
  ref.id = HR_IDType(g_objManager.scnData.textures.size() - 1);

  HRTextureNode& texture   = g_objManager.scnData.textures[ref.id];
  texture.m_loadedFromFile = true;

  g_objManager.scnData.textures      [ref.id].update_this(a_node);
  g_objManager.scnData.m_textureCache[a_fileName2] = ref.id; // remember texture id for given file name

  if (std::wstring(a_chunkPath) != L"")
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

  HRMesh mesh;
  mesh.name = std::wstring(a_objectName);
  g_objManager.scnData.meshes.push_back(mesh);

  HRMeshRef ref;
  ref.id = HR_IDType(g_objManager.scnData.meshes.size() - 1);

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
  g_objManager.scnData.lights.push_back(light);

  g_objManager.scnData.lights[ref.id].update_this(a_node);
  g_objManager.scnData.lights[ref.id].id = ref.id;

  return ref;
}

HAPI HRCameraRef _hrCameraCreateFromNode(pugi::xml_node a_node)
{
  const wchar_t* a_objectName = a_node.attribute(L"name").as_string();

  HRCamera cam;
  cam.name = std::wstring(a_objectName);
  g_objManager.scnData.cameras.push_back(cam);

  HRCameraRef ref;
  ref.id = HR_IDType(g_objManager.scnData.cameras.size() - 1);

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
    g_objManager.scnInst.push_back(HRSceneInst());

    HRSceneInstRef a_pScn;
    a_pScn.id = HR_IDType(g_objManager.scnInst.size()-1);

    for (pugi::xml_node nodeInst = node.first_child(); nodeInst != nullptr; nodeInst = nodeInst.next_sibling())
    {
      if (std::wstring(nodeInst.name()) == L"instance")
        _hrMeshInstanceFromNode(a_pScn, nodeInst);
      else if (std::wstring(nodeInst.name()) == L"instance_light")
        _hrLightInstanceFromNode(a_pScn, nodeInst);
    }
    
    g_objManager.scnInst[a_pScn.id].driverDirtyFlag = true; // driver need to Update this scene
    //g_objManager.scnInst[a_pScn.id].update(node);
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