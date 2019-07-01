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
#include "../hydra_api/HydraXMLHelpers.h"

#include "Timer.h"

#pragma warning(disable:4996)
using namespace TEST_UTILS;

extern GLFWwindow* g_window;

namespace hlm = HydraLiteMath;

bool test90_proc_tex_normalmap()
{
  initGLIfNeeded(512, 512);
  hrErrorCallerPlace(L"test_90");
  
  hrSceneLibraryOpen(L"tests/test_90", HR_WRITE_DISCARD);
  
  SimpleMesh sphere   = CreateSphere(2.0f, 128);
  SimpleMesh cubeOpen = CreateCubeOpen(4.0f);
  
  // textures
  //
  HRTextureNodeRef texBitmap1 = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
  HRTextureNodeRef texNormal  = hrTexture2DCreateFromFile(L"data/textures/normal_map2.jpg");
  
  HRTextureNodeRef texProc    = hrTextureCreateAdvanced(L"proc", L"my_custom_tex");
  HRTextureNodeRef texProc2   = hrTextureCreateAdvanced(L"proc", L"my_custom_tex2");
  HRTextureNodeRef texProcNM  = hrTextureCreateAdvanced(L"proc", L"my_custom_normalmap");
  
  hrTextureNodeOpen(texProc, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc);
    
    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/show_normals.c";
    code_node.append_attribute(L"main") = L"mainNorm";
  }
  hrTextureNodeClose(texProc);
  
  hrTextureNodeOpen(texProc2, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc2);
    
    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/blue_water.c";
    code_node.append_attribute(L"main") = L"main";
  }
  hrTextureNodeClose(texProc2);
  
  hrTextureNodeOpen(texProcNM, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProcNM);
    
    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/read_normal.c";
    code_node.append_attribute(L"main") = L"main";
  }
  hrTextureNodeClose(texProcNM);
  
  
  // other as usual in this test
  //
  HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
  HRMaterialRef mat1 = hrMaterialCreate(L"red");
  HRMaterialRef mat2 = hrMaterialCreate(L"green");
  HRMaterialRef mat3 = hrMaterialCreate(L"white");
  HRMaterialRef mat4 = hrMaterialCreate(L"normalmaptest");

  hrMaterialOpen(mat4, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(mat4);

    auto diffuse = matNode.append_child(L"diffuse");
    diffuse.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

    auto displacement = matNode.append_child(L"displacement");
    auto heightNode = displacement.append_child(L"normal_map");

    displacement.append_attribute(L"type").set_value(L"normal_bump");

    auto invert = heightNode.append_child(L"invert");
    invert.append_attribute(L"x").set_value(0);
    invert.append_attribute(L"y").set_value(0);

    auto texNode = hrTextureBind(texNormal, heightNode);

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 2, 0, 0, 0,
                                0, 2, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(1.0f);              // !!! this is important for normalmap !!!
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
    VERIFY_XML(matNode);
  }
  hrMaterialClose(mat4);

  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat0);
    
    auto emission = matNode.append_child(L"emission");
    {
      emission.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);
      
      auto color = emission.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.25 0.25 0.25");
      color.append_attribute(L"tex_apply_mode ").set_value(L"multiply");
      
      auto texNode = hrTextureBind(texProc2, color);
    }
    
    xml_node diff = matNode.append_child(L"diffuse");
    {
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      auto colorNode = diff.append_child(L"color");
      
      colorNode.append_attribute(L"val") = L"0.5 0.5 0.5";
      
      auto texNode = hrTextureBind(texBitmap1, colorNode);
      
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
    }
    
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


    // bind proc texture to diffuse slot
    {
      auto texNode = hrTextureBind(texProc, colorNode);
    
      // not used currently
      //
      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = {1, 0, 0, 0,
                                 0, 1, 0, 0,
                                 0, 0, 1, 0,
                                 0, 0, 0, 1};
    
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");
    
      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
    
      // proc texture sampler settings
      //
      xml_node p1 = texNode.append_child(L"arg");
      p1.append_attribute(L"id") = 0;
      p1.append_attribute(L"name") = L"inColor";
      p1.append_attribute(L"type") = L"float4";
      p1.append_attribute(L"size") = 1;
      p1.append_attribute(L"val") = L"1 1 1 1";
    }


    auto displacement = matNode.append_child(L"displacement");
    displacement.append_attribute(L"type").set_value(L"normal_bump");
    auto heightNode = displacement.append_child(L"normal_map");

    /*{
      auto invert = heightNode.append_child(L"invert");
      invert.append_attribute(L"x").set_value(0);
      invert.append_attribute(L"y").set_value(0);

      auto texNode = hrTextureBind(texNormal, heightNode);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 2, 0, 0, 0,
                                  0, 2, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(1.0f);              // !!! this is important for normalmap !!!
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
      VERIFY_XML(matNode);
    }*/


    {
      auto invert = heightNode.append_child(L"invert");
      invert.append_attribute(L"x").set_value(0);
      invert.append_attribute(L"y").set_value(0);

      //auto texNode = hrTextureBind(texNormal, heightNode);
      auto texNode = hrTextureBind(texProcNM, heightNode);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 2, 0, 0, 0,
                                  0, 2, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(1.0f);              // !!! this is important for normalmap !!!
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

      xml_node p1 = texNode.append_child(L"arg");
      p1.append_attribute(L"id")   = 0;
      p1.append_attribute(L"name") = L"texNorm";
      p1.append_attribute(L"type") = L"sampler2D";
      p1.append_attribute(L"size") = 1;
      p1.append_attribute(L"val")  = texNormal.id;
    }

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
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(mat3);
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
  HRMeshRef planeRef = hrMeshCreate(L"my_plane");
  HRMeshRef sphereRef = hrMeshCreate(L"my_sphere");
  
  hrMeshOpen(cubeOpenRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeOpenRef, L"pos", &cubeOpen.vPos[0]);
    hrMeshVertexAttribPointer4f(cubeOpenRef, L"norm", &cubeOpen.vNorm[0]);
    hrMeshVertexAttribPointer2f(cubeOpenRef, L"texcoord", &cubeOpen.vTexCoord[0]);
    
    int cubeMatIndices[10] = { mat3.id, mat3.id, mat3.id, mat3.id, mat0.id, mat0.id, mat2.id, mat2.id, mat1.id, mat1.id };
    
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
      sphere.matIndices[i] = mat1.id;
    
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
    sizeNode.append_attribute(L"half_width") = 1.0f;
    
    pugi::xml_node intensityNode = lightNode.append_child(L"intensity");
    
    intensityNode.append_child(L"color").append_attribute(L"val")      = L"1 1 1";
    intensityNode.append_child(L"multiplier").append_attribute(L"val") = 8.0f*IRRADIANCE_TO_RADIANCE;
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
  
  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);
    
    node.append_child(L"width").text()  = L"512";
    node.append_child(L"height").text() = L"512";
    
    node.append_child(L"method_primary").text()   = L"pathtracing";
    node.append_child(L"method_secondary").text() = L"pathtracing";
    node.append_child(L"method_tertiary").text()  = L"pathtracing";
    node.append_child(L"method_caustic").text()   = L"pathtracing";
    node.append_child(L"shadows").text()          = L"1";
    
    node.append_child(L"trace_depth").text()      = L"8";
    node.append_child(L"diff_trace_depth").text() = L"4";
    node.append_child(L"maxRaysPerPixel").text()  = 1024;
    node.append_child(L"evalgbuffer").text()      = 1;
  }
  hrRenderClose(renderRef);
  
  //hrRenderLogDir(renderRef, L"/home/frol/hydra/", true);
  
  // create scene
  //
  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");
  
  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;
  
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    // instance sphere and cornell box
    //
    auto mtranslate = hlm::translate4x4(hlm::float3(0, -2, 1));
    hrMeshInstance(scnRef, sphereRef, mtranslate.L());
    
    auto mrot = hlm::rotate_Y_4x4(180.0f*DEG_TO_RAD);
    hrMeshInstance(scnRef, cubeOpenRef, mrot.L());
    
    //// instance light (!!!)
    //
    mtranslate = hlm::translate4x4(hlm::float3(0, 3.85f, 0));
    hrLightInstance(scnRef, rectLight, mtranslate.L());
  }
  hrSceneClose(scnRef);
  
  hrFlush(scnRef, renderRef, camRef);
  
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
  
  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_90/z_out.png");
  hrRenderSaveGBufferLayerLDR(renderRef, L"tests_images/test_90/z_out2.png", L"diffcolor");
  
  return check_images("test_90", 1, 10);
}


