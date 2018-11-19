//
// Created by vsan on 17.08.18.
//
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
#include <algorithm>

#include "../hydra_api/HR_HDRImageTool.h"
#include "../hydra_api/HydraPostProcessAPI.h"
#include "../hydra_api/LiteMath.h"
#include "../hydra_api/HydraXMLHelpers.h"

using namespace TEST_UTILS;

namespace hlm = HydraLiteMath;
extern GLFWwindow* g_window;

using HydraLiteMath::float2;
using HydraLiteMath::float3;
using HydraLiteMath::float4;




bool test_x1_displace_car_by_noise()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_x1");

  hrSceneLibraryOpen(L"tests/test_x1", HR_WRITE_DISCARD);

 /* auto matShd = hrMaterialCreate(L"shadow");

  hrMaterialOpen(matShd, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matShd);
    matNode.attribute(L"type").set_value(L"shadow_catcher");

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matShd);

  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(100.0f), matShd.id);*/

  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/23_antwerp_night.hdr");
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Light
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRLightRef sky = hrLightCreate(L"sky");

  hrLightOpen(sky, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sky);

    lightNode.attribute(L"type").set_value(L"sky");
    lightNode.attribute(L"distribution").set_value(L"map");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");

    auto texNode = hrTextureBind(texEnv, intensityNode.child(L"color"));

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

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
    camNode.append_child(L"position").text().set(L"4.0 1.5 -2.5");
    camNode.append_child(L"look_at").text().set(L"-0.0 0.85 0.0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  int w = 1024;
  int h = 1024;

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, w, h, 512, 512);
  hrRenderOpen(renderRef, HR_OPEN_EXISTING);
  {
    auto node = hrRenderParamNode(renderRef);

    node.append_child(L"evalDisplacement").text() = true;


  };
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

//  ///////////
//  mTranslate.identity();
//  mRes.identity();
//
//  mTranslate = translate4x4(float3(-15.0f, -40.0f, 0.0f));
//  mScale = scale4x4(float3(10.01f, 10.01f, 10.01f));
//  mRes = mul(mTranslate, mScale);
//
//  hrMeshInstance(scnRef, tess, mRes.L());
//
//  mTranslate = translate4x4(float3(15.0f, -40.0f, 0.0f));
//  mScale = scale4x4(float3(10.01f, 10.01f, 10.01f));
//  mRes = mul(mTranslate, mScale);
//
//  int32_t remapList1[2] = { mat1.id, mat2.id };
//
//  hrMeshInstance(scnRef, tess, mRes.L(), remapList1, sizeof(remapList1)/sizeof(int32_t));

  mTranslate = translate4x4(float3(0.0f, 0.0f, 0.0f));
//  hrMeshInstance(scnRef, planeRef, mTranslate.L());


  ///////////

  mTranslate.identity();
  mRes.identity();

  mTranslate = translate4x4(float3(0, 0.0f, 0.0));
  mRes = mul(mTranslate, mRes);

  hrLightInstance(scnRef, sky, mRes.L());


  ///////////

  hrSceneClose(scnRef);

  std::wstring car_path(L"/CarsLibSeparate/002");

  auto merged_car = MergeLibraryIntoLibrary(car_path.c_str(), false, true);
  hrCommit(scnRef);

  auto materials = HydraXMLHelpers::GetMaterialNameToIdMap();

  int body_mat_id = 0;
  for(auto& mat :materials)
  {
    if(mat.first == "outside_body")
    {
      body_mat_id = mat.second;
      break;
    }
  }

  auto newBodyMat = MergeOneMaterialIntoLibrary(car_path.c_str(), L"outside_body");


  TEST_UTILS::displace_data_1 data1;
  data1.global_dir = float3(0.0f, -1.0f, 0.0f);
  data1.mult = -0.07f;

  auto displace_1 = HRExtensions::hrTextureDisplacementCustom(TEST_UTILS::customDisplacementSpots, &data1, sizeof(data1));

  hrMaterialOpen(newBodyMat, HR_OPEN_EXISTING);
  auto matNode = hrMaterialParamNode(newBodyMat);
  /*auto displacement = matNode.append_child(L"displacement");
  displacement.append_attribute(L"type").set_value(L"true_displacement");
  displacement.append_attribute(L"subdivs").set_value(0);

  auto customNode   = displacement.append_child(L"custom");
  hrTextureBind(displace_1, customNode);*/
 /* auto noiseNode = displ.append_child(L"noise");
  noiseNode.append_attribute(L"scale").set_value(2.0f);
  noiseNode.append_attribute(L"amount").set_value(-0.02f);
  noiseNode.append_attribute(L"base_freq").set_value(4.0f);
  noiseNode.append_attribute(L"octaves").set_value(4);
  noiseNode.append_attribute(L"persistence").set_value(0.75f);
  noiseNode.append_attribute(L"lacunarity").set_value(1.25f);*/
  hrMaterialClose(newBodyMat);

  int32_t remapList1[2] = { body_mat_id, newBodyMat.id };
  mRes.identity();

  auto bbox = HRUtils::InstanceSceneIntoScene(merged_car, scnRef, mRes.L(), false, remapList1, 2);

  hrFlush(scnRef, renderRef, camRef);

  glViewport(0, 0, w, h);
  std::vector<int32_t> image(w * h);

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      hrRenderGetFrameBufferLDR1i(renderRef, w, h, &image[0]);

      glDisable(GL_TEXTURE_2D);
      glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);

      glfwSwapBuffers(g_window);
      glfwPollEvents();
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_x1/z_out.png");

  return check_images("test_x1", 1, 30);
}


