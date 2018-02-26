//
// Created by vsan on 26.02.18.
//

#include "HydraXMLHelpers.h"
#include <string>

namespace HydraXMLHelpers
{
    std::vector<std::pair<std::wstring, int> > GetMaterialNameToIdMap(const pugi::xml_node &some_material)
    {
      std::vector<std::pair<std::wstring, int> > my_map;

      auto mat_lib_node = some_material.parent();

      for(auto node = mat_lib_node.first_child(); node != nullptr; node = node.next_sibling())
      {
        my_map.emplace_back(std::make_pair(node.attribute(L"name").as_string(), node.attribute(L"id").as_int()));
      }

      return my_map;
    };
}