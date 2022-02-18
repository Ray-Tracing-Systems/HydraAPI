//
// Created by frol on 25.06.19.
//

#include <iostream>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>

#include <iomanip> // for (std::fixed, std::setfill, std::setw)

#include "../hydra_api/HydraAPI.h"
#include "../hydra_api/HydraXMLVerify.h"
#include "../hydra_api/HydraXMLHelpers.h"
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

void demo_05_pbrt_spheres()
{
  const int DEMO_WIDTH  = 1024;
  const int DEMO_HEIGHT = 1024;
  const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

  hrErrorCallerPlace(L"demo_05_pbrt_spheres");
  hrSceneLibraryOpen(L"demos/demo_05", HR_WRITE_DISCARD);

  HRMaterialRef matPlane  = hrMaterialCreate(L"mysimplemat");
  HRMaterialRef matGlass  = hrMaterialCreate(L"glass");
  HRMaterialRef matMirror = hrMaterialCreate(L"mirror");

  HRTextureNodeRef texFloor = hrTexture2DCreateFromFile(L"/home/frol/PROG/pbrt-v3-scenes/simple/textures/lines.png");

  hrMaterialOpen(matPlane, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(matPlane);
    xml_node diff    = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto color = diff.append_child(L"color");
    
    color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

    hrTextureBind(texFloor, color);

    auto texNode = color.child(L"texture");
    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    //auto mTex = hlm::rotate4x4Z(+45.0f*DEG_TO_RAD)*hlm::scale4x4(hlm::float3(10,10,10)); // *hlm::translate4x4(hlm::float3(0.5f,0.5f,0.0f))
    auto mTex = hlm::scale4x4(hlm::float3(-10,10,10));
    mTex.StoreRowMajor(samplerMatrix);
 
    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");
    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
    VERIFY_XML(matNode); // you can verify XML parameters for the main renderer "HydraModern" in this way
  }
  hrMaterialClose(matPlane);

  hrMaterialOpen(matMirror, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(matMirror);
    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type")                        = L"phong";
    refl.append_child(L"color").append_attribute(L"val")       = L"0.95 0.95 0.95";
    refl.append_child(L"glossiness").append_attribute(L"val")  = 1.0f;
    VERIFY_XML(matNode);
  }
  hrMaterialClose(matMirror);

  hrMaterialOpen(matGlass, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGlass);
    auto refl    = matNode.append_child(L"reflectivity");

    refl.append_attribute(L"brdf_type")                        = L"ggx";
    refl.append_child(L"color").append_attribute(L"val")       = L"1.0 1.0 1.0";
    refl.append_child(L"glossiness").append_attribute(L"val")  = 1.0f;
    refl.append_child(L"extrusion").append_attribute(L"val")   = L"maxcolor";
    refl.append_child(L"fresnel").append_attribute(L"val")     = 1;
    refl.append_child(L"fresnel_ior").append_attribute(L"val") = 1.5f;

    auto transp = matNode.append_child(L"transparency");

    transp.append_attribute(L"brdf_type").set_value(L"ggx");
    transp.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
    transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0);
    transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
    transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
    transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(0.0f);
    transp.append_child(L"ior").append_attribute(L"val").set_value(1.5f);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matGlass);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMeshRef bunnyRef  = hrMeshCreateFromFile(L"data/meshes/bunny.obj");   // #NOTE: loaded from ".obj" models are guarantee to have material id '0' for all triangles
  HRMeshRef teapotRef = hrMeshCreateFromFile(L"data/meshes/teapot.vsgf"); // load teapot from our simple internal format

  // create plane
  //
  auto planeData  = CreatePlane(100.0f);
  auto sphereData = CreateSphere(1.0f, 100);

  HRMeshRef planeRef  = hrMeshCreate(L"my_plane");
  HRMeshRef sphereRef = hrMeshCreate(L"my_sphere");

  hrMeshOpen(planeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(planeRef, L"pos",     planeData.vPos.data());
    hrMeshVertexAttribPointer4f(planeRef, L"norm",    planeData.vNorm.data());
    hrMeshVertexAttribPointer2f(planeRef, L"texcoord",planeData.vTexCoord.data());
    hrMeshMaterialId           (planeRef, matPlane.id);
    hrMeshAppendTriangles3     (planeRef, int(planeData.triIndices.size()), planeData.triIndices.data());
  }
  hrMeshClose(planeRef);

  hrMeshOpen(sphereRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(sphereRef, L"pos",      &sphereData.vPos[0]);
    hrMeshVertexAttribPointer4f(sphereRef, L"norm",     &sphereData.vNorm[0]);
    hrMeshVertexAttribPointer2f(sphereRef, L"texcoord", &sphereData.vTexCoord[0]);
    hrMeshMaterialId           (sphereRef, matGlass.id);
    hrMeshAppendTriangles3     (sphereRef, int(sphereData.triIndices.size()), &sphereData.triIndices[0]);
  }
  hrMeshClose(sphereRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRLightRef sunLight = hrLightCreate(L"direct_light");
  hrLightOpen(sunLight, HR_WRITE_DISCARD);
  {
    pugi::xml_node lightNode = hrLightParamNode(sunLight);

    lightNode.attribute(L"type").set_value(L"directional");
    lightNode.attribute(L"shape").set_value(L"point");
    lightNode.attribute(L"distribution").set_value(L"directional");

    pugi::xml_node intensityNode = lightNode.append_child(L"intensity");
    intensityNode.append_child(L"color").append_attribute(L"val")      = L"1 1 1";
    intensityNode.append_child(L"multiplier").append_attribute(L"val") = 3.141593f;

    VERIFY_XML(lightNode);
  }
  hrLightClose(sunLight);

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

    camNode.append_child(L"fov").text()           = 30;
    camNode.append_child(L"nearClipPlane").text() = 0.01f;
    camNode.append_child(L"farClipPlane").text()  = 1000.0f;

    camNode.append_child(L"position").text().set(L"0 2 10");
    camNode.append_child(L"look_at").text().set(L"0 -0.4 0");
    camNode.append_child(L"up").text().set(L"0 1 0");

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

    node.append_child(L"method_primary").text()   = L"PT";
    node.append_child(L"method_secondary").text() = L"PT";
    node.append_child(L"method_tertiary").text()  = L"PT";
    node.append_child(L"method_caustic").text()   = L"PT";
    node.append_child(L"qmc_variant").text()      = (HYDRA_QMC_DOF_FLAG | HYDRA_QMC_MTL_FLAG | HYDRA_QMC_LGT_FLAG); // enable all of them, results to '7'

    node.append_child(L"trace_depth").text()      = 10;
    node.append_child(L"diff_trace_depth").text() = 4;
    node.append_child(L"maxRaysPerPixel").text()  = 1024;
  }
  hrRenderClose(renderRef);

  // create scene
  //
  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");
  //const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    float rowMajorData[16];

    auto mtranslate = hlm::translate4x4(hlm::float3(-0.25f, -1, 0));
    mtranslate.StoreRowMajor(rowMajorData);
    hrMeshInstance(scnRef, planeRef, rowMajorData);  //

    mtranslate = hlm::translate4x4(hlm::float3(-1.3, 0, 0));
    mtranslate.StoreRowMajor(rowMajorData);
    hrMeshInstance(scnRef, sphereRef, rowMajorData);  //
    
    int32_t remapList[2] = {matGlass.id, matMirror.id};  
    mtranslate = hlm::translate4x4(hlm::float3(+1.3, 0, 0));
    mtranslate.StoreRowMajor(rowMajorData);
    hrMeshInstance(scnRef, sphereRef, rowMajorData, remapList, sizeof(remapList) / sizeof(int32_t));  //

    //// instance light 
    //

    //auto mRot  = hlm::rotate4x4X(0.0001f);
    mtranslate = hlm::translate4x4(hlm::float3(0, 10, 0)); //*mRot;            
    mtranslate.StoreRowMajor(rowMajorData);
    hrLightInstance(scnRef, sunLight, rowMajorData);
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef, camRef);

  //////////////////////////////////////////////////////// opengl
  std::vector<int32_t> image(DEMO_WIDTH*DEMO_HEIGHT);
  initGLIfNeeded(DEMO_WIDTH,DEMO_HEIGHT, "pbrt spheres demo");
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

  hrRenderSaveFrameBufferLDR(renderRef, L"demos/demo_05/z_out.png");
}
