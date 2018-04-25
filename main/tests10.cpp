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
  HRMaterialRef matRefl = hrFindMaterialByName(mat1.c_str());

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
  HRMaterialRef matGray = hrFindMaterialByName(mat2.c_str());

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
  HRMaterialRef matSmth = hrFindMaterialByName(mat3.c_str());

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
  auto lightRef1 = hrFindLightByName(light1.c_str());

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
  auto lightRef2 = hrFindLightByName(light2.c_str());

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
  auto lightRef3 = hrFindLightByName(light3.c_str());

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
  HRCameraRef camRef1 = hrFindCameraByName(cam1.c_str());

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
  HRCameraRef camRef2 = hrFindCameraByName(cam2.c_str());

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
                             0, 1,        0, 1,
                       0.7071f, 0,  0.7071f, 0,
                             0, 0,        0, 1 };


  HRUtils::InstanceSceneIntoScene(scnRef, scnRef, matrix, false);

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

  return check_images("test_1005", 1, 100.0f);
}

bool test1006_transform_all_instances_origin()
{
  initGLIfNeeded();

  hrSceneLibraryOpen(L"tests/test_1006", HR_OPEN_EXISTING);

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
                       0, 1,        0,       0,
                       0.7071f, 0,  0.7071f, 0,
                       0, 0,        0,       1 };


  HRUtils::InstanceSceneIntoScene(scnRef, scnRef, matrix);

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
        std::remove("tests/test_1006/change_00001.xml");
        std::remove("tests/test_1006/statex_00002.xml");
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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_1006/z_out.png");

  return check_images("test_1006", 1, 100.0f);
}

bool test1007_merge_library()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_1007");

  hrSceneLibraryOpen(L"tests/test_1007", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  HRMaterialRef matGray = hrMaterialCreate(L"matGray");
  HRMaterialRef matRefl = hrMaterialCreate(L"matRefl");


  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(matGray);

  hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matRefl);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);
  }
  hrMaterialClose(matRefl);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matGray.id);
  HRMeshRef sphereG = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matRefl.id);
  HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matGray.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Light
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRLightRef sky = hrLightCreate(L"sky");

  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/Factory_Catwalk_2k_BLUR.exr");

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


  HRUtils::MergeLibraryIntoLibrary(L"tests_f/test_131");

  HRMeshRef mergedMesh;
  mergedMesh.id = 5;

  auto blend = hrFindMaterialByName(L"matBlend3");

  hrMeshOpen(torusB, HR_TRIANGLE_IND3, HR_OPEN_EXISTING);
  {
    hrMeshMaterialId(torusB, blend.id);
  }
  hrMeshClose(torusB);


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
    camNode.append_child(L"position").text().set(L"0 13 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 1024);


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

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();
  mRot2.identity();

  mTranslate = translate4x4(float3(4.0f, 1.0f, 5.5f));
  mRot = rotate_Y_4x4(-60.0f*DEG_TO_RAD);
  mRot2 = rotate_X_4x4(90.0f*DEG_TO_RAD);
  mRes = mul(mRot, mRot2);
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, torusB, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(0.0f, 2.0f, -1.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, sphereG, mRes.L());

  ///////////
  mRes.identity();

  mTranslate = translate4x4(float3(8.0f, 2.0f, 0.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, mergedMesh, mRes.L());

  ///////////

  mRes.identity();

  hrLightInstance(scnRef, sky, mRes.L());

  ///////////

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_1007/z_out.png");

  return check_images("test_1007", 1, 60);
}

bool test1008_merge_one_texture()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_1008");

  hrSceneLibraryOpen(L"tests/test_1008", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  HRMaterialRef matGray = hrMaterialCreate(L"matGray");
  HRMaterialRef matRefl = hrMaterialCreate(L"matRefl");


  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(matGray);

  hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matRefl);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);
  }
  hrMaterialClose(matRefl);


  //

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matGray.id);
  HRMeshRef sphereG = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matRefl.id);
  HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matGray.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Light
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRLightRef sky = hrLightCreate(L"sky");

  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/Factory_Catwalk_2k_BLUR.exr");

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

  HRTextureNodeRef mergedTex = HRUtils::MergeOneTextureIntoLibrary(L"tests_f/test_131", L"data/textures/yinyang.png");

  hrMaterialOpen(matGray, HR_OPEN_EXISTING);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto color = matNode.child(L"diffuse").child(L"color");
    auto texNode = hrTextureBind(mergedTex, color);

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 2, 0, 0, 0,
                                0, 2, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
  }
  hrMaterialClose(matGray);


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
    camNode.append_child(L"position").text().set(L"0 13 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 1024);


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

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();
  mRot2.identity();

  mTranslate = translate4x4(float3(4.0f, 1.0f, 5.5f));
  mRot = rotate_Y_4x4(-60.0f*DEG_TO_RAD);
  mRot2 = rotate_X_4x4(90.0f*DEG_TO_RAD);
  mRes = mul(mRot, mRot2);
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, torusB, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(0.0f, 2.0f, -1.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, sphereG, mRes.L());

  ///////////

  mRes.identity();

  hrLightInstance(scnRef, sky, mRes.L());

  ///////////

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_1008/z_out.png");

  return check_images("test_1008", 1, 60);
}


