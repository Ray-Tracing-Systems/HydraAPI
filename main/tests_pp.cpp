#include "tests.h"
#include <math.h>
#include <iomanip>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include <math.h>
#include "linmath.h"

#if defined(WIN32)
#include <FreeImage.h>
#include <GLFW/glfw3.h>
#pragma comment(lib, "glfw3dll.lib")
#pragma comment(lib, "FreeImage.lib")
#else
#include <FreeImage.h>
#include <GLFW/glfw3.h>
#endif

#include "mesh_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sstream>

#include "../hydra_api/HR_HDRImageTool.h"
#include "../hydra_api/HydraPostProcessAPI.h"
#include "../hydra_api/HydraXMLHelpers.h"

using namespace TEST_UTILS;

bool PP_TESTS::test301_resample()
{
  HRFBIRef image1 = hrFBICreateFromFile(L"data/textures/texture1.bmp");

  int w, h;
  hrFBIGetData(image1, &w, &h, nullptr);

  HRFBIRef image2 = hrFBICreate(L"temp", w / 2, h / 2, 16);

  hrFilterApply(L"resample",  pugi::xml_node(), HRRenderRef(), // empty settimgs and render, will resample to width and height of the second image
                L"in_color",  image1,            
                L"out_color", image2);        

  hrFBISaveToFile(image2, L"tests_images/test_301/z_out.png");

  return check_images("test_301");
}

bool PP_TESTS::test302_median()
{
  HRFBIRef image1 = hrFBICreateFromFile(L"data/textures/texture1_nosy.bmp");

  int w, h;
  hrFBIGetData(image1, &w, &h, nullptr);

  HRFBIRef image2 = hrFBICreate(L"temp", w, h, 16);

  pugi::xml_document docSettings;
  pugi::xml_node settings = docSettings.append_child(L"settings");

  settings.append_attribute(L"threshold") = 0.2f; // for LDR images is like 0.02*255 = 5 bit differense; for HDR is just 0.02

  hrFilterApply(L"median",    settings, HRRenderRef(),
                L"in_color",  image1,            
                L"out_color", image2);        

  hrFBISaveToFile(image2, L"tests_images/test_302/z_out.png");

  return check_images("test_302");
}

bool PP_TESTS::test303_median_in_place()
{
  HRFBIRef image1 = hrFBICreateFromFile(L"data/textures/texture1_nosy.bmp");

  pugi::xml_document docSettings;
  pugi::xml_node settings = docSettings.append_child(L"settings");

  settings.append_attribute(L"threshold") = 0.2f; // for LDR images is like 0.02*255 = 5 bit difference; for HDR is just 0.02

  hrFilterApply(L"median",    settings, HRRenderRef(), //
                L"in_color",  image1,                  // image1; special case (!!!); median filter can work in place; Most filers can not!
                L"out_color", image1);                 // image1; special case (!!!); median filter can work in place; Most filers can not!

  hrFBISaveToFile(image1, L"tests_images/test_303/z_out.png");

  return check_images("test_303");
}

bool PP_TESTS::test304_obsolete_tone_mapping()
{
  HRFBIRef image1 = hrFBICreateFromFile(L"data/textures/building.hdr");

  int w, h;
  hrFBIGetData(image1, &w, &h, nullptr);

  HRFBIRef image2 = hrFBICreate(L"temp", w, h, 16);

  pugi::xml_document docSettings;
  pugi::xml_node settings = docSettings.append_child(L"settings");

  settings.append_attribute(L"low")      = -3.0f; 
  settings.append_attribute(L"high")     = 7.5f;
  settings.append_attribute(L"exposure") = 3.0f;
  settings.append_attribute(L"defog")    = 0.0f;

  hrFilterApply(L"tonemapping_obsolette", settings, HRRenderRef(),
                L"in_color",  image1,              
                L"out_color", image2);             

  hrFBISaveToFile(image2, L"tests_images/test_304/z_out.png");

  return check_images("test_304");
}

bool PP_TESTS::test320_blur()
{
  HRFBIRef image1 = hrFBICreateFromFile(L"data/textures/texture1.bmp");

  int w, h;
  hrFBIGetData(image1, &w, &h, nullptr);

  HRFBIRef image2 = hrFBICreate(L"temp", w / 2, h / 2, 16);

  pugi::xml_document docSettings;
  pugi::xml_node settings = docSettings.append_child(L"settings");

  settings.append_attribute(L"radius") = 7;
  settings.append_attribute(L"sigma")  = 2.0f;

  hrFilterApply(L"blur",      settings, HRRenderRef(), // empty settimgs and render, will resample to width and height of the second image
                L"in_color",  image1,
                L"out_color", image2);

  hrFBISaveToFile(image2, L"tests_images/test_320/z_out.png");

  return false; // this test is not ready due to boundaries does not processed now
}

