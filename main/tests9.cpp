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
#include "simplerandom.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <fstream>

#include "../hydra_api/HR_HDRImageTool.h"
#include "../hydra_api/HydraXMLHelpers.h"

#pragma warning(disable:4996)
using namespace TEST_UTILS;


SimpleMesh CreateTestMeshForSplit(float a_size)
{
  uint32_t numberVertices = 9;
  uint32_t numberIndices  = 9;

  float cubeVertices[] =
  {
    -1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, +1.0f, +1.0f,
    +1.0f, -1.0f, +1.0f, +1.0f,

    +1.0f, -1.0f, -1.0f, +1.0f,
    +1.0f, -0.8f, -1.0f, +1.0f,
    +2.0f, +0.5f, +0.5f, +1.0f,

    -3.0f, +1.0f, -0.5f, +1.0f,
    -3.0f, -1.0f, +1.0f, +1.0f,
    -2.5f, -1.0f, +1.0f, +1.0f,
  };


  uint32_t cubeIndices[] =
  {
    0, 1, 2,
    3, 4, 5,
    6, 7, 8,
    9, 10,11,
    12,13,14,
    15,16,17
  };

  SimpleMesh cube;

  cube.vPos.resize(numberVertices*4);
  cube.vNorm.resize(numberVertices*4);
  cube.vTexCoord.resize(numberVertices*2);
  cube.triIndices.resize(numberIndices);
  cube.matIndices.resize(numberIndices/3);

  for (size_t i = 0; i < cube.vPos.size(); i++)
    cube.vPos[i] = cubeVertices[i] * a_size;

  for (size_t i = 0; i < cube.triIndices.size(); i++)
    cube.triIndices[i] = cubeIndices[i];

  for (size_t i = 0; i < cube.matIndices.size(); i++)
    cube.matIndices[i] = 0;

  return cube;
}



bool test98_motion_blur()
{
  hrErrorCallerPlace(L"test_98");
  //hrSceneLibraryOpen(L"/home/frol/PROG/HydraNLM/data/scenelib_anim/statex_00001.xml", HR_OPEN_EXISTING);
  hrSceneLibraryOpen(L"/home/frol/PROG/HydraNLM/data/scenelib_anim", HR_OPEN_EXISTING);
  //hrSceneLibraryOpen(L"D:/PROG/HydraNLM/data/scenelib_anim", HR_OPEN_EXISTING);
  
  /////////////////////////////////////////////////////////
  HRRenderRef renderRef;
  renderRef.id = 0;
  
  HRSceneInstRef scnRef;
  scnRef.id = 0;
  /////////////////////////////////////////////////////////
  
  hrRenderEnableDevice(renderRef, 0, true);
  //hrRenderLogDir(renderRef, L"/home/frol/hydra/", true);
  //hrRenderLogDir(renderRef, L"C:/[Hydra]/logs/", true);
  
  const int samplesPerSubFrame = 32;
  
  hrRenderOpen(renderRef, HR_OPEN_EXISTING);
  {
    auto node = hrRenderParamNode(renderRef);
    node.force_child(L"maxRaysPerPixel").text() = samplesPerSubFrame*2;
    node.force_child(L"dont_run").text()        = 1;
  }
  hrRenderClose(renderRef);
  
  hrCommit(scnRef, renderRef);
  
  // tell render to render subframe with exact samples number
  //
  {
    std::wstringstream strOut;
    strOut << L"runhydra -cl_device_id 0 -contribsamples 32 -maxsamples 64 -statefile statex_00001.xml "; // << L"-contribsamples " << samplesPerSubFrame;
    auto str = strOut.str();
    hrRenderCommand(renderRef, str.c_str());
  }

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);
    
    if (info.haveUpdateFB)
    {
      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
      std::cout.flush();
      std::cout.precision(pres);
    }
    
    if (info.progress >= 49.0f)
    {
      hrRenderCommand(renderRef, L"exitnow");
      break;
    }
  }
  
  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_98/z_out.png");
  
  //std::cout << "before sleep" << std::endl;
  //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  //std::cout << "after sleep" << std::endl;
  
  // tell render to render subframe with exact samples number
  //
  {
    std::wstringstream strOut;
    strOut << L"runhydra -cl_device_id 0 -contribsamples 32 -maxsamples 64 -statefile statex_00009.xml "; // << L"-contribsamples " << samplesPerSubFrame;
    auto str = strOut.str();
    hrRenderCommand(renderRef, str.c_str());
  }

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);
    
    if (info.haveUpdateFB)
    {
      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
      std::cout.flush();
      std::cout.precision(pres);
    }
    
    if (info.finalUpdate)
      break;
  }
  
  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_98/z_out2.png");
  
  return false;
  //return check_images("test_98", 2, 50);
}