bool test1009_merge_one_material()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_1009");

  hrSceneLibraryOpen(L"tests/test_1009", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  HRMaterialRef matGray = hrMaterialCreate(L"matGray");
  HRMaterialRef matRefl = hrMaterialCreate(L"matRefl");


  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(matGray);

  hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matRefl);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);
  }
  hrMaterialClose(matRefl);


  //

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matGray.id);
  HRMeshRef sphereG = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matRefl.id);
  HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matGray.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Light
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRLightRef sky = hrLightCreate(L"sky");

  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/Factory_Catwalk_2k_BLUR.exr");

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



  HRMaterialRef mergedMat1 = HRUtils::MergeOneMaterialIntoLibrary(L"tests_f/test_131", L"matBlend1");
  HRMaterialRef mergedMat2 = HRUtils::MergeOneMaterialIntoLibrary(L"tests_f/test_131", L"matBlend2");
  HRMaterialRef mergedMat3 = HRUtils::MergeOneMaterialIntoLibrary(L"tests_f/test_131", L"matBlend1");

  hrMeshOpen(sphereG, HR_TRIANGLE_IND3, HR_OPEN_EXISTING);
  {
    hrMeshMaterialId(sphereG, mergedMat1.id);
  }
  hrMeshClose(sphereG);

  hrMeshOpen(torusB, HR_TRIANGLE_IND3, HR_OPEN_EXISTING);
  {
    hrMeshMaterialId(torusB, mergedMat2.id);
  }
  hrMeshClose(torusB);

  hrMeshOpen(planeRef, HR_TRIANGLE_IND3, HR_OPEN_EXISTING);
  {
    hrMeshMaterialId(planeRef, mergedMat3.id);
  }
  hrMeshClose(planeRef);


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
    camNode.append_child(L"position").text().set(L"0 13 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 1024);


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

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();
  mRot2.identity();

  mTranslate = translate4x4(float3(4.0f, 1.0f, 5.5f));
  mRot = rotate_Y_4x4(-60.0f*DEG_TO_RAD);
  mRot2 = rotate_X_4x4(90.0f*DEG_TO_RAD);
  mRes = mul(mRot, mRot2);
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, torusB, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(0.0f, 2.0f, -1.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, sphereG, mRes.L());

  ///////////

  mRes.identity();

  hrLightInstance(scnRef, sky, mRes.L());

  ///////////

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_1009/z_out.png");

  return check_images("test_1009", 1, 60);
}

