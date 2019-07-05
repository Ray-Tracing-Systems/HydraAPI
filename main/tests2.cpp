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
#include <fstream>

#include "../hydra_api/HR_HDRImageTool.h"

static GLFWwindow* g_window = nullptr;
static int         g_width  = 1024;
static int         g_height = 768;

using namespace TEST_UTILS;

///////////////////////////////////////////////////////////

extern float g_MSEOutput;

bool check_images(const char* a_path, const int a_numImages, const float a_mse)
{
  g_MSEOutput = 0.0f;
  bool result = true;
  for (int i = 0; i < a_numImages; i++)
  {
    std::string path1 = std::string("tests_images/") + std::string(a_path) + "/z_out";
    std::string path2 = std::string("tests_images/") + std::string(a_path) + "/z_ref";
    std::string path3 = std::string("tests_images/") + std::string(a_path) + "/w_ref";

    if (i > 0)
    {
      std::stringstream strOut;
      strOut << i + 1;

      path1 += strOut.str();
      path2 += strOut.str();
      path3 += strOut.str();
    }

    path1 += ".png";
    path2 += ".png";
    path3 += ".png";

    // use w_ref.png by default, if don't have such, use z_ref.png
    //
    path2 = path3;

    int w1, h1, w2, h2;

    std::vector<int32_t> image1, image2;
    HydraRender::LoadLDRImageFromFile(path1.c_str(), &w1, &h1, image1);
    HydraRender::LoadLDRImageFromFile(path2.c_str(), &w2, &h2, image2);

    if (w1 != w2 || h1 != h2)
    {
      g_MSEOutput = 1000000.0f;
      return false;
    }

    const float mseVal = HydraRender::MSE_RGB_LDR(image1, image2);
    g_MSEOutput = fmax(g_MSEOutput, mseVal);

    result = result && (mseVal <= a_mse);
  }

  return result;
}

///////////////////////////////////////////////////////////


bool test10_render_ogl_cube()
{
  initGLIfNeeded(1024, 768);

  hrErrorCallerPlace(L"test10_render_ogl_cube");

  HRCameraRef    camRef;
  HRMeshRef      cubeRef;
  HRSceneInstRef scnRef;
  HRRenderRef    settingsRef;

  // cube geometry
  //
  uint32_t numberVertices = 24;
  uint32_t numberIndices  = 36;

  float cubeVertices[] =
  {
    -1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, +1.0f, +1.0f,
    +1.0f, -1.0f, +1.0f, +1.0f,
    +1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, +1.0f, -1.0f, +1.0f,
    -1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, +1.0f, -1.0f, +1.0f,
    +1.0f, +1.0f, -1.0f, +1.0f,
    +1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, +1.0f, +1.0f,
    -1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, -1.0f, +1.0f, +1.0f,
    -1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, +1.0f, +1.0f,
    -1.0f, +1.0f, +1.0f, +1.0f,
    -1.0f, +1.0f, -1.0f, +1.0f,
    +1.0f, -1.0f, -1.0f, +1.0f,
    +1.0f, -1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, -1.0f, +1.0f
  };

  float cubeNormals[] =
  {
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f
  };

  float cubeTexCoords[] =
  {
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
  };

  uint32_t cubeIndices[] =
  {
    0, 2, 1,
    0, 3, 2,
    4, 5, 6,
    4, 6, 7,
    8, 9, 10,
    8, 10, 11,
    12, 15, 14,
    12, 14, 13,
    16, 17, 18,
    16, 18, 19,
    20, 23, 22,
    20, 22, 21
  };

  // form scene library

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_10", HR_WRITE_DISCARD);

  // material and textures
  //
  HRTextureNodeRef testTex2 = hrTexture2DCreateFromFile(L"data/textures/chess_red.bmp");

  HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");

  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat0);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"0.5 0.75 0.5");

    HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
    hrTextureBind(testTex, diff);
  }
  hrMaterialClose(mat0);

  // geometry
  //
  cubeRef = hrMeshCreate(L"cube");

  hrMeshOpen(cubeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeRef, L"pos", cubeVertices);
    hrMeshVertexAttribPointer4f(cubeRef, L"norm", cubeNormals);
    hrMeshVertexAttribPointer2f(cubeRef, L"texcoord", cubeTexCoords);

    hrMeshMaterialId(cubeRef, 0);
    hrMeshAppendTriangles3(cubeRef, numberIndices, (int*)cubeIndices);
  }
  hrMeshClose(cubeRef);

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

    node.append_child(L"width").text() = 1024;
    node.append_child(L"height").text() = 768;
  }
  hrRenderClose(settingsRef);

  // create scene
  //
  scnRef = hrSceneCreate(L"my scene");

  float	rtri  = 25.0f; // Angle For The Triangle ( NEW )
  float	rquad = 40.0f;
  float g_FPS = 60.0f;
  int   frameCounter = 0;

  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

  float matrixT[4][4];
  float mRot1[4][4], mTranslate[4][4], mRes[4][4];

  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, 0.0f, -5.0f);
  mat4x4_rotate_X(mRot1, mRot1, -rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, -rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);

  mat4x4_transpose(matrixT, mRes); // this fucking math library swap rows and columns

  // draw scene
  //
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    hrMeshInstance(scnRef, cubeRef, &matrixT[0][0]);
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, settingsRef);

  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_10/z_out.png");

  return check_images("test_10");
}