bool test55_clear_scene()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test55");

  HRCameraRef    camRef;
  HRSceneInstRef scnRef;
  HRRenderRef    settingsRef;

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_55", HR_WRITE_DISCARD);

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
    camNode.append_child(L"position").text().set(L"0 2 10");
    camNode.append_child(L"look_at").text().set(L"0 0 -1");
  }
  hrCameraClose(camRef);

  // set up render settings
  //
  settingsRef = hrRenderCreate(L"opengl1");

  hrRenderOpen(settingsRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(settingsRef);

    node.append_child(L"width").text() = 512;
    node.append_child(L"height").text() = 512;
  }
  hrRenderClose(settingsRef);

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

  auto rgen = simplerandom::RandomGenInit(779);

  for (int i = -2; i <= 2; i++)
  {
    for (int j = -2; j <= 2; j++)
    {
      float xCoord = 2.0f*float(i);
      float yCoord = 2.0f*float(j);
      
      mat4x4_identity(mRot1);
      mat4x4_identity(mTranslate);
      mat4x4_identity(mRes);

      mat4x4_translate(mTranslate, xCoord, 0.5f, yCoord);
      mat4x4_rotate_X(mRot1, mRot1, rnd(0.0f, 360.0f)*DEG_TO_RAD);
      mat4x4_rotate_Y(mRot1, mRot1, rnd(0.0f, 360.0f)*DEG_TO_RAD*0.5f);
      mat4x4_mul(mRes, mTranslate, mRot1);
      mat4x4_transpose(matrixT, mRes); // this fucking math library swap rows and columns

      hrMeshInstance(scnRef, refs[simplerandom::rand(rgen) % 3], &matrixT[0][0]);
    }
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, settingsRef);

  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_55/z_out.png");


  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  hrMeshInstance(scnRef, planeRef, &matrixT2[0][0]);

  for (int i = -2; i <= 2; i++)
  {
    for (int j = -2; j <= 2; j++)
    {
      float xCoord = 2.0f*float(i);
      float yCoord = 2.0f*float(j);
      
      mat4x4_identity(mRot1);
      mat4x4_identity(mTranslate);
      mat4x4_identity(mRes);

      mat4x4_translate(mTranslate, xCoord, 0.5f, yCoord);
      mat4x4_rotate_X(mRot1, mRot1, rnd(0.0f, 360.0f)*DEG_TO_RAD);
      mat4x4_rotate_Y(mRot1, mRot1, rnd(0.0f, 360.0f)*DEG_TO_RAD*0.5f);
      mat4x4_mul(mRes, mTranslate, mRot1);
      mat4x4_transpose(matrixT, mRes); // this fucking math library swap rows and columns

      hrMeshInstance(scnRef, refs[simplerandom::rand(rgen) % 3], &matrixT[0][0]);
    }
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, settingsRef);

  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_55/z_out2.png");

  return check_images("test_55", 2);
}



