//
// Created by vsan on 09.02.18.
//

#include <iomanip>

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

#include "tests.h"
#include "HydraXMLHelpers.h"
#include "../hydra_api/HR_HDRImageTool.h"

#pragma warning(disable:4996)
using namespace TEST_UTILS;
using HydraRender::SaveImageToFile;

extern GLFWwindow* g_window;

bool test1000_loadlibrary_and_edit()
{
  initGLIfNeeded();

  hrSceneLibraryOpen(L"tests/test_1000", HR_OPEN_EXISTING);

  /////////////////////////////////////////////////////////
  HRRenderRef renderRef;
  renderRef.id = 0;

  HRSceneInstRef scnRef;
  scnRef.id = 0;
  /////////////////////////////////////////////////////////

  auto pList = hrRenderGetDeviceList(renderRef);

  while (pList != nullptr)
  {
    std::wcout << L"device id = " << pList->id << L", name = " << pList->name << L", driver = " << pList->driver << std::endl;
    pList = pList->next;
  }

  hrRenderEnableDevice(renderRef, 0, true);

  hrCommit(scnRef, renderRef);
  hrRenderCommand(renderRef, L"pause");
  std::this_thread::sleep_for(std::chrono::milliseconds(500)); //so render has some time to actually stop


  HRLightRef sky;
  sky.id = 0;

  hrLightOpen(sky, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sky);

    lightNode.attribute(L"type").set_value(L"sky");
    lightNode.attribute(L"distribution").set_value(L"map");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"0.5");

    VERIFY_XML(lightNode);
  }
  hrLightClose(sky);

  HRMaterialRef matRefl;
  matRefl.id = 1;
  hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matRefl);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.05 0.8 0.025");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.99");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(16.0f);
  }
  hrMaterialClose(matRefl);


  hrFlush(scnRef, renderRef);
  //hrCommit(scnRef, renderRef);
  //hrRenderCommand(renderRef, L"resume");

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
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);

      glfwSwapBuffers(g_window);
      glfwPollEvents();
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_1000/z_out.png");

  std::remove("tests/test_1000/change_00001.xml");
  std::remove("tests/test_1000/statex_00002.xml");

  return check_images("test_1000", 1, 50.0f);
}

bool test1001_loadlibrary_and_add_textures()
{
  initGLIfNeeded();

  hrSceneLibraryOpen(L"tests/test_1001", HR_OPEN_EXISTING);

  /////////////////////////////////////////////////////////
  HRRenderRef renderRef;
  renderRef.id = 0;

  HRSceneInstRef scnRef;
  scnRef.id = 0;
  /////////////////////////////////////////////////////////

  auto pList = hrRenderGetDeviceList(renderRef);

  while (pList != nullptr)
  {
    std::wcout << L"device id = " << pList->id << L", name = " << pList->name << L", driver = " << pList->driver << std::endl;
    pList = pList->next;
  }

  hrRenderEnableDevice(renderRef, 0, true);

  hrCommit(scnRef, renderRef);
  hrRenderCommand(renderRef, L"pause");
  std::this_thread::sleep_for(std::chrono::milliseconds(500)); //so render has some time to actually stop

  glViewport(0, 0, 1024, 768);
  std::vector<int32_t> image(1024 * 768);


  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/LA_Downtown_Afternoon_Fishing_B_8k.jpg"); //23_antwerp_night.hdr LA_Downtown_Afternoon_Fishing_B_8k.jpg

  HRLightRef sky;
  sky.id = 0;

  hrLightOpen(sky, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sky);

    lightNode.attribute(L"type").set_value(L"sky");
    lightNode.attribute(L"distribution").set_value(L"map");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 0 0");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");

    auto texNode = hrTextureBind(texEnv, intensityNode.child(L"color"));

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(lightNode);
  }
  hrLightClose(sky);

  hrFlush(scnRef, renderRef);
  //hrCommit(scnRef, renderRef);
  //hrRenderCommand(renderRef, L"resume");


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
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);

      glfwSwapBuffers(g_window);
      glfwPollEvents();
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_1001/z_out.png");

  return check_images("test_1001", 1, 50.0f);
}