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
using HydraRender::SaveImageToFile;

extern GLFWwindow* g_window;

bool test35_cornell_with_light()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_35");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_35", HR_WRITE_DISCARD);

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
  HRRenderRef renderRef = hrRenderCreate(L"HydraModern"); // opengl1 HydraModern
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  auto pList = hrRenderGetDeviceList(renderRef);

  while (pList != nullptr)
  {
    std::wcout << L"device id = " << pList->id << L", name = " << pList->name << L", driver = " << pList->driver << std::endl;
    pList = pList->next;
  }

  //hrRenderEnableDevice(renderRef, 0, true);
  //hrRenderEnableDevice(renderRef, 1, true);
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

  glViewport(0, 0, 1024, 768);
  std::vector<int32_t> image(1024 * 768);

  while(true)
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
  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_35/z_out.png");

  return check_images("test_35");
}

static inline int RealColorToUint32(const float real_color[4])
{
  float  r = fmin(real_color[0] * 255.0f, 255.0f);
  float  g = fmin(real_color[1] * 255.0f, 255.0f);
  float  b = fmin(real_color[2] * 255.0f, 255.0f);
  float  a = fmin(real_color[3] * 255.0f, 255.0f);

  unsigned char red   = (unsigned char)r;
  unsigned char green = (unsigned char)g;
  unsigned char blue  = (unsigned char)b;
  unsigned char alpha = (unsigned char)a;

  return red | (green << 8) | (blue << 16) | (alpha << 24);
}