bool test1010_merge_one_mesh()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_1010");

  hrSceneLibraryOpen(L"tests/test_1010", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  HRMaterialRef matGray = hrMaterialCreate(L"matGray");
  HRMaterialRef matRefl = hrMaterialCreate(L"matRefl");


  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(matGray);

  hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matRefl);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);
  }
  hrMaterialClose(matRefl);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matGray.id);
  HRMeshRef sphereG = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matRefl.id);
  HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matGray.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Light
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRLightRef sky = hrLightCreate(L"sky");

  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/Factory_Catwalk_2k_BLUR.exr");

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



  HRMeshRef mergedTree = HRUtils::MergeOneMeshIntoLibrary(L"tests_f/test_005", L"data/meshes/bigtree.vsgf");
  HRMeshRef mergedLucy = HRUtils::MergeOneMeshIntoLibrary(L"tests_f/test_002", L"data/meshes/lucy.vsgf");


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
    camNode.append_child(L"position").text().set(L"0 13 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 1024);


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

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();
  mRot2.identity();

  mTranslate = translate4x4(float3(4.0f, 1.0f, 5.5f));
  mRot = rotate_Y_4x4(-60.0f*DEG_TO_RAD);
  mRot2 = rotate_X_4x4(90.0f*DEG_TO_RAD);
  mRes = mul(mRot, mRot2);
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, torusB, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(0.0f, 2.0f, -1.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, sphereG, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(6.0f, 0.0f, -1.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, mergedTree, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(-6.0f, 0.0f, 1.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, mergedLucy, mRes.L());

  ///////////

  mRes.identity();

  hrLightInstance(scnRef, sky, mRes.L());

  ///////////

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_1010/z_out.png");

  return check_images("test_1010", 1, 60);
}

bool test1011_merge_scene()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_1011");

  hrSceneLibraryOpen(L"tests/test_1011", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  HRMaterialRef matGray = hrMaterialCreate(L"matGray");
  HRMaterialRef matRefl = hrMaterialCreate(L"matRefl");


  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(matGray);

  hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matRefl);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);
  }
  hrMaterialClose(matRefl);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matGray.id);
  HRMeshRef sphereG = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matRefl.id);
  HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matGray.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Light
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRLightRef sky = hrLightCreate(L"sky");

  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/Factory_Catwalk_2k_BLUR.exr");

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
    camNode.append_child(L"position").text().set(L"0 13 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  HRCameraRef camRef2 = hrCameraCreate(L"my camera");

  hrCameraOpen(camRef2, HR_WRITE_DISCARD);
  {
    auto camNode = hrCameraParamNode(camRef2);

    camNode.append_child(L"fov").text().set(L"60");
    camNode.append_child(L"nearClipPlane").text().set(L"0.01");
    camNode.append_child(L"farClipPlane").text().set(L"100.0");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 20 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef2);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 1024);


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

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();
  mRot2.identity();

  mTranslate = translate4x4(float3(4.0f, 1.0f, 5.5f));
  mRot = rotate_Y_4x4(-60.0f*DEG_TO_RAD);
  mRot2 = rotate_X_4x4(90.0f*DEG_TO_RAD);
  mRes = mul(mRot, mRot2);
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, torusB, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(0.0f, 2.0f, -1.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, sphereG, mRes.L());

  ///////////

  mRes.identity();

  hrLightInstance(scnRef, sky, mRes.L());

  ///////////

  hrSceneClose(scnRef);


  HRSceneInstRef scnRef2 = HRUtils::MergeLibraryIntoLibrary(L"tests_f/test_131", true, true);

  mTranslate = translate4x4(float3(0.0f, 5.0f, 0.0f));

  HRUtils::InstanceSceneIntoScene(scnRef2, scnRef, mTranslate.L());

  hrFlush(scnRef, renderRef, camRef);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_1011/z_out.png");

  return check_images("test_1011", 1, 60);
}

bool test1012_merge_one_light()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_1012");

  hrSceneLibraryOpen(L"tests/test_1012", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  HRMaterialRef matGray = hrMaterialCreate(L"matGray");
  HRMaterialRef matRefl = hrMaterialCreate(L"matRefl");


  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(matGray);

  hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matRefl);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);
  }
  hrMaterialClose(matRefl);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matGray.id);
  HRMeshRef sphereG = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matRefl.id);
  HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matGray.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Light
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRLightRef sky = hrLightCreate(L"sky");

  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/Factory_Catwalk_2k_BLUR.exr");

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



  HRLightRef mergedLight1 = HRUtils::MergeOneLightIntoLibrary(L"tests_f/test_221", L"spot2");
  HRLightRef mergedLight2 = HRUtils::MergeOneLightIntoLibrary(L"tests_f/test_221", L"spot3");


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
    camNode.append_child(L"position").text().set(L"0 13 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 1024);


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

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();
  mRot2.identity();

  mTranslate = translate4x4(float3(4.0f, 1.0f, 5.5f));
  mRot = rotate_Y_4x4(-60.0f*DEG_TO_RAD);
  mRot2 = rotate_X_4x4(90.0f*DEG_TO_RAD);
  mRes = mul(mRot, mRot2);
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, torusB, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(0.0f, 2.0f, -1.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, sphereG, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(6.0f, 4.0f, 0.0f));
  mRes = mul(mTranslate, mRes);

  hrLightInstance(scnRef, mergedLight1, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(-6.0f, 4.0f, 0.0f));
  mRes = mul(mTranslate, mRes);

  hrLightInstance(scnRef, mergedLight2, mRes.L());

  ///////////

  mRes.identity();

  hrLightInstance(scnRef, sky, mRes.L());

  ///////////

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_1012/z_out.png");

  return check_images("test_1012", 1, 60);
}


