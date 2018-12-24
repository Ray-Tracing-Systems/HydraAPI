#include "tests.h"
#include <math.h>
#include <iomanip>

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
#include <fstream>

#include "../hydra_api/HR_HDRImageTool.h"

#pragma warning(disable:4996)
using namespace TEST_UTILS;

extern GLFWwindow* g_window;

bool test41_load_library_basic()
{
  initGLIfNeeded();
  hrErrorCallerPlace(L"test41");

  hrSceneLibraryOpen(L"tests/test_41", HR_OPEN_EXISTING);

  /////////////////////////////////////////////////////////
  HRRenderRef renderRef;
  renderRef.id = 0;
  
  HRSceneInstRef scnRef;
  scnRef.id = 0;
  /////////////////////////////////////////////////////////

  hrCommit(scnRef, renderRef);

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_41/z_out.png");

  return check_images("test_41");
}


bool test42_load_library_basic()
{
  initGLIfNeeded();
  hrErrorCallerPlace(L"test42");
  hrSceneLibraryOpen(L"tests/test_42", HR_OPEN_EXISTING);

  /////////////////////////////////////////////////////////
  HRRenderRef renderRef;
  renderRef.id = 0;

  HRSceneInstRef scnRef;
  scnRef.id = 0;
  /////////////////////////////////////////////////////////
  
  auto pList = hrRenderGetDeviceList(renderRef);
  
  while (pList != nullptr)
  {
    std::wcout << L"device id = " << pList->id << L", name = " << pList->name << L", driver = " << pList->driver << std::endl;
    pList = pList->next;
  }
  
  hrRenderEnableDevice(renderRef, 0, true);
  //hrRenderEnableDevice(renderRef, 1, true);
  
  hrRenderOpen(renderRef, HR_OPEN_EXISTING);    // disable automatic process run for hrCommit, ... probably need to add custom params to tis function
  {
    auto node = hrRenderParamNode(renderRef);
    //node.force_child(L"forceGPUFrameBuffer").text() = 1;
  }
  hrRenderClose(renderRef);
  
  hrCommit(scnRef, renderRef);
  hrRenderCommand(renderRef, L"resume");

  // hrRenderOpen(renderRef, HR_OPEN_EXISTING);    // disable automatic process run for hrCommit, ... probably need to add custom params to tis function
  // {
  //   auto node = hrRenderParamNode(renderRef);
  //   node.force_child(L"dont_run").text() = 1; // forceGPUFrameBuffer
  // }
  // hrRenderClose(renderRef);
  //
  // hrCommit(scnRef, renderRef);
  // hrRenderCommand(renderRef, L"runhydra -cl_device_id 0 -contribsamples 256 -maxsamples 512");

  auto timeBeg = std::chrono::system_clock::now();
  
  glViewport(0, 0, 1024, 1024);
  std::vector<int32_t> image(1024 * 1024);

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      hrRenderGetFrameBufferLDR1i(renderRef, 1024, 1024, &image[0]);
      
      glDisable(GL_TEXTURE_2D);
      glDrawPixels(1024, 1024, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
      std::cout.precision(pres);

      glfwSwapBuffers(g_window);
      glfwPollEvents();
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_42/z_out.png");
  
  auto timeEnd  = std::chrono::system_clock::now();
  
  auto msPassed = std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeBeg).count();
  
  std::cout << std::endl;
  std::cout << "rendering time (in seconds) = " << double(msPassed)/1000.0 << std::endl;
  
  return check_images("test_42", 1, 35.0f);
}


