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

#include "../hydra_api/HR_HDRImageTool.h"

using namespace TEST_UTILS;

bool test18_camera_move()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_18");

  HRCameraRef    camRef;
  HRSceneInstRef scnRef;
  HRRenderRef    settingsRef;

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_18", HR_WRITE_DISCARD);

  // material and textures
  //
  HRTextureNodeRef testTex2 = hrTexture2DCreateFromFile(L"data/textures/chess_red.bmp");

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  std::wstring rotationMatrixStr = L"0.707107 -0.707107 0 0.5 0.707107 0.707107 0 -0.207107 0 0 1 0 0 0 0 1";

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

    diff.append_child(L"sampler").append_child(L"matrix").text().set(rotationMatrixStr.c_str());
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
  SimpleMesh cube = CreateCube(0.75f);
  SimpleMesh plane = CreatePlane(2.0f);
  SimpleMesh sphere = CreateSphere(0.5f, 32);
  SimpleMesh torus = CreateTorus(0.2f, 0.5f, 32, 32);

  HRMeshRef cubeRef = hrMeshCreate(L"my_cube");
  HRMeshRef planeRef = hrMeshCreate(L"my_plane");
  HRMeshRef sphereRef = hrMeshCreate(L"my_sphere");
  HRMeshRef torusRef = hrMeshCreate(L"my_torus");

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

  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_18/z_out.png");

  // move camera
  //
  hrCameraOpen(camRef, HR_OPEN_EXISTING);
  {
    xml_node camNode = hrCameraParamNode(camRef);
    camNode.child(L"position").text().set(L"0 1 5");
  }
  hrCameraClose(camRef);

  hrFlush(scnRef, settingsRef, camRef);

  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_18/z_out2.png");


  // move camera
  //
  hrCameraOpen(camRef, HR_OPEN_EXISTING);
  {
    xml_node camNode = hrCameraParamNode(camRef);
    camNode.child(L"position").text().set(L"-2 2 3");
  }
  hrCameraClose(camRef);

  hrFlush(scnRef, settingsRef, camRef);

  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_18/z_out3.png");

  // move camera
  //
  hrCameraOpen(camRef, HR_OPEN_EXISTING);
  {
    xml_node camNode = hrCameraParamNode(camRef);
    camNode.child(L"position").text().set(L"2 4 3");
    camNode.child(L"look_at").text().set(L"0 0 -5");
  }
  hrCameraClose(camRef);

  hrFlush(scnRef, settingsRef, camRef);

  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_18/z_out4.png");

  return check_images("test_18", 4);
}


bool test19_material_change()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_19");

  HRCameraRef    camRef;
  HRSceneInstRef scnRef;
  HRRenderRef    settingsRef;

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_19", HR_WRITE_DISCARD);

  // material and textures
  //
  HRTextureNodeRef testTex2 = hrTexture2DCreateFromFile(L"data/textures/chess_red.bmp");

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  std::wstring rotationMatrixStr = L"0.707107 -0.707107 0 0.5 0.707107 0.707107 0 -0.207107 0 0 1 0 0 0 0 1";

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

    diff.append_child(L"sampler").append_child(L"matrix").text().set(rotationMatrixStr.c_str());
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
  SimpleMesh cube = CreateCube(0.75f);
  SimpleMesh plane = CreatePlane(2.0f);
  SimpleMesh sphere = CreateSphere(0.5f, 32);
  SimpleMesh torus = CreateTorus(0.2f, 0.5f, 32, 32);

  HRMeshRef cubeRef = hrMeshCreate(L"my_cube");
  HRMeshRef planeRef = hrMeshCreate(L"my_plane");
  HRMeshRef sphereRef = hrMeshCreate(L"my_sphere");
  HRMeshRef torusRef = hrMeshCreate(L"my_torus");

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
    camNode.append_child(L"look_at").text().set(L"0 0 -1");
    camNode.append_child(L"position").text().set(L"0 0 0");
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
  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_19/z_out.png");

  // frame 2
  //
  hrMaterialOpen(mat0, HR_OPEN_EXISTING);
  {
    xml_node matNode = hrMaterialParamNode(mat0);
    matNode.child(L"diffuse").child(L"color").text().set(L"0.25 0.25 0.95");
  }
  hrMaterialClose(mat0);

  hrFlush(scnRef, settingsRef);
  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_19/z_out2.png");

  // frame 3
  //
  hrMaterialOpen(mat1, HR_OPEN_EXISTING);
  {
    xml_node matNode = hrMaterialParamNode(mat1);
    matNode.child(L"diffuse").child(L"color").text().set(L"0.0 0.0 0.0");
    hrTextureBind(HRTextureNodeRef(),  matNode.child(L"diffuse")); // kill texture
  }
  hrMaterialClose(mat1);

  hrMaterialOpen(mat1, HR_OPEN_EXISTING); // test open several times between commits, we have to take the last one 
  {
    xml_node matNode = hrMaterialParamNode(mat1);
    matNode.child(L"diffuse").child(L"color").text().set(L"0.5 0.75 0.5");
    hrTextureBind(HRTextureNodeRef(), matNode.child(L"diffuse")); // kill texture
  }
  hrMaterialClose(mat1);

  hrFlush(scnRef, settingsRef);
  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_19/z_out3.png");

  bool noDups1 = check_all_duplicates(L"tests/test_19/statex_00002.xml");
  bool noDups2 = check_all_duplicates(L"tests/test_19/statex_00003.xml");

  return check_images("test_19", 3) && noDups1 && noDups2;
}


