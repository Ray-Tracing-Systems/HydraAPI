#if defined(WIN32)
#define _USE_MATH_DEFINES // Make MS math.h define M_PI
#endif

#include "tests.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../hydra_api/OpenGLCoreProfileUtils.h"

#include <sstream>
#include <random>

#include <GLFW/glfw3.h>
#if defined(WIN32)
#pragma comment(lib, "glfw3dll.lib")
#else
#endif

#include "Timer.h"
#include "linmath.h"

#include "mesh_utils.h"
#include "input.h"
#include "Camera.h"

#include "../hydra_api/HydraXMLHelpers.h"
#include <algorithm>
using namespace HydraXMLHelpers;

extern Input       g_input;
extern GLFWwindow* g_window;
extern int         g_width;
extern int         g_height;
extern int         g_ssao;
extern int         g_lightgeo;

extern Camera         g_cam;
extern HRCameraRef    camRef;
extern HRRenderRef    renderRef;

static int            g_filling = 0;
static HRSceneInstRef scnRef;
static HRMeshRef      cubeRef;
static HRMeshRef      cubeOpenRef;
static HRMeshRef      planeRef;
static HRMeshRef      sphereRef;
static HRMeshRef      torusRef;
static HRMeshRef      lucyRef;
static HRMeshRef      flatCubeRef;
static HRMeshRef      buildingRef;
static HRMeshRef      groundRef;
static HRLightRef     pointLight;

static std::vector<HRLightRef> iesLights;

static int numLights = 512;
static std::vector<float3> lightPos;
static std::vector<float> lightSpeed;

void test02_init()
{
  hrErrorCallerPlace(L"init");

  SimpleMesh cube   = CreateCube(0.75f);
  SimpleMesh plane  = CreatePlane(10.0f);
  SimpleMesh sphere = CreateSphere(1.0f, 32);
  SimpleMesh torus  = CreateTorus(0.25f, 0.6f, 32, 32);

  SimpleMesh cubeOpen = CreateCubeOpen(4.0f);

  for (size_t i = 0; i < plane.vTexCoord.size(); i++)
    plane.vTexCoord[i] *= 2.0f;

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/zgl1_test_cube", HR_WRITE_DISCARD);

  HRTextureNodeRef testTex2 = hrTexture2DCreateFromFile(L"data/textures/chess_red.bmp"); //

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
    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.75 0.5");

    HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
    hrTextureBind(testTex, diff);
  }
  hrMaterialClose(mat0);

  hrMaterialOpen(mat1, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat1);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"1 1 1");

    hrTextureBind(testTex2, diff);
  }
  hrMaterialClose(mat1);

  hrMaterialOpen(mat2, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat2);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.75 0.75 0.75");

    HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/relief_wood.jpg");
    hrTextureBind(testTex, color);
  }
  hrMaterialClose(mat2);

  hrMaterialOpen(mat3, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat3);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.75 0.75 0.75");

    HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/163.jpg");
    hrTextureBind(testTex, color);
  }
  hrMaterialClose(mat3);

  hrMaterialOpen(mat4, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat4);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.1 0.1 0.75");

  }
  hrMaterialClose(mat4);

  hrMaterialOpen(mat5, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat5);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.75 0.75 0.25");


    HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
    hrTextureBind(testTex, color);
  }
  hrMaterialClose(mat5);


  hrMaterialOpen(mat6, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat6);
    xml_node diff    = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.0 0.0");
  }
  hrMaterialClose(mat6);

  hrMaterialOpen(mat7, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat7);
    xml_node diff    = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.0 0.5 0.0");
  }
  hrMaterialClose(mat7);

  hrMaterialOpen(mat8, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat8);
    xml_node diff    = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(mat8);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  cubeRef     = hrMeshCreate(L"my_cube");
  cubeOpenRef = hrMeshCreate(L"my_box");
  planeRef    = hrMeshCreate(L"my_plane");
  sphereRef   = hrMeshCreate(L"my_sphere");
  torusRef    = hrMeshCreate(L"my_torus");

  hrMeshOpen(cubeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeRef, L"pos", &cube.vPos[0]);
    hrMeshVertexAttribPointer4f(cubeRef, L"norm", &cube.vNorm[0]);
    hrMeshVertexAttribPointer2f(cubeRef, L"texcoord", &cube.vTexCoord[0]);

    int cubeMatIndices[12] = { mat8.id, mat8.id, mat8.id, mat8.id, mat8.id, mat8.id, mat7.id, mat7.id, mat6.id, mat6.id, mat5.id, mat5.id };

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

    int cubeMatIndices[10] = { mat8.id, mat8.id, mat8.id, mat8.id, mat8.id, mat8.id, mat7.id, mat7.id, mat6.id, mat6.id};

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

  camRef = hrCameraCreate(L"my camera");

  hrCameraOpen(camRef, HR_WRITE_DISCARD); // write camera parameters
  {
    xml_node camNode = hrCameraParamNode(camRef);

    camNode.append_child(L"fov").text().set(L"45");
    camNode.append_child(L"nearClipPlane").text().set(L"0.01");
    camNode.append_child(L"farClipPlane").text().set(L"100.0");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 1 15");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  hrCameraOpen(camRef, HR_OPEN_READ_ONLY);  // read camera parameters to g_cam
  {
    xml_node camNode = hrCameraParamNode(camRef);

    ReadFloat3(camNode.child(L"position"), &g_cam.pos.x);
    ReadFloat3(camNode.child(L"look_at"), &g_cam.lookAt.x);
    ReadFloat3(camNode.child(L"up"), &g_cam.up.x);
    g_cam.fov = ReadFloat(camNode.child(L"fov"));
  }
  hrCameraClose(camRef);

  // set up render settings
  //
  renderRef = hrRenderCreate(g_input.inputRenderName.c_str());

  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);

    node.force_child(L"width").text()       = g_width;
    node.force_child(L"height").text()      = g_height;
    node.force_child(L"draw_length").text() = 0.25f;
  }
  hrRenderClose(renderRef);

  // create scene
  //
  scnRef = hrSceneCreate(L"my scene");
}


