#include "HRExtensions_VectorTex.h"
#include "LiteMath.h"
#include "HydraLegacyUtils.h"
#include "HydraRenderDriverAPI.h"
#include "pugixml.hpp"

#include "HR_HDRImageTool.h"

#include "HydraObjectManager.h"
extern HRObjectManager g_objManager;

#include <filesystem>
#include <iostream>
#include <cassert>

#include "msdfgen.h"
#include "msdfgen-ext.h"

#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"

#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

#define DEBUG_SDF_GEN

#ifdef DEBUG_SDF_GEN
#define DEBUG_SDF_GEN_DIR "E:/repos/hydra/HydraAPI-tests/tests_images/test_ext_vtex_1/"
#endif

namespace hr_vtex
{

  using SDF_variant = std::variant<msdfgen::Bitmap<float, 1>, msdfgen::Bitmap<float, 3>>;
  using SDF_variant_vec = std::vector<SDF_variant>;

  struct SDFData
  {
    SDF_variant_vec sdfs;
    SDF_variant combinedSDF;
    std::vector<msdfgen::Shape::Bounds> bounds;
    std::vector<unsigned int> flat_colors;
    msdfgen::Bitmap<msdfgen::byte, 4> instances;
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

      //if (contour.winding() < 0)
      contour.reverse();
    }
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


  SDFData generateSDFs(const NSVGimage *a_image, const vtexInfo& a_settings)
  {
    // sdf generator settings for high quality
    const msdfgen::ErrorCorrectionConfig errCorr(msdfgen::ErrorCorrectionConfig::EDGE_PRIORITY,
                                                 msdfgen::ErrorCorrectionConfig::CHECK_DISTANCE_AT_EDGE);
    const msdfgen::MSDFGeneratorConfig   msdfConf(true, errCorr);
    const msdfgen::GeneratorConfig       sdfConf(true);
    const msdfgen::Projection            proj(1.0f, 0.0f);

    const auto width  = static_cast<uint32_t>(std::ceilf(a_image->width));
    const auto height = static_cast<uint32_t>(std::ceilf(a_image->height));

    SDFData result;
    int i = 0;
    for (NSVGshape* s = a_image->shapes; s != NULL; s = s->next)
    {
      auto shape = buildShapeFromBezierCurves(s);
      shape.normalize();
      msdfgen::Shape::Bounds b = { };
      b = shape.getBounds();
      result.bounds.push_back(std::move(b));

      if (a_settings.mode == VTEX_MODE::VTEX_SDF)
      {
        msdfgen::Bitmap<float, 1> sdf(width, height);
        msdfgen::generateSDF(sdf, shape, proj, a_settings.sdfRange, sdfConf);
#ifdef DEBUG_SDF_GEN
        std::string path = std::string(DEBUG_SDF_GEN_DIR) + std::string("sdf") + std::to_string(i) + ".png";
        ++i;
        msdfgen::savePng(sdf, path.c_str());
#endif
        result.sdfs.push_back(std::move(sdf));
      }
      else if (a_settings.mode == VTEX_MODE::VTEX_MSDF)
      {
        //msdfgen::edgeColoringByDistance(shape, a_settings.sdfAngThres);
        msdfgen::edgeColoringSimple(shape, a_settings.sdfAngThres);
        msdfgen::Bitmap<float, 3> msdf(width, height);
        msdfgen::generateMSDF(msdf, shape, proj, a_settings.sdfRange, msdfConf);
#ifdef DEBUG_SDF_GEN
        std::string path = std::string(DEBUG_SDF_GEN_DIR) + std::string("msdf") + std::to_string(i) + ".png";
        ++i;
        msdfgen::savePng(msdf, path.c_str());
#endif
        result.sdfs.push_back(std::move(msdf));
      }
      
      result.flat_colors.push_back(s->fill.color);
    }

    return result;
  }

