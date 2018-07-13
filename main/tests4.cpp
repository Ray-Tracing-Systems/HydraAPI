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

#pragma warning(disable:4996)
#pragma warning(disable:4838)

using namespace TEST_UTILS;


bool test23_texture_from_memory()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test23");

  HRCameraRef    camRef;
  HRSceneInstRef scnRef;
  HRRenderRef    settingsRef;

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_23", HR_WRITE_DISCARD);

  // geometry
  //
  HRMeshRef cubeRef  = HRMeshFromSimpleMesh(L"my_cube",   CreateCube(0.75f), 0);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane",  CreatePlane(2.0f), 1);
  HRMeshRef sphRef   = HRMeshFromSimpleMesh(L"my_sphere", CreateSphere(0.5f, 32), 2);
  HRMeshRef torRef   = HRMeshFromSimpleMesh(L"my_torus",  CreateTorus(0.2f, 0.5f, 32, 32), 0);

  // material and textures
  //
  unsigned int colors1[4] = { 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF, 0xFF000000 };
  unsigned int colors2[4] = { 0xFF00FF00, 0xFFFF0000, 0xFFFF00FF, 0xFFFFFFFF };

  std::vector<unsigned int> imageData1 = CreateStripedImageData(colors1, 4, 128, 128);
  std::vector<unsigned int> imageData2 = CreateStripedImageData(colors2, 4, 300, 300);

  HRTextureNodeRef testTex2 = hrTexture2DCreateFromMemory(128, 128, 4, &imageData1[0]);
  HRTextureNodeRef testTex3 = hrTexture2DCreateFromMemory(300, 300, 4, &imageData2[0]);

  //CreateStripedImageFile("tests_images/test_23/TexFromMemory.png", colors, 4, 128, 128);

  HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
  HRMaterialRef mat1 = hrMaterialCreate(L"mysimplemat2");
  HRMaterialRef mat2 = hrMaterialCreate(L"wood");

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
    diff.append_child(L"color").text().set(L"0.95 0.95 0.95");

    hrTextureBind(testTex3, diff);
  }
  hrMaterialClose(mat2);


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

    wchar_t temp[256];
    swprintf(temp, 256, L"%d", 1024);
    node.append_child(L"width").text().set(temp);
    swprintf(temp, 256, L"%d", 768);
    node.append_child(L"height").text().set(temp);
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
  mat4x4_rotate_X(mRot1, mRot1, -rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, -rquad*DEG_TO_RAD*0.5f);
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
  mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT4, mRes); // this fucking math library swap rows and columns

  // draw scene
  //
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    hrMeshInstance(scnRef, cubeRef, &matrixT[0][0]);
    hrMeshInstance(scnRef, planeRef, &matrixT2[0][0]);
    hrMeshInstance(scnRef, sphRef, &matrixT3[0][0]);
    hrMeshInstance(scnRef, torRef, &matrixT4[0][0]);
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, settingsRef);

  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_23/z_out.png");

  return check_images("test_23");
}


