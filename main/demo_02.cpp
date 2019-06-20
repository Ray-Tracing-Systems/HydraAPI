#include <iostream>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>

#include <iomanip> // for (std::fixed, std::setfill, std::setw)

#include "../hydra_api/HydraAPI.h"
#include "../hydra_api/HydraXMLVerify.h"

#include "mesh_utils.h"

using pugi::xml_node;

#include "../hydra_api/LiteMath.h"
namespace hlm = HydraLiteMath;

///////////////////////////////////////////////////////////////////////// window and opegl
#if defined(WIN32)
#include <FreeImage.h>
#include <GLFW/glfw3.h>
#pragma comment(lib, "glfw3dll.lib")
#pragma comment(lib, "FreeImage.lib")
#else
#include <FreeImage.h>
#include <GLFW/glfw3.h>
#endif

extern GLFWwindow* g_window;
void initGLIfNeeded(int a_width = 512, int a_height = 512, const char* name = "glfw demo");
///////////////////////////////////////////////////////////////////////// window and opegl

void demo_02_load_obj()
{
  const int DEMO_WIDTH  = 512;
  const int DEMO_HEIGHT = 512;
  
  hrErrorCallerPlace(L"demo_02_load_obj");
  
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  hrSceneLibraryOpen(L"demos/demo_02", HR_WRITE_DISCARD);
  
  SimpleMesh sphere   = CreateSphere(2.0f, 128);
  SimpleMesh cubeOpen = CreateCubeOpen(4.0f);
  
  HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
  HRMaterialRef mat1 = hrMaterialCreate(L"red");
  HRMaterialRef mat2 = hrMaterialCreate(L"green");
  HRMaterialRef mat3 = hrMaterialCreate(L"white");
  HRMaterialRef mat4 = hrMaterialCreate(L"gold");
  
  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat0);
    xml_node diff    = matNode.append_child(L"diffuse");
    
    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val") = L"0.5 0.5 0.5";
    
    VERIFY_XML(matNode); // you can verify XML parameters for the main renderer "HydraModern" in this way
  }
  hrMaterialClose(mat0);
  
  hrMaterialOpen(mat1, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat1);
    xml_node diff = matNode.append_child(L"diffuse");
    
    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val") = L"0.5 0.0 0.0";
    
    VERIFY_XML(matNode);
  }
  hrMaterialClose(mat1);
  
  hrMaterialOpen(mat2, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat2);
    xml_node diff = matNode.append_child(L"diffuse");
    
    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.5 0.0");
  
    VERIFY_XML(matNode);
  }
  hrMaterialClose(mat2);
  
  hrMaterialOpen(mat3, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat3);
    xml_node diff = matNode.append_child(L"diffuse");
    
    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  
    VERIFY_XML(matNode);
  }
  hrMaterialClose(mat3);
  
  hrMaterialOpen(mat4, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat4);
    xml_node diff    = matNode.append_child(L"diffuse");
    xml_node refl    = matNode.append_child(L"reflectivity");
    
    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val") = L"0.40 0.4 0";
    
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val")      = L"0.10 0.10 0";
    refl.append_child(L"glossiness").append_attribute(L"val") = 0.85f;
  
    //refl.append_child(L"fresnel").text() = 1;                   // uncomment this to enable fresnel reflections
    //refl.append_child(L"fresnel_IOR").text().set(L"2.5");
    
    VERIFY_XML(matNode);
  }
  hrMaterialClose(mat4);
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
  HRMeshRef bunnyRef    = hrMeshCreateFromFile(L"data/meshes/bunny.obj"); //#NOTE: loaded from ".obj" models are guarantee to have material id '0' for all triangles
                                                                          // to apply other material, please see further for remap list application to object instance
  hrMeshOpen(cubeOpenRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeOpenRef, L"pos",      &cubeOpen.vPos[0]);
    hrMeshVertexAttribPointer4f(cubeOpenRef, L"norm",     &cubeOpen.vNorm[0]);
    hrMeshVertexAttribPointer2f(cubeOpenRef, L"texcoord", &cubeOpen.vTexCoord[0]);
    
    int cubeMatIndices[10] = { mat3.id, mat3.id, mat3.id, mat3.id, mat3.id, mat3.id, mat2.id, mat2.id, mat1.id, mat1.id };
    
    //hrMeshMaterialId(cubeRef, 0);
    hrMeshPrimitiveAttribPointer1i(cubeOpenRef, L"mind", cubeMatIndices);
    hrMeshAppendTriangles3(cubeOpenRef, int(cubeOpen.triIndices.size()), &cubeOpen.triIndices[0]);
  }
  hrMeshClose(cubeOpenRef);
  
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
    
    intensityNode.append_child(L"color").append_attribute(L"val")      = L"1 1 1";
    intensityNode.append_child(L"multiplier").append_attribute(L"val") = 8.0f;
  
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
    camNode.append_child(L"position").text().set(L"0 0 14");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  
    VERIFY_XML(camNode);
  }
  hrCameraClose(camRef);
  
  // set up render settings
  //
  HRRenderRef renderRef = hrRenderCreate(L"HydraModern"); // "HydraModern" is our main renderer name
  hrRenderEnableDevice(renderRef, 0, true);
  
  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);
    
    node.append_child(L"width").text()  = DEMO_WIDTH;
    node.append_child(L"height").text() = DEMO_HEIGHT;
    
    node.append_child(L"method_primary").text()   = L"pathtracing";
    node.append_child(L"method_secondary").text() = L"pathtracing";
    node.append_child(L"method_tertiary").text()  = L"pathtracing";
    node.append_child(L"method_caustic").text()   = L"pathtracing";
    
    node.append_child(L"trace_depth").text()      = 8;
    node.append_child(L"diff_trace_depth").text() = 4;
    node.append_child(L"maxRaysPerPixel").text()  = 1024;
    node.append_child(L"qmc_variant").text()      = (HYDRA_QMC_DOF_FLAG | HYDRA_QMC_MTL_FLAG | HYDRA_QMC_LGT_FLAG); // enable all of them, results to '7'
  }
  hrRenderClose(renderRef);
  
  // create scene
  //
  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");
  
  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;
  
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    // instance bynny and cornell box
    //
    auto mscale     = hlm::scale4x4(hlm::float3(3,3,3));
    auto mtranslate = hlm::translate4x4(hlm::float3(1, -4.2, 0));
    auto mres       = hlm::mul(mtranslate,mscale);
    
    int32_t remapList[2] = {0, mat4.id};                                                       // #NOTE: remaplist of size 1 here: [0 --> mat4.id]
    hrMeshInstance(scnRef, bunnyRef, mres.L(), remapList, sizeof(remapList)/sizeof(int32_t));  //
    
    auto mrot = hlm::rotate_Y_4x4(180.0f*DEG_TO_RAD);
    hrMeshInstance(scnRef, cubeOpenRef, mrot.L());
    
    //// instance light (!!!)
    //
    mtranslate = hlm::translate4x4(hlm::float3(0, 3.85f, 0));
    hrLightInstance(scnRef, rectLight, mtranslate.L());
  }
  hrSceneClose(scnRef);
  
  hrFlush(scnRef, renderRef, camRef);
  
  //////////////////////////////////////////////////////// opengl
  std::vector<int32_t> image(DEMO_WIDTH*DEMO_HEIGHT);
  initGLIfNeeded(DEMO_WIDTH,DEMO_HEIGHT, "load 'obj.' file demo");
  glViewport(0,0,DEMO_WIDTH,DEMO_HEIGHT);
  //////////////////////////////////////////////////////// opengl
  
  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);
    
    if (info.haveUpdateFB)
    {
      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
      std::cout.precision(pres);
      
      hrRenderGetFrameBufferLDR1i(renderRef, DEMO_WIDTH, DEMO_HEIGHT, &image[0]);
  
      //////////////////////////////////////////////////////// opengl
      glDisable(GL_TEXTURE_2D);
      glDrawPixels(DEMO_WIDTH, DEMO_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
      
      glfwSwapBuffers(g_window);
      glfwPollEvents();
      //////////////////////////////////////////////////////// opengl
    }
    
    if (info.finalUpdate)
      break;
  }
  
  hrRenderSaveFrameBufferLDR(renderRef, L"demos/demo_02/z_out.png");
  
}