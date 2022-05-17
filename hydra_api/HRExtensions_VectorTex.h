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
    VTEX_SDF,       // ordinary sdf
    VTEX_MSDF,      // multi-channel sdf (overall, better quality)
    VTEX_RASTERIZE  // rasterize vector texture
  };

  constexpr uint32_t VTEX_MASK_FLAG = 8;

  enum TextureFlags
  {
    TEX_WRAP    = 0,
    TEX_CLAMP_U = 4,
    TEX_CLAMP_V = 8
  };

  struct VectorTexCreateInfo
  {
    float     bgColor[4]    = { 1.0f, 1.0f, 1.0f, 1.0f };  // background color
    bool      generateMask  = false; // use vector texture as mask (for example, in a blend material) -> alpha will be written to rgb
    VTEX_MODE mode          = VTEX_MODE::VTEX_MSDF; 
    float     dpi           = 128.0f;   // dots per inch - affects output resolution
    float     sdfAngThres   = 3.0f;  // maximum angle (in radians) to be considered a corner when generating MSDF

   
    // Background color transitions into shape color through a zone, determined by the next two parameters in the following way:
    //            [distThreshold - smoothFac, distThreshold + smoothFac] 
    // Making the transition zone too narrow by reducing these parameters too much will produce aliasing effects.
    // These parameters should be changed mostly in the case of drawing outlines. 
    // Outline (or "stroke") throughout it's width will have distance changing from 0.0 (background) to 1.0 (shape).
    // Thus, the transition zone will "eat up" outline making rendered figure shrink in size, compared to SVG original.
    // So, if the correct size of a shape is important, these values will need to be reduced. 
    // To somewhat compensate aliasing produced in this case, dpi can be increased.
    float     smoothFac     = 1.0f / 64.0f;  
    float     distThreshold = 0.1f;  
    bool      drawOutline   = true;  // draw outlines for shapes that have them (have "stroke fill" defined in SVG)
                                     // only solid outlines are supported   

    bool      sdfCombine = false; // combine all separate shapes sdfs into one texture or create a texture array
    // if sdfCombine == true, the following two values will be used to override shape and outline colors:
    float     overrideShapeColor[4]   = { 1.0f, 1.0f, 0.0f, 1.0f };
    float     overrideOutlineColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

    float     textureMatrix[6] = { 1.0f, 0.0f, 0.0f, // 2-by-3 row-major matrix for transforming texture coordinates
                                   0.0f, 1.0f, 0.0f };
    int       textureFlags     = TEX_WRAP;
  };

  static constexpr float defaultSDFRange = 4.0f;

  HAPI HRTextureNodeRef hrTextureVector2DCreateFromFile(const wchar_t* a_fileName, const VectorTexCreateInfo* a_createInfo, pugi::xml_node* texNode);
}