//
void test02_draw(void)
{
  hrErrorCallerPlace(L"draw");

  static GLfloat	rtri = 25.0f; // Angle For The Triangle ( NEW )
  static GLfloat	rquad = 40.0f;
  static float    g_FPS = 60.0f;
  static int      frameCounter = 0;
#if defined WIN32
  static Timer    timer(true);
#endif
  const float DEG_TO_RAD = float(M_PI) / 180.0f;

  float matrixT[4][4], matrixT3[4][4], matrixT4[4][4];
  float mRot1[4][4], mTranslate[4][4], mRes[4][4];

  float mTranslateDown[4][4], mRes2[4][4];


  int sphereMMs[3][2] = { { 3, 4 },    { 4, 2 },    { 1, 3 } };
  int torusMMs[3][3]  = { { 0, 1, 2 }, { 1, 2, 3 }, { 3, 4, 1 } };

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  int mmIndex = 0;
  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, -1.5f, -1.0f);
  mat4x4_rotate_X(mRot1, mRot1, -rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, -rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT, mRes); // this fucking math library swap rows and columns

  mat4x4_identity(mRot1);
  mat4x4_identity(mRes);
  mat4x4_rotate_Y(mRes, mRot1, rquad*DEG_TO_RAD);
  mat4x4_translate(mTranslateDown, -2.5f, -1.5f, 0.0f);
  mat4x4_mul(mRes2, mTranslateDown, mRes);
  mat4x4_transpose(matrixT3, mRes2);

  mat4x4_identity(mRot1);
  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 3.0f, -1.25f, -1.0f);
  mat4x4_rotate_X(mRot1, mRot1, rquad*DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD*0.5f);
  mat4x4_mul(mRes, mTranslate, mRot1);
  mat4x4_transpose(matrixT4, mRes); // this fucking math library swap rows and columns

  hrMeshInstance(scnRef, cubeRef, &matrixT[0][0]);
  hrMeshInstance(scnRef, sphereRef, &matrixT3[0][0], &sphereMMs[mmIndex][0], 2);
  hrMeshInstance(scnRef, torusRef, &matrixT4[0][0], &torusMMs[mmIndex][0], 3);

  mat4x4_identity(mRot1);
  mat4x4_rotate_Y(mRot1, mRot1, 180.0f*DEG_TO_RAD);
  //mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD);
  mat4x4_transpose(matrixT, mRot1);
  hrMeshInstance(scnRef, cubeOpenRef, &matrixT[0][0]);
  
  hrSceneClose(scnRef);

  hrCameraOpen(camRef, HR_OPEN_EXISTING);
  {
    xml_node camNode = hrCameraParamNode(camRef);

    WriteFloat3(camNode.child(L"position"), &g_cam.pos.x);
    WriteFloat3(camNode.child(L"look_at"),  &g_cam.lookAt.x);
    WriteFloat3(camNode.child(L"up"),       &g_cam.up.x);
    WriteFloat(camNode.child(L"fov"), g_cam.fov);
  }
  hrCameraClose(camRef);

  hrRenderOpen(renderRef, HR_OPEN_EXISTING);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);

    node.force_child(L"draw_solid").text()    = 1;
    node.force_child(L"draw_wire").text()     = 0;
    node.force_child(L"draw_normals").text()  = 1;
    node.force_child(L"draw_tangents").text() = 1;
    node.force_child(L"draw_length").text()   = 0.01f;

    node.force_child(L"draw_axis").text()            = 0;
    node.force_child(L"axis_arrow_length").text()    = 1.0f;
    node.force_child(L"axis_arrow_thickness").text() = 0.075f;
  }
  hrRenderClose(renderRef);

  //hrFlush(scnRef, renderRef, camRef);
  hrCommit(scnRef, renderRef, camRef);

  // count fps 
  //
  const float coeff = 100.0f / fmax(g_FPS, 1.0f);
  rtri += coeff*0.2f;
  rquad -= coeff*0.15f;