bool test56_mesh_change_open_existing()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_56");

  HRCameraRef    camRef;
  HRSceneInstRef scnRef;
  HRRenderRef    settingsRef;

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_56", HR_WRITE_DISCARD);

  // material and textures
  //
  HRTextureNodeRef testTex2 = hrTexture2DCreateFromFile(L"data/textures/chess_red.bmp");

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  std::wstring rotationMatrixStr = L"0.707107 -0.707107 0 0.5 0.707107 0.707107 0 -0.207107 0 0 1 0 0 0 0 1";

  HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
  HRMaterialRef mat1 = hrMaterialCreate(L"mysimplemat2");
  HRMaterialRef mat2 = hrMaterialCreate(L"mysimplemat3");
  HRMaterialRef mat3 = hrMaterialCreate(L"mysimplemat4");
  HRMaterialRef mat4 = hrMaterialCreate(L"mysimplemat5");
  HRMaterialRef mat5 = hrMaterialCreate(L"mysimplemat6");

  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat0);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"0.75 0.75 0.25");

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

  hrMaterialOpen(mat4, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat4);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"0.75 0.75 0.0");
  }
  hrMaterialClose(mat4);

  hrMaterialOpen(mat5, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat5);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"0.0 0.0 0.85");
  }
  hrMaterialClose(mat5);

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  SimpleMesh cube   = CreateCube(0.75f);
  SimpleMesh cube2  = CreateCube(0.25f);
  SimpleMesh plane  = CreatePlane(2.0f);
  SimpleMesh sphere = CreateSphere(0.5f, 32);
  SimpleMesh torus  = CreateTorus(0.2f, 0.5f, 32, 32);

  HRMeshRef cubeRef   = hrMeshCreate(L"my_cube");
  HRMeshRef planeRef  = hrMeshCreate(L"my_plane");
  HRMeshRef sphereRef = hrMeshCreate(L"my_sphere");
  HRMeshRef torusRef  = hrMeshCreate(L"my_torus");

  hrMeshOpen(cubeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeRef, L"pos", &cube.vPos[0]);
    hrMeshVertexAttribPointer4f(cubeRef, L"norm", &cube.vNorm[0]);
    hrMeshVertexAttribPointer2f(cubeRef, L"texcoord", &cube.vTexCoord[0]);

    int cubeMatIndices[12] = { 0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 1, 1 };

    //hrMeshMaterialId(cubeRef, 0);
    hrMeshPrimitiveAttribPointer1i(cubeRef, L"mind", cubeMatIndices);
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

    // test second call of hrMeshAppendTriangles3
    //
    hrMeshVertexAttribPointer4f(torusRef, L"pos",      &cube2.vPos[0]);
    hrMeshVertexAttribPointer4f(torusRef, L"norm",     &cube2.vNorm[0]);
    hrMeshVertexAttribPointer2f(torusRef, L"texcoord", &cube2.vTexCoord[0]);

    int cubeMatIndices[12] = { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 1, 1 };

    hrMeshPrimitiveAttribPointer1i(torusRef, L"mind", cubeMatIndices);
    hrMeshAppendTriangles3(torusRef, int(cube2.triIndices.size()), &cube2.triIndices[0]);
  }
  hrMeshClose(torusRef);

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
    camNode.append_child(L"position").text().set(L"0 0 0");
    camNode.append_child(L"look_at").text().set(L"0 0 -1");
  }
  hrCameraClose(camRef);

  // set up render settings
  //
  settingsRef = hrRenderCreate(L"opengl1");

  hrRenderOpen(settingsRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(settingsRef);
    node.append_child(L"width").text()  = 512;
    node.append_child(L"height").text() = 512;
  }
  hrRenderClose(settingsRef);

  // create scene
  //
  scnRef = hrSceneCreate(L"my scene");

  float	rtri = 25.0f; // Angle For The Triangle ( NEW )
  float	rquad = 40.0f;
  float g_FPS = 60.0f;
  int   frameCounter = 0;

  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

  float matrixT[4][4], matrixT2[4][4], matrixT3[4][4], matrixT4[4][4];
  float mRot1[4][4], mTranslate[4][4], mRes[4][4];

  float mTranslateDown[4][4], mRes2[4][4];

  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, 0.25f, -5.0f);
  mat4x4_rotate_X(mRot1, mRot1, -3.5f*rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, -7.0f*rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT, mRes); // this fucking math library swap rows and columns

  mat4x4_identity(mRes);
  mat4x4_translate(mTranslateDown, 0.0f, -1.0f, -5.0f);
  mat4x4_mul(mRes2, mTranslateDown, mRes);
  mat4x4_transpose(matrixT2, mRes2);

  mat4x4_identity(mRot1);
  mat4x4_identity(mRes);
  mat4x4_rotate_Y(mRes, mRot1, rquad*DEG_TO_RAD);
  mat4x4_translate(mTranslateDown, -1.5f, -0.5f, -4.0f);
  mat4x4_mul(mRes2, mTranslateDown, mRes);
  mat4x4_transpose(matrixT3, mRes2);

  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 2.0f, 0.25f, -5.0f);
  mat4x4_rotate_X(mRot1, mRot1, rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, -rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT4, mRes); // this fucking math library swap rows and columns

  // draw scene
  //
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    hrMeshInstance(scnRef, cubeRef,   &matrixT[0][0]);
    hrMeshInstance(scnRef, planeRef,  &matrixT2[0][0]);
    hrMeshInstance(scnRef, sphereRef, &matrixT3[0][0]);
    hrMeshInstance(scnRef, torusRef,  &matrixT4[0][0]);
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, settingsRef);
  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_56/z_out.png");

  hrMeshOpen(torusRef, HR_TRIANGLE_IND3, HR_OPEN_EXISTING);
  {
    auto meshInfo = hrMeshGetInfo(torusRef);

    const int triNum = meshInfo.indicesNum / 3;

    int32_t* matindices = (int32_t*)hrMeshGetPrimitiveAttribPointer(torusRef, L"mind");
    //for(int i=0;i<triNum;i++)
      //matindices[i] = mat5.id;

    matindices[triNum - 2] = mat5.id;
    matindices[triNum - 1] = mat5.id;
  }
  hrMeshClose(torusRef);
  
  
  hrFlush(scnRef, settingsRef);
  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_56/z_out2.png");

  hrMeshOpen(sphereRef, HR_TRIANGLE_IND3, HR_OPEN_EXISTING);
  {
    auto meshInfo = hrMeshGetInfo(sphereRef);

    const int vertNum = meshInfo.vertNum;
    const int triNum  = meshInfo.indicesNum / 3;

    int32_t* matindices = (int32_t*)hrMeshGetPrimitiveAttribPointer(sphereRef, L"mind");
    
    HydraLiteMath::float4* vpos = (HydraLiteMath::float4*)hrMeshGetAttribPointer(sphereRef, L"pos");

    HydraLiteMath::float4 center(0, 0, 0, 0);
    for (size_t i = 0; i < vertNum; i++)
      center += vpos[i];
    center = center*(1.0f / float(vertNum));

    auto center3 = to_float3(center);
    for (size_t i = 0; i < vertNum; i++)
    {

      if (i % 8 == 0)
      {
        float r  = length(to_float3(vpos[i]) - center3);
        auto dir1 = normalize(to_float3(vpos[i + 0]) - center3);
        auto dir2 = normalize(to_float3(vpos[i + 1]) - center3);

        vpos[i] = HydraLiteMath::to_float4(center3 + 1.25f*r*dir1, 1.0f);
        vpos[i] = HydraLiteMath::to_float4(center3 + 1.25f*r*dir2, 1.0f);
      }
    }

    for (int i = 0; i < triNum; i++)
    {
      if (i % 7 == 0)
        matindices[i] = mat5.id;
    }

  }
  hrMeshClose(sphereRef);
  
  hrFlush(scnRef, settingsRef);
  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_56/z_out3.png");

  return check_images("test_56", 3, 20.0f);
}