bool test24_many_textures_big_data()
{
  CreateTestBigTexturesFilesIfNeeded();

  initGLIfNeeded();
  hrSceneLibraryOpen(L"tests/test_24", HR_WRITE_DISCARD);
  
  const int NMats = 20;

  HRTextureNodeRef textures[NMats];

  textures[0] = hrTexture2DCreateFromFile(L"data/textures_gen/texture_big_red.png");
  textures[1] = hrTexture2DCreateFromFile(L"data/textures_gen/texture_green.png");
  textures[2] = hrTexture2DCreateFromFile(L"data/textures_gen/texture_blue.png");
  textures[3] = hrTexture2DCreateFromFile(L"data/textures_gen/texture_big_z01.png");
  textures[4] = hrTexture2DCreateFromFile(L"data/textures_gen/texture_big_z02.png");
  textures[5] = hrTexture2DCreateFromFile(L"data/textures_gen/texture_big_z03.png");
  textures[6] = hrTexture2DCreateFromFile(L"data/textures_gen/texture_big_z04.png");
  textures[7] = hrTexture2DCreateFromFile(L"data/textures_gen/texture_big_z05.png");
  textures[8] = hrTexture2DCreateFromFile(L"data/textures_gen/texture_big_z06.png");
  textures[9] = hrTexture2DCreateFromFile(L"data/textures_gen/texture_big_z07.png");
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  auto rgen = simplerandom::RandomGenInit(777);
  
  size_t memTotal = 0;
  for (int i = 0; i < 500; i++)
  {
    HRTextureNodeRef texRef = AddRandomTextureFromMemory(memTotal, rgen);
  
    if (i % 20 == 0)
      std::cout << "[test_mbg]: total mem = " << memTotal / size_t(1024 * 1024) << " MB, total textures = " << i << "\r";

    if (i < 10)
      textures[i + 10] = texRef;
  }
  std::cout << std::endl;

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  HRMaterialRef matetials[NMats];

  for (int i = 0; i < NMats; i++)
  {
    matetials[i] = hrMaterialCreate(L"");

    hrMaterialOpen(matetials[i], HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(matetials[i]);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.9 0.9 0.9");

      hrTextureBind(textures[i], diff);
    }
    hrMaterialClose(matetials[i]);

  }

  // geometry
  //
  HRMeshRef cubeRef   = HRMeshFromSimpleMesh(L"my_cube",   CreateCube(0.5f), 0);

  HRMeshRef planeRef  = HRMeshFromSimpleMesh(L"my_plane",  CreatePlane(2.0f), 1);
  HRMeshRef sphRef    = HRMeshFromSimpleMesh(L"my_sphere", CreateSphere(0.5f, 32), 2);
  HRMeshRef torRef    = HRMeshFromSimpleMesh(L"my_torus",  CreateTorus(0.2f, 0.5f, 32, 32), 3);

  HRMeshRef cubeRef2  = HRMeshFromSimpleMesh(L"my_cube2",   CreateCube(0.5f), 8);
  HRMeshRef sphRef2   = HRMeshFromSimpleMesh(L"my_sphere2", CreateSphere(0.5f, 32), 15);
  HRMeshRef torRef2   = HRMeshFromSimpleMesh(L"my_torus2",  CreateTorus(0.2f, 0.5f, 32, 32), 7);

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
    camNode.append_child(L"position").text().set(L"0 0 0");
    camNode.append_child(L"look_at").text().set(L"0 0 -1");
  }
  hrCameraClose(camRef);

  // set up render settings
  //
  HRRenderRef  settingsRef = hrRenderCreate(L"opengl1");

  hrRenderOpen(settingsRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(settingsRef);
    
    node.append_child(L"width").text()  = 512;
    node.append_child(L"height").text() = 512;
  }
  hrRenderClose(settingsRef);

  // create scene
  //
  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  float	rtri = 25.0f; // Angle For The Triangle ( NEW )
  float	rquad = 40.0f;
  float g_FPS = 60.0f;
  int   frameCounter = 0;

  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

  float matrixT[4][4], matrixT2[4][4], matrixT3[4][4], matrixT4[4][4];
  float mRot1[4][4], mTranslate[4][4], mRes[4][4];
  float matrixT5[4][4], matrixT6[4][4], matrixT7[4][4];

  float mTranslateDown[4][4], mRes2[4][4];

  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, 0.0f, -5.0f);
  mat4x4_rotate_X(mRot1, mRot1, -rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, -rquad*DEG_TO_RAD*0.5f);
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
  mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT4, mRes); // this fucking math library swap rows and columns

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  mat4x4_translate(mTranslate, 0.0f, 1.5f, -6.0f);
  mat4x4_rotate_X(mRot1, mRot1, -rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, -rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT5, mRes); // this fucking math library swap rows and columns


  mat4x4_identity(mRot1);
  mat4x4_identity(mRes);
  mat4x4_rotate_Y(mRes, mRot1, 2.5f*rquad*DEG_TO_RAD);
  mat4x4_translate(mTranslateDown, -1.5f, 1.0f, -5.0f);
  mat4x4_mul(mRes2, mTranslateDown, mRes);
  mat4x4_transpose(matrixT6, mRes2);

  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 2.0f, 1.75f, -6.0f);
  mat4x4_rotate_X(mRot1, mRot1, rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT7, mRes); // this fucking math library swap rows and columns

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // draw scene
  //
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    hrMeshInstance(scnRef, planeRef, &matrixT2[0][0]);

    hrMeshInstance(scnRef, cubeRef, &matrixT[0][0]);
    hrMeshInstance(scnRef, sphRef, &matrixT3[0][0]);
    hrMeshInstance(scnRef, torRef, &matrixT4[0][0]);

    hrMeshInstance(scnRef, cubeRef2, &matrixT5[0][0]);
    hrMeshInstance(scnRef, sphRef2, &matrixT6[0][0]);
    hrMeshInstance(scnRef, torRef2, &matrixT7[0][0]);
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, settingsRef);
  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_24/z_out.png");

  return check_images("test_24");
}