bool test37_cornell_with_light_different_image_layers()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_37");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_37", HR_WRITE_DISCARD);

  SimpleMesh cube     = CreateCube(0.75f);
  SimpleMesh plane    = CreatePlane(10.0f);
  SimpleMesh sphere   = CreateSphere(1.0f, 32);
  SimpleMesh torus    = CreateTorus(0.25f, 0.6f, 32, 32);
  SimpleMesh cubeOpen = CreateCubeOpen(4.0f);

  for (size_t i = 0; i < plane.vTexCoord.size(); i++)
    plane.vTexCoord[i] *= 2.0f;


  HRTextureNodeRef testTex2 = hrTexture2DCreateFromFileDL(L"data/textures/chess_white.bmp");

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
    diff.append_child(L"color").text().set(L"0.75 0.75 0.75");

    HRTextureNodeRef testTex = hrTexture2DCreateFromFileDL(L"data/textures/texture1.bmp");
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

    HRTextureNodeRef testTex = hrTexture2DCreateFromFileDL(L"data/textures/relief_wood.jpg");
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

    node.append_child(L"width").text()  = 1024;
    node.append_child(L"height").text() = 768;

    node.append_child(L"method_primary").text()   = L"pathtracing"; // L"pathtracing"; // L"lighttracing";
    node.append_child(L"trace_depth").text()      = 5;
    node.append_child(L"diff_trace_depth").text() = 3;
    node.append_child(L"maxRaysPerPixel").text()  = 1024;
    node.append_child(L"evalgbuffer").text()      = 1;
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
    else if (info.haveUpdateMSG)
    {
      std::wcout << L"msg = " << info.msg << " \r";
    }

    if (info.finalUpdate)
      break;

  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_37/z_out.png");

  std::vector<int32_t> normalsImageLDR(1024 * 768);
  std::vector<int32_t> txColorImageLDR(1024 * 768);
  std::vector<int32_t> depthImageLDR(1024 * 768);
  std::vector<int32_t> mindicesImageLDR(1024 * 768);
  std::vector<int32_t> instIndicesImageLDR(1024 * 768);
  std::vector<int32_t> objIndicesImageLDR(1024 * 768);
  std::vector<int32_t> shadowImageLDR(1024 * 768);

  std::vector<HRGBufferPixel> line(1024);

  const float invGamma = 1.0f / 2.2f;

  const unsigned int palette[20] = { 0xffe6194b, 0xff3cb44b, 0xffffe119, 0xff0082c8,
                                     0xfff58231, 0xff911eb4, 0xff46f0f0, 0xfff032e6,
                                     0xffd2f53c, 0xfffabebe, 0xff008080, 0xffe6beff,
                                     0xffaa6e28, 0xfffffac8, 0xff800000, 0xffaaffc3,
                                     0xff808000, 0xffffd8b1, 0xff000080, 0xff808080 };

  for (int y = 0; y < 768; y++)
  {
    hrRenderGetGBufferLine(renderRef, y, &line[0], 0, 1024);

    for (int x = 0; x < 1024; x++)
    {
      float norm[4];
      float texc[4];
      float depth[4];

      norm[0] = fabs(line[x].norm[0]);
      norm[1] = fabs(line[x].norm[1]);
      norm[2] = fabs(line[x].norm[2]);
      norm[3] = 1.0f;

      texc[0] = line[x].rgba[0];
      texc[1] = line[x].rgba[1];
      texc[2] = line[x].rgba[2];
      texc[3] = 1.0f;

      float ldepth = line[x].depth;

      if (ldepth >= 1000000.0f)
        ldepth = 0.0f;

      depth[0] = ldepth*0.025f;
      depth[1] = ldepth*0.025f;
      depth[2] = ldepth*0.025f;
      depth[3] = 1.0f;


      const float color[4] = { powf(texc[0], invGamma),
                               powf(texc[1], invGamma), 
                               powf(texc[2], invGamma),
                                    texc[3]
      };

      const float shadow[4] = { line[x].shadow, line[x].shadow, line[x].shadow, 1 };

      normalsImageLDR[y * 1024 + x] = RealColorToUint32(norm);
      txColorImageLDR[y * 1024 + x] = RealColorToUint32(color);
      depthImageLDR  [y * 1024 + x] = RealColorToUint32(depth);
      shadowImageLDR [y * 1024 + x] = RealColorToUint32(shadow);

      if (line[x].matId == -1 || line[x].depth >= 5e5f)
      {
        mindicesImageLDR   [y * 1024 + x] = 0xFF000000;
        instIndicesImageLDR[y * 1024 + x] = 0xFF000000;
        objIndicesImageLDR [y * 1024 + x] = 0xFF000000;
      }
      else
      {
        mindicesImageLDR   [y * 1024 + x] = palette[line[x].matId  % 20];
        instIndicesImageLDR[y * 1024 + x] = palette[line[x].instId % 20];
        objIndicesImageLDR [y * 1024 + x] = palette[line[x].objId  % 20];
      }
    }

  }

  SaveImageToFile(std::string("tests_images/test_37/z_out2.png"), 1024, 768, (unsigned int*)&normalsImageLDR[0]);
  SaveImageToFile(std::string("tests_images/test_37/z_out3.png"), 1024, 768, (unsigned int*)&txColorImageLDR[0]);
  SaveImageToFile(std::string("tests_images/test_37/z_out4.png"), 1024, 768, (unsigned int*)&depthImageLDR[0]);

  SaveImageToFile(std::string("tests_images/test_37/z_out5.png"), 1024, 768, (unsigned int*)&mindicesImageLDR[0]);
  SaveImageToFile(std::string("tests_images/test_37/z_out6.png"), 1024, 768, (unsigned int*)&instIndicesImageLDR[0]);
  SaveImageToFile(std::string("tests_images/test_37/z_out7.png"), 1024, 768, (unsigned int*)&objIndicesImageLDR[0]);
  SaveImageToFile(std::string("tests_images/test_37/z_out8.png"), 1024, 768, (unsigned int*)&shadowImageLDR[0]);

  return check_images("test_37", 8);
}