extern GLFWwindow* g_window;

bool test57_single_instance()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_57");
  hrSceneLibraryOpen(L"tests/test_57", HR_WRITE_DISCARD);

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

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 2048);


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
  mRot2.identity();

  mTranslate = translate4x4(float3(0.0f, 1.0f, 2.5f));
  mRot       = rotate_Y_4x4(-60.0f*DEG_TO_RAD);
  mRot2      = rotate_X_4x4(90.0f*DEG_TO_RAD);
  mRes       = mul(mRot, mRot2);
  mRes       = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, torusB, mRes.L());

  ///////////

  mRes.identity();

  hrLightInstance(scnRef, sky, mRes.L());

  ///////////

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
      std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
      std::cout.precision(pres);

      glfwSwapBuffers(g_window);
      glfwPollEvents();
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_57/z_out.png");

  return check_images("test_57", 1, 60);
}


bool test58_crysponza_and_opacity1_perf()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_58");
  hrSceneLibraryOpen(L"tests/test_58", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");

  for (int i = 0; i < 100; i++)
  {
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

    }
    hrMaterialClose(matGray);
  }

  HRMaterialRef mat0;
  mat0.id = 0;

  HRMaterialRef mat99;
  mat99.id = 99;

  hrMaterialOpen(mat99, HR_OPEN_EXISTING);
  {
    auto matNode = hrMaterialParamNode(mat99);

    matNode.force_child(L"diffuse").force_child(L"color").force_attribute(L"val").set_value(L"0.0 0.65 0.0");

    auto opacity = matNode.force_child(L"opacity");
    auto texNode = hrTextureBind(texChecker, opacity);

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
  }
  hrMaterialClose(mat99);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR    = HRMeshFromSimpleMesh(L"cubeR",    CreateCube(2.0f), mat99.id);
  HRMeshRef pillar   = HRMeshFromSimpleMesh(L"pillar",   CreateCube(1.0f), mat0.id);
  HRMeshRef sphereG  = HRMeshFromSimpleMesh(L"sphereG",  CreateSphere(4.0f, 64), mat0.id);
  HRMeshRef torusB   = HRMeshFromSimpleMesh(L"torusB",   CreateTorus(0.8f, 2.0f, 64, 64), mat0.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(10000.0f), mat0.id);

  HRMeshRef sponzaRef= hrMeshCreateFromFileDL(L"data/meshes/crysponza.vsgf");

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Light
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRLightRef sun = hrLightCreate(L"sun");

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

    camNode.append_child(L"fov").text().set(L"60");
    camNode.append_child(L"nearClipPlane").text().set(L"0.01");
    camNode.append_child(L"farClipPlane").text().set(L"1000.0");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"1 30 46");
    camNode.append_child(L"look_at").text().set(L"-15 16 0");
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

  float4x4 mRot, mRot2;
  float4x4 mTranslate;
  float4x4 mScale;
  float4x4 mRes;

  const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  ///////////

  mTranslate = translate4x4(float3(0, 0, 0));
  mScale     = scale4x4(float3(5.0f, 5.0f, 5.0f));
  mRot       = rotate_Y_4x4(90.0f*DEG_TO_RAD);
  mRes       = mul(mTranslate, mul(mRot, mScale));

  hrMeshInstance(scnRef, sponzaRef, mRes.L());

  srand(555);
  for (int i = 0; i < 100; i++)
  {
    mTranslate = translate4x4(float3(HydraLiteMath::rnd(-20.0f, 20.0f), HydraLiteMath::rnd(0.0f, 40.0f), HydraLiteMath::rnd(-50.0f, 50.0f)));
    mRes       = mTranslate;

    hrMeshInstance(scnRef, cubeR, mRes.L());
  }

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();
  mRot2.identity();

  mTranslate = translate4x4(float3(200.0f, 200.0f, -100.0f));
  mRot       = rotate_X_4x4(10.0f*DEG_TO_RAD);
  mRot2      = rotate_Z_4x4(10.f*DEG_TO_RAD);
  mRes       = mul(mRot2, mRot);
  mRes       = mul(mTranslate, mRes);

  hrLightInstance(scnRef, sun, mRes.L());

  ///////////

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef, camRef);

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
      std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
      std::cout.precision(pres);

      glfwSwapBuffers(g_window);
      glfwPollEvents();
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_53/z_out.png");

  return false;
}

