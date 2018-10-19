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
    camNode.append_child(L"position").text().set(L"3 1.5 -1");
    camNode.append_child(L"look_at").text().set(L"-0.0 0.85 0.0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  int w = 800;
  int h = 800;

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

  std::wstring car_path(L"/home/vs/Yandex.Disk/samsungdata/CarsLibSeparate/002");

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
  auto displacement = matNode.append_child(L"displacement");
  displacement.append_attribute(L"type").set_value(L"true_displacement");
  displacement.append_attribute(L"subdivs").set_value(1);

  auto customNode   = displacement.append_child(L"custom");
  hrTextureBind(displace_1, customNode);
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

    int w = 800;
    int h = 800;

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

    std::wstring car_path(L"/CarsLibSeparate/006");

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
        displacement.append_attribute(L"subdivs").set_value(2);

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