bool test_x2_car_displacement_triplanar()
{
    initGLIfNeeded();

    hrErrorCallerPlace(L"test_x2");

    hrSceneLibraryOpen(L"tests/test_x2", HR_WRITE_DISCARD);

    HRTextureNodeRef texX = hrTexture2DCreateFromFile(L"data/textures/blur_pattern.bmp");
    HRTextureNodeRef texY  = hrTexture2DCreateFromFile(L"data/textures/MapleLeaf.TGA");
    HRTextureNodeRef texZ  = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
    HRTextureNodeRef texX2 = hrTexture2DCreateFromFile(L"data/textures/yinyang.png");
    HRTextureNodeRef texY2  = hrTexture2DCreateFromFile(L"data/textures/MapleLeaf.TGA");
    HRTextureNodeRef texZ2  = hrTexture2DCreateFromFile(L"data/textures/yinyang.png");

    HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/kitchen.hdr");
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef sky = hrLightCreate(L"sky");

    hrLightOpen(sky, HR_WRITE_DISCARD);
    {
        auto lightNode = hrLightParamNode(sky);

        lightNode.attribute(L"type").set_value(L"sky");
        lightNode.attribute(L"distribution").set_value(L"map");

        auto intensityNode = lightNode.append_child(L"intensity");

        intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
        intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");

        auto texNode = hrTextureBind(texEnv, intensityNode.child(L"color"));

        texNode.append_attribute(L"matrix");
        float samplerMatrix[16] = { 1, 0, 0, 0,
                                    0, 1, 0, 0,
                                    0, 0, 1, 0,
                                    0, 0, 0, 1 };

        texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
        texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");

        HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

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
        camNode.append_child(L"position").text().set(L"3 5 -1");
        camNode.append_child(L"look_at").text().set(L"-0.0 0.85 0.0");
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    int w = 1024;
    int h = 1024;

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, w, h, 256, 256);
    hrRenderOpen(renderRef, HR_OPEN_EXISTING);
    {
        auto node = hrRenderParamNode(renderRef);
        node.append_child(L"evalDisplacement").text() = true;
    };
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

//  ///////////
//  mTranslate.identity();
//  mRes.identity();
//
//  mTranslate = translate4x4(float3(-15.0f, -40.0f, 0.0f));
//  mScale = scale4x4(float3(10.01f, 10.01f, 10.01f));
//  mRes = mul(mTranslate, mScale);
//
//  hrMeshInstance(scnRef, tess, mRes.L());
//
//  mTranslate = translate4x4(float3(15.0f, -40.0f, 0.0f));
//  mScale = scale4x4(float3(10.01f, 10.01f, 10.01f));
//  mRes = mul(mTranslate, mScale);
//
//  int32_t remapList1[2] = { mat1.id, mat2.id };
//
//  hrMeshInstance(scnRef, tess, mRes.L(), remapList1, sizeof(remapList1)/sizeof(int32_t));

    mTranslate = translate4x4(float3(0.0f, 0.0f, 0.0f));
//  hrMeshInstance(scnRef, planeRef, mTranslate.L());


    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0, 0.0f, 0.0));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, sky, mRes.L());


    ///////////

    hrSceneClose(scnRef);

    std::wstring car_path(L"CarsLibSeparate/002");

    auto merged_car = MergeLibraryIntoLibrary(car_path.c_str(), false, true);
    hrCommit(scnRef);

    auto materials = HydraXMLHelpers::GetMaterialNameToIdMap();

    int body_mat_id = 0;
    for(auto& mat :materials)
    {
        if(mat.first == "outside_body")
        {
            body_mat_id = mat.second;
            break;
        }
    }

    auto newBodyMat = MergeOneMaterialIntoLibrary(car_path.c_str(), L"outside_body");

    hrMaterialOpen(newBodyMat, HR_WRITE_DISCARD);
    {
        xml_node matNode = hrMaterialParamNode(newBodyMat);
        xml_node diff = matNode.append_child(L"diffuse");

        diff.append_attribute(L"brdf_type").set_value(L"lambert");
        auto colorNode = diff.append_child(L"color");

        colorNode.append_attribute(L"val") = L"0.5 0.5 0.5";

        auto displacement = matNode.append_child(L"displacement");
        auto heightNode   = displacement.append_child(L"height_map_triplanar");

        displacement.append_attribute(L"type").set_value(L"true_displacement");
        heightNode.append_attribute(L"amount").set_value(0.035f);
        displacement.append_attribute(L"subdivs").set_value(1);

        auto texNode = heightNode.append_child(L"textures_hexaplanar");

        texNode.append_attribute(L"texX").set_value(texX.id);
        texNode.append_attribute(L"texX2").set_value(texX.id);
        texNode.append_attribute(L"texY").set_value(texX.id);
        texNode.append_attribute(L"texY2").set_value(texX.id);
        texNode.append_attribute(L"texZ").set_value(texX.id);
        texNode.append_attribute(L"texZ2").set_value(texX.id);

        texNode.append_attribute(L"matrix");
        float samplerMatrix[16] = { 1, 0, 0, 0,
                                    0, 1, 0, 0,
                                    0, 0, 1, 0,
                                    0, 0, 0, 1 };
        HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
    }
    hrMaterialClose(newBodyMat);

    int32_t remapList1[2] = { body_mat_id, newBodyMat.id };
    mRes.identity();

    auto bbox = HRUtils::InstanceSceneIntoScene(merged_car, scnRef, mRes.L(), false, remapList1, 2);

    hrFlush(scnRef, renderRef, camRef);

    glViewport(0, 0, w, h);
    std::vector<int32_t> image(w * h);

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

        if (info.haveUpdateFB)
        {
            hrRenderGetFrameBufferLDR1i(renderRef, w, h, &image[0]);

            glDisable(GL_TEXTURE_2D);
            glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

            auto pres = std::cout.precision(2);
            std::cout << "rendering progress = " << info.progress << "% \r";
            std::cout.precision(pres);

            glfwSwapBuffers(g_window);
            glfwPollEvents();
        }

        if (info.finalUpdate)
            break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_x2/z_out.png");

    return check_images("test_x2", 1, 30);
}