bool test91_proc_tex_bump()
{
  hrErrorCallerPlace(L"test_91");
  
  hrSceneLibraryOpen(L"tests/test_91", HR_WRITE_DISCARD);
  
  SimpleMesh sphere   = CreateSphere(2.0f, 128);
  SimpleMesh cubeOpen = CreateCubeOpen(4.0f);
  
  // textures
  //
  HRTextureNodeRef texBitmap1 = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
  HRTextureNodeRef texHeight  = hrTexture2DCreateFromFile(L"data/textures/ornament.jpg");
  
  HRTextureNodeRef texProc    = hrTextureCreateAdvanced(L"proc", L"my_custom_tex");
  HRTextureNodeRef texProc2   = hrTextureCreateAdvanced(L"proc", L"my_custom_tex2");
  HRTextureNodeRef texProcNM  = hrTextureCreateAdvanced(L"proc", L"my_custom_normalmap");
  
  hrTextureNodeOpen(texProc, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc);
    
    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/mul_tex_coord2.c";
    code_node.append_attribute(L"main") = L"userProc";
  }
  hrTextureNodeClose(texProc);
  
  hrTextureNodeOpen(texProc2, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc2);
    
    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/blue_water.c";
    code_node.append_attribute(L"main") = L"main";
  }
  hrTextureNodeClose(texProc2);
  
  hrTextureNodeOpen(texProcNM, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProcNM);
    
    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/normal_from_height.c";
    code_node.append_attribute(L"main") = L"main";
  }
  hrTextureNodeClose(texProcNM);
  
  
  // other as usual in this test
  //
  HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
  HRMaterialRef mat1 = hrMaterialCreate(L"red");
  HRMaterialRef mat2 = hrMaterialCreate(L"green");
  HRMaterialRef mat3 = hrMaterialCreate(L"white");
  
  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat0);
    
    auto emission = matNode.append_child(L"emission");
    {
      emission.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);
      
      auto color = emission.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.25 0.25 0.25");
      color.append_attribute(L"tex_apply_mode ").set_value(L"multiply");
      
      auto texNode = hrTextureBind(texProc2, color);
    }
    
    xml_node diff = matNode.append_child(L"diffuse");
    {
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      auto colorNode = diff.append_child(L"color");
      
      colorNode.append_attribute(L"val") = L"0.5 0.5 0.5";
      
      auto texNode = hrTextureBind(texBitmap1, colorNode);
      
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
    }
    
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
    
    // bind proc texture to diffuse slot
    {
      auto texNode = hrTextureBind(texProc, colorNode);
      
      // not used currently
      //
      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = {1, 0, 0, 0,
                                 0, 1, 0, 0,
                                 0, 0, 1, 0,
                                 0, 0, 0, 1};
      
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");
      
      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
      
      // proc texture sampler settings
      //
      xml_node p1 = texNode.append_child(L"arg");
      p1.append_attribute(L"id") = 0;
      p1.append_attribute(L"name") = L"inColor";
      p1.append_attribute(L"type") = L"float4";
      p1.append_attribute(L"size") = 1;
      p1.append_attribute(L"val") = L"1 1 1 1";
    }
    
    // bind proc texture to normals slot
    {
      auto displacement = matNode.append_child(L"displacement");
      auto heightNode   = displacement.append_child(L"normal_map");
      
      displacement.append_attribute(L"type").set_value(L"normal_bump");
      
      auto invert = heightNode.append_child(L"invert");
      invert.append_attribute(L"x").set_value(0);
      invert.append_attribute(L"y").set_value(0);
      
      //auto nmTexNode = hrTextureBind(texNormal, heightNode);
      auto nmTexNode = hrTextureBind(texProcNM, heightNode);
      
      xml_node p1 = nmTexNode.append_child(L"arg");
      xml_node p2 = nmTexNode.append_child(L"arg");
      
      p1.append_attribute(L"id")   = 0;
      p1.append_attribute(L"name") = L"texHeight";
      p1.append_attribute(L"type") = L"sampler2D";
      p1.append_attribute(L"size") = 1;
      p1.append_attribute(L"val")  = texHeight.id;
  
      p2.append_attribute(L"id")   = 1;
      p2.append_attribute(L"name") = L"invTexRes";
      p2.append_attribute(L"type") = L"float2";
      p2.append_attribute(L"size") = 1;
      p2.append_attribute(L"val")  = L"0.002222222 0.002222222"; // (1.0f/texWidth 1.0f/texHeight)
    }
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
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(mat3);
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
  HRMeshRef planeRef = hrMeshCreate(L"my_plane");
  HRMeshRef sphereRef = hrMeshCreate(L"my_sphere");
  
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
    
    intensityNode.append_child(L"color").append_attribute(L"val") = L"1 1 1";
    intensityNode.append_child(L"multiplier").append_attribute(L"val") = 8.0f*IRRADIANCE_TO_RADIANCE;
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
  
  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);
    
    node.append_child(L"width").text()  = L"512";
    node.append_child(L"height").text() = L"512";
    
    node.append_child(L"method_primary").text()   = L"pathtracing";
    node.append_child(L"method_secondary").text() = L"pathtracing";
    node.append_child(L"method_tertiary").text()  = L"pathtracing";
    node.append_child(L"method_caustic").text()   = L"pathtracing";
    node.append_child(L"shadows").text()          = L"1";
    
    node.append_child(L"trace_depth").text()      = L"8";
    node.append_child(L"diff_trace_depth").text() = L"4";
    node.append_child(L"maxRaysPerPixel").text()  = 1024;
    node.append_child(L"evalgbuffer").text()      = 1;
  }
  hrRenderClose(renderRef);
  
  //hrRenderLogDir(renderRef, L"/home/frol/hydra/", true);
  
  // create scene
  //
  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");
  
  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;
  
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    // instance sphere and cornell box
    //
    auto mtranslate = hlm::translate4x4(hlm::float3(0, -2, 1));
    hrMeshInstance(scnRef, sphereRef, mtranslate.L());
    
    auto mrot = hlm::rotate_Y_4x4(180.0f*DEG_TO_RAD);
    hrMeshInstance(scnRef, cubeOpenRef, mrot.L());
    
    //// instance light (!!!)
    //
    mtranslate = hlm::translate4x4(hlm::float3(0, 3.85f, 0));
    hrLightInstance(scnRef, rectLight, mtranslate.L());
  }
  hrSceneClose(scnRef);
  
  hrFlush(scnRef, renderRef, camRef);
  
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
  
  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_91/z_out.png");
  hrRenderSaveGBufferLayerLDR(renderRef, L"tests_images/test_91/z_out2.png", L"diffcolor");
  
  return check_images("test_91", 2, 20);
}