extern bool g_testWasIgnored;

bool test59_cornell_water_mlt()
{
  g_testWasIgnored = true; // this test is disable untill we got MMLT working.
  return false;

  initGLIfNeeded();

  hrErrorCallerPlace(L"test_59");

  hrSceneLibraryOpen(L"tests/test_59", HR_WRITE_DISCARD);


  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  for (int i = 0; i < 33; i++)
  {
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

    }
    hrMaterialClose(matGray);
  }

  HRMaterialRef mat0, mat2, mat3, mat4, mat5;
  mat0.id = 0;
  mat2.id = 2;
  mat3.id = 3;
  mat4.id = 4;
  mat5.id = 5;

  hrMaterialOpen(mat2, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(mat2);
    auto diff    = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.352941 0.0196078 0.0196078");
  }
  hrMaterialClose(mat2);

  hrMaterialOpen(mat3, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(mat3);
    auto diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0196078 0.352941 0.0196078");
  }
  hrMaterialClose(mat3);

  hrMaterialOpen(mat4, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(mat4);
   
    auto refl = matNode.append_child(L"reflectivity");

    refl.append_attribute(L"brdf_type").set_value(L"phong");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(3.0f);

    auto transp = matNode.append_child(L"transparency");

    transp.append_attribute(L"brdf_type").set_value(L"phong");
    transp.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
    transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0);
    transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
    transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
    transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
    transp.append_child(L"ior").append_attribute(L"val").set_value(1.5f);
  }
  hrMaterialClose(mat4);

  hrMaterialOpen(mat5, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(mat5);
    auto diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.207843 0.188235 0");

    auto refl = matNode.append_child(L"reflectivity");

    refl.append_attribute(L"brdf_type").set_value(L"phong");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.367059 0.345882 0");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.75");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(2.5f);
  }
  hrMaterialClose(mat5);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR    = HRMeshFromSimpleMesh(L"cubeR",    CreateCube(2.0f), mat0.id);
  HRMeshRef pillar   = HRMeshFromSimpleMesh(L"pillar",   CreateCube(1.0f), mat0.id);
  HRMeshRef sphereG  = HRMeshFromSimpleMesh(L"sphereG",  CreateSphere(4.0f, 64), mat0.id);
  HRMeshRef torusB   = HRMeshFromSimpleMesh(L"torusB",   CreateTorus(0.8f, 2.0f, 64, 64), mat0.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(10000.0f), mat0.id);

  HRMeshRef treeRef  = hrMeshCreateFromFileDL(L"data/meshes/cornell_water.vsgf");

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Light
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRLightRef sun = hrLightCreate(L"sun");

  hrLightOpen(sun, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sun);

    lightNode.attribute(L"type").set_value(L"area");
    lightNode.attribute(L"shape").set_value(L"rect");
    lightNode.attribute(L"distribution").set_value(L"diffuse");

    auto sizeNode = lightNode.append_child(L"size");
    sizeNode.append_attribute(L"half_length").set_value(L"0.0635f");
    sizeNode.append_attribute(L"half_width").set_value(L"0.0635f");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"250.0");

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
    camNode.append_child(L"farClipPlane").text().set(L"1000.0");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 0.125 -4");
    camNode.append_child(L"look_at").text().set(L"0 0.0 0");
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
    node.append_child(L"method_secondary").text() = L"mlt";
    node.append_child(L"method_tertiary").text()  = L"mlt";
    node.append_child(L"method_caustic").text()   = L"mlt";
    node.append_child(L"shadows").text()          = L"1";

    node.append_child(L"trace_depth").text()      = L"5";
    node.append_child(L"diff_trace_depth").text() = L"3";

    node.append_child(L"pt_error").text() = L"2";
    node.append_child(L"minRaysPerPixel").text() = 256;
    node.append_child(L"maxRaysPerPixel").text() = 2048;
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

  mTranslate = translate4x4(float3(0,0,0));
  mScale     = scale4x4(float3(1.0f, 1.0f, 1.0f));
  mRot       = rotate_Y_4x4(180.0f*DEG_TO_RAD);
  mRes       = mul(mTranslate, mul(mRot, mScale));

  hrMeshInstance(scnRef, treeRef, mRes.L());

  mTranslate = translate4x4(float3(0.0f, 1.0f, 0.0f));
  hrLightInstance(scnRef, sun, mTranslate.L());

  ///////////

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef, camRef);

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
      std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
      std::cout.precision(pres);

      glfwSwapBuffers(g_window);
      glfwPollEvents();
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_59/z_out.png");

  return check_images("test_59", 1, 50.0f);
}