bool test38_test_for_mlt()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_38");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_38", HR_WRITE_DISCARD);

  SimpleMesh cube     = CreateCube(1.75f);
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

    // xml_node diff = matNode.append_child(L"diffuse");
    // 
    // diff.append_attribute(L"brdf_type").set_value(L"lambert");
    // diff.append_child(L"color").text().set(L"0.75 0.75 0.5");
    // 
    // HRTextureNodeRef testTex = hrTexture2DCreateFromFileDL(L"data/textures/texture1.bmp");
    // hrTextureBind(testTex, diff);

    xml_node refl = matNode.append_child(L"reflectivity");

    refl.append_attribute(L"brdf_type").set_value(L"phong");
    refl.append_child(L"color").text().set(L"0.85 0.85 0.85");
    refl.append_child(L"glossiness").text().set(L"0.85");
    //refl.append_child(L"glosiness_tex_mult").text().set(L"0.5");
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

    HRTextureNodeRef testTex = hrTexture2DCreateFromFileDL(L"data/textures/163.jpg");
    hrTextureBind(testTex, diff);
  }
  hrMaterialClose(mat2);

  hrMaterialOpen(mat3, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat3);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"0.85 0.85 0.85");

    HRTextureNodeRef testTex = hrTexture2DCreateFromFileDL(L"data/textures/texture1.bmp");
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

    int cubeMatIndices[12] = { 1, 1, 0, 0, 2, 2, 3, 3, 4, 4, 2, 2 };

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
    intensityNode.append_child(L"multiplier").text().set(L"10.0");
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

    node.append_child(L"width").text()  = 1024;
    node.append_child(L"height").text() = 768;

    node.append_child(L"method_primary").text()   = L"pathtracing";
    node.append_child(L"method_secondary").text() = L"pathtracing";
    node.append_child(L"method_tertiary").text()  = L"pathtracing";
    node.append_child(L"method_caustic").text()   = L"pathtracing";
    node.append_child(L"shadows").text()          = L"1";

    node.append_child(L"trace_depth").text()      = L"8";
    node.append_child(L"diff_trace_depth").text() = L"4";
    
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

  float matrixT[4][4], matrixT3[4][4], matrixT4[4][4];
  float mRot1[4][4], mTranslate[4][4], mRes[4][4];

  float mTranslateDown[4][4], mRes2[4][4];


  hrSceneOpen(scnRef, HR_WRITE_DISCARD);


  int mmIndex = 0;
  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, -1.5f, -5.0f + 5.0f);
  mat4x4_rotate_X(mRot1, mRot1, 0.25f*rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, -rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT, mRes); // this fucking math library swap rows and columns

  mat4x4_identity(mRot1);
  mat4x4_identity(mRes);
  mat4x4_rotate_Y(mRes, mRot1, rquad*DEG_TO_RAD);
  mat4x4_translate(mTranslateDown, -2.0f, 1.0f, -4.0f + 5.0f);
  mat4x4_mul(mRes2, mTranslateDown, mRes);
  mat4x4_transpose(matrixT3, mRes2);

  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 2.0f, 1.25f, -5.0f + 5.0f);
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
    else if (info.haveUpdateMSG)
    {
      std::wcout << L"msg = " << info.msg << " \r";
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_38/z_out.png");
  hrRenderSaveFrameBufferHDR(renderRef, L"tests_images/test_38/z_out.hdr");

  return check_images("test_38", 1, 25.0f);
}

#include "Timer.h"