bool test25_many_textures_big_data()
{
  CreateTestBigTexturesFilesIfNeeded();

  initGLIfNeeded();
  hrSceneLibraryOpen(L"tests/test_25", HR_WRITE_DISCARD);

  const int NMats = 10;

  HRTextureNodeRef textures[NMats];

  textures[0] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_big_red.png");
  textures[1] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_green.png");
  textures[2] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_blue.png");
  textures[3] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_big_z01.png");
  textures[4] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_big_z02.png");
  textures[5] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_big_z03.png");
  textures[6] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_big_z04.png");
  textures[7] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_big_z05.png");
  textures[8] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_big_z06.png");
  textures[9] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_big_z07.png");


  HRMaterialRef matetials[NMats];

  for (int i = 0; i < NMats; i++)
  {
    matetials[i] = hrMaterialCreate(L"");

    hrMaterialOpen(matetials[i], HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(matetials[i]);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.9 0.9 0.9");

      hrTextureBind(textures[i], diff);
    }
    hrMaterialClose(matetials[i]);

  }

  // geometry
  //
  HRMeshRef cubeRef  = HRMeshFromSimpleMesh(L"my_cube", CreateCube(0.5f), 0);

  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane",  CreatePlane(2.0f), 1);
  HRMeshRef sphRef   = HRMeshFromSimpleMesh(L"my_sphere", CreateSphere(0.5f, 32), 2);
  HRMeshRef torRef   = HRMeshFromSimpleMesh(L"my_torus",  CreateTorus(0.2f, 0.5f, 32, 32), 3);

  HRMeshRef cubeRef2 = HRMeshFromSimpleMesh(L"my_cube2",   CreateCube(0.5f), 8);
  HRMeshRef sphRef2  = HRMeshFromSimpleMesh(L"my_sphere2", CreateSphere(0.5f, 32), 9);
  HRMeshRef torRef2  = HRMeshFromSimpleMesh(L"my_torus2",  CreateTorus(0.2f, 0.5f, 32, 32), 7);

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
    camNode.append_child(L"position").text().set(L"0 0 0");
    camNode.append_child(L"look_at").text().set(L"0 0 -1");
  }
  hrCameraClose(camRef);

  // set up render settings
  //
  HRRenderRef  settingsRef = hrRenderCreate(L"opengl1");

  hrRenderOpen(settingsRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(settingsRef);
    
    node.append_child(L"width").text()  = 512;
    node.append_child(L"height").text() = 512;
  }
  hrRenderClose(settingsRef);

  // create scene
  //
  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  float	rtri = 25.0f; // Angle For The Triangle ( NEW )
  float	rquad = 40.0f;
  float g_FPS = 60.0f;
  int   frameCounter = 0;

  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

  float matrixT[4][4], matrixT2[4][4], matrixT3[4][4], matrixT4[4][4];
  float mRot1[4][4], mTranslate[4][4], mRes[4][4];
  float matrixT5[4][4], matrixT6[4][4], matrixT7[4][4];

  float mTranslateDown[4][4], mRes2[4][4];

  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, 0.0f, -5.0f);
  mat4x4_rotate_X(mRot1, mRot1, -rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, -rquad*DEG_TO_RAD*0.5f);
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
  mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT4, mRes); // this fucking math library swap rows and columns

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  mat4x4_translate(mTranslate, 0.0f, 1.5f, -6.0f);
  mat4x4_rotate_X(mRot1, mRot1, -rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, -rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT5, mRes); // this fucking math library swap rows and columns


  mat4x4_identity(mRot1);
  mat4x4_identity(mRes);
  mat4x4_rotate_Y(mRes, mRot1, 2.5f*rquad*DEG_TO_RAD);
  mat4x4_translate(mTranslateDown, -1.5f, 1.0f, -5.0f);
  mat4x4_mul(mRes2, mTranslateDown, mRes);
  mat4x4_transpose(matrixT6, mRes2);

  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 2.0f, 1.75f, -6.0f);
  mat4x4_rotate_X(mRot1, mRot1, rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT7, mRes); // this fucking math library swap rows and columns

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // draw scene
  //
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    hrMeshInstance(scnRef, planeRef, &matrixT2[0][0]);

    hrMeshInstance(scnRef, cubeRef, &matrixT[0][0]);
    hrMeshInstance(scnRef, sphRef, &matrixT3[0][0]);
    hrMeshInstance(scnRef, torRef, &matrixT4[0][0]);

    hrMeshInstance(scnRef, cubeRef2, &matrixT5[0][0]);
    hrMeshInstance(scnRef, sphRef2, &matrixT6[0][0]);
    hrMeshInstance(scnRef, torRef2, &matrixT7[0][0]);
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, settingsRef);
  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_25/z_out.png");

  return check_images("test_25");
}