bool test43_test_direct_light()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test43");

  HRCameraRef    camRef;
  HRSceneInstRef scnRef;
 
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_43", HR_WRITE_DISCARD);

  // geometry
  //
  HRMeshRef cubeRef  = HRMeshFromSimpleMesh(L"my_cube",   CreateCube(0.25f), 1);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane",  CreatePlane(20.0f), 2);
  HRMeshRef sphRef   = HRMeshFromSimpleMesh(L"my_sphere", CreateSphere(0.5f, 32), 3);
  HRMeshRef torRef   = HRMeshFromSimpleMesh(L"my_torus",  CreateTorus(0.1f, 0.3f, 32, 32), 0);

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
    camNode.append_child(L"position").text().set(L"-5 8 25");
    camNode.append_child(L"look_at").text().set(L"0.1 0 0.1");
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

  auto rgen = simplerandom::RandomGenInit(12368754);

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

  //mat4x4_rotate_Z(mRot1, mRot1, 1.0f*DEG_TO_RAD);
  mat4x4_translate(mTranslateDown, 0.0f, 100.0f, 0.0f);
  mat4x4_mul(mRes, mRot1, mRes);
  mat4x4_mul(mRes, mTranslateDown, mRes);
  mat4x4_transpose(matrixT2, mRes);
  hrLightInstance(scnRef, directLight, &matrixT2[0][0]);

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

    node.append_child(L"width").text()  = 512;
    node.append_child(L"height").text() = 512;

    node.append_child(L"method_primary").text()   = L"pathtracing";
    node.append_child(L"method_secondary").text() = L"pathtracing";
    node.append_child(L"method_tertiary").text()  = L"pathtracing";
    node.append_child(L"method_caustic").text()   = L"pathtracing";
    node.append_child(L"shadows").text()          = L"1";

    node.append_child(L"trace_depth").text()      = L"5";
    node.append_child(L"diff_trace_depth").text() = L"3";
    node.append_child(L"maxRaysPerPixel").text()  = L"1024";

  }
  hrRenderClose(renderRef);


  hrFlush(scnRef, renderRef);

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  glViewport(0, 0, 512, 512);
  std::vector<int32_t> image(512 * 512);

  while(true)
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

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_43/z_out.png");

  return check_images("test_43");
}


bool test44_four_lights()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_44");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_44", HR_WRITE_DISCARD);

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
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"4.0");
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
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  HRRenderRef renderRef = hrRenderCreate(L"HydraModern");
  //hrRenderLogDir(renderRef, L"C:/[Hydra]/logs/", false);


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

    node.append_child(L"trace_depth").text() = L"5";
    node.append_child(L"diff_trace_depth").text() = L"3";

    node.append_child(L"pt_error").text() = L"2";
    node.append_child(L"minRaysPerPixel").text() = 256;
    node.append_child(L"maxRaysPerPixel").text() = 2048;
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

  /////////////////////////////////////////////////////////////////////// instance light (!!!)

  mat4x4_identity(mTranslate);
  mat4x4_translate(mTranslate, -1.5f, 3.85f, -1.5f);
  mat4x4_transpose(matrixT, mTranslate);
  hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

  mat4x4_identity(mTranslate);
  mat4x4_translate(mTranslate, 1.5f, 3.85f, -1.5f);
  mat4x4_transpose(matrixT, mTranslate);
  hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

  mat4x4_identity(mTranslate);
  mat4x4_translate(mTranslate, -1.5f, 3.85f, 1.5f);
  mat4x4_transpose(matrixT, mTranslate);
  hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

  mat4x4_identity(mTranslate);
  mat4x4_translate(mTranslate, 1.5f, 3.85f, 1.5f);
  mat4x4_transpose(matrixT, mTranslate);
  hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef);

  glViewport(0, 0, 1024, 768);
  std::vector<int32_t> image(1024 * 768);

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      hrRenderGetFrameBufferLDR1i(renderRef, 1024, 768, &image[0]);

      glDisable(GL_TEXTURE_2D);
      glDrawPixels(1024, 768, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);

      glfwSwapBuffers(g_window);
      glfwPollEvents();
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_44/z_out.png");

  return check_images("test_44", 1, 25.0f);
}