bool test92_proc_tex_bump2()
{
  initGLIfNeeded();
  hrErrorCallerPlace(L"test_92");
  
  hrSceneLibraryOpen(L"tests/test_92", HR_WRITE_DISCARD);
  
  SimpleMesh sphere   = CreateSphere(2.0f, 128);
  SimpleMesh cubeOpen = CreateCubeOpen(4.0f);
  
  // textures
  //
  HRTextureNodeRef texBitmap1 = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
  HRTextureNodeRef texHeight  = hrTexture2DCreateFromFile(L"data/textures/ornament.jpg");
  
  HRTextureNodeRef texProc    = hrTextureCreateAdvanced(L"proc", L"my_custom_tex");
  HRTextureNodeRef texProc2   = hrTextureCreateAdvanced(L"proc", L"my_custom_tex2");
  HRTextureNodeRef texProcNM  = hrTextureCreateAdvanced(L"proc", L"my_custom_normalmap");
  
  hrTextureNodeOpen(texProc, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc);
    
    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/mul_tex_coord2.c";
    code_node.append_attribute(L"main") = L"userProc";
  }
  hrTextureNodeClose(texProc);
  
  hrTextureNodeOpen(texProc2, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc2);
    
    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/blue_water.c";
    code_node.append_attribute(L"main") = L"main";
  }
  hrTextureNodeClose(texProc2);
  
  hrTextureNodeOpen(texProcNM, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProcNM);
    
    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/normal_from_noise.c";
    code_node.append_attribute(L"main") = L"main";
  }
  hrTextureNodeClose(texProcNM);
  
  
  // other as usual in this test
  //
  HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
  HRMaterialRef mat1 = hrMaterialCreate(L"red");
  HRMaterialRef mat2 = hrMaterialCreate(L"green");
  HRMaterialRef mat3 = hrMaterialCreate(L"white");
  
  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat0);
    
    auto emission = matNode.append_child(L"emission");
    {
      emission.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);
      
      auto color = emission.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.25 0.25 0.25");
      color.append_attribute(L"tex_apply_mode ").set_value(L"multiply");
      
      auto texNode = hrTextureBind(texProc2, color);
    }
    
    xml_node diff = matNode.append_child(L"diffuse");
    {
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      auto colorNode = diff.append_child(L"color");
      
      colorNode.append_attribute(L"val") = L"0.5 0.5 0.5";
      
      auto texNode = hrTextureBind(texBitmap1, colorNode);
      
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
    }
    
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
    
    // bind proc texture to diffuse slot
    {
      auto texNode = hrTextureBind(texProc, colorNode);
      
      // not used currently
      //
      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = {1, 0, 0, 0,
                                 0, 1, 0, 0,
                                 0, 0, 1, 0,
                                 0, 0, 0, 1};
      
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");
      
      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
      
      // proc texture sampler settings
      //
      xml_node p1 = texNode.append_child(L"arg");
      p1.append_attribute(L"id") = 0;
      p1.append_attribute(L"name") = L"inColor";
      p1.append_attribute(L"type") = L"float4";
      p1.append_attribute(L"size") = 1;
      p1.append_attribute(L"val") = L"1 1 1 1";
    }
    
    // bind proc texture to normals slot
    {
      auto displacement = matNode.append_child(L"displacement");
      auto heightNode   = displacement.append_child(L"normal_map");
      
      displacement.append_attribute(L"type").set_value(L"normal_bump");
      
      auto invert = heightNode.append_child(L"invert");
      invert.append_attribute(L"x").set_value(0);
      invert.append_attribute(L"y").set_value(0);
      
      //auto nmTexNode = hrTextureBind(texNormal, heightNode);
      auto nmTexNode = hrTextureBind(texProcNM, heightNode);
      
      xml_node p1 = nmTexNode.append_child(L"arg");
      xml_node p2 = nmTexNode.append_child(L"arg");
      
      p1.append_attribute(L"id")   = 0;
      p1.append_attribute(L"name") = L"texNorm";
      p1.append_attribute(L"type") = L"sampler2D";
      p1.append_attribute(L"size") = 1;
      p1.append_attribute(L"val")  = texHeight.id;
      
      p2.append_attribute(L"id")   = 1;
      p2.append_attribute(L"name") = L"invTexRes";
      p2.append_attribute(L"type") = L"float2";
      p2.append_attribute(L"size") = 1;
      p2.append_attribute(L"val")  = L"0.002222222 0.002222222"; // (1.0f/texWidth 1.0f/texHeight)
    }
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
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(mat3);
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
  HRMeshRef planeRef = hrMeshCreate(L"my_plane");
  HRMeshRef sphereRef = hrMeshCreate(L"my_sphere");
  
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
    
    intensityNode.append_child(L"color").append_attribute(L"val") = L"1 1 1";
    intensityNode.append_child(L"multiplier").append_attribute(L"val") = 8.0f*IRRADIANCE_TO_RADIANCE;
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
  
  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);
    
    node.append_child(L"width").text()  = L"512";
    node.append_child(L"height").text() = L"512";
    
    node.append_child(L"method_primary").text()   = L"pathtracing";
    node.append_child(L"method_secondary").text() = L"pathtracing";
    node.append_child(L"method_tertiary").text()  = L"pathtracing";
    node.append_child(L"method_caustic").text()   = L"pathtracing";
    node.append_child(L"shadows").text()          = L"1";
    
    node.append_child(L"trace_depth").text()      = L"8";
    node.append_child(L"diff_trace_depth").text() = L"4";
    node.append_child(L"maxRaysPerPixel").text()  = 1024;
    node.append_child(L"evalgbuffer").text()      = 1;
  }
  hrRenderClose(renderRef);
  
  //hrRenderLogDir(renderRef, L"/home/frol/hydra/", true);
  
  // create scene
  //
  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");
  
  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;
  
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    // instance sphere and cornell box
    //
    auto mtranslate = hlm::translate4x4(hlm::float3(0, -2, 1));
    hrMeshInstance(scnRef, sphereRef, mtranslate.L());
    
    auto mrot = hlm::rotate_Y_4x4(180.0f*DEG_TO_RAD);
    hrMeshInstance(scnRef, cubeOpenRef, mrot.L());
    
    //// instance light (!!!)
    //
    mtranslate = hlm::translate4x4(hlm::float3(0, 3.85f, 0));
    hrLightInstance(scnRef, rectLight, mtranslate.L());
  }
  hrSceneClose(scnRef);
  
  hrFlush(scnRef, renderRef, camRef);
  
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
  
  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_92/z_out.png");
  //hrRenderSaveGBufferLayerLDR(renderRef, L"tests_images/test_92/z_out2.png", L"diffcolor");
  
  return check_images("test_92", 1, 20);
}