bool test_x3_car_fresnel_ice()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_x3");

  hrSceneLibraryOpen(L"tests/test_x3", HR_WRITE_DISCARD);

  HRTextureNodeRef texBump = hrTexture2DCreateFromFile(L"data/textures/noise.png");
  HRMaterialRef mat_diff = hrMaterialCreate(L"bump_diffuse_mat");
  HRMaterialRef mat_mirror = hrMaterialCreate(L"mirror");

  hrMaterialOpen(mat_diff, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat_diff);
    xml_node diff    = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val") = L"0.5 0.5 0.5";

    auto displacement = matNode.append_child(L"displacement");
    auto heightNode   = displacement.append_child(L"height_map");

    displacement.append_attribute(L"type").set_value(L"height_bump");
    heightNode.append_attribute(L"amount").set_value(0.5f);

    auto texNode = hrTextureBind(texBump, heightNode);

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 2, 0, 0, 0,
                                0, 2, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
    VERIFY_XML(matNode);
  }
  hrMaterialClose(mat_diff);

  HRTextureNodeRef texPattern = hrTexture2DCreateFromFile(L"data/textures/blur_pattern.bmp");
  hrMaterialOpen(mat_mirror, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat_mirror);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"phong");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(0);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.0f);

    auto transp = matNode.append_child(L"transparency");

    transp.append_attribute(L"brdf_type").set_value(L"phong");
    transp.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
    transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0);
    transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
    transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
    transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(0.0f);
    transp.append_child(L"ior").append_attribute(L"val").set_value(2.41f);

    auto texNode = hrTextureBind(texPattern, transp.child(L"glossiness"));

    texNode.append_attribute(L"matrix");
    float samplerMatrix1[16] = { 16, 0, 0, 0,
                                0, 16, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };
    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix1);

    auto displacement = matNode.append_child(L"displacement");
    auto heightNode   = displacement.append_child(L"height_map");

    displacement.append_attribute(L"type").set_value(L"height_bump");
    heightNode.append_attribute(L"amount").set_value(1.5f);

    texNode = hrTextureBind(texBump, heightNode);

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 2, 0, 0, 0,
                                0, 2, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
    VERIFY_XML(matNode);
  }
  hrMaterialClose(mat_mirror);


  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/kitchen.hdr");
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Light
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRLightRef sky = hrLightCreate(L"sky");

  hrLightOpen(sky, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sky);

    lightNode.attribute(L"type").set_value(L"sky");
    lightNode.attribute(L"distribution").set_value(L"map");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");

    auto texNode = hrTextureBind(texEnv, intensityNode.child(L"color"));

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

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
    camNode.append_child(L"position").text().set(L"5 1.5 -1");
    camNode.append_child(L"look_at").text().set(L"0.0 0.85 0.0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  int w = 2048;
  int h = 2048;

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, w, h, 1024, 1024);
  hrRenderOpen(renderRef, HR_OPEN_EXISTING);
  {
    auto node = hrRenderParamNode(renderRef);
    node.append_child(L"evalDisplacement").text() = true;
  };
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