bool test1013_commit_without_render()
{
  initGLIfNeeded();

  hrSceneLibraryOpen(L"tests/test_1013", HR_OPEN_EXISTING);

  /////////////////////////////////////////////////////////

  HRSceneInstRef scnRef;
  scnRef.id = 0;
  /////////////////////////////////////////////////////////

  hrCommit(scnRef);

  HRLightRef sky;
  sky.id = 0;

  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/LA_Downtown_Afternoon_Fishing_B_8k.jpg"); //23_antwerp_night.hdr LA_Downtown_Afternoon_Fishing_B_8k.jpg

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

  HRMaterialRef matRefl;
  matRefl.id = 1;
  hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matRefl);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.8 0.025");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.99");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(16.0f);
  }
  hrMaterialClose(matRefl);

  HRRenderRef renderRef;
  renderRef.id = 0;

  auto pList = hrRenderGetDeviceList(renderRef);

  while (pList != nullptr)
  {
    std::wcout << L"device id = " << pList->id << L", name = " << pList->name << L", driver = " << pList->driver << std::endl;
    pList = pList->next;
  }

  hrRenderEnableDevice(renderRef, 0, true);


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
        std::remove("tests/test_1013/change_00001.xml");
        std::remove("tests/test_1013/statex_00002.xml");
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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_1013/z_out.png");

  return check_images("test_1013", 1, 50.0f);
}

bool test1014_print_matlib_map()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_1014");

  hrSceneLibraryOpen(L"tests/test_1014", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  HRMaterialRef matGray = hrMaterialCreate(L"matGray");
  HRMaterialRef matRefl = hrMaterialCreate(L"matRefl");
  HRMaterialRef matGold    = hrMaterialCreate(L"matGold");
  HRMaterialRef matSilver  = hrMaterialCreate(L"matSilver");


  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(matGray);

  hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matRefl);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);
  }
  hrMaterialClose(matRefl);

  hrMaterialOpen(matGold, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGold);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.88 0.61 0.05");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matGold);

  hrMaterialOpen(matSilver, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matSilver);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.9 0.9 0.9");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(16.0f);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matSilver);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matGray.id);
  HRMeshRef sphereG = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matSilver.id);
  HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matGold.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Light
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRLightRef sky = hrLightCreate(L"sky");

  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/Factory_Catwalk_2k_BLUR.exr");

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


  HRUtils::MergeLibraryIntoLibrary(L"tests_f/test_131");

  HRMeshRef mergedMesh;
  mergedMesh.id = 5;

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
    camNode.append_child(L"position").text().set(L"0 13 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 1024);


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

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();
  mRot2.identity();

  mTranslate = translate4x4(float3(4.0f, 1.0f, 5.5f));
  mRot = rotate_Y_4x4(-60.0f*DEG_TO_RAD);
  mRot2 = rotate_X_4x4(90.0f*DEG_TO_RAD);
  mRes = mul(mRot, mRot2);
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, torusB, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(0.0f, 2.0f, -1.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, sphereG, mRes.L());

  ///////////
  mRes.identity();

  mTranslate = translate4x4(float3(8.0f, 2.0f, 0.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, mergedMesh, mRes.L());

  ///////////

  mRes.identity();

  hrLightInstance(scnRef, sky, mRes.L());

  ///////////

  hrSceneClose(scnRef);

  hrCommit(scnRef);

  auto materials = HydraXMLHelpers::GetMaterialNameToIdMap();

  std::stringstream ss;
  for(auto mat : materials)
  {
    ss << mat.first << " : " << mat.second << std::endl;
  }

  std::string outStr = ss.str();

  std::string correctStr = """matGray : 0\n"
                             "matRefl : 1\n"
                             "matGold : 2\n"
                             "matSilver : 3\n"
                             "matGold : 4\n"
                             "matSilver : 5\n"
                             "matLacquer : 6\n"
                             "matGlass : 7\n"
                             "matBricks1 : 8\n"
                             "matBricks2 : 9\n"
                             "matGray : 10\n"
                             "matBlend1 : 11\n"
                             "matBlend2 : 12\n"
                             "matBlend3 : 13\n"
                             "my_area_light_material : 14\n"
                             "sky_material : 15\n""";


  hrFlush(scnRef, renderRef, camRef);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_1014/z_out.png");

  return check_images("test_1014", 1, 60) && (outStr == correctStr);
}

bool test1015_merge_scene_with_remaps()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_1015");

  hrSceneLibraryOpen(L"tests/test_1015", HR_WRITE_DISCARD);


  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  HRMaterialRef matGray = hrMaterialCreate(L"matGray");
  HRMaterialRef matRefl = hrMaterialCreate(L"matRefl");


  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(matGray);

  hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matRefl);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);
  }
  hrMaterialClose(matRefl);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matGray.id);
  HRMeshRef sphereG = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matRefl.id);
  HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matGray.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);


  HRLightRef sky = hrLightCreate(L"sky");

  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/Factory_Catwalk_2k_BLUR.exr");

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
    camNode.append_child(L"position").text().set(L"0 13 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 256);
  hrRenderOpen(renderRef, HR_OPEN_EXISTING);
  {
    auto node = hrRenderParamNode(renderRef);
    node.append_child(L"evalgbuffer").text() = L"1";
  }
  hrRenderClose(renderRef);

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

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(-6.0f, 1.5f, -4.5f));
  mRot = rotate_Y_4x4(60.0f*DEG_TO_RAD);
  mRes = mul(mTranslate, mRot);

  hrMeshInstance(scnRef, cubeR, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();
  mRot2.identity();

  mTranslate = translate4x4(float3(6.0f, 1.5f, -4.5f));
  mRot = rotate_Y_4x4(-60.0f*DEG_TO_RAD);
  mRot2 = rotate_X_4x4(90.0f*DEG_TO_RAD);
  mRes = mul(mRot, mRot2);
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, torusB, mRes.L());


  ///////////

  mRes.identity();

  hrLightInstance(scnRef, sky, mRes.L());

  ///////////

  hrSceneClose(scnRef);


  HRSceneInstRef scnRef2 = HRUtils::MergeLibraryIntoLibrary(L"tests/test_1015_lib_to_merge", false, true);

  mTranslate = scale4x4(float3(7.0f, 7.0f, 7.0f));

  HRUtils::InstanceSceneIntoScene(scnRef2, scnRef, mTranslate.L(), false);

  hrFlush(scnRef, renderRef, camRef);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_1015/z_out.png");
  hrRenderSaveGBufferLayerLDR(renderRef, L"tests_images/test_1015/z_scnid.png", L"scnid");

  return check_images("test_1015", 1, 60);
}

