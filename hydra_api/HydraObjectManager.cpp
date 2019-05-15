#include "HydraObjectManager.h"

HRObjectManager g_objManager;

std::wstring      g_lastErrorCallerPlace = L"";
std::wstring      g_lastError = L"";
HR_ERROR_CALLBACK g_pErrorCallback = nullptr;
HR_INFO_CALLBACK  g_pInfoCallback  = nullptr;


void HrError(std::wstring a_str) 
{ 
  if (g_pInfoCallback != nullptr)
    g_pInfoCallback(a_str.c_str(), g_lastErrorCallerPlace.c_str(), HR_SEVERITY_ERROR);
  else if (g_pErrorCallback != nullptr)
    g_pErrorCallback(g_lastError.c_str(), g_lastErrorCallerPlace.c_str());

  g_lastError = a_str;
}

void _HrPrint(HR_SEVERITY_LEVEL a_level, const wchar_t* a_str)
{
  if (g_pInfoCallback != nullptr)
    g_pInfoCallback(a_str, g_lastErrorCallerPlace.c_str(), a_level);
  
  if (g_pErrorCallback != nullptr && a_level >= HR_SEVERITY_ERROR)
    g_pErrorCallback(a_str, g_lastErrorCallerPlace.c_str());
}


std::wstring&     getErrCallerWstrObject() { return g_lastErrorCallerPlace; }
std::wstring&     getErrWstrObject()       { return g_lastError; }
HR_ERROR_CALLBACK getErrorCallback()       { return g_pErrorCallback; }
HR_INFO_CALLBACK  getPrintCallback()       { return g_pInfoCallback; }


pugi::xml_node get_global_trash_node() { return g_objManager.trash_node(); }


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void _hrInitPostProcess();

namespace HydraRender
{
  std::unique_ptr<IHRImageTool> CreateImageTool();
};

void HRObjectManager::init(const wchar_t* a_className)
{
  m_useLocalPath               = false;
  m_copyTexFilesToLocalStorage = false;
  m_sortTriIndices             = false;
  m_attachMode                 = false;
  m_computeBBoxes              = false;

  std::wistringstream instr(a_className);

  while (!instr.eof())
  {
    std::wstring name;
    int val = 0;

    instr >> name >> val; // #TODO: make this code secure, check string length, prevent buffer overflow

    if (std::wstring(name) == L"-copy_textures_to_local_folder" && val != 0)
      m_copyTexFilesToLocalStorage = true;
    else if (std::wstring(name) == L"-local_data_path" && val != 0)
      m_useLocalPath = true;
    else if (std::wstring(name) == L"-sort_indices" && val != 0)
      m_sortTriIndices = true;
    else if (std::wstring(name) == L"-emptyvirtualbuffer" && val != 0)
      m_attachMode = true;
    else if (std::wstring(name) == L"-compute_bboxes" && val != 0)
      m_computeBBoxes = true;
  }
  
  m_pFactory = new HydraFactoryCommon;
  if(SharedVirtualBufferIsEnabled())
    m_pVBSysMutex = hr_create_system_mutex("hydra_virtual_buffer_lock");
  else
    m_pVBSysMutex = nullptr;
  scnData.init(m_attachMode, m_pVBSysMutex);

  m_pImgTool = HydraRender::CreateImageTool();
  _hrInitPostProcess();
}

void _hrDestroyPostProcess();

void HRObjectManager::destroy()
{
  hr_free_system_mutex(m_pVBSysMutex);
  delete m_pFactory; m_pFactory = nullptr;
  
  g_objManager.m_pDriver = nullptr; // delete curr render driver pointer to prevent global reference;
  for (auto& r : renderSettings)
    r.clear();
  renderSettings.clear();

  scnData.clear(); // for all scnData --> .clear()
  scnInst.clear();
  _hrDestroyPostProcess();

	scnData.m_xmlDoc.reset();
	scnData.m_xmlDocChanges.reset();

	scnData.m_texturesLib  = pugi::xml_node();
	scnData.m_materialsLib = pugi::xml_node();
	scnData.m_lightsLib    = pugi::xml_node();
	scnData.m_cameraLib    = pugi::xml_node();
	scnData.m_geometryLib  = pugi::xml_node();
	scnData.m_settingsNode = pugi::xml_node();
	scnData.m_sceneNode    = pugi::xml_node();

	scnData.m_texturesLibChanges	 = pugi::xml_node();
	scnData.m_materialsLibChanges  = pugi::xml_node();
	scnData.m_lightsLibChanges		 = pugi::xml_node();
	scnData.m_cameraLibChanges		 = pugi::xml_node();
	scnData.m_geometryLibChanges	 = pugi::xml_node();
	scnData.m_settingsNodeChanges  = pugi::xml_node();
	scnData.m_sceneNodeChanges		 = pugi::xml_node();

	scnData.m_vbCache.Destroy();
}