bool test39_mmlt_or_ibpt()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_39");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_39", HR_WRITE_DISCARD);

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

    xml_node diff    = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"0.207843 0.188235 0");

    xml_node refl = matNode.append_child(L"reflectivity");

    refl.append_attribute(L"brdf_type").set_value(L"phong");
    refl.append_child(L"color").text().set(L"0.367059 0.345882 0");
    refl.append_child(L"glossiness").text().set(L"0.5");
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
    intensityNode.append_child(L"multiplier").text().set(L"10.0");
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
  HRRenderRef renderRef = hrRenderCreate(L"HydraModern"); // opengl1 // 

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  auto pList = hrRenderGetDeviceList(renderRef);

  while (pList != nullptr)
  {
    std::wcout << L"device id = " << pList->id << L", name = " << pList->name << L", driver = " << pList->driver << std::endl;
    pList = pList->next;
  }

  // hrRenderEnableDevice(renderRef, 0, true);
  // hrRenderEnableDevice(renderRef, 1, true); // CURR_RENDER_DEVICE

  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true); //

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);

    node.append_child(L"width").text()  = 1024;
    node.append_child(L"height").text() = 768;

    node.append_child(L"method_primary").text()   = L"pathtracing"; // L"pathtracing"; // L"lighttracing"; // IBPT
    node.append_child(L"method_secondary").text() = L"mmlt"; // L"mmlt";
    node.append_child(L"trace_depth").text()      = 6;
    node.append_child(L"diff_trace_depth").text() = 3;
    node.append_child(L"maxRaysPerPixel").text()  = 1024;
    node.append_child(L"evalgbuffer").text()      = 0;
  
    node.append_child(L"mmlt_burn_iters").text()     = 256;    // [1024, 2048, 4096, 8192, 16384, 65536]
    node.append_child(L"mmlt_threads").text()        = 524288; // [524288, 262144, 131072(?), 65536, 16384]
    
    node.append_child(L"mlt_med_threshold").text()   = 0.0f;
    node.append_child(L"mlt_med_enable").text()      = 1;
    
    //node.append_child(L"offline_pt").text()       = 1;
  }
  hrRenderClose(renderRef);
  
  //hrRenderLogDir(renderRef, L"/home/frol/hydra/", true);
  
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
  mat4x4_translate(mTranslate, 0, 3.85f, 0);
  mat4x4_transpose(matrixT, mTranslate);
  hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef);

  glViewport(0, 0, 1024, 768);
  std::vector<int32_t> image(1024 * 768);
  
  Timer timer(true);
  
  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
  
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
  
  auto oldPrecision = std::cout.precision(3);
  std::cout << "[test_39], rendering time = " << timer.getElapsed() << "s " << std::endl;
  std::cout.precision(oldPrecision);
  
  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_39/z_out.bmp");
  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_39/z_out.png");
  hrRenderSaveFrameBufferHDR(renderRef, L"tests_images/test_39/z_out.hdr");

  return check_images("test_39", 1, 40.0f);
}

void test_device_list()
{
  hrSceneLibraryOpen(L"scenelib", HR_WRITE_DISCARD);

  HRRenderRef renderRef = hrRenderCreate(L"HydraModern");

  auto pList = hrRenderGetDeviceList(renderRef);

  while (pList != nullptr)
  {
    std::wcout << L"id: " << pList->id << L", name: " << pList->name << L", driver: " << pList->driver << std::endl;
    pList = pList->next;
  }

  //hrRenderEnableDevice(renderRef, 0, true);
  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
}

