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

#include "../hydra_api/HR_HDRImageTool.h"
#include "../hydra_api/HydraPostProcessAPI.h"
#include "../hydra_api/LiteMath.h"
#include "../hydra_api/HydraXMLHelpers.h"

using namespace TEST_UTILS;

extern GLFWwindow* g_window;

using HydraLiteMath::float2;
using HydraLiteMath::float3;
using HydraLiteMath::float4;

namespace hlm = HydraLiteMath;

bool MTL_TESTS::test_158_proc_dirt1()
{
  const int THIS_TEST_WIDTH  = 1024;
  const int THIS_TEST_HEIGHT = 1024;

  initGLIfNeeded();

  hrErrorCallerPlace(L"test_158");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests_f/test_158", HR_WRITE_DISCARD);

  SimpleMesh sphere   = CreateSphere(2.0f, 128);
  SimpleMesh cubeOpen = CreateCubeOpen(4.0f);
  SimpleMesh cube     = CreateCube(1.0f);

  // textures
  //
  HRTextureNodeRef texBitmap1 = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
  HRTextureNodeRef texBitmap2 = hrTexture2DCreateFromFile(L"data/textures/300px-Bump2.jpg");
  HRTextureNodeRef texProc    = hrTextureCreateAdvanced(L"proc", L"my_custom_faloff");
  HRTextureNodeRef texProc2   = hrTextureCreateAdvanced(L"proc", L"my_show_normals");
  HRTextureNodeRef texProc3   = hrTextureCreateAdvanced(L"proc", L"my_ao_test");
  HRTextureNodeRef texProc4   = hrTextureCreateAdvanced(L"proc", L"my_voronoi_test");
  HRTextureNodeRef texProc5   = hrTextureCreateAdvanced(L"proc", L"my_cavity_test");
  HRTextureNodeRef texProc6   = hrTextureCreateAdvanced(L"proc", L"my_dirt");

  hrTextureNodeOpen(texProc, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc);

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/mul_tex_coord.c";
    code_node.append_attribute(L"main") = L"userProc";
  }
  hrTextureNodeClose(texProc);

  hrTextureNodeOpen(texProc2, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc2);

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/noise3D.c";
    code_node.append_attribute(L"main") = L"main";
  }
  hrTextureNodeClose(texProc2);

  hrTextureNodeOpen(texProc3, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc3);

    // Occluded color(с текстурой).       // put them in source code later! this test is only for AO.
    // Unoccluded color(с текстурой).     // put them in source code later! this test is only for AO.
    // Falloff(0.1f - 10.0f) - гамма.     // put them in source code later! this test is only for AO.
    // 
    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/noise3D_mul_ao.c";
    code_node.append_attribute(L"main") = L"main";

    xml_node aoNode = texNode.append_child(L"ao");

    // Distribution: Corner, Edge, Both(Corner and Edge).
    //
    aoNode.append_attribute(L"length") = 1.25f;

    auto texAoLengthNode = hrTextureBind(texBitmap1, aoNode);
    {
      // set ao length texture params
    }

    // Distribution: Corner/Up, Edge/Down, Both/Both(Corner and Edge).
    //
    aoNode.append_attribute(L"hemisphere") = L"up";

    // Only for this object(on / off) - работает только для этого объекта.
    //
    aoNode.append_attribute(L"local") = 1;
  }
  hrTextureNodeClose(texProc3);

  hrTextureNodeOpen(texProc4, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc4);

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/blue_water.c";
    code_node.append_attribute(L"main") = L"main";
  }
  hrTextureNodeClose(texProc4);


  hrTextureNodeOpen(texProc5, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc5);

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/voronoi.c";
    code_node.append_attribute(L"main") = L"main";

    xml_node aoNode = texNode.append_child(L"ao");

    aoNode.append_attribute(L"length") = 0.5f;

    // Distribution: Corner/Up, Edge/Down, Both/Both(Corner and Edge).
    //
    aoNode.append_attribute(L"hemisphere") = L"down";

    // Only for this object(on / off) - работает только для этого объекта.
    //
    aoNode.append_attribute(L"local") = 1;
  }
  hrTextureNodeClose(texProc5);

  hrTextureNodeOpen(texProc6, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc6);

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/dirt_ao.c";
    code_node.append_attribute(L"main") = L"main";

    xml_node aoNode = texNode.append_child(L"ao");

    aoNode.append_attribute(L"length")     = 1.0f;
    aoNode.append_attribute(L"hemisphere") = L"up";
    aoNode.append_attribute(L"local")      = 0;
  }
  hrTextureNodeClose(texProc6);

  // other as usual in this test
  //
  HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
  
  HRMaterialRef mat1 = hrMaterialCreate(L"red");
  HRMaterialRef mat2 = hrMaterialCreate(L"green");
  HRMaterialRef mat3 = hrMaterialCreate(L"white");

  HRMaterialRef mat4 = hrMaterialCreate(L"cubemat1");
  HRMaterialRef mat5 = hrMaterialCreate(L"cubemat2");
  HRMaterialRef mat6 = hrMaterialCreate(L"cubemat3");

  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat0);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val") = L"0.5 0.5 0.5";
    colorNode.append_attribute(L"tex_apply_mode") = L"replace";

    auto texNode = hrTextureBind(texProc4, colorNode);
    texNode.append_attribute(L"input_gamma") = 1.0f;

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"phong");

    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(0);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(2.5);

    auto reflColor = refl.append_child(L"color");
    reflColor.append_attribute(L"val").set_value(L"0.5 0.5 0.5");

    auto texNodeR = hrTextureBind(texProc2, reflColor);
    //texNodeR.append_attribute(L"input_gamma") = 1.0f;
  }
  hrMaterialClose(mat0);

  hrMaterialOpen(mat1, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat1);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val") = L"0.5 0.0 0.0";
    colorNode.append_attribute(L"tex_apply_mode") = L"replace";

    auto texNode = hrTextureBind(texProc, colorNode);

    // not used currently #TODO: figure out what of theese we needed!
    //
    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);


    // proc texture sampler settings
    //
    xml_node p1 = texNode.append_child(L"arg");
    xml_node p2 = texNode.append_child(L"arg");
    xml_node p3 = texNode.append_child(L"arg");
    xml_node p4 = texNode.append_child(L"arg");

    p1.append_attribute(L"id") = 0;
    p1.append_attribute(L"name") = L"texId1";
    p1.append_attribute(L"type") = L"sampler2D";
    p1.append_attribute(L"size") = 1;
    p1.append_attribute(L"val") = texBitmap1.id;

    p2.append_attribute(L"id") = 1;
    p2.append_attribute(L"name") = L"texId2";
    p2.append_attribute(L"type") = L"sampler2D";
    p2.append_attribute(L"size") = 1;
    p2.append_attribute(L"val") = texBitmap2.id;

    p3.append_attribute(L"id") = 3;
    p3.append_attribute(L"name") = L"offset";
    p3.append_attribute(L"type") = L"float2";
    p3.append_attribute(L"size") = 1;
    p3.append_attribute(L"val") = L"0 0";

    p4.append_attribute(L"id") = 4;
    p4.append_attribute(L"name") = L"color";
    p4.append_attribute(L"type") = L"float4";
    p4.append_attribute(L"size") = 1;
    p4.append_attribute(L"val") = L"1 1 1 1";
  }
  hrMaterialClose(mat1);

  hrMaterialOpen(mat2, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat2);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.5 0.0");
  }
  hrMaterialClose(mat2);

  hrMaterialOpen(mat3, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat3);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val").set_value(L"0.5 0.5 0.5");

    auto texNode = hrTextureBind(texProc3, colorNode);
    texNode.append_attribute(L"input_gamma") = 1.0f;
  }
  hrMaterialClose(mat3);

  hrMaterialOpen(mat4, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat4);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val").set_value(L"0.5 0.5 0.5");

    auto texNode = hrTextureBind(texProc6, colorNode);
    texNode.append_attribute(L"input_gamma") = 2.2f;

    xml_node p1 = texNode.append_child(L"arg");
    xml_node p2 = texNode.append_child(L"arg");
    xml_node p3 = texNode.append_child(L"arg");
    xml_node p4 = texNode.append_child(L"arg");
    xml_node p5 = texNode.append_child(L"arg");

    p1.append_attribute(L"id")   = 0;
    p1.append_attribute(L"name") = L"colorHit";
    p1.append_attribute(L"type") = L"float3";
    p1.append_attribute(L"size") = 1;
    p1.append_attribute(L"val")  = L"1 0 1";

    p2.append_attribute(L"id")   = 1;
    p2.append_attribute(L"name") = L"texHit";
    p2.append_attribute(L"type") = L"sampler2D";
    p2.append_attribute(L"size") = 1;
    p2.append_attribute(L"val")  = -1;

    p3.append_attribute(L"id")   = 2;
    p3.append_attribute(L"name") = L"colorMiss";
    p3.append_attribute(L"type") = L"float3";
    p3.append_attribute(L"size") = 1;
    p3.append_attribute(L"val")  = L"1 1 1";

    p4.append_attribute(L"id")   = 3;
    p4.append_attribute(L"name") = L"texMiss";
    p4.append_attribute(L"type") = L"sampler2D";
    p4.append_attribute(L"size") = 1;
    p4.append_attribute(L"val")  = texBitmap1.id;

    p5.append_attribute(L"id")   = 4;
    p5.append_attribute(L"name") = L"faloffPower";
    p5.append_attribute(L"type") = L"float";
    p5.append_attribute(L"size") = 1;
    p5.append_attribute(L"val")  = 2.0f;

  }
  hrMaterialClose(mat4);

  hrMaterialOpen(mat5, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat5);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    
    auto colorNode = diff.append_child(L"color");
    colorNode.append_attribute(L"val").set_value(L"1 1 1");

    auto texNode = hrTextureBind(texProc6, colorNode);
    texNode.append_attribute(L"input_gamma") = 2.2f;

    xml_node p1 = texNode.append_child(L"arg");
    xml_node p2 = texNode.append_child(L"arg");
    xml_node p3 = texNode.append_child(L"arg");
    xml_node p4 = texNode.append_child(L"arg");
    xml_node p5 = texNode.append_child(L"arg");

    p1.append_attribute(L"id")   = 0;
    p1.append_attribute(L"name") = L"colorHit";
    p1.append_attribute(L"type") = L"float3";
    p1.append_attribute(L"size") = 1;
    p1.append_attribute(L"val")  = L"0 0 1";

    p2.append_attribute(L"id")   = 1;
    p2.append_attribute(L"name") = L"texHit";
    p2.append_attribute(L"type") = L"sampler2D";
    p2.append_attribute(L"size") = 1;
    p2.append_attribute(L"val")  = -1;

    p3.append_attribute(L"id")   = 2;
    p3.append_attribute(L"name") = L"colorMiss";
    p3.append_attribute(L"type") = L"float3";
    p3.append_attribute(L"size") = 1;
    p3.append_attribute(L"val")  = L"1 1 1";

    p4.append_attribute(L"id")   = 3;
    p4.append_attribute(L"name") = L"texMiss";
    p4.append_attribute(L"type") = L"sampler2D";
    p4.append_attribute(L"size") = 1;
    p4.append_attribute(L"val")  = -1;

    p5.append_attribute(L"id")   = 4;
    p5.append_attribute(L"name") = L"faloffPower";
    p5.append_attribute(L"type") = L"float";
    p5.append_attribute(L"size") = 1;
    p5.append_attribute(L"val")  = 1.0f;
  }
  hrMaterialClose(mat5);

  hrMaterialOpen(mat6, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat6);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto colorNode = diff.append_child(L"color");
    colorNode.append_attribute(L"val").set_value(L"1 1 1");

    
  }
  hrMaterialClose(mat6);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
  HRMeshRef planeRef    = hrMeshCreate(L"my_plane");
  HRMeshRef sphereRef   = hrMeshCreate(L"my_sphere");
  HRMeshRef cubeRef     = hrMeshCreate(L"my_cube");

  hrMeshOpen(cubeOpenRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeOpenRef, L"pos", &cubeOpen.vPos[0]);
    hrMeshVertexAttribPointer4f(cubeOpenRef, L"norm", &cubeOpen.vNorm[0]);
    hrMeshVertexAttribPointer2f(cubeOpenRef, L"texcoord", &cubeOpen.vTexCoord[0]);

    int cubeMatIndices[10] = { mat3.id, mat3.id, mat3.id, mat3.id, mat3.id, mat3.id, mat2.id, mat2.id, mat1.id, mat1.id };

    //hrMeshMaterialId(cubeRef, 0);
    hrMeshPrimitiveAttribPointer1i(cubeOpenRef, L"mind", cubeMatIndices);
    hrMeshAppendTriangles3(cubeOpenRef, int(cubeOpen.triIndices.size()), &cubeOpen.triIndices[0]);
  }
  hrMeshClose(cubeOpenRef);

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

  hrMeshOpen(cubeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeRef, L"pos", &cube.vPos[0]);
    hrMeshVertexAttribPointer4f(cubeRef, L"norm", &cube.vNorm[0]);
    hrMeshVertexAttribPointer2f(cubeRef, L"texcoord", &cube.vTexCoord[0]);

    hrMeshMaterialId(cubeRef, mat4.id);
    hrMeshAppendTriangles3(cubeRef, int(cube.triIndices.size()), &cube.triIndices[0]);
  }
  hrMeshClose(cubeRef);

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
    sizeNode.append_attribute(L"half_width") = 1.0f;

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
    camNode.append_child(L"position").text().set(L"0 0 14");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  // set up render settings
  //
  HRRenderRef renderRef = hrRenderCreate(L"HydraModern"); // opengl1
  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
  hrRenderLogDir(renderRef, L"C:/[Hydra]/logs/", true);


  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);

    node.append_child(L"width").text()  = THIS_TEST_WIDTH;
    node.append_child(L"height").text() = THIS_TEST_HEIGHT;

    node.append_child(L"method_primary").text()   = L"pathtracing";
    node.append_child(L"method_secondary").text() = L"pathtracing";
    node.append_child(L"method_tertiary").text()  = L"pathtracing";
    node.append_child(L"method_caustic").text()   = L"pathtracing";
    node.append_child(L"shadows").text() = L"1";

    node.append_child(L"trace_depth").text()      = 8;
    node.append_child(L"diff_trace_depth").text() = 4;
    node.append_child(L"maxRaysPerPixel").text()  = 1024;
  }
  hrRenderClose(renderRef);

  // create scene
  //
  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    // instance sphere and cornell box
    //
    auto mscale     = hlm::scale4x4(float3(1.0f, 1.0f, 1.0f));
    auto mrot1      = hlm::rotate_Y_4x4(-5.0f*DEG_TO_RAD);
    auto mtranslate = hlm::translate4x4(hlm::float3(0.0, -3.0f, -1));
    auto mtransform = mul(mtranslate, mul(mrot1, mscale)); 
    hrMeshInstance(scnRef, cubeRef, mtransform.L());

    int32_t remapList1[2] = { mat4.id, mat5.id }; 
    mscale     = hlm::scale4x4(float3(1.0f, 1.0f, 1.0f));
    mrot1      = hlm::rotate_Y_4x4(-10.0f*DEG_TO_RAD);
    mtranslate = hlm::translate4x4(hlm::float3(-2.5f, -3.0f, 1));
    mtransform = mul(mtranslate, mul(mrot1, mscale));
    hrMeshInstance(scnRef, cubeRef, mtransform.L(), remapList1, sizeof(remapList1) / sizeof(int32_t));

    int32_t remapList2[2] = { mat4.id, mat6.id };
    mscale     = hlm::scale4x4(float3(1.0f, 1.0f, 1.0f));
    mrot1      = hlm::rotate_Y_4x4(+15.0f*DEG_TO_RAD);
    mtranslate = hlm::translate4x4(hlm::float3(+2.5f, -3.0f, 2.0));
    mtransform = mul(mtranslate, mul(mrot1, mscale));
    hrMeshInstance(scnRef, cubeRef, mtransform.L(), remapList2, sizeof(remapList2) / sizeof(int32_t));

    auto mrot = hlm::rotate_Y_4x4(180.0f*DEG_TO_RAD);
    hrMeshInstance(scnRef, cubeOpenRef, mrot.L());

    //// instance light (!!!)
    //
    mtranslate = hlm::translate4x4(hlm::float3(0, 3.85f, 0));
    hrLightInstance(scnRef, rectLight, mtranslate.L());
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef, camRef);

  glViewport(0, 0, THIS_TEST_WIDTH, THIS_TEST_HEIGHT);
  std::vector<int32_t> image(THIS_TEST_WIDTH * THIS_TEST_HEIGHT);

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      hrRenderGetFrameBufferLDR1i(renderRef, THIS_TEST_WIDTH, THIS_TEST_HEIGHT, &image[0]);

      glDisable(GL_TEXTURE_2D);
      glDrawPixels(THIS_TEST_WIDTH, THIS_TEST_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);

      glfwSwapBuffers(g_window);
      glfwPollEvents();
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_158/z_out.png");

  return check_images("test_158", 1, 10);
}