bool test45_mesh_from_vsgf_opengl_bug_teapot()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_45");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_45", HR_WRITE_DISCARD);

  SimpleMesh cube = CreateCube(0.75f);
  SimpleMesh plane = CreatePlane(10.0f);
  SimpleMesh sphere = CreateSphere(1.0f, 32);
  SimpleMesh torus = CreateTorus(0.25f, 0.6f, 32, 32);
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

  HRMeshRef teapotRef = hrMeshCreateFromFileDL(L"data/meshes/teapot.vsgf"); 
  //HRMeshRef teapotRef = hrMeshCreateFromFileDL(L"data/meshes/pyramid.vsgf");

  HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
  HRMeshRef planeRef = hrMeshCreate(L"my_plane");
  HRMeshRef sphereRef = hrMeshCreate(L"my_sphere");
  HRMeshRef torusRef = hrMeshCreate(L"my_torus");

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

    sizeNode.append_attribute(L"half_length") = 1.0f;
    sizeNode.append_attribute(L"half_width")  = 1.0f;

    pugi::xml_node intensityNode = lightNode.append_child(L"intensity");
    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"10 10 10");
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
  HRRenderRef renderRef = hrRenderCreate(L"opengl1"); // opengl1 // 

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);

    node.append_child(L"width").text() = L"1024";
    node.append_child(L"height").text() = L"768";

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

  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

  float matrixT[4][4];
  float mRot1[4][4], mTranslate[4][4], mRes[4][4];
  //float mTranslateDown[4][4];

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);
  mat4x4_translate(mTranslate, 0.0f, -0.70f*3.65f, -5.0f + 5.0f);
  mat4x4_scale(mRot1, mRot1, 3.65f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT, mRes); // this fucking math library swap rows and columns
  matrixT[3][3] = 1.0f;
  hrMeshInstance(scnRef, teapotRef, &matrixT[0][0]);

  mat4x4_identity(mRot1);
  mat4x4_rotate_Y(mRot1, mRot1, 180.0f*DEG_TO_RAD);
  mat4x4_transpose(matrixT, mRot1);
  hrMeshInstance(scnRef, cubeOpenRef, &matrixT[0][0]);

  /////////////////////////////////////////////////////////////////////// instance light (!!!)

  mat4x4_identity(mTranslate);
  mat4x4_translate(mTranslate, 0, 3.85f, 0);
  mat4x4_transpose(matrixT, mTranslate);
  hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef);

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_45/z_out.png");

  return check_images("test_45");
}

bool test46_light_geom_rect()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_46");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_46", HR_WRITE_DISCARD);

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

  HRMeshRef teapotRef = hrMeshCreateFromFileDL(L"data/meshes/teapot.vsgf"); // chunk_00009.vsgf // teapot.vsgf // chunk_00591.vsgf

  HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
  HRMeshRef planeRef = hrMeshCreate(L"my_plane");
  HRMeshRef sphereRef = hrMeshCreate(L"my_sphere");
  HRMeshRef torusRef = hrMeshCreate(L"my_torus");

  // hrMeshOpen(cubeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  // {
  //   hrMeshVertexAttribPointer4f(cubeRef, L"pos", &cube.vPos[0]);
  //   hrMeshVertexAttribPointer4f(cubeRef, L"norm", &cube.vNorm[0]);
  //   hrMeshVertexAttribPointer2f(cubeRef, L"texcoord", &cube.vTexCoord[0]);
  // 
  //   int cubeMatIndices[12] = { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 2, 2 };
  // 
  //   //hrMeshMaterialId(cubeRef, 0);
  //   hrMeshPrimitiveAttribPointer1i(cubeRef, L"mind", cubeMatIndices);
  //   hrMeshAppendTriangles3(cubeRef, int(cube.triIndices.size()), &cube.triIndices[0]);
  // }
  // hrMeshClose(cubeRef);

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

    sizeNode.append_attribute(L"half_length") = 1.0f;
    sizeNode.append_attribute(L"half_width")  = 1.0f;

    pugi::xml_node intensityNode = lightNode.append_child(L"intensity");
    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"10 10 10");
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
  HRRenderRef renderRef = hrRenderCreate(L"opengl1"); // opengl1 // 

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

    node.append_child(L"width").text()  = L"1024";
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
  float mRot1[4][4], mRot2[4][4], mTranslate[4][4], mRes[4][4];
  //float mTranslateDown[4][4];

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  int mmIndex = 0;
  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, -0.70f*3.65f, -5.0f + 5.0f);
  mat4x4_scale(mRot1, mRot1, 3.65f);   
  mat4x4_mul(mRes, mTranslate, mRot1);
  mRes[3][3] = 1.0f;
  mat4x4_transpose(matrixT, mRes); // this fucking math library swap rows and columns

  hrMeshInstance(scnRef, teapotRef, &matrixT[0][0]);

  mat4x4_identity(mRot1);
  mat4x4_rotate_Y(mRot1, mRot1, 180.0f*DEG_TO_RAD);
  //mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD);
  mRot1[3][3] = 1.0f;
  mat4x4_transpose(matrixT, mRot1);
  hrMeshInstance(scnRef, cubeOpenRef, &matrixT[0][0]);

  /////////////////////////////////////////////////////////////////////// instance light (!!!)

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRot1);
  mat4x4_identity(mRot2);

  mat4x4_translate(mTranslate, -1.0f, 3.5f, -1.0f);
  mat4x4_rotate_Y(mRot1, mRot1, 30.0f*DEG_TO_RAD);
  mat4x4_rotate_X(mRot2, mRot2, -30.0f*DEG_TO_RAD);
  mRot1[3][3] = 1.0f;
  mRot2[3][3] = 1.0f;

  mat4x4_mul(mRot1, mRot1, mRot2);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mRes[3][3] = 1.0f;
  
  mat4x4_transpose(matrixT, mRes);
  hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef);

  // glViewport(0, 0, 1024, 768);
  // std::vector<int32_t> image(1024 * 768);
  // 
  // while (true)
  // {
  //   std::this_thread::sleep_for(std::chrono::milliseconds(500));
  // 
  //   HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);
  // 
  //   if (info.haveUpdateFB)
  //   {
  //     hrRenderGetFrameBufferLDR1i(renderRef, 1024, 768, &image[0]);
  // 
  //     glDisable(GL_TEXTURE_2D);
  //     glDrawPixels(1024, 768, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
  // 
  //     auto pres = std::cout.precision(2);
  //     std::cout << "rendering progress = " << info.progress << "% \r";
  //     std::cout.precision(pres);
  // 
  //     glfwSwapBuffers(g_window);
  //     glfwPollEvents();
  //   }
  // 
  //   if (info.finalUpdate)
  //     break;
  // }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_46/z_out.png");

  return check_images("test_46");
}