//  ///////////
//  mTranslate.identity();
//  mRes.identity();
//
//  mTranslate = translate4x4(float3(-15.0f, -40.0f, 0.0f));
//  mScale = scale4x4(float3(10.01f, 10.01f, 10.01f));
//  mRes = mul(mTranslate, mScale);
//
//  hrMeshInstance(scnRef, tess, mRes.L());
//
//  mTranslate = translate4x4(float3(15.0f, -40.0f, 0.0f));
//  mScale = scale4x4(float3(10.01f, 10.01f, 10.01f));
//  mRes = mul(mTranslate, mScale);
//
//  int32_t remapList1[2] = { mat1.id, mat2.id };
//
//  hrMeshInstance(scnRef, tess, mRes.L(), remapList1, sizeof(remapList1)/sizeof(int32_t));

  mTranslate = translate4x4(float3(0.0f, 0.0f, 0.0f));
//  hrMeshInstance(scnRef, planeRef, mTranslate.L());


  ///////////

  mTranslate.identity();
  mRes.identity();

  mTranslate = translate4x4(float3(0, 0.0f, 0.0));
  mRes = mul(mTranslate, mRes);

  hrLightInstance(scnRef, sky, mRes.L());


  ///////////

  hrSceneClose(scnRef);

  std::wstring car_path(L"CarsLibSeparate/002");

  auto merged_car = MergeLibraryIntoLibrary(car_path.c_str(), false, true);
  hrCommit(scnRef);

  auto materials = HydraXMLHelpers::GetMaterialNameToIdMap();

  int body_mat_id = 0;
  std::vector<int> materials_for_ice;
  std::vector<std::string> materials_for_ice_names = {"outside_elements", "outside_windscreen_glass", "outside_window_glass",
          "outside_mirror", "outside_radiator_screen", "outside_windscreen_wipers", "outside_bottom_cover", "outside_logo",
          "outside_chrome", "outside_license_plate", "outside_black_glass", "outside_body2","light_cover_glass"};

