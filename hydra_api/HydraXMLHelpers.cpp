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

/*
 * onst wchar_t* matrixStr = a_node.attribute(a_attrib_name).value();
    if(matrixStr != nullptr)
    {
      std::wstringstream inputStream(matrixStr);
      inputStream >> a_outData[0]  >> a_outData[1]  >> a_outData[2]  >> a_outData[3]
                  >> a_outData[4]  >> a_outData[5]  >> a_outData[6]  >> a_outData[7]
                  >> a_outData[8]  >> a_outData[9]  >> a_outData[10] >> a_outData[11]
                  >> a_outData[12] >> a_outData[13] >> a_outData[14] >> a_outData[15];
    }
 *
 */