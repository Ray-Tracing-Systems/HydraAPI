#include "HRExtensions_VectorTex.h"
#include "LiteMath.h"
#include "HydraLegacyUtils.h"
#include "HydraRenderDriverAPI.h"
#include "pugixml.hpp"

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

      if (contour.winding() < 0)
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

    SDFData result;
    for (NSVGshape* s = a_image->shapes; s != NULL; s = s->next)
    {
      auto shape = buildShapeFromBezierCurves(s);
      shape.normalize();
      msdfgen::Shape::Bounds b = { };
      b = shape.getBounds();
      result.bounds.push_back(std::move(b));

      if (a_settings.mode == VTEX_MODE::VTEX_SDF)
      {
        msdfgen::Bitmap<float, 1> sdf(a_image->width, a_image->height);
        msdfgen::generateSDF(sdf, shape, proj, a_settings.sdfRange, sdfConf);
        result.sdfs.push_back(std::move(sdf));
      }
      else if (a_settings.mode == VTEX_MODE::VTEX_MSDF)
      {
        edgeColoringByDistance(shape, a_settings.sdfAngThres);
        msdfgen::Bitmap<float, 3> msdf(a_image->width, a_image->height);
        msdfgen::generateMSDF(msdf, shape, proj, a_settings.sdfRange, msdfConf);
        result.sdfs.push_back(std::move(msdf));
      }
      
      result.flat_colors.push_back(s->fill.color);
    }

    return result;
  }

  void mergeSDFs(const NSVGimage* a_image, const vtexInfo& a_settings, SDFData& a_sdfData)
  {
    assert(a_sdfData.sdfs.size() == a_sdfData.bounds.size());
    
    if (a_settings.mode == VTEX_MODE::VTEX_SDF)
    {
      a_sdfData.combinedSDF = msdfgen::Bitmap<float, 1>(a_image->width, a_image->height);
    }
    else if (a_settings.mode == VTEX_MODE::VTEX_MSDF)
    {
      a_sdfData.combinedSDF = msdfgen::Bitmap<float, 3>(a_image->width, a_image->height);
    }

    for (size_t y = 0; y < a_image->height; ++y)
    {
      for (size_t x = 0; x < a_image->width; ++x)
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

      auto bottom = static_cast<int>(a_image->height - std::floorf(b.t));
      auto top    = static_cast<int>(a_image->height - std::fmaxf(0.0f, std::floorf(b.b)));
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
    struct NSVGrasterizer* rast = nsvgCreateRasterizer();
    unsigned char* data = new unsigned char[a_image->width * a_image->height * 4];

    nsvgRasterize(rast, a_image, 0, 0, 1, data, a_image->width, a_image->height, a_image->width * 4);

    return hrTexture2DCreateFromMemory(a_image->width, a_image->height, 4, data);
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
      texSDF = hrTexture2DCreateFromMemory(image->width, image->height, sizeof(float), sdf.pixels);
    }
    else if (a_createInfo->mode == VTEX_MODE::VTEX_MSDF)
    {
      const auto& sdf = msdfgen::BitmapRef<float, 3>(std::get<1>(sdfData.combinedSDF));
      texSDF = hrTexture2DCreateFromMemory(image->width, image->height, sizeof(float) * 3, sdf.pixels);
    }

    // @TODO: add define for the shaders directory!
    std::filesystem::path sdf_shader_path;
    if(a_createInfo->mode == VTEX_MODE::VTEX_SDF)
      sdf_shader_path = "E:/repos/hydra/HydraAPI/hydra_api/data/sdf_texture.c";
    else if (a_createInfo->mode == VTEX_MODE::VTEX_MSDF)
      sdf_shader_path = "E:/repos/hydra/HydraAPI/hydra_api/data/msdf_texture.c";

    HRTextureNodeRef texProc = hrTextureCreateAdvanced(L"proc", L"SDF");
    hrTextureNodeOpen(texProc, HR_WRITE_DISCARD);
    {
      auto texNode = hrTextureParamNode(texProc);
    
      auto code_node = texNode.append_child(L"code");
      code_node.append_attribute(L"file") = sdf_shader_path.wstring().c_str();
      code_node.append_attribute(L"main") = L"main";
    }
    hrTextureNodeClose(texProc);


    // proc texture node for binding to material
    auto p1 = texNode->append_child(L"arg");
    auto p2 = texNode->append_child(L"arg");
    auto p3 = texNode->append_child(L"arg");

    // @TODO: colors, texture array, etc.
    p1.append_attribute(L"id") = 0;
    p1.append_attribute(L"name") = L"sdfTexture";
    p1.append_attribute(L"type") = L"sampler2D";
    p1.append_attribute(L"size") = 1;
    p1.append_attribute(L"val")  = std::to_wstring(texProc.id).c_str();

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

    return ref;
  }

  void test_sdfgen_2()
  {
    const std::filesystem::path svgPath = "E:/repos/hydra/clsp2/ru_alph.svg";
    constexpr float DPI = 78.0f;
    const msdfgen::ErrorCorrectionConfig errCorr(msdfgen::ErrorCorrectionConfig::EDGE_PRIORITY, msdfgen::ErrorCorrectionConfig::CHECK_DISTANCE_AT_EDGE);
    const msdfgen::MSDFGeneratorConfig msdfConf(true, errCorr);
    const msdfgen::GeneratorConfig sdfConf(true);
    const msdfgen::Projection proj(1.0f, 0.0f);

    auto image = nsvgParseFromFile(svgPath.string().c_str(), "px", DPI);
    if (!image)
    {
      std::cout << "Could not open SVG image: " << svgPath << std::endl;
    }
    
    //int i = 0;
    std::vector<msdfgen::Bitmap<float, 1>> sdfs;
    std::vector<msdfgen::Bitmap<float, 3>> msdfs;
    std::vector<msdfgen::Shape::Bounds> bounds;
    std::vector<unsigned int> flat_colors;
    int i = 0;

    msdfgen::Shape totalShape;
    totalShape.contours.clear();
    totalShape.inverseYAxis = true;
    for (NSVGshape* s = image->shapes; s != NULL; s = s->next)
    {
      auto shape = buildShapeFromBezierCurves(s);
      shape.normalize();
      msdfgen::Shape::Bounds b = { };
      b = shape.getBounds();
      bounds.push_back(std::move(b));

      /*msdfgen::Bitmap<float, 1> sdf(image->width, image->height);
      msdfgen::generateSDF(sdf, shape, proj, 4.0, sdfConf);
      sdfs.push_back(std::move(sdf));*/
      edgeColoringByDistance(shape, 3.0);
      //edgeColoringSimple(shape, 3.0);
      msdfgen::Bitmap<float, 3> msdf(image->width, image->height);
      msdfgen::generateMSDF(msdf, shape, proj, 4.0, msdfConf);
      msdfs.push_back(std::move(msdf));

      flat_colors.push_back(s->fill.color);
      /*std::string outfile = std::to_string(i++) + ".png";
      savePng(msdf, outfile.c_str());*/

     // addBezierCurvesToShape(s, totalShape);
    }

    totalShape.normalize();

    //msdfgen::Bitmap<float, 3> msdf(image->width, image->height);
    //msdfgen::generateMSDF(msdf, totalShape, proj, 4.0, msdfConf);
    //msdfs.push_back(std::move(msdf));

    //msdfgen::Bitmap<float, 1> sdf(image->width, image->height);
    //msdfgen::generateSDF(sdf, totalShape, proj, 4.0, sdfConf);
    //savePng(sdf, "SDF.png");
    
    msdfgen::Bitmap<float, 3> distance(image->width, image->height );
    msdfgen::Bitmap<byte, 4> instances(image->width, image->height);
    for (size_t y = 0; y < distance.height(); ++y)
    {
      for (size_t x = 0; x < distance.width(); ++x)
      {
        //*distance(x, y)  = 1.0f;

        distance(x, y)[0] = 1000.0f;
        distance(x, y)[1] = 1000.0f;
        distance(x, y)[2] = 1000.0f;

        instances(x, y)[0] = 0;
        instances(x, y)[1] = 0;
        instances(x, y)[2] = 0;
        instances(x, y)[3] = 0;
      }
    }

    for (uint32_t idx = 0; idx < msdfs.size(); ++idx)//
    {
      //auto& sdf  = sdfs[idx];
      auto& msdf = msdfs[idx];
      auto b = bounds[idx];
      /*int bottom = std::fmaxf(0.0f, std::floorf(b.b));
      int top = std::floorf(b.t);*/
      int bottom = image->height - std::floorf(b.t);
      int top = image->height - std::fmaxf(0.0f, std::floorf(b.b));
      
      int left = std::fmaxf(0.0f, std::floorf(b.l));
      int right = std::floorf(b.r);
      std::cout << bottom << " " << left << " " << top << " " << right << std::endl;
      for (size_t y = bottom; y < top; ++y)
      {
        for (size_t x = left; x < right; ++x)
        {
          float r = msdf(x, y)[0];
          float g = msdf(x, y)[1];
          float b = msdf(x, y)[2];
         // if (r < distance(x, y)[0])
          {
            distance(x, y)[0] = r;
            memcpy(instances(x, y), &idx, sizeof(uint32_t));
          }
        //  if (g < distance(x, y)[1])
          {
            distance(x, y)[1] = g;
          }
       //   if (b < distance(x, y)[2])
          {
            distance(x, y)[2] = b;
          }
        }
      }
      auto tmp = std::to_string(idx) + ".png";
      //savePng(sdf, tmp.c_str());
      savePng(msdf, tmp.c_str());
    }
    
    //  for (uint32_t idx = 0; idx < sdfs.size(); ++idx)
   /* for (uint32_t idx = 0; idx < msdfs.size(); ++idx)
    {
      //auto& sdf  = sdfs[idx];
      auto& msdf = msdfs[idx];
      auto color = flat_colors[idx];
      for (size_t y = 0; y < distance.height(); ++y)
      {
        for (size_t x = 0; x < distance.width(); ++x)
        {
          float r = msdf(x, y)[0];
          float g = msdf(x, y)[1];
          float b = msdf(x, y)[2];
          if (r < distance(x, y)[0])
          {
            distance(x, y)[0] = r;
            memcpy(instances(x, y), &idx, sizeof(uint32_t));
          }
          if (g < distance(x, y)[1])
          {
            distance(x, y)[1] = g;
          }
          if (b < distance(x, y)[2])
          {
            distance(x, y)[2] = b;
          }
          //float r = msdf(x, y)[0];
          //float g = msdf(x, y)[1];
          //float b = msdf(x, y)[2];
          //distance(x, y)[0] = r;
          //distance(x, y)[1] = g;
          //distance(x, y)[2] = b;
          //memcpy(instances(x, y), &idx, sizeof(uint32_t));
        }
      }
      auto tmp = std::to_string(idx) + ".png";
      //savePng(sdf, tmp.c_str());
      savePng(msdf, tmp.c_str());
    }*/

    struct NSVGrasterizer* rast = nsvgCreateRasterizer();
    unsigned char* data = new unsigned char[image->width * image->height * 4];
    msdfgen::Bitmap<byte, 4> raster(image->width, image->height);
    msdfgen::BitmapRef<byte, 4> ref = raster;
    // Rasterize
    nsvgRasterize(rast, image, 0, 0, 1, data, image->width, image->height, image->width * 4);
    //stbi_write_png("raster.png", image->width, image->height, 4, data, image->width * 4);

    savePng(distance, "distance.png");
    savePng(instances, "instances.png");
  }
}