bool test11_render_ogl_some_figures()
{
  initGLIfNeeded(1024,768);

  hrErrorCallerPlace(L"test11");

  HRCameraRef    camRef;
  HRSceneInstRef scnRef;
  HRRenderRef    settingsRef;


  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_11", HR_WRITE_DISCARD);

  // geometry
  //
  HRMeshRef cubeRef  = HRMeshFromSimpleMesh(L"my_cube",   CreateCube(0.75f), 0);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane",  CreatePlane(2.0f), 1);
  HRMeshRef sphRef   = HRMeshFromSimpleMesh(L"my_sphere", CreateSphere(0.5f, 32), 2);
  HRMeshRef torRef   = HRMeshFromSimpleMesh(L"my_torus",  CreateTorus(0.2f, 0.5f, 32, 32), 0);

  // material and textures
  //
  HRTextureNodeRef testTex2 = hrTexture2DCreateFromFile(L"data/textures/chess_red.bmp");

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
    diff.append_child(L"color").text().set(L"0.75 0.75 0.75");

    HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/relief_wood.jpg");
    hrTextureBind(testTex, diff);
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
    hrMeshInstance(scnRef, cubeRef,  &matrixT[0][0]);
    hrMeshInstance(scnRef, planeRef, &matrixT2[0][0]);
    hrMeshInstance(scnRef, sphRef,   &matrixT3[0][0]);
    hrMeshInstance(scnRef, torRef,   &matrixT4[0][0]);
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, settingsRef);

  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_11/z_out.png");

  return check_images("test_11");
}



bool test12_render_ogl_100_random_figures()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test12");

  HRCameraRef    camRef;
  HRSceneInstRef scnRef;
  HRRenderRef    settingsRef;

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_12", HR_WRITE_DISCARD);

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

  auto rgen = simplerandom::RandomGenInit(125467);

  for (int i = -5; i <= 5; i++)
  {
    for (int j = -5; j <= 5; j++)
    {
      float xCoord = 2.0f*float(i);
      float yCoord = 2.0f*float(j);


      mat4x4_identity(mRot1);
      mat4x4_identity(mTranslate);
      mat4x4_identity(mRes);

      mat4x4_translate(mTranslate, xCoord, 0.5f, yCoord);
      mat4x4_rotate_X(mRot1, mRot1, simplerandom::rnd(rgen, 0.0f, 360.0f)*DEG_TO_RAD);
      mat4x4_rotate_Y(mRot1, mRot1, simplerandom::rnd(rgen, 0.0f, 360.0f)*DEG_TO_RAD*0.5f);
      mat4x4_mul(mRes, mTranslate, mRot1);
      mat4x4_transpose(matrixT, mRes); // this fucking math library swap rows and columns

      hrMeshInstance(scnRef, refs[simplerandom::rand(rgen) % 3], &matrixT[0][0]);
    }
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, settingsRef);

  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_12/z_out.png");

  return check_images("test_12");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool test13_render_ogl_some_figures_diff_mats_prom_ptr()
{
  initGLIfNeeded(1024,768);

  hrErrorCallerPlace(L"test11");

  HRCameraRef    camRef;
  HRSceneInstRef scnRef;
  HRRenderRef    settingsRef;


  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_13", HR_WRITE_DISCARD);

  // material and textures
  //
  HRTextureNodeRef testTex2 = hrTexture2DCreateFromFile(L"data/textures/chess_red.bmp");

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
  HRMaterialRef mat1 = hrMaterialCreate(L"mysimplemat2");
  HRMaterialRef mat2 = hrMaterialCreate(L"mysimplemat3");
  HRMaterialRef mat3 = hrMaterialCreate(L"mysimplemat4");

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

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  SimpleMesh cube   = CreateCube(0.75f);
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
    hrMeshVertexAttribPointer4f(sphereRef, L"pos",  &sphere.vPos[0]);
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
  mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT4, mRes); // this fucking math library swap rows and columns

  // draw scene
  //
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    hrMeshInstance(scnRef, cubeRef, &matrixT[0][0]);
    hrMeshInstance(scnRef, planeRef, &matrixT2[0][0]);
    hrMeshInstance(scnRef, sphereRef, &matrixT3[0][0]);
    hrMeshInstance(scnRef, torusRef, &matrixT4[0][0]);
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, settingsRef);

  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_13/z_out.png");

  return check_images("test_13");
}

