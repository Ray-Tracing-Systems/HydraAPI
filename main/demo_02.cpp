#include <iostream>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>

#include <iomanip> // for (std::fixed, std::setfill, std::setw)

#include "../hydra_api/HydraAPI.h"
#include "../hydra_api/HydraXMLVerify.h"
#include "../utils/mesh_utils.h"

using pugi::xml_node;

#include "../hydra_api/LiteMath.h"
namespace hlm = LiteMath;

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
void initGLIfNeeded(int a_width, int a_height, const char* name);
///////////////////////////////////////////////////////////////////////// window and opegl

void _hrDebugPrintVSGF(const wchar_t* a_fileNameIn, const wchar_t* a_fileNameOut);

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
  //HRMeshRef bunnyRef    = hrMeshCreateFromFile(L"data/meshes/bunny.obj"); //#NOTE: loaded from ".obj" models are guarantee to have material id '0' for all triangles
  //                                                                        // to apply other material, please see further for remap list application to object instance
  
  HRMeshRef        bunnyRef = hrMeshCreateFromFile(L"/home/frol/temp/03_ResultsFromPasha_Magic123/chair2/mesh.obj");
  HRTextureNodeRef texBunny = hrTexture2DCreateFromFile(L"/home/frol/temp/03_ResultsFromPasha_Magic123/chair2/albedo.png");
  hlm::float3 modelOffset   = hlm::float3(1, -2.0, 0);
  float modelScale          = 8.0f;

  //HRMeshRef        bunnyRef = hrMeshCreateFromFile(L"/home/frol/temp/03_ResultsFromPasha_Magic123/bird/mesh.obj");
  //HRTextureNodeRef texBunny = hrTexture2DCreateFromFile(L"/home/frol/temp/03_ResultsFromPasha_Magic123/bird/albedo.png");
  //hlm::float3 modelOffset   = hlm::float3(1, -1.0, 0);
  //float modelScale          = 6.0f;
  
  HRMaterialRef matObj = hrMaterialCreate(L"matObject");
  hrMaterialOpen(matObj, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(matObj);
    xml_node diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto color = diff.append_child(L"color");
    color.append_attribute(L"val") = L"1 1 1";
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");
    hrTextureBind(texBunny, color);
    VERIFY_XML(matNode);
  }
  hrMaterialClose(matObj);
  
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
    intensityNode.append_child(L"multiplier").append_attribute(L"val") = 25.0f;
  
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
    
    node.append_child(L"trace_depth").text()      = 6;
    node.append_child(L"diff_trace_depth").text() = 3;
    node.append_child(L"maxRaysPerPixel").text()  = 256;
    node.append_child(L"qmc_variant").text()      = (HYDRA_QMC_DOF_FLAG | HYDRA_QMC_MTL_FLAG | HYDRA_QMC_LGT_FLAG); // enable all of them, results to '7'
  }
  hrRenderClose(renderRef);
  
  // create scene
  //
  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");
  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;
  
  const int NFrames = 25;
  for(int frame = 0; frame < NFrames; frame++)
  {
    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    {
      // instance bynny and cornell box
      //
      auto mrotObj    = hlm::rotate4x4Y(360.0f*DEG_TO_RAD*float(frame)/float(NFrames));
      auto mscale     = hlm::scale4x4(hlm::float3(modelScale,modelScale,modelScale));
      auto mtranslate = hlm::translate4x4(modelOffset);
      auto mres       = mtranslate*mrotObj*mscale;
      
      float rowMajorData[16];
      mres.StoreRowMajor(rowMajorData);
  
      int32_t remapList[2] = {0, matObj.id};                                                          // #NOTE: remaplist of size 1 here: [0 --> mat4.id]
      hrMeshInstance(scnRef, bunnyRef, rowMajorData, remapList, sizeof(remapList)/sizeof(int32_t)); //
      
      auto mrot = hlm::rotate4x4Y(180.0f*DEG_TO_RAD);
      mrot.StoreRowMajor(rowMajorData);
  
      hrMeshInstance(scnRef, cubeOpenRef, rowMajorData);
      
      //// instance light (!!!)
      //
      mtranslate = hlm::translate4x4(hlm::float3(0, 3.85f, 0));
      mtranslate.StoreRowMajor(rowMajorData);
  
      hrLightInstance(scnRef, rectLight, rowMajorData);
    }
    hrSceneClose(scnRef);
  
    hrFlush(scnRef, renderRef, camRef);

    std::cout << "rendering frame " << frame << std::endl;

    // and wait while it finish
    //
    while (true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
      HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);
    
      if (info.haveUpdateFB)
      {
        std::cout << "rendering progress = " << info.progress << "% \r";
        std::cout.flush();
      }
    
      if (info.finalUpdate)
        break;
    }
  
    std::wstringstream strOut;
    strOut << std::fixed  << L"demos/demo_02/zout_" << std::setfill(L"0"[0]) << std::setw(2) << frame << L".png";
    auto str = strOut.str();
    
    hrRenderSaveFrameBufferLDR(renderRef, str.c_str());
  }
  
  /*
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
  }*/
  
  //hrRenderSaveFrameBufferLDR(renderRef, L"demos/demo_02/z_out.png");
  
  //_hrDebugPrintVSGF(L"/home/frol/PROG/HydraRepos/HydraAPI/main/demos/demo_02/data/chunk_00001.vsgf", 
  //                  L"/home/frol/PROG/HydraRepos/HydraAPI/main/demos/demo_02/data/chunk_00001.txt");
}