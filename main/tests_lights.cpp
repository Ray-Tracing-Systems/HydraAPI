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


namespace LGHT_TESTS
{
  bool test_200_spot()
  {
    

    hrErrorCallerPlace(L"test_200");

    hrSceneLibraryOpen(L"tests_f/test_200", HR_WRITE_DISCARD);

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
      lightNode.attribute(L"distribution").set_value(L"spot");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 0.5 1");
			intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"8.0");

      lightNode.append_child(L"falloff_angle").append_attribute(L"val").set_value(60);
      lightNode.append_child(L"falloff_angle2").append_attribute(L"val").set_value(45);

			VERIFY_XML(lightNode);
    }
    hrLightClose(spot1);

    HRLightRef spot2 = hrLightCreate(L"spot2");

    hrLightOpen(spot2, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(spot2);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"disk");
      lightNode.attribute(L"distribution").set_value(L"spot");

      lightNode.append_child(L"size").append_attribute(L"radius").set_value(1.0f);

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.5 1 0.5");
			intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"8.0");

      lightNode.append_child(L"falloff_angle").append_attribute(L"val").set_value(90);
      lightNode.append_child(L"falloff_angle2").append_attribute(L"val").set_value(60);
			
			VERIFY_XML(lightNode);
    }
    hrLightClose(spot2);

    HRLightRef spot3 = hrLightCreate(L"spot3");

    hrLightOpen(spot3, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(spot3);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"spot");

      auto sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"half_length").set_value(1.0f);
      sizeNode.append_attribute(L"half_width").set_value(1.0f);

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 1");
			intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"8.0");

      lightNode.append_child(L"falloff_angle").append_attribute(L"val").set_value(100);
      lightNode.append_child(L"falloff_angle2").append_attribute(L"val").set_value(75);

			VERIFY_XML(lightNode);
    }
    hrLightClose(spot3);


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

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 2048);


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

    mTranslate = translate4x4(float3(3.0f, 8.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, spot1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 8.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, spot2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, -2.5f, 0.0f));
    mRot = rotate_Z_4x4(180.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrLightInstance(scnRef, spot3, mRes.L());

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
        std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
        std::cout.precision(pres);

        
        
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_200/z_out.png");

    return check_images("test_200", 1, 15);
  }

  bool test_201_sphere()
  {
    

    hrErrorCallerPlace(L"test_201");

    hrSceneLibraryOpen(L"tests_f/test_201", HR_WRITE_DISCARD);

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
    }
    hrMaterialClose(matGray);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matGray.id);
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
			intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"8.0");

      auto sizeNode = lightNode.append_child(L"size").append_attribute(L"radius").set_value(0.5f);
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
			intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"8.0");

      auto sizeNode = lightNode.append_child(L"size").append_attribute(L"radius").set_value(0.5f);
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

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 2048);


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

    mTranslate = translate4x4(float3(5.0f, 8.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, sphere1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-5.0f, 8.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, sphere2, mRes.L());


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
        std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
        std::cout.precision(pres);

        
        
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_201/z_out.png");

    return check_images("test_201", 1, 40);
  }

  bool test_202_sky_color()
  {
    

    hrErrorCallerPlace(L"test_202");

    hrSceneLibraryOpen(L"tests_f/test_202", HR_WRITE_DISCARD);

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

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.9 0.9 0.9");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);
    }
    hrMaterialClose(matRefl);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matGray.id);
    HRMeshRef sphereG = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matRefl.id);
    HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matGray.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);
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

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 2048);


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

    mTranslate = translate4x4(float3(0.0f, 2.0f, -1.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sphereG, mRes.L());

    ///////////

    mRes.identity();

    hrLightInstance(scnRef, sky, mRes.L());

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
        std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
        std::cout.precision(pres);

        
        
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_202/z_out.png");

    return check_images("test_202", 1, 10);
  }

  bool test_203_sky_hdr()
  {
    hrErrorCallerPlace(L"test_203");

    hrSceneLibraryOpen(L"tests_f/test_203", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    HRMaterialRef matGray = hrMaterialCreate(L"matGray");
    HRMaterialRef matRefl = hrMaterialCreate(L"matRefl");


    HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/23_antwerp_night.hdr");
    

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

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.9 0.9 0.9");
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
    HRMeshRef sphereG  = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matRefl.id);
    HRMeshRef torusB   = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matGray.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);
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
      camNode.append_child(L"position").text().set(L"0 13 16");
      camNode.append_child(L"look_at").text().set(L"0 0 0");
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPTNoCaust(CURR_RENDER_DEVICE, 512, 512, 512, 4096);


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

    mTranslate = translate4x4(float3(0.0f, 2.0f, -1.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sphereG, mRes.L());

    ///////////

    mRes.identity();

    hrLightInstance(scnRef, sky, mRes.L());

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
        std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
        std::cout.precision(pres);

        
        
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_203/z_out.png");

    return check_images("test_203", 1, 20);
  }

  bool test_204_sky_hdr_rotate()
  {
    hrErrorCallerPlace(L"test_204");

    hrSceneLibraryOpen(L"tests_f/test_204", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    HRMaterialRef matGray = hrMaterialCreate(L"matGray");
    HRMaterialRef matRefl = hrMaterialCreate(L"matRefl");


    HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/23_antwerp_night.hdr");


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

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.9 0.9 0.9");
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
    HRMeshRef sphereG  = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matRefl.id);
    HRMeshRef torusB   = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matGray.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);
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

      // well, due to enviromnet texture coordinates are related to polar coordinates, 
      // environment rotate can be implemented via shift matrix. So, this is not classic rotation matrix.
      //
      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 1, 0, 0, -0.4f,
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
      camNode.append_child(L"position").text().set(L"0 13 16");
      camNode.append_child(L"look_at").text().set(L"0 0 0");
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPTNoCaust(CURR_RENDER_DEVICE, 512, 512, 512, 4096);


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

    mTranslate = translate4x4(float3(0.0f, 2.0f, -1.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sphereG, mRes.L());

    ///////////

    mRes.identity();

    hrLightInstance(scnRef, sky, mRes.L());

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
        std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
        std::cout.precision(pres);
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_204/z_out.png");

    return check_images("test_204", 1, 25);
  }

  bool test_205_sky_and_directional_sun()
  {
    hrErrorCallerPlace(L"test_205");

    hrSceneLibraryOpen(L"tests_f/test_205", HR_WRITE_DISCARD);

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

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.98 0.98 0.98");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);
    }
    hrMaterialClose(matRefl);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matGray.id);
    HRMeshRef pillar = HRMeshFromSimpleMesh(L"pillar", CreateCube(1.0f), matGray.id);
    HRMeshRef sphereG = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matRefl.id);
    HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matRefl.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);
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
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");
			
			VERIFY_XML(lightNode);
    }
    hrLightClose(sky);


    HRLightRef sun = hrLightCreate(L"sun");

    hrLightOpen(sun, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sun);

      lightNode.attribute(L"type").set_value(L"directional");
      lightNode.attribute(L"shape").set_value(L"point");
      lightNode.attribute(L"distribution").set_value(L"directional");

      auto sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"inner_radius").set_value(L"0.0");
      sizeNode.append_attribute(L"outer_radius").set_value(L"1000.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1.0 0.85 0.64");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"2.0");

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

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 2048);
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

    mRes.identity();

    hrLightInstance(scnRef, sky, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mRot2.identity();

    mTranslate = translate4x4(float3(100.0f, 100.0f, -50.0f));
    mRot = rotate_X_4x4(-45.0f*DEG_TO_RAD);
    mRot2 = rotate_Z_4x4(-30.f*DEG_TO_RAD);
    mRes = mul(mRot2, mRot);
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, sun, mRes.L());

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
        std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
        std::cout.precision(pres);

        
        
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_205/z_out.png");

    return check_images("test_205", 1, 20);
  }

  bool test_206_ies1()
  {
    hrErrorCallerPlace(L"test_206");

    hrSceneLibraryOpen(L"tests_f/test_206", HR_WRITE_DISCARD);

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
    }
    hrMaterialClose(matGray);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(6.0f), matGray.id);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef light1 = hrLightCreate(L"light1");

    hrLightOpen(light1, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(light1);

      lightNode.attribute(L"type").set_value(L"area");
	    lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"ies");

      auto distribution = lightNode.append_child(L"ies");
      distribution.append_attribute(L"data").set_value(L"data/ies/ies_1.ies");

      distribution.append_attribute(L"matrix");
      float samplerMatrix[16] = { 1, 0, 0, 0,
                                  0, 1, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      HydraXMLHelpers::WriteMatrix4x4(distribution, L"matrix", samplerMatrix);

	    auto sizeNode = lightNode.append_child(L"size");
		  
	    sizeNode.append_attribute(L"half_length").set_value(L"0.125");
	    sizeNode.append_attribute(L"half_width").set_value(L"0.125");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.5 1.0 0.5");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1000.0");

			VERIFY_XML(lightNode);
    }
    hrLightClose(light1);

    HRLightRef light2 = hrLightCreate(L"light2");

    hrLightOpen(light2, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(light2);

      lightNode.attribute(L"type").set_value(L"area");
	    lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"ies");

      auto distribution = lightNode.append_child(L"ies");
      distribution.append_attribute(L"data").set_value(L"data/ies/ies_1.ies");

      distribution.append_attribute(L"matrix");
      float samplerMatrix[16] = { 1.2f, 0, 0, 0,
                                  0, 1.2f, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      HydraXMLHelpers::WriteMatrix4x4(distribution, L"matrix", samplerMatrix);

	    auto sizeNode = lightNode.append_child(L"size");
		  
	    sizeNode.append_attribute(L"half_length").set_value(L"0.125");
	    sizeNode.append_attribute(L"half_width").set_value(L"0.125");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1.0 0.5 1.0");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1000.0");

			VERIFY_XML(lightNode);
    }
    hrLightClose(light2);



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

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 2048);


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

    mTranslate = translate4x4(float3(3.0f, 8.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, light1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 8.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, light2, mRes.L());


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
        std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
        std::cout.precision(pres);

        
        
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_206/z_out.png");

    return check_images("test_206", 1, 40);
  }

  bool test_207_ies2()
  {
    hrErrorCallerPlace(L"test_207");

    hrSceneLibraryOpen(L"tests_f/test_207", HR_WRITE_DISCARD);

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
    }
    hrMaterialClose(matGray);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(6.0f), matGray.id);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef light1 = hrLightCreate(L"light1");

    hrLightOpen(light1, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(light1);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"disk");
      lightNode.attribute(L"distribution").set_value(L"ies");

      auto distribution = lightNode.append_child(L"ies");
      distribution.append_attribute(L"data").set_value(L"data/ies/ies_4.ies");

      distribution.append_attribute(L"matrix");
      float samplerMatrix[16] = { 1, 0, 0, 0,
                                  0, 1, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      HydraXMLHelpers::WriteMatrix4x4(distribution, L"matrix", samplerMatrix);

      auto sizeNode = lightNode.append_child(L"size").append_attribute(L"radius").set_value(L"0.75");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.5 1.0 0.5");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"150.0");

			VERIFY_XML(lightNode);
    }
    hrLightClose(light1);

    HRLightRef light2 = hrLightCreate(L"light2");

    hrLightOpen(light2, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(light2);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"disk");
      lightNode.attribute(L"distribution").set_value(L"ies");

      auto distribution = lightNode.append_child(L"ies");
      distribution.append_attribute(L"data").set_value(L"data/ies/ies_10.ies");

      distribution.append_attribute(L"matrix");
      float samplerMatrix[16] = { 1, 0, 0, 0,
                                  0, 1, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      HydraXMLHelpers::WriteMatrix4x4(distribution, L"matrix", samplerMatrix);

      auto sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"radius").set_value(L"0.75");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1.0 0.5 1.0");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"5.0");

			VERIFY_XML(lightNode);
    }
    hrLightClose(light2);



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

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 2048);


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

    mTranslate = translate4x4(float3(0.0f, 8.5f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, light1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, -2.5f, 0.0f));
    mRot = rotate_Z_4x4(180.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrLightInstance(scnRef, light2, mRes.L());


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
        std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
        std::cout.precision(pres);
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_207/z_out.png");

    return check_images("test_207", 1, 40);
  }

  bool test_208_ies3()
  {
    hrErrorCallerPlace(L"test_208");

    hrSceneLibraryOpen(L"tests_f/test_208", HR_WRITE_DISCARD);

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
    }
    hrMaterialClose(matGray);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(6.0f), matGray.id);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef light1 = hrLightCreate(L"light1");

    hrLightOpen(light1, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(light1);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"disk");
      lightNode.attribute(L"distribution").set_value(L"ies");

      auto distribution = lightNode.append_child(L"ies");
      distribution.append_attribute(L"data").set_value(L"data/ies/ies_5.ies");

      distribution.append_attribute(L"matrix");
      float samplerMatrix[16] = { 1, 0, 0, 0,
                                  0, 1, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      HydraXMLHelpers::WriteMatrix4x4(distribution, L"matrix", samplerMatrix);

      auto sizeNode = lightNode.append_child(L"size").append_attribute(L"radius").set_value(L"0.5");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.5 1.0 0.5");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"100.0");

			VERIFY_XML(lightNode);
    }
    hrLightClose(light1);

    HRLightRef light2 = hrLightCreate(L"light2");

    hrLightOpen(light2, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(light2);

      lightNode.attribute(L"type").set_value(L"point");
      lightNode.attribute(L"shape").set_value(L"point");
      lightNode.attribute(L"distribution").set_value(L"ies");

      auto distribution = lightNode.append_child(L"ies");
      distribution.append_attribute(L"data").set_value(L"data/ies/ies_16.ies");

      distribution.append_attribute(L"matrix");
      float samplerMatrix[16] = { 1, 0, 0, 0,
                                  0, 1, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      HydraXMLHelpers::WriteMatrix4x4(distribution, L"matrix", samplerMatrix);

      auto sizeNode = lightNode.append_child(L"size");
      sizeNode.append_child(L"radius").text().set(L"0.15");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1.0 0.5 1.0");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"20.0");

			VERIFY_XML(lightNode);
    }
    hrLightClose(light2);



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

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 2048);


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

    mTranslate = translate4x4(float3(3.0f, 5.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, light1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-4.5f, 6.5f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, light2, mRes.L());


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
        std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
        std::cout.precision(pres);
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_208/z_out.png");

    return check_images("test_208", 1, 40);
  }

  bool test_209_skyportal()
  {
    hrErrorCallerPlace(L"test_209");

    hrSceneLibraryOpen(L"tests_f/test_209", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    HRMaterialRef matGray = hrMaterialCreate(L"matGray");


    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

    }
    hrMaterialClose(matGray);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(24.0f), matGray.id);
    HRMeshRef wall = HRMeshFromSimpleMesh(L"wall", CreatePlane(12.0f), matGray.id);
    HRMeshRef cube = HRMeshFromSimpleMesh(L"cube", CreateCube(2.5f), matGray.id);

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
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"5.0");

			VERIFY_XML(lightNode);
    }
    hrLightClose(sky);


    HRLightRef portal1 = hrLightCreate(L"portal1");

    hrLightOpen(portal1, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(portal1);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"uniform");
			lightNode.attribute(L"visible").set_value(L"0");

      auto sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"half_length").set_value(4.0f);
      sizeNode.append_attribute(L"half_width").set_value(6.0f);

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 0 0");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"2.0");

      auto portalNode = lightNode.append_child(L"sky_portal");
      portalNode.append_attribute(L"val")       = 1;
      portalNode.append_attribute(L"source_id") = sky.id;

			VERIFY_XML(lightNode);
    }
    hrLightClose(portal1);

    HRLightRef portal2 = hrLightCreate(L"portal2");

    hrLightOpen(portal2, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(portal2);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"uniform");
			lightNode.attribute(L"visible").set_value(L"0");

      auto sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"half_length").set_value(4.0f);
      sizeNode.append_attribute(L"half_width").set_value(6.0f);

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0 1 0");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"2.0");

      auto portalNode = lightNode.append_child(L"sky_portal");
      portalNode.append_attribute(L"val")       = 1;
      portalNode.append_attribute(L"source_id") = sky.id;

			VERIFY_XML(lightNode);
    }
    hrLightClose(portal2);


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
      camNode.append_child(L"position").text().set(L"0 1 23");
      camNode.append_child(L"look_at").text().set(L"0 1 0");
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 4096);


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


    mTranslate = translate4x4(float3(0.0f, 6.0f, 0.0f));
    //mRot = rotate_Y_4x4(180.0f*DEG_TO_RAD);
    mScale = scale4x4(float3(1.0f, 0.5f, 1.0f));
    mRes = mul(mTranslate, mScale);

    hrMeshInstance(scnRef, cubeOpen, mRes.L());
    
    ///////////WALL1

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(18.0f, 6.0f, -24.0f));
    mRot = rotate_X_4x4(90.f*DEG_TO_RAD);
    mScale = scale4x4(float3(0.5f, 1.0f, 1.0f));
    mRes = mul(mRot, mScale);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, wall, mRes.L());
    
    ///////////WALL2

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(-18.0f, 6.0f, -24.0f));
    mRot = rotate_X_4x4(90.f*DEG_TO_RAD);
    mScale = scale4x4(float3(0.5f, 1.0f, 1.0f));
    mRes = mul(mRot, mScale);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, wall, mRes.L());

    ///////////WALL3

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(0.0f, 6.0f, -24.0f));
    mRot = rotate_X_4x4(90.f*DEG_TO_RAD);
    mScale = scale4x4(float3(0.5f, 1.0f, 1.0f));
    mRes = mul(mRot, mScale);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, wall, mRes.L());


    ///////////WALL

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(8.0f, 0.0f, -24.0f));
    mRot = rotate_X_4x4(90.f*DEG_TO_RAD);
    mScale = scale4x4(float3(0.35f, 1.0f, 0.5f));
    mRes = mul(mRot, mScale);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, wall, mRes.L());

    ///////////WALL

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(-8.0f, 0.0f, -24.0f));
    mRot = rotate_X_4x4(90.f*DEG_TO_RAD);
    mScale = scale4x4(float3(0.35f, 1.0f, 0.5f));
    mRes = mul(mRot, mScale);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, wall, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(-8.0f, -3.5f, 0.0f));
    mScale = scale4x4(float3(1.25f, 1.25f, 1.25f));
    mRes = mul(mTranslate, mScale);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(-4.0f, -3.0f, -3.0f));
    mScale = scale4x4(float3(1.5f, 1.5f, 1.5f));
    mRes = mul(mTranslate, mScale);

    hrMeshInstance(scnRef, sph2, mRes.L());


    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(7.0f, -1.75f, 1.0f));
    mRot = rotate_Y_4x4(60.f*DEG_TO_RAD);
    mScale = scale4x4(float3(1.0f, 1.75f, 1.0f));
    mRes = mul(mRot, mScale);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, cube, mRes.L());


    ///////////

    mRes.identity();

    hrLightInstance(scnRef, sky, mRes.L());

    ///////////

    
    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(-8.0f, 12.0f, -24.05f));
    mRot = rotate_X_4x4(-90.f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrLightInstance(scnRef, portal1, mRes.L());

    ///////////


    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(8.0f, 12.0f, -24.05f));
    mRot = rotate_X_4x4(-90.f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrLightInstance(scnRef, portal2, mRes.L());
    

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
        std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
        std::cout.precision(pres);
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_209/z_out.png");

    return check_images("test_209", 1, 50);
  }

  bool test_210_skyportal_hdr()
  {
    hrErrorCallerPlace(L"test_210");

    hrSceneLibraryOpen(L"tests_f/test_210", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    HRMaterialRef matGray = hrMaterialCreate(L"matGray");


    HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/Factory_Catwalk_2k_BLUR.exr");

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

    }
    hrMaterialClose(matGray);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(24.0f), matGray.id);
    HRMeshRef wall = HRMeshFromSimpleMesh(L"wall", CreatePlane(12.0f), matGray.id);
    HRMeshRef cube = HRMeshFromSimpleMesh(L"cube", CreateCube(2.5f), matGray.id);

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
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"20.0");

      hrTextureBind(texEnv, intensityNode.child(L"color"));

      auto texNode = intensityNode.child(L"color").child(L"texture");

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 1, 0, 0, -1,
                                  0, 1, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(lightNode);
    }
    hrLightClose(sky);


    HRLightRef portal1 = hrLightCreate(L"portal1");

    hrLightOpen(portal1, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(portal1);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"uniform");
			lightNode.attribute(L"visible").set_value(L"0");

      auto sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"half_length").set_value(4.0f);
      sizeNode.append_attribute(L"half_width").set_value(6.0f);

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"10.0");

      auto portalNode = lightNode.append_child(L"sky_portal");
      portalNode.append_attribute(L"val").set_value(1);
      portalNode.append_attribute(L"source_id").set_value(sky.id);

			VERIFY_XML(lightNode);
    }
    hrLightClose(portal1);



    HRLightRef portal2 = hrLightCreate(L"portal2");

    hrLightOpen(portal2, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(portal2);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"uniform");
			lightNode.attribute(L"visible").set_value(L"0");

      auto sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"half_length").set_value(4.0f);
      sizeNode.append_attribute(L"half_width").set_value(6.0f);

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.25 0.25 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"10.0");

      auto portalNode = lightNode.append_child(L"sky_portal");
      portalNode.append_attribute(L"val").set_value(1);
      portalNode.append_attribute(L"source_id").set_value(sky.id);

			VERIFY_XML(lightNode);
    }
    hrLightClose(portal2);


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
      camNode.append_child(L"position").text().set(L"0 1 23");
      camNode.append_child(L"look_at").text().set(L"0 1 0");
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 2048);


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


    mTranslate = translate4x4(float3(0.0f, 6.0f, 0.0f));
    //mRot = rotate_Y_4x4(180.0f*DEG_TO_RAD);
    mScale = scale4x4(float3(1.0f, 0.5f, 1.0f));
    mRes = mul(mTranslate, mScale);

    hrMeshInstance(scnRef, cubeOpen, mRes.L());

    ///////////WALL1

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(18.0f, 6.0f, -24.0f));
    mRot = rotate_X_4x4(90.f*DEG_TO_RAD);
    mScale = scale4x4(float3(0.5f, 1.0f, 1.0f));
    mRes = mul(mRot, mScale);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, wall, mRes.L());

    ///////////WALL2

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(-18.0f, 6.0f, -24.0f));
    mRot = rotate_X_4x4(90.f*DEG_TO_RAD);
    mScale = scale4x4(float3(0.5f, 1.0f, 1.0f));
    mRes = mul(mRot, mScale);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, wall, mRes.L());

    ///////////WALL3

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(0.0f, 6.0f, -24.0f));
    mRot = rotate_X_4x4(90.f*DEG_TO_RAD);
    mScale = scale4x4(float3(0.5f, 1.0f, 1.0f));
    mRes = mul(mRot, mScale);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, wall, mRes.L());


    ///////////WALL

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(8.0f, 0.0f, -24.0f));
    mRot = rotate_X_4x4(90.f*DEG_TO_RAD);
    mScale = scale4x4(float3(0.35f, 1.0f, 0.5f));
    mRes = mul(mRot, mScale);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, wall, mRes.L());

    ///////////WALL

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(-8.0f, 0.0f, -24.0f));
    mRot = rotate_X_4x4(90.f*DEG_TO_RAD);
    mScale = scale4x4(float3(0.35f, 1.0f, 0.5f));
    mRes = mul(mRot, mScale);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, wall, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(-8.0f, -3.5f, 0.0f));
    mScale = scale4x4(float3(1.25f, 1.25f, 1.25f));
    mRes = mul(mTranslate, mScale);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(-4.0f, -3.0f, -3.0f));
    mScale = scale4x4(float3(1.5f, 1.5f, 1.5f));
    mRes = mul(mTranslate, mScale);

    hrMeshInstance(scnRef, sph2, mRes.L());


    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(7.0f, -1.75f, 1.0f));
    mRot = rotate_Y_4x4(60.f*DEG_TO_RAD);
    mScale = scale4x4(float3(1.0f, 1.75f, 1.0f));
    mRes = mul(mRot, mScale);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, cube, mRes.L());


    ///////////

    mRes.identity();

    hrLightInstance(scnRef, sky, mRes.L());

    ///////////


    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(-8.0f, 12.0f, -24.05f));
    mRot = rotate_X_4x4(-90.f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrLightInstance(scnRef, portal1, mRes.L());

    ///////////


    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(8.0f, 12.0f, -24.05f));
    mRot = rotate_X_4x4(-90.f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrLightInstance(scnRef, portal2, mRes.L());


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
        std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
        std::cout.precision(pres);
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_210/z_out.png");

    return check_images("test_210", 1, 270); // #TODO: double ckeck this later; sky portals = shit;
  }

  bool test_211_sky_and_sun_perez()
  {
    hrErrorCallerPlace(L"test_211");

    hrSceneLibraryOpen(L"tests_f/test_211", HR_WRITE_DISCARD);

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
      sizeNode.append_attribute(L"outer_radius").set_value(L"1000.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1.0 0.85 0.64");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"2.0");

			//lightNode.append_child(L"shadow_softness").append_attribute(L"val").set_value(1.0f);
      lightNode.append_child(L"angle_radius").append_attribute(L"val").set_value(0.5f);

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

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 2048);
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

    mRes.identity();

    hrLightInstance(scnRef, sky, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mRot2.identity();

    mRot  = rotate_X_4x4(-45.0f*DEG_TO_RAD);
    mRot2 = rotate_Z_4x4(-30.f*DEG_TO_RAD);
    mRes  = mul(mRot2, mRot);

    const float3 sunPos = mul(mRes, float3(0.0f, 100.0f, 0.0f));

    mTranslate = translate4x4(sunPos);
    mRes       = mul(mTranslate, mRes);

    hrLightInstance(scnRef, sun, mRes.L());

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
        std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
        std::cout.precision(pres);
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_211/z_out.png");

    return check_images("test_211", 1, 20);
  }

  bool test_212_skyportal_sun()
  {
    hrErrorCallerPlace(L"test_212");

    hrSceneLibraryOpen(L"tests_f/test_212", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    HRMaterialRef matGray = hrMaterialCreate(L"matGray");


    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

    }
    hrMaterialClose(matGray);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(24.0f), matGray.id);
    HRMeshRef wall = HRMeshFromSimpleMesh(L"wall", CreatePlane(12.0f), matGray.id);
    HRMeshRef cube = HRMeshFromSimpleMesh(L"cube", CreateCube(2.5f), matGray.id);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRLightRef sky = hrLightCreate(L"sky");
    HRLightRef sun = hrLightCreate(L"sun");

    hrLightOpen(sky, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sky);

      lightNode.attribute(L"type").set_value(L"sky");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.7 0.7 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"2.0");

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
      sizeNode.append_attribute(L"outer_radius").set_value(L"1000.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1.0 0.85 0.64");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"5.0");

			VERIFY_XML(lightNode);
    }
    hrLightClose(sun);

    HRLightRef portal1 = hrLightCreate(L"portal1");

    hrLightOpen(portal1, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(portal1);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"uniform");
			lightNode.attribute(L"visible").set_value(0);

      auto sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"half_length").set_value(4.0f);
      sizeNode.append_attribute(L"half_width").set_value(6.0f);

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"2.0");

      auto portalNode = lightNode.append_child(L"sky_portal");
      portalNode.append_attribute(L"val").set_value(1);
      portalNode.append_attribute(L"source").set_value(L"skylight");

			VERIFY_XML(lightNode);
    }
    hrLightClose(portal1);



    HRLightRef portal2 = hrLightCreate(L"portal2");

    hrLightOpen(portal2, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(portal2);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"uniform");
			lightNode.attribute(L"visible").set_value(0);               // NOTE: this is important for sky portals !!!

      auto sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"half_length").set_value(4.0f);
      sizeNode.append_attribute(L"half_width").set_value(6.0f);

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"2.0");

      auto portalNode = lightNode.append_child(L"sky_portal");
      portalNode.append_attribute(L"val").set_value(1);
      portalNode.append_attribute(L"source").set_value(L"skylight");

			VERIFY_XML(lightNode);
    }
    hrLightClose(portal2);


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
      camNode.append_child(L"position").text().set(L"0 1 23");
      camNode.append_child(L"look_at").text().set(L"0 1 0");
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 4096);


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


    mTranslate = translate4x4(float3(0.0f, 6.0f, 0.0f));
    //mRot = rotate_Y_4x4(180.0f*DEG_TO_RAD);
    mScale = scale4x4(float3(1.0f, 0.5f, 1.0f));
    mRes = mul(mTranslate, mScale);

    hrMeshInstance(scnRef, cubeOpen, mRes.L());

    ///////////WALL1

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(18.0f, 6.0f, -24.0f));
    mRot = rotate_X_4x4(90.f*DEG_TO_RAD);
    mScale = scale4x4(float3(0.5f, 1.0f, 1.0f));
    mRes = mul(mRot, mScale);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, wall, mRes.L());

    ///////////WALL2

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(-18.0f, 6.0f, -24.0f));
    mRot = rotate_X_4x4(90.f*DEG_TO_RAD);
    mScale = scale4x4(float3(0.5f, 1.0f, 1.0f));
    mRes = mul(mRot, mScale);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, wall, mRes.L());

    ///////////WALL3

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(0.0f, 6.0f, -24.0f));
    mRot = rotate_X_4x4(90.f*DEG_TO_RAD);
    mScale = scale4x4(float3(0.5f, 1.0f, 1.0f));
    mRes = mul(mRot, mScale);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, wall, mRes.L());


    ///////////WALL

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(8.0f, 0.0f, -24.0f));
    mRot = rotate_X_4x4(90.f*DEG_TO_RAD);
    mScale = scale4x4(float3(0.35f, 1.0f, 0.5f));
    mRes = mul(mRot, mScale);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, wall, mRes.L());

    ///////////WALL

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(-8.0f, 0.0f, -24.0f));
    mRot = rotate_X_4x4(90.f*DEG_TO_RAD);
    mScale = scale4x4(float3(0.35f, 1.0f, 0.5f));
    mRes = mul(mRot, mScale);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, wall, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(-8.0f, -3.5f, 0.0f));
    mScale = scale4x4(float3(1.25f, 1.25f, 1.25f));
    mRes = mul(mTranslate, mScale);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(-4.0f, -3.0f, -3.0f));
    mScale = scale4x4(float3(1.5f, 1.5f, 1.5f));
    mRes = mul(mTranslate, mScale);

    hrMeshInstance(scnRef, sph2, mRes.L());


    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(7.0f, -1.75f, 1.0f));
    mRot = rotate_Y_4x4(60.f*DEG_TO_RAD);
    mScale = scale4x4(float3(1.0f, 1.75f, 1.0f));
    mRes = mul(mRot, mScale);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, cube, mRes.L());


    ///////////

    mRes.identity();

    hrLightInstance(scnRef, sky, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mRot2.identity();

    mTranslate = translate4x4(float3(50.0f, 50.0f, -250.0f));
    mRot = rotate_X_4x4(-45.0f*DEG_TO_RAD);
    mRot2 = rotate_Z_4x4(-30.f*DEG_TO_RAD);
    mRes = mul(mRot2, mRot);
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, sun, mRes.L());

    ///////////


 
    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(-8.0f, 12.0f, -24.05f));
    mRot = rotate_X_4x4(-90.f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrLightInstance(scnRef, portal1, mRes.L());

    ///////////


    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mScale.identity();

    mTranslate = translate4x4(float3(8.0f, 12.0f, -24.05f));
    mRot = rotate_X_4x4(-90.f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrLightInstance(scnRef, portal2, mRes.L());
    
    
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
        std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
        std::cout.precision(pres);
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_212/z_out.png");

    return check_images("test_212", 1, 50);
  }

  bool test_213_point_omni()
  {
    hrErrorCallerPlace(L"test_213");

    hrSceneLibraryOpen(L"tests_f/test_213", HR_WRITE_DISCARD);

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
    }
    hrMaterialClose(matGray);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(6.0f), matGray.id);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef sphere1 = hrLightCreate(L"sphere1");

    hrLightOpen(sphere1, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sphere1);

      lightNode.attribute(L"type").set_value(L"point");
      lightNode.attribute(L"shape").set_value(L"point");
      lightNode.attribute(L"distribution").set_value(L"uniform");

      auto intensityNode = lightNode.append_child(L"intensity");
      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"8 4 8");

			VERIFY_XML(lightNode);
    }
    hrLightClose(sphere1);

    HRLightRef sphere2 = hrLightCreate(L"sphere2");

    hrLightOpen(sphere2, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sphere2);

      lightNode.attribute(L"type").set_value(L"point");
      lightNode.attribute(L"shape").set_value(L"point");
      lightNode.attribute(L"distribution").set_value(L"uniform");

      auto intensityNode = lightNode.append_child(L"intensity");
      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"4 8 4");

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

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 2048);


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

    mTranslate = translate4x4(float3(5.0f, 8.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, sphere1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-5.0f, 8.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, sphere2, mRes.L());


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
        std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
        std::cout.precision(pres);
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_213/z_out.png");

    return check_images("test_213", 1, 30);
  }


	bool test_214_sky_ldr()
	{
		hrErrorCallerPlace(L"test_214");

		hrSceneLibraryOpen(L"tests_f/test_214", HR_WRITE_DISCARD);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Materials
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		HRMaterialRef matGray = hrMaterialCreate(L"matGray");
		HRMaterialRef matRefl = hrMaterialCreate(L"matRefl");


		HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/LA_Downtown_Afternoon_Fishing_B_8k.jpg");


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

			auto refl = matNode.append_child(L"reflectivity");
			refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
			refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
			refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
			refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
			refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
			refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);
		}
		hrMaterialClose(matRefl);


		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Meshes
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		HRMeshRef cubeR    = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matGray.id);
		HRMeshRef sphereG  = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matRefl.id);
		HRMeshRef torusB   = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matGray.id);
		HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);
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
			texNode.append_attribute(L"input_gamma").set_value(2.2f);

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
			camNode.append_child(L"position").text().set(L"0 13 16");
			camNode.append_child(L"look_at").text().set(L"0 0 0");
		}
		hrCameraClose(camRef);

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Render settings
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		HRRenderRef renderRef = CreateBasicTestRenderPTNoCaust(CURR_RENDER_DEVICE, 512, 512, 256, 2048);


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

		mTranslate = translate4x4(float3(0.0f, 2.0f, -1.0f));
		mRes = mul(mTranslate, mRes);

		hrMeshInstance(scnRef, sphereG, mRes.L());

		///////////

		mRes.identity();

		hrLightInstance(scnRef, sky, mRes.L());

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
				std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
				std::cout.precision(pres);
			}

			if (info.finalUpdate)
				break;
		}

		hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_214/z_out.png");

		return check_images("test_214", 1, 30);
	}


	bool test_215_light_scale_intensity()
	{
		hrErrorCallerPlace(L"test_215");

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		hrSceneLibraryOpen(L"tests_f/test_215", HR_WRITE_DISCARD);

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
			diff.append_child(L"color").text().set(L"1 1 1");

			hrTextureBind(testTex2, diff);
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

		HRMeshRef cubeRef = hrMeshCreate(L"my_cube");
		HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
		HRMeshRef planeRef = hrMeshCreate(L"my_plane");
		HRMeshRef sphereRef = hrMeshCreate(L"my_sphere");
		HRMeshRef torusRef = hrMeshCreate(L"my_torus");

		hrMeshOpen(cubeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
		{
			hrMeshVertexAttribPointer4f(cubeRef, L"pos", &cube.vPos[0]);
			hrMeshVertexAttribPointer4f(cubeRef, L"norm", &cube.vNorm[0]);
			hrMeshVertexAttribPointer2f(cubeRef, L"texcoord", &cube.vTexCoord[0]);

			int cubeMatIndices[12] = { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 2, 2 };

			//hrMeshMaterialId(cubeRef, 0);
			hrMeshPrimitiveAttribPointer1i(cubeRef, L"mind", cubeMatIndices);
			hrMeshAppendTriangles3(cubeRef, int(cube.triIndices.size()), &cube.triIndices[0]);
		}
		hrMeshClose(cubeRef);

		hrMeshOpen(cubeOpenRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
		{
			hrMeshVertexAttribPointer4f(cubeOpenRef, L"pos", &cubeOpen.vPos[0]);
			hrMeshVertexAttribPointer4f(cubeOpenRef, L"norm", &cubeOpen.vNorm[0]);
			hrMeshVertexAttribPointer2f(cubeOpenRef, L"texcoord", &cubeOpen.vTexCoord[0]);

			int cubeMatIndices[10] = { mat8.id, mat8.id, mat8.id, mat8.id, mat8.id, mat8.id, mat7.id, mat7.id, mat6.id, mat6.id };

			//hrMeshMaterialId(cubeRef, 0);
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

			sizeNode.append_attribute(L"half_length").set_value(L"1.0");
			sizeNode.append_attribute(L"half_width").set_value(L"1.0");

			pugi::xml_node intensityNode = lightNode.append_child(L"intensity");

			intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
			intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");

			VERIFY_XML(lightNode);
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
		hrRenderEnableDevice(renderRef, 0, true);

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		hrRenderOpen(renderRef, HR_WRITE_DISCARD);
		{
			pugi::xml_node node = hrRenderParamNode(renderRef);

			node.append_child(L"width").text()  = 512;
			node.append_child(L"height").text() = 512;

			node.append_child(L"method_primary").text() = L"pathtracing";
			node.append_child(L"method_secondary").text() = L"pathtracing";
			node.append_child(L"method_tertiary").text() = L"pathtracing";
			node.append_child(L"method_caustic").text() = L"pathtracing";
			node.append_child(L"shadows").text() = L"1";

			node.append_child(L"trace_depth").text() = L"5";
			node.append_child(L"diff_trace_depth").text() = L"3";

			node.append_child(L"pt_error").text() = L"2";
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

		const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

		float matrixT[4][4], matrixT3[4][4], matrixT4[4][4];
		float mRot1[4][4], mTranslate[4][4], mRes[4][4];

		float mTranslateDown[4][4], mRes2[4][4];


		hrSceneOpen(scnRef, HR_WRITE_DISCARD);


		int mmIndex = 0;
		mat4x4_identity(mRot1);
		mat4x4_identity(mTranslate);
		mat4x4_identity(mRes);

		mat4x4_translate(mTranslate, 0.0f, -1.5f, -5.0f + 5.0f);
		mat4x4_rotate_X(mRot1, mRot1, -rquad*DEG_TO_RAD);
		mat4x4_rotate_Y(mRot1, mRot1, -rquad*DEG_TO_RAD*0.5f);
		mat4x4_mul(mRes, mTranslate, mRot1);
		mat4x4_transpose(matrixT, mRes); // this fucking math library swap rows and columns

		mat4x4_identity(mRot1);
		mat4x4_identity(mRes);
		mat4x4_rotate_Y(mRes, mRot1, rquad*DEG_TO_RAD);
		mat4x4_translate(mTranslateDown, -2.0f, -1.5f, -4.0f + 5.0f);
		mat4x4_mul(mRes2, mTranslateDown, mRes);
		mat4x4_transpose(matrixT3, mRes2);

		mat4x4_identity(mRot1);
		mat4x4_identity(mTranslate);
		mat4x4_identity(mRes);

		mat4x4_translate(mTranslate, 2.0f, -1.25f, -5.0f + 5.0f);
		mat4x4_rotate_X(mRot1, mRot1, rquad*DEG_TO_RAD);
		mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD*0.5f);
		mat4x4_mul(mRes, mTranslate, mRot1);
		mat4x4_transpose(matrixT4, mRes); // this fucking math library swap rows and columns

		hrMeshInstance(scnRef, cubeRef, &matrixT[0][0]);
		hrMeshInstance(scnRef, sphereRef, &matrixT3[0][0]);
		hrMeshInstance(scnRef, torusRef, &matrixT4[0][0]);

		mat4x4_identity(mRot1);
		mat4x4_rotate_Y(mRot1, mRot1, 180.0f*DEG_TO_RAD);
		//mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD);
		mat4x4_transpose(matrixT, mRot1);
		hrMeshInstance(scnRef, cubeOpenRef, &matrixT[0][0]);

		/////////////////////////////////////////////////////////////////////// begin instance light (!!!)

		mat4x4_identity(mTranslate);
		mat4x4_identity(mRot1);

		mat4x4_translate(mTranslate, -3.5f, 3.85f, -1.5f);
		mat4x4_scale(mRot1, mRot1, 0.5f);
		mat4x4_mul(mRes, mTranslate, mRot1);
		mat4x4_transpose(matrixT, mRes);
		hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

		mat4x4_identity(mTranslate);
		mat4x4_identity(mRot1);

		mat4x4_translate(mTranslate, 2.0f, 3.85f, -2.0f);
		mat4x4_scale(mRot1, mRot1, 2.0f);
		mat4x4_mul(mRes, mTranslate, mRot1);
		mat4x4_transpose(matrixT, mRes);
		hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

		mat4x4_identity(mTranslate);
		mat4x4_identity(mRot1);

		mat4x4_translate(mTranslate, -3.5f, 3.85f, 2.0f);
		mat4x4_scale_aniso(mRot1, mRot1, 0.5f, 1.0f, 2.0f);
		mat4x4_mul(mRes, mTranslate, mRot1);
		mat4x4_transpose(matrixT, mRes);
		hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

		mat4x4_identity(mTranslate);
		mat4x4_identity(mRot1);

		mat4x4_translate(mTranslate, 2.5f, 3.85f, 2.0f);
		mat4x4_scale_aniso(mRot1, mRot1, 2.0f, 1.0f, 0.5f);
		mat4x4_mul(mRes, mTranslate, mRot1);
		mat4x4_transpose(matrixT, mRes);
		hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

		/////////////////////////////////////////////////////////////////////// end instance light (!!!)

		hrSceneClose(scnRef);

		hrFlush(scnRef, renderRef);

		while (true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(500));

			HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

			if (info.haveUpdateFB)
			{
				auto pres = std::cout.precision(2);
				std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
				std::cout.precision(pres);
			}

			if (info.finalUpdate)
				break;
		}

		hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_215/z_out.png");

		return check_images("test_215", 1, 20.0f);
	}

	bool test_216_ies4()
	{
		hrErrorCallerPlace(L"test_216");

		hrSceneLibraryOpen(L"tests_f/test_216", HR_WRITE_DISCARD);

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
		}
		hrMaterialClose(matGray);


		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Meshes
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matGray.id);
		HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matGray.id);
		HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(6.0f), matGray.id);
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Light
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		HRLightRef light1 = hrLightCreate(L"light1");

		hrLightOpen(light1, HR_WRITE_DISCARD);
		{
			auto lightNode = hrLightParamNode(light1);

			lightNode.attribute(L"type").set_value(L"area");
			lightNode.attribute(L"shape").set_value(L"disk");
			lightNode.attribute(L"distribution").set_value(L"ies");

			auto distribution = lightNode.append_child(L"ies");
			distribution.append_attribute(L"data").set_value(L"data/ies/ies_5.ies");

			distribution.append_attribute(L"matrix");
			float samplerMatrix[16] = { 1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1 };

			HydraXMLHelpers::WriteMatrix4x4(distribution, L"matrix", samplerMatrix);

			auto sizeNode = lightNode.append_child(L"size").append_attribute(L"radius").set_value(L"0.5");

			auto intensityNode = lightNode.append_child(L"intensity");

			intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.5 1.0 0.5");
			intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"100.0");

			VERIFY_XML(lightNode);
		}
		hrLightClose(light1);

		HRLightRef light2 = hrLightCreate(L"light2");

		hrLightOpen(light2, HR_WRITE_DISCARD);
		{
			auto lightNode = hrLightParamNode(light2);

			lightNode.attribute(L"type").set_value(L"point");
			lightNode.attribute(L"shape").set_value(L"point");
			lightNode.attribute(L"distribution").set_value(L"ies");

			auto distribution = lightNode.append_child(L"ies");
			distribution.append_attribute(L"data").set_value(L"data/ies/ies_16.ies");

			distribution.append_attribute(L"matrix");
			float samplerMatrix[16] = { 1, 0, 0, 0,
				                          0, 1, 0, 0,
				                          0, 0, 1, 0,
				                          0, 0, 0, 1 };

			HydraXMLHelpers::WriteMatrix4x4(distribution, L"matrix", samplerMatrix);

			auto sizeNode = lightNode.append_child(L"size");
			sizeNode.append_child(L"radius").text().set(L"0.15");

			auto intensityNode = lightNode.append_child(L"intensity");

			intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1.0 0.5 1.0");
			intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"20.0");

			VERIFY_XML(lightNode);
		}
		hrLightClose(light2);



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

		HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 2048);


		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Create scene
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

		using namespace HydraLiteMath;

		float4x4 mRot, mRot2, mTranslate, mScale, mRes;

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

		mTranslate = translate4x4(float3(3.0f, 6.0f, 0.0f));
		mRot       = rotate_Z_4x4(-10.0f*DEG_TO_RAD);
		mRot2      = rotate_X_4x4(-15.0f*DEG_TO_RAD);
		mRes       = mul(mTranslate, mul(mRot, mul(mRot2,mRes)));
		hrLightInstance(scnRef, light1, mRes.L());
		///////////

		mTranslate.identity();
		mRes.identity();

		mTranslate = translate4x4(float3(-4.5f, 6.5f, 0.0f));
		mRot       = rotate_Y_4x4(-90.0f*DEG_TO_RAD);
		mRot2      = rotate_X_4x4(25.0f*DEG_TO_RAD);
		mRes       = mul(mTranslate, mul(mRot2, mul(mRot, mRes)));
		hrLightInstance(scnRef, light2, mRes.L());
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
				std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
				std::cout.precision(pres);
			}

			if (info.finalUpdate)
				break;
		}

		hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_216/z_out.png");

		return check_images("test_216", 1, 15);
	}
 


  bool test_217_cylinder()
  {
    hrErrorCallerPlace(L"test_217");

    hrSceneLibraryOpen(L"tests_f/test_217", HR_WRITE_DISCARD);

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

    HRLightRef spot2 = hrLightCreate(L"spot2");

    hrLightOpen(spot2, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(spot2);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"cylinder");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      pugi::xml_node sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"radius").set_value(0.5f);
      sizeNode.append_attribute(L"height").set_value(4.0f);
      sizeNode.append_attribute(L"angle").set_value(360.0f);

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
			intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");
			
			VERIFY_XML(lightNode);
    }
    hrLightClose(spot2);

    HRLightRef spot3 = hrLightCreate(L"spot3");

    hrLightOpen(spot3, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(spot3);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"half_length").set_value(1.0f);
      sizeNode.append_attribute(L"half_width").set_value(1.0f);

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
			intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");

			VERIFY_XML(lightNode);
    }
    hrLightClose(spot3);


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

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 2048);


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

    mTranslate = translate4x4(float3(-3.0f, 8.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, spot2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    
    mTranslate = translate4x4(float3(0.0f, -2.5f, 0.0f));
    mRot = rotate_Z_4x4(180.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);
    
    hrLightInstance(scnRef, spot3, mRes.L());

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
        std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
        std::cout.precision(pres);
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_217/z_out.png");

    return check_images("test_217", 1, 25);
  }

  bool test_218_cylinder2()
  {
    hrErrorCallerPlace(L"test_218");
    hrSceneLibraryOpen(L"tests_f/test_218", HR_WRITE_DISCARD);

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
    HRMeshRef sph1     = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef sph2     = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(6.0f), matGray.id);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef spot2 = hrLightCreate(L"spot2");

    hrLightOpen(spot2, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(spot2);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"cylinder");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      pugi::xml_node sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"radius").set_value(0.5f);
      sizeNode.append_attribute(L"height").set_value(4.0f);
      sizeNode.append_attribute(L"angle").set_value(360.0f);

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");

      VERIFY_XML(lightNode);
    }
    hrLightClose(spot2);

    HRLightRef spot3 = hrLightCreate(L"spot3");

    hrLightOpen(spot3, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(spot3);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"half_length").set_value(1.0f);
      sizeNode.append_attribute(L"half_width").set_value(1.0f);

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");

      VERIFY_XML(lightNode);
    }
    hrLightClose(spot3);


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

    hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
    //hrRenderEnableDevice(renderRef, 0, true);
    //hrRenderEnableDevice(renderRef, 1, true);

    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      auto node = hrRenderParamNode(renderRef);

      node.append_child(L"width").text()  = 512;
      node.append_child(L"height").text() = 512;

      node.append_child(L"method_primary").text()   = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text()  = L"pathtracing";
      node.append_child(L"method_caustic").text()   = L"pathtracing";
      node.append_child(L"shadows").text()          = L"1";

      node.append_child(L"trace_depth").text()      = L"5";
      node.append_child(L"diff_trace_depth").text() = L"3";

      node.append_child(L"pt_error").text()         = L"2";
      node.append_child(L"minRaysPerPixel").text()  = 256;
      node.append_child(L"maxRaysPerPixel").text()  = 2048;
    }
    hrRenderClose(renderRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scene
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace HydraLiteMath;

    float4x4 mRot, mTranslate, mScale, mRes;
    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    ///////////

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

    mTranslate = translate4x4(float3(3.0f, 2.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 8.0f, 0.0f));
    mRot = rotate_Y_4x4(90.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrLightInstance(scnRef, spot2, mRes.L());

    ///////////
    float4x4 mRot2;
    mTranslate = translate4x4(float3(4.0f, -1.5f, 0.0f));
    mRot  = rotate_Y_4x4(20.0f*DEG_TO_RAD);
    mRot2 = rotate_X_4x4(-20.0f*DEG_TO_RAD);
    mRes  = mul(mTranslate, mul(mRot, mRot2));

    hrLightInstance(scnRef, spot2, mRes.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);
    
    int counter = 0;

    while (true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

      if (info.haveUpdateFB)
      {
        auto pres = std::cout.precision(2);
        std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
        std::cout.precision(pres);
        counter++;
      }

      if (info.finalUpdate)
        break;

      // if (counter == 10)
      // {
      //   hrRenderCommand(renderRef, L"exitnow");
      //   break;
      // }
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_218/z_out.png");

    return check_images("test_218", 1, 25);
  }



}