bool PP_TESTS::test321_median_mostly_bad_pixels()
{
  HRFBIRef image1 = hrFBICreateFromFile(L"data/textures/texture1_nosy.bmp");

  int w, h;
  hrFBIGetData(image1, &w, &h, nullptr);

  HRFBIRef image2 = hrFBICreate(L"temp", w, h, 16);

  pugi::xml_document docSettings;
  pugi::xml_node settings = docSettings.append_child(L"settings");

  settings.append_attribute(L"threshold")  = 0.2f; // for LDR images is like 0.02*255 = 5 bit differense; for HDR is just 0.02
  settings.append_attribute(L"pixels_num") = 100;

  hrFilterApply(L"median_n", settings, HRRenderRef(),
                L"in_color", image1,
                L"out_color", image2);

  hrFBISaveToFile(image2, L"tests_images/test_321/z_out.png");

  return false;
  //return check_images("test_321");
}

bool PP_TESTS::test306_post_process_hydra1_exposure05()
{
  HRFBIRef image1 = hrFBICreateFromFile(L"data/textures/kitchen.hdr");

  int w, h;
  hrFBIGetData(image1, &w, &h, nullptr);

  HRFBIRef image2 = hrFBICreate(L"temp", w, h, 16);

  pugi::xml_document docSettings;
  pugi::xml_node settings = docSettings.append_child(L"settings");

  settings.append_attribute(L"numThreads") = 0;
  settings.append_attribute(L"exposure") = 0.5f;
  settings.append_attribute(L"compress") = 0.0f;
  settings.append_attribute(L"contrast") = 1.0f;
  settings.append_attribute(L"saturation") = 1.0f;
  settings.append_attribute(L"whiteBalance") = 0.0f;
  settings.append_attribute(L"uniformContrast") = 0.0f;
  settings.append_attribute(L"normalize") = 0.0f;
  settings.append_attribute(L"vignette") = 0.0f;
  settings.append_attribute(L"chromAberr") = 0.0f;

  hrFilterApply(L"post_process_hydra1", settings, HRRenderRef(),
                L"in_color", image1,
                L"out_color", image2);

  hrFBISaveToFile(image2, L"tests_images/test_306/z_out.png");

  return check_images("test_306");
}

bool PP_TESTS::test307_post_process_hydra1_exposure2()
{
  HRFBIRef image1 = hrFBICreateFromFile(L"data/textures/kitchen.hdr");

  int w, h;
  hrFBIGetData(image1, &w, &h, nullptr);

  HRFBIRef image2 = hrFBICreate(L"temp", w, h, 16);

  pugi::xml_document docSettings;
  pugi::xml_node settings = docSettings.append_child(L"settings");

  settings.append_attribute(L"numThreads") = 0;
  settings.append_attribute(L"exposure") = 2.0f;
  settings.append_attribute(L"compress") = 0.0f;
  settings.append_attribute(L"contrast") = 1.0f;
  settings.append_attribute(L"saturation") = 1.0f;
  settings.append_attribute(L"whiteBalance") = 0.0f;
  settings.append_attribute(L"uniformContrast") = 0.0f;
  settings.append_attribute(L"normalize") = 0.0f;
  settings.append_attribute(L"vignette") = 0.0f;
  settings.append_attribute(L"chromAberr") = 0.0f;

  hrFilterApply(L"post_process_hydra1", settings, HRRenderRef(),
                L"in_color", image1,
                L"out_color", image2);

  hrFBISaveToFile(image2, L"tests_images/test_307/z_out.png");

  return check_images("test_307");
}

bool PP_TESTS::test308_post_process_hydra1_compress()
{
  HRFBIRef image1 = hrFBICreateFromFile(L"data/textures/kitchen.hdr");

  int w, h;
  hrFBIGetData(image1, &w, &h, nullptr);

  HRFBIRef image2 = hrFBICreate(L"temp", w, h, 16);

  pugi::xml_document docSettings;
  pugi::xml_node settings = docSettings.append_child(L"settings");

  settings.append_attribute(L"numThreads") = 0;
  settings.append_attribute(L"exposure") = 1.0f;
  settings.append_attribute(L"compress") = 1.0f;
  settings.append_attribute(L"contrast") = 1.0f;
  settings.append_attribute(L"saturation") = 1.0f;
  settings.append_attribute(L"whiteBalance") = 0.0f;
  settings.append_attribute(L"uniformContrast") = 0.0f;
  settings.append_attribute(L"normalize") = 0.0f;
  settings.append_attribute(L"vignette") = 0.0f;
  settings.append_attribute(L"chromAberr") = 0.0f;

  hrFilterApply(L"post_process_hydra1", settings, HRRenderRef(),
                L"in_color", image1,
                L"out_color", image2);

  hrFBISaveToFile(image2, L"tests_images/test_308/z_out.png");

  return check_images("test_308");
}