bool test60_debug_print()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_60");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_60", HR_WRITE_DISCARD);

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
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.75 0.5");

    HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp"); // hrTexture2DCreateFromFileDL
    hrTextureBind(testTex, diff.child(L"color"));
  }
  hrMaterialClose(mat0);

  hrMaterialOpen(mat1, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat1);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");

    hrTextureBind(testTex2, diff.child(L"color"));
  }
  hrMaterialClose(mat1);

  hrMaterialOpen(mat2, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat2);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.75 0.75");

    HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/relief_wood.jpg");
    hrTextureBind(testTex, diff.child(L"color"));
  }
  hrMaterialClose(mat2);

  hrMaterialOpen(mat3, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat3);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.75 0.75");

    HRTextureNodeRef testTex = hrTexture2DCreateFromFileDL(L"data/textures/163.jpg");
    hrTextureBind(testTex, diff.child(L"color"));
  }
  hrMaterialClose(mat3);

  hrMaterialOpen(mat4, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat4);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.1 0.1 0.75");
  }
  hrMaterialClose(mat4);

  hrMaterialOpen(mat5, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat5);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.75 0.25");

    HRTextureNodeRef testTex = hrTexture2DCreateFromFileDL(L"data/textures/texture1.bmp");
    hrTextureBind(testTex, diff.child(L"color"));
  }
  hrMaterialClose(mat5);


  hrMaterialOpen(mat6, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat6);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.0 0.0");
  }
  hrMaterialClose(mat6);

  hrMaterialOpen(mat7, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat7);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.5 0.0");
  }
  hrMaterialClose(mat7);

  hrMaterialOpen(mat8, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat8);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(mat8);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMeshRef cubeRef     = hrMeshCreate(L"my_cube");
  HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
  HRMeshRef planeRef    = hrMeshCreate(L"my_plane");
  HRMeshRef sphereRef   = hrMeshCreate(L"my_sphere");
  HRMeshRef torusRef    = hrMeshCreate(L"my_torus");

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

    sizeNode.append_attribute(L"half_length") = 1.0f;
    sizeNode.append_attribute(L"half_width")  = 1.0f;

    pugi::xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").text().set(L"1 1 1");
    intensityNode.append_child(L"multiplier").text().set(L"8.0");

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
  HRRenderRef renderRef = hrRenderCreate(L"DebugPrint"); // opengl1
  
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

    node.append_child(L"width").text()            = L"1024";
    node.append_child(L"height").text()           = L"768";

    node.append_child(L"method_primary").text()   = L"pathtracing";
    node.append_child(L"method_secondary").text() = L"pathtracing";
    node.append_child(L"method_tertiary").text()  = L"pathtracing";
    node.append_child(L"method_caustic").text()   = L"pathtracing";
    node.append_child(L"shadows").text()          = L"1";
    
    node.append_child(L"trace_depth").text()      = L"5";
    node.append_child(L"diff_trace_depth").text() = L"3";
    
    node.append_child(L"pt_error").text()         = L"2";
    node.append_child(L"minRaysPerPixel").text()  = L"256";
    node.append_child(L"maxRaysPerPixel").text()  = L"1024";

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
  mat4x4_translate(mTranslate, 0, 3.85f, 0);
  mat4x4_transpose(matrixT, mTranslate);
  hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef);

  // glViewport(0, 0, 1024, 768);
  // std::vector<int32_t> image(1024 * 768);
  // 
  // while(true)
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
  //     std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
  //     std::cout.precision(pres);
  // 
  //     glfwSwapBuffers(g_window);
  //     glfwPollEvents();
  //   }
  // 
  //   if (info.finalUpdate)
  //     break;
  // }
  // 
  // hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_60/z_out.png");
  // 
  // return check_images("test_60");

  return true;
}


bool test61_cornell_with_light_near_wall_and_glossy_wall()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_61");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_61", HR_WRITE_DISCARD);

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

  HRMaterialRef mat9 = hrMaterialCreate(L"glossy_wall_mat");

  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat0);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.75 0.5");

    HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp"); // hrTexture2DCreateFromFileDL
    hrTextureBind(testTex, diff);
  }
  hrMaterialClose(mat0);

  hrMaterialOpen(mat1, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat1);
    xml_node diff    = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.207843 0.188235 0");

    xml_node refl = matNode.append_child(L"reflectivity");

    refl.append_attribute(L"brdf_type").set_value(L"phong");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.367059 0.345882 0");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.5");
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

  hrMaterialOpen(mat9, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat9);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.0 0");

    xml_node refl = matNode.append_child(L"reflectivity");

    refl.append_attribute(L"brdf_type").set_value(L"phong");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.4 0.4 0.4");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.75");
  }
  hrMaterialClose(mat9);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMeshRef teapotRef   = hrMeshCreateFromFileDL(L"data/meshes/teapot.vsgf"); // chunk_00009.vsgf // teapot.vsgf // chunk_00591.vsgf

  HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
  HRMeshRef planeRef    = hrMeshCreate(L"my_plane");
  HRMeshRef sphereRef   = hrMeshCreate(L"my_sphere");
  HRMeshRef torusRef    = hrMeshCreate(L"my_torus");

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

    int cubeMatIndices[10] = { mat8.id, mat8.id, mat8.id, mat8.id, mat9.id, mat9.id, mat7.id, mat7.id, mat6.id, mat6.id };

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
    sizeNode.append_attribute(L"half_width").set_value(L"2.0");

    pugi::xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"2.0");
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
    node.append_child(L"maxRaysPerPixel").text() = L"2048";
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
  mat4x4_translate(mTranslate, -3.0f, 3.85f, -2.0);
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
      std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
      std::cout.precision(pres);
  
      glfwSwapBuffers(g_window);
      glfwPollEvents();
    }
  
    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_61/z_out.png");

  return check_images("test_61", 1, 20.0f);
}






