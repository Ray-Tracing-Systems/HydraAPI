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

HAPI HRMaterialRef hrMaterialCreate(const wchar_t* a_objectName)
{
  HRMaterialRef ref;
  ref.id = HR_IDType(g_objManager.scnData.materials.size());

  std::wstring nameGenerated;
  if (a_objectName == nullptr) // create internal name for material
  {
    std::wstringstream strOut;
    strOut << L"mat#" << ref.id;
    nameGenerated = strOut.str();
    a_objectName = nameGenerated.c_str();
  }

  HRMaterial mat;
  mat.name = std::wstring(a_objectName);
  g_objManager.scnData.materials.push_back(mat);

  pugi::xml_node matNodeXml = g_objManager.materials_lib_append_child();

	matNodeXml.append_attribute(L"id").set_value(ref.id);
  matNodeXml.append_attribute(L"name").set_value(mat.name.c_str());
  matNodeXml.append_attribute(L"type").set_value(L"hydra_material");

  g_objManager.scnData.materials[ref.id].update_next(matNodeXml);

  return ref;
}

HAPI HRMaterialRef hrMaterialCreateBlend(const wchar_t* a_objectName, HRMaterialRef a_pMat1, HRMaterialRef a_pMat2)
{
  HRMaterial*  pMat1 = g_objManager.PtrById(a_pMat1);
  HRMaterial*  pMat2 = g_objManager.PtrById(a_pMat2);

  if (pMat1 == nullptr || pMat2 == nullptr)
  {
    HrError(L"hrMaterialCreateBlend: nullptr input reference");
    HRMaterialRef ref;
    ref.id = -1;
    return ref;
  }

  HRMaterialRef ref;
  ref.id = HR_IDType(g_objManager.scnData.materials.size());

  std::wstring nameGenerated;
  if (a_objectName == nullptr) // create internal name for material
  {
    std::wstringstream strOut;
    strOut << L"mat#" << ref.id;
    nameGenerated = strOut.str();
    a_objectName = nameGenerated.c_str();
  }

  HRMaterial mat; // # create blend
  mat.name = std::wstring(a_objectName);
  g_objManager.scnData.materials.push_back(mat);


  pugi::xml_node matNodeXml = g_objManager.materials_lib_append_child();

	matNodeXml.append_attribute(L"id").set_value(ref.id);
  matNodeXml.append_attribute(L"name").set_value(mat.name.c_str());
  matNodeXml.append_attribute(L"type").set_value(L"hydra_blend");
  matNodeXml.append_attribute(L"node_top").set_value(a_pMat1.id);
  matNodeXml.append_attribute(L"node_bottom").set_value(a_pMat2.id);

  g_objManager.scnData.materials[ref.id].update_next(matNodeXml);

  return ref;
}

HAPI void hrMaterialOpen(HRMaterialRef a_pMat, HR_OPEN_MODE a_openMode)
{
  HRMaterial* pMat = g_objManager.PtrById(a_pMat);

  if (pMat == nullptr)
  {
    HrError(L"hrMaterialOpen: nullptr input");
    return;
  }

  if (pMat->opened)
  {
    HrError(L"hrMaterialOpen, double open material, with id = ", pMat->id);
    return;
  }

  pugi::xml_node nodeXml = pMat->xml_node_next(a_openMode);
  if (a_openMode == HR_WRITE_DISCARD)
  {
    clear_node_childs(nodeXml);
  }

  pMat->opened   = true;
  pMat->openMode = a_openMode;
}

HAPI void hrMaterialClose(HRMaterialRef a_pMat)
{
  HRMaterial* pMat = g_objManager.PtrById(a_pMat);

  if (pMat == nullptr)
  {
    HrError(L"hrMaterialClose: nullptr input");
    return;
  }

  if (!pMat->opened)
  {
    HrError(L"hrMaterialClose, double close material, with id = ", pMat->id);
    return;
  }

  pMat->opened = false;
  pMat->pImpl = nullptr;
}

HAPI pugi::xml_node hrMaterialParamNode(HRMaterialRef a_matRef)
{
  HRMaterial* pMat = g_objManager.PtrById(a_matRef);
  if (pMat == nullptr)
  {
    HrError(L"hrMaterialParamNode, nullptr input ");
    return pugi::xml_node();
  }

  if (!pMat->opened)
  {
    HrError(L"hrMaterialParamNode, light is not opened, light id = ", pMat->id);
    return  pugi::xml_node();
  }

  return pMat->xml_node_next(pMat->openMode);
}


HAPI HRMaterialRef hrMaterialFindByName(const wchar_t *a_matName)
{
  HRMaterialRef material;

  if(a_matName != nullptr)
  {
    for (auto mat : g_objManager.scnData.lights)
    {
      if (mat.name == std::wstring(a_matName))
      {
        material.id = mat.id;
        break;
      }
    }
  }

  if(material.id == -1)
  {
    std::wstringstream ss;
    ss << L"hrMaterialFindByName: can't find material \"" << a_matName << "\"";
    HrError(ss.str());
  }

  return material;
}