#include "HRExtensions_VectorTex.h"
#include "LiteMath.h"
#include "HydraLegacyUtils.h"
#include "HydraRenderDriverAPI.h"
#include "pugixml.hpp"
#include "HydraXMLHelpers.h"

#include "HR_HDRImageTool.h"

#include "HydraObjectManager.h"
extern HRObjectManager g_objManager;

#include <filesystem>
#include <iostream>
#include <cassert>
#include <cstdio>
#include <cmath>

#define  MSDFGEN_USE_SKIA
#include "msdfgen.h"
#include "msdfgen-ext.h"

#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"

#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h"

//#define DEBUG_SDF_GEN

#ifdef DEBUG_SDF_GEN
#define DEBUG_SDF_GEN_DIR "tests_images/test_ext_vtex_1/"
#endif

namespace hr_vtex
{

  using SDF_variant = std::variant<msdfgen::Bitmap<float, 1>, msdfgen::Bitmap<float, 3>>;
  using SDF_variant_vec = std::vector<SDF_variant>;


  static const std::string HELPER_FUNCS = R"END(
      float clamp(float x, float minVal, float maxVal)
      {
	      return min(max(x, minVal), maxVal);
      }

      float fract(float x)
      {
	      return x - floor(x);
      }

      float smoothstep(float edge0, float edge1, float x)
      {
        const float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
      }

      float median(float r, float g, float b) {
	      return max(min(r, g), min(max(r, g), b));
      }

      float mix(float x, float y, float a)
      {
        return x * (1.0f - a) + y * a;
      }

      float4 mix4(float4 x, float4 y, float a)
      {
        return make_float4(mix(x.x, y.x, a), mix(x.y, y.y, a), mix(x.z, y.z, a), mix(x.w, y.w, a));
      }

      float4 colorFromUint(unsigned val)
      {
        const float a = ((val & 0xFF000000) >> 24) / 255.0f;
        const float b = ((val & 0x00FF0000) >> 16) / 255.0f;
        const float g = ((val & 0x0000FF00) >> 8 ) / 255.0f;
        const float r = ((val & 0x000000FF)      ) / 255.0f;
  
        return make_float4(r, g, b, a);
      }

      float2 applyTexMatrix(const float2 texCoords, const float matrix[6])
      {
        float2 result = texCoords;
        result.x = result.x * matrix[0] + result.y * matrix[1] + matrix[2];
        result.y = result.x * matrix[3] + result.y * matrix[4] + matrix[5];
        
        return result;
      }

      float4 addOutline(const float distance, const float smoothing_factor, const float4 outlineColor, const float4 objColor)
      {
        float4 baseColor = objColor;
        if((distance  > 0.0f) && (distance < 1.0f))
        {
          float outlineFactor = 1.0f;
          if(distance < smoothing_factor)
          {
	          outlineFactor = smoothstep(0.0f, 0.0f + smoothing_factor, distance);
          }
          else
          {
	          outlineFactor = smoothstep(1.0f, 1.0f - smoothing_factor, distance);
          }

          baseColor = mix4(baseColor, outlineColor, outlineFactor);
        }

        return baseColor;
      }
      