bool test47_light_geom_disk()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_47");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_47", HR_WRITE_DISCARD);

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

  HRMeshRef teapotRef = hrMeshCreateFromFileDL(L"data/meshes/teapot.vsgf"); // chunk_00009.vsgf // teapot.vsgf // chunk_00591.vsgf

  HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
  HRMeshRef planeRef = hrMeshCreate(L"my_plane");
  HRMeshRef sphereRef = hrMeshCreate(L"my_sphere");
  HRMeshRef torusRef = hrMeshCreate(L"my_torus");

  // hrMeshOpen(cubeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  // {
  //   hrMeshVertexAttribPointer4f(cubeRef, L"pos", &cube.vPos[0]);
  //   hrMeshVertexAttribPointer4f(cubeRef, L"norm", &cube.vNorm[0]);
  //   hrMeshVertexAttribPointer2f(cubeRef, L"texcoord", &cube.vTexCoord[0]);
  // 
  //   int cubeMatIndices[12] = { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 2, 2 };
  // 
  //   //hrMeshMaterialId(cubeRef, 0);
  //   hrMeshPrimitiveAttribPointer1i(cubeRef, L"mind", cubeMatIndices);
  //   hrMeshAppendTriangles3(cubeRef, int(cube.triIndices.size()), &cube.triIndices[0]);
  // }
  // hrMeshClose(cubeRef);

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
    lightNode.attribute(L"shape").set_value(L"disk");
    lightNode.attribute(L"distribution").set_value(L"diffuse");

    lightNode.append_child(L"size").append_attribute(L"radius").set_value(1.0f);

    pugi::xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"10 10 10");
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
  HRRenderRef renderRef = hrRenderCreate(L"opengl1"); // opengl1 // 

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

    node.append_child(L"width").text()  = L"1024";
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
  float mRot1[4][4], mRot2[4][4], mTranslate[4][4], mRes[4][4];
  //float mTranslateDown[4][4];

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  int mmIndex = 0;
  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, -0.70f*3.65f, -5.0f + 5.0f);
  mat4x4_scale(mRot1, mRot1, 3.65f); 
  mat4x4_mul(mRes, mTranslate, mRot1);
  mRes[3][3] = 1.0f;
  mat4x4_transpose(matrixT, mRes); // this fucking math library swap rows and columns

  hrMeshInstance(scnRef, teapotRef, &matrixT[0][0]);

  mat4x4_identity(mRot1);
  mat4x4_rotate_Y(mRot1, mRot1, 180.0f*DEG_TO_RAD);
  //mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD);
  mRot1[3][3] = 1.0f;
  mat4x4_transpose(matrixT, mRot1);
  hrMeshInstance(scnRef, cubeOpenRef, &matrixT[0][0]);

  /////////////////////////////////////////////////////////////////////// instance light (!!!)

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRot1);
  mat4x4_identity(mRot2);

  mat4x4_translate(mTranslate, 1.0f, 3.5f, -1.0f);
  mat4x4_rotate_Y(mRot1, mRot1, -30.0f*DEG_TO_RAD);
  mat4x4_rotate_X(mRot2, mRot2, -30.0f*DEG_TO_RAD);
  mRot1[3][3] = 1.0f;
  mRot2[3][3] = 1.0f;

  mat4x4_mul(mRot1, mRot1, mRot2);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mRes[3][3] = 1.0f;
  
  mat4x4_transpose(matrixT, mRes);
  hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef);

  // glViewport(0, 0, 1024, 768);
  // std::vector<int32_t> image(1024 * 768);
  // 
  // while (true)
  // {
  //   std::this_thread::sleep_for(std::chrono::milliseconds(500));
  // 
  //   HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);
  // 
  //   if (info.haveUpdateFB)
  //   {
  //     hrRenderGetFrameBufferLDR1i(renderRef, 1024, 768, &image[0]);
  // 
  //     glDisable(GL_TEXTURE_2D);
  //     glDrawPixels(1024, 768, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
  // 
  //     auto pres = std::cout.precision(2);
  //     std::cout << "rendering progress = " << info.progress << "% \r";
  //     std::cout.precision(pres);
  // 
  //     glfwSwapBuffers(g_window);
  //     glfwPollEvents();
  //   }
  // 
  //   if (info.finalUpdate)
  //     break;
  // }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_47/z_out.png");

  return check_images("test_47");
}