bool test1016_merge_scene_remap_override()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_1016");

  hrSceneLibraryOpen(L"tests/test_1016", HR_WRITE_DISCARD);


  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  HRMaterialRef matGray = hrMaterialCreate(L"matGray");
  HRMaterialRef matRefl = hrMaterialCreate(L"matRefl");


  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(matGray);

  hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matRefl);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);
  }
  hrMaterialClose(matRefl);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matGray.id);
  HRMeshRef sphereG = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matRefl.id);
  HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matGray.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);


  HRLightRef sky = hrLightCreate(L"sky");

  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/Factory_Catwalk_2k_BLUR.exr");

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
    camNode.append_child(L"position").text().set(L"0 13 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 1024);


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

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(-6.0f, 1.5f, -4.5f));
  mRot = rotate_Y_4x4(60.0f*DEG_TO_RAD);
  mRes = mul(mTranslate, mRot);

  hrMeshInstance(scnRef, cubeR, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();
  mRot2.identity();

  mTranslate = translate4x4(float3(6.0f, 1.5f, -4.5f));
  mRot = rotate_Y_4x4(-60.0f*DEG_TO_RAD);
  mRot2 = rotate_X_4x4(90.0f*DEG_TO_RAD);
  mRes = mul(mRot, mRot2);
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, torusB, mRes.L());


  ///////////

  mRes.identity();

  hrLightInstance(scnRef, sky, mRes.L());

  ///////////

  hrSceneClose(scnRef);


  HRSceneInstRef scnRef2 = HRUtils::MergeLibraryIntoLibrary(L"tests/test_1015_lib_to_merge", false, true);

  hrCommit(scnRef);

  auto materials = HydraXMLHelpers::GetMaterialNameToIdMap();


  mTranslate = scale4x4(float3(7.0f, 7.0f, 7.0f));

  //std::vector<int32_t> override_list = {4, matRefl.id};

  std::vector<int32_t> override_list = {4, 13, 6, 12, 7, 11, 8, 13, 0, 1, 2, 3, 10, 15, 20, 25};
  //std::vector<int32_t> override_list = {4, 13, 6, 13, 7, 13, 8, 13, 0, 1, 2, 3, 10, 15, 20, 25};
  //std::vector<int32_t> override_list = {4, 12};



  HRUtils::InstanceSceneIntoScene(scnRef2, scnRef, mTranslate.L(), false, &override_list[0], override_list.size());

  hrFlush(scnRef, renderRef, camRef);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_1016/z_out.png");

  return check_images("test_1016", 1, 60);
}