bool test93_proc_tex_recursive()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_93");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_93", HR_WRITE_DISCARD);

  SimpleMesh sphere   = CreateSphere(2.0f, 128);
  SimpleMesh cubeOpen = CreateCubeOpen(4.0f);

  // textures
  //
  HRTextureNodeRef texBitmap1 = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
  HRTextureNodeRef texBitmap2 = hrTexture2DCreateFromFile(L"data/textures/300px-Bump2.jpg");
  HRTextureNodeRef texProc    = hrTextureCreateAdvanced(L"proc", L"my_custom_tex");
  HRTextureNodeRef texProc2   = hrTextureCreateAdvanced(L"proc", L"my_custom_tex2");

  hrTextureNodeOpen(texProc, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc);

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/mul_tex_coord2.c";
    code_node.append_attribute(L"main") = L"userProc";
  }
  hrTextureNodeClose(texProc);

  hrTextureNodeOpen(texProc2, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc2);

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/blue_water.c";
    code_node.append_attribute(L"main") = L"main";
  }
  hrTextureNodeClose(texProc2);


  // other as usual in this test
  //
  HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
  HRMaterialRef mat1 = hrMaterialCreate(L"red");
  HRMaterialRef mat2 = hrMaterialCreate(L"green");
  HRMaterialRef mat3 = hrMaterialCreate(L"white");

  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat0);

    auto emission = matNode.append_child(L"emission");
    {
      emission.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);

      auto color = emission.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.25 0.25 0.25");
      color.append_attribute(L"tex_apply_mode ").set_value(L"multiply");

      auto texNode = hrTextureBind(texProc2, color);
    }

    xml_node diff = matNode.append_child(L"diffuse");
    {
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      auto colorNode = diff.append_child(L"color");

      colorNode.append_attribute(L"val") = L"0.5 0.5 0.5";

      auto texNode = hrTextureBind(texBitmap1, colorNode);

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
    }

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

    // not used currently
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

    // proc texture sampler settings /////////////////////////////////////// <---------- !!! (for Ilya)
    //

    // old: no recursive proc texture
    //
    xml_node p1 = texNode.append_child(L"arg");
    p1.append_attribute(L"id")   = 0;
    p1.append_attribute(L"name") = L"inColor";
    p1.append_attribute(L"type") = L"float4";
    p1.append_attribute(L"size") = 1;
    p1.append_attribute(L"val")  = L"1 1 1 1";

    // new: with recursive proc texture
    // xml_node p1 = texNode.append_child(L"arg");
    // {
    //   p1.append_attribute(L"id") = 0;
    //   p1.append_attribute(L"name") = L"inColor";
    //   p1.append_attribute(L"type") = L"inline_code"; // mark that we want to inline some proc texture (texProc2) as input for texProc
    //   p1.append_attribute(L"size") = 1;
    //   p1.append_attribute(L"val")  = texProc2.id;    // here we show what texture should be inlined (texProc2)
    //
    //   // if child texture has parameters (they usually have , texProc2 in this example don't have):
    //   xml_node p11 = p1.append_child(L"arg");
    //   {
    //     p11.append_attribute(L"id") = 0;
    //     p11.append_attribute(L"name") = L"inColor";
    //     p11.append_attribute(L"type") = L"float4";
    //     p11.append_attribute(L"size") = 1;
    //     p11.append_attribute(L"val")  = L"1 1 1 1";
    //   }
    //
    //   // xml_node p12 = p1.append_child(L"arg");
    //   // ...
    // }
    //
    // // it is assmumed that API will generate inlined C code in folder 'data' when material will be closed.
    // // in the same way as it fix code for common procedural textures

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
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(mat3);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
  //HRMeshRef planeRef = hrMeshCreate(L"my_plane");
  HRMeshRef sphereRef = hrMeshCreate(L"my_sphere");

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

    intensityNode.append_child(L"color").append_attribute(L"val") = L"1 1 1";
    intensityNode.append_child(L"multiplier").append_attribute(L"val") = 8.0f*IRRADIANCE_TO_RADIANCE;
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
  hrRenderLogDir(renderRef, L"/home/frol/temp/", true);

  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);

    node.append_child(L"width").text()  = L"512";
    node.append_child(L"height").text() = L"512";

    node.append_child(L"method_primary").text()   = L"pathtracing";
    node.append_child(L"method_secondary").text() = L"pathtracing";
    node.append_child(L"method_tertiary").text()  = L"pathtracing";
    node.append_child(L"method_caustic").text()   = L"pathtracing";
    node.append_child(L"shadows").text()          = L"1";

    node.append_child(L"trace_depth").text()      = L"8";
    node.append_child(L"diff_trace_depth").text() = L"4";
    node.append_child(L"maxRaysPerPixel").text()  = 1024;
    node.append_child(L"evalgbuffer").text()      = 1;
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
    auto mtranslate = hlm::translate4x4(hlm::float3(0, -2, 1));
    hrMeshInstance(scnRef, sphereRef, mtranslate.L());

    auto mrot = hlm::rotate_Y_4x4(180.0f*DEG_TO_RAD);
    hrMeshInstance(scnRef, cubeOpenRef, mrot.L());

    //// instance light (!!!)
    //
    mtranslate = hlm::translate4x4(hlm::float3(0, 3.85f, 0));
    hrLightInstance(scnRef, rectLight, mtranslate.L());
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef, camRef);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_93/z_out.png");

  return false;
}


