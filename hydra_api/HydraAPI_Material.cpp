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
  mat.id = ref.id;
  g_objManager.scnData.materials.push_back(mat);

  pugi::xml_node matNodeXml = g_objManager.materials_lib_append_child();

	matNodeXml.append_attribute(L"id").set_value(ref.id);
  matNodeXml.append_attribute(L"name").set_value(mat.name.c_str());
  matNodeXml.append_attribute(L"type").set_value(L"hydra_material");

  g_objManager.scnData.materials[ref.id].update(matNodeXml);

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
  mat.id = ref.id;
  g_objManager.scnData.materials.push_back(mat);


  pugi::xml_node matNodeXml = g_objManager.materials_lib_append_child();

	matNodeXml.append_attribute(L"id").set_value(ref.id);
  matNodeXml.append_attribute(L"name").set_value(mat.name.c_str());
  matNodeXml.append_attribute(L"type").set_value(L"hydra_blend");
  matNodeXml.append_attribute(L"node_top").set_value(a_pMat1.id);
  matNodeXml.append_attribute(L"node_bottom").set_value(a_pMat2.id);

  g_objManager.scnData.materials[ref.id].update(matNodeXml);

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

  pugi::xml_node nodeXml = pMat->xml_node();
  if (a_openMode == HR_WRITE_DISCARD)
    clear_node_childs(nodeXml);

  pMat->opened   = true;
  pMat->openMode = a_openMode;
}


void ArgCheck(int a_matId, pugi::xml_node a_nodeDecl, pugi::xml_node a_nodeVal)
{
  auto argDecl = a_nodeDecl.child(L"arg");
  auto argVal  = a_nodeVal.child(L"arg");
  
  int argNumDecl   = 0;
  int argNumActual = 0;
 
  struct Arg
  {
    std::wstring type;
    std::wstring name;
    std::wstring value;
    int size;
    int id;
  };
  
  // (1) read arguments to maps
  //
  
  std::unordered_map<int, Arg> decl;
  std::unordered_map<int, Arg> actual;
  
  do
  {
    if(argDecl != nullptr) argNumDecl++;
    if(argVal  != nullptr) argNumActual++;
    
    Arg adecl, aactual;
  
    adecl.type    = argDecl.attribute(L"type").as_string();
    adecl.name    = argDecl.attribute(L"name").as_string();
    adecl.value   = L"";
    adecl.size    = argDecl.attribute(L"size").as_int();
    adecl.id      = argDecl.attribute(L"id").as_int();
  
    aactual.type  = argVal.attribute(L"type").as_string();
    aactual.name  = argVal.attribute(L"name").as_string();
    aactual.value = argVal.attribute(L"val").as_string();
    aactual.size  = argVal.attribute(L"size").as_int();
    aactual.id    = argVal.attribute(L"id").as_int();
    
    if(actual.find(aactual.id) != actual.end())
      HrPrint(HR_SEVERITY_ERROR, L"ArgCheck(matId = ", a_matId, "): arg(", aactual.id, ") set twice!!! check 'id' attribute!!!");
    
    if(argDecl != nullptr)
      decl[adecl.id] = adecl;
    
    if(argVal != nullptr)
      actual[aactual.id] = aactual;
    
    argDecl = argDecl.next_sibling(L"arg");
    argVal  = argVal.next_sibling(L"arg");
  
  } while(argDecl != nullptr || argVal != nullptr);
  
  // (2) do actual checks
  //
  if(argNumDecl != argNumActual)
  {
    HrPrint(HR_SEVERITY_ERROR, L"ArgCheck(matId = ", a_matId, "): wrong argument number ");
    HrPrint(HR_SEVERITY_ERROR, L"ArgCheck(matId = ", a_matId, "): declared : ", argNumDecl);
    HrPrint(HR_SEVERITY_ERROR, L"ArgCheck(matId = ", a_matId, "): actual   : ", argNumActual);
  }
  
  for(auto& argDecl : decl)
  {
    auto p = actual.find(argDecl.first);
    if(p == actual.end())
      HrPrint(HR_SEVERITY_ERROR, L"ArgCheck(matId = ", a_matId, "): decl. arg(", argDecl.first, "), name = '", argDecl.second.name.c_str(), "' is not set");
    else
    {
      if(argDecl.second.type != p->second.type)
        HrPrint(HR_SEVERITY_ERROR, L"ArgCheck(matId = ", a_matId, "): wrong arg(",
                argDecl.first, ") type; declared is '", argDecl.second.type.c_str(), "', actual is '", p->second.type.c_str(), "'");
  
      if(argDecl.second.name != p->second.name)
        HrPrint(HR_SEVERITY_WARNING, L"ArgCheck(matId = ", a_matId, "): different arg(",
                argDecl.first, ") name; declared is '", argDecl.second.name.c_str(), "', actual is '", p->second.name.c_str(), "'");
  
      if(argDecl.second.size != p->second.size)
        HrPrint(HR_SEVERITY_ERROR, L"ArgCheck(matId = ", a_matId, "): wrong arg(",
                argDecl.first, ") array size; declared is '", argDecl.second.size, "', actual is '", p->second.size, "'");
      
      if(p->second.value == L"")
        HrPrint(HR_SEVERITY_ERROR, L"ArgCheck(matId = ", a_matId, "): empty arg(", argDecl.first, ") value");
    }
  }
  
}

void VerifyTex(int a_matId, pugi::xml_node a_currNode)
{
  if(std::wstring(L"texture") == a_currNode.name())
  {
    int32_t texId = a_currNode.attribute(L"id").as_int();
    HRTextureNodeRef tex;
    tex.id = texId;
    
    HRTextureNode* pTex = g_objManager.PtrById(tex);
    if(pTex == nullptr)
      HrPrint(HR_SEVERITY_WARNING, L"hrMaterialClose(",a_matId,")->VerifyTex: invalid texture id: ", texId);
  
    ArgCheck(a_matId, pTex->xml_node().child(L"code").child(L"generated"), a_currNode);
  }
  
  for(auto child : a_currNode.children())
    VerifyTex(a_matId, child);
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

  auto matNode = pMat->xml_node();
  VerifyTex(a_pMat.id, matNode);
  
  pMat->opened     = false;
  pMat->pImpl      = nullptr;
  pMat->wasChanged = true;
  g_objManager.scnData.m_changeList.matUsed.insert(pMat->id);
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

  return pMat->xml_node();
}


HAPI HRMaterialRef hrFindMaterialByName(const wchar_t *a_matName)
{
  HRMaterialRef material;

  if(a_matName != nullptr)
  {
    for (auto mat : g_objManager.scnData.materials)
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