bool test48_light_geom_sphere()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_48");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_48", HR_WRITE_DISCARD);

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

    lightNode.attribute(L"type").set_value(L"sphere");
    lightNode.attribute(L"shape").set_value(L"sphere");
    lightNode.attribute(L"distribution").set_value(L"uniform");

    lightNode.append_child(L"size").append_attribute(L"radius").set_value(0.65f);

    pugi::xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"10 10 10");
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
  HRRenderRef renderRef = hrRenderCreate(L"opengl1"); // opengl1 // 

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

  static GLfloat	rtri  = 25.0f; // Angle For The Triangle ( NEW )
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

  mat4x4_translate(mTranslate, 0.0f, -0.70f*3.65f, -5.0f + 5.0f);
  mat4x4_scale(mRot1, mRot1, 3.65f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mRes[3][3] = 1.0f;
  mat4x4_transpose(matrixT, mRes); // this fucking math library swap rows and columns

  hrMeshInstance(scnRef, teapotRef, &matrixT[0][0]);

  mat4x4_identity(mRot1);
  mat4x4_rotate_Y(mRot1, mRot1, 180.0f*DEG_TO_RAD);
  //mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD);
  mRot1[3][3] = 1.0f;
  mat4x4_transpose(matrixT, mRot1);
  hrMeshInstance(scnRef, cubeOpenRef, &matrixT[0][0]);

  /////////////////////////////////////////////////////////////////////// instance light (!!!)

  mat4x4_identity(mTranslate);
  mat4x4_translate(mTranslate, 1.0f, 3.25f, -1.0f);
  mat4x4_transpose(matrixT, mTranslate);
  hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef);

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_48/z_out.png");

  return check_images("test_48");
}



