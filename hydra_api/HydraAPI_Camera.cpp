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

extern std::wstring      g_lastError;
extern std::wstring      g_lastErrorCallerPlace;
extern HR_ERROR_CALLBACK g_pErrorCallback;
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

  g_objManager.scnData.cameras[ref.id].update_next(nodeXml);
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

  pugi::xml_node nodeXml = pCam->xml_node_next(a_openMode);

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

  pCam->opened = false;
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

  return pCam->xml_node_next();
}