      float4 rgb_mask_from_alpha(float4 color)
      {
        return make_float4(color.w, color.w, color.w, color.w);
      }
    )END";

  static std::string makeDefines()
  {
    std::stringstream ss;
    ss << "#define SDF " << static_cast<int>(VTEX_MODE::VTEX_SDF) << "\n";
    ss << "#define MSDF " << static_cast<int>(VTEX_MODE::VTEX_MSDF) << "\n";
    ss << "#define RASTER " << static_cast<int>(VTEX_MODE::VTEX_RASTERIZE) << "\n";
    ss << "#define MASK_FLAG " << static_cast<int>(VTEX_MASK_FLAG) << "\n";

    return ss.str();
  }

  static const std::string SINGLE_TEX_BODY_PART1 = R"END(
        const float2 texCoord = readAttr(sHit,"TexCoord0");
        float2 texCoord_adj = applyTexMatrix(texCoord, texMatrix);
        const float4 texColor = texture2D(sdfTexture, texCoord_adj, texFlags);
        if(mode & RASTER)
        {
          const float4 resColor = mix4(bgColor, texColor, texColor.w);
          if(mode & MASK_FLAG)
            return rgb_mask_from_alpha(resColor);
          else
	          return resColor;
        }
        else if(mode & SDF || mode & MSDF)
        {
	        const float4 objColor = colorFromUint(objColorU);
	        const float distance  = (mode & MSDF) ? median(texColor.x, texColor.y, texColor.z) : texColor.x;
	        const float alpha     = smoothstep(DIST_THRESHOLD - SMOOTHING_CONST, DIST_THRESHOLD + SMOOTHING_CONST, distance);
    )END";

  static const std::string SINGLE_TEX_BODY_PART2 = R"END(
	        const float4 resColor = mix4(bgColor, baseColor, alpha);
          if(mode & MASK_FLAG)
            return rgb_mask_from_alpha(resColor);
          else
	          return resColor;
        }
    )END";

  std::string makeSingleTexMainDeclaration(bool addOutlineSupport)
  {
    std::stringstream ss;
    ss << "float4 main(const SurfaceInfo* sHit, unsigned mode, float texMatrix[6], int texFlags, float4 bgColor, sampler2D sdfTexture, unsigned objColorU";
    if (addOutlineSupport)
    {
      ss << ", unsigned outlineColor";
    }

    ss << ")";

    return ss.str();
  }

  std::string makeSingleTexMain(bool addOutlineSupport, float distance_threshold, float smoothing_const)
  {
    std::stringstream ss;
    ss << makeSingleTexMainDeclaration(addOutlineSupport);
    ss << "{";
    ss << "  const float SMOOTHING_CONST = " << smoothing_const << ";\n";
    ss << "  const float DIST_THRESHOLD = "  << distance_threshold << ";\n";
    ss << SINGLE_TEX_BODY_PART1;
    
    if (addOutlineSupport)
    {
      ss << "float4 baseColor = addOutline(distance, SMOOTHING_CONST, outlineColor, objColor);\n";
    }
    else
    {
      ss << "float4 baseColor = objColor;\n";
    }
    ss << SINGLE_TEX_BODY_PART2;
    ss << "}";

    return ss.str();
  }

  static const std::string MULTI_TEX_BODY_PART1 = R"END(
        const float2 texCoord = readAttr(sHit,"TexCoord0");
        float2 texCoord_adj = applyTexMatrix(texCoord, texMatrix);
	      float4 resColor = bgColor;
	      for(int i = 0; i < numTextures; i += 1)
	      {
	        const float4 objColor = colorFromUint(objColors[i]);
	        const float4 texColor = texture2D(sdfTexture[i], texCoord_adj, texFlags); 
	        const float distance  = (mode & MSDF) ? median(texColor.x, texColor.y, texColor.z) : texColor.x;
	        const float alpha     = smoothstep(DIST_THRESHOLD - SMOOTHING_CONST, DIST_THRESHOLD + SMOOTHING_CONST, distance); 
      )END";

  static const std::string MULTI_TEX_BODY_PART2 = R"END(
	        resColor = mix4(resColor, baseColor, alpha);
	      }
        //resColor = mix4(bgColor, resColor, resColor.w);
        if(mode & MASK_FLAG)
          return rgb_mask_from_alpha(resColor);
        else
          return resColor;
      )END";

  std::string makeMultiTexMainDeclaration(bool addOutlineSupport, uint32_t numTextures)
  {
    std::stringstream ss;
    ss << "float4 main(const SurfaceInfo * sHit, unsigned mode, float texMatrix[6], int texFlags, float4 bgColor, sampler2D sdfTexture[" << numTextures << "], "
      << "unsigned objColors[" << numTextures << "], ";
    if (addOutlineSupport)
    {
      ss << "unsigned outlineColors[" << numTextures << "], "
         << "unsigned hasOutline[" << numTextures << "], ";
    }

    ss << " unsigned numTextures)";

    return ss.str();
  }

  std::string makeMultiTexMain(bool addOutlineSupport, uint32_t numTextures, float distance_threshold, float smoothing_const)
  {
    std::stringstream ss;
    ss << makeMultiTexMainDeclaration(addOutlineSupport, numTextures);
    ss << "{";
    ss << "  const float SMOOTHING_CONST = " << smoothing_const << ";\n";
    ss << "  const float DIST_THRESHOLD = "  << distance_threshold << ";\n";
    ss << MULTI_TEX_BODY_PART1;

    if (addOutlineSupport)
    {
      ss << "const float4 outlineColor = colorFromUint(outlineColors[i]);\n";
      ss << "float4 baseColor = (hasOutline[i] == 0) ? objColor : addOutline(distance, SMOOTHING_CONST, outlineColor, objColor);\n";
    }
    else
    {
      ss << "float4 baseColor = objColor;\n";
    }
    ss << MULTI_TEX_BODY_PART2;
    ss << "}";

    return ss.str();
  }

  std::string makeVecTexShader(bool addOutlineSupport, uint32_t numTextures, float distance_threshold = 0.5f, float smoothing_const = ( 1.0f / 64.0f))
  {
    std::stringstream ss;
    ss << HELPER_FUNCS;
    ss << makeDefines();

    if (numTextures == 1)
    {
      ss << makeSingleTexMain(addOutlineSupport, distance_threshold, smoothing_const);
    }
    else
    {
      ss << makeMultiTexMain(addOutlineSupport, numTextures, distance_threshold, smoothing_const);
    }

    return ss.str();
  }


  float median(float r, float g, float b) 
  {
    return std::max(std::min(r, g), std::min(std::max(r, g), b));
  }

  struct SDFData
  {
    SDF_variant_vec sdfs;
    SDF_variant combinedSDF;
    std::vector<msdfgen::Shape::Bounds> bounds;
    std::vector<unsigned int> flat_colors;
    std::vector<unsigned int> stroke_colors;
    std::vector<unsigned int> has_outline;
  };

  msdfgen::Shape buildShapeFromBezierCurves(NSVGshape* nsvgShape)
  {
    msdfgen::Shape outShape;
    outShape.contours.clear();
    outShape.inverseYAxis = true;
    for (NSVGpath* path = nsvgShape->paths; path != NULL; path = path->next)
    {
      msdfgen::Contour& contour = outShape.addContour();
      for (int i = 0; i < path->npts - 1; i += 3)
      {
        float* p = &path->pts[i * 2];
        contour.addEdge(new msdfgen::CubicSegment({ p[0], p[1] }, { p[2], p[3] }, { p[4], p[5] }, { p[6], p[7] }));
      }

      //contour.reverse();
    }
    msdfgen::resolveShapeGeometry(outShape);
    return outShape;
  }

  void addBezierCurvesToShape(NSVGshape* nsvgShape, msdfgen::Shape& genShape)
  {
    for (NSVGpath* path = nsvgShape->paths; path != NULL; path = path->next)
    {
      msdfgen::Contour& contour = genShape.addContour();
      for (int i = 0; i < path->npts - 1; i += 3)
      {
        float* p = &path->pts[i * 2];
        contour.addEdge(new msdfgen::CubicSegment({ p[0], p[1] }, { p[2], p[3] }, { p[4], p[5] }, { p[6], p[7] }));
      }
    }
  }

  LiteMath::float4 colorFromUint(unsigned val)
  {
    const float a = ((val & 0xFF000000) >> 24) / 255.0f;
    const float b = ((val & 0x00FF0000) >> 16) / 255.0f;
    const float g = ((val & 0x0000FF00) >> 8 ) / 255.0f;
    const float r = ((val & 0x000000FF)      ) / 255.0f;

    return {r, g, b, a};
  }

  SDFData generateSDFs(const NSVGimage *a_image, const VectorTexCreateInfo& a_settings)
  {
    // sdf generator settings for high quality
    const msdfgen::ErrorCorrectionConfig errCorr(msdfgen::ErrorCorrectionConfig::EDGE_PRIORITY,  //EDGE_ONLY
                                                 msdfgen::ErrorCorrectionConfig::ALWAYS_CHECK_DISTANCE);
    const msdfgen::MSDFGeneratorConfig   msdfConf(true, errCorr);
    const msdfgen::GeneratorConfig       sdfConf(true);
    const msdfgen::Projection            proj(1.0f, 0.0f);

    const auto width  = static_cast<uint32_t>(std::ceil(a_image->width));
    const auto height = static_cast<uint32_t>(std::ceil(a_image->height));

    SDFData result;
    int i = 0;
    for (NSVGshape* s = a_image->shapes; s != NULL; s = s->next)
    {
      float sdfRange = defaultSDFRange;
      if (a_settings.drawOutline && s->fill.color != s->stroke.color)
      {
        sdfRange = s->strokeWidth;
      }

      auto shape = buildShapeFromBezierCurves(s);
      shape.normalize();
      msdfgen::Shape::Bounds b = { };
      b = shape.getBounds();
      result.bounds.push_back(std::move(b));

      if (a_settings.mode == VTEX_MODE::VTEX_SDF)
      {
        msdfgen::Bitmap<float, 1> sdf(width, height);
        msdfgen::generateSDF(sdf, shape, proj, sdfRange, sdfConf);
#ifdef DEBUG_SDF_GEN
        std::string path = std::string(DEBUG_SDF_GEN_DIR) + std::string("sdf") + std::to_string(i) + ".png";
        ++i;
        msdfgen::savePng(sdf, path.c_str());
#endif
        result.sdfs.push_back(std::move(sdf));
      }
      else if (a_settings.mode == VTEX_MODE::VTEX_MSDF)
      {
        msdfgen::edgeColoringByDistance(shape, a_settings.sdfAngThres);
        //msdfgen::edgeColoringSimple(shape, a_settings.sdfAngThres);
        msdfgen::Bitmap<float, 3> msdf(width, height);
        msdfgen::generateMSDF(msdf, shape, proj, sdfRange, msdfConf);

        auto bottom = static_cast<int>(height - std::floor(b.t));
        auto top    = static_cast<int>(height - std::fmaxf(0.0f, std::floor(b.b)));
        auto left   = static_cast<int>(std::fmaxf(0.0f, std::floor(b.l)));
        auto right  = static_cast<int>(std::floor(b.r));

       /* msdfgen::Bitmap<float, 3> msdf_clean(width, height);
        for (size_t y = bottom; y < top; ++y)
        {
          for (size_t x = left; x < right; ++x)
          {
            msdf_clean(x, y)[0] = msdf(x, y)[0];
            msdf_clean(x, y)[1] = msdf(x, y)[1];
            msdf_clean(x, y)[2] = msdf(x, y)[2];
          }
        }*/

#ifdef DEBUG_SDF_GEN
        std::string path = std::string(DEBUG_SDF_GEN_DIR) + std::string("msdf") + std::to_string(i) + ".png";
        ++i;
        msdfgen::savePng(msdf, path.c_str());
#endif
        result.sdfs.push_back(std::move(msdf));
      }
      
      result.flat_colors.push_back(s->fill.color);
//      const auto col = colorFromUint(result.flat_colors.back());
//      std::cout << col.x << " " << col.y << " " << col.z << " " << col.w << std::endl;

      if (s->stroke.type == 0)
      {
        result.stroke_colors.push_back(s->fill.color);
        result.has_outline.push_back(0);
      }
      else
      {
        result.stroke_colors.push_back(s->stroke.color);
        result.has_outline.push_back(1);
      }
    }

    return result;
  }

  void mergeSDFs(const NSVGimage* a_image, const VectorTexCreateInfo& a_settings, SDFData& a_sdfData)
  {
    assert(a_sdfData.sdfs.size() == a_sdfData.bounds.size());

    const auto width = static_cast<uint32_t>(std::ceil(a_image->width));
    const auto height = static_cast<uint32_t>(std::ceil(a_image->height));

    if (a_settings.mode == VTEX_MODE::VTEX_SDF)
    {
      a_sdfData.combinedSDF = std::move(msdfgen::Bitmap<float, 1>(width, height));
    }
    else if (a_settings.mode == VTEX_MODE::VTEX_MSDF)
    {
      a_sdfData.combinedSDF = std::move(msdfgen::Bitmap<float, 3>(width, height));
    }

    for (size_t y = 0; y < height; ++y)
    {
      for (size_t x = 0; x < width; ++x)
      {
        if (a_settings.mode == VTEX_MODE::VTEX_SDF)
        {
          std::get<0>(a_sdfData.combinedSDF)(x, y)[0] = std::numeric_limits<float>::min();
        }
        else if (a_settings.mode == VTEX_MODE::VTEX_MSDF)
        {
          std::get<1>(a_sdfData.combinedSDF)(x, y)[0] = std::numeric_limits<float>::min();
          std::get<1>(a_sdfData.combinedSDF)(x, y)[1] = std::numeric_limits<float>::min();
          std::get<1>(a_sdfData.combinedSDF)(x, y)[2] = std::numeric_limits<float>::min();
        }
      }
    }

    size_t num_sdfs = a_sdfData.sdfs.size();
    for (uint32_t idx = 0; idx < num_sdfs; ++idx)
    {
      const auto& b = a_sdfData.bounds[idx];

      auto bottom = static_cast<int>(height - std::floor(b.t));
      auto top    = static_cast<int>(height - std::fmaxf(0.0f, std::floor(b.b)));
      auto left   = static_cast<int>(std::fmaxf(0.0f, std::floor(b.l)));
      auto right  = static_cast<int>(std::floor(b.r));

      for (size_t y = bottom; y < top; ++y)
      {
        for (size_t x = left; x < right; ++x)
        {
          if (a_settings.mode == VTEX_MODE::VTEX_SDF)
          {
            std::get<0>(a_sdfData.combinedSDF)(x, y)[0] = std::get<0>(a_sdfData.sdfs[idx])(x, y)[0];
          }
          else if (a_settings.mode == VTEX_MODE::VTEX_MSDF)
          {
            std::get<1>(a_sdfData.combinedSDF)(x, y)[0] = std::get<1>(a_sdfData.sdfs[idx])(x, y)[0];
            std::get<1>(a_sdfData.combinedSDF)(x, y)[1] = std::get<1>(a_sdfData.sdfs[idx])(x, y)[1];
            std::get<1>(a_sdfData.combinedSDF)(x, y)[2] = std::get<1>(a_sdfData.sdfs[idx])(x, y)[2];
          }
        }
      }
    }
  }

  HRTextureNodeRef rasterizeSVG(NSVGimage* a_image)
  {
    const auto width  = static_cast<uint32_t>(std::ceil(a_image->width));
    const auto height = static_cast<uint32_t>(std::ceil(a_image->height));

    struct NSVGrasterizer* rast = nsvgCreateRasterizer();
    std::vector<unsigned char> pixel_data(width * height * 4, 0);

    nsvgRasterize(rast, a_image, 0, 0, 1, (unsigned char*)pixel_data.data(), width, height, width * sizeof(pixel_data[0]) * 4);
    auto ref = hrTexture2DCreateFromMemory(width, height, sizeof(pixel_data[0]) * 4, pixel_data.data());


#ifdef DEBUG_SDF_GEN
    std::string path = std::string(DEBUG_SDF_GEN_DIR) + std::string("rasterized_texture.png");
    stbi_write_png(path.c_str(), width, height, 4, pixel_data.data(), width * 4);
#endif
    
    return ref;
  }


  // Hydra Core 2 only supports 4-channel textures...
  std::vector<uint32_t> convertSDFData(const msdfgen::BitmapConstRef<float, 1>& sdf)
  {
    std::vector<uint32_t> pixels(sdf.width * sdf.height, 0);
    for (int y = 0; y < sdf.height; ++y)
    {
      for (int x = 0; x < sdf.width; ++x)
      {
        auto idx = x + y * sdf.width;
        pixels[idx] = HRUtils::RealColorToUint32(sdf(x, y)[0], 0.0f, 0.0f, 0.0f);
      }
    }
    return pixels;
  }

  // Hydra Core 2 only supports 4-channel textures...
  std::vector<uint32_t> convertSDFData(const msdfgen::BitmapConstRef<msdfgen::byte, 1>& sdf)
  {
    std::vector<uint32_t> pixels(sdf.width * sdf.height, 0);
    for (int y = 0; y < sdf.height; ++y)
    {
      for (int x = 0; x < sdf.width; ++x)
      {
        auto idx = x + y * sdf.width;
        pixels[idx] = sdf(x, y)[0];
      }
    }
    return pixels;
  }

  std::vector<uint32_t> convertSDFData(const msdfgen::BitmapConstRef<float, 3>& sdf)
  {
    std::vector<uint32_t> pixels(sdf.width * sdf.height, 0);
    for (int y = 0; y < sdf.height; ++y)
    {
      for (int x = 0; x < sdf.width; ++x)
      {
        auto idx = x + y * sdf.width;
        pixels[idx] = HRUtils::RealColorToUint32(sdf(x, y)[0], sdf(x, y)[1], sdf(x, y)[2], 0.0f);
      }
    }
    return pixels;
  }

  HRTextureNodeRef hrTextureVector2DCreateFromFile(const wchar_t* a_fileName, const VectorTexCreateInfo* a_createInfo, pugi::xml_node* texNode)
  {
    {
      auto p = g_objManager.scnData.m_textureCache.find(a_fileName);
      if (p != g_objManager.scnData.m_textureCache.end())
      {
        HRTextureNodeRef ref;
        ref.id = p->second;
        return ref;
      }
    }

    size_t totalTextureMemoryUsed = 0;

    HRTextureNodeRef ref;
    const std::filesystem::path inputPath(a_fileName);
    if (!std::filesystem::exists(inputPath))
    {
      HrPrint(HR_SEVERITY_WARNING, L"hrTextureVector2DCreateFromFile can't open file ", a_fileName);
      ref.id = 0;
      return ref;
    }

    auto image = nsvgParseFromFile(inputPath.string().c_str(), "px", a_createInfo->dpi);
    if (!image)
    {
      HrPrint(HR_SEVERITY_WARNING, L"hrTextureVector2DCreateFromFile can't parse SVG image: ", a_fileName);
      ref.id = 0;
      return ref;
    }

    hr_vtex::SDFData sdfData;
    std::vector<HRTextureNodeRef> texSDFs;
    if (a_createInfo->mode == VTEX_MODE::VTEX_RASTERIZE)
    {
      texSDFs.push_back(rasterizeSVG(image));
      sdfData.flat_colors.push_back(0); // placeholder value, not actually used

      hrTextureNodeOpen(texSDFs.back(), HR_OPEN_READ_ONLY);
      {
        auto node = hrTextureParamNode(texSDFs.back());
        totalTextureMemoryUsed += node.attribute(L"bytesize").as_ullong();
      }
      hrTextureNodeClose(texSDFs.back());
    }
    else
    {
      sdfData = generateSDFs(image, *a_createInfo);

      if (a_createInfo->sdfCombine)
      {
        mergeSDFs(image, *a_createInfo, sdfData);

        if (a_createInfo->mode == VTEX_MODE::VTEX_SDF)
        {
          const auto& sdf = msdfgen::BitmapRef<float, 1>(std::get<0>(sdfData.combinedSDF));
#ifdef DEBUG_SDF_GEN
          auto debug_save_path = std::string(DEBUG_SDF_GEN_DIR) + std::string("sdf.png");
          msdfgen::savePng(sdf, debug_save_path.c_str());
#endif
          const auto pixel_data = convertSDFData(sdf);
          texSDFs.push_back(hrTexture2DCreateFromMemory(sdf.width, sdf.height, sizeof(pixel_data[0]), pixel_data.data()));
          totalTextureMemoryUsed += sdf.width * sdf.height * sizeof(pixel_data[0]);
        }
        else if (a_createInfo->mode == VTEX_MODE::VTEX_MSDF)
        {
          const auto& sdf = msdfgen::BitmapRef<float, 3>(std::get<1>(sdfData.combinedSDF));
#ifdef DEBUG_SDF_GEN
          auto debug_save_path = std::string(DEBUG_SDF_GEN_DIR) + std::string("msdf.png");
          msdfgen::savePng(sdf, debug_save_path.c_str());
#endif
          const auto pixel_data = convertSDFData(sdf);
          texSDFs.push_back(hrTexture2DCreateFromMemory(sdf.width, sdf.height, sizeof(pixel_data[0]), pixel_data.data()));
          totalTextureMemoryUsed += sdf.width * sdf.height * sizeof(pixel_data[0]);
        }
      }
      else
      {
        for (auto& sdf : sdfData.sdfs)
        {
          if (a_createInfo->mode == VTEX_MODE::VTEX_SDF)
          {
            const auto& sdf_ = msdfgen::BitmapRef<float, 1>(std::get<0>(sdf));
            const auto pixel_data = convertSDFData(sdf_);
            texSDFs.push_back(hrTexture2DCreateFromMemory(sdf_.width, sdf_.height, sizeof(pixel_data[0]), pixel_data.data()));
            totalTextureMemoryUsed += sdf_.width * sdf_.height * sizeof(pixel_data[0]);
          }
          else if (a_createInfo->mode == VTEX_MODE::VTEX_MSDF)
          {
            const auto& sdf_ = msdfgen::BitmapRef<float, 3>(std::get<1>(sdf));
            const auto pixel_data = convertSDFData(sdf_);
            auto tex = hrTexture2DCreateFromMemory(sdf_.width, sdf_.height, sizeof(pixel_data[0]), pixel_data.data());
            totalTextureMemoryUsed += sdf_.width * sdf_.height * sizeof(pixel_data[0]);
            texSDFs.push_back(tex);
          }
        }
      }
    }

    std::string           tmp_shader_name = std::tmpnam(nullptr);
    std::filesystem::path sdf_shader_path = std::filesystem::temp_directory_path().append(tmp_shader_name); 
    {
      std::ofstream out(sdf_shader_path);
      if (out.is_open())
      {
        out << makeVecTexShader(a_createInfo->drawOutline, texSDFs.size(), a_createInfo->distThreshold, a_createInfo->smoothFac);
        out.close();
      }
      else
      {
        HrPrint(HR_SEVERITY_WARNING, L"hrTextureVector2DCreateFromFile. Can't create temporary shader file");
        ref.id = 0;
        return ref;
      }
     
    }

    // add proc tex arguments to used memory amount
    totalTextureMemoryUsed += 6 * sizeof(float) + sizeof(uint32_t) * 4 + sizeof(float) * 4 + sizeof(int32_t) * texSDFs.size() * 2;
    if (a_createInfo->drawOutline)
    {
      totalTextureMemoryUsed += sizeof(int32_t) * texSDFs.size() * 2;
    }

    HRTextureNodeRef texProc = hrTextureCreateAdvanced(L"proc", L"vector_tex");
    pugi::xml_node proc_tex_node;
    hrTextureNodeOpen(texProc, HR_WRITE_DISCARD);
    {
      proc_tex_node = hrTextureParamNode(texProc);

      auto code_node = proc_tex_node.append_child(L"code");
      code_node.append_attribute(L"file") = sdf_shader_path.wstring().c_str();
      code_node.append_attribute(L"main") = L"main";

      proc_tex_node.append_attribute(L"memory_estimate").set_value(totalTextureMemoryUsed);
    }
    hrTextureNodeClose(texProc);

    *texNode = proc_tex_node.append_child(L"temp_args");

    float textureMatrix[6] = { a_createInfo->textureMatrix[0], a_createInfo->textureMatrix[1], a_createInfo->textureMatrix[2],
                               a_createInfo->textureMatrix[3], a_createInfo->textureMatrix[4], a_createInfo->textureMatrix[5] };

    if (a_createInfo->mode == VTEX_MODE::VTEX_RASTERIZE) //flip Y coordinate
      textureMatrix[4] *= -1.0f;

    int argIdx = 0;
    auto mode  = static_cast<uint32_t>(a_createInfo->mode);
    if(a_createInfo->generateMask) mode = mode | VTEX_MASK_FLAG;

    HydraXMLHelpers::procTexUintArg    (*texNode, argIdx++, L"mode", mode);
    HydraXMLHelpers::procTexFloatArrArg(*texNode, argIdx++, L"texMatrix", textureMatrix, 6);
    HydraXMLHelpers::procTexIntArg     (*texNode, argIdx++, L"texFlags", a_createInfo->textureFlags);
    HydraXMLHelpers::procTexFloat4Arg  (*texNode, argIdx++, L"bgColor", a_createInfo->bgColor);

    if (a_createInfo->sdfCombine || texSDFs.size() == 1 || a_createInfo->mode == VTEX_MODE::VTEX_RASTERIZE)
    {
      uint32_t shapeColor = HRUtils::RealColorToUint32(a_createInfo->overrideShapeColor[0], a_createInfo->overrideShapeColor[1],
                                                       a_createInfo->overrideShapeColor[2], a_createInfo->overrideShapeColor[3]);
      HydraXMLHelpers::procTexSampler2DArg(*texNode, argIdx++, L"sdfTexture", texSDFs[0]);
      HydraXMLHelpers::procTexUintArg     (*texNode, argIdx++, L"objColorU", shapeColor);

      if (a_createInfo->drawOutline)
      {
        uint32_t strokeColor = HRUtils::RealColorToUint32(a_createInfo->overrideOutlineColor[0], a_createInfo->overrideOutlineColor[1],
                                                          a_createInfo->overrideOutlineColor[2], a_createInfo->overrideOutlineColor[3]);
        HydraXMLHelpers::procTexUintArg(*texNode, argIdx++, L"outlineColor", strokeColor);
      }
    }
    else
    {
      assert(sdfData.flat_colors.size() == texSDFs.size());
      HydraXMLHelpers::procTexSampler2DArrArg(*texNode, argIdx++, L"sdfTexture", texSDFs.data(), texSDFs.size());
      HydraXMLHelpers::procTexUintArrArg     (*texNode, argIdx++, L"objColors",  sdfData.flat_colors.data(), texSDFs.size());
      if (a_createInfo->drawOutline)
      {
        HydraXMLHelpers::procTexUintArrArg(*texNode, argIdx++, L"outlineColors", sdfData.stroke_colors.data(), texSDFs.size());
        HydraXMLHelpers::procTexUintArrArg(*texNode, argIdx++, L"hasOutline",    sdfData.has_outline.data(), texSDFs.size());
      }
      HydraXMLHelpers::procTexUintArg     (*texNode, argIdx++, L"numTextures", texSDFs.size());
    }

    return texProc;
  }
}