bool PP_TESTS::test309_post_process_hydra1_contrast()
{
  HRFBIRef image1 = hrFBICreateFromFile(L"data/textures/kitchen.hdr");

  int w, h;
  hrFBIGetData(image1, &w, &h, nullptr);

  HRFBIRef image2 = hrFBICreate(L"temp", w, h, 16);

  pugi::xml_document docSettings;
  pugi::xml_node settings = docSettings.append_child(L"settings");

  settings.append_attribute(L"numThreads") = 0;
  settings.append_attribute(L"exposure") = 1.0f;
  settings.append_attribute(L"compress") = 0.0f;
  settings.append_attribute(L"contrast") = 2.0f;
  settings.append_attribute(L"saturation") = 1.0f;
  settings.append_attribute(L"whiteBalance") = 0.0f;
  settings.append_attribute(L"uniformContrast") = 0.0f;
  settings.append_attribute(L"normalize") = 0.0f;
  settings.append_attribute(L"vignette") = 0.0f;
  settings.append_attribute(L"chromAberr") = 0.0f;

  hrFilterApply(L"post_process_hydra1", settings, HRRenderRef(),
                L"in_color", image1,
                L"out_color", image2);

  hrFBISaveToFile(image2, L"tests_images/test_309/z_out.png");

  return check_images("test_309");
}

bool PP_TESTS::test310_post_process_hydra1_desaturation()
{
  HRFBIRef image1 = hrFBICreateFromFile(L"data/textures/kitchen.hdr");

  int w, h;
  hrFBIGetData(image1, &w, &h, nullptr);

  HRFBIRef image2 = hrFBICreate(L"temp", w, h, 16);

  pugi::xml_document docSettings;
  pugi::xml_node settings = docSettings.append_child(L"settings");

  settings.append_attribute(L"numThreads") = 0;
  settings.append_attribute(L"exposure") = 1.0f;
  settings.append_attribute(L"compress") = 0.0f;
  settings.append_attribute(L"contrast") = 1.0f;
  settings.append_attribute(L"saturation") = 0.0f;
  settings.append_attribute(L"whiteBalance") = 0.0f;
  settings.append_attribute(L"uniformContrast") = 0.0f;
  settings.append_attribute(L"normalize") = 0.0f;
  settings.append_attribute(L"vignette") = 0.0f;
  settings.append_attribute(L"chromAberr") = 0.0f;

  hrFilterApply(L"post_process_hydra1", settings, HRRenderRef(),
                L"in_color", image1,
                L"out_color", image2);

  hrFBISaveToFile(image2, L"tests_images/test_310/z_out.png");

  return check_images("test_310");
}

bool PP_TESTS::test311_post_process_hydra1_saturation()
{
  HRFBIRef image1 = hrFBICreateFromFile(L"data/textures/kitchen.hdr");

  int w, h;
  hrFBIGetData(image1, &w, &h, nullptr);

  HRFBIRef image2 = hrFBICreate(L"temp", w, h, 16);

  pugi::xml_document docSettings;
  pugi::xml_node settings = docSettings.append_child(L"settings");

  settings.append_attribute(L"numThreads") = 0;
  settings.append_attribute(L"exposure") = 1.0f;
  settings.append_attribute(L"compress") = 0.0f;
  settings.append_attribute(L"contrast") = 1.0f;
  settings.append_attribute(L"saturation") = 2.0f;
  settings.append_attribute(L"whiteBalance") = 0.0f;
  settings.append_attribute(L"uniformContrast") = 0.0f;
  settings.append_attribute(L"normalize") = 0.0f;
  settings.append_attribute(L"vignette") = 0.0f;
  settings.append_attribute(L"chromAberr") = 0.0f;

  hrFilterApply(L"post_process_hydra1", settings, HRRenderRef(),
                L"in_color", image1,
                L"out_color", image2);

  hrFBISaveToFile(image2, L"tests_images/test_311/z_out.png");

  return check_images("test_311");
}

bool PP_TESTS::test312_post_process_hydra1_whiteBalance()
{
  HRFBIRef image1 = hrFBICreateFromFile(L"data/textures/kitchen.hdr");

  int w, h;
  hrFBIGetData(image1, &w, &h, nullptr);

  HRFBIRef image2 = hrFBICreate(L"temp", w, h, 16);

  pugi::xml_document docSettings;
  pugi::xml_node settings = docSettings.append_child(L"settings");

  settings.append_attribute(L"numThreads") = 0;
  settings.append_attribute(L"exposure") = 1.0f;
  settings.append_attribute(L"compress") = 0.0f;
  settings.append_attribute(L"contrast") = 1.0f;
  settings.append_attribute(L"saturation") = 1.0f;
  settings.append_attribute(L"whiteBalance") = 1.0f;
  settings.append_attribute(L"uniformContrast") = 0.0f;
  settings.append_attribute(L"normalize") = 0.0f;
  settings.append_attribute(L"vignette") = 0.0f;
  settings.append_attribute(L"chromAberr") = 0.0f;

  hrFilterApply(L"post_process_hydra1", settings, HRRenderRef(),
                L"in_color", image1,
                L"out_color", image2);

  hrFBISaveToFile(image2, L"tests_images/test_312/z_out.png");

  return check_images("test_312");
}