#if defined WIN32
  if (frameCounter % 10 == 0)
  {
    std::stringstream out; out.precision(4);
    g_FPS = (10.0f / timer.getElapsed());
    out << "FPS = " << g_FPS;
    glfwSetWindowTitle(g_window, out.str().c_str());
    timer.start();
  }
#else
  glfwSetWindowTitle(g_window, "test");
#endif
  frameCounter++;
}



void test_gl32_001_init(void)
{

 // initGLIfNeeded();

  hrErrorCallerPlace(L"test_002");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests_f/test_gl32_001", HR_WRITE_DISCARD);

  HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/checker_8x8.gif");
  HRTextureNodeRef texOrn = hrTexture2DCreateFromFile(L"data/textures/yinyang.png");
  HRTextureNodeRef normalMap = hrTexture2DCreateFromFile(L"data/textures/normal_map.jpg");
  
  //the mesh we're going to load has material id attribute set to 1
  //so we first need to add the material with id 0 and then add material for the mesh
  HRMaterialRef mat0_unused = hrMaterialCreate(L"mat0_unused");
  HRMaterialRef mat1 = hrMaterialCreate(L"mat1");
  HRMaterialRef matPlastic = hrMaterialCreate(L"matPlastic");
  HRMaterialRef matPlasticMatte = hrMaterialCreate(L"matPlasticMatte");

  hrMaterialOpen(mat1, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(mat1);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.3 0.9 0.05");
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");
    hrTextureBind(texOrn, color);
    auto texNode = color.child(L"texture");
    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 5, 0, 0, 0,
                                0, 5, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"clamp");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"clamp");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"phong");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.0 1.0");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.95");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(16.0);

  }
  hrMaterialClose(mat1);

  hrMaterialOpen(matPlastic, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matPlastic);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.8 0.0 0.0");

    hrTextureBind(texChecker, color);

    auto texNode = color.child(L"texture");
    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"clamp");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"clamp");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"phong");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5);


    VERIFY_XML(matNode);
  }
  hrMaterialClose(matPlastic);

  hrMaterialOpen(matPlasticMatte, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matPlasticMatte);

    auto diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");


    auto refl = matNode.append_child(L"reflectivity");

    refl.append_attribute(L"brdf_type").set_value(L"phong");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.99");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5);


    auto displacement = matNode.append_child(L"displacement");
    auto heightNode = displacement.append_child(L"normal_map");

    displacement.append_attribute(L"type").set_value(L"normal_bump");

    auto invert = heightNode.append_child(L"invert");
    invert.append_attribute(L"x").set_value(0);
    invert.append_attribute(L"y").set_value(0);

    auto texNode2 = hrTextureBind(normalMap, heightNode);

    texNode2.append_attribute(L"matrix");
    float samplerMatrix2[16] = { 4, 0, 0, 0,
                                0, 4, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode2.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode2.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode2.append_attribute(L"input_gamma").set_value(1.0f);
    texNode2.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode2, L"matrix", samplerMatrix2);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matPlasticMatte);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  lucyRef = hrMeshCreateFromFileDL(L"data/meshes/lucy.vsgf");
  torusRef = TEST_UTILS::HRMeshFromSimpleMesh(L"torus", CreateTorus(0.8f, 2.0f, 64, 64), matPlastic.id);
  planeRef = TEST_UTILS::HRMeshFromSimpleMesh(L"my_plane", CreatePlane(64.0f), matPlasticMatte.id);
  flatCubeRef = TEST_UTILS::HRMeshFromSimpleMesh(L"flatCube", CreateCube(1.0f), matPlasticMatte.id);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Light
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  pointLight = hrLightCreate(L"my_point_light");

  hrLightOpen(pointLight, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(pointLight);

    lightNode.attribute(L"type").set_value(L"point");
    lightNode.attribute(L"shape").set_value(L"point");
    lightNode.attribute(L"distribution").set_value(L"uniform");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"8.0");
  }
  hrLightClose(pointLight);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Camera
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  camRef = hrCameraCreate(L"my camera");

  hrCameraOpen(camRef, HR_WRITE_DISCARD); // write camera parameters
  {
    xml_node camNode = hrCameraParamNode(camRef);

    camNode.append_child(L"fov").text().set(L"45");
    camNode.append_child(L"nearClipPlane").text().set(L"0.01");
    camNode.append_child(L"farClipPlane").text().set(L"100.0");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 1 15");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  hrCameraOpen(camRef, HR_OPEN_READ_ONLY);  // read camera parameters to g_cam
  {
    xml_node camNode = hrCameraParamNode(camRef);

    ReadFloat3(camNode.child(L"position"), &g_cam.pos.x);
    ReadFloat3(camNode.child(L"look_at"), &g_cam.lookAt.x);
    ReadFloat3(camNode.child(L"up"), &g_cam.up.x);
    g_cam.fov = ReadFloat(camNode.child(L"fov"));
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  renderRef = hrRenderCreate(g_input.inputRenderName.c_str());

  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);

    node.force_child(L"width").text()       = g_width;
    node.force_child(L"height").text()      = g_height;
    node.force_child(L"draw_length").text() = 0.25f;

    node.force_child(L"SSAO").text() = 1;
    node.force_child(L"lightgeo_mode").text() = 0;
  }
  hrRenderClose(renderRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Create scene
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  scnRef = hrSceneCreate(L"my scene");


}

