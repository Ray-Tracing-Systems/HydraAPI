//
// Created by vsan on 26.02.18.
//

#include "HydraXMLHelpers.h"
#include "HydraObjectManager.h"
#include "HydraLegacyUtils.h"

#include <string>

extern HRObjectManager g_objManager;

namespace HydraXMLHelpers
{
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
}