bool test93_check_xml_fail_materials()
{
	hrErrorCallerPlace(L"test_93");
	hrSceneLibraryOpen(L"tests_f/test_93", HR_WRITE_DISCARD);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Materials
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	HRMaterialRef matPlastic = hrMaterialCreate(L"matPlastic");
	HRMaterialRef matPlasticMatte = hrMaterialCreate(L"matPlasticMatte");
	HRMaterialRef matMetal = hrMaterialCreate(L"matMetal");
	HRMaterialRef matGray = hrMaterialCreate(L"matGray");

	hrMaterialOpen(matPlastic, HR_WRITE_DISCARD);
	{
		auto matNode = hrMaterialParamNode(matPlastic);
		VERIFY_XML(matNode);                                                             // error, empty material
	}
	hrMaterialClose(matPlastic);

	hrMaterialOpen(matPlasticMatte, HR_WRITE_DISCARD);
	{
		auto matNode = hrMaterialParamNode(matPlasticMatte);

		auto diff = matNode.append_child(L"diffuse");

		diff.append_attribute(L"brdf_type").set_value(L"lambert");
		// diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.8 0.0"); // error introduced here

		auto refl = matNode.append_child(L"reflectivity");

		refl.append_attribute(L"brdf_type").set_value(L"phong");
		refl.append_child(L"color").append_attribute(L"val").set_value(L"0.5, 0.5, 0.5"); // error introduced here
		refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.7");
		refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
		refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
		refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5);

		VERIFY_XML(matNode);
	}
	hrMaterialClose(matPlasticMatte);

	hrMaterialOpen(matMetal, HR_WRITE_DISCARD);
	{
		auto matNode = hrMaterialParamNode(matMetal);

		auto diff = matNode.append_child(L"diffuse");

		diff.append_attribute(L"brdf_type").set_value(L"lambert");
		diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.0 0.0");

		auto refl = matNode.append_child(L"reflectivity");

		refl.append_attribute(L"brdf_type").set_value(L"phong");
		refl.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.0 1.0");
		refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.95");
		refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
		refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
		refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(16.0);

		VERIFY_XML(matNode);
	}
	hrMaterialClose(matMetal);

	return false;
}


