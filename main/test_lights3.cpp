#include "tests.h"
#include <iomanip>
#include <cstring>
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

namespace LGHT_TESTS
{
  const int TESTS_IMG_SIZE = 512;
  
  bool test_226_area_spot_simple()
  {
    initGLIfNeeded();

    hrErrorCallerPlace(L"test_226");

    hrSceneLibraryOpen(L"tests_f/test_226", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1     = HRMeshFromSimpleMesh(L"sph1",    CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef sph2     = HRMeshFromSimpleMesh(L"sph2",    CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(6.0f),   matGray.id);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef spot1 = hrLightCreate(L"spot1");

    hrLightOpen(spot1, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(spot1);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"spot"); 

      auto sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"half_length").set_value(0.2f);
      sizeNode.append_attribute(L"half_width").set_value(0.1f);

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
			intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"700");

      lightNode.append_child(L"falloff_angle").append_attribute(L"val").set_value(100);
      lightNode.append_child(L"falloff_angle2").append_attribute(L"val").set_value(60);

			VERIFY_XML(lightNode);
    }
    hrLightClose(spot1);


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

			VERIFY_XML(camNode);
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, TESTS_IMG_SIZE, TESTS_IMG_SIZE, 256, 2048);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scene
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace HydraLiteMath;

    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mRes;

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();


    mTranslate = translate4x4(float3(0.0f, 3.0f, 0.0f));
    mRot = rotate_Y_4x4(180.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrMeshInstance(scnRef, cubeOpen, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(3.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());


    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, 8.95f, -5.0f));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, spot1, mRes.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    glViewport(0, 0, TESTS_IMG_SIZE, TESTS_IMG_SIZE);
    std::vector<int32_t> image(TESTS_IMG_SIZE * TESTS_IMG_SIZE);

    while (true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

      if (info.haveUpdateFB)
      {
        hrRenderGetFrameBufferLDR1i(renderRef, TESTS_IMG_SIZE, TESTS_IMG_SIZE, &image[0]);

        glDisable(GL_TEXTURE_2D);
        glDrawPixels(TESTS_IMG_SIZE, TESTS_IMG_SIZE, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

        auto pres = std::cout.precision(2);
        std::cout << "rendering progress = " << info.progress << "% \r";
        std::cout.precision(pres);

        glfwSwapBuffers(g_window);
        glfwPollEvents();
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_226/z_out.png");

    return check_images("test_226", 1, 20);
  }

  bool test_227_point_spot_glossy_wall()
  {
    initGLIfNeeded();

    hrErrorCallerPlace(L"test_227");
    hrSceneLibraryOpen(L"tests_f/test_227", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    HRMaterialRef matGlossy = hrMaterialCreate(L"matGray");

    hrMaterialOpen(matGlossy, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGlossy);

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.75 0.75");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.75");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(0);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(50);

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matGlossy);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    auto cube = CreateCubeOpen(6.0f);
    HRMeshRef sph1     = HRMeshFromSimpleMesh(L"sph1",    CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef sph2     = HRMeshFromSimpleMesh(L"sph2",    CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef cubeOpen = hrMeshCreate(L"CubeOpen");

    hrMeshOpen(cubeOpen, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(cubeOpen, L"pos",      &cube.vPos[0]);
      hrMeshVertexAttribPointer4f(cubeOpen, L"norm",     &cube.vNorm[0]);
      hrMeshVertexAttribPointer2f(cubeOpen, L"texcoord", &cube.vTexCoord[0]);

      const int g = matGray.id;
      const int r = matGlossy.id;
      int mid[10] = { g,g,g,g,r,r,g,g,g,g };
      hrMeshPrimitiveAttribPointer1i(cubeOpen, L"mind", mid);

      hrMeshAppendTriangles3(cubeOpen, int(cube.triIndices.size()), &cube.triIndices[0]);
    }
    hrMeshClose(cubeOpen);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef spot1 = hrLightCreate(L"spot1");

    hrLightOpen(spot1, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(spot1);

      lightNode.attribute(L"type").set_value(L"point");
      lightNode.attribute(L"shape").set_value(L"point");
      lightNode.attribute(L"distribution").set_value(L"spot");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
			intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"50");

      lightNode.append_child(L"falloff_angle").append_attribute(L"val").set_value(100);
      lightNode.append_child(L"falloff_angle2").append_attribute(L"val").set_value(60);

			VERIFY_XML(lightNode);
    }
    hrLightClose(spot1);


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

			VERIFY_XML(camNode);
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = hrRenderCreate(L"HydraModern");
  
    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      auto node = hrRenderParamNode(renderRef);
    
      node.append_child(L"width").text()  = 512;
      node.append_child(L"height").text() = 512;
    
      node.append_child(L"method_primary").text()   = L"IBPT";
      node.append_child(L"method_secondary").text() = L"IBPT";
      node.append_child(L"shadows").text()          = L"1";
    
      node.append_child(L"trace_depth").text()      = L"6";
      node.append_child(L"diff_trace_depth").text() = L"3";
      node.append_child(L"maxRaysPerPixel").text()  = 1024;
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
    float4x4 mRes;

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();


    mTranslate = translate4x4(float3(0.0f, 3.0f, 0.0f));
    mRot = rotate_Y_4x4(180.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrMeshInstance(scnRef, cubeOpen, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(3.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());


    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, 8.95f, -5.0f));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, spot1, mRes.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    glViewport(0, 0, TESTS_IMG_SIZE, TESTS_IMG_SIZE);
    std::vector<int32_t> image(TESTS_IMG_SIZE * TESTS_IMG_SIZE);

    while (true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

      if (info.haveUpdateFB)
      {
        hrRenderGetFrameBufferLDR1i(renderRef, TESTS_IMG_SIZE, TESTS_IMG_SIZE, &image[0]);

        glDisable(GL_TEXTURE_2D);
        glDrawPixels(TESTS_IMG_SIZE, TESTS_IMG_SIZE, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

        auto pres = std::cout.precision(2);
        std::cout << "rendering progress = " << info.progress << "% \r";
        std::cout.precision(pres);
        std::cout.flush();

        glfwSwapBuffers(g_window);
        glfwPollEvents();
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_227/z_out.png");

    return check_images("test_227", 1, 60);
  }

  bool test_228_point_ies_for_bpt()
  {
    initGLIfNeeded();

    hrErrorCallerPlace(L"test_228");

    hrSceneLibraryOpen(L"tests_f/test_228", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1     = HRMeshFromSimpleMesh(L"sph1",    CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef sph2     = HRMeshFromSimpleMesh(L"sph2",    CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(6.0f),   matGray.id);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef spot1 = hrLightCreate(L"spot1");

    hrLightOpen(spot1, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(spot1);

      lightNode.attribute(L"type").set_value(L"point");
      lightNode.attribute(L"shape").set_value(L"point");
      lightNode.attribute(L"distribution").set_value(L"ies");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"10");

      auto honioNode = lightNode.append_child(L"ies");

      honioNode.append_attribute(L"data")   = L"data/ies/ies_1.ies";
      honioNode.append_attribute(L"matrix") = L"1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1";

      //VERIFY_XML(lightNode);
    }
    hrLightClose(spot1);


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

			VERIFY_XML(camNode);
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, TESTS_IMG_SIZE, TESTS_IMG_SIZE, 256, 2048);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scene
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace HydraLiteMath;

    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mRes;

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();


    mTranslate = translate4x4(float3(0.0f, 3.0f, 0.0f));
    mRot = rotate_Y_4x4(180.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrMeshInstance(scnRef, cubeOpen, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(3.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());


    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, 8.95f, -5.0f));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, spot1, mRes.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    glViewport(0, 0, TESTS_IMG_SIZE, TESTS_IMG_SIZE);
    std::vector<int32_t> image(TESTS_IMG_SIZE * TESTS_IMG_SIZE);

    while (true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

      if (info.haveUpdateFB)
      {
        hrRenderGetFrameBufferLDR1i(renderRef, TESTS_IMG_SIZE, TESTS_IMG_SIZE, &image[0]);

        glDisable(GL_TEXTURE_2D);
        glDrawPixels(TESTS_IMG_SIZE, TESTS_IMG_SIZE, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

        auto pres = std::cout.precision(2);
        std::cout << "rendering progress = " << info.progress << "% \r";
        std::cout.precision(pres);

        glfwSwapBuffers(g_window);
        glfwPollEvents();
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_228/z_out.png");

    return check_images("test_228", 1, 20);
  }

  bool test_229_point_ies_for_bpt()
  {
    initGLIfNeeded();

    hrErrorCallerPlace(L"test_229");

    hrSceneLibraryOpen(L"tests_f/test_229", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1     = HRMeshFromSimpleMesh(L"sph1",    CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef sph2     = HRMeshFromSimpleMesh(L"sph2",    CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(6.0f),   matGray.id);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef spot1 = hrLightCreate(L"spot1");

    hrLightOpen(spot1, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(spot1);

      lightNode.attribute(L"type").set_value(L"point");
      lightNode.attribute(L"shape").set_value(L"point");
      lightNode.attribute(L"distribution").set_value(L"ies");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"10");

      auto honioNode = lightNode.append_child(L"ies");

      honioNode.append_attribute(L"data")   = L"data/ies/ies_5.ies";
      honioNode.append_attribute(L"matrix") = L"1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1";

      //VERIFY_XML(lightNode);
    }
    hrLightClose(spot1);


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

			VERIFY_XML(camNode);
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, TESTS_IMG_SIZE, TESTS_IMG_SIZE, 256, 2048);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scene
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace HydraLiteMath;

    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mRes;

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();


    mTranslate = translate4x4(float3(0.0f, 3.0f, 0.0f));
    mRot = rotate_Y_4x4(180.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrMeshInstance(scnRef, cubeOpen, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(3.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());


    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, 8.0f, -5.0f));
    mRot = rotate_Z_4x4(-20.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrLightInstance(scnRef, spot1, mRes.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    glViewport(0, 0, TESTS_IMG_SIZE, TESTS_IMG_SIZE);
    std::vector<int32_t> image(TESTS_IMG_SIZE * TESTS_IMG_SIZE);

    while (true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

      if (info.haveUpdateFB)
      {
        hrRenderGetFrameBufferLDR1i(renderRef, TESTS_IMG_SIZE, TESTS_IMG_SIZE, &image[0]);

        glDisable(GL_TEXTURE_2D);
        glDrawPixels(TESTS_IMG_SIZE, TESTS_IMG_SIZE, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

        auto pres = std::cout.precision(2);
        std::cout << "rendering progress = " << info.progress << "% \r";
        std::cout.precision(pres);

        glfwSwapBuffers(g_window);
        glfwPollEvents();
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_229/z_out.png");

    return check_images("test_229", 1, 20);
  }

  bool test_230_area_ies_for_bpt()
  {
    initGLIfNeeded();

    hrErrorCallerPlace(L"test_230");

    hrSceneLibraryOpen(L"tests_f/test_230", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1     = HRMeshFromSimpleMesh(L"sph1",    CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef sph2     = HRMeshFromSimpleMesh(L"sph2",    CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(6.0f),   matGray.id);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef spot1 = hrLightCreate(L"spot1");

    hrLightOpen(spot1, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(spot1);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"disk");
      lightNode.attribute(L"distribution").set_value(L"ies");

      lightNode.append_child(L"size").append_attribute(L"radius") = 0.25f;

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"100");

      auto honioNode = lightNode.append_child(L"ies");

      honioNode.append_attribute(L"data")   = L"data/ies/ies_5.ies";
      honioNode.append_attribute(L"matrix") = L"1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1";

      VERIFY_XML(lightNode);
    }
    hrLightClose(spot1);


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

			VERIFY_XML(camNode);
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, TESTS_IMG_SIZE, TESTS_IMG_SIZE, 512, 4096);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scene
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace HydraLiteMath;

    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mRes;

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();


    mTranslate = translate4x4(float3(0.0f, 3.0f, 0.0f));
    mRot = rotate_Y_4x4(180.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrMeshInstance(scnRef, cubeOpen, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(3.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());


    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, 8.0f, -5.0f));
    mRot = rotate_Z_4x4(-20.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrLightInstance(scnRef, spot1, mRes.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    glViewport(0, 0, TESTS_IMG_SIZE, TESTS_IMG_SIZE);
    std::vector<int32_t> image(TESTS_IMG_SIZE * TESTS_IMG_SIZE);

    while (true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

      if (info.haveUpdateFB)
      {
        hrRenderGetFrameBufferLDR1i(renderRef, TESTS_IMG_SIZE, TESTS_IMG_SIZE, &image[0]);

        glDisable(GL_TEXTURE_2D);
        glDrawPixels(TESTS_IMG_SIZE, TESTS_IMG_SIZE, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

        auto pres = std::cout.precision(2);
        std::cout << "rendering progress = " << info.progress << "% \r";
        std::cout.precision(pres);

        glfwSwapBuffers(g_window);
        glfwPollEvents();
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_230/z_out.png");

    return check_images("test_230", 1, 20);
  }

  bool test_231_direct_soft_shadow()
  {
    initGLIfNeeded();

    hrErrorCallerPlace(L"test_231");

    hrSceneLibraryOpen(L"tests_f/test_231", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");
    HRMaterialRef matRefl = hrMaterialCreate(L"matRefl");


    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

    }
    hrMaterialClose(matGray);

    hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matRefl);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);

    }
    hrMaterialClose(matRefl);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef cubeR    = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matGray.id);
    HRMeshRef pillar   = HRMeshFromSimpleMesh(L"pillar", CreateCube(1.0f), matGray.id);
    HRMeshRef sphereG  = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matRefl.id);
    HRMeshRef torusB   = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matRefl.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef sky = hrLightCreate(L"sky");
    HRLightRef sun = hrLightCreate(L"sun");

    hrLightOpen(sky, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sky);

      lightNode.attribute(L"type").set_value(L"sky");
			lightNode.attribute(L"distribution").set_value(L"perez");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");

      auto sunModel = lightNode.append_child(L"perez");

      sunModel.append_attribute(L"sun_id")    = sun.id;
      sunModel.append_attribute(L"turbidity") = 2.0f;

			VERIFY_XML(lightNode);
    }
    hrLightClose(sky);

    hrLightOpen(sun, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sun);

      lightNode.attribute(L"type").set_value(L"directional");
      lightNode.attribute(L"shape").set_value(L"point");
      lightNode.attribute(L"distribution").set_value(L"directional");

      auto sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"inner_radius").set_value(L"0.0");
      sizeNode.append_attribute(L"outer_radius").set_value(L"50.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1.0 0.85 0.64");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"5.0");

			lightNode.append_child(L"shadow_softness").append_attribute(L"val").set_value(1.0f);
      lightNode.append_child(L"angle_radius").append_attribute(L"val").set_value(0.25f);

			VERIFY_XML(lightNode);
    }
    hrLightClose(sun);



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
      camNode.append_child(L"position").text().set(L"0 7 25");
      camNode.append_child(L"look_at").text().set(L"0 3 0");
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, TESTS_IMG_SIZE, TESTS_IMG_SIZE, 256, 2048);
    hrRenderOpen(renderRef, HR_OPEN_EXISTING);
    {
      auto node = hrRenderParamNode(renderRef);
      node.force_child(L"method_caustic").text() = L"none";
    }
    hrRenderClose(renderRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scene
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace HydraLiteMath;

    float4x4 mRot, mRot2;
    float4x4 mTranslate;
    float4x4 mScale;
    float4x4 mRes;

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();


    mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, planeRef, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    mTranslate = translate4x4(float3(-4.75f, 1.0f, 5.0f));
    mRot = rotate_Y_4x4(60.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrMeshInstance(scnRef, cubeR, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    mTranslate = translate4x4(float3(15.0f, 7.0f, -15.0f));
    mScale = scale4x4(float3(2.0f, 8.0f, 2.0f));
    mRes = mul(mTranslate, mScale);

    hrMeshInstance(scnRef, pillar, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    mTranslate = translate4x4(float3(-15.0f, 7.0f, -15.0f));
    mScale = scale4x4(float3(2.0f, 8.0f, 2.0f));
    mRes = mul(mTranslate, mScale);

    hrMeshInstance(scnRef, pillar, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mRot2.identity();

    mTranslate = translate4x4(float3(4.0f, 1.0f, 5.5f));
    mRot = rotate_Y_4x4(-60.0f*DEG_TO_RAD);
    mRot2 = rotate_X_4x4(90.0f*DEG_TO_RAD);
    mRes = mul(mRot, mRot2);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, torusB, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    mTranslate = translate4x4(float3(0.0f, 3.0f, -1.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sphereG, mRes.L());

    ///////////
    //mRes.identity();
    //hrLightInstance(scnRef, sky, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mRot2.identity();

    mTranslate = translate4x4(float3(10.0f, 50.0f, -10.0f));
    mRot  = rotate_X_4x4(-45.0f*DEG_TO_RAD);
    mRot2 = rotate_Z_4x4(-30.f*DEG_TO_RAD);
    mRes  = mul(mRot2, mRot);
    mRes  = mul(mTranslate, mRes);

    hrLightInstance(scnRef, sun, mRes.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    glViewport(0, 0, TESTS_IMG_SIZE, TESTS_IMG_SIZE);
    std::vector<int32_t> image(TESTS_IMG_SIZE * TESTS_IMG_SIZE);

    while (true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

      if (info.haveUpdateFB)
      {
        hrRenderGetFrameBufferLDR1i(renderRef, TESTS_IMG_SIZE, TESTS_IMG_SIZE, &image[0]);

        glDisable(GL_TEXTURE_2D);
        glDrawPixels(TESTS_IMG_SIZE, TESTS_IMG_SIZE, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

        auto pres = std::cout.precision(2);
        std::cout << "rendering progress = " << info.progress << "% \r";
        std::cout.precision(pres);

        glfwSwapBuffers(g_window);
        glfwPollEvents();
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_231/z_out.png");

    return check_images("test_231", 1, 20);
  }

  bool test_232_point_area_ies()
  {
    initGLIfNeeded();

    hrErrorCallerPlace(L"test_232");

    hrSceneLibraryOpen(L"tests_f/test_232", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1     = HRMeshFromSimpleMesh(L"sph1",    CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef sph2     = HRMeshFromSimpleMesh(L"sph2",    CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(6.0f),   matGray.id);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef spot1 = hrLightCreate(L"spot1");

    hrLightOpen(spot1, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(spot1);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"ies");

      lightNode.append_child(L"size");
      lightNode.child(L"size").append_attribute(L"half_width")  = 0.25f;
      lightNode.child(L"size").append_attribute(L"half_length") = 0.25f;

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"100");

      auto honioNode = lightNode.append_child(L"ies");

      honioNode.append_attribute(L"data")   = L"data/ies/ies_5.ies";
      //honioNode.append_attribute(L"data")   = L"data/ies/ies_1.ies";
      honioNode.append_attribute(L"matrix") = L"1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1";

      VERIFY_XML(lightNode);
    }
    hrLightClose(spot1);

    HRLightRef spot2 = hrLightCreate(L"spot2");

    hrLightOpen(spot2, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(spot2);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"ies");

      lightNode.append_child(L"size");
      lightNode.child(L"size").append_attribute(L"half_width")  = 0.25f;
      lightNode.child(L"size").append_attribute(L"half_length") = 0.25f;

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"100");

      auto honioNode = lightNode.append_child(L"ies");

      honioNode.append_attribute(L"data")       = L"data/ies/ies_5.ies";
      //honioNode.append_attribute(L"data")     = L"data/ies/ies_1.ies";
      honioNode.append_attribute(L"matrix")     = L"1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1";
      honioNode.append_attribute(L"point_area") = 1; /// indicates that we must always eval ies honiogram from the center of light, not from the point we actually sample/hit.

      VERIFY_XML(lightNode);
    }
    hrLightClose(spot2);



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

			VERIFY_XML(camNode);
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, TESTS_IMG_SIZE, TESTS_IMG_SIZE, 512, 4096);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scene
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace HydraLiteMath;

    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mRes;

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();


    mTranslate = translate4x4(float3(0.0f, 3.0f, 0.0f));
    mRot = rotate_Y_4x4(180.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrMeshInstance(scnRef, cubeOpen, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(3.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());


    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 8.0f, -5.0f));
    hrLightInstance(scnRef, spot1, mTranslate.L());

    mTranslate = translate4x4(float3(+3.0f, 8.0f, -5.0f));
    hrLightInstance(scnRef, spot2, mTranslate.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    glViewport(0, 0, TESTS_IMG_SIZE, TESTS_IMG_SIZE);
    std::vector<int32_t> image(TESTS_IMG_SIZE * TESTS_IMG_SIZE);

    while (true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

      if (info.haveUpdateFB)
      {
        hrRenderGetFrameBufferLDR1i(renderRef, TESTS_IMG_SIZE, TESTS_IMG_SIZE, &image[0]);

        glDisable(GL_TEXTURE_2D);
        glDrawPixels(TESTS_IMG_SIZE, TESTS_IMG_SIZE, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

        auto pres = std::cout.precision(2);
        std::cout << "rendering progress = " << info.progress << "% \r";
        std::cout.precision(pres);

        glfwSwapBuffers(g_window);
        glfwPollEvents();
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_232/z_out.png");

    return check_images("test_232", 1, 20.0f);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  using HydraLiteMath::float4x4;
  using HydraLiteMath::float3;
  using HydraLiteMath::translate4x4;
  using HydraLiteMath::rotate_X_4x4;

  void MyCAD_SetupLED(HRLightRef a_light, const std::wstring a_iesPath, const std::wstring a_colorStr)
  {
    hrLightOpen(a_light, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(a_light);
    
      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"ies");
    
      lightNode.append_child(L"size");
      lightNode.child(L"size").append_attribute(L"half_width")  = 0.1f;
      lightNode.child(L"size").append_attribute(L"half_length") = 0.1f;
    
      auto intensityNode = lightNode.append_child(L"intensity");
    
      intensityNode.append_child(L"color").append_attribute(L"val")      = a_colorStr.c_str();
      intensityNode.append_child(L"multiplier").append_attribute(L"val") = 100;
    
      auto honioNode = lightNode.append_child(L"ies");
      honioNode.append_attribute(L"data")       = a_iesPath.c_str();
      honioNode.append_attribute(L"matrix")     = L"1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1";
      honioNode.append_attribute(L"point_area") = 1; /// indicates that we must always eval ies honiogram from the center of light, not from the point we actually sample/hit.
    
      VERIFY_XML(lightNode);
    }
    hrLightClose(a_light);

  }

  HRLightGroupExt MyCAD_CreateAreaMatrixLEDLight3x3(const std::wstring& a_lightName)
  {
    const std::wstring redLedName   = a_lightName + L"_red_led";
    const std::wstring greenLedName = a_lightName + L"_green_led";
    const std::wstring blueLedName  = a_lightName + L"_blue_led";

    HRLightRef redLed   = hrLightCreate(redLedName.c_str());
    HRLightRef greenLed = hrLightCreate(greenLedName.c_str());
    HRLightRef blueLed  = hrLightCreate(blueLedName.c_str());

    MyCAD_SetupLED(redLed  , L"data/ies/ies_1.ies", L"1 0 0");
    MyCAD_SetupLED(greenLed, L"data/ies/ies_1.ies", L"0 1 0");
    MyCAD_SetupLED(blueLed , L"data/ies/ies_1.ies", L"0 0 1");

    HRLightRef r = redLed, g = greenLed, b = blueLed; // just wanna te have single letter reference

    HRLightRef lights[3 * 3] = {r,g,b,
                                g,b,r,
                                b,r,g};               // put them is some way to array :)

    // now form translate matrices for lights
    //
    const float tsize = 0.33f;

    float4x4 translate[3*3];
    translate[0] = translate4x4(float3(-tsize, 0, -tsize));
    translate[1] = translate4x4(float3(0,      0, -tsize));
    translate[2] = translate4x4(float3(+tsize, 0, -tsize));

    translate[3] = translate4x4(float3(-tsize, 0, 0));
    translate[4] = translate4x4(float3(0,      0, 0));
    translate[5] = translate4x4(float3(+tsize, 0, 0));

    translate[6] = translate4x4(float3(-tsize, 0, +tsize));
    translate[7] = translate4x4(float3(0,      0, +tsize));
    translate[8] = translate4x4(float3(+tsize, 0, +tsize));

    HRLightGroupExt res(3*3);
    for (int i = 0; i < res.lightsNum; i++)
    {
      res.lights[i] = lights[i];
      memcpy(res.matrix[i], translate[i].L(), 16 * sizeof(float));
    }

    return res;
  }


  bool test_233_light_group_point_area_ies()
  {
    initGLIfNeeded();

    hrErrorCallerPlace(L"test_233");

    hrSceneLibraryOpen(L"tests_f/test_233", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1     = HRMeshFromSimpleMesh(L"sph1",    CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef sph2     = HRMeshFromSimpleMesh(L"sph2",    CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(6.0f),   matGray.id);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef spot1 = hrLightCreate(L"spot1");

    hrLightOpen(spot1, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(spot1);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"disk");
      lightNode.attribute(L"distribution").set_value(L"ies");

      lightNode.append_child(L"size").append_attribute(L"radius") = 0.1f;

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1000");

      auto honioNode = lightNode.append_child(L"ies");

      honioNode.append_attribute(L"data")   = L"data/ies/ies_1.ies";
      honioNode.append_attribute(L"matrix") = L"1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1";

      VERIFY_XML(lightNode);
    }
    hrLightClose(spot1);

    HRLightGroupExt lightGroup = MyCAD_CreateAreaMatrixLEDLight3x3(L"IntiLED_3x3");


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

			VERIFY_XML(camNode);
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, TESTS_IMG_SIZE, TESTS_IMG_SIZE, 512, 4096);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scene
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace HydraLiteMath;

    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mRes;

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();


    mTranslate = translate4x4(float3(0.0f, 3.0f, 0.0f));
    mRot = rotate_Y_4x4(180.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrMeshInstance(scnRef, cubeOpen, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(3.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());


    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 8.0f, -5.0f));
    hrLightInstance(scnRef, spot1, mTranslate.L());

    mTranslate = translate4x4(float3(+3.0f, 8.0f, -5.5f));
    hrLightGroupInstanceExt(scnRef, lightGroup, mTranslate.L());

    mRot       = rotate_X_4x4(+30.0f*DEG_TO_RAD);
    mTranslate = translate4x4(float3(-5.5f, 8.0f, -2.0f));
    mRes       = mul(mTranslate, mRot);
    hrLightGroupInstanceExt(scnRef, lightGroup, mRes.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    glViewport(0, 0, TESTS_IMG_SIZE, TESTS_IMG_SIZE);
    std::vector<int32_t> image(TESTS_IMG_SIZE * TESTS_IMG_SIZE);

    while (true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

      if (info.haveUpdateFB)
      {
        hrRenderGetFrameBufferLDR1i(renderRef, TESTS_IMG_SIZE, TESTS_IMG_SIZE, &image[0]);

        glDisable(GL_TEXTURE_2D);
        glDrawPixels(TESTS_IMG_SIZE, TESTS_IMG_SIZE, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

        auto pres = std::cout.precision(2);
        std::cout << "rendering progress = " << info.progress << "% \r";
        std::cout.precision(pres);

        glfwSwapBuffers(g_window);
        glfwPollEvents();
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_233/z_out.png");

    return check_images("test_233", 1, 30.0f);
  }

  bool test_234_light_group_light_inst_cust_params()
  {
    initGLIfNeeded();

    hrErrorCallerPlace(L"test_234");

    hrSceneLibraryOpen(L"tests_f/test_234", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1     = HRMeshFromSimpleMesh(L"sph1",    CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef sph2     = HRMeshFromSimpleMesh(L"sph2",    CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(6.0f),   matGray.id);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef spot1 = hrLightCreate(L"spot1");

    hrLightOpen(spot1, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(spot1);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"disk");
      lightNode.attribute(L"distribution").set_value(L"ies");

      lightNode.append_child(L"size").append_attribute(L"radius") = 0.1f;

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1000");

      auto honioNode = lightNode.append_child(L"ies");

      honioNode.append_attribute(L"data")   = L"data/ies/ies_1.ies";
      honioNode.append_attribute(L"matrix") = L"1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1";

      VERIFY_XML(lightNode);
    }
    hrLightClose(spot1);

    HRLightGroupExt lightGroup = MyCAD_CreateAreaMatrixLEDLight3x3(L"IntiLED_3x3");


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

			VERIFY_XML(camNode);
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, TESTS_IMG_SIZE, TESTS_IMG_SIZE, 512, 4096);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scene
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace HydraLiteMath;

    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mRes;

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();


    mTranslate = translate4x4(float3(0.0f, 3.0f, 0.0f));
    mRot = rotate_Y_4x4(180.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrMeshInstance(scnRef, cubeOpen, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(3.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());


    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 8.0f, -5.0f));
    hrLightInstance(scnRef, spot1, mTranslate.L(), L"color_mult = \"0.5 1.0 0.5\"");


    std::vector<std::wstring>   arrayForGroup(lightGroup.lightsNum); // well, light group instance has many light instances, so you have to pass many attributes.
    std::vector<const wchar_t*> ptrsForGroup(lightGroup.lightsNum);  // arrayForGroup store std::wstring, but we need pointers to pass them to hrLightGroupInstanceExt.
    for (size_t i = 0; i < arrayForGroup.size(); i++)                // Sorry for this shit please. We don't want to pass STL types to HydraAPI to have less dependencies.
    {
      arrayForGroup[i] = L"color_mult = \"2.0 1.0 1.0\"";
      ptrsForGroup [i] = &arrayForGroup[i][0];
    }

    mTranslate = translate4x4(float3(+3.0f, 8.0f, -5.5f));
    hrLightGroupInstanceExt(scnRef, lightGroup, mTranslate.L(), &ptrsForGroup[0]);

    mRot       = rotate_X_4x4(+30.0f*DEG_TO_RAD);
    mTranslate = translate4x4(float3(-5.5f, 8.0f, -2.0f));
    mRes       = mul(mTranslate, mRot);
    hrLightGroupInstanceExt(scnRef, lightGroup, mRes.L(), &ptrsForGroup[0]);

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    glViewport(0, 0, TESTS_IMG_SIZE, TESTS_IMG_SIZE);
    std::vector<int32_t> image(TESTS_IMG_SIZE * TESTS_IMG_SIZE);

    while (true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

      if (info.haveUpdateFB)
      {
        hrRenderGetFrameBufferLDR1i(renderRef, TESTS_IMG_SIZE, TESTS_IMG_SIZE, &image[0]);

        glDisable(GL_TEXTURE_2D);
        glDrawPixels(TESTS_IMG_SIZE, TESTS_IMG_SIZE, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

        auto pres = std::cout.precision(2);
        std::cout << "rendering progress = " << info.progress << "% \r";
        std::cout.precision(pres);

        glfwSwapBuffers(g_window);
        glfwPollEvents();
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_234/z_out.png");

    return check_images("test_234", 1, 40);
  }

  bool test_235_stadium()
  {
    initGLIfNeeded();

    hrErrorCallerPlace(L"test_235");

    hrSceneLibraryOpen(L"tests_f/test_235", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    HRMaterialRef matGray = hrMaterialCreate(L"matGray");
    HRMaterialRef matRefl = hrMaterialCreate(L"matRefl");


    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
    }
    hrMaterialClose(matGray);

    hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matRefl);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.0 0.6 0.0");
    }
    hrMaterialClose(matRefl);

    // add some more materials because stadium have 11 different material ids
    //
    HRMaterialRef otherStadiumMats[10];
    for (int i = 0; i < 10; i++) 
    {
      otherStadiumMats[i] = hrMaterialCreate(L"matAux");
    
      hrMaterialOpen(otherStadiumMats[i], HR_WRITE_DISCARD);
      {
        auto matNode = hrMaterialParamNode(otherStadiumMats[i]);
    
        auto diff = matNode.append_child(L"diffuse");
        diff.append_attribute(L"brdf_type").set_value(L"lambert");
    
        auto color = diff.append_child(L"color");
        color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
      }
      hrMaterialClose(otherStadiumMats[i]);
    
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMeshRef stadiumRef = hrMeshCreateFromFileDL(L"data/meshes/stadium.vsgf"); // HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef sky = hrLightCreate(L"sky");

    hrLightOpen(sky, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sky);

      lightNode.attribute(L"type").set_value(L"sky");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.75 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);

      VERIFY_XML(lightNode);
    }
    hrLightClose(sky);

    HRLightRef spot1 = hrLightCreate(L"spot1");

    hrLightOpen(spot1, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(spot1);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"disk");
      lightNode.attribute(L"distribution").set_value(L"ies");

      lightNode.append_child(L"size").append_attribute(L"radius") = 0.25f;

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"200");

      auto honioNode = lightNode.append_child(L"ies");

      honioNode.append_attribute(L"data")       = L"data/ies/ies_1.ies";
      honioNode.append_attribute(L"matrix")     = L"1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1";
      honioNode.append_attribute(L"point_area") = 1;

      VERIFY_XML(lightNode);
    }
    hrLightClose(spot1);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Camera
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRCameraRef camRef = hrCameraCreate(L"my camera");

    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      auto camNode = hrCameraParamNode(camRef);

      camNode.append_child(L"fov").text().set(L"90");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      //camNode.append_child(L"up").text().set(L"0 1 0");
      //camNode.append_child(L"position").text().set(L"-25 -35 100");
      //camNode.append_child(L"look_at").text().set(L"-150 -40 0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"-80 -35 100");
      camNode.append_child(L"look_at").text().set(L"-120 -30 0");
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    HRRenderRef renderRef = hrRenderCreate(L"HydraModern");
    hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
  
    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      auto node = hrRenderParamNode(renderRef);
    
      node.append_child(L"width").text()  = 1024;
      node.append_child(L"height").text() = 768;
    
      node.append_child(L"method_primary").text()   = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text()  = L"pathtracing";
      node.append_child(L"method_caustic").text()   = L"pathtracing";
      node.append_child(L"shadows").text()          = L"1";
    
      node.append_child(L"trace_depth").text()      = L"6";
      node.append_child(L"diff_trace_depth").text() = L"3";
      node.append_child(L"maxRaysPerPixel").text()  = 8192;
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

    mTranslate.identity();
    mRes.identity();
    mRot.identity();


    mTranslate = translate4x4(float3(0.0f, 0.0f, 0.0f));
    mScale     = scale4x4(float3(1, 1, 1));
    mRot       = rotate_Y_4x4(0.0f*DEG_TO_RAD);
    mRes       = mul(mTranslate, mul(mScale, mRot));

    hrMeshInstance(scnRef, stadiumRef, mRot.L());

    ///////////

    mRes.identity();
    //hrLightInstance(scnRef, sky, mRes.L());

    for (float offZ = -120.0f; offZ < 120.0f; offZ += 5.0f)
    {
      mTranslate = translate4x4(float3(-125.0f + 2.5f, 0.0f, 0.0f + offZ));
      hrLightInstance(scnRef, spot1, mTranslate.L(), L"color_mult = \"1.0 1.0 1.0\"");

      mTranslate = translate4x4(float3(-115.0f + 2.5f, 0.0f, 0.0f + offZ));
      hrLightInstance(scnRef, spot1, mTranslate.L(), L"color_mult = \"0.0 0.0 1.0\"");

      mTranslate = translate4x4(float3(-105.0f + 2.5f, 0.0f, 0.0f + offZ));
      hrLightInstance(scnRef, spot1, mTranslate.L(), L"color_mult = \"1.0 0.0 0.0\"");

      mTranslate = translate4x4(float3(-95.0f + 2.5f, 0.0f,  0.0f + offZ));
      hrLightInstance(scnRef, spot1, mTranslate.L(), L"color_mult = \"1.0 1.0 1.0\"");
    }

    for (float offZ = -120.0f; offZ < 120.0f; offZ += 5.0f)
    {
      mTranslate = translate4x4(float3(+125.0f - 2.5f, 0.0f, 0.0f + offZ));
      hrLightInstance(scnRef, spot1, mTranslate.L());

      mTranslate = translate4x4(float3(+115.0f - 2.5f, 0.0f, 0.0f + offZ));
      hrLightInstance(scnRef, spot1, mTranslate.L());

      mTranslate = translate4x4(float3(+105.0f - 2.5f, 0.0f, 0.0f + offZ));
      hrLightInstance(scnRef, spot1, mTranslate.L());

      mTranslate = translate4x4(float3(+95.0f - 2.5f, 0.0f, 0.0f + offZ));
      hrLightInstance(scnRef, spot1, mTranslate.L());
    }
   
    for (float offX = -80.0f; offX < 80.0f; offX += 5.0f)
    {
      mTranslate = translate4x4(float3(offX, 0.0f, +160.0f));
      hrLightInstance(scnRef, spot1, mTranslate.L());

      mTranslate = translate4x4(float3(offX, 0.0f, +150.0f));
      hrLightInstance(scnRef, spot1, mTranslate.L());

      mTranslate = translate4x4(float3(offX, 0.0f, +140.0f));
      hrLightInstance(scnRef, spot1, mTranslate.L());

      mTranslate = translate4x4(float3(offX, 0.0f, +130.0f));
      hrLightInstance(scnRef, spot1, mTranslate.L());
    }

    for (float offX = -80.0f; offX < 80.0f; offX += 5.0f)
    {
      mTranslate = translate4x4(float3(offX, 0.0f, -160.0f));
      hrLightInstance(scnRef, spot1, mTranslate.L());

      mTranslate = translate4x4(float3(offX, 0.0f, -150.0f));
      hrLightInstance(scnRef, spot1, mTranslate.L());

      mTranslate = translate4x4(float3(offX, 0.0f, -140.0f));
      hrLightInstance(scnRef, spot1, mTranslate.L());

      mTranslate = translate4x4(float3(offX, 0.0f, -130.0f));
      hrLightInstance(scnRef, spot1, mTranslate.L());
    }

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    while (true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

      if (info.haveUpdateFB)
      {
        auto pres = std::cout.precision(2);
        std::cout << "rendering progress = " << info.progress << "% \r";
        std::cout.precision(pres);

        glfwSwapBuffers(g_window);
        glfwPollEvents();
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_235/z_out.png");

    return check_images("test_235", 1, 200);
  }



  HRLightGroupExt MyCAD_CreateAreaMatrixLEDLight(const std::wstring& a_lightName, const int a_sizeX, const int a_sizeY, const float a_size = 1.0f)
  {
    const std::wstring redLedName   = a_lightName + L"_red_led";
    const std::wstring greenLedName = a_lightName + L"_green_led";
    const std::wstring blueLedName  = a_lightName + L"_blue_led";

    HRLightRef redLed   = hrLightCreate(redLedName.c_str());
    HRLightRef greenLed = hrLightCreate(greenLedName.c_str());
    HRLightRef blueLed  = hrLightCreate(blueLedName.c_str());

    MyCAD_SetupLED(redLed  , L"data/ies/ies_1.ies", L"1 0 0");
    MyCAD_SetupLED(greenLed, L"data/ies/ies_1.ies", L"0 1 0");
    MyCAD_SetupLED(blueLed , L"data/ies/ies_1.ies", L"0 0 1");

    HRLightRef r = redLed, g = greenLed, b = blueLed; // just wanna te have single letter reference

    const int totalSizeX = (a_sizeX/3) * 3;

    // create lighs ref array and translate matrices 
    //
    std::vector<HRLightRef> lights(totalSizeX * a_sizeY);
    std::vector<float4x4>   translate(totalSizeX * a_sizeY);

    HRLightRef rgb[3] = { r,g,b };

    const float tsize = a_size/float(a_sizeY);

    int currLEDIndex = 0;
    for (int y = 0; y < a_sizeY; y++)
    {
      for (int x = 0; x < totalSizeX; x++)
      {
        int ledIdex = (x + currLEDIndex) % 3;
        lights   [y*a_sizeY + x] = rgb[ledIdex];
        translate[y*a_sizeY + x] = translate4x4(float3(tsize*float(x-totalSizeX/2), 0, tsize*float(y- a_sizeY/2)));
      }

      currLEDIndex++;
      if (currLEDIndex > 2)
        currLEDIndex = 0;
    }

    HRLightGroupExt res(int32_t(lights.size()));
    for (int i = 0; i < res.lightsNum; i++)
    {
      res.lights[i] = lights[i];
      memcpy(res.matrix[i], translate[i].L(), 16 * sizeof(float));
    }

    return res;
  }



  
  bool test_236_light_group_point_area_ies2()
  {
    initGLIfNeeded();

    hrErrorCallerPlace(L"test_236");

    hrSceneLibraryOpen(L"tests_f/test_236", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1     = HRMeshFromSimpleMesh(L"sph1",    CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef sph2     = HRMeshFromSimpleMesh(L"sph2",    CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(6.0f),   matGray.id);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef spot1 = hrLightCreate(L"spot1");

    hrLightOpen(spot1, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(spot1);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"disk");
      lightNode.attribute(L"distribution").set_value(L"ies");

      lightNode.append_child(L"size").append_attribute(L"radius") = 0.1f;

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1000");

      auto honioNode = lightNode.append_child(L"ies");

      honioNode.append_attribute(L"data")   = L"data/ies/ies_1.ies";
      honioNode.append_attribute(L"matrix") = L"1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1";

      VERIFY_XML(lightNode);
    }
    hrLightClose(spot1);

    HRLightGroupExt lightGroup = MyCAD_CreateAreaMatrixLEDLight(L"IntiLED_NxN", 6, 6, 1.5f);


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

			VERIFY_XML(camNode);
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, TESTS_IMG_SIZE, TESTS_IMG_SIZE, 512, 4096);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scene
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace HydraLiteMath;

    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mRes;

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();


    mTranslate = translate4x4(float3(0.0f, 3.0f, 0.0f));
    mRot = rotate_Y_4x4(180.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrMeshInstance(scnRef, cubeOpen, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(3.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());


    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 8.0f, -5.0f));
    hrLightInstance(scnRef, spot1, mTranslate.L());

    mTranslate = translate4x4(float3(+3.0f, 8.0f, -5.0f));
    hrLightGroupInstanceExt(scnRef, lightGroup, mTranslate.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    glViewport(0, 0, TESTS_IMG_SIZE, TESTS_IMG_SIZE);
    std::vector<int32_t> image(TESTS_IMG_SIZE * TESTS_IMG_SIZE);

    while (true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

      if (info.haveUpdateFB)
      {
        hrRenderGetFrameBufferLDR1i(renderRef, TESTS_IMG_SIZE, TESTS_IMG_SIZE, &image[0]);

        glDisable(GL_TEXTURE_2D);
        glDrawPixels(TESTS_IMG_SIZE, TESTS_IMG_SIZE, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

        auto pres = std::cout.precision(2);
        std::cout << "rendering progress = " << info.progress << "% \r";
        std::cout.precision(pres);

        glfwSwapBuffers(g_window);
        glfwPollEvents();
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_236/z_out.png");

    return check_images("test_236", 1, 25);
  }

};