bool test63_cornell_with_caustic_from_torus()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_63");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_63", HR_WRITE_DISCARD);

  SimpleMesh cube     = CreateCube(0.75f);
  SimpleMesh plane    = CreatePlane(10.0f);
  SimpleMesh sphere   = CreateSphere(1.0f, 32);
  SimpleMesh torus    = CreateTorus(0.4f, 1.5f, 64, 64);
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

  HRMaterialRef mat9 = hrMaterialCreate(L"torus");

  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat0);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.75 0.5");

    HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp"); // hrTexture2DCreateFromFileDL
    hrTextureBind(testTex, diff.child(L"color"));
  }
  hrMaterialClose(mat0);

  hrMaterialOpen(mat1, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat1);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");

    hrTextureBind(testTex2, diff.child(L"color"));
  }
  hrMaterialClose(mat1);

  hrMaterialOpen(mat2, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat2);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.75 0.75");

    HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/relief_wood.jpg");
    hrTextureBind(testTex, diff.child(L"color"));
  }
  hrMaterialClose(mat2);

  hrMaterialOpen(mat3, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat3);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.75 0.75");

    HRTextureNodeRef testTex = hrTexture2DCreateFromFileDL(L"data/textures/163.jpg");
    hrTextureBind(testTex, diff.child(L"color"));
  }
  hrMaterialClose(mat3);

  hrMaterialOpen(mat4, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat4);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.1 0.1 0.75");
  }
  hrMaterialClose(mat4);

  hrMaterialOpen(mat5, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat5);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.75 0.25");

    HRTextureNodeRef testTex = hrTexture2DCreateFromFileDL(L"data/textures/texture1.bmp");
    hrTextureBind(testTex, diff.child(L"color"));
  }
  hrMaterialClose(mat5);


  hrMaterialOpen(mat6, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat6);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.0 0.0");
  }
  hrMaterialClose(mat6);

  hrMaterialOpen(mat7, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat7);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.5 0.0");
  }
  hrMaterialClose(mat7);

  hrMaterialOpen(mat8, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat8);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    
    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
    VERIFY_XML(matNode);
  }
  hrMaterialClose(mat9);

  hrMaterialOpen(mat9, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat9);
   
    // auto diff = matNode.append_child(L"diffuse");
    // diff.append_attribute(L"brdf_type").set_value(L"lambert");
    // 
    // auto color = diff.append_child(L"color");
    // color.append_attribute(L"val").set_value(L"0.1 0.1 0.1");
    // 
    // auto refl = matNode.append_child(L"reflectivity");
    // refl.append_attribute(L"brdf_type").set_value(L"phong");
    // refl.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");
    // refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.85");
    // refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    // refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    // refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(10.0f);

    auto refl = matNode.append_child(L"reflectivity");
    
    refl.append_attribute(L"brdf_type").set_value(L"phong");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.85 0.85 0.85");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(0);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(2.5f);
    
    // auto transp = matNode.append_child(L"transparency");
    // 
    // transp.append_attribute(L"brdf_type").set_value(L"phong");
    // transp.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
    // transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0);
    // transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
    // transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
    // transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
    // transp.append_child(L"ior").append_attribute(L"val").set_value(1.25f);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(mat9);

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

    hrMeshMaterialId(torusRef, mat9.id);
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

    sizeNode.append_attribute(L"half_length") = 0.5f;
    sizeNode.append_attribute(L"half_width")  = 0.5f;

    pugi::xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").text().set(L"1 1 1");
    intensityNode.append_child(L"multiplier").text().set(L"32.0");
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

  hrRenderEnableDevice(renderRef, 1, true);
  //hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);

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
    node.append_child(L"shadows").text() = L"1";

    node.append_child(L"trace_depth").text()      = L"10";
    node.append_child(L"diff_trace_depth").text() = L"3";

    node.append_child(L"pt_error").text()        = L"2";
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

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    float matrixT[4][4], matrixT4[4][4];
    float mRot1[4][4], mTranslate[4][4], mRes[4][4];

    int mmIndex = 0;
    mat4x4_identity(mRot1);
    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, 0.0f, 0.0f, -0.0f);
    mat4x4_rotate_X(mRot1, mRot1, 110.0f*DEG_TO_RAD);
    mat4x4_mul(mRes, mTranslate, mRot1);
    mat4x4_transpose(matrixT4, mRes); // this fucking math library swap rows and columns
    hrMeshInstance(scnRef, torusRef, &matrixT4[0][0]);

    mat4x4_identity(mRot1);
    mat4x4_rotate_Y(mRot1, mRot1, 180.0f*DEG_TO_RAD);
    mat4x4_transpose(matrixT, mRot1);
    hrMeshInstance(scnRef, cubeOpenRef, &matrixT[0][0]);

    /////////////////////////////////////////////////////////////////////// instance light (!!!)

    mat4x4_identity(mTranslate);
    mat4x4_translate(mTranslate, 0, 1.0f, 0);
    mat4x4_transpose(matrixT, mTranslate);
    hrLightInstance(scnRef, rectLight, &matrixT[0][0]);
  }
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
      std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
      std::cout.precision(pres);

      glfwSwapBuffers(g_window);
      glfwPollEvents();
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_63/z_out.png");

  return check_images("test_63");
}



