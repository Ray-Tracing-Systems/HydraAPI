#pragma once
#define NOMINMAX
#include <cstdint>
#include <vector>
#include <variant>

#include "HydraAPI.h"

namespace hr_vtex
{

  enum class VTEX_MODE
  {
    VTEX_SDF,
    VTEX_MSDF,
    VTEX_RASTERIZE
  };

  struct vtexInfo
  {
    float     bgColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    VTEX_MODE mode        = VTEX_MODE::VTEX_MSDF;
    uint32_t  dpi         = 128;   // dots per inch - affects output resolution
    bool      sdfCombine  = false; // combine all separate shapes sdfs into one texture or create a texture array
    float     sdfAngThres = 3.0f;  // maximum angle (in radians) to be considered a corner when generating MSDF
    bool      drawOutline = true;
  };

  static constexpr float defaultSDFRange = 4.0f;

  HAPI HRTextureNodeRef hrTextureVector2DCreateFromFile(const wchar_t* a_fileName, const vtexInfo* a_createInfo, pugi::xml_node* texNode);
}