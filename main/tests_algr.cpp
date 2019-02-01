//
// Created by frol on 2/1/19.
//

#include "tests.h"
#include <iomanip>

#include <stdlib.h>
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

#include "../hydra_api/HR_HDRImageTool.h"
#include "../hydra_api/HydraXMLHelpers.h"

#pragma warning(disable:4996)
#pragma warning(disable:4838)
#pragma warning(disable:4244)

extern GLFWwindow* g_window;

using namespace TEST_UTILS;


bool ALGR_TESTS::test_401_ibpt_and_glossy_glass()
{
  initGLIfNeeded();
  
  hrErrorCallerPlace(L"test_401");
  
  hrSceneLibraryOpen(L"tests_f/test_401", HR_WRITE_DISCARD);
  
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  
  HRMaterialRef matGray  = hrMaterialCreate(L"matGray");
  HRMaterialRef matGlass = hrMaterialCreate(L"matGlass");
  
  
  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);
    
    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    
    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(matGray);
  
  hrMaterialOpen(matGlass, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGlass);
    
    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"phong");
    
    auto color = refl.append_child(L"color");
    color.append_attribute(L"val").set_value(L"1 1 1");
    
    refl.append_child(L"fresnel").append_attribute(L"val")     = 1;
    refl.append_child(L"fresnel_ior").append_attribute(L"val") = 1.5;
    refl.append_child(L"glossiness").append_attribute(L"val")  = 1;
    
    auto trans = matNode.append_child(L"transparency");
    
    trans.append_child(L"color").append_attribute(L"val")      = L"1 1 1";
    trans.append_child(L"fog_color").append_attribute(L"val")  = L"1 1 1";
    
    trans.append_child(L"fog_multiplier").append_attribute(L"val") = 0.0f;
    trans.append_child(L"glossiness").append_attribute(L"val")     = 0.90f;
    trans.append_child(L"ior").append_attribute(L"val")            = 1.5f;
    
    VERIFY_XML(matNode);
  }
  hrMaterialClose(matGlass);
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef sph1     = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64),  matGlass.id);
  HRMeshRef sph2     = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64),  matGlass.id);
  HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(6.0f), matGray.id);
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Light
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  HRLightRef sphere1 = hrLightCreate(L"sphere1");
  
  hrLightOpen(sphere1, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sphere1);
    
    lightNode.attribute(L"type").set_value(L"area");
    lightNode.attribute(L"shape").set_value(L"sphere");
    lightNode.attribute(L"distribution").set_value(L"uniform");
    
    auto intensityNode = lightNode.append_child(L"intensity");
    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 0.5 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"200.0");
    
    auto sizeNode = lightNode.append_child(L"size").append_attribute(L"radius").set_value(0.1f);
    VERIFY_XML(lightNode);
  }
  hrLightClose(sphere1);
  
  HRLightRef sphere2 = hrLightCreate(L"sphere2");
  
  hrLightOpen(sphere2, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sphere2);
    
    lightNode.attribute(L"type").set_value(L"area");
    lightNode.attribute(L"shape").set_value(L"sphere");
    lightNode.attribute(L"distribution").set_value(L"uniform");
    
    auto intensityNode = lightNode.append_child(L"intensity");
    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.5 1 0.5");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"200.0");
    
    auto sizeNode = lightNode.append_child(L"size").append_attribute(L"radius").set_value(0.1f);
    VERIFY_XML(lightNode);
  }
  hrLightClose(sphere2);
  
  
  
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
    camNode.append_child(L"position").text().set(L"0 3 18");
    camNode.append_child(L"look_at").text().set(L"0 3 0");
  }
  hrCameraClose(camRef);
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  //HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 4096);
  
  HRRenderRef renderRef = hrRenderCreate(L"HydraModern");
  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
  
  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    auto node = hrRenderParamNode(renderRef);
    
    node.append_child(L"width").text()  = 512;
    node.append_child(L"height").text() = 512;
  
    node.append_child(L"method_primary").text()   = L"IBPT";
    node.append_child(L"method_secondary").text() = L"IBPT";
    node.append_child(L"method_tertiary").text()  = L"IBPT";
    node.append_child(L"method_caustic").text()   = L"IBPT";
    node.append_child(L"shadows").text()          = L"1";
    
    node.append_child(L"trace_depth").text()      = L"10";
    node.append_child(L"diff_trace_depth").text() = L"5";
    node.append_child(L"maxRaysPerPixel").text()  = 4096;
    node.append_child(L"resources_path").text()   = L"..";
    node.append_child(L"offline_pt").text()       = 0;
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
  
  const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;
  
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  ///////////
  
  mTranslate = translate4x4(float3(0.0f, 3.0f, 0.0f));
  mRot       = rotate_Y_4x4(180.0f*DEG_TO_RAD);
  mRes       = mul(mTranslate, mRot);
  
  hrMeshInstance(scnRef, cubeOpen, mRes.L());
  
  ///////////
  
  mTranslate.identity();
  mRes.identity();
  
  mTranslate = translate4x4(float3(-3.0f, 4.25f, 0.0f));
  mRes       = mul(mTranslate, mRes);
  
  hrMeshInstance(scnRef, sph1, mRes.L());
  
  //mScale = scale4x4(float3(0.5f, 0.5f, 0.5f));
  //mRes   = mul(mTranslate, mScale);
  //hrMeshInstance(scnRef, sph1, mRes.L());
  
  ///////////
  
  mTranslate.identity();
  mRes.identity();
  
  mTranslate = translate4x4(float3(3.0f, 4.25f, 0.0f));
  mRes       = mul(mTranslate, mRes);
  
  hrMeshInstance(scnRef, sph2, mRes.L());
  
  
  ///////////
  
  mTranslate.identity();
  mRes.identity();
  
  mTranslate = translate4x4(float3(5.0f, 8.0f, 0.0f));
  mRes       = mul(mTranslate, mRes);
  
  hrLightInstance(scnRef, sphere1, mRes.L());
  
  ///////////
  
  mTranslate.identity();
  mRes.identity();
  
  mTranslate = translate4x4(float3(-5.0f, 8.0f, 0.0f));
  mRes       = mul(mTranslate, mRes);
  
  hrLightInstance(scnRef, sphere2, mRes.L());
  
  
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
      std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
      std::cout.precision(pres);
      
      glfwSwapBuffers(g_window);
      glfwPollEvents();
    }
    
    if (info.finalUpdate)
      break;
  }
  
  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_401/z_out.png");
  
  return check_images("test_401", 1, 40);
  
}