const std::wstring HRObjectManager::GetLoc(pugi::xml_node a_node) const
{
  return scnData.m_path + std::wstring(L"/") + std::wstring(a_node.attribute(L"loc").as_string());
}

void HRObjectManager::SetLoc(pugi::xml_node a_node, const std::wstring& a_loc)
{
  if(a_loc == L"unknown")
  {
    a_node.attribute(L"loc") = a_loc.c_str();
    return;
  }

  const std::wstring& libPath = scnData.m_path;
  const size_t charsNum       = libPath.size();
  const std::wstring loc      = a_loc.substr(charsNum+1, a_loc.size());

  if (a_node.attribute(L"loc") != nullptr)
    a_node.attribute(L"loc").set_value(loc.c_str());
  else
    a_node.append_attribute(L"loc").set_value(loc.c_str());
}


HRMesh* HRObjectManager::PtrById(HRMeshRef a_ref)
{
  if (scnData.meshes.empty())
    return nullptr;
  else if (a_ref.id < 0 || a_ref.id > (int)(scnData.meshes.size()))
  {
    HrError(L"Invalid HRMeshRef, id = ", a_ref.id);
    return nullptr;
  }
  else
    return &scnData.meshes[a_ref.id];
}

HRLight* HRObjectManager::PtrById(HRLightRef a_ref)
{
  if (scnData.lights.empty())
    return nullptr;
  else if (a_ref.id < 0 || a_ref.id > (int)scnData.lights.size())
  {
    HrError(L"Invalid HRLightRef, id = ", a_ref.id);
    return nullptr;
  }
  else
    return &scnData.lights[a_ref.id];
}

HRMaterial* HRObjectManager::PtrById(HRMaterialRef a_ref)
{
  if (scnData.materials.empty())
    return nullptr;
  else if (a_ref.id < 0 || a_ref.id > (int)scnData.materials.size())
  {
    HrError(L"Invalid HRMaterialRef, id = ", a_ref.id);
    return nullptr;
  }
  else
    return &scnData.materials[a_ref.id];
}

HRCamera* HRObjectManager::PtrById(HRCameraRef a_ref)
{
  if (scnData.cameras.empty())
    return nullptr;
  else if (a_ref.id < 0 || a_ref.id > (int)scnData.cameras.size())
  {
    //Error(L"Invalid HRCameraRef, id = ", a_ref.id);
    return nullptr;
  }
  else
    return &scnData.cameras[a_ref.id];
}

HRTextureNode* HRObjectManager::PtrById(HRTextureNodeRef a_ref)
{
  if (scnData.textures.empty())
    return nullptr;
  else if (a_ref.id < 0 || a_ref.id > (int)scnData.textures.size())
  {
    //Error(L"Invalid HRTextureNodeRef, id = ", a_ref.id);
    return nullptr;
  }
  else
    return &scnData.textures[a_ref.id];
}

HRSceneInst* HRObjectManager::PtrById(HRSceneInstRef a_ref)
{
  if (scnInst.empty())
    return nullptr;
  else if (a_ref.id < 0 || a_ref.id > (int)scnInst.size())
  {
    //Error(L"Invalid HRSceneInstRef, id = ", a_ref.id);
    return nullptr;
  }
  else
    return &scnInst[a_ref.id];
}

HRRender* HRObjectManager::PtrById(HRRenderRef a_ref)
{
  if (renderSettings.empty())
    return nullptr;
  else if (a_ref.id < 0 || a_ref.id > (int)renderSettings.size())
  {
    //Error(L"Invalid HRRenderRef, id = ", a_ref.id);
    return nullptr;
  }
  else
    return &renderSettings[a_ref.id];
}