bool test40_several_changes()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_40");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_40", HR_WRITE_DISCARD);

  SimpleMesh cube     = CreateCube(0.75f);
  SimpleMesh plane    = CreatePlane(10.0f);
  SimpleMesh sphere   = CreateSphere(1.0f, 32);
  SimpleMesh torus    = CreateTorus(0.35f, 1.5f, 32, 32);
  SimpleMesh cubeOpen = CreateCubeOpen(4.0f);

  for (size_t i = 0; i < plane.vTexCoord.size(); i++)
    plane.vTexCoord[i] *= 2.0f;


  HRTextureNodeRef testTex2 = hrTexture2DCreateFromFileDL(L"data/textures/chess_white.bmp");

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

  //HRMeshRef teapotRef = hrMeshCreateFromFile(L"data/meshes/teapot.vsgf");
  //std::vector<HRMeshRef> ref_vec = hrMeshCreateFromObj(L"data/meshes/bunnycube.obj");
  HRMeshRef teapotRef = hrMeshCreateFromObjMerged(L"data/meshes/bunnycube.obj");




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
    intensityNode.append_child(L"multiplier").text().set(L"10.0");
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
  HRRenderRef renderRef = hrRenderCreate(L"HydraModern"); // opengl1 // 

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  auto pList = hrRenderGetDeviceList(renderRef);

  while (pList != nullptr)
  {
    std::wcout << L"device id = " << pList->id << L", name = " << pList->name << L", driver = " << pList->driver << std::endl;
    pList = pList->next;
  }

  //hrRenderEnableDevice(renderRef, 0, true);
  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);

  hrRenderLogDir(renderRef, L"/home/denispavlov/hydra/logs/", true);

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

    node.append_child(L"trace_depth").text()      = 5;
    node.append_child(L"diff_trace_depth").text() = 3;
    node.append_child(L"maxRaysPerPixel").text()  = 1024;

    // #{CUSTOM EXE PATH}:
    //node.append_child(L"render_exe_dir").text() = L"E:/PROG/hydratest"; // copy all files from "C:/[Hydra]/bin2" to "E:/PROG/hydratest". Don't create "E:/PROG/hydratest/bin2". 
  }
  hrRenderClose(renderRef);

  // #{CUSTOM EXE PATH}:
  //hrRenderLogDir(renderRef, L"E:/PROG/hydratest/logs", false);          // we also fix logs folder path. it if does not end with "/", we add "/" inside hrRenderLogDir

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

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  int mmIndex = 0;
  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, -0.70f*3.65f, -5.0f + 5.0f);
  mat4x4_scale(mRot1, mRot1, 3.65f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT, mRes); // this fucking math library swaps rows and columns
  matrixT[3][3] = 1.0f;

  hrMeshInstance(scnRef, teapotRef, &matrixT[0][0]);
  //hrMeshInstance(scnRef, ref_vec[1], &matrixT[0][0]);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_40/z_out.png");
  hrRenderSaveFrameBufferHDR(renderRef, L"tests_images/test_40/z_out.exr");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrMaterialOpen(mat1, HR_OPEN_EXISTING);
  {
    xml_node matNode = hrMaterialParamNode(mat1);
    xml_node refl    = matNode.child(L"reflectivity");

    refl.child(L"color").text().set(L"0.367059 0.345882 0.35f");
    hrTextureBind(testTex2, refl);
  }
  hrMaterialClose(mat1);

  hrFlush(scnRef, renderRef);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_40/z_out2.png");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

  hrMaterialOpen(mat2, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat2);
    matNode.append_child(L"emission").append_child(L"color").text() = L"4 4 4";
  }
  hrMaterialClose(mat2);

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
  //hrMeshInstance(scnRef, ref_vec[1], &matrixT[0][0]);

  mat4x4_identity(mRot1);
  mat4x4_rotate_Y(mRot1, mRot1, 180.0f*DEG_TO_RAD);
  //mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD);
  mat4x4_transpose(matrixT, mRot1);
  hrMeshInstance(scnRef, cubeOpenRef, &matrixT[0][0]);


  mat4x4_translate(mTranslate, 0.0f, 0.5f, -5.0f + 5.0f);
  mat4x4_rotate_X(mRot1, mRot1, 60.0f*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, 15.0f*DEG_TO_RAD);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT, mRes); // this fucking math library swap rows and columns
  hrMeshInstance(scnRef, torusRef, &matrixT[0][0]);

  /////////////////////////////////////////////////////////////////////// instance light (!!!)

  mat4x4_identity(mTranslate);
  mat4x4_translate(mTranslate, 0, 3.85f, 0);
  mat4x4_transpose(matrixT, mTranslate);
  hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

  hrSceneClose(scnRef);

  hrRenderOpen(renderRef, HR_OPEN_EXISTING);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);
    node.child(L"maxRaysPerPixel").text() = 2048;
  }
  hrRenderClose(renderRef);


  hrFlush(scnRef, renderRef);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_40/z_out3.png");

  return check_images("test_40", 3, 20.0f);
}