//  std::sort(materials_for_ice_names);

  for(auto& mat :materials)
  {
    if(mat.first == "outside_body")
    {
      body_mat_id = mat.second;
    //  break;
    }
    if(std::find(materials_for_ice_names.begin(), materials_for_ice_names.end(), mat.first) != materials_for_ice_names.end())
    {
      materials_for_ice.push_back(mat.second);
    }

  }

  auto newBodyMat = MergeOneMaterialIntoLibrary(car_path.c_str(), L"outside_body");
  hrMaterialOpen(newBodyMat, HR_OPEN_EXISTING);
  {
    auto matNode = hrMaterialParamNode(newBodyMat);

    auto diff = matNode.child(L"diffuse");
    diff.child(L"color").attribute(L"val").set_value(L"0.0 0.0 0.99");

    VERIFY_XML(matNode);
  }
  hrMaterialClose(newBodyMat);


  HRMaterialRef matBlend = hrMaterialCreateBlend(L"matBlend", newBodyMat, mat_mirror);
  hrMaterialOpen(matBlend, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matBlend);

    auto blend = matNode.append_child(L"blend");
    blend.append_attribute(L"type").set_value(L"fresnel_blend");
    blend.append_child(L"fresnel_ior").append_attribute(L"val") = 8.0f;

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matBlend);

  std::vector<int> remapList;
  for(auto& mat: materials_for_ice)
  {
    HRMaterialRef tmpMat;
    tmpMat.id = mat;

    HRMaterialRef blendIce = hrMaterialCreateBlend(L"blendIce", tmpMat, mat_mirror);
    hrMaterialOpen(blendIce, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(blendIce);
      auto blend = matNode.append_child(L"blend");
      blend.append_attribute(L"type").set_value(L"fresnel_blend");
      blend.append_child(L"fresnel_ior").append_attribute(L"val") = 5.0f;

      VERIFY_XML(matNode);
    }
    hrMaterialClose(blendIce);

    remapList.push_back(tmpMat.id);
    remapList.push_back(blendIce.id);
  }

  remapList.push_back(body_mat_id);
  remapList.push_back(matBlend.id);
//  int32_t remapList1[2] = { body_mat_id, matBlend.id };
  mRes.identity();

  auto bbox = HRUtils::InstanceSceneIntoScene(merged_car, scnRef, mRes.L(), false, &remapList[0], remapList.size());

  hrFlush(scnRef, renderRef, camRef);

  glViewport(0, 0, w, h);
  std::vector<int32_t> image(w * h);

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      hrRenderGetFrameBufferLDR1i(renderRef, w, h, &image[0]);

      glDisable(GL_TEXTURE_2D);
      glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);

      glfwSwapBuffers(g_window);
      glfwPollEvents();
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_x3/z_out.png");

  return check_images("test_x3", 1, 30);
}


