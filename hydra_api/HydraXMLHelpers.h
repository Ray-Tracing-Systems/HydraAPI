#pragma once

#include <sstream>
#include "pugixml.hpp"
#include "LiteMath.h"
#include "HydraAPI.h"

namespace HydraXMLHelpers
{

  static inline float ReadFloat(pugi::xml_node a_node)
  {
    const wchar_t* camPosStr = a_node.text().as_string();
    std::wstringstream inputStream(camPosStr);
    float val = 0.0f;
    inputStream >> val;
    return val;
  }

  static inline HydraLiteMath::float3 ReadFloat3(pugi::xml_node a_node)
  {
    HydraLiteMath::float3 res(0,0,0);
    const wchar_t* camPosStr = a_node.text().as_string();
    if (camPosStr != nullptr)
    {
      std::wstringstream inputStream(camPosStr);
      inputStream >> res.x >> res.y >> res.z;
    }
    return res;
  }

  static inline HydraLiteMath::float3 ReadFloat3(pugi::xml_attribute a_attr)
  {
    HydraLiteMath::float3 res(0, 0, 0);
    const wchar_t* camPosStr = a_attr.as_string();
    if (camPosStr != nullptr)
    {
      std::wstringstream inputStream(camPosStr);
      inputStream >> res.x >> res.y >> res.z;
    }
    return res;
  }

  static inline void ReadFloat3(pugi::xml_node a_node, float a_outData[3])
  {
    const wchar_t* camPosStr = a_node.text().as_string();
    if (camPosStr != nullptr)
    {
      std::wstringstream inputStream(camPosStr);
      inputStream >> a_outData[0] >> a_outData[1] >> a_outData[2];
    }
    else
    {
      a_outData[0] = 0.0f;
      a_outData[1] = 0.0f;
      a_outData[2] = 0.0f;
    }
  }

  static inline void ReadFloat3(pugi::xml_attribute a_attr, float a_outData[3])
  {
    const wchar_t* camPosStr = a_attr.as_string();
    if (camPosStr != nullptr)
    {
      std::wstringstream inputStream(camPosStr);
      inputStream >> a_outData[0] >> a_outData[1] >> a_outData[2];
    }
    else
    {
      a_outData[0] = 0.0f;
      a_outData[1] = 0.0f;
      a_outData[2] = 0.0f;
    }
  }

  static inline void ReadMatrix4x4(pugi::xml_node a_node, const wchar_t* a_attrib_name, float a_outData[16])
  {
    const wchar_t* matrixStr = a_node.attribute(a_attrib_name).value();
    if(matrixStr != nullptr)
    { 
      std::wstringstream inputStream(matrixStr);
      inputStream >> a_outData[0]  >> a_outData[1]  >> a_outData[2]  >> a_outData[3]
                  >> a_outData[4]  >> a_outData[5]  >> a_outData[6]  >> a_outData[7]
                  >> a_outData[8]  >> a_outData[9]  >> a_outData[10] >> a_outData[11]
                  >> a_outData[12] >> a_outData[13] >> a_outData[14] >> a_outData[15];
    }
    else
    {
      a_outData[0]  = 1.0f; a_outData[1]  = 0.0f; a_outData[2]  = 0.0f; a_outData[3]  = 0.0f;
      a_outData[4]  = 0.0f; a_outData[5]  = 1.0f; a_outData[6]  = 0.0f; a_outData[7]  = 0.0f;
      a_outData[8]  = 0.0f; a_outData[9]  = 0.0f; a_outData[10] = 1.0f; a_outData[11] = 0.0f;
      a_outData[12] = 0.0f; a_outData[13] = 0.0f; a_outData[14] = 0.0f; a_outData[15] = 1.0f;
    }
  }

  static inline void WriteFloat(pugi::xml_node a_node, float a_value)
  {
    std::wstringstream outStream;
    outStream << a_value;
    a_node.text() = outStream.str().c_str();
  }

  static inline void WriteFloat3(pugi::xml_node a_node, HydraLiteMath::float3 a_value)
  {
    std::wstringstream outStream;
    outStream << a_value.x << L" " << a_value.y << L" " << a_value.z;
    a_node.text() = outStream.str().c_str();
  }

  static inline void WriteFloat3(pugi::xml_attribute a_attr, HydraLiteMath::float3 a_value)
  {
    std::wstringstream outStream;
    outStream << a_value.x << L" " << a_value.y << L" " << a_value.z;
    a_attr.set_value(outStream.str().c_str());
  }

  static inline void WriteFloat3(pugi::xml_node a_node, float a_value[3])
  {
    std::wstringstream outStream;
    outStream << a_value[0] << L" " << a_value[1] << L" " << a_value[2];
    a_node.text() = outStream.str().c_str();
  }

  static inline void WriteFloat3(pugi::xml_attribute a_attr, float a_value[3])
  {
    std::wstringstream outStream;
    outStream << a_value[0] << L" " << a_value[1] << L" " << a_value[2];
    a_attr.set_value(outStream.str().c_str());
  }