void test_gl32_001_draw(void)
{
  hrErrorCallerPlace(L"draw");

  using namespace HydraLiteMath;

  float4x4 mRot;
  float4x4 mTranslate;
  float4x4 mScale;
  float4x4 mRes;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  ///////////

/*  mTranslate.identity();
  mRes.identity();

  mTranslate = translate4x4(float3(0.0f, -4.0f, 0.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, planeRef, mRes.L());*/

  ///////////

  mTranslate.identity();
  mRes.identity();
  mScale.identity();

  mTranslate = translate4x4(float3(0.0f, -4.0f, 0.0f));
  mScale = scale4x4(float3(64.0f, 0.5f, 64.0f));
  mRes = mul(mTranslate, mScale);

  hrMeshInstance(scnRef, flatCubeRef, mRes.L());


  ///////////

  constexpr float a = -18.0f;

  for(int i = 0; i < 10; ++i)
  {
    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    mTranslate = translate4x4(float3(a + float(i) * 4.0f, -3.5f, 2.0f));
    mRot = rotate_Y_4x4(5.0f * i);
    mRes = mul(mTranslate, mRot);

    hrMeshInstance(scnRef, lucyRef, mRes.L());

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    mTranslate = translate4x4(float3(a + float(i) * 4.0f, -3.5f, -2.0f));
    mRot = rotate_Y_4x4(6.0f * i);
    mRes = mul(mTranslate, mRot);

    hrMeshInstance(scnRef, lucyRef, mRes.L());

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    mTranslate = translate4x4(float3(a + float(i) * 4.0f, -3.5f, -6.0f));
    mRot = rotate_Y_4x4(7.0f * float(i));
    mRes = mul(mTranslate, mRot);

    hrMeshInstance(scnRef, lucyRef, mRes.L());
  }
  ///////////

  mTranslate.identity();
  mRes.identity();

  mTranslate = translate4x4(float3(0.0f, 7.0f, 0.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, torusRef, mRes.L());

  ///////////

  for(int i = 0; i < 10; ++i)
  {
    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(a + float(i) * 4.0f, 5.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, pointLight, mRes.L());
  }

  mTranslate.identity();
  mRes.identity();

  mTranslate = translate4x4(float3(0.0f, 5.0f, 0.0f));
  mRes = mul(mTranslate, mRes);

  hrLightInstance(scnRef, pointLight, mRes.L());

  ///////////

  hrSceneClose(scnRef);

  hrCameraOpen(camRef, HR_OPEN_EXISTING);
  {
    xml_node camNode = hrCameraParamNode(camRef);

    WriteFloat3(camNode.child(L"position"), &g_cam.pos.x);
    WriteFloat3(camNode.child(L"look_at"),  &g_cam.lookAt.x);
    WriteFloat3(camNode.child(L"up"),       &g_cam.up.x);
    WriteFloat(camNode.child(L"fov"), g_cam.fov);
  }
  hrCameraClose(camRef);

  hrRenderOpen(renderRef, HR_OPEN_EXISTING);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);

    node.force_child(L"width").text()       = g_width;
    node.force_child(L"height").text()      = g_height;

    node.force_child(L"SSAO").text() = g_ssao;
    node.force_child(L"lightgeo_mode").text() = g_lightgeo;
  }
  hrRenderClose(renderRef);

  hrCommit(scnRef, renderRef, camRef);

  glfwSetWindowTitle(g_window, "test");
}




// 
static void key(GLFWwindow* window, int k, int s, int action, int mods)
{
  if (action != GLFW_PRESS) return;

  switch (k) {
  case GLFW_KEY_Z:
    //if (mods & GLFW_MOD_SHIFT)
    //  view_rotz -= 5.0;
    //else
    //  view_rotz += 5.0;
    break;
  case GLFW_KEY_ESCAPE:
    glfwSetWindowShouldClose(window, GL_TRUE);
    break;
  case GLFW_KEY_UP:
    //view_rotx += 5.0;
    break;
  case GLFW_KEY_DOWN:
    //view_rotx -= 5.0;
    break;
  case GLFW_KEY_LEFT:
    //view_roty += 5.0;
    break;
  case GLFW_KEY_RIGHT:
    //view_roty -= 5.0;
    break;
  case GLFW_KEY_SPACE:
    if (action == GLFW_PRESS)
    {
      if (g_filling == 0)
      {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        g_filling = 1;
      }
      else
      {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        g_filling = 0;
      }
    }
    break;
  default:
    return;
  }
}