bool g_test14ErrorIsOk = false;

void ErrorCallBack2(const wchar_t* message, const wchar_t* callerPlace)
{
  std::wstring tmp(message);
  
  auto foundPos  = tmp.find(L"bad material id");
  auto foundPos2 = tmp.find(L"[5, 3");
  auto foundPos3 = tmp.find(L"[3, 5");

  g_test14ErrorIsOk = (foundPos != std::wstring::npos && (foundPos2 != std::wstring::npos || foundPos3 != std::wstring::npos) && std::wstring(callerPlace) == L"test_14");
}


bool test14_bad_material_indices()
{
  initGLIfNeeded();

  g_test14ErrorIsOk = false;

  hrErrorCallerPlace(L"test_14");
  hrErrorCallback(ErrorCallBack2);
  hrInfoCallback(nullptr);

  HRCameraRef    camRef;
  HRSceneInstRef scnRef;
  HRRenderRef    settingsRef;

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_14", HR_WRITE_DISCARD);

  // geometry
  //
  HRMeshRef cubeRef  = HRMeshFromSimpleMesh(L"my_cube",   CreateCube(0.75f), 0);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane",  CreatePlane(2.0f), 5);
  HRMeshRef sphRef   = HRMeshFromSimpleMesh(L"my_sphere", CreateSphere(0.5f, 32), 2);
  HRMeshRef torRef   = HRMeshFromSimpleMesh(L"my_torus",  CreateTorus(0.2f, 0.5f, 32, 32), 3);

  // material and textures
  //
  HRTextureNodeRef testTex2 = hrTexture2DCreateFromFile(L"data/textures/chess_red.bmp");

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
    diff.append_child(L"color").text().set(L"0.75 0.75 0.75");

    HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/relief_wood.jpg");
    hrTextureBind(testTex, diff);
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

  hrErrorCallback(ErrorCallBack);
  hrInfoCallback(InfoCallBack);

  return g_test14ErrorIsOk;
}