bool test_x4_car_triplanar()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_x4");

  hrSceneLibraryOpen(L"tests/test_x4", HR_WRITE_DISCARD);

  HRTextureNodeRef texX = hrTexture2DCreateFromFile(L"data/textures/blur_pattern.bmp");
  HRTextureNodeRef texY  = hrTexture2DCreateFromFile(L"data/textures/MapleLeaf.TGA");
  HRTextureNodeRef texZ  = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
  HRTextureNodeRef texX2 = hrTexture2DCreateFromFile(L"data/textures/yinyang.png");
  HRTextureNodeRef texY2  = hrTexture2DCreateFromFile(L"data/textures/MapleLeaf.TGA");
  HRTextureNodeRef texZ2  = hrTexture2DCreateFromFile(L"data/textures/yinyang.png");

  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/kitchen.hdr");

  HRTextureNodeRef texProc1 = hrTextureCreateAdvanced(L"proc", L"hexaplanar");

  hrTextureNodeOpen(texProc1, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc1);

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/hexaplanar_scalers.c";
    code_node.append_attribute(L"main") = L"main";
  }
  hrTextureNodeClose(texProc1);
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Light
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRLightRef sky = hrLightCreate(L"sky");

  hrLightOpen(sky, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sky);

    lightNode.attribute(L"type").set_value(L"sky");
    lightNode.attribute(L"distribution").set_value(L"map");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");

    auto texNode = hrTextureBind(texEnv, intensityNode.child(L"color"));

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

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
    camNode.append_child(L"position").text().set(L"3 5 -1");
    camNode.append_child(L"look_at").text().set(L"-0.0 0.85 0.0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  int w = 1024;
  int h = 1024;

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, w, h, 256, 256);
  hrRenderOpen(renderRef, HR_OPEN_EXISTING);
  {
    auto node = hrRenderParamNode(renderRef);
    node.append_child(L"evalDisplacement").text() = true;
  };
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

  mTranslate = translate4x4(float3(0.0f, 0.0f, 0.0f));


  ///////////

  mTranslate.identity();
  mRes.identity();

  mTranslate = translate4x4(float3(0, 0.0f, 0.0));
  mRes = mul(mTranslate, mRes);

  hrLightInstance(scnRef, sky, mRes.L());


  ///////////

  hrSceneClose(scnRef);

  std::wstring car_path(L"CarsLibSeparate/002");

  auto merged_car = MergeLibraryIntoLibrary(car_path.c_str(), false, true);
  hrCommit(scnRef);

  auto materials = HydraXMLHelpers::GetMaterialNameToIdMap();

  int body_mat_id = 0;
  for(auto& mat :materials)
  {
    if(mat.first == "outside_body")
    {
      body_mat_id = mat.second;
      break;
    }
  }

  auto newBodyMat = MergeOneMaterialIntoLibrary(car_path.c_str(), L"outside_body");

  hrMaterialOpen(newBodyMat, HR_OPEN_EXISTING);
  {
    xml_node matNode = hrMaterialParamNode(newBodyMat);
    xml_node diff = matNode.child(L"diffuse");
    auto colorNode = diff.child(L"color");

    colorNode.append_attribute(L"tex_apply_mode") = L"replace";

    auto texNode = hrTextureBind(texProc1, colorNode);
    texNode.append_attribute(L"input_gamma") = 1.0f;
//
    xml_node p1 = texNode.append_child(L"arg");
    xml_node p2 = texNode.append_child(L"arg");
    xml_node p3 = texNode.append_child(L"arg");
    xml_node p4 = texNode.append_child(L"arg");
    xml_node p5 = texNode.append_child(L"arg");
    xml_node p6 = texNode.append_child(L"arg");
    xml_node p7 = texNode.append_child(L"arg");
//
    p1.append_attribute(L"id")   = 0;
    p1.append_attribute(L"name") = L"texX1";
    p1.append_attribute(L"type") = L"sampler2D";
    p1.append_attribute(L"size") = 1;
    p1.append_attribute(L"val")  = texX.id;
//
    p2.append_attribute(L"id")   = 1;
    p2.append_attribute(L"name") = L"texY1";
    p2.append_attribute(L"type") = L"sampler2D";
    p2.append_attribute(L"size") = 1;
    p2.append_attribute(L"val")  = texY.id;

    p3.append_attribute(L"id")   = 2;
    p3.append_attribute(L"name") = L"texZ1";
    p3.append_attribute(L"type") = L"sampler2D";
    p3.append_attribute(L"size") = 1;
    p3.append_attribute(L"val")  = texZ.id;

    p4.append_attribute(L"id")   = 3;
    p4.append_attribute(L"name") = L"texX2";
    p4.append_attribute(L"type") = L"sampler2D";
    p4.append_attribute(L"size") = 1;
    p4.append_attribute(L"val")  = texX2.id;
//
    p5.append_attribute(L"id")   = 4;
    p5.append_attribute(L"name") = L"texY2";
    p5.append_attribute(L"type") = L"sampler2D";
    p5.append_attribute(L"size") = 1;
    p5.append_attribute(L"val")  = texY2.id;

    p6.append_attribute(L"id")   = 5;
    p6.append_attribute(L"name") = L"texZ2";
    p6.append_attribute(L"type") = L"sampler2D";
    p6.append_attribute(L"size") = 1;
    p6.append_attribute(L"val")  = texZ2.id;

    p7.append_attribute(L"id")   = 6;
    p7.append_attribute(L"name") = L"tex_scale";
    p7.append_attribute(L"type") = L"float3";
    p7.append_attribute(L"size") = 1;
    p7.append_attribute(L"val")  = L"10.0 1.0 1.0";

  }
  hrMaterialClose(newBodyMat);

  int32_t remapList1[2] = { body_mat_id, newBodyMat.id };
  mRes.identity();

  auto bbox = HRUtils::InstanceSceneIntoScene(merged_car, scnRef, mRes.L(), false, remapList1, 2);

  hrFlush(scnRef, renderRef, camRef);

  glViewport(0, 0, w, h);
  std::vector<int32_t> image(w * h);

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      hrRenderGetFrameBufferLDR1i(renderRef, w, h, &image[0]);

      glDisable(GL_TEXTURE_2D);
      glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);

      glfwSwapBuffers(g_window);
      glfwPollEvents();
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_x4/z_out.png");

  return check_images("test_x4", 1, 30);
}