// new window size 
static void reshape(GLFWwindow* window, int width, int height)
{
  hrErrorCallerPlace(L"reshape");

  g_width  = width;
  g_height = height;

  hrRenderOpen(renderRef, HR_OPEN_EXISTING);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);
    node.force_child(L"width").text()  = g_width;
    node.force_child(L"height").text() = g_height;
  }
  hrRenderClose(renderRef);

  hrCommit(scnRef, renderRef);
}



void test02_simple_gl1_render(const wchar_t* a_drvName)
{
  if (!glfwInit())
  {
    fprintf(stderr, "Failed to initialize GLFW\n");
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_DEPTH_BITS, 24);

  g_window = glfwCreateWindow(800, 600, "OpenGL1 GLFW test", NULL, NULL);
  if (!g_window)
  {
    fprintf(stderr, "Failed to open GLFW window\n");
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  // Set callback functions
  glfwSetFramebufferSizeCallback(g_window, reshape);
  glfwSetKeyCallback(g_window, key);

  glfwMakeContextCurrent(g_window);
  //gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
  glfwSwapInterval(1);

  // Parse command-line options
  test02_init();

  glfwGetFramebufferSize(g_window, &g_width, &g_height);
  reshape(g_window, g_width, g_height);



  // Main loop
  while (!glfwWindowShouldClose(g_window))
  {
    test02_draw();

    // Swap buffers
    glfwSwapBuffers(g_window);
    glfwPollEvents();
  }

  // Terminate GLFW
  glfwTerminate();

  // Exit program
  exit(EXIT_SUCCESS);
}


void initGLIfNeeded()
{
  static bool firstCall = true;

  if (firstCall)
  {
    if (!glfwInit())
    {
      fprintf(stderr, "Failed to initialize GLFW\n");
      exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    g_window = glfwCreateWindow(g_width, g_height, "OpenGL1 GLFW test", NULL, NULL);
    if (!g_window)
    {
      fprintf(stderr, "Failed to open GLFW window\n");
      glfwTerminate();
      exit(EXIT_FAILURE);
    }

    // Set callback functions
    //glfwSetFramebufferSizeCallback(g_window, reshape);
    glfwSetKeyCallback(g_window, key);

    glfwMakeContextCurrent(g_window);
    //gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);

    glfwGetFramebufferSize(g_window, &g_width, &g_height);
    //reshape(g_window, g_width, g_height);

    firstCall = false;
  }
}


void test_gl32_002_init()
{
 // initGLIfNeeded();

  hrErrorCallerPlace(L"test_gl32_002");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_gl32_002", HR_WRITE_DISCARD);



  //HRTextureNodeRef testTex2 = hrTexture2DCreateFromFileDL(L"data/textures/chess_red.bmp");

  HRMaterialRef mat0 = hrMaterialCreate(L"unused");
  HRMaterialRef mat1 = hrMaterialCreate(L"white");
  HRMaterialRef mat2 = hrMaterialCreate(L"turquoise");
  HRMaterialRef mat3 = hrMaterialCreate(L"glass");
  HRMaterialRef mat4 = hrMaterialCreate(L"turquoise2");
  HRMaterialRef mat5 = hrMaterialCreate(L"fundament");
  HRMaterialRef mat6 = hrMaterialCreate(L"roof");
  HRMaterialRef mat7 = hrMaterialCreate(L"marble");
  HRMaterialRef mat8 = hrMaterialCreate(L"mat8");
  HRMaterialRef mat9 = hrMaterialCreate(L"mat9");
  HRMaterialRef mat10 = hrMaterialCreate(L"Gray");

  HRTextureNodeRef texTiles1 = hrTexture2DCreateFromFile(L"data/textures/tiles1.png");
  HRTextureNodeRef texTiles2 = hrTexture2DCreateFromFile(L"data/textures/tiles2.png");
  HRTextureNodeRef texTilesNormal1 = hrTexture2DCreateFromFile(L"data/textures/tiles1_normal.png");
  HRTextureNodeRef texTilesNormal2 = hrTexture2DCreateFromFile(L"data/textures/tiles2_noiseNormal.png");
  HRTextureNodeRef texTilesSpecular2 = hrTexture2DCreateFromFile(L"data/textures/tiles2_noiseSpec.png");

  HRTextureNodeRef texGroundDiff = hrTexture2DCreateFromFile(L"data/textures/tiles01_diffuse.png");
  HRTextureNodeRef texGroundNormal = hrTexture2DCreateFromFile(L"data/textures/tiles01_normal.png");

  hrMaterialOpen(mat1, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat1);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.85 0.85 0.85");


  }
  hrMaterialClose(mat1);

  hrMaterialOpen(mat2, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat2);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.45 0.84 0.81");
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

    hrTextureBind(texTiles1, color);

    auto texNode = color.append_child(L"texture");
    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"clamp");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"clamp");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    auto displacement = matNode.append_child(L"displacement");
    auto heightNode = displacement.append_child(L"normal_map");

    displacement.append_attribute(L"type").set_value(L"normal_bump");

    auto invert = heightNode.append_child(L"invert");
    invert.append_attribute(L"x").set_value(0);
    invert.append_attribute(L"y").set_value(0);

    auto texNode2 = hrTextureBind(texTilesNormal1, heightNode);

    texNode2.append_attribute(L"matrix");
    float samplerMatrix2[16] = { 1, 0, 0, 0,
                              0, 1, 0, 0,
                              0, 0, 1, 0,
                              0, 0, 0, 1 };

    texNode2.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode2.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode2.append_attribute(L"input_gamma").set_value(1.0f);
    texNode2.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode2, L"matrix", samplerMatrix2);

  }
  hrMaterialClose(mat2);

  hrMaterialOpen(mat3, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat3);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.05 0.05 0.05");

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"phong");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.9");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5);
  }
  hrMaterialClose(mat3);

  hrMaterialOpen(mat4, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat4);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.59 0.74 0.71");

  }
  hrMaterialClose(mat4);

  hrMaterialOpen(mat5, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat5);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.35 0.32 0.32");


  }
  hrMaterialClose(mat5);


  hrMaterialOpen(mat6, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat6);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.01 0.01 0.01");
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

  /*  hrTextureBind(texTiles2, color);

    auto texNode = color.append_child(L"texture");
    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"clamp");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"clamp");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);*/


    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"phong");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.8");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5);

    auto texNode3 = hrTextureBind(texTilesSpecular2, refl.child(L"color"));

    texNode3.append_attribute(L"matrix");
    float samplerMatrix3[16] = { 1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1 };

    texNode3.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode3.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode3.append_attribute(L"input_gamma").set_value(1.0f);
    texNode3.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode3, L"matrix", samplerMatrix3);


    auto displacement = matNode.append_child(L"displacement");
    auto heightNode = displacement.append_child(L"normal_map");

    displacement.append_attribute(L"type").set_value(L"normal_bump");

    auto invert = heightNode.append_child(L"invert");
    invert.append_attribute(L"x").set_value(0);
    invert.append_attribute(L"y").set_value(0);

    auto texNode2 = hrTextureBind(texTilesNormal2, heightNode);

    texNode2.append_attribute(L"matrix");
    float samplerMatrix2[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode2.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode2.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode2.append_attribute(L"input_gamma").set_value(1.0f);
    texNode2.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode2, L"matrix", samplerMatrix2);

  }
  hrMaterialClose(mat6);

  hrMaterialOpen(mat7, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat7);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.6 0.54 0.49");

  }
  hrMaterialClose(mat7);

  hrMaterialOpen(mat10, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat10);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");

    hrTextureBind(texGroundDiff, color);

    auto texNode = color.append_child(L"texture");
    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 256, 0, 0, 0,
                                0, 256, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"clamp");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"clamp");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);


    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"phong");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5);

    hrTextureBind(texGroundDiff, refl.child(L"color"));

    auto texNode3 = color.append_child(L"texture");
    texNode.append_attribute(L"matrix");
    float samplerMatrix3[16] = { 256, 0, 0, 0,
      0, 256, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"clamp");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"clamp");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode3, L"matrix", samplerMatrix3);


    auto displacement = matNode.append_child(L"displacement");
    auto heightNode = displacement.append_child(L"normal_map");

    displacement.append_attribute(L"type").set_value(L"normal_bump");

    auto invert = heightNode.append_child(L"invert");
    invert.append_attribute(L"x").set_value(0);
    invert.append_attribute(L"y").set_value(0);

    auto texNode2 = hrTextureBind(texGroundNormal, heightNode);

    texNode2.append_attribute(L"matrix");
    float samplerMatrix2[16] = { 256, 0, 0, 0,
      0, 256, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1 };

    texNode2.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode2.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode2.append_attribute(L"input_gamma").set_value(1.0f);
    texNode2.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode2, L"matrix", samplerMatrix2);

  }
  hrMaterialClose(mat10);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  buildingRef = hrMeshCreateFromFileDL(L"data/meshes/vokzal.vsgf");
  groundRef = hrMeshCreateFromFileDL(L"data/meshes/vokzal_ground.vsgf");
  //HRMeshRef lightpole = hrMeshCreateFromFileDL(L"data/meshes/lightpole.vsgf");
  SimpleMesh plane = CreateCubeOpen(1.0f);
  planeRef = hrMeshCreate(L"my_plane");//TEST_UTILS::HRMeshFromSimpleMesh(L"my_plane", CreatePlane(64.0f), mat10.id);

  hrMeshOpen(planeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
   // std::for_each(plane.vNorm.begin(), plane.vNorm.end(), [](float &x) { x *= -1.0f; });
    hrMeshVertexAttribPointer4f(planeRef, L"pos", &plane.vPos[0]);
    hrMeshVertexAttribPointer4f(planeRef, L"norm", &plane.vNorm[0]);
    hrMeshVertexAttribPointer2f(planeRef, L"texcoord", &plane.vTexCoord[0]);

    hrMeshMaterialId(planeRef, mat10.id);
    hrMeshAppendTriangles3(planeRef, int32_t(plane.triIndices.size()), &plane.triIndices[0]);
  }
  hrMeshClose(planeRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  pointLight = hrLightCreate(L"my_point_light");

  hrLightOpen(pointLight, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(pointLight);

    lightNode.attribute(L"type").set_value(L"point");
    lightNode.attribute(L"shape").set_value(L"point");
    lightNode.attribute(L"distribution").set_value(L"uniform");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.5");
  }
  hrLightClose(pointLight);

  iesLights.resize(7);

  iesLights.at(0) = hrLightCreate(L"ies1");

  hrLightOpen(iesLights.at(0), HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(iesLights.at(0));

    lightNode.attribute(L"type").set_value(L"point");
    lightNode.attribute(L"shape").set_value(L"point");
    lightNode.attribute(L"distribution").set_value(L"ies");

    auto distribution = lightNode.append_child(L"ies");
    distribution.append_attribute(L"data").set_value(L"data/ies/ies_5.ies");

    distribution.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    HydraXMLHelpers::WriteMatrix4x4(distribution, L"matrix", samplerMatrix);

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.1 0.1 0.99");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"6.0");

  }
  hrLightClose(iesLights.at(0));

  iesLights.at(1) = hrLightCreate(L"ies2");

  hrLightOpen(iesLights.at(1), HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(iesLights.at(1));

    lightNode.attribute(L"type").set_value(L"point");
    lightNode.attribute(L"shape").set_value(L"point");
    lightNode.attribute(L"distribution").set_value(L"ies");

    auto distribution = lightNode.append_child(L"ies");
    distribution.append_attribute(L"data").set_value(L"data/ies/ies_10.ies");

    distribution.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    HydraXMLHelpers::WriteMatrix4x4(distribution, L"matrix", samplerMatrix);

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.45 0.1 0.99");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"4.0");

  }
  hrLightClose(iesLights.at(1));

  iesLights.at(2) = hrLightCreate(L"ies3");

  hrLightOpen(iesLights.at(2), HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(iesLights.at(2));

    lightNode.attribute(L"type").set_value(L"point");
    lightNode.attribute(L"shape").set_value(L"point");
    lightNode.attribute(L"distribution").set_value(L"ies");

    auto distribution = lightNode.append_child(L"ies");
    distribution.append_attribute(L"data").set_value(L"data/ies/ies_11.ies");

    distribution.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    HydraXMLHelpers::WriteMatrix4x4(distribution, L"matrix", samplerMatrix);

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.95 0.4 0.01");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"3.5");

  }
  hrLightClose(iesLights.at(2));

  iesLights.at(3) = hrLightCreate(L"ies4");

  hrLightOpen(iesLights.at(3), HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(iesLights.at(3));

    lightNode.attribute(L"type").set_value(L"point");
    lightNode.attribute(L"shape").set_value(L"point");
    lightNode.attribute(L"distribution").set_value(L"ies");

    auto distribution = lightNode.append_child(L"ies");
    distribution.append_attribute(L"data").set_value(L"data/ies/ies_12.ies");

    distribution.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    HydraXMLHelpers::WriteMatrix4x4(distribution, L"matrix", samplerMatrix);

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.56 0.9 0.01");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"6.0");

  }
  hrLightClose(iesLights.at(3));

  iesLights.at(4) = hrLightCreate(L"ies5");

  hrLightOpen(iesLights.at(4), HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(iesLights.at(4));

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

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.1 0.7 0.8");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"3.5");

  }
  hrLightClose(iesLights.at(4));


  iesLights.at(5) = hrLightCreate(L"ies6");

  hrLightOpen(iesLights.at(5), HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(iesLights.at(5));

    lightNode.attribute(L"type").set_value(L"point");
    lightNode.attribute(L"shape").set_value(L"point");
    lightNode.attribute(L"distribution").set_value(L"ies");

    auto distribution = lightNode.append_child(L"ies");
    distribution.append_attribute(L"data").set_value(L"data/ies/111621PN.IES");

    distribution.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    HydraXMLHelpers::WriteMatrix4x4(distribution, L"matrix", samplerMatrix);

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.1 0.7 0.1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"4.0");

  }
  hrLightClose(iesLights.at(5));


  iesLights.at(6) = hrLightCreate(L"ies7");

  hrLightOpen(iesLights.at(6), HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(iesLights.at(6));

    lightNode.attribute(L"type").set_value(L"point");
    lightNode.attribute(L"shape").set_value(L"point");
    lightNode.attribute(L"distribution").set_value(L"ies");

    auto distribution = lightNode.append_child(L"ies");
    distribution.append_attribute(L"data").set_value(L"data/ies/APWP3T8.IES");

    distribution.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    HydraXMLHelpers::WriteMatrix4x4(distribution, L"matrix", samplerMatrix);

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.2 0.1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"4.5");

  }
  hrLightClose(iesLights.at(6));


  std::vector<float3> lightColor;

  GL_RENDER_DRIVER_UTILS::CreateRandomLights(numLights, lightPos, lightColor);

  std::random_device rand;
  std::mt19937 rng(rand());
  std::uniform_real_distribution<float> speedDist(0.4f, 1.0f);

  for(int i = 0; i < numLights; ++i)
  {
    lightSpeed.push_back(speedDist(rng));
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  camRef = hrCameraCreate(L"my camera");

  hrCameraOpen(camRef, HR_WRITE_DISCARD); // write camera parameters
  {
    xml_node camNode = hrCameraParamNode(camRef);

    camNode.append_child(L"fov").text().set(L"45");
    camNode.append_child(L"nearClipPlane").text().set(L"0.01");
    camNode.append_child(L"farClipPlane").text().set(L"100.0");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"48.07 2.98 6.07");
    camNode.append_child(L"look_at").text().set(L"-7.99 -9.32 -75");
  }
  hrCameraClose(camRef);

  hrCameraOpen(camRef, HR_OPEN_READ_ONLY);  // read camera parameters to g_cam
  {
    xml_node camNode = hrCameraParamNode(camRef);

    ReadFloat3(camNode.child(L"position"), &g_cam.pos.x);
    ReadFloat3(camNode.child(L"look_at"), &g_cam.lookAt.x);
    ReadFloat3(camNode.child(L"up"), &g_cam.up.x);
    g_cam.fov = ReadFloat(camNode.child(L"fov"));
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  renderRef = hrRenderCreate(g_input.inputRenderName.c_str());

  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);

    node.force_child(L"width").text()       = g_width;
    node.force_child(L"height").text()      = g_height;
    node.force_child(L"draw_length").text() = 0.25f;

    node.force_child(L"SSAO").text() = 1;
    node.force_child(L"lightgeo_mode").text() = 0;

  }
  hrRenderClose(renderRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Create scene
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  scnRef = hrSceneCreate(L"my scene");
}