  void mergeSDFs(const NSVGimage* a_image, const vtexInfo& a_settings, SDFData& a_sdfData)
  {
    assert(a_sdfData.sdfs.size() == a_sdfData.bounds.size());

    const auto width = static_cast<uint32_t>(std::ceilf(a_image->width));
    const auto height = static_cast<uint32_t>(std::ceilf(a_image->height));

    a_sdfData.instances = std::move(msdfgen::Bitmap<msdfgen::byte, 4>(width, height));
    
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

        a_sdfData.instances(x, y)[0] = 0;
        a_sdfData.instances(x, y)[1] = 0;
        a_sdfData.instances(x, y)[2] = 0;
        a_sdfData.instances(x, y)[3] = 0;
      }
    }

    size_t num_sdfs = a_sdfData.sdfs.size();
    for (uint32_t idx = 0; idx < num_sdfs; ++idx)
    {
      const auto& b = a_sdfData.bounds[idx];

      auto bottom = static_cast<int>(height - std::floorf(b.t));
      auto top    = static_cast<int>(height - std::fmaxf(0.0f, std::floorf(b.b)));
      auto left   = static_cast<int>(std::fmaxf(0.0f, std::floorf(b.l)));
      auto right  = static_cast<int>(std::floorf(b.r));

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
          memcpy(&a_sdfData.instances(x, y)[0], &idx, sizeof(uint32_t));
        }
      }
    }
  }

  void fillInstances(const NSVGimage* a_image, const vtexInfo& a_settings, SDFData& a_sdfData)
  {
    assert(a_sdfData.sdfs.size() == a_sdfData.bounds.size());

    for (size_t y = 0; y < a_image->height; ++y)
    {
      for (size_t x = 0; x < a_image->width; ++x)
      {
        a_sdfData.instances(x, y)[0] = 0;
        a_sdfData.instances(x, y)[1] = 0;
        a_sdfData.instances(x, y)[2] = 0;
        a_sdfData.instances(x, y)[3] = 0;
      }
    }

    size_t num_sdfs = a_sdfData.sdfs.size();
    for (uint32_t idx = 0; idx < num_sdfs; ++idx)
    {
      const auto& b = a_sdfData.bounds[idx];

      auto bottom = static_cast<int>(a_image->height - std::floorf(b.t));
      auto top    = static_cast<int>(a_image->height - std::fmaxf(0.0f, std::floorf(b.b)));
      auto left   = static_cast<int>(std::fmaxf(0.0f, std::floorf(b.l)));
      auto right  = static_cast<int>(std::floorf(b.r));

      for (size_t y = bottom; y < top; ++y)
      {
        for (size_t x = left; x < right; ++x)
        {
          memcpy(&a_sdfData.instances(x, y)[0], &idx, sizeof(uint32_t));
        }
      }
    }
  }

  HRTextureNodeRef rasterizeSVG(NSVGimage* a_image)
  {
    const auto width  = static_cast<uint32_t>(std::ceilf(a_image->width));
    const auto height = static_cast<uint32_t>(std::ceilf(a_image->height));

    struct NSVGrasterizer* rast = nsvgCreateRasterizer();
    std::vector<unsigned char> pixel_data(width * height * 4, 0);

    nsvgRasterize(rast, a_image, 0, 0, 1, (unsigned char*)pixel_data.data(), width, height, width * sizeof(pixel_data[0]) * 4);
    auto ref = hrTexture2DCreateFromMemory(width, height, sizeof(pixel_data[0]) * 4, pixel_data.data());

    /*HydraRender::SaveImageToFile(L"raster.png", width, height, (unsigned int*)pixel_data.data());
    auto ref = hrTexture2DCreateFromFile(L"raster.png");*/

    return ref;
  }


  // Hydra Modern only supports 4-channel textures...
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

  // Hydra Modern only supports 4-channel textures...
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

  HRTextureNodeRef hrTextureVector2DCreateFromFile(const wchar_t* a_fileName, const vtexInfo* a_createInfo, pugi::xml_node* texNode)
  {
    /////////////////////////////////////////////////////////////////////////////////////////////////
    {
      auto p = g_objManager.scnData.m_textureCache.find(a_fileName);
      if (p != g_objManager.scnData.m_textureCache.end())
      {
        HRTextureNodeRef ref;
        ref.id = p->second;
        return ref;
      }
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////

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

    if (a_createInfo->mode == VTEX_MODE::VTEX_RASTERIZE)
    {
      return rasterizeSVG(image);
    }

    auto sdfData = generateSDFs(image, *a_createInfo);
    mergeSDFs(image, *a_createInfo, sdfData);

    // @TODO: combined and texture array variants
    HRTextureNodeRef texSDF;
    if (a_createInfo->mode == VTEX_MODE::VTEX_SDF)
    {
      const auto& sdf = msdfgen::BitmapRef<float, 1>(std::get<0>(sdfData.combinedSDF));
#ifdef DEBUG_SDF_GEN
      auto debug_save_path = std::string(DEBUG_SDF_GEN_DIR) + std::string("sdf.png");
      msdfgen::savePng(sdf, debug_save_path.c_str());
#endif
      const auto pixel_data = convertSDFData(sdf);
      texSDF = hrTexture2DCreateFromMemory(sdf.width, sdf.height, sizeof(pixel_data[0]), pixel_data.data());
    }
    else if (a_createInfo->mode == VTEX_MODE::VTEX_MSDF)
    {
      const auto& sdf = msdfgen::BitmapRef<float, 3>(std::get<1>(sdfData.combinedSDF));
#ifdef DEBUG_SDF_GEN
      auto debug_save_path = std::string(DEBUG_SDF_GEN_DIR) + std::string("msdf.png");
      msdfgen::savePng(sdf, debug_save_path.c_str());
#endif
      const auto pixel_data = convertSDFData(sdf);
      texSDF = hrTexture2DCreateFromMemory(sdf.width, sdf.height, sizeof(pixel_data[0]), pixel_data.data());  
    }

    // @TODO: store shaders in a string and create temp file
    std::filesystem::path sdf_shader_path;
    if(a_createInfo->mode == VTEX_MODE::VTEX_SDF)
      sdf_shader_path = "E:/repos/hydra/HydraAPI/hydra_api/data/sdf_texture.c";
    else if (a_createInfo->mode == VTEX_MODE::VTEX_MSDF)
      sdf_shader_path = "E:/repos/hydra/HydraAPI/hydra_api/data/msdf_texture.c";

    pugi::xml_node proc_tex_node;
    HRTextureNodeRef texProc = hrTextureCreateAdvanced(L"proc", L"SDF");
    hrTextureNodeOpen(texProc, HR_WRITE_DISCARD);
    {
      proc_tex_node = hrTextureParamNode(texProc);
    
      auto code_node = proc_tex_node.append_child(L"code");
      code_node.append_attribute(L"file") = sdf_shader_path.wstring().c_str();
      code_node.append_attribute(L"main") = L"main";
    }
    hrTextureNodeClose(texProc);

    *texNode = proc_tex_node.append_child(L"temp_args");
    // proc texture node for binding to material
    auto p1 = texNode->append_child(L"arg");
    auto p2 = texNode->append_child(L"arg");
    auto p3 = texNode->append_child(L"arg");

    // @TODO: colors, texture array, etc.
    p1.append_attribute(L"id") = 0;
    p1.append_attribute(L"name") = L"sdfTexture";
    p1.append_attribute(L"type") = L"sampler2D";
    p1.append_attribute(L"size") = 1;
    p1.append_attribute(L"val")  = std::to_wstring(texSDF.id).c_str();

    p2.append_attribute(L"id")   = 1;
    p2.append_attribute(L"name") = L"texScale";
    p2.append_attribute(L"type") = L"float2";
    p2.append_attribute(L"size") = 1;
    p2.append_attribute(L"val")  = L"1 1";

    p3.append_attribute(L"id")   = 2;
    p3.append_attribute(L"name") = L"inColor";
    p3.append_attribute(L"type") = L"float4";
    p3.append_attribute(L"size") = 1;
    p3.append_attribute(L"val")  = L"1 0 1 1";

    return texProc;
  }
}