bool test49_light_geom_disk()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_49");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_49", HR_WRITE_DISCARD);

  SimpleMesh cube = CreateCube(0.75f);
  SimpleMesh plane = CreatePlane(10.0f);
  SimpleMesh sphere = CreateSphere(1.0f, 32);
  SimpleMesh torus = CreateTorus(0.25f, 0.6f, 32, 32);
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

  HRMeshRef teapotRef = hrMeshCreateFromFileDL(L"data/meshes/teapot.vsgf"); // chunk_00009.vsgf // teapot.vsgf // chunk_00591.vsgf

  HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
  HRMeshRef planeRef = hrMeshCreate(L"my_plane");
  HRMeshRef sphereRef = hrMeshCreate(L"my_sphere");
  HRMeshRef torusRef = hrMeshCreate(L"my_torus");

  // hrMeshOpen(cubeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  // {
  //   hrMeshVertexAttribPointer4f(cubeRef, L"pos", &cube.vPos[0]);
  //   hrMeshVertexAttribPointer4f(cubeRef, L"norm", &cube.vNorm[0]);
  //   hrMeshVertexAttribPointer2f(cubeRef, L"texcoord", &cube.vTexCoord[0]);
  // 
  //   int cubeMatIndices[12] = { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 2, 2 };
  // 
  //   //hrMeshMaterialId(cubeRef, 0);
  //   hrMeshPrimitiveAttribPointer1i(cubeRef, L"mind", cubeMatIndices);
  //   hrMeshAppendTriangles3(cubeRef, int(cube.triIndices.size()), &cube.triIndices[0]);
  // }
  // hrMeshClose(cubeRef);

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
    lightNode.attribute(L"shape").set_value(L"disk");
    lightNode.attribute(L"distribution").set_value(L"diffuse");

    lightNode.append_child(L"size").append_attribute(L"radius").set_value(1.0f);

    pugi::xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"10 10 10");
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

  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

  float matrixT[4][4];
  float mRot1[4][4], mRot2[4][4], mTranslate[4][4], mRes[4][4];
  //float mTranslateDown[4][4];

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  int mmIndex = 0;
  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, -0.70f*3.65f, -5.0f + 5.0f);
  mat4x4_scale(mRot1, mRot1, 3.65f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mRes[3][3] = 1.0f;
  mat4x4_transpose(matrixT, mRes); // this fucking math library swap rows and columns

  hrMeshInstance(scnRef, teapotRef, &matrixT[0][0]);

  mat4x4_identity(mRot1);
  mat4x4_rotate_Y(mRot1, mRot1, 180.0f*DEG_TO_RAD);
  //mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD);
  mRot1[3][3] = 1.0f;
  mat4x4_transpose(matrixT, mRot1);
  hrMeshInstance(scnRef, cubeOpenRef, &matrixT[0][0]);

  /////////////////////////////////////////////////////////////////////// instance light (!!!)

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRot1);
  mat4x4_identity(mRot2);

  mat4x4_translate(mTranslate, 1.0f, 3.5f, -1.0f);
  mat4x4_rotate_Y(mRot1, mRot1, -30.0f*DEG_TO_RAD);
  mat4x4_rotate_X(mRot2, mRot2, -30.0f*DEG_TO_RAD);
  mRot1[3][3] = 1.0f;
  mRot2[3][3] = 1.0f;

  mat4x4_mul(mRot1, mRot1, mRot2);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mRes[3][3] = 1.0f;

  mat4x4_transpose(matrixT, mRes);
  hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef);

  glViewport(0, 0, 1024, 768);
  std::vector<int32_t> image(1024 * 768);
  
  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  
    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);
  
    if (info.haveUpdateFB)
    {
      hrRenderGetFrameBufferLDR1i(renderRef, 1024, 768, &image[0]);
  
      glDisable(GL_TEXTURE_2D);
      glDrawPixels(1024, 768, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
  
      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);
  
      glfwSwapBuffers(g_window);
      glfwPollEvents();
    }
  
    if (info.finalUpdate)
      break;
  }

	hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_49/z_out.png");
  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_49/z_out.bmp");
	//hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_49/z_out.jpg");

  return check_images("test_49", 1, 25.0f);
}