bool PP_TESTS::test312_2_post_process_hydra1_whitePointColor()
{
  HRFBIRef image1 = hrFBICreateFromFile(L"data/textures/kitchen.hdr");

  int w, h;
  hrFBIGetData(image1, &w, &h, nullptr);

  HRFBIRef image2 = hrFBICreate(L"temp", w, h, 16);

  pugi::xml_document docSettings;
  pugi::xml_node settings = docSettings.append_child(L"settings");

  settings.append_attribute(L"numThreads") = 0;
  settings.append_attribute(L"exposure") = 1.0f;
  settings.append_attribute(L"compress") = 0.0f;
  settings.append_attribute(L"contrast") = 1.0f;
  settings.append_attribute(L"saturation") = 1.0f;
  settings.append_attribute(L"whiteBalance") = 1.0f;
  settings.append_attribute(L"whitePointColor");
  float colorArr[3] = { 0.36f, 0.32f, 0.27f };
  HydraXMLHelpers::WriteFloat3(settings.attribute(L"whitePointColor"), colorArr);

  settings.append_attribute(L"uniformContrast") = 0.0f;
  settings.append_attribute(L"normalize") = 0.0f;
  settings.append_attribute(L"vignette") = 0.0f;
  settings.append_attribute(L"chromAberr") = 0.0f;

  hrFilterApply(L"post_process_hydra1", settings, HRRenderRef(),
                L"in_color", image1,
                L"out_color", image2);

  hrFBISaveToFile(image2, L"tests_images/test_312_2/z_out.png");

  return check_images("test_312_2");
}

bool PP_TESTS::test313_post_process_hydra1_uniformContrast()
{
  HRFBIRef image1 = hrFBICreateFromFile(L"data/textures/kitchen.hdr");

  int w, h;
  hrFBIGetData(image1, &w, &h, nullptr);

  HRFBIRef image2 = hrFBICreate(L"temp", w, h, 16);

  pugi::xml_document docSettings;
  pugi::xml_node settings = docSettings.append_child(L"settings");

  settings.append_attribute(L"numThreads") = 0;
  settings.append_attribute(L"exposure") = 1.0f;
  settings.append_attribute(L"compress") = 0.0f;
  settings.append_attribute(L"contrast") = 1.0f;
  settings.append_attribute(L"saturation") = 1.0f;
  settings.append_attribute(L"whiteBalance") = 0.0f;
  settings.append_attribute(L"uniformContrast") = 1.0f;
  settings.append_attribute(L"normalize") = 0.0f;
  settings.append_attribute(L"vignette") = 0.0f;
  settings.append_attribute(L"chromAberr") = 0.0f;

  hrFilterApply(L"post_process_hydra1", settings, HRRenderRef(),
                L"in_color", image1,
                L"out_color", image2);

  hrFBISaveToFile(image2, L"tests_images/test_313/z_out.png");

  return check_images("test_313");
}

bool PP_TESTS::test314_post_process_hydra1_normalize()
{
  HRFBIRef image1 = hrFBICreateFromFile(L"data/textures/kitchen.hdr");

  int w, h;
  hrFBIGetData(image1, &w, &h, nullptr);

  HRFBIRef image2 = hrFBICreate(L"temp", w, h, 16);

  pugi::xml_document docSettings;
  pugi::xml_node settings = docSettings.append_child(L"settings");

  settings.append_attribute(L"numThreads") = 0;
  settings.append_attribute(L"exposure") = 1.0f;
  settings.append_attribute(L"compress") = 1.0f;
  settings.append_attribute(L"contrast") = 1.0f;
  settings.append_attribute(L"saturation") = 1.0f;
  settings.append_attribute(L"whiteBalance") = 0.0f;
  settings.append_attribute(L"uniformContrast") = 0.0f;
  settings.append_attribute(L"normalize") = 1.0f;
  settings.append_attribute(L"vignette") = 0.0f;
  settings.append_attribute(L"chromAberr") = 0.0f;

  hrFilterApply(L"post_process_hydra1", settings, HRRenderRef(),
                L"in_color", image1,
                L"out_color", image2);

  hrFBISaveToFile(image2, L"tests_images/test_314/z_out.png");

  return check_images("test_314");
}

bool PP_TESTS::test315_post_process_hydra1_vignette()
{
  HRFBIRef image1 = hrFBICreateFromFile(L"data/textures/kitchen.hdr");

  int w, h;
  hrFBIGetData(image1, &w, &h, nullptr);

  HRFBIRef image2 = hrFBICreate(L"temp", w, h, 16);

  pugi::xml_document docSettings;
  pugi::xml_node settings = docSettings.append_child(L"settings");

  settings.append_attribute(L"numThreads") = 0;
  settings.append_attribute(L"exposure") = 1.0f;
  settings.append_attribute(L"compress") = 0.0f;
  settings.append_attribute(L"contrast") = 1.0f;
  settings.append_attribute(L"saturation") = 1.0f;
  settings.append_attribute(L"whiteBalance") = 0.0f;
  settings.append_attribute(L"uniformContrast") = 0.0f;
  settings.append_attribute(L"normalize") = 0.0f;
  settings.append_attribute(L"vignette") = 1.0f;
  settings.append_attribute(L"chromAberr") = 0.0f;

  hrFilterApply(L"post_process_hydra1", settings, HRRenderRef(),
                L"in_color", image1,
                L"out_color", image2);

  hrFBISaveToFile(image2, L"tests_images/test_315/z_out.png");

  return check_images("test_315");
}