bool test26_many_textures_big_data()
{
  CreateTestBigTexturesFilesIfNeeded();

  initGLIfNeeded();
  hrSceneLibraryOpen(L"tests/test_26", HR_WRITE_DISCARD);

  const int NMats = 10;

  HRTextureNodeRef textures[NMats];

  textures[0] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_big_red.png");
  textures[1] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_green.png");
  textures[2] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_blue.png");
  textures[3] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_big_z01.png");
  textures[4] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_big_z02.png");
  textures[5] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_big_z03.png");
  textures[6] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_big_z04.png");
  textures[7] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_big_z05.png");
  textures[8] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_big_z06.png");
  textures[9] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_big_z07.png");


  HRMaterialRef matetials[NMats];

  for (int i = 0; i < NMats; i++)
  {
    matetials[i] = hrMaterialCreate(L"");

    hrMaterialOpen(matetials[i], HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(matetials[i]);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.9 0.9 0.9");

      hrTextureBind(textures[i], diff);
    }
    hrMaterialClose(matetials[i]);

  }

  // geometry
  //
  HRMeshRef cubeRef  = HRMeshFromSimpleMesh(L"my_cube", CreateCube(0.5f), 0);

  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(2.0f), 1);
  HRMeshRef sphRef   = HRMeshFromSimpleMesh(L"my_sphere", CreateSphere(0.5f, 32), 2);
  HRMeshRef torRef   = HRMeshFromSimpleMesh(L"my_torus", CreateTorus(0.2f, 0.5f, 32, 32), 3);

  HRMeshRef cubeRef2 = HRMeshFromSimpleMesh(L"my_cube2", CreateCube(0.5f), 8);
  HRMeshRef sphRef2  = HRMeshFromSimpleMesh(L"my_sphere2", CreateSphere(0.5f, 32), 9);
  HRMeshRef torRef2  = HRMeshFromSimpleMesh(L"my_torus2", CreateTorus(0.2f, 0.5f, 32, 32), 7);

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
    camNode.append_child(L"position").text().set(L"0 0 0");
    camNode.append_child(L"look_at").text().set(L"0 0 -1");
  }
  hrCameraClose(camRef);

  // set up render settings
  //
  HRRenderRef  settingsRef = hrRenderCreate(L"opengl1DelayedLoad");

  hrRenderOpen(settingsRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(settingsRef);
    
    node.append_child(L"width").text()  = 512;
    node.append_child(L"height").text() = 512;
  }
  hrRenderClose(settingsRef);

  // create scene
  //
  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  float	rtri = 25.0f; // Angle For The Triangle ( NEW )
  float	rquad = 40.0f;
  float g_FPS = 60.0f;
  int   frameCounter = 0;

  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

  float matrixT[4][4], matrixT2[4][4], matrixT3[4][4], matrixT4[4][4];
  float mRot1[4][4], mTranslate[4][4], mRes[4][4];
  float matrixT5[4][4], matrixT6[4][4], matrixT7[4][4];

  float mTranslateDown[4][4], mRes2[4][4];

  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, 0.0f, -5.0f);
  mat4x4_rotate_X(mRot1, mRot1, -rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, -rquad*DEG_TO_RAD*0.5f);
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
  mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT4, mRes); // this fucking math library swap rows and columns

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  mat4x4_translate(mTranslate, 0.0f, 1.5f, -6.0f);
  mat4x4_rotate_X(mRot1, mRot1, -rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, -rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT5, mRes); // this fucking math library swap rows and columns


  mat4x4_identity(mRot1);
  mat4x4_identity(mRes);
  mat4x4_rotate_Y(mRes, mRot1, 2.5f*rquad*DEG_TO_RAD);
  mat4x4_translate(mTranslateDown, -1.5f, 1.0f, -5.0f);
  mat4x4_mul(mRes2, mTranslateDown, mRes);
  mat4x4_transpose(matrixT6, mRes2);

  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 2.0f, 1.75f, -6.0f);
  mat4x4_rotate_X(mRot1, mRot1, rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT7, mRes); // this fucking math library swap rows and columns

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // draw scene
  //
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    hrMeshInstance(scnRef, planeRef, &matrixT2[0][0]);

    hrMeshInstance(scnRef, cubeRef, &matrixT[0][0]);
    hrMeshInstance(scnRef, sphRef, &matrixT3[0][0]);
    hrMeshInstance(scnRef, torRef, &matrixT4[0][0]);

    hrMeshInstance(scnRef, cubeRef2, &matrixT5[0][0]);
    hrMeshInstance(scnRef, sphRef2, &matrixT6[0][0]);
    hrMeshInstance(scnRef, torRef2, &matrixT7[0][0]);
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, settingsRef);
  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_26/z_out.png");

  return check_images("test_26");
}


