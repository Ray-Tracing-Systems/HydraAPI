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

#include "../hydra_api/HydraObjectManager.h"
#include "../hydra_api/HydraXMLHelpers.h"
#include "../hydra_api/HR_HDRImageTool.h"

#include "tests.h"

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
  bool firstUpdate = true;

  glViewport(0, 0, 1024, 768);
  std::vector<int32_t> image(1024 * 768);
  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      if(firstUpdate)
      {
        std::remove("tests/test_1000/change_00001.xml");
        std::remove("tests/test_1000/statex_00002.xml");
        firstUpdate = false;
      }
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

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
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
  bool firstUpdate = true;

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      if(firstUpdate)
      {
        std::remove("tests/test_1001/change_00001.xml");
        std::remove("tests/test_1001/statex_00002.xml");
        firstUpdate = false;
      }

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

bool test1002_get_material_by_name_and_edit()
{
  initGLIfNeeded();

  hrSceneLibraryOpen(L"tests/test_1002", HR_OPEN_EXISTING);

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

  std::wstring mat1(L"matRefl");
  HRMaterialRef matRefl = hrMaterialFindByName(mat1.c_str());

  if(matRefl.id != -1)
  {
    hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matRefl);
      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.025");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.5");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(16.0f);
    }
    hrMaterialClose(matRefl);
  }

  std::wstring mat2(L"matGray");
  HRMaterialRef matGray = hrMaterialFindByName(mat2.c_str());

  if(matGray.id != -1)
  {
    hrMaterialOpen(matGray, HR_OPEN_EXISTING);
    {
      auto matNode = hrMaterialParamNode(matGray);
      auto diff = matNode.force_child(L"diffuse");
      diff.force_child(L"color").force_attribute(L"val").set_value(L"0.1 0.2 0.725");
    }
    hrMaterialClose(matGray);
  }

  std::wstring mat3(L"matSomething");
  HRMaterialRef matSmth = hrMaterialFindByName(mat3.c_str());

  if(matSmth.id != -1)
  {
    hrMaterialOpen(matSmth, HR_OPEN_EXISTING);
    {
      auto matNode = hrMaterialParamNode(matSmth);
      auto diff = matNode.force_child(L"diffuse");
      diff.force_child(L"color").force_attribute(L"val").set_value(L"1.0 0.1 0.1");
    }
    hrMaterialClose(matSmth);
  }


  hrFlush(scnRef, renderRef);

  //hrCommit(scnRef, renderRef);
  //hrRenderCommand(renderRef, L"resume");
  bool firstUpdate = true;

  glViewport(0, 0, 1024, 768);
  std::vector<int32_t> image(1024 * 768);
  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      if(firstUpdate)
      {
        std::remove("tests/test_1002/change_00001.xml");
        std::remove("tests/test_1002/statex_00002.xml");
        firstUpdate = false;
      }
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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_1002/z_out.png");

  return check_images("test_1002", 1, 50.0f);
}