bool test100_dummy_hydra_exec()
{
	initGLIfNeeded();

	hrErrorCallerPlace(L"test_100");

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	hrSceneLibraryOpen(L"tests/test_100", HR_WRITE_DISCARD);

	SimpleMesh sphere = CreateSphere(1.0f, 32);
	HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");

	hrMaterialOpen(mat0, HR_WRITE_DISCARD);
	{
		xml_node matNode = hrMaterialParamNode(mat0);

		xml_node diff = matNode.append_child(L"diffuse");

		diff.append_attribute(L"brdf_type").set_value(L"lambert");
		diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.75 0.5");
	}
	hrMaterialClose(mat0);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	HRMeshRef sphereRef = hrMeshCreate(L"my_sphere");


	hrMeshOpen(sphereRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
	{
		hrMeshVertexAttribPointer4f(sphereRef, L"pos", &sphere.vPos[0]);
		hrMeshVertexAttribPointer4f(sphereRef, L"norm", &sphere.vNorm[0]);
		hrMeshVertexAttribPointer2f(sphereRef, L"texcoord", &sphere.vTexCoord[0]);

		for (size_t i = 0; i < sphere.matIndices.size(); i++)
			sphere.matIndices[i] = mat0.id;

		hrMeshPrimitiveAttribPointer1i(sphereRef, L"mind", &sphere.matIndices[0]);
		hrMeshAppendTriangles3(sphereRef, int32_t(sphere.triIndices.size()), &sphere.triIndices[0]);
	}
	hrMeshClose(sphereRef);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	HRLightRef rectLight = hrLightCreate(L"my_area_light");

	hrLightOpen(rectLight, HR_WRITE_DISCARD);
	{
		pugi::xml_node lightNode = hrLightParamNode(rectLight);

		lightNode.attribute(L"type").set_value(L"area");
		lightNode.attribute(L"shape").set_value(L"rect");
		lightNode.attribute(L"distribution").set_value(L"diffuse");

		pugi::xml_node sizeNode = lightNode.append_child(L"size");

		sizeNode.append_attribute(L"half_length") = 0.5f;
		sizeNode.append_attribute(L"half_width") = 0.5f;

		pugi::xml_node intensityNode = lightNode.append_child(L"intensity");

		intensityNode.append_child(L"color").text().set(L"1 1 1");
		intensityNode.append_child(L"multiplier").text().set(L"32.0");
	}
	hrLightClose(rectLight);

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

		node.append_child(L"trace_depth").text() = L"10";
		node.append_child(L"diff_trace_depth").text() = L"3";

		node.append_child(L"pt_error").text() = L"2";
		node.append_child(L"minRaysPerPixel").text() = L"256";
		node.append_child(L"maxRaysPerPixel").text() = L"1024";
    node.append_child(L"render_executable").text() = L"/home/vsan/test";
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

	using namespace HydraLiteMath;

	hrSceneOpen(scnRef, HR_WRITE_DISCARD);
	{
		float4x4 mRot;
		float4x4 mTranslate;
		float4x4 mScale;
		float4x4 mRes;

		///////////
		mTranslate.identity();
		mRes.identity();

		mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
		mRes = mul(mTranslate, mRes);

		hrMeshInstance(scnRef, sphereRef, mRes.L());

		/////////////////////////////////////////////////////////////////////// instance light (!!!)
		mTranslate.identity();
		mRes.identity();

		mTranslate = translate4x4(float3(0.0f, 4.0f, 0.0f));
		mRes = mul(mTranslate, mRes);

		hrLightInstance(scnRef, rectLight, mRes.L());
	}
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
			std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
			std::cout.precision(pres);

			glfwSwapBuffers(g_window);
			glfwPollEvents();
		}
		if (info.finalUpdate)
			break;
	}

	hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_100/z_out.png");

	return check_images("test_100");
}