bool ALGR_TESTS::test_402_ibpt_and_glossy_double_glass()
{
  initGLIfNeeded();
  
  hrErrorCallerPlace(L"test_402");
  
  hrSceneLibraryOpen(L"tests_f/test_402", HR_WRITE_DISCARD);
  
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  
  HRMaterialRef matGray  = hrMaterialCreate(L"matGray");
  HRMaterialRef matGlass = hrMaterialCreate(L"matGlass");
  
  
  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);
    
    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    
    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(matGray);
  
  hrMaterialOpen(matGlass, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGlass);
    
    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"phong");
    
    auto color = refl.append_child(L"color");
    color.append_attribute(L"val").set_value(L"1 1 1");
    
    refl.append_child(L"fresnel").append_attribute(L"val")     = 1;
    refl.append_child(L"fresnel_ior").append_attribute(L"val") = 1.5;
    refl.append_child(L"glossiness").append_attribute(L"val")  = 1;
    
    auto trans = matNode.append_child(L"transparency");
    
    trans.append_child(L"color").append_attribute(L"val")      = L"1 1 1";
    trans.append_child(L"fog_color").append_attribute(L"val")  = L"1 1 1";
    
    trans.append_child(L"fog_multiplier").append_attribute(L"val") = 0.0f;
    trans.append_child(L"glossiness").append_attribute(L"val")     = 0.90f;
    trans.append_child(L"ior").append_attribute(L"val")            = 1.5f;
    
    VERIFY_XML(matNode);
  }
  hrMaterialClose(matGlass);
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef sph1     = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64),  matGlass.id);
  HRMeshRef sph2     = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64),  matGlass.id);
  HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(6.0f), matGray.id);
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Light
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  HRLightRef sphere1 = hrLightCreate(L"sphere1");
  
  hrLightOpen(sphere1, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sphere1);
    
    lightNode.attribute(L"type").set_value(L"area");
    lightNode.attribute(L"shape").set_value(L"sphere");
    lightNode.attribute(L"distribution").set_value(L"uniform");
    
    auto intensityNode = lightNode.append_child(L"intensity");
    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 0.5 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"200.0");
    
    auto sizeNode = lightNode.append_child(L"size").append_attribute(L"radius").set_value(0.1f);
    VERIFY_XML(lightNode);
  }
  hrLightClose(sphere1);
  
  HRLightRef sphere2 = hrLightCreate(L"sphere2");
  
  hrLightOpen(sphere2, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sphere2);
    
    lightNode.attribute(L"type").set_value(L"area");
    lightNode.attribute(L"shape").set_value(L"sphere");
    lightNode.attribute(L"distribution").set_value(L"uniform");
    
    auto intensityNode = lightNode.append_child(L"intensity");
    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.5 1 0.5");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"200.0");
    
    auto sizeNode = lightNode.append_child(L"size").append_attribute(L"radius").set_value(0.1f);
    VERIFY_XML(lightNode);
  }
  hrLightClose(sphere2);
  
  
  
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
    camNode.append_child(L"position").text().set(L"0 3 18");
    camNode.append_child(L"look_at").text().set(L"0 3 0");
  }
  hrCameraClose(camRef);
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  //HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 4096);
  
  HRRenderRef renderRef = hrRenderCreate(L"HydraModern");
  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
  
  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    auto node = hrRenderParamNode(renderRef);
    
    node.append_child(L"width").text()  = 512;
    node.append_child(L"height").text() = 512;
    
    node.append_child(L"method_primary").text()   = L"IBPT";
    node.append_child(L"method_secondary").text() = L"IBPT";
    node.append_child(L"method_tertiary").text()  = L"IBPT";
    node.append_child(L"method_caustic").text()   = L"IBPT";
    node.append_child(L"shadows").text()          = L"1";
    
    node.append_child(L"trace_depth").text()      = L"10";
    node.append_child(L"diff_trace_depth").text() = L"10";
    node.append_child(L"maxRaysPerPixel").text()  = 4096;
    node.append_child(L"resources_path").text()   = L"..";
    node.append_child(L"offline_pt").text()       = 0;
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
  
  const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;
  
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  ///////////
  
  mTranslate = translate4x4(float3(0.0f, 3.0f, 0.0f));
  mRot       = rotate_Y_4x4(180.0f*DEG_TO_RAD);
  mRes       = mul(mTranslate, mRot);
  
  hrMeshInstance(scnRef, cubeOpen, mRes.L());
  
  ///////////
  
  mTranslate.identity();
  mRes.identity();
  
  mTranslate = translate4x4(float3(-3.0f, 4.25f, 0.0f));
  mRes       = mul(mTranslate, mRes);
  
  hrMeshInstance(scnRef, sph1, mRes.L());
  
  mScale = scale4x4(float3(0.65f, 0.65f, 0.65f));
  mRes   = mul(mTranslate, mScale);
  hrMeshInstance(scnRef, sph1, mRes.L());
  
  ///////////
  
  mTranslate.identity();
  mRes.identity();
  
  mTranslate = translate4x4(float3(3.0f, 4.25f, 0.0f));
  mRes       = mul(mTranslate, mRes);
  
  hrMeshInstance(scnRef, sph2, mRes.L());
  
  
  ///////////
  
  mTranslate.identity();
  mRes.identity();
  
  mTranslate = translate4x4(float3(5.0f, 8.0f, 0.0f));
  mRes       = mul(mTranslate, mRes);
  
  hrLightInstance(scnRef, sphere1, mRes.L());
  
  ///////////
  
  mTranslate.identity();
  mRes.identity();
  
  mTranslate = translate4x4(float3(-5.0f, 8.0f, 0.0f));
  mRes       = mul(mTranslate, mRes);
  
  hrLightInstance(scnRef, sphere2, mRes.L());
  
  
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
      std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
      std::cout.precision(pres);
      
      glfwSwapBuffers(g_window);
      glfwPollEvents();
    }
    
    if (info.finalUpdate)
      break;
  }
  
  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_402/z_out.png");
  
  // return check_images("test_402", 1, 40);
  
  
  return false;
}