bool test94_check_xml_fail_camera()
{
	HRCameraRef camRef = hrCameraCreate(L"my camera");

	hrCameraOpen(camRef, HR_WRITE_DISCARD);
	{
		auto camNode = hrCameraParamNode(camRef);

		//camNode.append_child(L"fov").text().set(L"45");                  // error introduced here
		camNode.append_child(L"nearClipPlane").text().set(L"0.01");
		camNode.append_child(L"farClipPlane").text().set(L"100.0");

		//camNode.append_child(L"up").text().set(L"0 1 0");                // error introduced here
		camNode.append_child(L"position").text().set(L"0 13 16");
		camNode.append_child(L"look_at").text().set(L"0 0 0");

		VERIFY_XML(camNode);
	}
	hrCameraClose(camRef);

	HRCameraRef camRef2 = hrCameraCreate(L"my camera2");

	hrCameraOpen(camRef2, HR_WRITE_DISCARD);
	{
		auto camNode = hrCameraParamNode(camRef2);

		camNode.append_child(L"fov").text().set(L"45");                 
		camNode.append_child(L"nearClipPlane").text().set(L"0.01");
		camNode.append_child(L"farClipPlane").text().set(L"100.0");

		camNode.append_child(L"up").text().set(L"0.0 1.0556 0.556");                
		camNode.append_child(L"position").text().set(L"0; 13; 16");        // error introduced here
		camNode.append_child(L"look_at").text().set(L"0 0 0");

		VERIFY_XML(camNode);
	}
	hrCameraClose(camRef2);

	return false;
}





bool test_126_debug_bump()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_126");

  hrSceneLibraryOpen(L"tests_f/test_126", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMaterialRef mat025 = hrMaterialCreate(L"mat025");
  HRMaterialRef mat050 = hrMaterialCreate(L"mat050");
  HRMaterialRef mat090 = hrMaterialCreate(L"mat090");
  HRMaterialRef matGray = hrMaterialCreate(L"matGray");

  HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
  HRTextureNodeRef tex = hrTexture2DCreateFromFile(L"data/textures/ornament.jpg");

  hrMaterialOpen(mat025, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(mat025);

    auto diffuse = matNode.append_child(L"diffuse");
    diffuse.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

    auto displacement = matNode.append_child(L"displacement");
    auto heightNode   = displacement.append_child(L"height_map");

    displacement.append_attribute(L"type").set_value(L"height_bump");
    heightNode.append_attribute(L"amount").set_value(0.25f);

		auto texNode = hrTextureBind(tex, heightNode);

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 2, 0, 0, 0,
                                0, 2, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };
    
    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
		VERIFY_XML(matNode);
  }
  hrMaterialClose(mat025);

  hrMaterialOpen(mat050, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(mat050);

    auto diffuse = matNode.append_child(L"diffuse");
    diffuse.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

    auto displacement = matNode.append_child(L"displacement");
    auto heightNode   = displacement.append_child(L"height_map");

    displacement.append_attribute(L"type").set_value(L"height_bump");
    heightNode.append_attribute(L"amount").set_value(0.5f);

    auto texNode = hrTextureBind(tex, heightNode);

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 2, 0, 0, 0,
                                0, 2, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
		VERIFY_XML(matNode);
  }
  hrMaterialClose(mat050);

  hrMaterialOpen(mat090, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(mat090);

    auto diffuse = matNode.append_child(L"diffuse");
    diffuse.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

    auto displacement = matNode.append_child(L"displacement");
    auto heightNode   = displacement.append_child(L"height_map");

    displacement.append_attribute(L"type").set_value(L"height_bump");
    heightNode.append_attribute(L"amount").set_value(0.9f);

    auto texNode = hrTextureBind(tex, heightNode);

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 2, 0, 0, 0,
                                0, 2, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
		VERIFY_XML(matNode);
  }
  hrMaterialClose(mat090);

  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
    color.append_attribute(L"tex_apply_mode ").set_value(L"multiply");

		auto texNode = hrTextureBind(texChecker, color);

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 16, 0, 0, 0,
                                0, 16, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
		VERIFY_XML(matNode);
  }
  hrMaterialClose(matGray);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), mat025.id);
  HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), mat050.id);
  HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), mat090.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), mat025.id);

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

    sizeNode.append_attribute(L"half_length").set_value(L"10.0");
    sizeNode.append_attribute(L"half_width").set_value(L"10.0");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(2.0f*IRRADIANCE_TO_RADIANCE);
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
    camNode.append_child(L"position").text().set(L"0 10 8");
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

  float4x4 mRot;
  float4x4 mTranslate;
  float4x4 mScale;
  float4x4 mRes;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  ///////////
  mTranslate.identity();
  mRes.identity();

  //mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
  //mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, planeRef, mRes.L());

  ///////////
  
  mTranslate.identity();
  mRes.identity();
  
  mTranslate = translate4x4(float3(-4.25f, 1.25f, 0.0f));
  mRes = mul(mTranslate, mRes);
  
  hrMeshInstance(scnRef, sph1, mRes.L());
  
  ///////////
  
  mTranslate.identity();
  mRes.identity();
  
  mTranslate = translate4x4(float3(0.0f, 1.25f, 0.0f));
  mRes = mul(mTranslate, mRes);
  
  hrMeshInstance(scnRef, sph2, mRes.L());
  
  ///////////
  
  mTranslate.identity();
  mRes.identity();
  
  mTranslate = translate4x4(float3(4.25f, 1.25f, 0.0f));
  mRes = mul(mTranslate, mRes);
  
  hrMeshInstance(scnRef, sph3, mRes.L());


  ///////////

  mTranslate.identity();
  mRes.identity();

  mTranslate = translate4x4(float3(0, 17.0f, 0.0));
  mRes = mul(mTranslate, mRes);

  hrLightInstance(scnRef, rectLight, mRes.L());

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_126/z_out.png");

  return check_images("test_126", 1, 30);
}


