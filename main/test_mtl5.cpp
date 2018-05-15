
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
#include "../hydra_api/LiteMath.h"
#include "../hydra_api/HydraXMLHelpers.h"

using namespace TEST_UTILS;

extern GLFWwindow* g_window;

using HydraLiteMath::float2;
using HydraLiteMath::float3;
using HydraLiteMath::float4;

namespace hlm = HydraLiteMath;


bool MTL_TESTS::test_162_shadow_matte_back1()
{
  const int IMG_WIDTH  = 1024;
  const int IMG_HEIGHT = 682;

  initGLIfNeeded();

  hrErrorCallerPlace(L"test_162");

  hrSceneLibraryOpen(L"tests_f/test_162", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMaterialRef matR = hrMaterialCreate(L"matR");
  HRMaterialRef matG = hrMaterialCreate(L"matG");
  HRMaterialRef matB = hrMaterialCreate(L"matB");
  HRMaterialRef matBG = hrMaterialCreate(L"matBG");
  HRMaterialRef matGray = hrMaterialCreate(L"matGray");

  HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
  HRTextureNodeRef texPattern = hrTexture2DCreateFromFile(L"data/textures/blur_pattern.bmp");
  HRTextureNodeRef texBack    = hrTexture2DCreateFromFile(L"data/textures/0019.jpg");

  hrMaterialOpen(matR, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matR);

    auto diff = matNode.append_child(L"diffuse");

    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.15 0.15");

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"phong");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.4 0.4 0.4");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(0);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matR);

  hrMaterialOpen(matG, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matG);
    matNode.attribute(L"type").set_value(L"shadow_catcher");

    auto opacity = matNode.append_child(L"opacity");
    opacity.append_child(L"skip_shadow").append_attribute(L"val").set_value(1);

    auto back = matNode.append_child(L"back");
    back.append_attribute(L"reflection") = 1;

    auto texNode = hrTextureBind(texBack, back);
    texNode.append_attribute(L"input_gamma") = 2.2f;

    auto shadow = matNode.append_child(L"shadow");
    shadow.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.0 0.5");

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matG);

  hrMaterialOpen(matB, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matB);
    auto diff = matNode.append_child(L"diffuse");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 1.0");

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matB);

  hrMaterialOpen(matBG, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matBG);

    auto diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.25 0.25 1.0");

    auto texNode = hrTextureBind(texChecker, color);

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 32, 0, 0, 0,
                                0, 16, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };
    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matBG);

  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");

    auto texNode = hrTextureBind(texChecker, color);

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 16, 0, 0, 0,
                                 0, 16, 0, 0,
                                 0, 0, 1, 0,
                                 0, 0, 0, 1 };
    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matGray);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(1.0f, 64), matR.id);
  HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(1.0f, 64), matR.id);
  HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(1.0f, 64), matR.id);
  HRMeshRef boxBG = HRMeshFromSimpleMesh(L"boxBG", CreateCube(1.0f), matBG.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);
  HRMeshRef planeRef2 = HRMeshFromSimpleMesh(L"my_plane2", CreatePlane(20.0f), matG.id);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Light
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRLightRef rectLight = hrLightCreate(L"my_area_light");

  hrLightOpen(rectLight, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(rectLight);

    lightNode.attribute(L"type").set_value(L"area");
    lightNode.attribute(L"shape").set_value(L"rect");
    lightNode.attribute(L"distribution").set_value(L"diffuse");

    auto sizeNode = lightNode.append_child(L"size");

    sizeNode.append_attribute(L"half_length").set_value(L"2.0");
    sizeNode.append_attribute(L"half_width").set_value(L"2.0");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"20.0");
    VERIFY_XML(lightNode);
  }
  hrLightClose(rectLight);

  HRLightRef sky = hrLightCreate(L"sky");

  hrLightOpen(sky, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sky);

    lightNode.attribute(L"type").set_value(L"sky");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.05 0.05 0.25");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);

    VERIFY_XML(lightNode);
  }
  hrLightClose(sky);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Camera
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRCameraRef camRef = hrCameraCreate(L"my camera");

  hrCameraOpen(camRef, HR_WRITE_DISCARD);
  {
    auto camNode = hrCameraParamNode(camRef);

    camNode.append_child(L"fov").text().set(L"45");
    camNode.append_child(L"nearClipPlane").text().set(L"0.01");
    camNode.append_child(L"farClipPlane").text().set(L"100.0");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 2.25 6"); 
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPTNoCaust(CURR_RENDER_DEVICE, IMG_WIDTH, IMG_HEIGHT, 256, 2048);

  hrRenderOpen(renderRef, HR_OPEN_EXISTING);
  {
    auto camNode = hrRenderParamNode(renderRef);
    camNode.force_child(L"evalgbuffer").text() = 1;
  }
  hrRenderClose(renderRef);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Create scene
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  using namespace HydraLiteMath;

  float4x4 mRot;
  float4x4 mTranslate;
  float4x4 mScale;
  float4x4 mRes;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  ///////////

  //mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
  //hrMeshInstance(scnRef, planeRef, mTranslate.L());

  mTranslate = translate4x4(float3(0.0f, -1.75f, 0.0f)); 
  hrMeshInstance(scnRef, planeRef2, mTranslate.L());

  mTranslate.identity();
  mTranslate = translate4x4(float3(0.0f, -0.75f, 0.0f)); 
  hrMeshInstance(scnRef, sph2, mTranslate.L());

  ///////////

  mTranslate.identity();
  mRes.identity();

  mTranslate = translate4x4(float3(0, 10.0f, -5.0)); 
  mRes = mul(mTranslate, mRes);

  hrLightInstance(scnRef, rectLight, mRes.L());
  hrLightInstance(scnRef, sky, mRes.L(), L"prob_mult = \"0.25\"");

  ///////////

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef);

  glViewport(0, 0, IMG_WIDTH, IMG_HEIGHT);
  std::vector<int32_t> image(IMG_WIDTH * IMG_HEIGHT);

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      hrRenderGetFrameBufferLDR1i(renderRef, IMG_WIDTH, IMG_HEIGHT, &image[0]);

      glDisable(GL_TEXTURE_2D);
      glDrawPixels(IMG_WIDTH, IMG_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);

      glfwSwapBuffers(g_window);
      glfwPollEvents();
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_162/z_out.png");

  hrRenderSaveGBufferLayerLDR(renderRef, L"tests_images/test_162/z_out2.png", L"depth");
  hrRenderSaveGBufferLayerLDR(renderRef, L"tests_images/test_162/z_out3.png", L"diffcolor");
  hrRenderSaveGBufferLayerLDR(renderRef, L"tests_images/test_162/z_out4.png", L"alpha");
  hrRenderSaveGBufferLayerLDR(renderRef, L"tests_images/test_162/z_out5.png", L"shadow");

  return check_images("test_162", 5, 25);
}