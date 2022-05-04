#include "HydraXMLHelpers.h"
#include "HydraObjectManager.h"
#include "HydraLegacyUtils.h"
#include "LiteMath.h"

#include <string>
#include <sstream>

extern HRObjectManager g_objManager;

namespace HydraXMLHelpers
{
  pugi::xml_node procTexFloat4Arg(pugi::xml_node& parent, uint32_t id, const std::wstring& argName, const LiteMath::float4& val)
  {
    auto arg = parent.append_child(L"arg");

    std::wstringstream ws;
    ws << val.x << L" " << val.y << L" " << val.z << L" " << val.w;
    arg.append_attribute(L"id")   = id;
    arg.append_attribute(L"name") = argName.c_str();
    arg.append_attribute(L"type") = L"float4";
    arg.append_attribute(L"size") = 1;
    arg.append_attribute(L"val")  = ws.str().c_str();

    return arg;
  }

  pugi::xml_node procTexFloat4Arg(pugi::xml_node& parent, uint32_t id, const std::wstring& argName, const float val[4])
  {
    auto arg = parent.append_child(L"arg");

    std::wstringstream ws;
    ws << val[0] << L" " << val[1] << L" " << val[2] << L" " << val[3];
    arg.append_attribute(L"id")   = id;
    arg.append_attribute(L"name") = argName.c_str();
    arg.append_attribute(L"type") = L"float4";
    arg.append_attribute(L"size") = 1;
    arg.append_attribute(L"val")  = ws.str().c_str();

    return arg;
  }

  pugi::xml_node procTexFloat3Arg(pugi::xml_node& parent, uint32_t id, const std::wstring& argName, const LiteMath::float3& val)
  {
    auto arg = parent.append_child(L"arg");

    std::wstringstream ws;
    ws << val.x << L" " << val.y << L" " << val.z;
    arg.append_attribute(L"id")   = id;
    arg.append_attribute(L"name") = argName.c_str();
    arg.append_attribute(L"type") = L"float3";
    arg.append_attribute(L"size") = 1;
    arg.append_attribute(L"val")  = ws.str().c_str();

    return arg;
  }

  pugi::xml_node procTexFloat3Arg(pugi::xml_node& parent, uint32_t id, const std::wstring& argName, const float val[3])
  {
    auto arg = parent.append_child(L"arg");

    std::wstringstream ws;
    ws << val[0] << L" " << val[1] << L" " << val[2];
    arg.append_attribute(L"id")   = id;
    arg.append_attribute(L"name") = argName.c_str();
    arg.append_attribute(L"type") = L"float3";
    arg.append_attribute(L"size") = 1;
    arg.append_attribute(L"val")  = ws.str().c_str();

    return arg;
  }

  pugi::xml_node procTexFloat2Arg(pugi::xml_node& parent, uint32_t id, const std::wstring& argName, const LiteMath::float2& val)
  {
    auto arg = parent.append_child(L"arg");

    std::wstringstream ws;
    ws << val.x << L" " << val.y;
    arg.append_attribute(L"id")   = id;
    arg.append_attribute(L"name") = argName.c_str();
    arg.append_attribute(L"type") = L"float2";
    arg.append_attribute(L"size") = 1;
    arg.append_attribute(L"val")  = ws.str().c_str();

    return arg;
  }

  pugi::xml_node procTexFloat2Arg(pugi::xml_node& parent, uint32_t id, const std::wstring& argName, const float val[2])
  {
    auto arg = parent.append_child(L"arg");

    std::wstringstream ws;
    ws << val[0] << L" " << val[1];
    arg.append_attribute(L"id")   = id;
    arg.append_attribute(L"name") = argName.c_str();
    arg.append_attribute(L"type") = L"float2";
    arg.append_attribute(L"size") = 1;
    arg.append_attribute(L"val")  = ws.str().c_str();

    return arg;
  }

  pugi::xml_node procTexSampler2DArg(pugi::xml_node& parent, uint32_t id, const std::wstring& argName, HRTextureNodeRef tex)
  {
    auto arg = parent.append_child(L"arg");

    arg.append_attribute(L"id")   = id;
    arg.append_attribute(L"name") = argName.c_str();
    arg.append_attribute(L"type") = L"sampler2D";
    arg.append_attribute(L"size") = 1;
    std::wstring valStr = std::to_wstring(tex.id);
    arg.append_attribute(L"val")  = valStr.c_str();

    return arg;
  }

  pugi::xml_node procTexSampler2DArrArg(pugi::xml_node& parent, uint32_t id, const std::wstring& argName, HRTextureNodeRef* texs, uint32_t numTexs)
  {
    auto arg = parent.append_child(L"arg");

    std::wstringstream ws;
    for (size_t i = 0; i < numTexs; ++i)
    {
      ws << texs[i].id;
      if (i != numTexs - 1)
        ws << L" ";
    }

    arg.append_attribute(L"id")   = id;
    arg.append_attribute(L"name") = L"sdfTexture";
    arg.append_attribute(L"type") = L"sampler2D";
    arg.append_attribute(L"size") = numTexs;
    arg.append_attribute(L"val")  = ws.str().c_str();

    return arg;
  }