bool test27_many_textures_big_data_from_mem()
{
  initGLIfNeeded();
  hrSceneLibraryOpen(L"tests/test_27", HR_WRITE_DISCARD);

  const int NMats = 20;

  HRTextureNodeRef textures[NMats];
 
  auto rgen = simplerandom::RandomGenInit(768756);

  size_t memTotal = 0;
  for (int i = 0; i < 500; i++)
  {
    HRTextureNodeRef texRef = CreateRandomStrippedTextureFromMemory(memTotal, rgen);
    
    if (i < NMats)
      textures[i] = texRef;

    if (i % 20 == 0)
      std::cout << "[test_mbg]: total mem = " << memTotal / size_t(1024 * 1024) << " MB, total textures = " << i << "\r";
  }
  std::cout << std::endl;

  HRMaterialRef matetials[NMats];

  for (int i = 0; i < NMats; i++)
  {
    matetials[i] = hrMaterialCreate(L"");

    hrMaterialOpen(matetials[i], HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(matetials[i]);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.9 0.9 0.9");

      hrTextureBind(textures[i], diff);
    }
    hrMaterialClose(matetials[i]);

  }

  // geometry
  //
  srand(888);

  HRMeshRef cubeRef  = HRMeshFromSimpleMesh(L"my_cube",    CreateCube(0.5f), rand()%20);
                                                           
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane",   CreatePlane(2.0f), rand() % 20);
  HRMeshRef sphRef   = HRMeshFromSimpleMesh(L"my_sphere",  CreateSphere(0.5f, 32), rand()%20);
  HRMeshRef torRef   = HRMeshFromSimpleMesh(L"my_torus",   CreateTorus(0.2f, 0.5f, 32, 32), rand()%20);

  HRMeshRef cubeRef2 = HRMeshFromSimpleMesh(L"my_cube2",   CreateCube(0.5f), 8);
  HRMeshRef sphRef2  = HRMeshFromSimpleMesh(L"my_sphere2", CreateSphere(0.5f, 32), 15);
  HRMeshRef torRef2  = HRMeshFromSimpleMesh(L"my_torus2",  CreateTorus(0.2f, 0.5f, 32, 32), rand()%20);

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
    camNode.append_child(L"position").text().set(L"0 0 0");
    camNode.append_child(L"look_at").text().set(L"0 0 -1");
  }
  hrCameraClose(camRef);

  // set up render settings
  //
  HRRenderRef  settingsRef = hrRenderCreate(L"opengl1DelayedLoad");

  hrRenderOpen(settingsRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(settingsRef);
    
    node.append_child(L"width").text()  = 512;
    node.append_child(L"height").text() = 512;
  }
  hrRenderClose(settingsRef);

  // create scene
  //
  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  float	rtri = 25.0f; // Angle For The Triangle ( NEW )
  float	rquad = 40.0f;
  float g_FPS = 60.0f;
  int   frameCounter = 0;

  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

  float matrixT[4][4], matrixT2[4][4], matrixT3[4][4], matrixT4[4][4];
  float mRot1[4][4], mTranslate[4][4], mRes[4][4];
  float matrixT5[4][4], matrixT6[4][4], matrixT7[4][4];

  float mTranslateDown[4][4], mRes2[4][4];

  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, 0.0f, -5.0f);
  mat4x4_rotate_X(mRot1, mRot1, -rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, -rquad*DEG_TO_RAD*0.5f);
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
  mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT4, mRes); // this fucking math library swap rows and columns

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  mat4x4_translate(mTranslate, 0.0f, 1.5f, -6.0f);
  mat4x4_rotate_X(mRot1, mRot1, -rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, -rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT5, mRes); // this fucking math library swap rows and columns


  mat4x4_identity(mRot1);
  mat4x4_identity(mRes);
  mat4x4_rotate_Y(mRes, mRot1, 2.5f*rquad*DEG_TO_RAD);
  mat4x4_translate(mTranslateDown, -1.5f, 1.0f, -5.0f);
  mat4x4_mul(mRes2, mTranslateDown, mRes);
  mat4x4_transpose(matrixT6, mRes2);

  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 2.0f, 1.75f, -6.0f);
  mat4x4_rotate_X(mRot1, mRot1, rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT7, mRes); // this fucking math library swap rows and columns

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // draw scene
  //
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    hrMeshInstance(scnRef, planeRef, &matrixT2[0][0]);

    hrMeshInstance(scnRef, cubeRef, &matrixT[0][0]);
    hrMeshInstance(scnRef, sphRef, &matrixT3[0][0]);
    hrMeshInstance(scnRef, torRef, &matrixT4[0][0]);

    hrMeshInstance(scnRef, cubeRef2, &matrixT5[0][0]);
    hrMeshInstance(scnRef, sphRef2, &matrixT6[0][0]);
    hrMeshInstance(scnRef, torRef2, &matrixT7[0][0]);
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, settingsRef);
  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_27/z_out.png");

  return check_images("test_27");
}