bool PP_TESTS::test316_post_process_hydra1_chromAberr()
{
  HRFBIRef image1 = hrFBICreateFromFile(L"data/textures/wire01.hdr");

  int w, h;
  hrFBIGetData(image1, &w, &h, nullptr);

  HRFBIRef image2 = hrFBICreate(L"temp", w, h, 16);

  pugi::xml_document docSettings;
  pugi::xml_node settings = docSettings.append_child(L"settings");

  settings.append_attribute(L"numThreads") = 0;
  settings.append_attribute(L"exposure") = 1.0f;
  settings.append_attribute(L"compress") = 0.0f;
  settings.append_attribute(L"contrast") = 1.0f;
  settings.append_attribute(L"saturation") = 1.0f;
  settings.append_attribute(L"whiteBalance") = 0.0f;
  settings.append_attribute(L"uniformContrast") = 0.0f;
  settings.append_attribute(L"normalize") = 0.0f;
  settings.append_attribute(L"vignette") = 0.0f;
  settings.append_attribute(L"chromAberr") = 1.0f;

  hrFilterApply(L"post_process_hydra1", settings, HRRenderRef(),
                L"in_color", image1,
                L"out_color", image2);

  hrFBISaveToFile(image2, L"tests_images/test_316/z_out.png");

  return check_images("test_316");
}

bool PP_TESTS::test317_post_process_hydra1_sharpness()
{
  HRFBIRef image1 = hrFBICreateFromFile(L"data/textures/kitchen.hdr");

  int w, h;
  hrFBIGetData(image1, &w, &h, nullptr);

  HRFBIRef image2 = hrFBICreate(L"temp", w, h, 16);

  pugi::xml_document docSettings;
  pugi::xml_node settings = docSettings.append_child(L"settings");

  settings.append_attribute(L"numThreads") = 0;
  settings.append_attribute(L"exposure") = 1.0f;
  settings.append_attribute(L"compress") = 0.0f;
  settings.append_attribute(L"contrast") = 1.0f;
  settings.append_attribute(L"saturation") = 1.0f;
  settings.append_attribute(L"whiteBalance") = 0.0f;
  settings.append_attribute(L"uniformContrast") = 0.0f;
  settings.append_attribute(L"normalize") = 0.0f;
  settings.append_attribute(L"sharpness") = 1.0f;
  settings.append_attribute(L"vignette") = 0.0f;
  settings.append_attribute(L"chromAberr") = 0.0f;

  
  hrFilterApply(L"post_process_hydra1", settings, HRRenderRef(),
                L"in_color", image1,
                L"out_color", image2);

  hrFBISaveToFile(image2, L"tests_images/test_317/z_out.png");

  return check_images("test_317");
}

bool PP_TESTS::test318_post_process_hydra1_ECCSWUNSVC()
{
  HRFBIRef image1 = hrFBICreateFromFile(L"data/textures/kitchen.hdr");

  int w, h;
  hrFBIGetData(image1, &w, &h, nullptr);

  HRFBIRef image2 = hrFBICreate(L"temp", w, h, 16);

  pugi::xml_document docSettings;
  pugi::xml_node settings = docSettings.append_child(L"settings");

  settings.append_attribute(L"numThreads") = 0;
  settings.append_attribute(L"exposure") = 1.5f;
  settings.append_attribute(L"compress") = 0.9f;
  settings.append_attribute(L"contrast") = 1.5f;
  settings.append_attribute(L"saturation") = 1.2f;
  settings.append_attribute(L"whiteBalance") = 0.5f;
  settings.append_attribute(L"uniformContrast") = 0.2f;
  settings.append_attribute(L"normalize") = 1.0f;
  settings.append_attribute(L"sharpness") = 1.0f;
  settings.append_attribute(L"vignette") = 0.3f;
  settings.append_attribute(L"chromAberr") = 0.5f;


  hrFilterApply(L"post_process_hydra1", settings, HRRenderRef(),
                L"in_color", image1,
                L"out_color", image2);

  hrFBISaveToFile(image2, L"tests_images/test_318/z_out.png");

  return check_images("test_318");
}

