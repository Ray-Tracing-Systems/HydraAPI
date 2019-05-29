//
// Created by vsan on 20.08.18.
//
#include "HydraAPI.h"
#include "HydraInternal.h"

#include <memory>
#include <vector>
#include <string>
#include <map>

#include <sstream>
#include <fstream>
#include <iomanip>

#include "HydraObjectManager.h"
#include "xxhash.h"

extern std::wstring      g_lastError;
extern std::wstring      g_lastErrorCallerPlace;
extern HR_ERROR_CALLBACK g_pErrorCallback;
extern HRObjectManager   g_objManager;


HAPI HRTextureNodeRef HRExtensions::hrTextureDisplacementCustom(HR_TEXTURE_DISPLACEMENT_CALLBACK a_proc, void* a_customData,
                                                                uint32_t a_customDataSize)
{

  if (a_proc == nullptr || (a_customData == nullptr && a_customDataSize > 0))
  {
    HrPrint(HR_SEVERITY_WARNING, L"hrTextureDisplacementCustom, invalid input");
    HRTextureNodeRef ref; // dummy white texture
    ref.id = 0;
    return ref;
  }

  std::wstringstream outStr;
  outStr << L"texture2d_" << g_objManager.scnData.textures.size();

  HRTextureNode texRes;
  texRes.name = outStr.str();
  texRes.id = int32_t(g_objManager.scnData.textures.size());
  texRes.customData = malloc(a_customDataSize);
  memcpy(texRes.customData, a_customData, size_t(a_customDataSize));
  texRes.displaceCallback = a_proc;
  texRes.customDataSize = a_customDataSize;

  g_objManager.scnData.textures.push_back(texRes);

  HRTextureNodeRef ref;
  ref.id = texRes.id;

  pugi::xml_node texNodeXml = g_objManager.textures_lib_append_child();

  auto byteSize = 0;

  std::wstringstream namestr;
  namestr << L"Map#" << ref.id;
  std::wstring texName = namestr.str();
  std::wstring id = ToWString(ref.id);
  std::wstring bytesize = ToWString(byteSize);

  texNodeXml.append_attribute(L"id").set_value(id.c_str());
  texNodeXml.append_attribute(L"name").set_value(texName.c_str());
  texNodeXml.append_attribute(L"type").set_value(L"displacement");

  g_objManager.scnData.textures[ref.id].update(texNodeXml);

  return ref;

}