bool test15_main_scene_and_mat_editor()
{
  initGLIfNeeded(1024,768);

  HRCameraRef    camRef;
  HRSceneInstRef scnRef;
  HRRenderRef    settingsRef;

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_15", HR_WRITE_DISCARD);

  SimpleMesh cube   = CreateCube(0.65f);
  SimpleMesh plane  = CreatePlane(10.0f);
  SimpleMesh sphere = CreateSphere(0.5f, 32);
  SimpleMesh torus  = CreateTorus(0.3f, 0.6f, 32, 32);

  for (size_t i = 0; i < plane.vTexCoord.size(); i++)
    plane.vTexCoord[i] *= 2.0f;

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRTextureNodeRef testTex2 = hrTexture2DCreateFromFile(L"data/textures/chess_red.bmp");

  HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
  HRMaterialRef mat1 = hrMaterialCreate(L"mysimplemat2");
  HRMaterialRef mat2 = hrMaterialCreate(L"mysimplemat3");
  HRMaterialRef mat3 = hrMaterialCreate(L"mysimplemat4");
  HRMaterialRef mat4 = hrMaterialCreate(L"myblue");
  HRMaterialRef mat5 = hrMaterialCreate(L"mymatplane");

  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat0);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"0.5 0.75 0.5");

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
    diff.append_child(L"color").text().set(L"0.1 0.1 0.75");
  }
  hrMaterialClose(mat4);

  hrMaterialOpen(mat5, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat5);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"0.75 0.75 0.25");

    HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
    hrTextureBind(testTex, diff);
  }
  hrMaterialClose(mat5);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMeshRef cubeRef   = hrMeshCreate(L"my_cube");
  HRMeshRef planeRef  = hrMeshCreate(L"my_plane");
  HRMeshRef sphereRef = hrMeshCreate(L"my_sphere");
  HRMeshRef torusRef  = hrMeshCreate(L"my_torus");

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

  camRef = hrCameraCreate(L"my camera");

  hrCameraOpen(camRef, HR_WRITE_DISCARD);
  {
    xml_node camNode = hrCameraParamNode(camRef);

    camNode.append_child(L"fov").text().set(L"45");
    camNode.append_child(L"nearClipPlane").text().set(L"0.01");
    camNode.append_child(L"farClipPlane").text().set(L"100.0");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 2 10");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  // set up render settings
  //
  settingsRef = hrRenderCreate(L"opengl1");

  hrRenderOpen(settingsRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(settingsRef);

    wchar_t temp[256];
    swprintf(temp, 256, L"%d", g_width);
    node.append_child(L"width").text().set(temp);
    swprintf(temp, 256, L"%d", g_height);
    node.append_child(L"height").text().set(temp);
  }
  hrRenderClose(settingsRef);

  static GLfloat	rtri = 25.0f; // Angle For The Triangle ( NEW )
  static GLfloat	rquad = 40.0f;
  static float    g_FPS = 60.0f;
  static int      frameCounter = 0;

  // create scene
  //
  scnRef = hrSceneCreate(L"my scene");

  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

  float matrixT[4][4], matrixT2[4][4], matrixT3[4][4], matrixT4[4][4];
  float mRot1[4][4], mTranslate[4][4], mRes[4][4];

  float mTranslateDown[4][4], mRes2[4][4];

  int sphereMMs[3][2] = { { 3, 4 },    { 4, 2    }, { 1, 3    } };
  int torusMMs[3][3]  = { { 0, 1, 2 }, { 1, 2, 3 }, { 3, 4, 1 } };

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  mat4x4_identity(mRes);
  mat4x4_translate(mTranslateDown, 0.0f, -1.0f, -5.0f);
  mat4x4_mul(mRes2, mTranslateDown, mRes);
  mat4x4_transpose(matrixT2, mRes2);

  hrMeshInstance(scnRef, planeRef, &matrixT2[0][0]);

  for (int i = -1; i <= 1; i++)
  {
    int mmIndex = i + 1;
    mat4x4_identity(mRot1);
    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, 0.0f, 0.25f, -5.0f + float(i)*8.0f);
    mat4x4_rotate_X(mRot1, mRot1, -rquad*DEG_TO_RAD);
    mat4x4_rotate_Y(mRot1, mRot1, -rquad*DEG_TO_RAD*0.5f);
    mat4x4_mul(mRes, mTranslate, mRot1);
    mat4x4_transpose(matrixT, mRes); // this fucking math library swap rows and columns

    mat4x4_identity(mRot1);
    mat4x4_identity(mRes);
    mat4x4_rotate_Y(mRes, mRot1, rquad*DEG_TO_RAD);
    mat4x4_translate(mTranslateDown, -1.5f, -0.5f, -4.0f + float(i)*8.0f);
    mat4x4_mul(mRes2, mTranslateDown, mRes);
    mat4x4_transpose(matrixT3, mRes2);

    mat4x4_identity(mRot1);
    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, 2.0f, 0.25f, -5.0f + float(i)*8.0f);
    mat4x4_rotate_X(mRot1, mRot1, rquad*DEG_TO_RAD);
    mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD*0.5f);
    mat4x4_mul(mRes, mTranslate, mRot1);
    mat4x4_transpose(matrixT4, mRes); // this fucking math library swap rows and columns

    hrMeshInstance(scnRef, cubeRef, &matrixT[0][0]);
    hrMeshInstance(scnRef, sphereRef, &matrixT3[0][0], &sphereMMs[mmIndex][0], 2);
    hrMeshInstance(scnRef, torusRef, &matrixT4[0][0], &torusMMs[mmIndex][0], 3);
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, settingsRef, camRef);

  std::vector<int> frameBufferData(1024 * 768);
  hrRenderGetFrameBufferLDR1i(settingsRef, 1024, 768, &frameBufferData[0]);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // now render 3 material balls with different render instance

  float mScale[4][4];

  hrCameraOpen(camRef, HR_OPEN_EXISTING);
  {
    xml_node camNode = hrCameraParamNode(camRef);
    camNode.child(L"position").text().set(L"0 1 2.25");
    camNode.child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);


  HRSceneInstRef matScene = hrSceneCreate(L"mat_editor1");

  HRMeshRef sphForMatRef2 = HRMeshFromSimpleMesh(L"mat-sphere2", CreateSphere(0.5f, 32), 2);
  HRMeshRef sphForMatRef3 = HRMeshFromSimpleMesh(L"mat-sphere3", CreateSphere(0.5f, 32), 3);
  HRMeshRef sphForMatRef4 = HRMeshFromSimpleMesh(L"mat-sphere4", CreateSphere(0.5f, 32), 4);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneOpen(matScene, HR_WRITE_DISCARD);
  {
    mat4x4_identity(mRes);
    mat4x4_identity(mTranslateDown);

    mat4x4_translate(mTranslateDown, 0.0f, -1.0f, -5.0f);
    mat4x4_mul(mRes2, mTranslateDown, mRes);
    mat4x4_transpose(matrixT2, mRes2);

    hrMeshInstance(matScene, planeRef, &matrixT2[0][0]);

    mat4x4_identity(mRes);
    mat4x4_identity(mScale);
    mScale[0][0] = mScale[1][1] = mScale[2][2] = 1.5f;

    mat4x4_mul(mRes, mScale, mRes);
    mat4x4_transpose(matrixT3, mRes);

    hrMeshInstance(matScene, sphForMatRef2, &matrixT3[0][0]);
  }
  hrSceneClose(matScene);


  HRRenderRef settingsRef2 = hrRenderCreate(L"opengl1");

  hrRenderOpen(settingsRef2, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(settingsRef2);
    node.append_child(L"width").text().set(L"256");
    node.append_child(L"height").text().set(L"256");
  }
  hrRenderClose(settingsRef2);

  std::vector<int> frameBufferDataMat(256 * 256);

  hrFlush(matScene, settingsRef2, camRef);
  hrRenderGetFrameBufferLDR1i(settingsRef2, 256, 256, &frameBufferDataMat[0]);

  for (int y = 0; y < 256; y++)
    for (int x = 0; x < 256; x++)
      frameBufferData[(768 - y - 1) * 1024 + x] = frameBufferDataMat[(256 - y - 1) * 256 + x];


  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneOpen(matScene, HR_WRITE_DISCARD);
  {
    mat4x4_identity(mRes);
    mat4x4_identity(mTranslateDown);

    mat4x4_translate(mTranslateDown, 0.0f, -1.0f, -5.0f);
    mat4x4_mul(mRes2, mTranslateDown, mRes);
    mat4x4_transpose(matrixT2, mRes2);

    hrMeshInstance(matScene, planeRef, &matrixT2[0][0]);

    mat4x4_identity(mRes);
    mat4x4_identity(mScale);
    mScale[0][0] = mScale[1][1] = mScale[2][2] = 1.5f;

    mat4x4_mul(mRes, mScale, mRes);
    mat4x4_transpose(matrixT3, mRes);

    hrMeshInstance(matScene, sphForMatRef4, &matrixT3[0][0]);
  }
  hrSceneClose(matScene);


  hrFlush(matScene, settingsRef2, camRef);
  hrRenderGetFrameBufferLDR1i(settingsRef2, 256, 256, &frameBufferDataMat[0]);

  for (int y = 0; y < 256; y++)
    for (int x = 0; x < 256; x++)
      frameBufferData[(768 - y - 1 - 256) * 1024 + x] = frameBufferDataMat[(256 - y - 1) * 256 + x];

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneOpen(matScene, HR_WRITE_DISCARD);
  {
    mat4x4_identity(mRes);
    mat4x4_identity(mTranslateDown);

    mat4x4_translate(mTranslateDown, 0.0f, -1.0f, -5.0f);
    mat4x4_mul(mRes2, mTranslateDown, mRes);
    mat4x4_transpose(matrixT2, mRes2);

    hrMeshInstance(matScene, planeRef, &matrixT2[0][0]);

    mat4x4_identity(mRes);
    mat4x4_identity(mScale);
    mScale[0][0] = mScale[1][1] = mScale[2][2] = 1.5f;

    mat4x4_mul(mRes, mScale, mRes);
    mat4x4_transpose(matrixT3, mRes);

    hrMeshInstance(matScene, sphForMatRef3, &matrixT3[0][0]);
  }
  hrSceneClose(matScene);


  hrFlush(matScene, settingsRef2, camRef);
  hrRenderGetFrameBufferLDR1i(settingsRef2, 256, 256, &frameBufferDataMat[0]);

  for (int y = 0; y < 256; y++)
    for (int x = 0; x < 256; x++)
      frameBufferData[(768 - y - 1 - 512) * 1024 + x] = frameBufferDataMat[(256 - y - 1) * 256 + x];

  HydraRender::SaveImageToFile(std::string("tests_images/test_15/z_out.png"), 1024, 768, (unsigned int*)&frameBufferData[0]);

  bool noDups1 = check_all_duplicates(L"tests/test_15/statex_00002.xml");
  bool noDups2 = check_all_duplicates(L"tests/test_15/statex_00003.xml");
  bool noDups3 = check_all_duplicates(L"tests/test_15/statex_00004.xml");

  return check_images("test_15") && noDups1 && noDups2 && noDups3;
}