bool PP_TESTS::test319_post_process_hydra1_diffStars()
{
  HRFBIRef image1 = hrFBICreateFromFile(L"data/textures/10.hdr");

  int w, h;
  hrFBIGetData(image1, &w, &h, nullptr);

  HRFBIRef image2 = hrFBICreate(L"temp", w, h, 16);

  pugi::xml_document docSettings;
  pugi::xml_node settings = docSettings.append_child(L"settings");

  settings.append_attribute(L"numThreads") = 0;
  settings.append_attribute(L"exposure") = 1.0f;
  settings.append_attribute(L"compress") = 1.0f;
  settings.append_attribute(L"contrast") = 1.0f;
  settings.append_attribute(L"saturation") = 1.0f;
  settings.append_attribute(L"whiteBalance") = 0.0f;
  settings.append_attribute(L"uniformContrast") = 0.0f;
  settings.append_attribute(L"normalize") = 0.0f;
  settings.append_attribute(L"sharpness") = 0.0f;

  // ----- Optics effects -----
  settings.append_attribute(L"vignette") = 0.0f;
  settings.append_attribute(L"chromAberr") = 0.0f;

  settings.append_attribute(L"diffStars_sizeStar") = 100.0f;
  settings.append_attribute(L"diffStars_numRay") = 8;
  settings.append_attribute(L"diffStars_rotateRay") = 20;
  settings.append_attribute(L"diffStars_randomAngle") = 0.2f;
  settings.append_attribute(L"diffStars_sprayRay") = 0.3f;


  hrFilterApply(L"post_process_hydra1", settings, HRRenderRef(),
                L"in_color", image1,
                L"out_color", image2);

  hrFBISaveToFile(image2, L"tests_images/test_319/z_out.png");

  return check_images("test_319");
}

extern GLFWwindow* g_window;

