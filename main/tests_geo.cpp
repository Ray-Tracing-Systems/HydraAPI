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
#include "simplerandom.h"

#include "../hydra_api/HR_HDRImageTool.h"
#include "../hydra_api/HydraXMLHelpers.h"

#pragma warning(disable:4996)

extern GLFWwindow* g_window;

using namespace TEST_UTILS;


HAPI HRMeshRef hrMeshCreateFromFileDL_NoNormals(const wchar_t* a_fileName);

namespace GEO_TESTS
{

  static const int TEST_IMG_SIZE = 512;

  bool test_001_mesh_from_memory()
  {
    initGLIfNeeded();

    hrErrorCallerPlace(L"test_001");


    hrSceneLibraryOpen(L"tests_f/test_001", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMaterialRef mat0 = hrMaterialCreate(L"mat0");
    HRMaterialRef mat1 = hrMaterialCreate(L"mat1");

    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat0);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

	    auto color = diff.append_child(L"color");
	    color.append_attribute(L"val").set_value(L"0.5 0.75 0.5");

			VERIFY_XML(matNode);
    }
    hrMaterialClose(mat0);

    hrMaterialOpen(mat1, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat1);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

	    auto color = diff.append_child(L"color");
	    color.append_attribute(L"val").set_value(L"0.25 0.25 0.25");

			VERIFY_XML(matNode);
    }
    hrMaterialClose(mat1);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    SimpleMesh cube = CreateCube(0.75f);
    SimpleMesh plane = CreatePlane(10.0f);
    HRMeshRef cubeRef = hrMeshCreate(L"my_cube");
    HRMeshRef planeRef = hrMeshCreate(L"my_plane");


    hrMeshOpen(cubeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(cubeRef, L"pos", &cube.vPos[0]);
      hrMeshVertexAttribPointer4f(cubeRef, L"norm", &cube.vNorm[0]);
      hrMeshVertexAttribPointer2f(cubeRef, L"texcoord", &cube.vTexCoord[0]);

      hrMeshMaterialId(cubeRef, mat0.id);

      hrMeshAppendTriangles3(cubeRef, int(cube.triIndices.size()), &cube.triIndices[0]);
    }
    hrMeshClose(cubeRef);

    hrMeshOpen(planeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(planeRef, L"pos", &plane.vPos[0]);
      hrMeshVertexAttribPointer4f(planeRef, L"norm", &plane.vNorm[0]);
      hrMeshVertexAttribPointer2f(planeRef, L"texcoord", &plane.vTexCoord[0]);

      hrMeshMaterialId(planeRef, mat1.id);
      hrMeshAppendTriangles3(planeRef, int32_t(plane.triIndices.size()), &plane.triIndices[0]);
    }
    hrMeshClose(planeRef);

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

      sizeNode.append_attribute(L"half_length").set_value(1.0f);
      sizeNode.append_attribute(L"half_width").set_value(1.0f);

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"8.0");

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
      camNode.append_child(L"position").text().set(L"0 0 15");
      camNode.append_child(L"look_at").text().set(L"0 0 0");

			VERIFY_XML(camNode);
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, TEST_IMG_SIZE, TEST_IMG_SIZE, 256, 2048);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scene
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    float matrixT[4][4];
    float mTranslate[4][4];
    float mRes[4][4];
    float mRes2[4][4];

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

    mat4x4_translate(mTranslate, -2.0f, 0.0f, 0.0f);
    mat4x4_mul(mRes2, mTranslate, mRes);
    mat4x4_transpose(matrixT, mRes2);

    hrMeshInstance(scnRef, cubeRef, &matrixT[0][0]);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, 2.0f, 0.0f, 0.0f);
    mat4x4_mul(mRes2, mTranslate, mRes);
    mat4x4_transpose(matrixT, mRes2); //swap rows and columns

    hrMeshInstance(scnRef, cubeRef, &matrixT[0][0]);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_translate(mTranslate, 0, 3.85f, 0);
    mat4x4_transpose(matrixT, mTranslate);

    hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

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

        glfwSwapBuffers(g_window);
        glfwPollEvents();
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_001/z_out.png");

    return check_images("test_001");
  }

  bool test_002_mesh_from_vsgf()
  {
    
    initGLIfNeeded();

    hrErrorCallerPlace(L"test_002");

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    hrSceneLibraryOpen(L"tests_f/test_002", HR_WRITE_DISCARD);

    //the mesh we're going to load has material id attribute set to 1
    //so we first need to add the material with id 0 and then add material for the mesh 
    HRMaterialRef mat0_unused = hrMaterialCreate(L"mat0_unused"); 
    HRMaterialRef mat1 = hrMaterialCreate(L"mat1");

    hrMaterialOpen(mat1, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat1);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.3 0.3 0.85");

			VERIFY_XML(matNode);
    }
    hrMaterialClose(mat1);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   
    HRMeshRef lucyRef = hrMeshCreateFromFileDL(L"data/meshes/lucy.vsgf");


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

      sizeNode.append_attribute(L"half_length").set_value(5.0f);
      sizeNode.append_attribute(L"half_width").set_value(5.0f);

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"8.0");

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
      camNode.append_child(L"position").text().set(L"0 0 15");
      camNode.append_child(L"look_at").text().set(L"0 0 0");

			VERIFY_XML(camNode);
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, TEST_IMG_SIZE, TEST_IMG_SIZE, 256, 2048);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scene
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    float matrixT[4][4];
    float mTranslate[4][4];
    float mRes[4][4];
    float mRes2[4][4];

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, 0.0f, -5.0f, 0.0f);
    mat4x4_mul(mRes2, mTranslate, mRes);
    mat4x4_transpose(matrixT, mRes2);

    hrMeshInstance(scnRef, lucyRef, &matrixT[0][0]);


    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_translate(mTranslate, 0, 10.0f, 0);
    mat4x4_transpose(matrixT, mTranslate);

    hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

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

        glfwSwapBuffers(g_window);
        glfwPollEvents();
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_002/z_out.png");

    return check_images("test_002");
  }

  bool test_003_compute_normals()
  {
   
    initGLIfNeeded();

    hrErrorCallerPlace(L"test_003");
    hrSceneLibraryOpen(L"tests_f/test_003", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMaterialRef mat0 = hrMaterialCreate(L"mat0");
    HRMaterialRef mat1 = hrMaterialCreate(L"mat1");

    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat0);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");

			VERIFY_XML(matNode);
    }
    hrMaterialClose(mat0);

    hrMaterialOpen(mat1, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat1);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.3 0.3 0.85");

      VERIFY_XML(matNode);
    }
    hrMaterialClose(mat1);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMeshRef teapotRef = hrMeshCreateFromFileDL_NoNormals(L"data/meshes/teapot.vsgf");
    HRMeshRef lucyRef = hrMeshCreateFromFileDL_NoNormals(L"data/meshes/lucy.vsgf");
    
