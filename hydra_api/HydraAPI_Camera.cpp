#include "HydraAPI.h"
#include "HydraInternal.h"

#include <memory>
#include <vector>
#include <string>
#include <map>

#include <sstream>
#include <iomanip>

#include "HydraObjectManager.h"

extern std::wstring      g_lastError;
extern std::wstring      g_lastErrorCallerPlace;
extern HRObjectManager   g_objManager;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HAPI HRCameraRef hrCameraCreate(const wchar_t* a_objectName)
{
  HRCamera cam;
  cam.name = std::wstring(a_objectName);
  g_objManager.scnData.cameras.push_back(cam);

  HRCameraRef ref;
  ref.id = HR_IDType(g_objManager.scnData.cameras.size() - 1);

  pugi::xml_node nodeXml = g_objManager.camera_lib_append_child();

	nodeXml.append_attribute(L"id").set_value(ref.id);
  nodeXml.append_attribute(L"name").set_value(a_objectName);
  nodeXml.append_attribute(L"type").set_value(L"default_cam");

  g_objManager.scnData.cameras[ref.id].update(nodeXml);
  g_objManager.scnData.cameras[ref.id].id = ref.id;

  return ref;
}

HAPI void hrCameraOpen(HRCameraRef a_pCam, HR_OPEN_MODE a_openMode)
{
  /////////////////////////////////////////////////////////////////////////////////////////////////

  HRCamera* pCam = g_objManager.PtrById(a_pCam);
  if (pCam == nullptr)
  {
    HrError(L"hrCameraOpen, nullptr input");
    return;
  }

  if (pCam->opened)
  {
    HrError(L"hrLightOpen, double open material, with id = ", pCam->id);
    return;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////

  pCam->opened   = true;
  pCam->openMode = a_openMode;

  pugi::xml_node nodeXml = pCam->xml_node();

  if (a_openMode == HR_WRITE_DISCARD)
  {
    clear_node_childs(nodeXml);
    nodeXml.attribute(L"name").set_value(pCam->name.c_str());
    nodeXml.attribute(L"type").set_value(L"uvn");
    nodeXml.attribute(L"id").set_value(pCam->id);
  }
  else if (a_openMode == HR_OPEN_EXISTING)
  {

  }
  else if (a_openMode == HR_OPEN_READ_ONLY)
  {
    
  }
  else
  {
    HrError(L"hrLightOpen, bad open mode ");
  }

}

HAPI void hrCameraClose(HRCameraRef a_pCam)
{
  HRCamera* pCam = g_objManager.PtrById(a_pCam);
  if (pCam == nullptr)
  {
    HrError(L"hrCameraClose, nullptr input");
    return;
  }

  if (!pCam->opened)
  {
    HrError(L"hrCameraClose, double close light, with id = ", pCam->id);
    return;
  }

  pCam->opened     = false;
  pCam->wasChanged = true;
  //g_objManager.scnData.m_changeList.cameraChanged.insert(pCam->id);
}

HAPI pugi::xml_node hrCameraParamNode(HRCameraRef a_camRef)
{
  HRCamera* pCam = g_objManager.PtrById(a_camRef);
  if (pCam == nullptr)
  {
    HrError(L"hrCameraParamNode, nullptr input");
    return pugi::xml_node();
  }

  if (!pCam->opened)
  {
    HrError(L"hrCameraParamNode,camera is not opened = ", pCam->id);
    return pugi::xml_node();
  }

  return pCam->xml_node();
}

HAPI HRCameraRef hrFindCameraByName(const wchar_t *a_cameraName)
{
  HRCameraRef camera;

  if(a_cameraName != nullptr)
  {
    for (auto cam : g_objManager.scnData.cameras)
    {
      if (cam.name == std::wstring(a_cameraName))
      {
        camera.id = cam.id;
        break;
      }
    }
  }

  if(camera.id == -1)
  {
    std::wstringstream ss;
    ss << L"hrCameraFindByName: can't find camera \"" << a_cameraName << "\"";
    HrError(ss.str());
  }

  return camera;
}


HAPI HRRenderRef hrFindRenderByTypeName(const wchar_t* a_renderTypeName)
{
  HRRenderRef res;
  res.id = -1;

  if (a_renderTypeName == nullptr)
    return res;

  const std::wstring typeName = std::wstring(a_renderTypeName);

  for (auto rend : g_objManager.renderSettings)
  {
    if (typeName == rend.xml_node().attribute(L"type").as_string())
    {
      res.id = rend.xml_node().attribute(L"id").as_int();
      break;
    }
  }

  return res;
}