void HRObjectManager::CommitChanges(pugi::xml_document& a_from, pugi::xml_document& a_to)
{
  // copy 'a_from' to 'a_to' #TODO: optimize this brute force Update loop
  //
  for (size_t i = 0; i < g_objManager.scnData.lights.size(); i++)
    g_objManager.scnData.lights[i].commit();

  for (size_t i = 0; i < g_objManager.scnData.materials.size(); i++)
    g_objManager.scnData.materials[i].commit();

	for (size_t i = 0; i < g_objManager.scnData.textures.size(); i++)
		g_objManager.scnData.textures[i].commit();

  for (size_t i = 0; i < g_objManager.scnData.cameras.size(); i++)
    g_objManager.scnData.cameras[i].commit();

  for (size_t i = 0; i < g_objManager.scnData.meshes.size(); i++)
    g_objManager.scnData.meshes[i].commit();

  for (size_t i = 0; i < g_objManager.scnInst.size(); i++)
    g_objManager.scnInst[i].commit();

  for (size_t i = 0; i < g_objManager.renderSettings.size(); i++)
    g_objManager.renderSettings[i].commit();

  // ...
  //
  scnData.m_texturesLib         = a_to.child(L"textures_lib");
  scnData.m_materialsLib        = a_to.child(L"materials_lib");
  scnData.m_lightsLib           = a_to.child(L"lights_lib");
  scnData.m_geometryLib         = a_to.child(L"geometry_lib");
  scnData.m_cameraLib           = a_to.child(L"cam_lib");
  scnData.m_settingsNode        = a_to.child(L"render_lib");
  scnData.m_sceneNode           = a_to.child(L"scenes");

	scnData.m_texturesLibChanges  = a_from.child(L"textures_lib");
	scnData.m_materialsLibChanges = a_from.child(L"materials_lib");
  scnData.m_lightsLibChanges    = a_from.child(L"lights_lib");
  scnData.m_geometryLibChanges  = a_from.child(L"geometry_lib");
  scnData.m_cameraLibChanges    = a_from.child(L"cam_lib");
  scnData.m_settingsNodeChanges = a_from.child(L"render_lib");
  scnData.m_sceneNodeChanges    = a_from.child(L"scenes");


  // clear changes
  //
  clear_node_childs(scnData.m_texturesLibChanges);
  clear_node_childs(scnData.m_materialsLibChanges);
  clear_node_childs(scnData.m_lightsLibChanges);
  clear_node_childs(scnData.m_geometryLibChanges);
  clear_node_childs(scnData.m_settingsNodeChanges);
  clear_node_childs(scnData.m_sceneNodeChanges);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void clear_node(pugi::xml_node a_xmlNode)
{
  if (a_xmlNode == nullptr)
    return;

  // clear all attribures
  //
  for (pugi::xml_attribute attr = a_xmlNode.first_attribute(); attr; )
  {
    pugi::xml_attribute next = attr.next_attribute();
    a_xmlNode.remove_attribute(attr);
    attr = next;
  }

  clear_node_childs(a_xmlNode);
}


void clear_node_childs(pugi::xml_node a_xmlNode)
{
  if (a_xmlNode == nullptr)
    return;

  for (pugi::xml_node child = a_xmlNode.first_child(); child;)
  {
    pugi::xml_node next = child.next_sibling();
    child.parent().remove_child(child);
    child = next;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
\brief copy a_proto without its internal nodes. Only attributes are copied.
\param a_to    - node data we want to copy
\param a_proto - node where we want copy data from a_proto to.

*/

inline pugi::xml_node append_copy_lite(pugi::xml_node a_to, pugi::xml_node a_proto) ///< not deep (i.e. lite) copy for xml node
{
  const wchar_t* nodeName = a_proto.name();

  pugi::xml_node copy = a_to.append_child(nodeName);
  for (pugi::xml_attribute attr = a_proto.first_attribute(); attr != nullptr; attr = attr.next_attribute())
    copy.append_copy(attr);
  
  return copy;
}

/**
\brief copy a_proto data to nodeCopyTo by replacing all nodeCopyTo's internal data. nodeCopyTo is a child of libNodeTo
\param a_proto    - node data we want to copy
\param nodeCopyTo - node where we want copy data from a_proto to.
\param libNodeTo  - parent of nodeCopyTo. Note that nodeCopyTo mat be null ,so we must pass libNodeTo explicit from some-where.
\param a_lite     - make deep or lite copy

*/

inline pugi::xml_node replace_copy(pugi::xml_node a_proto, pugi::xml_node& nodeCopyTo, pugi::xml_node& libNodeTo, bool a_lite)
{
  if (nodeCopyTo == nullptr)
  {
    if (a_lite)
      nodeCopyTo = append_copy_lite(libNodeTo, a_proto);
    else
      nodeCopyTo = libNodeTo.append_copy(a_proto);
      //nodeCopyTo = libNodeTo.append_move(a_proto);
  }
  else
  {
    pugi::xml_node resNode = libNodeTo.insert_copy_after(a_proto, nodeCopyTo);
    libNodeTo.remove_child(nodeCopyTo);
    nodeCopyTo = resNode;
  }

  return nodeCopyTo;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

pugi::xml_node HRMesh::copy_node(pugi::xml_node a_proto, bool a_lite)
{
  const int32_t id = a_proto.attribute(L"id").as_int();
  if (id == -1)
    return pugi::xml_node();

  auto& nodeCopyTo = g_objManager.scnData.meshes[id].m_xmlNodeNext;
  auto& libNodeTo  = g_objManager.scnData.m_geometryLibChanges;

  return replace_copy(a_proto, nodeCopyTo, libNodeTo, a_lite);
}

pugi::xml_node HRLight::copy_node(pugi::xml_node a_proto, bool a_lite)
{
  const int32_t id = a_proto.attribute(L"id").as_int();
  if (id == -1)
    return pugi::xml_node();

  auto& nodeCopyTo = g_objManager.scnData.lights[id].m_xmlNodeNext;
  auto& libNodeTo  = g_objManager.scnData.m_lightsLibChanges;

  return replace_copy(a_proto, nodeCopyTo, libNodeTo, a_lite);
}

pugi::xml_node HRMaterial::copy_node(pugi::xml_node a_proto, bool a_lite)
{
  const int32_t id = a_proto.attribute(L"id").as_int();
  if (id == -1)
    return pugi::xml_node();

  auto& nodeCopyTo = g_objManager.scnData.materials[id].m_xmlNodeNext;
  auto& libNodeTo  = g_objManager.scnData.m_materialsLibChanges;

  return replace_copy(a_proto, nodeCopyTo, libNodeTo, a_lite);
}

pugi::xml_node HRCamera::copy_node(pugi::xml_node a_proto, bool a_lite)
{
  const int32_t id = a_proto.attribute(L"id").as_int();
  if (id == -1)
    return pugi::xml_node();

  auto& nodeCopyTo = g_objManager.scnData.cameras[id].m_xmlNodeNext;
  auto& libNodeTo  = g_objManager.scnData.m_cameraLibChanges;

  return replace_copy(a_proto, nodeCopyTo, libNodeTo, a_lite);
}

pugi::xml_node HRTextureNode::copy_node(pugi::xml_node a_proto, bool a_lite)
{
  const int32_t id = a_proto.attribute(L"id").as_int();
  if (id == -1)
    return pugi::xml_node();

  auto& nodeCopyTo = g_objManager.scnData.textures[id].m_xmlNodeNext;
  auto& libNodeTo  = g_objManager.scnData.m_texturesLibChanges;

  return replace_copy(a_proto, nodeCopyTo, libNodeTo, a_lite);
}

pugi::xml_node HRSceneData::copy_node(pugi::xml_node a_node, bool a_lite)
{
  return a_node;
}

pugi::xml_node HRSceneInst::copy_node(pugi::xml_node a_proto, bool a_lite)
{
  const int32_t id = a_proto.attribute(L"id").as_int();
  if (id == -1)
    return pugi::xml_node();
  
  auto& nodeCopyTo = g_objManager.scnInst[id].m_xmlNodeNext;
  auto& libNodeTo  = g_objManager.scnData.m_sceneNodeChanges;
  
  return replace_copy(a_proto, nodeCopyTo, libNodeTo, a_lite);
}

pugi::xml_node HRRender::copy_node(pugi::xml_node a_proto, bool a_lite)
{
  const int32_t id = a_proto.attribute(L"id").as_int();
  if (id == -1)
    return pugi::xml_node();

  auto& nodeCopyTo = g_objManager.renderSettings[id].m_xmlNodeNext;
  auto& libNodeTo  = g_objManager.scnData.m_settingsNodeChanges;

  return replace_copy(a_proto, nodeCopyTo, libNodeTo, a_lite);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

pugi::xml_node HRMesh::copy_node_back(pugi::xml_node a_proto)
{
  const int32_t id = a_proto.attribute(L"id").as_int();
  if (id == -1)
    return pugi::xml_node();

  auto& nodeCopyTo = g_objManager.scnData.meshes[id].m_xmlNode;
  auto& libNodeTo  = g_objManager.scnData.m_geometryLib;

  return replace_copy(a_proto, nodeCopyTo, libNodeTo, false);
}

pugi::xml_node HRLight::copy_node_back(pugi::xml_node a_proto)
{
  const int32_t id = a_proto.attribute(L"id").as_int();
  if (id == -1)
    return pugi::xml_node();

  auto& nodeCopyTo = g_objManager.scnData.lights[id].m_xmlNode;
  auto& libNodeTo  = g_objManager.scnData.m_lightsLib;

  return replace_copy(a_proto, nodeCopyTo, libNodeTo, false);
}

pugi::xml_node HRMaterial::copy_node_back(pugi::xml_node a_proto)
{
  const int32_t id = a_proto.attribute(L"id").as_int();
  if (id == -1)
    return pugi::xml_node();

  auto& nodeCopyTo = g_objManager.scnData.materials[id].m_xmlNode;
  auto& libNodeTo  = g_objManager.scnData.m_materialsLib;

  return replace_copy(a_proto, nodeCopyTo, libNodeTo, false);
}

pugi::xml_node HRCamera::copy_node_back(pugi::xml_node a_proto)
{
  const int32_t id = a_proto.attribute(L"id").as_int();
  if (id == -1)
    return pugi::xml_node();

  auto& nodeCopyTo = g_objManager.scnData.cameras[id].m_xmlNode;
  auto& libNodeTo  = g_objManager.scnData.m_cameraLib;

  return replace_copy(a_proto, nodeCopyTo, libNodeTo, false);
}

pugi::xml_node HRTextureNode::copy_node_back(pugi::xml_node a_proto)
{
  const int32_t id = a_proto.attribute(L"id").as_int();
  if (id == -1)
    return pugi::xml_node();

  auto& nodeCopyTo = g_objManager.scnData.textures[id].m_xmlNode;
  auto& libNodeTo  = g_objManager.scnData.m_texturesLib;

  return replace_copy(a_proto, nodeCopyTo, libNodeTo, false);
}

pugi::xml_node HRSceneData::copy_node_back(pugi::xml_node a_node)
{
  return a_node;
}


pugi::xml_node HRSceneInst::copy_node_back(pugi::xml_node a_proto)
{
  const int32_t id = a_proto.attribute(L"id").as_int();
  if (id == -1)
    return pugi::xml_node();

  auto& nodeCopyTo = g_objManager.scnInst[id].m_xmlNode;
  auto& libNodeTo = g_objManager.scnData.m_sceneNode;

  return replace_copy(a_proto, nodeCopyTo, libNodeTo, false);
}


inline pugi::xml_node lite_copy_node_to(pugi::xml_node a_proto, pugi::xml_node libNodeTo, pugi::xml_node& nodeCopyTo)
{
  pugi::xml_node resNode;

  if (nodeCopyTo != nullptr)
  {
    resNode = libNodeTo.insert_copy_after(a_proto, nodeCopyTo); //#TODO implement lite version of insert_copy_after
    libNodeTo.remove_child(nodeCopyTo);
  }
  else
    resNode = append_copy_lite(libNodeTo, a_proto);

  return resNode;
}

inline pugi::xml_node copy_node_to(pugi::xml_node a_proto, pugi::xml_node libNodeTo, pugi::xml_node& nodeCopyTo)
{
  pugi::xml_node resNode;

  if (nodeCopyTo != nullptr)
  {
    resNode = libNodeTo.insert_copy_after(a_proto, nodeCopyTo); //#TODO implement lite version of insert_copy_after
    libNodeTo.remove_child(nodeCopyTo);
  }
  else
    resNode = libNodeTo.append_copy(a_proto);

  return resNode;
}

pugi::xml_node HRSceneInst::append_instances_back(pugi::xml_node a_node)
{
  const wchar_t* sceneId     = a_node.attribute(L"id").value();
  pugi::xml_node sceneToCopy = g_objManager.scnData.m_sceneNode.find_child_by_attribute(L"id", sceneId);

  if (sceneToCopy == nullptr)
    sceneToCopy = g_objManager.scnData.m_sceneNode.append_copy(a_node);

  // #TODO: optimize this with special two-list scanning algorithm
  //
  std::unordered_map<std::wstring, pugi::xml_node> nodeByIdAndType;
  for (pugi::xml_node inst = sceneToCopy.first_child(); inst != nullptr; inst = inst.next_sibling())
  {
    std::wstringstream ss;
    ss << inst.attribute(L"id").as_int() << inst.name();
    nodeByIdAndType[ss.str()] = inst;
  }

  for (pugi::xml_node inst = a_node.first_child(); inst != nullptr; inst = inst.next_sibling())
  {
    std::wstringstream ss;
    ss << inst.attribute(L"id").as_int() << inst.name();
    pugi::xml_node& nodeCopyTo = nodeByIdAndType[ss.str()];
    //lite_copy_node_to(inst, sceneToCopy, nodeCopyTo);
    copy_node_to(inst, sceneToCopy, nodeCopyTo);
  }

  return sceneToCopy; 
}


pugi::xml_node HRRender::copy_node_back(pugi::xml_node a_proto)
{
  const int32_t id = a_proto.attribute(L"id").as_int();
  if (id == -1)
    return pugi::xml_node();

  auto& nodeCopyTo = g_objManager.renderSettings[id].m_xmlNode;
  auto& libNodeTo  = g_objManager.scnData.m_settingsNode;

  return replace_copy(a_proto, nodeCopyTo, libNodeTo, false);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void HRSceneData::init(bool a_attachMode, HRSystemMutex* a_pVBSysMutexLock)
{
  m_texturesLib  = m_xmlDoc.append_child(L"textures_lib");
  m_materialsLib = m_xmlDoc.append_child(L"materials_lib");
  m_lightsLib    = m_xmlDoc.append_child(L"lights_lib");
  m_cameraLib    = m_xmlDoc.append_child(L"cam_lib");
  m_geometryLib  = m_xmlDoc.append_child(L"geometry_lib");
  m_settingsNode = m_xmlDoc.append_child(L"render_lib");
  m_sceneNode    = m_xmlDoc.append_child(L"scenes");

  m_texturesLibChanges  = m_xmlDocChanges.append_child(L"textures_lib");
  m_materialsLibChanges = m_xmlDocChanges.append_child(L"materials_lib");
  m_lightsLibChanges    = m_xmlDocChanges.append_child(L"lights_lib");
  m_cameraLibChanges    = m_xmlDocChanges.append_child(L"cam_lib");
  m_geometryLibChanges  = m_xmlDocChanges.append_child(L"geometry_lib");
  m_settingsNodeChanges = m_xmlDocChanges.append_child(L"render_lib");
  m_sceneNodeChanges    = m_xmlDocChanges.append_child(L"scenes");

  m_trashNode = m_xmlDocChanges.append_child(L"trash");
  
  if(!a_attachMode)                                       // will do this init later inside HRSceneData::init_existing when open scene
    init_virtual_buffer(false, a_pVBSysMutexLock);

  m_initIsDone = true;
}

void HRSceneData::init_existing(bool a_attachMode, HRSystemMutex* a_pVBSysMutexLock)
{
  m_texturesLib  = m_xmlDoc.child(L"textures_lib");
  m_materialsLib = m_xmlDoc.child(L"materials_lib");
  m_lightsLib    = m_xmlDoc.child(L"lights_lib");
  m_cameraLib    = m_xmlDoc.child(L"cam_lib");
  m_geometryLib  = m_xmlDoc.child(L"geometry_lib");
  m_settingsNode = m_xmlDoc.child(L"render_lib");
  m_sceneNode    = m_xmlDoc.child(L"scenes");

  m_texturesLibChanges  = m_xmlDocChanges.child(L"textures_lib");
  m_materialsLibChanges = m_xmlDocChanges.child(L"materials_lib");
  m_lightsLibChanges    = m_xmlDocChanges.child(L"lights_lib");
  m_cameraLibChanges    = m_xmlDocChanges.child(L"cam_lib");
  m_geometryLibChanges  = m_xmlDocChanges.child(L"geometry_lib");
  m_settingsNodeChanges = m_xmlDocChanges.child(L"render_lib");
  m_sceneNodeChanges    = m_xmlDocChanges.child(L"scenes");

  m_trashNode = m_xmlDocChanges.child(L"trash");
  
  init_virtual_buffer(a_attachMode, a_pVBSysMutexLock);

  m_initIsDone = true;
}

void HRSceneData::init_virtual_buffer(bool a_attachMode, HRSystemMutex* a_pVBSysMutexLock)
{
  if (a_attachMode)
  {
    if(SharedVirtualBufferIsEnabled())
    {
      bool attached = m_vbCache.Attach(VIRTUAL_BUFFER_SIZE, "HYDRAAPISHMEM2", &g_objManager.m_tempBuffer);
      if (attached)
        m_vbCache.RestoreChunks();
      else
        m_vbCache.Init(4096, "NOSUCHSHMEM", &g_objManager.m_tempBuffer, a_pVBSysMutexLock); // if fail, init single page only, dummy virtual buffer
    }
    else
      m_vbCache.Init(4096, "NOSUCHSHMEM", &g_objManager.m_tempBuffer, a_pVBSysMutexLock);
  }
  else
    m_vbCache.Init(VIRTUAL_BUFFER_SIZE, "HYDRAAPISHMEM2", &g_objManager.m_tempBuffer, a_pVBSysMutexLock);
}

void HRSceneData::clear()
{
  if (textures.size() == 0 && meshes.size() == 0 && materials.size() == 0 && cameras.size() == 0 && lights.size() == 0 && m_commitId == 0 && !m_initIsDone) // was cleared before
    return;
      
  //std::cout << std::endl;
  //std::cout << "textures.size()  = " << textures.size()  << std::endl;
  //std::cout << "meshes.size()    = " << meshes.size()    << std::endl;
  //std::cout << "materials.size() = " << materials.size() << std::endl;
  //std::cout << "cameras.size()   = " << cameras.size()   << std::endl;
  //std::cout << "lights.size()    = " << lights.size()    << std::endl;
  //std::cout << "m_commitId       = " << m_commitId       << std::endl;      

  meshes.clear();
  lights.clear();
  materials.clear();
  cameras.clear();
  textures.clear();

  clear_node(m_texturesLib);  
  clear_node(m_materialsLib);
  clear_node(m_lightsLib);
  clear_node(m_cameraLib);
  clear_node(m_geometryLib);
  clear_node(m_settingsNode);
  clear_node(m_sceneNode);

  clear_node(m_texturesLibChanges);
  clear_node(m_materialsLibChanges);
  clear_node(m_lightsLibChanges);
  clear_node(m_cameraLibChanges);
  clear_node(m_geometryLibChanges);
  clear_node(m_settingsNodeChanges);
  clear_node(m_sceneNodeChanges);

  m_commitId = 0;
  m_vbCache.Clear();
  m_textureCache.clear();
  m_iesCache.clear();

  m_materialToMeshDependency.clear();
  m_shadowCatchers.clear();

  m_initIsDone = false;
}

void HRSceneData::clear_changes()
{
  for(size_t i=0;i<textures.size();i++)
    textures[i].update_next(pugi::xml_node());
  clear_node_childs(m_texturesLibChanges);
  
  for(size_t i=0;i<materials.size();i++)
    materials[i].update_next(pugi::xml_node());
  clear_node_childs(m_materialsLibChanges);
  
  for(size_t i=0;i<lights.size();i++)
    lights[i].update_next(pugi::xml_node());
  clear_node_childs(m_lightsLibChanges);
  
  for(size_t i=0;i<cameras.size();i++)
    cameras[i].update_next(pugi::xml_node());
  clear_node_childs(m_cameraLibChanges);
  
  for(size_t i=0;i<meshes.size();i++)
    meshes[i].update_next(pugi::xml_node());
  clear_node_childs(m_geometryLibChanges);
  
  //clear_node_childs(m_settingsNodeChanges);
  //clear_node_childs(m_sceneNodeChanges);
}