bool test29_many_textures_and_meshes()
{
  initGLIfNeeded();
  hrSceneLibraryOpen(L"tests/test_29", HR_WRITE_DISCARD);

  const int NMats = 100;

  HRTextureNodeRef textures[NMats];
  
  auto rgen = simplerandom::RandomGenInit(123);
  
  size_t memTotal = 0;
  for (int i = 0; i < 300; i++)
  {
    HRTextureNodeRef texRef = CreateRandomStrippedTextureFromMemory(memTotal, rgen);

    if (i < NMats)
      textures[i] = texRef;

    if (i % 20 == 0)
      std::cout << "[test_mbg]: total mem = " << memTotal / size_t(1024 * 1024) << " MB, total textures = " << i << "\r";
  }
  std::cout << std::endl;

  HRMaterialRef matetials[NMats];

  for (int i = 0; i < NMats; i++)
  {
    matetials[i] = hrMaterialCreate(L"");

    hrMaterialOpen(matetials[i], HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(matetials[i]);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.9 0.9 0.9");

      hrTextureBind(textures[i], diff);
    }
    hrMaterialClose(matetials[i]);
  }

  // geometry
  //
  
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(2.0f), simplerandom::rand(rgen) % 20);

  std::vector<HRMeshRef> meshes = CreateRandomMeshesArray(1000);
  
  // HRMeshRef cubeRef  = HRMeshFromSimpleMesh(L"my_cube",  CreateCube(0.5f), rand() % 20);
  // 
  // HRMeshRef sphRef   = HRMeshFromSimpleMesh(L"my_sphere", CreateSphere(0.5f, 32), rand() % 20);
  // HRMeshRef torRef   = HRMeshFromSimpleMesh(L"my_torus",  CreateTorus(0.2f, 0.5f, 32, 32), rand() % 20);
  // 
  // HRMeshRef cubeRef2 = HRMeshFromSimpleMesh(L"my_cube2",   CreateCube(0.5f), 8);
  // HRMeshRef sphRef2  = HRMeshFromSimpleMesh(L"my_sphere2", CreateSphere(0.5f, 32), 15);
  // HRMeshRef torRef2  = HRMeshFromSimpleMesh(L"my_torus2",  CreateTorus(0.2f, 0.5f, 32, 32), rand() % 20);

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
    camNode.append_child(L"position").text().set(L"0 0 0");
    camNode.append_child(L"look_at").text().set(L"0 0 -1");
  }
  hrCameraClose(camRef);

  // set up render settings
  //
  HRRenderRef  settingsRef = hrRenderCreate(L"opengl1DelayedLoad");

  hrRenderOpen(settingsRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(settingsRef);
    
    node.append_child(L"width").text()  = 512;
    node.append_child(L"height").text() = 512;
  }
  hrRenderClose(settingsRef);

  // create scene
  //
  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  float	rtri = 25.0f; // Angle For The Triangle ( NEW )
  float	rquad = 40.0f;
  float g_FPS = 60.0f;
  int   frameCounter = 0;

  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

  float matrixT[4][4], matrixT2[4][4], matrixT3[4][4], matrixT4[4][4];
  float mRot1[4][4], mTranslate[4][4], mRes[4][4];
  float matrixT5[4][4], matrixT6[4][4], matrixT7[4][4];

  float mTranslateDown[4][4], mRes2[4][4];

  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, 0.0f, -5.0f);
  mat4x4_rotate_X(mRot1, mRot1, -rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, -rquad*DEG_TO_RAD*0.5f);
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
  mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT4, mRes); // this fucking math library swap rows and columns

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  mat4x4_translate(mTranslate, 0.0f, 1.5f, -6.0f);
  mat4x4_rotate_X(mRot1, mRot1, -rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, -rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT5, mRes); // this fucking math library swap rows and columns


  mat4x4_identity(mRot1);
  mat4x4_identity(mRes);
  mat4x4_rotate_Y(mRes, mRot1, 2.5f*rquad*DEG_TO_RAD);
  mat4x4_translate(mTranslateDown, -1.5f, 1.0f, -5.0f);
  mat4x4_mul(mRes2, mTranslateDown, mRes);
  mat4x4_transpose(matrixT6, mRes2);

  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 2.0f, 1.75f, -6.0f);
  mat4x4_rotate_X(mRot1, mRot1, rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT7, mRes); // this fucking math library swap rows and columns

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // draw scene
  //
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    hrMeshInstance(scnRef, planeRef, &matrixT2[0][0]);

    hrMeshInstance(scnRef, meshes[rand() % (meshes.size() / 2)], &matrixT[0][0]);
    hrMeshInstance(scnRef, meshes[rand() % (meshes.size() / 2)], &matrixT3[0][0]);
    hrMeshInstance(scnRef, meshes[rand() % (meshes.size() / 2)], &matrixT4[0][0]);

    hrMeshInstance(scnRef, meshes[rand() % (meshes.size() / 2)], &matrixT5[0][0]);
    hrMeshInstance(scnRef, meshes[rand() % (meshes.size() / 2)], &matrixT6[0][0]);
    hrMeshInstance(scnRef, meshes[rand() % (meshes.size() / 2)], &matrixT7[0][0]);
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, settingsRef);
  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_29/z_out.png");

  return check_images("test_29");
}