void run_test_50()
{
	hrInit(L"OLOLO");
	hrSceneLibraryOpen(L"tests/test_50", HR_WRITE_DISCARD);

	// material definition
	//
	HRMaterialRef mat = hrMaterialCreate(L"MyRedMaterialName");   // create new HydraMaterial
	HRMaterialRef mat2 = hrMaterialCreate(L"MyGreenMaterialName"); // create new material

	hrMaterialOpen(mat, HR_WRITE_DISCARD);
	{
		xml_node matNode = hrMaterialParamNode(mat);

		xml_node diff = matNode.append_child(L"diffuse");
		xml_node refl = matNode.append_child(L"reflectivity");

		diff.append_attribute(L"brdf_type").set_value(L"lambert");
		diff.append_child(L"color").text().set(L"0.352941 0.0196078 0.0196078");
		diff.append_child(L"roughness").text().set(L"0");

		refl.append_attribute(L"brdf_type").set_value(L"phong");
		refl.append_child(L"color").text().set(L"0 0 0 ");
		refl.append_child(L"resnel_IOR").text().set(L"1.5");
		refl.append_child(L"glosiness").text().set(L"0.75");
	}
	hrMaterialClose(mat); // HRCheckErrorsMacro; ... 


	hrMaterialOpen(mat2, HR_WRITE_DISCARD);
	{
		xml_node matNode = hrMaterialParamNode(mat2);

		xml_node diff = matNode.append_child(L"diffuse");

		diff.append_attribute(L"brdf_type").set_value(L"lambert");
		diff.append_child(L"color").text().set(L"0.952941 0.0196078 0.0196078");
		diff.append_child(L"roughness").text().set(L"0");
	}
	hrMaterialClose(mat2);

	hrFlush();

	// make changes
	//
	hrMaterialOpen(mat, HR_WRITE_DISCARD);
	hrMaterialClose(mat);

	hrMaterialOpen(mat2, HR_OPEN_EXISTING);
	{
		xml_node matNode = hrMaterialParamNode(mat2);

		xml_node diff = matNode.child(L"diffuse");

		diff.attribute(L"brdf_type").set_value(L"oren_nayar");
		diff.child(L"roughness").text().set(L"1");
	}
	hrMaterialClose(mat2);

	hrFlush();
}

bool check_lib_tag_number_in_xml(const wchar_t* a_fileName)
{
	pugi::xml_document doc;
	doc.load_file(a_fileName);

	int numMatLib = 0;
	int numTextLib = 0;
	pugi::xml_node libLights = doc.first_child();

	for (auto node = libLights; node != nullptr; node = node.next_sibling())
	{
		if (std::wstring(node.name()) == L"materials_lib")
			numMatLib++;

		if (std::wstring(node.name()) == L"textures_lib")
			numTextLib++;
	}

	return (numMatLib == 1) && (numTextLib == 1);
}


bool test50_open_library_several_times()
{
	for (int i = 0; i < 3; i++)
		run_test_50();
	
  return check_lib_tag_number_in_xml(L"tests/test_50/statex_00002.xml");
}





void test_console_render(const wchar_t* a_libPath, const wchar_t* a_savePath)
{
	initGLIfNeeded();

	hrSceneLibraryOpen(a_libPath, HR_OPEN_EXISTING);

	/////////////////////////////////////////////////////////
	HRRenderRef renderRef;
	renderRef.id = 0;

	HRSceneInstRef scnRef;
	scnRef.id = 0;
	/////////////////////////////////////////////////////////

	auto pList = hrRenderGetDeviceList(renderRef);

	while (pList != nullptr)
	{
		std::wcout << L"device id = " << pList->id << L", name = " << pList->name << L", driver = " << pList->driver << std::endl;
		pList = pList->next;
	}

	hrRenderEnableDevice(renderRef, 0, true);

	hrCommit(scnRef, renderRef);


	glViewport(0, 0, 1024, 768);
	std::vector<int32_t> image(1024 * 768);

	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

		if (info.haveUpdateFB)
		{
			hrRenderGetFrameBufferLDR1i(renderRef, 1024, 768, &image[0]);

			glDisable(GL_TEXTURE_2D);
			glDrawPixels(1024, 768, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

			auto pres = std::cout.precision(2);
			std::cout << "rendering progress = " << info.progress << "% \r";
			std::cout.precision(pres);

			glfwSwapBuffers(g_window);
			glfwPollEvents();
		}

		if (info.finalUpdate)
			break;
	}

	hrRenderSaveFrameBufferLDR(renderRef, a_savePath);

}