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

void demo_06_lens_and_plane()
{
  const int DEMO_WIDTH   = 1024;
  const int DEMO_HEIGHT  = 1024;
  const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

  hrErrorCallerPlace(L"demo_06_lens_and_plane");
  hrSceneLibraryOpen(L"demos/demo_06", HR_WRITE_DISCARD);

  HRMaterialRef matPlane    = hrMaterialCreate(L"mysimplemat");
  HRTextureNodeRef texFloor = hrTexture2DCreateFromFile(L"data/textures/USAF1951_8mp_color_v2.png");

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
                                0, -1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };
 
    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");
    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
    VERIFY_XML(matNode); // you can verify XML parameters for the main renderer "HydraModern" in this way
  }
  hrMaterialClose(matPlane);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // create plane
  //
  std::vector<int> indices = {0, 1, 2, 0, 2, 3};
  std::vector<LiteMath::float4> positions = { {-2, -2, 0, 1},   
                                              { 2, -2, 0, 1},   
                                              { 2,  2, 0, 1},   
                                              {-2,  2, 0, 1} };
  
  std::vector<LiteMath::float4> normals  = { {0, 0, 1, 0},
                                             {0, 0, 1, 0},
                                             {0, 0, 1, 0},
                                             {0, 0, 1, 0} };

  std::vector<LiteMath::float2> texcoord = {{0,0}, {1,0}, {1,1}, {0,1} };

  HRMeshRef planeRef = hrMeshCreate(L"my_plane");
  hrMeshOpen(planeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(planeRef, L"pos",     (const float*)positions.data());
    hrMeshVertexAttribPointer4f(planeRef, L"norm",    (const float*)normals.data());
    hrMeshVertexAttribPointer2f(planeRef, L"texcoord",(const float*)texcoord.data());
    hrMeshMaterialId           (planeRef, matPlane.id);
    hrMeshAppendTriangles3     (planeRef, int(indices.size()), indices.data());
  }
  hrMeshClose(planeRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRLightRef sky = hrLightCreate(L"sky");
  hrLightOpen(sky, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sky);
    lightNode.attribute(L"type").set_value(L"sky");
	  lightNode.attribute(L"distribution").set_value(L"uniform");
    auto intensityNode = lightNode.append_child(L"intensity");
    intensityNode.append_child(L"color").append_attribute(L"val")      = L"1 1 1";
    intensityNode.append_child(L"multiplier").append_attribute(L"val") = 1.0f;
	  VERIFY_XML(lightNode);
  }
  hrLightClose(sky);

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

    camNode.append_child(L"position").text().set(L"0 0 0");
    camNode.append_child(L"look_at").text().set(L"0 0 -1");
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
    node.append_child(L"qmc_variant").text()      = 0; // (HYDRA_QMC_DOF_FLAG | HYDRA_QMC_MTL_FLAG | HYDRA_QMC_LGT_FLAG); // enable all of them, results to '7'

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

    auto mtranslate = hlm::translate4x4(hlm::float3(0, 0, -10));
    auto mscale     = hlm::scale4x4(hlm::float3(2.2358234982127496f, 2.2358234982127496f, 1.0f));
    auto mscale2    = hlm::scale4x4(hlm::float3(0.5f, 0.5f, 1.0f));
    auto mFinal     = mtranslate*mscale*mscale2;
    mFinal.StoreRowMajor(rowMajorData);
    hrMeshInstance(scnRef, planeRef, rowMajorData);  //

    //// instance light 
    //
    mtranslate = hlm::translate4x4(hlm::float3(0, 100, 0));          
    mtranslate.StoreRowMajor(rowMajorData);
    hrLightInstance(scnRef, sky, rowMajorData);
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

  hrRenderSaveFrameBufferLDR(renderRef, L"demos/demo_06/z_out.png");
}