bool test51_instance_many_trees_and_opacity()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_51");

  hrSceneLibraryOpen(L"tests/test_51", HR_WRITE_DISCARD);

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
    auto diff    = matNode.append_child(L"diffuse");
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
  HRMeshRef cubeR    = HRMeshFromSimpleMesh(L"cubeR",    CreateCube(2.0f), matCube.id);
  HRMeshRef pillar   = HRMeshFromSimpleMesh(L"pillar",   CreateCube(1.0f), matGray.id);
  HRMeshRef sphereG  = HRMeshFromSimpleMesh(L"sphereG",  CreateSphere(4.0f, 64), matTrunk.id);
  HRMeshRef torusB   = HRMeshFromSimpleMesh(L"torusB",   CreateTorus(0.8f, 2.0f, 64, 64), matTrunk.id);
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
    camNode.append_child(L"position").text().set(L"0 7 25");
    camNode.append_child(L"look_at").text().set(L"0 3 0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  //HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 2048);


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
  mRot       = rotate_Y_4x4(60.0f*DEG_TO_RAD);
  mRes       = mul(mTranslate, mRot);

  hrMeshInstance(scnRef, cubeR, mRes.L());

  auto rgen = simplerandom::RandomGenInit(114674);
  
  {
    const float dist1     = 40.0f;
    const int SQUARESIZE1 = 100;

    for (int i = -SQUARESIZE1; i < SQUARESIZE1; i++)
    {
      for (int j = -SQUARESIZE1; j < SQUARESIZE1; j++)
      {
        const float2 randOffset = float2(simplerandom::rnd(rgen, -1.0f, 1.0f), simplerandom::rnd(rgen, -1.0f, 1.0f));
        const float3 pos = dist1*float3(float(i), 0.0f, float(j)) + dist1*1.0f*float3(randOffset.x, 0.0f, randOffset.y);

        mTranslate = translate4x4(float3(pos.x, 1.0f, pos.z));
        mRot       = rotate_Y_4x4(simplerandom::rnd(rgen, -180.0f*DEG_TO_RAD, +180.0f*DEG_TO_RAD));
        mRes       = mul(mTranslate, mRot);

        hrMeshInstance(scnRef, cubeR, mRes.L());
      }
    }
  }

  ///////////
  {
    const float dist     = 40.0f;
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

        if((simplerandom::rnd(rgen, 0.0f, 1.0f) > 0.5f))
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
  mRot = rotate_X_4x4(10.0f*DEG_TO_RAD);
  mRot2 = rotate_Z_4x4(30.f*DEG_TO_RAD);
  mRes = mul(mRot2, mRot);
  mRes = mul(mTranslate, mRes);

  hrLightInstance(scnRef, sun, mRes.L());

  ///////////

  hrSceneClose(scnRef);

  // hrFlush(scnRef);
  //return false;

  HRRenderRef renderRef = hrRenderCreate(L"HydraModern");
  hrRenderLogDir(renderRef, L"C:/[Hydra]/logs/", false);

  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
  //hrRenderEnableDevice(renderRef, 0, true);
  //hrRenderEnableDevice(renderRef, 1, true);
  
  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    auto node = hrRenderParamNode(renderRef);
  
    node.append_child(L"width").text() = 1024;
    node.append_child(L"height").text() = 768;
  
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
    }
  
    if (info.finalUpdate)
      break;
  }
  
  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_51/z_out.png");
  
  return check_images("test_51", 1, 30);

}

