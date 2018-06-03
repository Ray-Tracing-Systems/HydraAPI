
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

    auto back = lightNode.append_child(L"back");
    hrTextureBind(texBack, back);

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

bool check_test_163(const wchar_t* a_path)
{
  pugi::xml_document doc;
  doc.load_file(a_path);

  pugi::xml_node libTex = doc.child(L"textures_lib");

  pugi::xml_node tex1 = libTex.find_child_by_attribute(L"id", L"2");
  pugi::xml_node tex2 = libTex.find_child_by_attribute(L"id", L"4");

  int32_t rw1 = tex1.attribute(L"rwidth").as_int(); // 1024, not 512 due to max mip level == 4
  int32_t rh1 = tex1.attribute(L"rheight").as_int();

  int32_t rw2 = tex2.attribute(L"rwidth").as_int();
  int32_t rh2 = tex2.attribute(L"rheight").as_int();

  return (rw1 == 1024) && (rh1 == 1024) && (rw2 == 2048) && (rh2 == 2048);
}


bool MTL_TESTS::test_163_diffuse_texture_recommended_res()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_163");

  hrSceneLibraryOpen(L"tests_f/test_163", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMaterialRef matR     = hrMaterialCreate(L"matR");
  HRMaterialRef matG     = hrMaterialCreate(L"matG");
  HRMaterialRef matB     = hrMaterialCreate(L"matB");
  HRMaterialRef matGray  = hrMaterialCreate(L"matGray");
  HRMaterialRef matClose = hrMaterialCreate(L"matClose");


  HRTextureNodeRef texFloor    = hrTexture2DCreateFromFile(L"data/textures/diff.jpg");
  HRTextureNodeRef texChecker  = hrTexture2DCreateFromFile(L"data/textures_gen/texture_big_z07.png");
  HRTextureNodeRef texChecker2 = hrTexture2DCreateFromFile(L"data/textures/checker_8x8.gif");
  HRTextureNodeRef texChecker3 = hrTexture2DCreateFromFile(L"data/textures/height_map1.png");


  hrMaterialOpen(matR, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matR);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.95 0.05 0.05");
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

    hrTextureBind(texChecker2, color);

    auto texNode = color.child(L"texture");
    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"clamp");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"clamp");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

		VERIFY_XML(matNode);
  }
  hrMaterialClose(matR);

  hrMaterialOpen(matG, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matG);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.05 0.95 0.05");
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

    hrTextureBind(texChecker3, color);

    auto texNode = color.child(L"texture");
    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 4, 0, 0, 0,
                                0, 4, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

		VERIFY_XML(matNode);
  }
  hrMaterialClose(matG);

  hrMaterialOpen(matB, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matB);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.05 0.05 0.95");
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

		auto texNode = hrTextureBind(texChecker, color);

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 4, 0, 0, 0,
                                0, 4, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

		VERIFY_XML(matNode);
  }
  hrMaterialClose(matB);

  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");
    color.append_attribute(L"tex_apply_mode").set_value(L"replace");

		auto texNode = hrTextureBind(texFloor, color);

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
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

  hrMaterialOpen(matClose, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matClose);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");
    color.append_attribute(L"tex_apply_mode").set_value(L"replace");

		auto texNode = hrTextureBind(texChecker3, color);

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 0.1, 0, 0, 0.65,
                                0, 0.1, 0, 0.65,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

		VERIFY_XML(matNode);
  }
  hrMaterialClose(matClose);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR    = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matR.id);
  HRMeshRef sphereG  = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matG.id);
  HRMeshRef torusB   = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matB.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(10.0f), matGray.id);

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

    sizeNode.append_attribute(L"half_length").set_value(L"8.0");
    sizeNode.append_attribute(L"half_width").set_value(L"8.0");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"4.0");
		VERIFY_XML(lightNode);
  }
  hrLightClose(rectLight);

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
    camNode.append_child(L"position").text().set(L"0 13 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 1024);

  hrRenderOpen(renderRef, HR_OPEN_EXISTING);
  auto node = hrRenderParamNode(renderRef);
  node.append_child(L"scenePrepass").text() = 1;
  hrRenderClose(renderRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Create scene
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  float matrixT[4][4];
  float mTranslate[4][4];
  float mRes[4][4];
  float mRes2[4][4];
  float mRot[4][4];

  const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, -1.0f, 0.0f);
  mat4x4_mul(mRes2, mTranslate, mRes);
  mat4x4_transpose(matrixT, mRes2);
  hrMeshInstance(scnRef, planeRef, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);
  mat4x4_identity(mRot);

  mat4x4_translate(mTranslate, -10.0f, 4.0f, 3.0f);
  mat4x4_rotate_X(mRot, mRot, 45.0f*DEG_TO_RAD);
  mat4x4_mul(mRes2, mTranslate, mRot);
  mat4x4_transpose(matrixT, mRes2);

  int32_t remap[2] = { matGray.id,  matClose.id };
  hrMeshInstance(scnRef, planeRef, &matrixT[0][0], remap, 2);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);
  mat4x4_identity(mRot);

  mat4x4_translate(mTranslate, -4.0f, 1.0f, 4.0f);
  mat4x4_rotate_Y(mRot, mRot, 60.0f*DEG_TO_RAD);
  mat4x4_mul(mRes2, mTranslate, mRot);
  mat4x4_transpose(matrixT, mRes2);

  hrMeshInstance(scnRef, cubeR, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);
  mat4x4_identity(mRot);

  mat4x4_translate(mTranslate, 4.0f, 1.0f, 4.5f);
  mat4x4_rotate_Y(mRot, mRot, -60.0f*DEG_TO_RAD);
  mat4x4_rotate_X(mRot, mRot, 80.0f*DEG_TO_RAD);
  mat4x4_mul(mRes2, mTranslate, mRot);
  mat4x4_transpose(matrixT, mRes2); //swap rows and columns

  hrMeshInstance(scnRef, torusB, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, 2.0f, -1.0f);
  mat4x4_mul(mRes2, mTranslate, mRes);
  mat4x4_transpose(matrixT, mRes2); //swap rows and columns

  hrMeshInstance(scnRef, sphereG, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_translate(mTranslate, 0, 12.0f, 0);
  mat4x4_transpose(matrixT, mTranslate);

  hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

  ///////////

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef);

  glViewport(0, 0, 512, 512);
  std::vector<int32_t> image(512 * 512);

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      hrRenderGetFrameBufferLDR1i(renderRef, 512, 512, &image[0]);

      glDisable(GL_TEXTURE_2D);
      glDrawPixels(512, 512, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);

      glfwSwapBuffers(g_window);
      glfwPollEvents();
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_163/z_out.png");

  return check_images("test_163", 1, 60) && check_test_163(L"tests_f/test_163/statex_00001_fixed.xml");
}