static inline int RealColorToUint32(const float real_color[4])
{
  float  r = real_color[0] * 255.0f;
  float  g = real_color[1] * 255.0f;
  float  b = real_color[2] * 255.0f;
  float  a = real_color[3] * 255.0f;

  unsigned char red   = (unsigned char)r;
  unsigned char green = (unsigned char)g;
  unsigned char blue  = (unsigned char)b;
  unsigned char alpha = (unsigned char)a;

  return red | (green << 8) | (blue << 16) | (alpha << 24);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PP_TESTS::test305_fbi_from_render()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_305");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_305", HR_WRITE_DISCARD);

  SimpleMesh cube     = CreateCube(0.75f);
  SimpleMesh plane    = CreatePlane(10.0f);
  SimpleMesh sphere   = CreateSphere(1.0f, 32);
  SimpleMesh torus    = CreateTorus(0.25f, 0.6f, 32, 32);
  SimpleMesh cubeOpen = CreateCubeOpen(4.0f);

  for (size_t i = 0; i < plane.vTexCoord.size(); i++)
    plane.vTexCoord[i] *= 2.0f;


  HRTextureNodeRef testTex2 = hrTexture2DCreateFromFileDL(L"data/textures/chess_red.bmp");

  HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
  HRMaterialRef mat1 = hrMaterialCreate(L"mysimplemat2");
  HRMaterialRef mat2 = hrMaterialCreate(L"mysimplemat3");
  HRMaterialRef mat3 = hrMaterialCreate(L"mysimplemat4");
  HRMaterialRef mat4 = hrMaterialCreate(L"myblue");
  HRMaterialRef mat5 = hrMaterialCreate(L"mymatplane");

  HRMaterialRef mat6 = hrMaterialCreate(L"red");
  HRMaterialRef mat7 = hrMaterialCreate(L"green");
  HRMaterialRef mat8 = hrMaterialCreate(L"white");

  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat0);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"0.5 0.75 0.5");

    HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp"); // hrTexture2DCreateFromFileDL
    hrTextureBind(testTex, diff);
  }
  hrMaterialClose(mat0);

  hrMaterialOpen(mat1, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat1);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"0.207843 0.188235 0");

    // hrTextureBind(testTex2, diff);

    xml_node refl = matNode.append_child(L"reflectivity");

    refl.append_attribute(L"brdf_type").set_value(L"phong");
    refl.append_child(L"color").text().set(L"0.367059 0.345882 0");
    refl.append_child(L"glossiness").text().set(L"0.5");
    //refl.append_child(L"fresnel_IOR").text().set(L"1.5");
    //refl.append_child(L"fresnel").text().set(L"1");

  }
  hrMaterialClose(mat1);

  hrMaterialOpen(mat2, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat2);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"0.75 0.75 0.75");

    HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/relief_wood.jpg");
    hrTextureBind(testTex, diff);
  }
  hrMaterialClose(mat2);

  hrMaterialOpen(mat3, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat3);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"0.75 0.75 0.75");

    HRTextureNodeRef testTex = hrTexture2DCreateFromFileDL(L"data/textures/163.jpg");
    hrTextureBind(testTex, diff);
  }
  hrMaterialClose(mat3);

  hrMaterialOpen(mat4, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat4);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"0.1 0.1 0.75");
  }
  hrMaterialClose(mat4);

  hrMaterialOpen(mat5, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat5);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"0.75 0.75 0.25");

    HRTextureNodeRef testTex = hrTexture2DCreateFromFileDL(L"data/textures/texture1.bmp");
    hrTextureBind(testTex, diff);
  }
  hrMaterialClose(mat5);


  hrMaterialOpen(mat6, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat6);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"0.5 0.0 0.0");
  }
  hrMaterialClose(mat6);

  hrMaterialOpen(mat7, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat7);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"0.0 0.5 0.0");
  }
  hrMaterialClose(mat7);

  hrMaterialOpen(mat8, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat8);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"0.5 0.5 0.5");
  }
  hrMaterialClose(mat8);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMeshRef teapotRef   = hrMeshCreateFromFileDL(L"data/meshes/teapot.vsgf"); // chunk_00009.vsgf // teapot.vsgf // chunk_00591.vsgf

  HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
  HRMeshRef planeRef    = hrMeshCreate(L"my_plane");
  HRMeshRef sphereRef   = hrMeshCreate(L"my_sphere");
  HRMeshRef torusRef    = hrMeshCreate(L"my_torus");

  hrMeshOpen(cubeOpenRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeOpenRef, L"pos", &cubeOpen.vPos[0]);
    hrMeshVertexAttribPointer4f(cubeOpenRef, L"norm", &cubeOpen.vNorm[0]);
    hrMeshVertexAttribPointer2f(cubeOpenRef, L"texcoord", &cubeOpen.vTexCoord[0]);

    int cubeMatIndices[10] = { mat8.id, mat8.id, mat8.id, mat8.id, mat8.id, mat8.id, mat7.id, mat7.id, mat6.id, mat6.id };

    hrMeshPrimitiveAttribPointer1i(cubeOpenRef, L"mind", cubeMatIndices);
    hrMeshAppendTriangles3(cubeOpenRef, int(cubeOpen.triIndices.size()), &cubeOpen.triIndices[0]);
  }
  hrMeshClose(cubeOpenRef);


  hrMeshOpen(planeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(planeRef, L"pos", &plane.vPos[0]);
    hrMeshVertexAttribPointer4f(planeRef, L"norm", &plane.vNorm[0]);
    hrMeshVertexAttribPointer2f(planeRef, L"texcoord", &plane.vTexCoord[0]);

    hrMeshMaterialId(planeRef, mat5.id);
    hrMeshAppendTriangles3(planeRef, int32_t(plane.triIndices.size()), &plane.triIndices[0]);
  }
  hrMeshClose(planeRef);

  hrMeshOpen(sphereRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(sphereRef, L"pos", &sphere.vPos[0]);
    hrMeshVertexAttribPointer4f(sphereRef, L"norm", &sphere.vNorm[0]);
    hrMeshVertexAttribPointer2f(sphereRef, L"texcoord", &sphere.vTexCoord[0]);

    for (size_t i = 0; i < sphere.matIndices.size() / 2; i++)
      sphere.matIndices[i] = mat0.id;

    for (size_t i = sphere.matIndices.size() / 2; i < sphere.matIndices.size(); i++)
      sphere.matIndices[i] = mat2.id;

    hrMeshPrimitiveAttribPointer1i(sphereRef, L"mind", &sphere.matIndices[0]);
    hrMeshAppendTriangles3(sphereRef, int32_t(sphere.triIndices.size()), &sphere.triIndices[0]);
  }
  hrMeshClose(sphereRef);

  hrMeshOpen(torusRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(torusRef, L"pos", &torus.vPos[0]);
    hrMeshVertexAttribPointer4f(torusRef, L"norm", &torus.vNorm[0]);
    hrMeshVertexAttribPointer2f(torusRef, L"texcoord", &torus.vTexCoord[0]);

    for (size_t i = 0; i < torus.matIndices.size() / 3; i++)
      torus.matIndices[i] = mat0.id;

    for (size_t i = 1 * torus.matIndices.size() / 3; i < 2 * torus.matIndices.size() / 3; i++)
      torus.matIndices[i] = mat3.id;

    for (size_t i = 2 * torus.matIndices.size() / 3; i < torus.matIndices.size(); i++)
      torus.matIndices[i] = mat2.id;

    //hrMeshMaterialId(torusRef, mat0.id);
    hrMeshPrimitiveAttribPointer1i(torusRef, L"mind", &torus.matIndices[0]);
    hrMeshAppendTriangles3(torusRef, int32_t(torus.triIndices.size()), &torus.triIndices[0]);
  }
  hrMeshClose(torusRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRLightRef rectLight = hrLightCreate(L"my_area_light");

  hrLightOpen(rectLight, HR_WRITE_DISCARD);
  {
    pugi::xml_node lightNode = hrLightParamNode(rectLight);

    lightNode.attribute(L"type").set_value(L"area");
    lightNode.attribute(L"shape").set_value(L"rect");
    lightNode.attribute(L"distribution").set_value(L"diffuse");

    pugi::xml_node sizeNode = lightNode.append_child(L"size");

    sizeNode.append_attribute(L"half_length") = 1.0f;
    sizeNode.append_attribute(L"half_width")  = 1.0f;

    pugi::xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").text().set(L"1 1 1");
    intensityNode.append_child(L"multiplier").text().set(L"10.0");
  }
  hrLightClose(rectLight);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // camera
  //
  HRCameraRef camRef = hrCameraCreate(L"my camera");

  hrCameraOpen(camRef, HR_WRITE_DISCARD);
  {
    xml_node camNode = hrCameraParamNode(camRef);

    camNode.append_child(L"fov").text().set(L"45");
    camNode.append_child(L"nearClipPlane").text().set(L"0.01");
    camNode.append_child(L"farClipPlane").text().set(L"100.0");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 0 15");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  // set up render settings
  //
  HRRenderRef renderRef = hrRenderCreate(L"HydraModern"); // opengl1

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  auto pList = hrRenderGetDeviceList(renderRef);

  while (pList != nullptr)
  {
    std::wcout << L"device id = " << pList->id << L", name = " << pList->name << L", driver = " << pList->driver << std::endl;
    pList = pList->next;
  }

  //hrRenderEnableDevice(renderRef, 0, true);
  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);

    node.append_child(L"width").text() = L"1024";
    node.append_child(L"height").text() = L"768";

    node.append_child(L"method_primary").text()   = L"pathtracing";
    node.append_child(L"method_secondary").text() = L"pathtracing";
    node.append_child(L"method_tertiary").text()  = L"pathtracing";
    node.append_child(L"method_caustic").text()   = L"pathtracing";
    node.append_child(L"shadows").text()          = L"1";

    node.append_child(L"trace_depth").text()      = L"5";
    node.append_child(L"diff_trace_depth").text() = L"3";

    node.append_child(L"pt_error").text()        = L"2";
    node.append_child(L"minRaysPerPixel").text() = L"256";
    node.append_child(L"maxRaysPerPixel").text() = L"1024";
  }
  hrRenderClose(renderRef);

  // create scene
  //
  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  static GLfloat	rtri = 25.0f; // Angle For The Triangle ( NEW )
  static GLfloat	rquad = 40.0f;
  static float    g_FPS = 60.0f;
  static int      frameCounter = 0;

  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

  float matrixT[4][4];
  float mRot1[4][4], mTranslate[4][4], mRes[4][4];
  //float mTranslateDown[4][4];

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  int mmIndex = 0;
  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, -0.70f*3.65f, 0.0f); 
  mat4x4_scale(mRot1, mRot1, 3.65f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT, mRes); // this fucking math library swap rows and columns
  matrixT[3][3] = 1.0f;

  hrMeshInstance(scnRef, teapotRef, &matrixT[0][0]);

  mat4x4_identity(mRot1);
  mat4x4_rotate_Y(mRot1, mRot1, 180.0f*DEG_TO_RAD);
  //mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD);
  mat4x4_transpose(matrixT, mRot1);
  hrMeshInstance(scnRef, cubeOpenRef, &matrixT[0][0]);

  /////////////////////////////////////////////////////////////////////// instance light (!!!)

  mat4x4_identity(mTranslate);
  mat4x4_translate(mTranslate, 0, 3.85f, 0);
  mat4x4_transpose(matrixT, mTranslate);
  hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef);

  HRFBIRef frameBufferImage = hrFBICreate(L"temp", 1024, 768, 16);

  glViewport(0, 0, 1024, 768);
  std::vector<int32_t> imageDataLDR(1024 * 768);
  
  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  
    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);
  
    if (info.haveUpdateFB)
    {
      hrRenderCopyFrameBufferToFBI(renderRef, L"color", 
                                  frameBufferImage);         

      int w, h, bpp;
      const float* data = (const float*)hrFBIGetData(frameBufferImage, 
                                                     &w, &h, &bpp); // color is always must be HDR, so bpp == 16 and this is float4 image
      
      const float invGamma = 1.0f / 2.2f;
      
      for (int i = 0; i < w*h; i++)
      {
        const float r = data[i * 4 + 0];
        const float g = data[i * 4 + 1];
        const float b = data[i * 4 + 2];
        const float a = 1.0f;
      
        const float color[4] = { 0.5f*fminf(powf(r, invGamma), 1.0f),   // these 1.0f coeffitients are just for example, you may try to change them
                                 1.0f*fminf(powf(g, invGamma), 1.0f),   // these 1.0f coeffitients are just for example, you may try to change them
                                 1.0f*fminf(powf(b, invGamma), 1.0f),   // these 1.0f coeffitients are just for example, you may try to change them
                                 a
        };
      
        imageDataLDR[i] = RealColorToUint32(color);
      }
  
      glDisable(GL_TEXTURE_2D);
      glDrawPixels(1024, 768, GL_RGBA, GL_UNSIGNED_BYTE, &imageDataLDR[0]);
  
      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);
  
      glfwSwapBuffers(g_window);
      glfwPollEvents();
    }
  
    if (info.finalUpdate)
      break;
  }

  hrFBISaveToFile(frameBufferImage, L"tests_images/test_305/z_out.png");

  return check_images("test_305", 1, 20.0f);
}