bool test1003_get_light_by_name_and_edit()
{
  initGLIfNeeded();

  hrSceneLibraryOpen(L"tests/test_1003", HR_OPEN_EXISTING);

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

  std::wstring light1(L"sphere1");
  auto lightRef1 = hrLightFindByName(light1.c_str());

  if(lightRef1.id != -1)
  {
    hrLightOpen(lightRef1, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(lightRef1);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      pugi::xml_node sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"1.0");
      sizeNode.append_attribute(L"half_width").set_value(L"1.0");

      auto intensityNode = lightNode.append_child(L"intensity");
      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.0 1.0 0.1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"4.0");

      VERIFY_XML(lightNode);
    }
    hrLightClose(lightRef1);
  }

  std::wstring light2(L"sphere1");
  auto lightRef2 = hrLightFindByName(light2.c_str());

  if(lightRef2.id != -1)
  {
    hrLightOpen(lightRef2, HR_OPEN_EXISTING);
    {
      auto lightNode = hrLightParamNode(lightRef2);

      auto intensityNode = lightNode.force_child(L"intensity");
      intensityNode.force_child(L"color").force_attribute(L"val").set_value(L"1.0 0.3 0.0");
      intensityNode.force_child(L"multiplier").force_attribute(L"val").set_value(L"12.0");

      VERIFY_XML(lightNode);
    }
    hrLightClose(lightRef2);
  }

  std::wstring light3(L"lightDoesntExist");
  auto lightRef3 = hrLightFindByName(light3.c_str());

  if(lightRef3.id != -1)
  {
    hrLightOpen(lightRef3, HR_OPEN_EXISTING);
    {
      auto lightNode = hrLightParamNode(lightRef3);

      auto intensityNode = lightNode.force_child(L"intensity");
      intensityNode.force_child(L"multiplier").force_attribute(L"val").set_value(L"24.0");

      VERIFY_XML(lightNode);
    }
    hrLightClose(lightRef3);
  }


  hrFlush(scnRef, renderRef);

  //hrCommit(scnRef, renderRef);
  //hrRenderCommand(renderRef, L"resume");
  bool firstUpdate = true;

  glViewport(0, 0, 1024, 768);
  std::vector<int32_t> image(1024 * 768);
  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      if(firstUpdate)
      {
        std::remove("tests/test_1003/change_00001.xml");
        std::remove("tests/test_1003/statex_00002.xml");
        firstUpdate = false;
      }
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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_1003/z_out.png");

  return check_images("test_1003", 1, 50.0f);
}

bool test1004_get_camera_by_name_and_edit()
{
  initGLIfNeeded();

  hrSceneLibraryOpen(L"tests/test_1004", HR_OPEN_EXISTING);

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

  std::wstring cam1(L"my camera");
  HRCameraRef camRef1 = hrCameraFindByName(cam1.c_str());

  if(camRef1.id != -1)
  {
    hrCameraOpen(camRef1, HR_OPEN_EXISTING);
    {
      auto camNode = hrCameraParamNode(camRef1);
      camNode.force_child(L"position").text().set(L"25 15 6");
    }
    hrCameraClose(camRef1);
  }

  std::wstring cam2(L"my non-existent camera");
  HRCameraRef camRef2 = hrCameraFindByName(cam2.c_str());

  if(camRef2.id != -1)
  {
    hrCameraOpen(camRef2, HR_OPEN_EXISTING);
    {
      auto camNode = hrCameraParamNode(camRef2);
      auto pos = camNode.child(L"position").text().set(L"0 150 0");
    }
    hrCameraClose(camRef2);
  }

  hrFlush(scnRef, renderRef, camRef1);

  //hrCommit(scnRef, renderRef);
  //hrRenderCommand(renderRef, L"resume");
  bool firstUpdate = true;

  glViewport(0, 0, 1024, 768);
  std::vector<int32_t> image(1024 * 768);
  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      if(firstUpdate)
      {
        std::remove("tests/test_1004/change_00001.xml");
        std::remove("tests/test_1004/statex_00002.xml");
        firstUpdate = false;
      }
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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_1004/z_out.png");

  return check_images("test_1004", 1, 50.0f);
}

bool test1005_transform_all_instances()
{
  initGLIfNeeded();

  hrSceneLibraryOpen(L"tests/test_1005", HR_OPEN_EXISTING);

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
  

  float matrix[16] = { 0.7071f, 0, -0.7071f, 0,
                             0, 1,        0, 6,
                       0.7071f, 0,  0.7071f, 0,
                             0, 0,        0, 1 };


  HRUtils::TransformAllInstances(scnRef, matrix);

  hrFlush(scnRef, renderRef);

  //hrCommit(scnRef, renderRef);
  //hrRenderCommand(renderRef, L"resume");
  bool firstUpdate = true;

  glViewport(0, 0, 1024, 768);
  std::vector<int32_t> image(1024 * 768);
  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      if(firstUpdate)
      {
        std::remove("tests/test_1005/change_00001.xml");
        std::remove("tests/test_1005/statex_00002.xml");
        firstUpdate = false;
      }
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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_1005/z_out.png");

  return check_images("test_1005", 1, 50.0f);
}