bool check_test_168(const wchar_t* a_path)
{
  pugi::xml_document doc;
  doc.load_file(a_path);

  pugi::xml_node libTex = doc.child(L"textures_lib");

  pugi::xml_node tex1 = libTex.find_child_by_attribute(L"id", L"2");
  pugi::xml_node tex2 = libTex.find_child_by_attribute(L"id", L"4");
  pugi::xml_node tex3 = libTex.find_child_by_attribute(L"id", L"5");

  int32_t rw1 = tex1.attribute(L"rwidth").as_int();
  int32_t rh1 = tex1.attribute(L"rheight").as_int();

  int32_t rw2 = tex2.attribute(L"rwidth").as_int();
  int32_t rh2 = tex2.attribute(L"rheight").as_int();

  int32_t rw3 = tex3.attribute(L"rwidth").as_int();
  int32_t rh3 = tex3.attribute(L"rheight").as_int();

  return (rw1 == 1024) && (rh1 == 1024) && (rw2 == 2048) && (rh2 == 2048) && (rw3 == 708) && (rh3 == 643);
}


bool MTL_TESTS::test_168_diffuse_texture_recommended_res2()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_168");

  hrSceneLibraryOpen(L"tests_f/test_168", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMaterialRef matR     = hrMaterialCreate(L"matR");
  HRMaterialRef matG     = hrMaterialCreate(L"matG");
  HRMaterialRef matB     = hrMaterialCreate(L"matB");
  HRMaterialRef matGray  = hrMaterialCreate(L"matGray");
  HRMaterialRef matClose = hrMaterialCreate(L"matClose");
  HRMaterialRef matBack  = hrMaterialCreate(L"matBack");


  HRTextureNodeRef texFloor    = hrTexture2DCreateFromFile(L"data/textures/diff.jpg");
  HRTextureNodeRef texChecker  = hrTexture2DCreateFromFile(L"data/textures_gen/texture_big_z07.png");
  HRTextureNodeRef texChecker2 = hrTexture2DCreateFromFile(L"data/textures/checker_8x8.gif");
  HRTextureNodeRef texChecker3 = hrTexture2DCreateFromFile(L"data/textures/height_map1.png");
  HRTextureNodeRef texChecker4 = hrTexture2DCreateFromFile(L"data/textures/blur_pattern.bmp");

  hrMaterialOpen(matR, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matR);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.95 0.05 0.05");
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

    hrTextureBind(texChecker2, color);

    auto texNode = color.child(L"texture");
    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"clamp");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"clamp");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

		VERIFY_XML(matNode);
  }
  hrMaterialClose(matR);

  hrMaterialOpen(matG, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matG);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.05 0.95 0.05");
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

    hrTextureBind(texChecker3, color);

    auto texNode = color.child(L"texture");
    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 4, 0, 0, 0,
                                0, 4, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

		VERIFY_XML(matNode);
  }
  hrMaterialClose(matG);

  hrMaterialOpen(matB, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matB);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.05 0.05 0.95");
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

		auto texNode = hrTextureBind(texChecker, color);

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 4, 0, 0, 0,
                                0, 4, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

		VERIFY_XML(matNode);
  }
  hrMaterialClose(matB);

  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");
    color.append_attribute(L"tex_apply_mode").set_value(L"replace");

		auto texNode = hrTextureBind(texFloor, color);

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
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

  hrMaterialOpen(matClose, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matClose);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");
    color.append_attribute(L"tex_apply_mode").set_value(L"replace");

		auto texNode = hrTextureBind(texChecker3, color);

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 0.1, 0, 0, 0.65,
                                0, 0.1, 0, 0.65,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

		VERIFY_XML(matNode);
  }
  hrMaterialClose(matClose);


  hrMaterialOpen(matBack, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matBack);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");
    color.append_attribute(L"tex_apply_mode").set_value(L"replace");

		auto texNode = hrTextureBind(texChecker4, color);

    // texNode.append_attribute(L"matrix");
    // float samplerMatrix[16] = { 0.1, 0, 0, 0.65,
    //                             0, 0.1, 0, 0.65,
    //                             0, 0, 1, 0,
    //                             0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    //HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

		VERIFY_XML(matNode);
  }
  hrMaterialClose(matBack);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR    = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matR.id);
  HRMeshRef sphereG  = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matG.id);
  HRMeshRef torusB   = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matB.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(10.0f), matGray.id);

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

    sizeNode.append_attribute(L"half_length").set_value(L"8.0");
    sizeNode.append_attribute(L"half_width").set_value(L"8.0");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"4.0");
		VERIFY_XML(lightNode);
  }
  hrLightClose(rectLight);

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
    camNode.append_child(L"position").text().set(L"0 13 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 1024);

  hrRenderOpen(renderRef, HR_OPEN_EXISTING);
  auto node = hrRenderParamNode(renderRef);
  node.append_child(L"scenePrepass").text() = 1;
  hrRenderClose(renderRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Create scene
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  float matrixT[4][4];
  float mTranslate[4][4];
  float mRes[4][4];
  float mRes2[4][4];
  float mRot[4][4];

  const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, -1.0f, 0.0f);
  mat4x4_mul(mRes2, mTranslate, mRes);
  mat4x4_transpose(matrixT, mRes2);
  hrMeshInstance(scnRef, planeRef, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);
  mat4x4_identity(mRot);

  mat4x4_translate(mTranslate, -10.0f, 4.0f, 3.0f);
  mat4x4_rotate_X(mRot, mRot, 45.0f*DEG_TO_RAD);
  mat4x4_mul(mRes2, mTranslate, mRot);
  mat4x4_transpose(matrixT, mRes2);

  int32_t remap[2] = { matGray.id,  matClose.id };
  hrMeshInstance(scnRef, planeRef, &matrixT[0][0], remap, 2);


  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);
  mat4x4_identity(mRot);

  mat4x4_translate(mTranslate, +10.0f, -40.0f, 3.0f);
  mat4x4_rotate_X(mRot, mRot, 45.0f*DEG_TO_RAD);
  mat4x4_mul(mRes2, mTranslate, mRot);
  mat4x4_transpose(matrixT, mRes2);

  int32_t remap2[2] = { matGray.id,  matBack.id };
  hrMeshInstance(scnRef, planeRef, &matrixT[0][0], remap2, 2);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);
  mat4x4_identity(mRot);

  mat4x4_translate(mTranslate, -4.0f, 1.0f, 4.0f);
  mat4x4_rotate_Y(mRot, mRot, 60.0f*DEG_TO_RAD);
  mat4x4_mul(mRes2, mTranslate, mRot);
  mat4x4_transpose(matrixT, mRes2);

  hrMeshInstance(scnRef, cubeR, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);
  mat4x4_identity(mRot);

  mat4x4_translate(mTranslate, 4.0f, 1.0f, 4.5f);
  mat4x4_rotate_Y(mRot, mRot, -60.0f*DEG_TO_RAD);
  mat4x4_rotate_X(mRot, mRot, 80.0f*DEG_TO_RAD);
  mat4x4_mul(mRes2, mTranslate, mRot);
  mat4x4_transpose(matrixT, mRes2); //swap rows and columns

  hrMeshInstance(scnRef, torusB, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, 2.0f, -1.0f);
  mat4x4_mul(mRes2, mTranslate, mRes);
  mat4x4_transpose(matrixT, mRes2); //swap rows and columns

  hrMeshInstance(scnRef, sphereG, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_translate(mTranslate, 0, 12.0f, 0);
  mat4x4_transpose(matrixT, mTranslate);

  hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

  ///////////

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef);

  /*glViewport(0, 0, 512, 512);
  std::vector<int32_t> image(512 * 512);

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      hrRenderGetFrameBufferLDR1i(renderRef, 512, 512, &image[0]);

      glDisable(GL_TEXTURE_2D);
      glDrawPixels(512, 512, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);

      glfwSwapBuffers(g_window);
      glfwPollEvents();
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_168/z_out.png");
  */

  hrRenderCommand(renderRef, L"exitnow", nullptr);

  return check_test_168(L"tests_f/test_168/statex_00001_fixed.xml"); // check_images("test_168", 1, 30); // &&
}