bool test52_instance_perf_test()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_52");

  hrSceneLibraryOpen(L"tests/test_52", HR_WRITE_DISCARD);

  HRTextureNodeRef texPattern = hrTexture2DCreateFromFile(L"data/textures/bigleaf3.tga");
  HRTextureNodeRef texture1   = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMaterialRef matGray = hrMaterialCreate(L"matGray");
  HRMaterialRef matTrunk = hrMaterialCreate(L"Trunk");
  HRMaterialRef matWigglers = hrMaterialCreate(L"Wigglers");
  HRMaterialRef matBranches = hrMaterialCreate(L"Branches");
  HRMaterialRef matPllarRts = hrMaterialCreate(L"PillarRoots");
  HRMaterialRef matLeaves = hrMaterialCreate(L"Leaves");
  HRMaterialRef matCanopy = hrMaterialCreate(L"Canopy");
  HRMaterialRef matCube = hrMaterialCreate(L"CubeMap");

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

  HRMeshRef treeRef = hrMeshCreateFromFileDL(L"data/meshes/bigtree.vsgf");

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Light
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRLightRef sky = hrLightCreate(L"sky");

  hrLightOpen(sky, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sky);

    lightNode.attribute(L"type").set_value(L"sky");
    lightNode.attribute(L"distribution").set_value(L"perez");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");

    auto sunModel = lightNode.append_child(L"perez");

    sunModel.append_attribute(L"sun_name").set_value(L"sun");
    sunModel.append_attribute(L"turbidity").set_value(2.0f);

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
    camNode.append_child(L"position").text().set(L"-2 10 35");
    camNode.append_child(L"look_at").text().set(L"8 8 0");
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

  
  auto rgen = simplerandom::RandomGenInit(34235);
  
  {
    const float dist1     = 40.0f;
    const int SQUARESIZE1 = 2;

    for (int i = -SQUARESIZE1; i < SQUARESIZE1; i++)
    {
      for (int j = -SQUARESIZE1; j < SQUARESIZE1; j++)
      {
        const float2 randOffset = float2(simplerandom::rnd(rgen, -1.0f, 1.0f), simplerandom::rnd(rgen,-1.0f, 1.0f));
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
    const float dist     = 40.0f;
    const int SQUARESIZE = 2;

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
  mRot = rotate_X_4x4(10.0f*DEG_TO_RAD);
  mRot2 = rotate_Z_4x4(30.f*DEG_TO_RAD);
  mRes = mul(mRot2, mRot);
  mRes = mul(mTranslate, mRes);

  hrLightInstance(scnRef, sun, mRes.L());

  ///////////

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef, camRef);
  
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
  
  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_52/z_out.png");
  
  return check_images("test_52", 1, 30);

  return false;
}

bool test53_crysponza_perf()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_53");

  hrSceneLibraryOpen(L"tests/test_53", HR_WRITE_DISCARD);


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

  HRMaterialRef mat0;
  mat0.id = 0;

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR    = HRMeshFromSimpleMesh(L"cubeR",    CreateCube(2.0f), mat0.id);
  HRMeshRef pillar   = HRMeshFromSimpleMesh(L"pillar",   CreateCube(1.0f), mat0.id);
  HRMeshRef sphereG  = HRMeshFromSimpleMesh(L"sphereG",  CreateSphere(4.0f, 64), mat0.id);
  HRMeshRef torusB   = HRMeshFromSimpleMesh(L"torusB",   CreateTorus(0.8f, 2.0f, 64, 64), mat0.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(10000.0f), mat0.id);

  std::cout << "loading sponza begin ... " << std::endl;
#if defined WIN32
  Timer myTimer(true);
#endif
  HRMeshRef treeRef  = hrMeshCreateFromFileDL(L"data/meshes/crysponza.vsgf");
#if defined WIN32
  std::cout << "loading sponza time = " << myTimer.getElapsed()*1000.0f << "ms" << std::endl;
#endif
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

  mTranslate = translate4x4(float3(0,0,0));
  mScale     = scale4x4(float3(5.0f, 5.0f, 5.0f));
  mRot       = rotate_Y_4x4(90.0f*DEG_TO_RAD);
  mRes       = mul(mTranslate, mul(mRot, mScale));

  hrMeshInstance(scnRef, treeRef, mRes.L());
  
  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();
  mRot2.identity();

  mTranslate = translate4x4(float3(200.0f, 200.0f, -100.0f));
  mRot = rotate_X_4x4(10.0f*DEG_TO_RAD);
  mRot2 = rotate_Z_4x4(10.f*DEG_TO_RAD);
  mRes = mul(mRot2, mRot);
  mRes = mul(mTranslate, mRes);

  hrLightInstance(scnRef, sun, mRes.L());

  ///////////

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef, camRef);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_53/z_out.png");

  return false;
}

bool test54_portalsroom_perf()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_54");

  hrSceneLibraryOpen(L"tests/test_54", HR_WRITE_DISCARD);


  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR    = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), mat0.id);
  HRMeshRef pillar   = HRMeshFromSimpleMesh(L"pillar", CreateCube(1.0f), mat0.id);
  HRMeshRef sphereG  = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), mat0.id);
  HRMeshRef torusB   = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), mat0.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(10000.0f), mat0.id);

  HRMeshRef treeRef  = hrMeshCreateFromFileDL(L"data/meshes/skyportals2.vsgf");

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

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"5.0");

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
    camNode.append_child(L"position").text().set(L"25 12 -60");
    camNode.append_child(L"look_at").text().set(L" 30 12 -58");
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

  hrMeshInstance(scnRef, cubeR, mRes.L());

  ///////////
  mTranslate = translate4x4(float3(-10, 0, 0));
  mScale     = scale4x4(float3(10.0f, 10.0f, 10.0f));
  mRot       = rotate_Y_4x4(90.0f*DEG_TO_RAD);
  mRes       = mul(mTranslate, mul(mRot, mScale));

  hrMeshInstance(scnRef, treeRef, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();
  mRot2.identity();

  mTranslate = translate4x4(float3(2000.0f, 2000.0f, 100.0f));
  mRot       = rotate_X_4x4(-10.0f*DEG_TO_RAD);
  mRot2      = rotate_Z_4x4(-50.f*DEG_TO_RAD);
  mRes       = mul(mRot2, mRot);
  mRes       = mul(mTranslate, mRes);

  hrLightInstance(scnRef, sun, mRes.L());

  ///////////

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef, camRef);
  
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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_54/z_out.png");

  return false;
}