bool test20_mesh_change()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_20");

  HRCameraRef    camRef;
  HRSceneInstRef scnRef;
  HRRenderRef    settingsRef;

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_20", HR_WRITE_DISCARD);

  // material and textures
  //
  HRTextureNodeRef testTex2 = hrTexture2DCreateFromFile(L"data/textures/chess_red.bmp");

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  std::wstring rotationMatrixStr = L"0.707107 -0.707107 0 0.5 0.707107 0.707107 0 -0.207107 0 0 1 0 0 0 0 1";

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

    diff.append_child(L"sampler").append_child(L"matrix").text().set(rotationMatrixStr.c_str());
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
  SimpleMesh cube = CreateCube(0.75f);
  SimpleMesh plane = CreatePlane(2.0f);
  SimpleMesh sphere = CreateSphere(0.5f, 32);
  SimpleMesh torus = CreateTorus(0.2f, 0.5f, 32, 32);

  HRMeshRef cubeRef = hrMeshCreate(L"my_cube");
  HRMeshRef planeRef = hrMeshCreate(L"my_plane");
  HRMeshRef sphereRef = hrMeshCreate(L"my_sphere");
  HRMeshRef torusRef = hrMeshCreate(L"my_torus");

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
  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_20/z_out.png");

  hrMeshOpen(cubeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeRef, L"pos", &torus.vPos[0]);
    hrMeshVertexAttribPointer4f(cubeRef, L"norm", &torus.vNorm[0]);
    hrMeshVertexAttribPointer2f(cubeRef, L"texcoord", &torus.vTexCoord[0]);

    hrMeshMaterialId(cubeRef, mat0.id);
    hrMeshPrimitiveAttribPointer1i(cubeRef, L"mind", &torus.matIndices[0]);
    hrMeshAppendTriangles3(cubeRef, int32_t(torus.triIndices.size()), &torus.triIndices[0]);
  }
  hrMeshClose(cubeRef);

  hrFlush(scnRef, settingsRef);
  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_20/z_out2.png");

  hrMeshOpen(sphereRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(sphereRef, L"pos", &cube.vPos[0]);
    hrMeshVertexAttribPointer4f(sphereRef, L"norm", &cube.vNorm[0]);
    hrMeshVertexAttribPointer2f(sphereRef, L"texcoord", &cube.vTexCoord[0]);

    hrMeshMaterialId(sphereRef, 2);
    hrMeshAppendTriangles3(sphereRef, int(cube.triIndices.size()), &cube.triIndices[0]);
  }
  hrMeshClose(sphereRef);

  hrFlush(scnRef, settingsRef);
  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_20/z_out3.png");

  bool noDups1 = check_all_duplicates(L"tests/test_20/statex_00002.xml");
  bool noDups2 = check_all_duplicates(L"tests/test_20/statex_00003.xml");

  return check_images("test_20", 3) && noDups1 && noDups2;
}


bool test21_add_same_textures_from_file()
{
  hrErrorCallerPlace(L"test_21");
  hrSceneLibraryOpen(L"tests/test_21", HR_WRITE_DISCARD);

  HRTextureNodeRef testTex1 = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
  HRTextureNodeRef testTex2 = hrTexture2DCreateFromFile(L"data/textures/chess_red.bmp");
  HRTextureNodeRef testTex3 = hrTexture2DCreateFromFile(L"data/textures/163.jpg");
 
  HRTextureNodeRef testTex6 = hrTexture2DCreateFromFile(L"data/textures/163.jpg");
  HRTextureNodeRef testTex4 = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
  HRTextureNodeRef testTex5 = hrTexture2DCreateFromFile(L"data/textures/chess_red.bmp");

  const bool eq1 = (testTex1.id == testTex4.id);
  const bool eq2 = (testTex2.id == testTex5.id);
  const bool eq3 = (testTex3.id == testTex6.id);

  const bool ne1 = (testTex1.id != testTex2.id);
  const bool ne2 = (testTex2.id != testTex3.id);

  return eq1 && eq2 && eq3 && ne1 && ne2;
}

