#include "HydraAPI.h"
#include "HydraInternal.h"
#include "HydraRenderDriverAPI.h"

#include <memory>
#include <vector>
#include <string>
#include <map>

#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>

#include "HydraObjectManager.h"

#include "xxhash.h"
#include <clocale>
#include "LiteMath.h"
#include "HydraXMLHelpers.h"

#include "FreeImage.h"

#pragma warning(disable:4996) // Visual Studio " 'wcsncpy': This function or variable may be unsafe. Consider using wcsncpy_s instead."

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern std::wstring      g_lastError;
extern std::wstring      g_lastErrorCallerPlace;
extern HR_INFO_CALLBACK  g_pInfoCallback;
extern HRObjectManager   g_objManager;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring ToWString(uint64_t i)
{
  std::wstringstream out;
  out << i;
  return out.str();
}

std::wstring ToWString(int64_t i)
{
  std::wstringstream out;
  out << i;
  return out.str();
}

std::wstring ToWString(int i)
{
  std::wstringstream out;
  out << i;
  return out.str();
}

std::wstring ToWString(float i)
{
  std::wstringstream out;
  out << i;
  return out.str();
}

std::wstring ToWString(unsigned int i)
{
  std::wstringstream out;
  out << i;
  return out.str();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HAPI void _hrInit(HRInitInfo a_initInfo)
{
  FreeImage_Initialise(true);
  g_objManager.destroy();
  g_objManager.init(a_initInfo);
  setlocale(LC_ALL, "C");
  registerBuiltInRenderDrivers();
}

HAPI void hrSceneLibraryClose()
{
  for (size_t i = 0; i < g_objManager.renderSettings.size(); i++)
  {
    HRRenderRef render;
    render.id = int32_t(i);
    hrRenderCommand(render, L"exitnow");
  }

  g_objManager.destroy();
}

HAPI const wchar_t* hrGetLastError() // if null else see msg
{
  if (g_lastError == L"")
    return nullptr;
  else
    return g_lastError.c_str();
}

HAPI void hrInfoCallback(HR_INFO_CALLBACK pCallback)
{
  g_pInfoCallback = pCallback;
  if (g_objManager.m_pDriver != nullptr)
    g_objManager.m_pDriver->SetInfoCallBack(pCallback);
}


HAPI void hrErrorCallerPlace(const wchar_t* a_placeName, int a_line)
{
  if (a_placeName == nullptr)
    g_lastErrorCallerPlace = L"";
  else
  {
    std::wstringstream strOut;
    if (a_line != 0)
      strOut << L", line " << a_line;
    g_lastErrorCallerPlace = a_placeName + strOut.str();
  }
}

int32_t _hrSceneLibraryLoad(const wchar_t* a_libPath, int a_stateId, const std::wstring& a_stateFileName);

HAPI int32_t hrSceneLibraryOpen(const wchar_t* a_libPath, HR_OPEN_MODE a_openMode, HRInitInfo a_initInfo)
{
  //hrSceneLibraryClose() ?
  _hrInit(a_initInfo);

  std::wstring input(a_libPath);
  
  int libStateId   = -1;
  std::wstring libFile, libFolder;
  if(input.substr(input.size() - 4) == L".xml")
  {
    // split path to file to (path to folder, file name)
    auto pos  = input.find_last_of(L"/");
    libFolder = input.substr(0, pos);
    libFile   = input.substr(pos+1, input.size());
    
    auto posOfNumber = libFile.find(L"_");
    auto numberStr   = libFile.substr(posOfNumber+1,5); //#TODO: implement more smart number parsing
    std::wstringstream strIn(numberStr);
    strIn >> libStateId;
    
    g_objManager.scnData.m_pathState = input;
    g_objManager.scnData.m_fileState = libFile;
    g_objManager.scnData.m_libFolder = libFolder;
    input = libFolder;
  }
  
  a_libPath = input.c_str();
  
  g_objManager.scnData.opened   = true;
  g_objManager.scnData.openMode = a_openMode;
  g_objManager.scnData.m_path   = a_libPath;

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
  std::wstring s1(a_libPath);
  std::string libPath(s1.begin(), s1.end());
  std::string dataPath = libPath + "/data";
#endif

  if (std::wstring(a_libPath) != L"" && a_openMode == HR_WRITE_DISCARD)
  {

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
    hr_mkdir(libPath.c_str());
    hr_mkdir(dataPath.c_str());
#elif defined WIN32
    std::wstring dataPath = std::wstring(a_libPath) + L"/data";
    hr_cleardir(a_libPath);
    hr_mkdir(a_libPath);
    hr_mkdir(dataPath.c_str());
#endif

  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  for (size_t i = 0; i < g_objManager.scnInst.size(); i++) // #TODO: do we have to clear them manually?
    g_objManager.scnInst[i].clear();
  g_objManager.scnInst.clear();

  for (size_t i = 0; i < g_objManager.renderSettings.size(); i++)
    g_objManager.renderSettings[i].clear();
  g_objManager.renderSettings.clear();

  g_objManager.scnData.clear();
  g_objManager.m_tempBuffer = std::vector<int>();

  if (g_objManager.m_pDriver != nullptr)
  {
    g_objManager.m_pDriver->ClearAll();
    g_objManager.m_pDriver = nullptr;
  }
  g_objManager.m_currSceneId = 0;

  g_objManager.driverAllocated.clear();

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  if (a_openMode == HR_WRITE_DISCARD) // total clear of all objects
  {
    if (a_libPath != nullptr && !std::wstring(a_libPath).empty())
    {
      std::wstring dataPath = std::wstring(a_libPath) + L"/data";
     
    #if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
      hr_cleardir(libPath.c_str());
      std::string dataPath2 = ws2s(a_libPath) + "/data";
      hr_mkdir(dataPath2.c_str());
    #elif defined WIN32
      hr_cleardir(a_libPath);
      hr_cleardir(dataPath.c_str());
    #endif
    }
    int32_t whileImage[4] = { int32_t(0xFFFFFFFF), int32_t(0xFFFFFFFF),
                              int32_t(0xFFFFFFFF), int32_t(0xFFFFFFFF) };

    hrTexture2DCreateFromMemory(2, 2, 4, whileImage); // dummy white texture

  }
  else if (a_openMode == HR_OPEN_EXISTING || a_openMode == HR_OPEN_READ_ONLY)
  {
    return _hrSceneLibraryLoad(a_libPath, libStateId, libFile);
  }
  else
  {
    HrError(L"[hrSceneLibraryOpen]: bad a_openMode = ", a_openMode);
    return 0;
  }

  HrPrint(HR_SEVERITY_DEBUG, L"[hrSceneLibraryOpen]: success");
  return 1;
}


HAPI HRSceneLibraryInfo hrSceneLibraryInfo()
{
  HRSceneLibraryInfo result;

  result.texturesNum      = int32_t(g_objManager.scnData.textures.size());
  result.materialsNum     = int32_t(g_objManager.scnData.materials.size());
  result.meshesNum        = int32_t(g_objManager.scnData.meshes.size());
                          
  result.camerasNum       = int32_t(g_objManager.scnData.cameras.size());
  result.scenesNum        = int32_t(g_objManager.scnInst.size());
  result.renderDriversNum = int32_t(g_objManager.renderSettings.size());

  return result;
}

void _hrFindTargetOrLastState(const wchar_t* a_libPath, int32_t a_stateId,
                              std::wstring& fileName, int& stateId);
std::wstring s2ws(const std::string& s);

HAPI HRSceneLibraryFileInfo hrSceneLibraryExists(const wchar_t* a_libPath, wchar_t a_quickResponce[256])
{
  HRSceneLibraryFileInfo info;

  std::wstring fileName = L"";
  int stateId = 0;

  _hrFindTargetOrLastState(a_libPath, -1,
                           fileName, stateId);

  if (fileName == L"")
  {
    info.empty  = true;
    info.valid  = false;
    info.exists = false;
    return info;
  }

  pugi::xml_document xmlDoc;

  auto loadResult = xmlDoc.load_file(fileName.c_str());
  if (!loadResult)
  {
    std::string  str(loadResult.description());
    std::wstring errorMsg (str.begin(), str.end());
    info.empty  = true;
    info.valid  = false;
    info.exists = true;
    if(a_quickResponce != nullptr)
      wcsncpy(a_quickResponce, errorMsg.c_str(), 256);
    return info;
  }

  auto texturesLib  = xmlDoc.child(L"textures_lib");
  auto materialsLib = xmlDoc.child(L"materials_lib");
  auto geometryLib  = xmlDoc.child(L"geometry_lib");
  auto lightsLib    = xmlDoc.child(L"lights_lib");
  
  auto cameraLib    = xmlDoc.child(L"cam_lib");
  auto settingsNode = xmlDoc.child(L"render_lib");
  auto sceneNode    = xmlDoc.child(L"scenes");

  info.empty  = false;
  info.valid  = true;
  info.exists = true;
  if (a_quickResponce != nullptr)
    wcsncpy(a_quickResponce, L"", 256);

  if (texturesLib == nullptr || materialsLib == nullptr || lightsLib == nullptr || cameraLib == nullptr || geometryLib == nullptr || settingsNode == nullptr || sceneNode == nullptr)
  {
    wcsncpy(a_quickResponce, L"nodn't have one of (textures_lib, materials_lib, lights_lib, cam_lib, geometry_lib, render_lib, scenes)", 256);
    info.valid = false;
  }

  if(stateId == 0)
    info.empty = true;

  if(sceneNode.first_child() == nullptr || geometryLib.first_child() == nullptr || materialsLib.first_child() == nullptr)
    info.empty = true;

  if(info.empty)
    wcsncpy(a_quickResponce, L"empty library", 256);

  info.lastStateId = stateId;

  return info;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Final Scene 
//
HAPI HRSceneInstRef hrSceneCreate(const wchar_t* a_objectName) 
{ 
  g_objManager.scnInst.push_back(HRSceneInst());
  HRSceneInstRef ref;
  ref.id = HR_IDType(g_objManager.scnInst.size() - 1);

  pugi::xml_node nodeXml = g_objManager.scenes_node_append_child();
  std::wstring idStr     = ToWString(ref.id);
  
	nodeXml.append_attribute(L"id").set_value(idStr.c_str());
  nodeXml.append_attribute(L"name").set_value(a_objectName);
  nodeXml.append_attribute(L"discard").set_value(L"1");
  
  g_objManager.scnInst[ref.id].update(nodeXml);
  g_objManager.scnInst[ref.id].id = ref.id;

  return ref; 
}

HAPI void hrSceneOpen(HRSceneInstRef a_pScn, HR_OPEN_MODE a_mode)
{
  HRSceneInst* pScn = g_objManager.PtrById(a_pScn);
  
  if (pScn == nullptr)
  {
    HrError(L"hrSceneOpen: nullptr input");
    return;
  }

  if (pScn->opened)
  {
    HrError(L"hrSceneOpen: scene is opened twice");
    return;
  }

  pugi::xml_node sceneNode = pScn->xml_node();

  if (a_mode == HR_WRITE_DISCARD)
  {
    pScn->drawBeginLight    = 0;
    pScn->drawBegin         = 0;
    pScn->lightGroupCounter = 0;
    pScn->instancedScenesCounter = 0;

    pScn->drawList.clear();
    pScn->drawListLights.clear();
    pScn->drawLightsCustom.clear();

    sceneNode.attribute(L"discard").set_value(L"1");
    clear_node_childs(sceneNode);                       // #NOTE: strange bug in 3ds max plugin; for some reason this node is not empty

    pScn->m_remapList.clear();
    pScn->m_remapCache.clear();        
  }
  else if (a_mode == HR_OPEN_EXISTING)
  {
    sceneNode.attribute(L"discard").set_value(L"0");
  }

  pScn->m_pTmpXlmDoc = std::make_shared<pugi::xml_document>();
  pScn->drawBegin    = pScn->drawList.size();
  pScn->opened       = true;
  pScn->openMode     = a_mode;
}

HAPI pugi::xml_node hrSceneParamNode(HRSceneInstRef a_pScn)
{
  HRSceneInst* pScn = g_objManager.PtrById(a_pScn);
  if (pScn == nullptr)
  {
    HrError(L"hrSceneParamNode: nullptr input");
    return pugi::xml_node();
  }

  if (!pScn->opened)
  {
    HrError(L"hrSceneParamNode: scene is not opened yet!");
    return pugi::xml_node();
  }

  return pScn->xml_node(); // pScn->_xml_node_curr();  //
}

HAPI void hrSceneClose(HRSceneInstRef a_pScn)
{
  HRSceneInst* pScn = g_objManager.PtrById(a_pScn);
  if (pScn == nullptr)
  {
    HrError(L"hrSceneClose: nullptr input");
    return;
  }

  if (!pScn->opened)
  {
    HrError(L"hrSceneClose: scene is not opened yet!");
    return;
  }

  pugi::xml_node sceneNode = pScn->xml_node();

  HydraXMLHelpers::WriteBBox(sceneNode, pScn->m_bbox);

  //// add remap list
  //
  if (!pScn->m_remapCache.empty())
  {

    pugi::xml_node allLists = sceneNode.force_child(L"remap_lists");
    clear_node_childs(allLists);
    for (int id = 0; id < pScn->m_remapList.size(); id++)
    {
      const auto& remapList = pScn->m_remapList[id];

      std::wstringstream strOut;
      for (size_t i = 0; i < remapList.size(); i++)
        strOut << remapList[i] << " ";
      const std::wstring finalStr = strOut.str();

      pugi::xml_node rlist = allLists.append_child(L"remap_list");

      rlist.append_attribute(L"id")   = id;
      rlist.append_attribute(L"size") = int32_t(remapList.size());
      rlist.append_attribute(L"val")  = finalStr.c_str();
    }
  }

  //// add all instances to xml
  //
  for (size_t i = pScn->drawBegin; i < pScn->drawList.size(); i++)
  {
    pugi::xml_node nodeXML = sceneNode.append_child(L"instance");
    
    auto& elem = pScn->drawList[i];

    std::wstring id      = ToWString(i);
    std::wstring mod_id  = ToWString(elem.meshId);
    std::wstring mat_id  = ToWString(elem.remapListId);
    std::wstring scn_id  = ToWString(elem.scene_id);
    std::wstring scn_sid = ToWString(elem.scene_sid);

    std::wstringstream outMat;
    for (int j = 0; j < 16;j++)
      outMat << elem.m[j] << L" ";

    std::wstring mstr = outMat.str();

    nodeXML.append_attribute(L"id")      = id.c_str();
    nodeXML.append_attribute(L"mesh_id") = mod_id.c_str();
    nodeXML.append_attribute(L"rmap_id") = mat_id.c_str();
    nodeXML.append_attribute(L"scn_id")  = scn_id.c_str();
    nodeXML.append_attribute(L"scn_sid") = scn_sid.c_str();
    nodeXML.append_attribute(L"matrix")  = mstr.c_str();

    if(elem.is_moving)
    {
      std::wstringstream outMat;
      for (int j = 0; j < 16;j++)
        outMat << elem.m_end[j] << L" ";

      std::wstring mstr = outMat.str();
      auto motionNode = nodeXML.append_child(L"motion");
      motionNode.append_attribute(L"matrix").set_value(mstr.c_str());
    }

    nodeXML.append_copy(elem.node);
  }

  // lights
  //
  for (size_t i = pScn->drawBeginLight; i < pScn->drawListLights.size(); i++)
  {
    pugi::xml_node nodeXML = sceneNode.append_child(L"instance_light");
    
    auto& elem = pScn->drawListLights[i];

    std::wstring id     = ToWString(i);
    std::wstring mod_id = ToWString(elem.lightId);
    std::wstring lgi_id = ToWString(elem.lightGroupInstId);

    std::wstringstream outMat;
    for (int j = 0; j < 16;j++)
      outMat << elem.m[j] << L" ";

    std::wstring mstr = outMat.str();

    nodeXML.append_attribute(L"id")        = id.c_str();
    nodeXML.append_attribute(L"light_id")  = mod_id.c_str();
    nodeXML.append_attribute(L"matrix")    = mstr.c_str();
    nodeXML.append_attribute(L"lgroup_id") = lgi_id.c_str();
    
    nodeXML.append_copy(elem.node);

    if (i < pScn->drawLightsCustom.size())
    {
      const std::wstring& customAttribs = pScn->drawLightsCustom[i];
      
      if (customAttribs != L"" && !HydraXMLHelpers::StringHasBadSymbols(customAttribs))
      {
        const std::wstring tempNodeStr = L"<temp " + customAttribs + L">";
        pugi::xml_document doc;
        doc.load_string(tempNodeStr.c_str());
        pugi::xml_node tempNode = doc.first_child();

        for (auto attrib : tempNode.attributes())
          nodeXML.append_attribute(attrib.name()) = attrib.value();
      }
      else if (customAttribs != L"")
        HrError(L"hrSceneClose: bad custom attribute string for light instance with id = ", id);
    }

    pScn->drawListLights[i].node = nodeXML; // store reference to instance xml node.
  }

  pScn->drawBegin       = pScn->drawList.size();
  pScn->drawBeginLight  = pScn->drawListLights.size();
  pScn->opened          = false;
  pScn->driverDirtyFlag = true;

  pScn->m_pTmpXlmDoc.reset();
  pScn->m_pTmpXlmDoc = nullptr;
}


HAPI int hrMeshInstance(HRSceneInstRef a_pScn, HRMeshRef a_pMesh, 
                         float a_mat[16], const int32_t* a_mmListm, int32_t a_mmListSize)
{
  HRSceneInst* pScn = g_objManager.PtrById(a_pScn);
  HRMesh*     pMesh = g_objManager.PtrById(a_pMesh);

  if (pScn == nullptr)
  {
    HrError(L"hrMeshInstance: nullptr input");
    return 0;
  }

  if (!pScn->opened)
  {
    HrError(L"hrMeshInstance: scene is not opened");
    return 0;
  }

  if (a_pMesh.id == -1 || pMesh == nullptr)
  {
    HrError(L"hrMeshInstance: mesh with id == -1");
    return -1;
  }

  if(pMesh->m_empty)
  {
    HrPrint(HR_SEVERITY_WARNING, L"hrMeshInstance: empty mesh with id = ", a_pMesh.id);
    return -1;
  }


  int32_t mmId = -1;

  if (a_mmListm != nullptr && a_mmListSize > 0 && a_mmListSize%2 == 0) // create new material remap list
  {

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// sort remap list by
    struct Int2
    {
      int32_t from;
      int32_t to;
    };

    std::vector<Int2> rempListSorted(a_mmListSize/2);
    memcpy(&rempListSorted[0], a_mmListm, a_mmListSize * sizeof(int));

    std::sort(rempListSorted.begin(), rempListSorted.end(), [](Int2 a, Int2 b) -> bool { return a.from < b.from; });
    a_mmListm = (const int32_t*)&rempListSorted[0];
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// sort remap list by

    uint64_t hash = XXH64(a_mmListm, a_mmListSize*sizeof(int32_t), 459662034736); // compute xx hash of material list
    
    auto p = pScn->m_remapCache.find(hash);
    if (p == pScn->m_remapCache.end())
    {
      std::vector<int32_t> data(a_mmListm, a_mmListm + a_mmListSize);
      pScn->m_remapList.push_back(data);

      mmId = int32_t(pScn->m_remapList.size()) - 1;
      pScn->m_remapCache[hash] = mmId;
    }
    else
      mmId = p->second;
  }

  HRSceneInst::Instance model;
  model.meshId                                  = a_pMesh.id;
  model.remapListId                             = mmId;                
  memcpy(model.m, a_mat, 16 * sizeof(float));
  model.scene_id                                = a_pScn.id;
  model.scene_sid                               = pScn->instancedScenesCounter;  
  model.node                                    = pScn->m_pTmpXlmDoc->append_child(L"transform_sequence");
  model.node.force_attribute(L"transformation") = L"scale * rotation * position";
  model.node.force_attribute(L"rotation")       = L"Euler in dergees";
  pScn->drawList.push_back(model);
  
  if(g_objManager.m_computeBBoxes && pMesh->pImpl != nullptr)
  {
    auto bbox = pMesh->pImpl->getBBox();
    auto mat  = LiteMath::float4x4(a_mat);
    auto inst_bbox = transformBBox(bbox, mat);
    pScn->m_bbox = mergeBBoxes(pScn->m_bbox, inst_bbox);
  }

  return int(pScn->drawList.size() - 1); // number current instance
}

HAPI int hrMeshInstanceMotion(HRSceneInstRef a_pScn, HRMeshRef a_pMesh, 
                              float a_start[16], float a_end[16], 
                              const int32_t* a_mmListm, int32_t a_mmListSize)
{
  HRSceneInst* pScn = g_objManager.PtrById(a_pScn);
  HRMesh*     pMesh = g_objManager.PtrById(a_pMesh);

  if (pScn == nullptr)
  {
    HrError(L"hrMeshInstance: nullptr input");
    return 0;
  }

  if (!pScn->opened)
  {
    HrError(L"hrMeshInstance: scene is not opened");
    return 0;
  }

  if (a_pMesh.id == -1 || pMesh == nullptr)
  {
    HrError(L"hrMeshInstance: mesh with id == -1");
    return -1;
  }

  if(pMesh->m_empty)
  {
    HrPrint(HR_SEVERITY_WARNING, L"hrMeshInstance: empty mesh with id = ", a_pMesh.id);
    return -1;
  }


  int32_t mmId = -1;

  if (a_mmListm != nullptr && a_mmListSize > 0 && a_mmListSize%2 == 0) // create new material remap list
  {

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// sort remap list by
    struct Int2
    {
      int32_t from;
      int32_t to;
    };

    std::vector<Int2> rempListSorted(a_mmListSize/2);
    memcpy(&rempListSorted[0], a_mmListm, a_mmListSize * sizeof(int));

    std::sort(rempListSorted.begin(), rempListSorted.end(), [](Int2 a, Int2 b) -> bool { return a.from < b.from; });
    a_mmListm = (const int32_t*)&rempListSorted[0];
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// sort remap list by

    uint64_t hash = XXH64(a_mmListm, a_mmListSize*sizeof(int32_t), 459662034736); // compute xx hash of material list
    
    auto p = pScn->m_remapCache.find(hash);
    if (p == pScn->m_remapCache.end())
    {
      std::vector<int32_t> data(a_mmListm, a_mmListm + a_mmListSize);
      pScn->m_remapList.push_back(data);

      mmId = int32_t(pScn->m_remapList.size()) - 1;
      pScn->m_remapCache[hash] = mmId;
    }
    else
      mmId = p->second;
  }

  HRSceneInst::Instance model;
  model.meshId                                  = a_pMesh.id;
  model.remapListId                             = mmId;                
  memcpy(model.m,     a_start, 16 * sizeof(float));
  memcpy(model.m_end, a_end,   16 * sizeof(float));
  model.scene_id                                = a_pScn.id;
  model.scene_sid                               = pScn->instancedScenesCounter;  
  model.node                                    = pScn->m_pTmpXlmDoc->append_child(L"transform_sequence");
  model.node.force_attribute(L"transformation") = L"scale * rotation * position";
  model.node.force_attribute(L"rotation")       = L"Euler in dergees";
  model.is_moving                               = true;
  pScn->drawList.push_back(model);
  
  if(g_objManager.m_computeBBoxes && pMesh->pImpl != nullptr)
  {
    auto bbox = pMesh->pImpl->getBBox();
    auto mat1  = LiteMath::float4x4(model.m);
    auto mat2  = LiteMath::float4x4(model.m_end);
    auto inst_bbox_start = transformBBox(bbox, mat1);
    auto inst_bbox_end = transformBBox(bbox, mat2);
    pScn->m_bbox = mergeBBoxes(pScn->m_bbox, inst_bbox_start);
    pScn->m_bbox = mergeBBoxes(pScn->m_bbox, inst_bbox_end);
  }

  return int(pScn->drawList.size() - 1); // number current instance
}


static int _hrLightInstance(HRSceneInstRef a_pScn, HRLightRef a_pLight, float a_mat[16], int32_t a_lightGroupInstanceId, const wchar_t* a_customAttribs)
{
  HRSceneInst* pScn = g_objManager.PtrById(a_pScn);
  if (pScn == nullptr)
  {
    HrError(L"hrLightInstance: nullptr input");
    return -1;
  }

  if (!pScn->opened)
  {
    HrError(L"hrLightInstance: scene is not opened");
    return -1;
  }

  HRSceneInst::Instance model;
  model.lightId                                 = a_pLight.id;
  model.lightGroupInstId                        = a_lightGroupInstanceId;
  model.meshId                                  = -1;
  model.remapListId                             = -1;
  model.node                                    = pScn->m_pTmpXlmDoc->append_child(L"transform_sequence");
  model.node.force_attribute(L"transformation") = L"scale * rotation * position";
  model.node.force_attribute(L"rotation")       = L"Euler in dergees";
  memcpy(model.m, a_mat, 16 * sizeof(float));

  pScn->drawListLights.push_back(model);
  if (a_customAttribs == nullptr)
    pScn->drawLightsCustom.push_back(L"");
  else
    pScn->drawLightsCustom.push_back(a_customAttribs);

  return int(pScn->drawListLights.size() - 1); // number current instance  
}


HAPI int hrLightInstance(HRSceneInstRef a_pScn, HRLightRef a_pLight, float a_mat[16], const wchar_t* a_customAttribs)
{
  return _hrLightInstance(a_pScn, a_pLight, a_mat, -1, a_customAttribs);
}


HAPI void hrLightGroupInstanceExt(HRSceneInstRef a_pScn, HRLightGroupExt lightGroup, float m[16], const wchar_t** a_customAttribsArray)
{
  if (lightGroup.lightsNum <= 0)
    return;
   
  HRSceneInst* pScn = g_objManager.PtrById(a_pScn);
  if (pScn == nullptr)
  {
    HrError(L"hrLightGroupInstanceExt: nullptr input scene ref");
    return;
  }

  for (int i = 0; i < lightGroup.lightsNum; i++)
  {
    const wchar_t* custPtr    = (a_customAttribsArray == nullptr) ? nullptr : a_customAttribsArray[i];
    LiteMath::float4x4 mFinal = LiteMath::float4x4(m)*LiteMath::float4x4(lightGroup.matrix[i]);

    float rowMajorRes[16];
    mFinal.StoreRowMajor(rowMajorRes);

    _hrLightInstance(a_pScn, lightGroup.lights[i], rowMajorRes, pScn->lightGroupCounter, custPtr);
  }

  pScn->lightGroupCounter++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//IHRRenderDriver* CreateOpenGL1DrawBVH_RenderDriver();   // debug drivers
//IHRRenderDriver* CreateOpenGL1TestSplit_RenderDriver();
//IHRRenderDriver* CreateDebugPrint_RenderDriver();
//IHRRenderDriver* CreateOpenGL1DrawRays_RenderDriver();
//IHRRenderDriver* CreateOpenGL1Debug_TestCustomAttributes();

//std::unique_ptr<IHRRenderDriver> CreateRenderFromString(const wchar_t *a_className, const wchar_t *a_options)
//{
//  if (!wcscmp(a_className, L"opengl1"))
//    return std::unique_ptr<IHRRenderDriver>(CreateOpenGL1_RenderDriver());
//  else if (!wcscmp(a_className, L"opengl1Debug"))
//    return std::unique_ptr<IHRRenderDriver>(CreateOpenGL1Debug_RenderDriver());
//  else if (!wcscmp(a_className, L"opengl1TestSplit"))
//    return std::unique_ptr<IHRRenderDriver>(CreateOpenGL1TestSplit_RenderDriver());
//  else if (!wcscmp(a_className, L"opengl1DelayedLoad"))
//    return std::unique_ptr<IHRRenderDriver>(CreateOpenGL1_DelayedLoad_RenderDriver(false));
//  else if (!wcscmp(a_className, L"opengl1DelayedLoad2"))
//    return std::unique_ptr<IHRRenderDriver>(CreateOpenGL1_DelayedLoad_RenderDriver(true));
//  else if (!wcscmp(a_className, L"opengl1TestCustomAttributes"))
//    return std::unique_ptr<IHRRenderDriver>(CreateOpenGL1Debug_TestCustomAttributes());
//  else if (!wcscmp(a_className, L"opengl1DrawBvh"))
//    return std::unique_ptr<IHRRenderDriver>(CreateOpenGL1DrawBVH_RenderDriver());
//  else if (!wcscmp(a_className, L"opengl1DrawRays"))
//    return std::unique_ptr<IHRRenderDriver>(CreateOpenGL1DrawRays_RenderDriver());
//  else if (!wcscmp(a_className, L"DebugPrint"))
//    return std::unique_ptr<IHRRenderDriver>(CreateDebugPrint_RenderDriver());
//  else if (!wcscmp(a_className, L"opengl32Forward"))
//    return std::unique_ptr<IHRRenderDriver>(CreateOpenGL32Forward_RenderDriver());
//  else if (!wcscmp(a_className, L"opengl32Deferred"))
//    return std::unique_ptr<IHRRenderDriver>(CreateOpenGL32Deferred_RenderDriver());
//  if (!wcscmp(a_className, L"opengl3Utility"))
//    return std::unique_ptr<IHRRenderDriver>(CreateOpenGL3_Utilty_RenderDriver());
//  else if (!wcscmp(a_className, L"vulkan"))
//  {
//    HrPrint(HR_SEVERITY_ERROR, L"Define CreateVulkan_RenderDriver() and uncomment its call in CreateRenderFromString", a_className);
////    return std::unique_ptr<IHRRenderDriver>(CreateVulkan_RenderDriver());
//  }
//  else if (!wcscmp(a_className, L"HydraModern"))
//    return std::unique_ptr<IHRRenderDriver>(CreateHydraConnection_RenderDriver());
//  else
//  {
//    HrPrint(HR_SEVERITY_ERROR, L"CreateRenderFromString, unknown render driver name ", a_className);
//    return nullptr;
//  }
//}

HAPI HRRenderRef hrRenderCreate(const wchar_t* a_className, const wchar_t* a_flags)
{ 
  HRRenderRef ref;
  ref.id = HR_IDType(g_objManager.renderSettings.size());
   
  HRRender settingsTmp;
  settingsTmp.name = a_className;
  g_objManager.renderSettings.push_back(settingsTmp);

  HRRender& settings = g_objManager.renderSettings[ref.id];

  pugi::xml_node nodeXml = g_objManager.settings_lib_append_child();
  
  nodeXml.append_attribute(L"type").set_value(a_className);
  nodeXml.append_attribute(L"id").set_value(ref.id);
  
  g_objManager.renderSettings[ref.id].update(nodeXml); // ???
  g_objManager.renderSettings[ref.id].id = ref.id;
	
  settings.m_pDriver = std::shared_ptr<IHRRenderDriver>(RenderDriverFactory::Create(a_className));

  settings.m_pDriver->SetInfoCallBack(g_pInfoCallback);

  return ref; 
}

HAPI HRRenderRef hrRenderCreateFromExistingDriver(const wchar_t* a_className, std::shared_ptr<IHRRenderDriver> a_pDriver)
{
  HRRenderRef ref;
  ref.id = HR_IDType(g_objManager.renderSettings.size());

  HRRender settingsTmp;
  settingsTmp.name = a_className;
  g_objManager.renderSettings.push_back(settingsTmp);

  HRRender& settings = g_objManager.renderSettings[ref.id];

  pugi::xml_node nodeXml = g_objManager.settings_lib_append_child();

  nodeXml.append_attribute(L"type").set_value(a_className);
  nodeXml.append_attribute(L"id").set_value(ref.id);

  g_objManager.renderSettings[ref.id].update(nodeXml); // ???
  g_objManager.renderSettings[ref.id].id = ref.id;

  settings.m_pDriver = a_pDriver;
  settings.m_pDriver->SetInfoCallBack(g_pInfoCallback);

  return ref;
}


HAPI void hrRenderOpen(HRRenderRef a_pRender, HR_OPEN_MODE a_mode) 
{
  HRRender* pSettings = g_objManager.PtrById(a_pRender);

  if (pSettings == nullptr)
  {
    HrError(L"hrRenderOpen: nullptr input");
    return;
  }

  if (pSettings->opened)
  {
    HrError(L"hrRenderOpen, double open settings, with id = ", pSettings->id);
    return;
  }
  
  if (a_mode == HR_WRITE_DISCARD)
  {
    auto nodeXML = pSettings->xml_node();
    clear_node_childs(nodeXML);
    nodeXML.append_attribute(L"type").set_value(pSettings->name.c_str());
    nodeXML.append_attribute(L"id").set_value(a_pRender.id);
  }
  
  pSettings->opened   = true;
  pSettings->openMode = a_mode;
}

HAPI void hrRenderClose(HRRenderRef a_pRender)
{
  HRRender* pSettings = g_objManager.PtrById(a_pRender);

  if (pSettings == nullptr)
  {
    HrError(L"hrRenderClose: nullptr input");
    return;
  }

  if (!pSettings->opened)
  {
    HrError(L"hrRenderClose, double close settings, with id = ", pSettings->id);
    return;
  }

  pugi::xml_node node = pSettings->xml_node();
  pugi::xml_node tool = node.child(L"maxRaysPerPixel");

  if (tool != nullptr)
    pSettings->maxRaysPerPixel = tool.text().as_int();
  else
    pSettings->maxRaysPerPixel = 1024;

  pSettings->opened = false;
}

HAPI pugi::xml_node hrRenderParamNode(HRRenderRef a_pRender)
{
  HRRender* pSettings = g_objManager.PtrById(a_pRender);

  if (pSettings == nullptr)
  {
    HrError(L"hrRenderParamNode: nullptr input");
    return pugi::xml_node();
  }

  if (!pSettings->opened)
  {
    HrError(L"hrRenderParamNode, is not opened with id = ", pSettings->id);
    return pugi::xml_node();
  }

  return pSettings->xml_node();
}

//const int MAX_DEVICES_NUM = 256;
//HRRenderDeviceInfoListElem g_deviceList[MAX_DEVICES_NUM];

HAPI const HRRenderDeviceInfoListElem* hrRenderGetDeviceList(HRRenderRef a_pRender)
{
  HRRender* pRender = g_objManager.PtrById(a_pRender);

  if (pRender == nullptr)
  {
    HrError(L"hrRenderGetDeviceList: nullptr input");
    return nullptr;
  }

  auto pDriver = pRender->m_pDriver;
  if (pDriver == nullptr)
    return nullptr;

  return pDriver->DeviceList();
}

HAPI void hrRenderEnableDevice(HRRenderRef a_pRender, int32_t a_deviceId, bool a_enable)
{
  HRRender* pRender = g_objManager.PtrById(a_pRender);

  if (pRender == nullptr)
  {
    HrError(L"hrRenderEnableDevice: nullptr input");
    return ;
  }

  auto pDriver = pRender->m_pDriver;
  if (pDriver == nullptr)
    return;

  if (!pDriver->EnableDevice(a_deviceId, a_enable))
  {
    std::wstringstream strOut;
    strOut << L"hrRenderEnableDevice, bad device id" << a_deviceId;
    std::wstring temp = strOut.str();
    HrError(temp);
  }
}

HAPI HRRenderUpdateInfo hrRenderHaveUpdate(HRRenderRef a_pRender)
{
  HRRender* pRender = g_objManager.PtrById(a_pRender);

  HRRenderUpdateInfo result;
  if (pRender == nullptr)
    return result;

  auto pDriver = pRender->m_pDriver;
  if (pDriver == nullptr)
    return result;

  result = pDriver->HaveUpdateNow(pRender->maxRaysPerPixel);
  return result;
}



static pugi::xml_attribute force_attrib(pugi::xml_node a_parent, const wchar_t* a_name) ///< helper function
{
  pugi::xml_attribute attr = a_parent.attribute(a_name);
  if (attr != nullptr)
    return attr;
  else
    return a_parent.append_attribute(a_name);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HR_UpdateLightsGeometryAndMaterial(pugi::xml_node a_lightLib, pugi::xml_node a_sceneInstances);

HAPI void hrDrawPassOnly(HRSceneInstRef a_pScn, HRRenderRef a_pRender, HRCameraRef a_pCam)
{
  HRRender* pSettings = g_objManager.PtrById(a_pRender);
  //HRSceneInst* pScn   = g_objManager.PtrById(a_pScn);
  //HRCamera*    pCam   = g_objManager.PtrById(a_pCam);

  if (a_pRender.id != -1)
  {
    g_objManager.m_currRenderId = a_pRender.id;
    g_objManager.m_pDriver      = pSettings->m_pDriver;
  }

  if (a_pScn.id != -1)
    g_objManager.m_currSceneId = a_pScn.id;

  if (a_pCam.id != -1)
    g_objManager.m_currCamId = a_pCam.id;

  if (g_objManager.scnInst.size() <= g_objManager.m_currSceneId)
  {
    HrError(L"hrDrawPassOnly(no scene instances were loaded), g_objManager.scnInst.size() = ", g_objManager.scnInst.size());
    return;
  }

  HR_DriverDraw(g_objManager.scnInst[g_objManager.m_currSceneId], pSettings);
}

HAPI void hrRenderEvalGbuffer(HRRenderRef a_pRender)
{
  HRRender* pSettings = g_objManager.PtrById(a_pRender);

  if (a_pRender.id != -1 && pSettings != nullptr)
  {
    g_objManager.m_currRenderId = a_pRender.id;
    g_objManager.m_pDriver      = pSettings->m_pDriver;
  }

  if(g_objManager.m_pDriver != nullptr)
    g_objManager.m_pDriver->EvalGBuffer();
}


HAPI IHRRenderDriver* hrRenderDriverPointer(HRRenderRef a_pRender)
{
  HRRender* pSettings = g_objManager.PtrById(a_pRender);
  return pSettings->m_pDriver.get();
}


void _hrSaveCurrentChanges(HRSceneData& a_scnData)
{
  const std::wstring& fileName = g_objManager.m_tempPathToChangeFile;
  
  pugi::xml_document  xmlDoc;
  auto texturesLib  = xmlDoc.append_child(L"textures_lib");
  auto materialsLib = xmlDoc.append_child(L"materials_lib");
  auto geometryLib  = xmlDoc.append_child(L"geometry_lib");
  auto lightsLib    = xmlDoc.append_child(L"lights_lib");
  auto cameraLib    = xmlDoc.append_child(L"cam_lib");
  auto settingsNode = xmlDoc.append_child(L"render_lib");
  auto sceneNode    = xmlDoc.append_child(L"scenes");
  
  for(auto& texture : a_scnData.textures)
  {
    if(texture.wasChanged)
    {
      texturesLib.append_copy(texture.xml_node());
      texture.wasChanged = false;
    }
  }
  
  for(auto& material : a_scnData.materials)
  {
    if(material.wasChanged)
    {
      materialsLib.append_copy(material.xml_node());
      material.wasChanged = false;
    }
  }
  
  for(auto& mesh : a_scnData.meshes)
  {
    if(mesh.wasChanged)
    {
      geometryLib.append_copy(mesh.xml_node());
      mesh.wasChanged = false;
    }
  }
  
  for(auto& light : a_scnData.lights)
  {
    if(light.wasChanged)
    {
      lightsLib.append_copy(light.xml_node());
      light.wasChanged = false;
    }
  }
  
  for(auto& cam : a_scnData.cameras)
  {
    if(cam.wasChanged)
    {
      cameraLib.append_copy(cam.xml_node());
      cam.wasChanged = false;
    }
  }
  
  for(auto& scene : g_objManager.scnInst)
    sceneNode.append_copy(scene.xml_node());
  
  //#TODO: add renderer settings ...
  
  xmlDoc.save_file(fileName.c_str(), L"  ");
}

HAPI void hrCommit(HRSceneInstRef a_pScn, HRRenderRef a_pRender, HRCameraRef a_pCam) ///< non blocking commit, send commands to renderer and return immediately 
{
  HRRender* pSettings = g_objManager.PtrById(a_pRender);
  HRSceneInst* pScn   = g_objManager.PtrById(a_pScn);

  if (pSettings == nullptr)
    HrPrint(HR_SEVERITY_WARNING, L"hrCommit, nullptr render");

  if (pScn == nullptr)
    HrPrint(HR_SEVERITY_WARNING, L"hrCommit, nullptr scene");

  if (a_pRender.id != -1 && pSettings != nullptr)
  {
    g_objManager.m_currRenderId = a_pRender.id;
    g_objManager.m_pDriver      = pSettings->m_pDriver;
  }

  if (a_pScn.id != -1)
    g_objManager.m_currSceneId = a_pScn.id;

  if (a_pCam.id != -1)
    g_objManager.m_currCamId = a_pCam.id;

  // Add/Update light as geometry if Render Driver can't do it itself
  //
  std::wstring driver_name = L"";
  if(g_objManager.m_pDriver != nullptr)
    g_objManager.m_pDriver->GetRenderDriverName(driver_name);
  auto driver_info = RenderDriverFactory::GetDriverInfo(driver_name.c_str());

  bool needToAddLightsAsGeometry = (g_objManager.m_pDriver == nullptr) || !driver_info.createsLightGeometryItself;
  if (needToAddLightsAsGeometry && pScn != nullptr)
    HR_UpdateLightsGeometryAndMaterial(g_objManager.scnData.m_lightsLib, pScn->xml_node());
  
  // we must loop through all scene element to define what mesh, material and light we need to Update on the render driver side
  //
  if (g_objManager.m_pDriver != nullptr && g_objManager.m_currSceneId < g_objManager.scnInst.size())
  {
    g_objManager.m_pDriver->SetInfoCallBack(g_pInfoCallback);
    HR_DriverUpdate(g_objManager.scnInst[g_objManager.m_currSceneId], pSettings);
    HR_DriverDraw  (g_objManager.scnInst[g_objManager.m_currSceneId], pSettings);
  }
  
  if(g_objManager.m_lastInitInfo.saveChanges)
    _hrSaveCurrentChanges(g_objManager.scnData); // save change if needed

  g_objManager.scnData.m_changeList.clear();

  size_t chunks = g_objManager.scnData.m_vbCache.size();
  force_attrib(g_objManager.scnData.m_geometryLib, L"total_chunks").set_value(chunks);
  force_attrib(g_objManager.scnData.m_texturesLib, L"total_chunks").set_value(chunks);
  
  // put chunks addresses in chunk table.
  // note that you don't have to lock mutex due to all data is appended to the end of virtual buffer.
  // due to this new object should not damadge previouse and only garbage collector operation must lock mutex
  //
  int64_t* chunkTable = g_objManager.scnData.m_vbCache.ChunksTablePtr();
  if(chunkTable != nullptr && !g_objManager.m_attachMode)
  {
    const size_t chunksNum = g_objManager.scnData.m_vbCache.size();
    for(size_t i=0;i<chunksNum;i++)
    {
      const auto& chunk = g_objManager.scnData.m_vbCache.chunk_at(i);
      if(chunk.InMemory())
        chunkTable[i] = int64_t(chunk.localAddress);
      else
        chunkTable[i] = int64_t(-1);
    }
  }
  
  if(g_objManager.m_tempBuffer.size() > TEMP_BUFFER_MAX_SIZE_DONT_FREE)
    g_objManager.m_tempBuffer = g_objManager.EmptyBuffer();
}

HAPI void hrFlush(HRSceneInstRef a_pScn, HRRenderRef a_pRender, HRCameraRef a_pCam)  ///< blocking commit, waiting for all current commands to be executed
{
  std::wstringstream outStr1, outStr2, outStr3;

  outStr1 << g_objManager.scnData.m_path.c_str() << L"/statex_" << std::setfill(L"0"[0]) << std::setw(5) << g_objManager.scnData.m_commitId     << L".xml";
  outStr2 << g_objManager.scnData.m_path.c_str() << L"/change_" << std::setfill(L"0"[0]) << std::setw(5) << g_objManager.scnData.m_commitId     << L".xml";
  outStr3 << g_objManager.scnData.m_path.c_str() << L"/statex_" << std::setfill(L"0"[0]) << std::setw(5) << g_objManager.scnData.m_commitId + 1 << L".xml";
  
  std::wstring oldPath = outStr1.str();
  std::wstring cngPath = outStr2.str();
  std::wstring newPath = outStr3.str();

  //g_objManager.scnData.m_xmlDoc.save_file(oldPath.c_str(), L"  ");
  g_objManager.m_tempPathToChangeFile = cngPath; // postpone g_objManager.scnData.m_xmlDocChanges.save_file(cngPath.c_str(), L"  ");

  hrCommit(a_pScn, a_pRender, a_pCam);
  
  g_objManager.scnData.m_commitId++;
  g_objManager.scnData.m_xmlDoc.save_file(newPath.c_str(), L"  ");
  
  HRRender* pSettings = g_objManager.PtrById(a_pRender);
  
  //////////////
  ////////////// Call utility render driver here
  if(g_objManager.m_pDriver != nullptr)
  {
    auto settings = g_objManager.renderSettings[a_pRender.id].xml_node();
    bool doPrepass      = false;
    bool doDisplacement = false;
    if (settings.child(L"scenePrepass") != nullptr)
      doPrepass = settings.child(L"scenePrepass").text().as_bool();
  
    if (settings.child(L"doDisplacement") != nullptr)
      doDisplacement = settings.child(L"doDisplacement").text().as_bool();

    //std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::wstring fixed_state = newPath;
    std::wstring driver_name;
    g_objManager.m_pDriver->GetRenderDriverName(driver_name);
    auto driver_info = RenderDriverFactory::GetDriverInfo(driver_name.c_str());

#if defined(USE_GL) || !defined(HYDRA_API_CMAKE)
    if (driver_info.supportUtilityPrepass && doPrepass)
    {
      std::cout << "Starting scene prepass..." << std::endl;
      fixed_state = HR_UtilityDriverStart(newPath.c_str(), pSettings);
    }
#endif
    //std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    //std::cout << "Resolution fix elapsed time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" <<std::endl;

//#ifdef IN_DEBUG
    if (driver_info.supportDisplacement && doDisplacement)
      fixed_state = HR_PreprocessMeshes(fixed_state.c_str());
//#endif
  }

  //////////////

  g_objManager.scnData.m_vbCache.FlushToDisc();

  if (pSettings != nullptr && pSettings->m_pDriver != nullptr)
    pSettings->m_pDriver->EndFlush();
  
} 

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HRLightGroupExt::HRLightGroupExt(int a_lightNum)
{
  lights     = new HRLightRef[a_lightNum];
  matrix     = new float16[a_lightNum];
  customAttr = new wchar_t*[a_lightNum];
  lightsNum  = a_lightNum;

  memset(customAttr, 0, lightsNum * sizeof(wchar_t*));
  for (int i = 0; i < a_lightNum; i++)
  {
    lights[i] = HRLightRef();

    float matrix2[16] = {1,0,0,0,
                         0,1,0,0,
                         0,0,1,0,
                         0,0,0,1};

    memcpy(matrix[i], matrix2, sizeof(float16));
  }
}

HRLightGroupExt::~HRLightGroupExt()
{
  delete[] lights;     lights     = nullptr;
  delete[] matrix;     matrix     = nullptr;
  delete[] customAttr; customAttr = nullptr;
  lightsNum = 0;
}

HRLightGroupExt::HRLightGroupExt(const HRLightGroupExt& a_in)
{
  lights     = new HRLightRef[a_in.lightsNum];
  matrix     = new float16   [a_in.lightsNum];
  customAttr = new wchar_t*  [a_in.lightsNum];
  lightsNum  = a_in.lightsNum;

  memcpy(lights, a_in.lights, a_in.lightsNum * sizeof(HRLightRef));
  memcpy(matrix, a_in.matrix, a_in.lightsNum * sizeof(float16));
  memcpy(customAttr, a_in.customAttr, a_in.lightsNum * sizeof(wchar_t*));
}

HRLightGroupExt& HRLightGroupExt::operator=(const HRLightGroupExt& a_in)
{
  lights     = new HRLightRef[a_in.lightsNum];
  matrix     = new float16   [a_in.lightsNum];
  customAttr = new wchar_t*[a_in.lightsNum];
  lightsNum  = a_in.lightsNum;

  memcpy(lights, a_in.lights, a_in.lightsNum * sizeof(HRLightRef));
  memcpy(matrix, a_in.matrix, a_in.lightsNum * sizeof(float16));
  memcpy(customAttr, a_in.customAttr, a_in.lightsNum * sizeof(wchar_t*));
  return *this;
}

HRLightGroupExt::HRLightGroupExt(HRLightGroupExt&& a_in)
{
  this->lightsNum  = a_in.lightsNum;
  this->lights     = a_in.lights; 
  this->matrix     = a_in.matrix;
  this->customAttr = a_in.customAttr;

  a_in.lightsNum  = 0;
  a_in.lights     = nullptr;
  a_in.matrix     = nullptr;
}

HRLightGroupExt& HRLightGroupExt::operator=(HRLightGroupExt&& a_in)
{
  this->lightsNum  = a_in.lightsNum;
  this->lights     = a_in.lights;
  this->matrix     = a_in.matrix;
  this->customAttr = a_in.customAttr;

  a_in.lightsNum  = 0;
  a_in.lights     = nullptr;
  a_in.matrix     = nullptr;

  return *this;
}