bool test1017_merge_scene_scene_id_mask()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_1017");

  hrSceneLibraryOpen(L"tests/test_1017", HR_WRITE_DISCARD);


  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  HRMaterialRef matGray = hrMaterialCreate(L"matGray");
  HRMaterialRef matRefl = hrMaterialCreate(L"matRefl");


  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(matGray);

  hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matRefl);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);
  }
  hrMaterialClose(matRefl);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matGray.id);
  HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matGray.id);

  HRLightRef sky = hrLightCreate(L"sky");

  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/Factory_Catwalk_2k_BLUR.exr");

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
    camNode.append_child(L"position").text().set(L"0 13 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 32, 512);
  hrRenderOpen(renderRef, HR_OPEN_EXISTING);
  {
    auto node = hrRenderParamNode(renderRef);
    node.append_child(L"evalgbuffer").text() = L"1";
  }
  hrRenderClose(renderRef);

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

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(-6.0f, 1.5f, -4.5f));
  mRot = rotate_Y_4x4(60.0f*DEG_TO_RAD);
  mRes = mul(mTranslate, mRot);

  hrMeshInstance(scnRef, cubeR, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();
  mRot2.identity();

  mTranslate = translate4x4(float3(6.0f, 1.5f, -4.5f));
  mRot = rotate_Y_4x4(-60.0f*DEG_TO_RAD);
  mRot2 = rotate_X_4x4(90.0f*DEG_TO_RAD);
  mRes = mul(mRot, mRot2);
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, torusB, mRes.L());


  ///////////

  mRes.identity();

  hrLightInstance(scnRef, sky, mRes.L());

  ///////////

  hrSceneClose(scnRef);


  HRSceneInstRef scnRef2 = HRUtils::MergeLibraryIntoLibrary(L"tests/test_1015_lib_to_merge", false, true);
  mTranslate = translate4x4(float3(-6.0f, -2.0f, 0.0f));
  mScale = scale4x4(float3(3.0f, 3.0f, 3.0f));
  mRes = mul(mTranslate, mScale);
  auto box = HRUtils::InstanceSceneIntoScene(scnRef2, scnRef, mRes.L(), false);


  HRSceneInstRef scnRef3 = HRUtils::MergeLibraryIntoLibrary(L"tests/test_1015_lib_to_merge", false, true);
  mTranslate = translate4x4(float3(6.0f, -2.0f, 0.0f));
  mScale = scale4x4(float3(3.0f, 3.0f, 3.0f));
  mRes = mul(mTranslate, mScale);
  HRUtils::InstanceSceneIntoScene(scnRef3, scnRef, mRes.L(), false);

  mTranslate = translate4x4(float3(0.0f, -4.0f, 0.0f));
  mScale = scale4x4(float3(3.0f, 3.0f, 3.0f));
  mRes = mul(mTranslate, mScale);
  HRUtils::InstanceSceneIntoScene(scnRef3, scnRef, mRes.L(), false);

  hrFlush(scnRef, renderRef, camRef);

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

  hrRenderSaveFrameBufferLDR(renderRef,  L"tests_images/test_1017/z_out.png");
  hrRenderSaveGBufferLayerLDR(renderRef, L"tests_images/test_1017/z_out2.png", L"scnid");
  hrRenderSaveGBufferLayerLDR(renderRef, L"tests_images/test_1017/z_out3.png", L"scnsid");
  hrRenderSaveGBufferLayerLDR(renderRef, L"tests_images/test_1017/z_out4.png", L"instid");
  hrRenderSaveGBufferLayerLDR(renderRef, L"tests_images/test_1017/z_out5.png", L"objid");


  return check_images("test_1017", 1, 60);
}