/*
    HRMeshRef teapotRef = hrMeshCreateFromFileDL(L"data/meshes/teapot.vsgf");
    HRMeshRef lucyRef = hrMeshCreateFromFileDL(L"data/meshes/lucy.vsgf");

    */
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

      sizeNode.append_attribute(L"half_length").set_value(5.0f);
      sizeNode.append_attribute(L"half_width").set_value(5.0f);

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"8.0");

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
      camNode.append_child(L"position").text().set(L"0 0 15");
      camNode.append_child(L"look_at").text().set(L"0 0 0");

			VERIFY_XML(camNode);
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, TEST_IMG_SIZE, TEST_IMG_SIZE, 256, 2048);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scene
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    float matrixT[4][4];
    float mTranslate[4][4];
    float mRes[4][4];
    float mRes2[4][4];
    float mScale[4][4];

    const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;


    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, 3.0f, -5.0f, 0.0f);
    mat4x4_mul(mRes2, mTranslate, mRes);
    mat4x4_transpose(matrixT, mRes2);

    hrMeshInstance(scnRef, lucyRef, &matrixT[0][0]);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);
    mat4x4_identity(mScale);

    mat4x4_translate(mTranslate, -3.0f, -1.0f, 0.0f);
    mat4x4_scale(mScale, mScale, 4.0f);
    mat4x4_mul(mRes2, mTranslate, mScale);
    mat4x4_transpose(matrixT, mRes2);

    hrMeshInstance(scnRef, teapotRef, &matrixT[0][0]);


    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_translate(mTranslate, 0, 10.0f, 0);
    mat4x4_transpose(matrixT, mTranslate);

    hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

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

        glfwSwapBuffers(g_window);
        glfwPollEvents();
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_003/z_out.png");

    return check_images("test_003");

  }



  bool test_004_dof()
  {

    initGLIfNeeded();

    hrErrorCallerPlace(L"test_004");

    HRCameraRef    camRef;
    HRSceneInstRef scnRef;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    hrSceneLibraryOpen(L"tests_f/test_004", HR_WRITE_DISCARD);

    // geometry
    //
    HRMeshRef cubeRef  = HRMeshFromSimpleMesh(L"my_cube", CreateCube(0.25f), 1);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), 2);
    HRMeshRef sphRef   = HRMeshFromSimpleMesh(L"my_sphere", CreateSphere(0.5f, 32), 3);
    HRMeshRef torRef   = HRMeshFromSimpleMesh(L"my_torus", CreateTorus(0.1f, 0.3f, 32, 32), 0);

    // material and textures
    //
    HRTextureNodeRef testTex2 = hrTexture2DCreateFromFile(L"data/textures/chess_red.bmp");

    HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
    HRMaterialRef mat1 = hrMaterialCreate(L"mysimplemat2");
    HRMaterialRef mat2 = hrMaterialCreate(L"wood");
    HRMaterialRef mat3 = hrMaterialCreate(L"rock");

    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat0);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.75 0.75 0.5");

      HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
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

      HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/163.jpg");
      hrTextureBind(testTex, diff);
    }
    hrMaterialClose(mat3);


    HRLightRef directLight = hrLightCreate(L"my_direct_light");

    hrLightOpen(directLight, HR_WRITE_DISCARD);
    {
      pugi::xml_node lightNode = hrLightParamNode(directLight);

      lightNode.attribute(L"type").set_value(L"directional");
      lightNode.attribute(L"shape").set_value(L"point");

      // lightNode.append_child(L"direction").text() = L"-0.340738 -0.766534 -0.544356"; // will be recalculated and overrided by light instance matrix

      pugi::xml_node sizeNode = lightNode.append_child(L"size");

      sizeNode.append_child(L"inner_radius").text().set(L"10.0");
      sizeNode.append_child(L"outer_radius").text().set(L"20.0");

      pugi::xml_node intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").text().set(L"1 1 1");
      intensityNode.append_child(L"multiplier").text().set(L"2.0");
    }
    hrLightClose(directLight);

    HRLightRef sky = hrLightCreate(L"sky");

    hrLightOpen(sky, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sky);

      lightNode.attribute(L"type").set_value(L"sky");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.25 0.25 0.5");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);

      VERIFY_XML(lightNode);
    }
    hrLightClose(sky);

    // camera
    //
    camRef = hrCameraCreate(L"my camera");

    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      xml_node camNode = hrCameraParamNode(camRef);

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"-8 2 15");
      camNode.append_child(L"look_at").text().set(L"0.1 0 0.1");

      camNode.append_child(L"enable_dof").text()      = 1;
      camNode.append_child(L"dof_lens_radius").text() = 0.06f;
    }
    hrCameraClose(camRef);



    // create scene
    //
    scnRef = hrSceneCreate(L"my scene");

    float	rtri = 25.0f; // Angle For The Triangle ( NEW )
    float	rquad = 40.0f;
    float g_FPS = 60.0f;
    int   frameCounter = 0;

    const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

    float matrixT[4][4], matrixT2[4][4];
    float mRot1[4][4], mTranslate[4][4], mRes[4][4];
    float mTranslateDown[4][4];

    mat4x4_identity(mRot1);
    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_identity(mRes);
    mat4x4_translate(mTranslateDown, 0.0f, -1.0f, -5.0f);
    mat4x4_mul(mRes, mTranslateDown, mRes);
    mat4x4_transpose(matrixT2, mRes);

    ////
    ////

    HRMeshRef refs[3] = { cubeRef, sphRef, torRef };

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    hrMeshInstance(scnRef, planeRef, &matrixT2[0][0]);

    auto rgen = simplerandom::RandomGenInit(878976);

    for (int i = -5; i <= 5; i++)
    {
      for (int j = -5; j <= 5; j++)
      {
        float xCoord = 2.5f*float(i);
        float yCoord = 2.5f*float(j);

        mat4x4_identity(mRot1);
        mat4x4_identity(mTranslate);
        mat4x4_identity(mRes);

        mat4x4_translate(mTranslate, xCoord, 0.1f, yCoord);
        mat4x4_rotate_X(mRot1, mRot1, simplerandom::rnd(rgen, 0.0f, 360.0f)*DEG_TO_RAD);
        mat4x4_rotate_Y(mRot1, mRot1, simplerandom::rnd(rgen, 0.0f, 360.0f)*DEG_TO_RAD*0.5f);
        mat4x4_mul(mRes, mTranslate, mRot1);
        mat4x4_transpose(matrixT, mRes); // this fucking math library swap rows and columns

        hrMeshInstance(scnRef, refs[simplerandom::rand(rgen) % 3], &matrixT[0][0]);
      }
    }

    // add directional light
    //
    mat4x4_identity(mRot1);
    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_identity(mRes);

    mat4x4_rotate_Z(mRot1, mRot1, 1.0f*DEG_TO_RAD);
    //mat4x4_rotate_Z(mRot1, mRot1, 0.0f*DEG_TO_RAD);
    mat4x4_translate(mTranslateDown, 0.0f, 100.0f, 0.0f);

    mat4x4_mul(mRes, mRot1, mRes);
    mat4x4_mul(mRes, mTranslateDown, mRes);

    mat4x4_transpose(matrixT2, mRes);
    hrLightInstance(scnRef, directLight, &matrixT2[0][0]);
    hrLightInstance(scnRef, sky,         &matrixT2[0][0]);

    hrSceneClose(scnRef);

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

    hrRenderEnableDevice(renderRef, 0, true);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      pugi::xml_node node = hrRenderParamNode(renderRef);

      node.append_child(L"width").text()  = TEST_IMG_SIZE;
      node.append_child(L"height").text() = TEST_IMG_SIZE;

      node.append_child(L"method_primary").text() = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text() = L"pathtracing";
      node.append_child(L"method_caustic").text() = L"pathtracing";
      node.append_child(L"shadows").text() = L"1";

      node.append_child(L"trace_depth").text() = L"5";
      node.append_child(L"diff_trace_depth").text() = L"3";

      node.append_child(L"maxRaysPerPixel").text() = L"1024";
      node.append_child(L"offline_pt").text()      = L"1";
    }
    hrRenderClose(renderRef);


    hrFlush(scnRef, renderRef);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    while (true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

      if (info.haveUpdateFB)
      {
        auto pres = std::cout.precision(2);
        std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
        std::cout.precision(pres);

        glfwSwapBuffers(g_window);
        glfwPollEvents();
      }

      if (info.finalUpdate)
        break;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_004/z_out.png");

    return check_images("test_004", 1, 20);
  }


  bool test_005_instancing()
  {
    initGLIfNeeded();

    hrErrorCallerPlace(L"test_005");

    hrSceneLibraryOpen(L"tests_f/test_005", HR_WRITE_DISCARD);

    HRTextureNodeRef texPattern = hrTexture2DCreateFromFile(L"data/textures/bigleaf3.tga");
    HRTextureNodeRef texture1   = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMaterialRef matGray     = hrMaterialCreate(L"matGray");
    HRMaterialRef matTrunk    = hrMaterialCreate(L"Trunk");
    HRMaterialRef matWigglers = hrMaterialCreate(L"Wigglers");
    HRMaterialRef matBranches = hrMaterialCreate(L"Branches");
    HRMaterialRef matPllarRts = hrMaterialCreate(L"PillarRoots");
    HRMaterialRef matLeaves   = hrMaterialCreate(L"Leaves");
    HRMaterialRef matCanopy   = hrMaterialCreate(L"Canopy");
    HRMaterialRef matCube     = hrMaterialCreate(L"CubeMap");

    hrMaterialOpen(matCube, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matCube);

      auto diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

      hrTextureBind(texture1, diff.child(L"color"));
    }
    hrMaterialClose(matCube);

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

    }
    hrMaterialClose(matGray);

    hrMaterialOpen(matTrunk, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matTrunk);
      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.345098 0.215686 0.0117647");
    }
    hrMaterialClose(matTrunk);

    hrMaterialOpen(matWigglers, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matWigglers);
      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.345098 0.215686 0.0117647");
    }
    hrMaterialClose(matWigglers);

    hrMaterialOpen(matBranches, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBranches);
      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.345098 0.215686 0.0117647");
    }
    hrMaterialClose(matBranches);

    hrMaterialOpen(matPllarRts, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matPllarRts);
      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.345098 0.215686 0.0117647");
    }
    hrMaterialClose(matPllarRts);

    //////////////////////////////////

    hrMaterialOpen(matLeaves, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matLeaves);
      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0533333 0.208627 0.00627451");

      auto opacity = matNode.append_child(L"opacity");
      auto texNode = hrTextureBind(texPattern, opacity);

    }
    hrMaterialClose(matLeaves);

    hrMaterialOpen(matCanopy, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matCanopy);
      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0941176 0.4 0");
    }
    hrMaterialClose(matCanopy);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    HRMeshRef cubeR    = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matCube.id);
    HRMeshRef pillar   = HRMeshFromSimpleMesh(L"pillar", CreateCube(1.0f), matGray.id);
    HRMeshRef sphereG  = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matTrunk.id);
    HRMeshRef torusB   = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matTrunk.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(10000.0f), matGray.id);
    
    HRMeshRef treeRef  = hrMeshCreateFromFileDL(L"data/meshes/bigtree.vsgf");
    
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

      sunModel.append_attribute(L"sun_id").set_value(sun.id);
      sunModel.append_attribute(L"turbidity").set_value(2.0f);

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
      sizeNode.append_attribute(L"outer_radius").set_value(L"10000.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1.0 0.85 0.64");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"2.0");

      lightNode.append_child(L"shadow_softness").append_attribute(L"val").set_value(1.0f);

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
      camNode.append_child(L"position").text().set(L"0 15 25");
      camNode.append_child(L"look_at").text().set(L"5 10 0");
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, TEST_IMG_SIZE, TEST_IMG_SIZE, 256, 2048);


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

    auto rgen = simplerandom::RandomGenInit(125);

    {
      const float dist1 = 40.0f;
      const int SQUARESIZE1 = 100;

      for (int i = -SQUARESIZE1; i < SQUARESIZE1; i++)
      {
        for (int j = -SQUARESIZE1; j < SQUARESIZE1; j++)
        {
          const float2 randOffset = float2(simplerandom::rnd(rgen, -1.0f, 1.0f), simplerandom::rnd(rgen, -1.0f, 1.0f));
          const float3 pos        = dist1*float3(float(i), 0.0f, float(j)) + dist1*1.0f*float3(randOffset.x, 0.0f, randOffset.y);

          mTranslate = translate4x4(float3(pos.x, 1.0f, pos.z));
          mRot       = rotate_Y_4x4(simplerandom::rnd(rgen, -180.0f*DEG_TO_RAD, +180.0f*DEG_TO_RAD));
          mRes       = mul(mTranslate, mRot);

          hrMeshInstance(scnRef, cubeR, mRes.L());
        }
      }
    }

    ///////////
    {
      const float dist = 40.0f;
      const int SQUARESIZE = 100;

      for (int i = -SQUARESIZE; i < SQUARESIZE; i++)
      {
        for (int j = -SQUARESIZE; j < SQUARESIZE; j++)
        {
          const float2 randOffset = float2(simplerandom::rnd(rgen, -1.0f, 1.0f), simplerandom::rnd(rgen, -1.0f, 1.0f));
          const float3 pos = dist*float3(float(i), 0.0f, float(j)) + dist*0.5f*float3(randOffset.x, 0.0f, randOffset.y);

          mTranslate = translate4x4(pos);
          mScale     = scale4x4(float3(5.0f, 5.0f, 5.0f));
          mRot       = rotate_Y_4x4(simplerandom::rnd(rgen, -180.0f*DEG_TO_RAD, +180.0f*DEG_TO_RAD));
          mRes       = mul(mTranslate, mul(mRot, mScale));

          if ((simplerandom::rnd(rgen, 0.0f, 1.0f) > 0.5f))
            hrMeshInstance(scnRef, treeRef, mRes.L());
        }
      }
    }

    ///////////
    ///////////

    mRes.identity();
    hrLightInstance(scnRef, sky, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mRot2.identity();

    mTranslate = translate4x4(float3(200.0f, 200.0f, -100.0f));
    mRot  = rotate_X_4x4(10.0f*DEG_TO_RAD);
    mRot2 = rotate_Z_4x4(30.f*DEG_TO_RAD);
    mRes  = mul(mRot2, mRot);
    mRes  = mul(mTranslate, mRes);

    hrLightInstance(scnRef, sun, mRes.L());
    
    
    hrSceneClose(scnRef);
    
    HRRenderRef renderRef = hrRenderCreate(L"HydraModern");
    
    hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);

    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      auto node = hrRenderParamNode(renderRef);

      node.append_child(L"width").text()  = TEST_IMG_SIZE;
      node.append_child(L"height").text() = TEST_IMG_SIZE;

      node.append_child(L"method_primary").text() = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text() = L"pathtracing";
      node.append_child(L"method_caustic").text() = L"pathtracing";
      node.append_child(L"shadows").text() = L"1";

      node.append_child(L"trace_depth").text() = L"5";
      node.append_child(L"diff_trace_depth").text() = L"3";

      node.append_child(L"pt_error").text() = L"2";
      node.append_child(L"minRaysPerPixel").text() = 256;
      node.append_child(L"maxRaysPerPixel").text() = 2048;
    }
    hrRenderClose(renderRef);
    
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

        glfwSwapBuffers(g_window);
        glfwPollEvents();
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_005/z_out.png");

    return check_images("test_005", 1, 20.0f);

  }

  bool test_006_points_on_mesh()
  {

    initGLIfNeeded();
    hrErrorCallerPlace(L"test_006");
    hrSceneLibraryOpen(L"tests_f/test_006", HR_WRITE_DISCARD);

    HRMaterialRef mat1 = hrMaterialCreate(L"mat1");
    HRMaterialRef mat2 = hrMaterialCreate(L"mat2");

    int rep = 8;

    HRTextureNodeRef tex = hrTexture2DCreateBakedLDR(&procTexCheckerLDR, (void *) (&rep), sizeof(int), 32, 32);

    hrMaterialOpen(mat1, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat1);

      auto diffuse = matNode.append_child(L"diffuse");
      diffuse.append_child(L"color").append_attribute(L"val").set_value(L"0.2 0.2 0.120");

      auto texNode = hrTextureBind(tex, diffuse.child(L"color"));
      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 1, 0, 0, 0.0f,
                                  0, 1, 0, 0.0f,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(1.0f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    }
    hrMaterialClose(mat1);

    hrMaterialOpen(mat2, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat2);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.99 0.6 0.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.999");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(8.0f);
    }
    hrMaterialClose(mat2);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef tess = CreateTriStrip(64, 64, 100);//hrMeshCreateFromFileDL(L"data/meshes/teapot.vsgf");//

    HRMeshRef lucyRef = hrMeshCreateFromFileDL(L"data/meshes/lucy.vsgf");
    hrMeshOpen(lucyRef, HR_TRIANGLE_IND3, HR_OPEN_EXISTING);
    {
      hrMeshMaterialId(lucyRef, mat2.id);
    }
    hrMeshClose(lucyRef);

    uint32_t n_points = 100;
    std::vector<float> points(n_points * 3, 0.0f);

    hrMeshOpen(tess, HR_TRIANGLE_IND3, HR_OPEN_EXISTING);
    {
      hrMeshMaterialId(tess, mat1.id);
      HRUtils::getRandomPointsOnMesh(tess, points.data(), n_points, true);
    }
    hrMeshClose(tess);


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
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(0.25f);

      VERIFY_XML(lightNode);
    }
    hrLightClose(sky);


    HRLightRef sphereLight = hrLightCreate(L"my_area_light");

    hrLightOpen(sphereLight, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sphereLight);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"sphere");
      lightNode.attribute(L"distribution").set_value(L"uniform");

      auto intensityNode = lightNode.append_child(L"intensity");
      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"6.0");

      auto sizeNode = lightNode.append_child(L"size").append_attribute(L"radius").set_value(5.5f);
    }
    hrLightClose(sphereLight);


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
      camNode.append_child(L"position").text().set(L"0 50 100");
      camNode.append_child(L"look_at").text().set(L"0 0 0");
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 1024);


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
    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
    mScale = scale4x4(float3(0.51f, 0.51f, 0.51f));
    mRes = mul(mTranslate, mScale);

    auto pt_matrix = mRes;

    hrMeshInstance(scnRef, tess, mRes.L());

    for(int i = 0; i < points.size(); i += 3)
    {
      auto pt = make_float3(points[i * 3], points[i * 3 + 1], points[i * 3 + 2]);
      pt = mul(pt_matrix, pt);
      mTranslate = translate4x4(pt);
      mScale = scale4x4(float3(1.0f, 1.0f, 1.0f));
      mRes = mul(mTranslate, mScale);

      hrMeshInstance(scnRef, lucyRef, mRes.L());
    }

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0, 0.0f, 0.0));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, sky, mRes.L());

    mTranslate = translate4x4(float3(0, 40.0f, 0.0));
    hrLightInstance(scnRef, sphereLight, mTranslate.L());

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

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_006/z_out.png");

    return check_images("test_006", 1, 50);
  }

}