bool g_test22ErrorIsOk = false;

void ErrorCallBack3(const wchar_t* message, const wchar_t* callerPlace)
{
  std::wstring tmp(message);
  auto foundPos = tmp.find(L"failed to load");
  g_test22ErrorIsOk = (foundPos != std::wstring::npos);
}


bool test22_can_not_load_texture()
{
  initGLIfNeeded();

  g_test22ErrorIsOk = false;

  hrErrorCallerPlace(L"test_22");
  hrErrorCallback(ErrorCallBack3);

  HRCameraRef    camRef;
  HRSceneInstRef scnRef;
  HRRenderRef    settingsRef;

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_22", HR_WRITE_DISCARD);

  // geometry
  //
  HRMeshRef cubeRef  = HRMeshFromSimpleMesh(L"my_cube",   CreateCube(0.75f), 0);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane",  CreatePlane(2.0f), 1);
  HRMeshRef sphRef   = HRMeshFromSimpleMesh(L"my_sphere", CreateSphere(0.5f, 32), 2);
  HRMeshRef torRef   = HRMeshFromSimpleMesh(L"my_torus",  CreateTorus(0.2f, 0.5f, 32, 32), 0);

  // material and textures
  //
  HRTextureNodeRef testTex2 = hrTexture2DCreateFromFile(L"data/textures/dont_exists.bmp");

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
  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_22/z_out.png");

  hrErrorCallback(ErrorCallBack);

  return check_images("test_22") && (testTex2.id == 0) && g_test22ErrorIsOk;
}


bool test28_compute_normals()
{
	hrErrorCallerPlace(L"test_28");
	hrSceneLibraryOpen(L"tests/test_28", HR_WRITE_DISCARD);

	// geometry
	//
	SimpleMesh cube = CreateCube(0.65f);
	SimpleMesh plane = CreatePlane(10.0f);
	SimpleMesh sphere = CreateSphere(0.5f, 32);
	SimpleMesh torus = CreateTorus(0.3f, 0.6f, 32, 32);

	HRMeshRef cubeRef = hrMeshCreate(L"my_cube");
	HRMeshRef planeRef = hrMeshCreate(L"my_plane");
	HRMeshRef sphRef = hrMeshCreate(L"my_sphere");
	HRMeshRef torRef = hrMeshCreate(L"my_torus");

	bool cube_ok = false;
	bool plane_ok = false;
	bool sph_ok = false;
	bool torus_ok = false;


	hrMeshOpen(cubeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
	{
		hrMeshVertexAttribPointer4f(cubeRef, L"pos", &cube.vPos[0]);
		hrMeshVertexAttribPointer2f(cubeRef, L"texcoord", &cube.vTexCoord[0]);

		hrMeshAppendTriangles3(cubeRef, int(cube.triIndices.size()), &cube.triIndices[0]);
    cube_ok = (hrMeshGetAttribPointer(cubeRef, L"norm") != nullptr);
	}
	hrMeshClose(cubeRef);

	hrMeshOpen(planeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
	{
		hrMeshVertexAttribPointer4f(planeRef, L"pos", &plane.vPos[0]);
		hrMeshVertexAttribPointer2f(planeRef, L"texcoord", &plane.vTexCoord[0]);

		hrMeshAppendTriangles3(planeRef, int(plane.triIndices.size()), &plane.triIndices[0]);
    plane_ok = (hrMeshGetAttribPointer(planeRef, L"norm") != nullptr);
	}
	hrMeshClose(planeRef);

	hrMeshOpen(sphRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
	{
		hrMeshVertexAttribPointer4f(sphRef, L"pos", &sphere.vPos[0]);
		hrMeshVertexAttribPointer2f(sphRef, L"texcoord", &sphere.vTexCoord[0]);

		hrMeshAppendTriangles3(sphRef, int(sphere.triIndices.size()), &sphere.triIndices[0]);
    sph_ok = (hrMeshGetAttribPointer(sphRef, L"norm") != nullptr);
	}
	hrMeshClose(sphRef);

	hrMeshOpen(torRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
	{
		hrMeshVertexAttribPointer4f(torRef, L"pos", &torus.vPos[0]);
		hrMeshVertexAttribPointer2f(torRef, L"texcoord", &torus.vTexCoord[0]);

		hrMeshAppendTriangles3(torRef, int(torus.triIndices.size()), &torus.triIndices[0]);
    torus_ok = (hrMeshGetAttribPointer(torRef, L"norm") != nullptr);
	}
	hrMeshClose(torRef);

 	return cube_ok && plane_ok && sph_ok && torus_ok;
}