  pugi::xml_node procTexIntArg(pugi::xml_node& parent, uint32_t id, const std::wstring& argName, const int32_t val)
  {
    auto arg = parent.append_child(L"arg");

    arg.append_attribute(L"id") = id;
    arg.append_attribute(L"name") = argName.c_str();
    arg.append_attribute(L"type") = L"int";
    arg.append_attribute(L"size") = 1;
    std::wstring valStr = std::to_wstring(val);
    arg.append_attribute(L"val")  = valStr.c_str();

    return arg;
  }

  pugi::xml_node procTexIntArrArg(pugi::xml_node& parent, uint32_t id, const std::wstring& argName, const int32_t* vals, uint32_t numVals)
  {
    auto arg = parent.append_child(L"arg");

    std::wstringstream ws;
    for (size_t i = 0; i < numVals; ++i)
    {
      ws << vals[i];
      if (i != numVals - 1)
        ws << L" ";
    }

    arg.append_attribute(L"id")   = id;
    arg.append_attribute(L"name") = argName.c_str();
    arg.append_attribute(L"type") = L"int";
    arg.append_attribute(L"size") = numVals;
    arg.append_attribute(L"val")  = ws.str().c_str();

    return arg;
  }

  pugi::xml_node procTexUintArg(pugi::xml_node& parent, uint32_t id, const std::wstring& argName, const uint32_t val)
  {
    auto arg = parent.append_child(L"arg");

    arg.append_attribute(L"id") = id;
    arg.append_attribute(L"name") = argName.c_str();
    arg.append_attribute(L"type") = L"unsigned";
    arg.append_attribute(L"size") = 1;
    std::wstring valStr = std::to_wstring(val);
    arg.append_attribute(L"val") = valStr.c_str();

    return arg;
  }

  pugi::xml_node procTexUintArrArg(pugi::xml_node& parent, uint32_t id, const std::wstring& argName, const uint32_t*vals, uint32_t numVals)
  {
    auto arg = parent.append_child(L"arg");

    std::wstringstream ws;
    for (size_t i = 0; i < numVals; ++i)
    {
      ws << vals[i];
      if (i != numVals - 1)
        ws << L" ";
    }

    arg.append_attribute(L"id")   = id;
    arg.append_attribute(L"name") = argName.c_str();
    arg.append_attribute(L"type") = L"unsigned";
    arg.append_attribute(L"size") = numVals;
    arg.append_attribute(L"val")  = ws.str().c_str();

    return arg;
  }

  pugi::xml_node procTexFloatArg(pugi::xml_node& parent, uint32_t id, const std::wstring& argName, const float val)
  {
    auto arg = parent.append_child(L"arg");

    arg.append_attribute(L"id") = id;
    arg.append_attribute(L"name") = argName.c_str();
    arg.append_attribute(L"type") = L"float";
    arg.append_attribute(L"size") = 1;
    std::wstring valStr = std::to_wstring(val);
    arg.append_attribute(L"val") = valStr.c_str();

    return arg;
  }

  pugi::xml_node procTexFloatArrArg(pugi::xml_node& parent, uint32_t id, const std::wstring& argName, const float* vals, uint32_t numVals)
  {
    auto arg = parent.append_child(L"arg");

    std::wstringstream ws;
    for (size_t i = 0; i < numVals; ++i)
    {
      ws << vals[i];
      if (i != numVals - 1)
        ws << L" ";
    }

    arg.append_attribute(L"id")   = id;
    arg.append_attribute(L"name") = argName.c_str();
    arg.append_attribute(L"type") = L"float";
    arg.append_attribute(L"size") = numVals;
    arg.append_attribute(L"val")  = ws.str().c_str();

    return arg;
  }

  std::vector<std::pair<std::string, int> > GetMaterialNameToIdMap()
  {
    std::vector<std::pair<std::string, int> > my_map;

    auto mat_lib_node = g_objManager.scnData.m_materialsLib;

    for(auto node = mat_lib_node.first_child(); node != nullptr; node = node.next_sibling())
    {
      std::wstring nameTmp = node.attribute(L"name").as_string();
      std::string nameConverted = ws2s(nameTmp);
      my_map.emplace_back(std::make_pair(nameConverted, node.attribute(L"id").as_int()));
    }

    return my_map;
  };

  std::vector<std::vector<int> > ReadRemapLists(pugi::xml_node a_node)
  {
    std::vector<std::vector<int> > remap_lists;

    for(auto node = a_node.first_child(); node != nullptr; node = node.next_sibling())
    {
      int list_size = node.attribute(L"size").as_int();

      std::vector<int> list(list_size, 0);

      const wchar_t* listStr = node.attribute(L"val").as_string();
      if(listStr != nullptr)
      {
        std::wstringstream inputStream(listStr);
        for(auto& elem : list)
          inputStream >> elem;

        remap_lists.emplace_back(list);
      }
    }

    return remap_lists;
  }

}