  static inline void WriteMatrix4x4(pugi::xml_node a_node, const wchar_t* a_attrib_name, float a_value[16])
  {
    std::wstringstream outStream;
    outStream << a_value[0]  << L" " << a_value[1]  << L" " << a_value[2]  << L" " << a_value[3]  << L" "
              << a_value[4]  << L" " << a_value[5]  << L" " << a_value[6]  << L" " << a_value[7]  << L" "
              << a_value[8]  << L" " << a_value[9]  << L" " << a_value[10] << L" " << a_value[11] << L" "
              << a_value[12] << L" " << a_value[13] << L" " << a_value[14] << L" " << a_value[15];

    a_node.attribute(a_attrib_name).set_value(outStream.str().c_str());
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  static HydraLiteMath::float3 ReadValue3f(const pugi::xml_node a_color) //#TODO: add this to documentation
  {
    HydraLiteMath::float3 color(0, 0, 0);
    if (a_color.attribute(L"val") != nullptr)
    {
      if (std::wstring(L"replace") == a_color.attribute(L"tex_apply_mode").as_string())
        color = HydraLiteMath::float3(1, 1, 1);
      else
        color = HydraXMLHelpers::ReadFloat3(a_color.attribute(L"val"));
    }
    else
      color = HydraXMLHelpers::ReadFloat3(a_color);          // deprecated
    return color;
  }


  static float ReadValue1f(const pugi::xml_node a_color) //#TODO: add this to documentation
  {
    float color = 0.0f;
    if (a_color.attribute(L"val") != nullptr)
      color = a_color.attribute(L"val").as_float();
    else
      color = a_color.text().as_float();          // deprecated
    return color;
  }

  static float ReadNamedValue1f(const pugi::xml_node a_color, const wchar_t* a_name) //#TODO: add this to documentation
  {
    float color = 0.0f;
    if (a_color.attribute(a_name) != nullptr)
      color = a_color.attribute(a_name).as_float();
    else
      color = a_color.child(a_name).text().as_float();          // deprecated
    return color;
  }

  static HydraLiteMath::float2 ReadRectLightSize(const pugi::xml_node a_lightNode) //#TODO: add this to documentation
  {
    float sizex = 0.0f;
    float sizey = 0.0f;

    const pugi::xml_node sizeNode = a_lightNode.child(L"size");

    if (sizeNode.attribute(L"width") != nullptr && sizeNode.attribute(L"length") != nullptr)
    {
      sizex = 0.5f*sizeNode.attribute(L"width").as_float();     // deprecated 
      sizey = 0.5f*sizeNode.attribute(L"length").as_float();    // deprecated 
    }
    else if (sizeNode.attribute(L"Half-length") != nullptr && sizeNode.attribute(L"Half-width") != nullptr)
    {
      sizex = sizeNode.attribute(L"Half-length").as_float();    // deprecated 
      sizey = sizeNode.attribute(L"Half-width").as_float();     // deprecated 
    }
    else if (sizeNode.attribute(L"half_length") != nullptr && sizeNode.attribute(L"half_width") != nullptr)
    {
      sizex = sizeNode.attribute(L"half_length").as_float();    // this is the new standard!
      sizey = sizeNode.attribute(L"half_width").as_float();     // this is the new standard!
    }
    else
    {
      sizex = sizeNode.child(L"Half-length").text().as_float(); // deprecated                                              
      sizey = sizeNode.child(L"Half-width").text().as_float();  // deprecated                                              
    }

    return HydraLiteMath::float2(sizex, sizey);
  }

  static float ReadSphereOrDiskLightRadius(const pugi::xml_node a_lightNode) //#TODO: add this to documentation
  {
    float sizex = 0.0f;

    const pugi::xml_node sizeNode = a_lightNode.child(L"size");

    if (sizeNode.attribute(L"radius") != nullptr )
      sizex = sizeNode.attribute(L"radius").as_float();
    else
      sizex = sizeNode.child(L"radius").text().as_float(); // deprecated                                                                                         

    return sizex;
  }

  static HydraLiteMath::float3 ReadLightIntensity(const pugi::xml_node a_lightNode) //#TODO: add this to documentation
  {
    const pugi::xml_node inode = a_lightNode.child(L"intensity");
    const pugi::xml_node icolr = inode.child(L"color");

    float  mult = 1.0f;
    HydraLiteMath::float3 color(0, 0, 0);

    if (icolr.attribute(L"val") != nullptr)
      color = HydraXMLHelpers::ReadFloat3(icolr.attribute(L"val"));
    else
      color = HydraXMLHelpers::ReadFloat3(icolr);          // deprecated

    if (inode.child(L"multiplier") != nullptr)             // multipliers are deprecated
    {
      if (inode.child(L"multiplier").attribute(L"val") != nullptr)
        mult = inode.child(L"multiplier").attribute(L"val").as_float();
      else
        mult = inode.child(L"multiplier").text().as_float(); // deprecated
    }

    return color*mult;
  }


  static inline bool StringHasBadSymbols(const std::wstring& a_str)
  {
    bool have = false;
    const std::wstring symbols = L"<>&\\";
    for (int i = 0; i < symbols.size(); i++)
    {
      wchar_t symbol = symbols[i];
      if (a_str.find(symbol) != std::wstring::npos)
        have = true;
    }
    return have;
  }

  std::vector<std::pair<std::string, int> > GetMaterialNameToIdMap();
 
};