bool test30_many_textures_and_meshes()
{
  initGLIfNeeded();
  hrSceneLibraryOpen(L"tests/test_30", HR_WRITE_DISCARD);

  const int NMats = 100;

  HRTextureNodeRef textures[NMats];

  auto rgen = simplerandom::RandomGenInit(123);
  
  size_t memTotal = 0;
  for (int i = 0; i < 300; i++)
  {
    HRTextureNodeRef texRef = CreateRandomStrippedTextureFromMemory(memTotal, rgen);

    if (i < NMats)
      textures[i] = texRef;

    if (i % 20 == 0)
      std::cout << "[test_mbg]: total mem = " << memTotal / size_t(1024 * 1024) << " MB, total textures = " << i << "\r";
  }
  std::cout << std::endl;

  HRMaterialRef matetials[NMats];

  for (int i = 0; i < NMats; i++)
  {
    matetials[i] = hrMaterialCreate(L"");

    hrMaterialOpen(matetials[i], HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(matetials[i]);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.9 0.9 0.9");

      hrTextureBind(textures[i], diff);
    }
    hrMaterialClose(matetials[i]);
  }

  // geometry
  //
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(2.0f), simplerandom::rand(rgen) % 20);

  std::vector<HRMeshRef> meshes = CreateRandomMeshesArray(600);

  // HRMeshRef cubeRef  = HRMeshFromSimpleMesh(L"my_cube",  CreateCube(0.5f), rand() % 20);
  // 
  // HRMeshRef sphRef   = HRMeshFromSimpleMesh(L"my_sphere", CreateSphere(0.5f, 32), rand() % 20);
  // HRMeshRef torRef   = HRMeshFromSimpleMesh(L"my_torus",  CreateTorus(0.2f, 0.5f, 32, 32), rand() % 20);
  // 
  // HRMeshRef cubeRef2 = HRMeshFromSimpleMesh(L"my_cube2",   CreateCube(0.5f), 8);
  // HRMeshRef sphRef2  = HRMeshFromSimpleMesh(L"my_sphere2", CreateSphere(0.5f, 32), 15);
  // HRMeshRef torRef2  = HRMeshFromSimpleMesh(L"my_torus2",  CreateTorus(0.2f, 0.5f, 32, 32), rand() % 20);

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
    camNode.append_child(L"position").text().set(L"0 0 0");
    camNode.append_child(L"look_at").text().set(L"0 0 -1");
  }
  hrCameraClose(camRef);

  // set up render settings
  //
  HRRenderRef  settingsRef = hrRenderCreate(L"opengl1DelayedLoad2");

  hrRenderOpen(settingsRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(settingsRef);
    
    node.append_child(L"width").text()  = 512;
    node.append_child(L"height").text() = 512;
  }
  hrRenderClose(settingsRef);

  // create scene
  //
  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  float	rtri = 25.0f; // Angle For The Triangle ( NEW )
  float	rquad = 40.0f;
  float g_FPS = 60.0f;
  int   frameCounter = 0;

  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

  float matrixT[4][4], matrixT2[4][4], matrixT3[4][4], matrixT4[4][4];
  float mRot1[4][4], mTranslate[4][4], mRes[4][4];
  float matrixT5[4][4], matrixT6[4][4], matrixT7[4][4];

  float mTranslateDown[4][4], mRes2[4][4];

  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, 0.0f, -5.0f);
  mat4x4_rotate_X(mRot1, mRot1, -rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, -rquad*DEG_TO_RAD*0.5f);
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
  mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT4, mRes); // this fucking math library swap rows and columns

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  mat4x4_translate(mTranslate, 0.0f, 1.5f, -6.0f);
  mat4x4_rotate_X(mRot1, mRot1, -rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, -rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT5, mRes); // this fucking math library swap rows and columns


  mat4x4_identity(mRot1);
  mat4x4_identity(mRes);
  mat4x4_rotate_Y(mRes, mRot1, 2.5f*rquad*DEG_TO_RAD);
  mat4x4_translate(mTranslateDown, -1.5f, 1.0f, -5.0f);
  mat4x4_mul(mRes2, mTranslateDown, mRes);
  mat4x4_transpose(matrixT6, mRes2);

  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 2.0f, 1.75f, -6.0f);
  mat4x4_rotate_X(mRot1, mRot1, rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT7, mRes); // this fucking math library swap rows and columns

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // draw scene
  //
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    hrMeshInstance(scnRef, planeRef, &matrixT2[0][0]);

    hrMeshInstance(scnRef, meshes[rand() % (meshes.size() / 2)], &matrixT[0][0]);
    hrMeshInstance(scnRef, meshes[rand() % (meshes.size() / 2)], &matrixT3[0][0]);
    hrMeshInstance(scnRef, meshes[rand() % (meshes.size() / 2)], &matrixT4[0][0]);

    hrMeshInstance(scnRef, meshes[rand() % (meshes.size() / 2)], &matrixT5[0][0]);
    hrMeshInstance(scnRef, meshes[rand() % (meshes.size() / 2)], &matrixT6[0][0]);
    hrMeshInstance(scnRef, meshes[rand() % (meshes.size() / 2)], &matrixT7[0][0]);
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, settingsRef);
  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_30/z_out.png");

  return check_images("test_30");
}