void test_gl32_002_draw()
{
  static GLfloat	rquad = 40.0f;
  static float    g_FPS = 60.0f;
  static int      frameCounter = 0;

  const float DEG_TO_RAD = float(M_PI) / 180.0f;


  float4x4 mRot;
  float4x4 mTranslate;
  float4x4 mScale;
  float4x4 mRes;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  mRes.identity();

  ///////////

  mTranslate = translate4x4(float3(100.0f, -0.0f, -30.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, buildingRef, mRes.L());

  ///////////

  mRes.identity();
  mTranslate.identity();
  mScale.identity();

  mTranslate = translate4x4(float3(0.0f, -11.6f, 0.0f));
  mScale = scale4x4(float3(128.0f, 1.0f, 128.0f));
  mRes = mul(mTranslate, mScale);
  hrMeshInstance(scnRef, planeRef, mRes.L());

  ///////////

 /* mRes.identity();
  mTranslate.identity();

  mTranslate = translate4x4(float3(-2.0f, 8.0f, 10.0f));

  hrLightInstance(scnRef, iesLights.at(6), mRes.L());*/

  for(int i = 0; i < lightPos.size(); ++i)
  {
    mRes.identity();
    mTranslate.identity();
    mRot.identity();

    float4x4 mTranslate2;

    mRot = rotate_Y_4x4(-rquad * DEG_TO_RAD * lightSpeed.at(i));
    mTranslate2 = translate4x4(float3(0.0f, 3.0f + 4.75f * sinf(-rquad * DEG_TO_RAD * lightSpeed.at(i) * 20.0f), 0.0f));

    mTranslate = translate4x4(lightPos.at(i));
    mRes = mul(mTranslate2, mTranslate);
    mRes = mul(mRot, mRes);
    hrLightInstance(scnRef, iesLights.at(i % iesLights.size()), mRes.L());
   // hrLightInstance(scnRef, pointLight, mRes.L());
  }

  ///////////

  hrSceneClose(scnRef);

  hrCameraOpen(camRef, HR_OPEN_EXISTING);
  {
    xml_node camNode = hrCameraParamNode(camRef);

    WriteFloat3(camNode.child(L"position"), &g_cam.pos.x);
    WriteFloat3(camNode.child(L"look_at"),  &g_cam.lookAt.x);
    WriteFloat3(camNode.child(L"up"),       &g_cam.up.x);
    WriteFloat(camNode.child(L"fov"), g_cam.fov);
  }
  hrCameraClose(camRef);

  hrRenderOpen(renderRef, HR_OPEN_EXISTING);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);

    node.force_child(L"width").text()       = g_width;
    node.force_child(L"height").text()      = g_height;

    node.force_child(L"SSAO").text() = g_ssao;
    node.force_child(L"lightgeo_mode").text() = g_lightgeo;
  }
  hrRenderClose(renderRef);

  hrCommit(scnRef, renderRef, camRef);

  const float coeff = 100.0f / fmax(g_FPS, 1.0f);
  rquad -= coeff*0.15f;

  glfwSetWindowTitle(g_window, "test");
}