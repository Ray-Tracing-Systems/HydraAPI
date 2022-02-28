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

void demo_04_instancing()
{
  const int DEMO_WIDTH  = 1024;
  const int DEMO_HEIGHT = 1024;

  hrErrorCallerPlace(L"demo_04_instancing");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"demos/demo_04", HR_WRITE_DISCARD);

  HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
  HRMaterialRef mat1 = hrMaterialCreate(L"red");
  HRMaterialRef mat2 = hrMaterialCreate(L"green");
  HRMaterialRef mat3 = hrMaterialCreate(L"white");
  HRMaterialRef mat4 = hrMaterialCreate(L"gold");
  HRMaterialRef matGlass = hrMaterialCreate(L"glass");

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
    diff.append_child(L"color").append_attribute(L"val") = L"0.2 0.2 0.1";

    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val")      = L"0.6 0.6 0.1";
    refl.append_child(L"glossiness").append_attribute(L"val") = 0.85f;

    //refl.append_child(L"fresnel").text() = 1;                   // uncomment this to enable fresnel reflections
    //refl.append_child(L"fresnel_IOR").text().set(L"2.5");

    VERIFY_XML(matNode);
  }
  hrMaterialClose(mat4);

  hrMaterialOpen(matGlass, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGlass);

    auto refl = matNode.append_child(L"reflectivity");

    refl.append_attribute(L"brdf_type").set_value(L"phong");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(2.41f);

    auto transp = matNode.append_child(L"transparency");

    transp.append_attribute(L"brdf_type").set_value(L"phong");
    transp.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
    transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0);
    transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
    transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
    transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(0.0f);
    transp.append_child(L"ior").append_attribute(L"val").set_value(2.41f);

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
  auto planeData    = CreatePlane(100.0f);
  HRMeshRef planeRef = hrMeshCreate(L"my_plane");

  hrMeshOpen(planeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(planeRef, L"pos",      &planeData.vPos[0]);
    hrMeshVertexAttribPointer4f(planeRef, L"norm",     &planeData.vNorm[0]);
    hrMeshVertexAttribPointer2f(planeRef, L"texcoord", &planeData.vTexCoord[0]);
    hrMeshMaterialId           (planeRef, mat0.id);
    hrMeshAppendTriangles3     (planeRef, int(planeData.triIndices.size()), &planeData.triIndices[0]);
  }
  hrMeshClose(planeRef);

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

  HRLightRef sky = hrLightCreate(L"sky");

  hrLightOpen(sky, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sky);
    lightNode.attribute(L"type").set_value(L"sky");
	  lightNode.attribute(L"distribution").set_value(L"map");
    auto intensityNode = lightNode.append_child(L"intensity");
    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");

    auto texEnv  = hrTexture2DCreateFromFile(L"data/textures/LA_Downtown_Afternoon_Fishing_B_8k.jpg");

	  auto texNode = hrTextureBind(texEnv, intensityNode.child(L"color"));

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

    camNode.append_child(L"fov").text().set(L"45");
    camNode.append_child(L"nearClipPlane").text().set(L"0.01");
    camNode.append_child(L"farClipPlane").text().set(L"100.0");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 10 40");
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

    node.append_child(L"method_primary").text()   = L"PT";
    node.append_child(L"method_secondary").text() = L"PT";
    node.append_child(L"method_tertiary").text()  = L"PT";
    node.append_child(L"method_caustic").text()   = L"PT";
    node.append_child(L"qmc_variant").text()      = (HYDRA_QMC_DOF_FLAG | HYDRA_QMC_MTL_FLAG | HYDRA_QMC_LGT_FLAG); // enable all of them, results to '7'

    node.append_child(L"trace_depth").text()      = 8;
    node.append_child(L"diff_trace_depth").text() = 3;
    node.append_child(L"maxRaysPerPixel").text()  = 1024;
  }
  hrRenderClose(renderRef);

  // create scene
  //
  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

  auto rgen = hr_prng::RandomGenInit(12368754);
  const float sceneSize = 75.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    for(int i=0;i<200;i++)
    {
      float teapotPosX  = hr_prng::rndFloatUniform(rgen, -sceneSize, sceneSize);
      float teapotPosY  = hr_prng::rndFloatUniform(rgen, -2.0f, 1.0f);
      float teapotPosZ  = hr_prng::rndFloatUniform(rgen, -sceneSize, sceneSize);
      float teapotScale = hr_prng::rndFloatUniform(rgen, 1.0f, 3.0f);
      float teapotRotX  = hr_prng::rndFloatUniform(rgen, -45.0f, 45.0f);
      float teapotRotY  = hr_prng::rndFloatUniform(rgen, -45.0f, 45.0f);

      int32_t matsNum     = (matGlass.id + 1); // matGlass was last added material ...
      int32_t teapotMatId = hr_prng::rndIntUniform(rgen,0,matsNum);

      if(teapotMatId >= matsNum) teapotMatId = matsNum-1;

      // instance bynny and cornell box
      //
      auto mscale     = hlm::scale4x4(hlm::float3(teapotScale, teapotScale, teapotScale));
      auto mtranslate = hlm::translate4x4(hlm::float3(teapotPosX, teapotPosY, teapotPosZ)); // -2.5
      auto mrot       = hlm::rotate4x4Y(teapotRotX * DEG_TO_RAD);
      auto mrot3      = hlm::rotate4x4X(teapotRotY * DEG_TO_RAD);
      auto mres       = mtranslate*mrot3*mrot*mscale;

      float rowMajorData[16];
      mres.StoreRowMajor(rowMajorData);

      int32_t remapList[6] = {0, teapotMatId, 1, teapotMatId, 2, teapotMatId};                          // #NOTE: remaplist of size 1 here: [0 --> mat4.id]
      hrMeshInstance(scnRef, teapotRef, rowMajorData, remapList, sizeof(remapList) / sizeof(int32_t));  //
    }

    for(int i=0;i<400;i++)
    {
      float bynnuPosX  = hr_prng::rndFloatUniform(rgen, -sceneSize, sceneSize);
      float bynnuPosZ  = hr_prng::rndFloatUniform(rgen, -sceneSize, sceneSize);
      float bynnuScale = hr_prng::rndFloatUniform(rgen, 1.0f, 2.0f);
      float bunnyRotY  = hr_prng::rndFloatUniform(rgen, -90.0f, 90.0f);

      int32_t matsNum     = (matGlass.id + 1); // matGlass was last added material ...
      int32_t bunnyMatId  = hr_prng::rndIntUniform(rgen,0,matsNum);

      if(bunnyMatId >= matsNum)   bunnyMatId = matsNum-1;

      auto mscale     = hlm::scale4x4    (hlm::float3(bynnuScale, bynnuScale, bynnuScale));
      auto mtranslate = hlm::translate4x4(hlm::float3(bynnuPosX, -4.0, bynnuPosZ));
      auto mrot       = hlm::rotate4x4Y  (bunnyRotY * DEG_TO_RAD);
      auto mres       = mtranslate*mrot*mscale;

      float rowMajorData[16];
      mres.StoreRowMajor(rowMajorData);

      int32_t remapList2[2] = {0, bunnyMatId};                                                           // #NOTE: remaplist of size 1 here: [0 --> mat4.id]
      hrMeshInstance(scnRef, bunnyRef, rowMajorData, remapList2, sizeof(remapList2) / sizeof(int32_t));  //
    }

    auto mtranslate = hlm::translate4x4(hlm::float3(0, -4, 0));
    
    float rowMajorData[16];
    mtranslate.StoreRowMajor(rowMajorData);

    hrMeshInstance(scnRef, planeRef, rowMajorData);  //

    //// instance light (!!!)
    //
    mtranslate = hlm::float4x4();                  // can use identity matrix for sky light
    mtranslate.StoreRowMajor(rowMajorData);

    hrLightInstance(scnRef, sky, rowMajorData);
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef, camRef);

  //////////////////////////////////////////////////////// opengl
  std::vector<int32_t> image(DEMO_WIDTH*DEMO_HEIGHT);
  initGLIfNeeded(DEMO_WIDTH,DEMO_HEIGHT, "instancing demo");
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

  hrRenderSaveFrameBufferLDR(renderRef, L"demos/demo_04/z_out.png");
}