bool test34_delayed_textures_does_not_exists()
{
  CreateTestBigTexturesFilesIfNeeded();

  initGLIfNeeded();
  hrSceneLibraryOpen(L"tests/test_34", HR_WRITE_DISCARD);

  const int NMats = 10;

  HRTextureNodeRef textures[NMats];

  textures[0] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_big_red_not_exists.png");
  textures[1] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_green_not_exists.png");
  textures[2] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_blue.png");
  textures[3] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_big_z01.png");
  textures[4] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_big_z02_not_exists.png");
  textures[5] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_big_z03_not_exists.png");
  textures[6] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_big_z04.png");
  textures[7] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_big_z05_not_exists.png");
  textures[8] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_big_z06.png");
  textures[9] = hrTexture2DCreateFromFileDL(L"data/textures_gen/texture_big_z07_not_exists.png");


  HRMaterialRef matetials[NMats];

  for (int i = 0; i < NMats; i++)
  {
    matetials[i] = hrMaterialCreate(L"");

    hrMaterialOpen(matetials[i], HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(matetials[i]);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.9 0.9 0.9");

      hrTextureBind(textures[i], diff);
    }
    hrMaterialClose(matetials[i]);

  }

  // geometry
  //
  HRMeshRef cubeRef = HRMeshFromSimpleMesh(L"my_cube", CreateCube(0.5f), 0);

  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(2.0f), 1);
  HRMeshRef sphRef = HRMeshFromSimpleMesh(L"my_sphere", CreateSphere(0.5f, 32), 2);
  HRMeshRef torRef = HRMeshFromSimpleMesh(L"my_torus", CreateTorus(0.2f, 0.5f, 32, 32), 3);

  HRMeshRef cubeRef2 = HRMeshFromSimpleMesh(L"my_cube2", CreateCube(0.5f), 8);
  HRMeshRef sphRef2 = HRMeshFromSimpleMesh(L"my_sphere2", CreateSphere(0.5f, 32), 9);
  HRMeshRef torRef2 = HRMeshFromSimpleMesh(L"my_torus2", CreateTorus(0.2f, 0.5f, 32, 32), 7);

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
    camNode.append_child(L"position").text().set(L"0 0 0");
    camNode.append_child(L"look_at").text().set(L"0 0 -1");
  }
  hrCameraClose(camRef);

  // set up render settings
  //
  HRRenderRef  settingsRef = hrRenderCreate(L"opengl1DelayedLoad");

  hrRenderOpen(settingsRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(settingsRef);

    wchar_t temp[256];
    swprintf(temp, 256, L"%d", 1024);
    node.append_child(L"width").text().set(temp);
    swprintf(temp, 256, L"%d", 768);
    node.append_child(L"height").text().set(temp);
  }
  hrRenderClose(settingsRef);

  // create scene
  //
  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  float	rtri = 25.0f; // Angle For The Triangle ( NEW )
  float	rquad = 40.0f;
  float g_FPS = 60.0f;
  int   frameCounter = 0;

  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

  float matrixT[4][4], matrixT2[4][4], matrixT3[4][4], matrixT4[4][4];
  float mRot1[4][4], mTranslate[4][4], mRes[4][4];
  float matrixT5[4][4], matrixT6[4][4], matrixT7[4][4];

  float mTranslateDown[4][4], mRes2[4][4];

  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, 0.0f, -5.0f);
  mat4x4_rotate_X(mRot1, mRot1, -rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, -rquad*DEG_TO_RAD*0.5f);
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
  mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT4, mRes); // this fucking math library swap rows and columns

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  mat4x4_translate(mTranslate, 0.0f, 1.5f, -6.0f);
  mat4x4_rotate_X(mRot1, mRot1, -rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, -rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT5, mRes); // this fucking math library swap rows and columns


  mat4x4_identity(mRot1);
  mat4x4_identity(mRes);
  mat4x4_rotate_Y(mRes, mRot1, 2.5f*rquad*DEG_TO_RAD);
  mat4x4_translate(mTranslateDown, -1.5f, 1.0f, -5.0f);
  mat4x4_mul(mRes2, mTranslateDown, mRes);
  mat4x4_transpose(matrixT6, mRes2);

  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 2.0f, 1.75f, -6.0f);
  mat4x4_rotate_X(mRot1, mRot1, rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT7, mRes); // this fucking math library swap rows and columns

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // draw scene
  //
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    hrMeshInstance(scnRef, planeRef, &matrixT2[0][0]);

    hrMeshInstance(scnRef, cubeRef, &matrixT[0][0]);
    hrMeshInstance(scnRef, sphRef, &matrixT3[0][0]);
    hrMeshInstance(scnRef, torRef, &matrixT4[0][0]);

    hrMeshInstance(scnRef, cubeRef2, &matrixT5[0][0]);
    hrMeshInstance(scnRef, sphRef2, &matrixT6[0][0]);
    hrMeshInstance(scnRef, torRef2, &matrixT7[0][0]);
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, settingsRef);
  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_34/z_out.png");

  return check_images("test_34");
}
