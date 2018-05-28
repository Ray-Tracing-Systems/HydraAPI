#include "tests.h"
#include <iomanip>

#include <stdlib.h>
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

#include "../hydra_api/HR_HDRImageTool.h"
#include "../hydra_api/HydraXMLHelpers.h"

#pragma warning(disable:4996)
#pragma warning(disable:4838)
#pragma warning(disable:4244)

extern GLFWwindow* g_window;

using namespace TEST_UTILS;

bool MTL_TESTS::test_150_gloss_mirror_cos_div()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_150");
  hrSceneLibraryOpen(L"tests_f/test_150", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMaterialRef matPlastic1 = hrMaterialCreate(L"matPlastic1");
  HRMaterialRef matPlastic2 = hrMaterialCreate(L"matPlastic2");
  HRMaterialRef matPlastic3 = hrMaterialCreate(L"matPlastic3");
  HRMaterialRef matPlastic4 = hrMaterialCreate(L"matPlastic4");
  HRMaterialRef matPlastic5 = hrMaterialCreate(L"matPlastic5");
  HRMaterialRef matGray     = hrMaterialCreate(L"matGray");

  const wchar_t* reflColor = L"1.0 1.0 1.0";
  const wchar_t* difColor  = L"0.1 0.1 0.1";

  hrMaterialOpen(matPlastic1, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matPlastic1);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(difColor);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"phong");
    refl.append_child(L"color").append_attribute(L"val").set_value(reflColor);
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.80");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(2.5);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matPlastic1);

  hrMaterialOpen(matPlastic2, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matPlastic2);

    auto diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(difColor);

    auto refl = matNode.append_child(L"reflectivity");

    refl.append_attribute(L"brdf_type").set_value(L"phong");
    refl.append_child(L"color").append_attribute(L"val").set_value(reflColor);
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.85");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(2.5);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matPlastic2);

  hrMaterialOpen(matPlastic3, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matPlastic3);

    auto diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(difColor);

    auto refl = matNode.append_child(L"reflectivity");

    refl.append_attribute(L"brdf_type").set_value(L"phong");
    refl.append_child(L"color").append_attribute(L"val").set_value(reflColor);
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.90");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(2.5);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matPlastic3);

  hrMaterialOpen(matPlastic4, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matPlastic4);

    auto diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(difColor);

    auto refl = matNode.append_child(L"reflectivity");

    refl.append_attribute(L"brdf_type").set_value(L"phong");
    refl.append_child(L"color").append_attribute(L"val").set_value(reflColor);
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.95");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(2.5);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matPlastic4);

  hrMaterialOpen(matPlastic5, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matPlastic5);

    auto diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(difColor);

    auto refl = matNode.append_child(L"reflectivity");

    refl.append_attribute(L"brdf_type").set_value(L"phong");
    refl.append_child(L"color").append_attribute(L"val").set_value(reflColor);
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(2.5);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matPlastic5);

  HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");

  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.25 0.25 0.25");

    auto texNode = hrTextureBind(texChecker, diff.child(L"color"));

    texNode.append_attribute(L"matrix");
		float samplerMatrix[16] = { 8, 0, 0, 0,
			                          0, 8, 0, 0,
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
  HRMeshRef sphere01 = HRMeshFromSimpleMesh(L"sphere00",    CreateSphere(2.0f, 64), matPlastic1.id);
  HRMeshRef sphere02 = HRMeshFromSimpleMesh(L"sphere03",    CreateSphere(2.0f, 64), matPlastic2.id);
  HRMeshRef sphere03 = HRMeshFromSimpleMesh(L"sphere08",    CreateSphere(2.0f, 64), matPlastic3.id);
  HRMeshRef sphere04 = HRMeshFromSimpleMesh(L"sphere09",    CreateSphere(2.0f, 64), matPlastic4.id);
  HRMeshRef sphere05 = HRMeshFromSimpleMesh(L"sphere10",    CreateSphere(2.0f, 64), matPlastic5.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane",    CreatePlane(10.0f), matGray.id);

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

    sizeNode.append_attribute(L"half_length").set_value(12.0f);
    sizeNode.append_attribute(L"half_width").set_value(12.0f);

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"2.0");
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
    camNode.append_child(L"position").text().set(L"0 0 12");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 2048);

  hrRenderOpen(renderRef, HR_OPEN_EXISTING);
  {
    auto node = hrRenderParamNode(renderRef);
    node.force_child(L"method_caustic").text() = L"none";
  }
  hrRenderClose(renderRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Create scene
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  float matrixT[4][4];
  float mTranslate[4][4];
  float mRes[4][4];
  float mRes2[4][4];

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, -1.0f, 0.0f);
  mat4x4_mul(mRes2, mTranslate, mRes);
  mat4x4_transpose(matrixT, mRes2);

  hrMeshInstance(scnRef, planeRef, &matrixT[0][0]);

  const float spheresShift = 4.0f;

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);
  mat4x4_translate(mTranslate, -4.05f, 1.0f, spheresShift);
  mat4x4_mul(mRes2, mTranslate, mRes);
  mat4x4_transpose(matrixT, mRes2); //swap rows and columns

  hrMeshInstance(scnRef, sphere03, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);
  mat4x4_translate(mTranslate, 0.0f, 1.0f, spheresShift);
  mat4x4_mul(mRes2, mTranslate, mRes);
  mat4x4_transpose(matrixT, mRes2); //swap rows and columns

  hrMeshInstance(scnRef, sphere04, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);
  mat4x4_translate(mTranslate, 4.05f, 1.0f, spheresShift);
  mat4x4_mul(mRes2, mTranslate, mRes);
  mat4x4_transpose(matrixT, mRes2); //swap rows and columns

  hrMeshInstance(scnRef, sphere05, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_translate(mTranslate, 0, 16.0f, 0.0);
  mat4x4_transpose(matrixT, mTranslate);

  hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_150/z_out.png");

  return check_images("test_150", 1, 60);
}


bool MTL_TESTS::test_151_gloss_mirror_cos_div2()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_151");
  hrSceneLibraryOpen(L"tests_f/test_151", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMaterialRef matPlastic1 = hrMaterialCreate(L"matPlastic1");
  HRMaterialRef matPlastic2 = hrMaterialCreate(L"matPlastic2");
  HRMaterialRef matPlastic3 = hrMaterialCreate(L"matPlastic3");
  HRMaterialRef matPlastic4 = hrMaterialCreate(L"matPlastic4");
  HRMaterialRef matPlastic5 = hrMaterialCreate(L"matPlastic5");
  HRMaterialRef matGray     = hrMaterialCreate(L"matGray");

  const wchar_t* reflColor = L"1.0 1.0 1.0";
  const wchar_t* difColor  = L"0.1 0.1 0.1";

  hrMaterialOpen(matPlastic1, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matPlastic1);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(difColor);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(reflColor);
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.80");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(2.5);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matPlastic1);

  hrMaterialOpen(matPlastic2, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matPlastic2);

    auto diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(difColor);

    auto refl = matNode.append_child(L"reflectivity");

    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(reflColor);
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.85");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(2.5);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matPlastic2);

  hrMaterialOpen(matPlastic3, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matPlastic3);

    auto diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(difColor);

    auto refl = matNode.append_child(L"reflectivity");

    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(reflColor);
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.90");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(2.5);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matPlastic3);

  hrMaterialOpen(matPlastic4, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matPlastic4);

    auto diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(difColor);

    auto refl = matNode.append_child(L"reflectivity");

    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(reflColor);
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.95");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(2.5);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matPlastic4);

  hrMaterialOpen(matPlastic5, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matPlastic5);

    auto diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(difColor);

    auto refl = matNode.append_child(L"reflectivity");

    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(reflColor);
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(2.5);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matPlastic5);

  HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");

  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.25 0.25 0.25");

    auto texNode = hrTextureBind(texChecker, diff.child(L"color"));

    texNode.append_attribute(L"matrix");
		float samplerMatrix[16] = { 8, 0, 0, 0,
			                          0, 8, 0, 0,
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
  HRMeshRef sphere01 = HRMeshFromSimpleMesh(L"sphere00",    CreateSphere(2.0f, 64), matPlastic1.id);
  HRMeshRef sphere02 = HRMeshFromSimpleMesh(L"sphere03",    CreateSphere(2.0f, 64), matPlastic2.id);
  HRMeshRef sphere03 = HRMeshFromSimpleMesh(L"sphere08",    CreateSphere(2.0f, 64), matPlastic3.id);
  HRMeshRef sphere04 = HRMeshFromSimpleMesh(L"sphere09",    CreateSphere(2.0f, 64), matPlastic4.id);
  HRMeshRef sphere05 = HRMeshFromSimpleMesh(L"sphere10",    CreateSphere(2.0f, 64), matPlastic5.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane",    CreatePlane(10.0f), matGray.id);

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

    sizeNode.append_attribute(L"half_length").set_value(12.0f);
    sizeNode.append_attribute(L"half_width").set_value(12.0f);

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"2.0");
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
    camNode.append_child(L"position").text().set(L"0 0 12");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 2048);

  hrRenderOpen(renderRef, HR_OPEN_EXISTING);
  {
    auto node = hrRenderParamNode(renderRef);
    node.force_child(L"method_caustic").text() = L"none";
  }
  hrRenderClose(renderRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Create scene
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  float matrixT[4][4];
  float mTranslate[4][4];
  float mRes[4][4];
  float mRes2[4][4];

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, -1.0f, 0.0f);
  mat4x4_mul(mRes2, mTranslate, mRes);
  mat4x4_transpose(matrixT, mRes2);

  hrMeshInstance(scnRef, planeRef, &matrixT[0][0]);

  const float spheresShift = 4.0f;

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);
  mat4x4_translate(mTranslate, -4.05f, 1.0f, spheresShift);
  mat4x4_mul(mRes2, mTranslate, mRes);
  mat4x4_transpose(matrixT, mRes2); //swap rows and columns

  hrMeshInstance(scnRef, sphere03, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);
  mat4x4_translate(mTranslate, 0.0f, 1.0f, spheresShift);
  mat4x4_mul(mRes2, mTranslate, mRes);
  mat4x4_transpose(matrixT, mRes2); //swap rows and columns

  hrMeshInstance(scnRef, sphere04, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);
  mat4x4_translate(mTranslate, 4.05f, 1.0f, spheresShift);
  mat4x4_mul(mRes2, mTranslate, mRes);
  mat4x4_transpose(matrixT, mRes2); //swap rows and columns

  hrMeshInstance(scnRef, sphere05, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_translate(mTranslate, 0, 16.0f, 0.0);
  mat4x4_transpose(matrixT, mTranslate);

  hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_151/z_out.png");

  return check_images("test_151", 1, 60);
}


bool MTL_TESTS::test_152_texture_color_replace_mode()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_152");

  hrSceneLibraryOpen(L"tests_f/test_152", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMaterialRef matR    = hrMaterialCreate(L"matR");
  HRMaterialRef matG    = hrMaterialCreate(L"matG");
  HRMaterialRef matB    = hrMaterialCreate(L"matB");
  HRMaterialRef matGray = hrMaterialCreate(L"matGray");

  HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");

  hrMaterialOpen(matR, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matR);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.95 0.05 0.05");
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

    auto texNode = hrTextureBind(texChecker, color);

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 2, 0, 0, -0.5,
                                0, 2, 0, -0.5,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matR);

  hrMaterialOpen(matG, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matG);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.05 0.95 0.05");
    color.append_attribute(L"tex_apply_mode").set_value(L"replace");

    auto texNode = hrTextureBind(texChecker, color);

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 4, 0, 0, -0.5,
                                0, 4, 0, -0.5,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matG);

  hrMaterialOpen(matB, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matB);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.05 0.05 0.95");
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

    auto texNode = hrTextureBind(texChecker, color);

    texNode.append_attribute(L"matrix");
    float sqrtOfTwo = static_cast<float>(2.0f);
    float samplerMatrix[16] = { 4.0f * sqrtOfTwo, -4.0f * sqrtOfTwo,  0.0f,                     0.5f,
                                4.0f * sqrtOfTwo,  4.0f * sqrtOfTwo,  0.0f, -4.0f * sqrtOfTwo + 0.5f,
                                0.0f,              0.0f,              1.0f,                     0.0f,
                                0.0f,              0.0f,              0.0f,                     1.0f };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matB);

  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matGray);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef sphereR = HRMeshFromSimpleMesh(L"sphereR", CreateSphere(2.0f, 64), matR.id);
  HRMeshRef sphereG = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(2.0f, 64), matG.id);
  HRMeshRef sphereB = HRMeshFromSimpleMesh(L"sphereB", CreateSphere(2.0f, 64), matB.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(10.0f), matGray.id);

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

    sizeNode.append_attribute(L"half_length").set_value(L"8.0");
    sizeNode.append_attribute(L"half_width").set_value(L"8.0");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"4.0");
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
    camNode.append_child(L"position").text().set(L"0 10 10");
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

  float matrixT[4][4];
  float mTranslate[4][4];
  float mRes[4][4];
  float mRes2[4][4];

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, -1.0f, 0.0f);
  mat4x4_mul(mRes2, mTranslate, mRes);
  mat4x4_transpose(matrixT, mRes2);

  hrMeshInstance(scnRef, planeRef, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, -4.25f, 1.0f, 0.0f);
  mat4x4_mul(mRes2, mTranslate, mRes);
  mat4x4_transpose(matrixT, mRes2);

  hrMeshInstance(scnRef, sphereR, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 4.25f, 1.0f, 0.0f);
  mat4x4_mul(mRes2, mTranslate, mRes);
  mat4x4_transpose(matrixT, mRes2); //swap rows and columns

  hrMeshInstance(scnRef, sphereB, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, 1.0f, 0.0f);
  mat4x4_mul(mRes2, mTranslate, mRes);
  mat4x4_transpose(matrixT, mRes2); //swap rows and columns

  hrMeshInstance(scnRef, sphereG, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_translate(mTranslate, 0, 16.0f, 0.0);
  mat4x4_transpose(matrixT, mTranslate);

  hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_152/z_out.png");

  return check_images("test_152", 1, 60);
}


bool MTL_TESTS::test_153_opacity_shadow_matte_opacity()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_153");

  hrSceneLibraryOpen(L"tests_f/test_153", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMaterialRef matR    = hrMaterialCreate(L"matR");
  HRMaterialRef matG    = hrMaterialCreate(L"matG");
  HRMaterialRef matB    = hrMaterialCreate(L"matB");
  HRMaterialRef matBG   = hrMaterialCreate(L"matBG");
  HRMaterialRef matGray = hrMaterialCreate(L"matGray");

  HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
  HRTextureNodeRef texPattern = hrTexture2DCreateFromFile(L"data/textures/blur_pattern.bmp");

  hrMaterialOpen(matR, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matR);

    auto diff = matNode.append_child(L"diffuse");

    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.15 0.15");

    auto opacity = matNode.append_child(L"opacity");
    opacity.append_child(L"skip_shadow").append_attribute(L"val").set_value(0);
    opacity.append_child(L"shadow_matte").append_attribute(L"val").set_value(0);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matR);

  hrMaterialOpen(matG, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matG);
    matNode.attribute(L"type").set_value(L"shadow_catcher");

    auto opacity = matNode.append_child(L"opacity");
    opacity.append_child(L"skip_shadow").append_attribute(L"val").set_value(1);

    auto texNode = hrTextureBind(texPattern, opacity);

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 3, 0, 0, 0,
                                0, 3, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };
    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matG);

  hrMaterialOpen(matB, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matB);

    auto diff = matNode.append_child(L"diffuse");

    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 1.0");

    auto opacity = matNode.append_child(L"opacity");
    opacity.append_child(L"skip_shadow").append_attribute(L"val").set_value(0);
    opacity.append_child(L"shadow_matte").append_attribute(L"val").set_value(0);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matB);

  hrMaterialOpen(matBG, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matBG);

    auto diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.25 0.25 1.0");

    auto texNode = hrTextureBind(texChecker, color);

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 32, 0, 0, 0,
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
  hrMaterialClose(matBG);

  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");

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
  HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(1.0f, 64), matR.id);
  HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(1.0f, 64), matR.id);
  HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(1.0f, 64), matR.id);
  HRMeshRef boxBG = HRMeshFromSimpleMesh(L"boxBG", CreateCube(1.0f), matBG.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);
  HRMeshRef planeRef2 = HRMeshFromSimpleMesh(L"my_plane2", CreatePlane(4.0f), matG.id);

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

    sizeNode.append_attribute(L"half_length").set_value(L"2.0");
    sizeNode.append_attribute(L"half_width").set_value(L"2.0");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"20.0");
    VERIFY_XML(lightNode);
  }
  hrLightClose(rectLight);

  HRLightRef sky = hrLightCreate(L"sky");

  hrLightOpen(sky, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sky);

    lightNode.attribute(L"type").set_value(L"sky");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.05 0.05 0.25");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);

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
    camNode.append_child(L"position").text().set(L"0 1 11");
    camNode.append_child(L"look_at").text().set(L"0 -0.5 0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPTNoCaust(CURR_RENDER_DEVICE, 512, 512, 256, 2048);

  hrRenderOpen(renderRef, HR_OPEN_EXISTING);
  {
    auto camNode = hrRenderParamNode(renderRef);
    camNode.force_child(L"evalgbuffer").text() = 1;
  }
  hrRenderClose(renderRef);


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

  //mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
  //hrMeshInstance(scnRef, planeRef, mTranslate.L());

  mTranslate = translate4x4(float3(0.0f, -0.75f, 5.0f));
  hrMeshInstance(scnRef, planeRef2, mTranslate.L());

  mTranslate.identity();
  mTranslate = translate4x4(float3(0.0f, 0.25f, 5.0f));
  hrMeshInstance(scnRef, sph2, mTranslate.L());

  ///////////

  mTranslate.identity();
  mRes.identity();

  mTranslate = translate4x4(float3(0, 15.0f, 0.0));
  mRes = mul(mTranslate, mRes);

  hrLightInstance(scnRef, rectLight, mRes.L());
  hrLightInstance(scnRef, sky, mRes.L());

  ///////////

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef);

  glViewport(0, 0, 512, 512);
  std::vector<int32_t> image(512 * 512);

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      hrRenderGetFrameBufferLDR1i(renderRef, 512, 512, &image[0]);

      glDisable(GL_TEXTURE_2D);
      glDrawPixels(512, 512, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);

      glfwSwapBuffers(g_window);
      glfwPollEvents();
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_153/z_out.png");

  hrRenderSaveGBufferLayerLDR(renderRef, L"tests_images/test_153/z_out2.png", L"depth");
  hrRenderSaveGBufferLayerLDR(renderRef, L"tests_images/test_153/z_out3.png", L"diffcolor");
  hrRenderSaveGBufferLayerLDR(renderRef, L"tests_images/test_153/z_out4.png", L"alpha");
  hrRenderSaveGBufferLayerLDR(renderRef, L"tests_images/test_153/z_out5.png", L"shadow");

  return check_images("test_153", 5, 25);
}


bool MTL_TESTS::test_154_proc_checker_precomp()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_154");

  hrSceneLibraryOpen(L"tests_f/test_154", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMaterialRef matR = hrMaterialCreate(L"matR");
  HRMaterialRef matG = hrMaterialCreate(L"matG");
  HRMaterialRef matB = hrMaterialCreate(L"matB");
  HRMaterialRef matGray = hrMaterialCreate(L"matGray");

  int rep1 = 16;
  int rep2 = 8;
  int rep3 = 4;
  int rep4 = 2;

  HRTextureNodeRef testTex1 = hrTexture2DCreateFromProcLDR(&procTexCheckerLDR, (void*)(&rep1), sizeof(int), -1, -1);
  HRTextureNodeRef testTex2 = hrTexture2DCreateFromProcLDR(&procTexCheckerLDR, (void*)(&rep2), sizeof(int), -1, -1);
  HRTextureNodeRef testTex3 = hrTexture2DCreateFromProcLDR(&procTexCheckerLDR, (void*)(&rep3), sizeof(int), -1, -1);
  HRTextureNodeRef testTex4 = hrTexture2DCreateFromProcLDR(&procTexCheckerLDR, (void*)(&rep4), sizeof(int), 16, 16);


  hrMaterialOpen(matR, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matR);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.95 0.05 0.05");
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

    hrTextureBind(testTex1, color);

    auto texNode = color.child(L"texture");
    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"clamp");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"clamp");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matR);

  hrMaterialOpen(matG, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matG);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.05 0.95 0.05");
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

    hrTextureBind(testTex2, color);

    auto texNode = color.child(L"texture");
    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 4, 0, 0, 0,
                                0, 4, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matG);

  hrMaterialOpen(matB, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matB);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.05 0.05 0.95");
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

    auto texNode = hrTextureBind(testTex3, color);

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 4, 0, 0, 0,
                                0, 4, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matB);

  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");
    color.append_attribute(L"tex_apply_mode").set_value(L"replace");

    auto texNode = hrTextureBind(testTex4, color);

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

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matGray);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matR.id);
  HRMeshRef sphereG = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matG.id);
  HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matB.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(10.0f), matGray.id);

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

    sizeNode.append_attribute(L"half_length").set_value(L"8.0");
    sizeNode.append_attribute(L"half_width").set_value(L"8.0");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"4.0");
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
    camNode.append_child(L"position").text().set(L"0 13 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  int w = 1024;
  int h = 768;

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, w, h, 256, 1024);

  hrRenderOpen(renderRef, HR_OPEN_EXISTING);
  auto node = hrRenderParamNode(renderRef);
  node.append_child(L"scenePrepass").text()  = 1;
  hrRenderClose(renderRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Create scene
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  float matrixT[4][4];
  float mTranslate[4][4];
  float mRes[4][4];
  float mRes2[4][4];
  float mRot[4][4];

  const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, -1.0f, 0.0f);
  mat4x4_mul(mRes2, mTranslate, mRes);
  mat4x4_transpose(matrixT, mRes2);

  hrMeshInstance(scnRef, planeRef, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);
  mat4x4_identity(mRot);

  mat4x4_translate(mTranslate, -4.0f, 1.0f, 4.0f);
  mat4x4_rotate_Y(mRot, mRot, 60.0f*DEG_TO_RAD);
  mat4x4_mul(mRes2, mTranslate, mRot);
  mat4x4_transpose(matrixT, mRes2);

  hrMeshInstance(scnRef, cubeR, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);
  mat4x4_identity(mRot);

  mat4x4_translate(mTranslate, 4.0f, 1.0f, 4.5f);
  mat4x4_rotate_Y(mRot, mRot, -60.0f*DEG_TO_RAD);
  mat4x4_rotate_X(mRot, mRot, 80.0f*DEG_TO_RAD);
  mat4x4_mul(mRes2, mTranslate, mRot);
  mat4x4_transpose(matrixT, mRes2); //swap rows and columns

  hrMeshInstance(scnRef, torusB, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, 2.0f, -1.0f);
  mat4x4_mul(mRes2, mTranslate, mRes);
  mat4x4_transpose(matrixT, mRes2); //swap rows and columns

  hrMeshInstance(scnRef, sphereG, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_translate(mTranslate, 0, 12.0f, 0);
  mat4x4_transpose(matrixT, mTranslate);

  hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

  ///////////

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef);

  glViewport(0, 0, w, h);
  std::vector<int32_t> image(w * h);

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      hrRenderGetFrameBufferLDR1i(renderRef, w, h, &image[0]);

      glDisable(GL_TEXTURE_2D);
      glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);

      glfwSwapBuffers(g_window);
      glfwPollEvents();
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_154/z_out.png");

  return check_images("test_154", 1, 60);
}

bool MTL_TESTS::test_155_proc_checker_HDR_precomp()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_155");

  hrSceneLibraryOpen(L"tests_f/test_155", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMaterialRef matR = hrMaterialCreate(L"matR");
  HRMaterialRef matG = hrMaterialCreate(L"matG");
  HRMaterialRef matB = hrMaterialCreate(L"matB");
  HRMaterialRef matGray = hrMaterialCreate(L"matGray");

  int rep1 = 2;
  int rep2 = 4;
  int rep3 = 8;
  int rep4 = 2;

  HRTextureNodeRef testTex1 = hrTexture2DCreateFromProcHDR(&procTexCheckerHDR, (void*)(&rep1), sizeof(int), -1, -1);
  HRTextureNodeRef testTex2 = hrTexture2DCreateFromProcHDR(&procTexCheckerHDR, (void*)(&rep2), sizeof(int), -1, -1);
  HRTextureNodeRef testTex3 = hrTexture2DCreateFromProcHDR(&procTexCheckerHDR, (void*)(&rep3), sizeof(int), -1, -1);
  HRTextureNodeRef testTex4 = hrTexture2DCreateFromProcHDR(&procTexCheckerHDR, (void*)(&rep4), sizeof(int), 16, 16);


  hrMaterialOpen(matR, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matR);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.95 0.05 0.05");
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

    hrTextureBind(testTex1, color);

    auto texNode = color.child(L"texture");
    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"clamp");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"clamp");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matR);

  hrMaterialOpen(matG, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matG);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.05 0.95 0.05");
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

    hrTextureBind(testTex2, color);

    auto texNode = color.child(L"texture");
    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 4, 0, 0, 0,
                                0, 4, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matG);

  hrMaterialOpen(matB, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matB);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.05 0.05 0.95");
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

    auto texNode = hrTextureBind(testTex3, color);

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 4, 0, 0, 0,
                                0, 4, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matB);

  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");
    color.append_attribute(L"tex_apply_mode").set_value(L"replace");

    auto texNode = hrTextureBind(testTex4, color);

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

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matGray);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matR.id);
  HRMeshRef sphereG = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matG.id);
  HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matB.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(10.0f), matGray.id);

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

    sizeNode.append_attribute(L"half_length").set_value(L"8.0");
    sizeNode.append_attribute(L"half_width").set_value(L"8.0");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"4.0");
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
    camNode.append_child(L"position").text().set(L"0 13 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 1024);

  hrRenderOpen(renderRef, HR_OPEN_EXISTING);
  auto node = hrRenderParamNode(renderRef);
  node.append_child(L"scenePrepass").text()  = 1;
  hrRenderClose(renderRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Create scene
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  float matrixT[4][4];
  float mTranslate[4][4];
  float mRes[4][4];
  float mRes2[4][4];
  float mRot[4][4];

  const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, -1.0f, 0.0f);
  mat4x4_mul(mRes2, mTranslate, mRes);
  mat4x4_transpose(matrixT, mRes2);

  hrMeshInstance(scnRef, planeRef, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);
  mat4x4_identity(mRot);

  mat4x4_translate(mTranslate, -4.0f, 1.0f, 4.0f);
  mat4x4_rotate_Y(mRot, mRot, 60.0f*DEG_TO_RAD);
  mat4x4_mul(mRes2, mTranslate, mRot);
  mat4x4_transpose(matrixT, mRes2);

  hrMeshInstance(scnRef, cubeR, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);
  mat4x4_identity(mRot);

  mat4x4_translate(mTranslate, 4.0f, 1.0f, 4.5f);
  mat4x4_rotate_Y(mRot, mRot, -60.0f*DEG_TO_RAD);
  mat4x4_rotate_X(mRot, mRot, 80.0f*DEG_TO_RAD);
  mat4x4_mul(mRes2, mTranslate, mRot);
  mat4x4_transpose(matrixT, mRes2); //swap rows and columns

  hrMeshInstance(scnRef, torusB, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, 2.0f, -1.0f);
  mat4x4_mul(mRes2, mTranslate, mRes);
  mat4x4_transpose(matrixT, mRes2); //swap rows and columns

  hrMeshInstance(scnRef, sphereG, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_translate(mTranslate, 0, 12.0f, 0);
  mat4x4_transpose(matrixT, mTranslate);

  hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_155/z_out.png");

  return check_images("test_155", 1, 60);
}

bool MTL_TESTS::test_156_proc_checker_precomp_update()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_156");

  hrSceneLibraryOpen(L"tests_f/test_156", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMaterialRef matR = hrMaterialCreate(L"matR");
  HRMaterialRef matG = hrMaterialCreate(L"matG");
  HRMaterialRef matB = hrMaterialCreate(L"matB");
  HRMaterialRef matGray = hrMaterialCreate(L"matGray");

  int rep1 = 16;
  int rep2 = 16;
  int rep3 = 16;
  int rep4 = 2;

  HRTextureNodeRef testTex1 = hrTexture2DCreateFromProcLDR(&procTexCheckerLDR, (void*)(&rep1), sizeof(int), -1, -1);
  HRTextureNodeRef testTex2 = hrTexture2DCreateFromProcLDR(&procTexCheckerLDR, (void*)(&rep2), sizeof(int), -1, -1);
  HRTextureNodeRef testTex3 = hrTexture2DCreateFromProcLDR(&procTexCheckerLDR, (void*)(&rep3), sizeof(int), -1, -1);
  HRTextureNodeRef testTex4 = hrTexture2DCreateFromProcLDR(&procTexCheckerLDR, (void*)(&rep4), sizeof(int), 16, 16);


  hrMaterialOpen(matR, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matR);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.95 0.05 0.05");
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

    hrTextureBind(testTex1, color);

    auto texNode = color.child(L"texture");
    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"clamp");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"clamp");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matR);

  hrMaterialOpen(matG, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matG);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.05 0.95 0.05");
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

    hrTextureBind(testTex2, color);

    auto texNode = color.child(L"texture");
    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 4, 0, 0, 0,
                                0, 4, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matG);

  hrMaterialOpen(matB, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matB);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.05 0.05 0.95");
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

    auto texNode = hrTextureBind(testTex3, color);

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 4, 0, 0, 0,
                                0, 4, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matB);

  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");
    color.append_attribute(L"tex_apply_mode").set_value(L"replace");

    auto texNode = hrTextureBind(testTex4, color);

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

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matGray);


  rep1 = 2;
  rep2 = 2;
  rep3 = 2;

  testTex1 = hrTexture2DUpdateFromProcLDR(testTex1, &procTexCheckerLDR, (void*)(&rep1), sizeof(int), -1, -1);
  testTex2 = hrTexture2DUpdateFromProcLDR(testTex2, &procTexCheckerLDR, (void*)(&rep2), sizeof(int), -1, -1);
  testTex3 = hrTexture2DUpdateFromProcLDR(testTex3, &procTexCheckerLDR, (void*)(&rep3), sizeof(int), -1, -1);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matR.id);
  HRMeshRef sphereG = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matG.id);
  HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matB.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(10.0f), matGray.id);

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

    sizeNode.append_attribute(L"half_length").set_value(L"8.0");
    sizeNode.append_attribute(L"half_width").set_value(L"8.0");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"4.0");
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
    camNode.append_child(L"position").text().set(L"0 13 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 1024);
  hrRenderOpen(renderRef, HR_OPEN_EXISTING);
  auto node = hrRenderParamNode(renderRef);
  node.append_child(L"scenePrepass").text()  = 1;
  hrRenderClose(renderRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Create scene
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  float matrixT[4][4];
  float mTranslate[4][4];
  float mRes[4][4];
  float mRes2[4][4];
  float mRot[4][4];

  const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, -1.0f, 0.0f);
  mat4x4_mul(mRes2, mTranslate, mRes);
  mat4x4_transpose(matrixT, mRes2);

  hrMeshInstance(scnRef, planeRef, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);
  mat4x4_identity(mRot);

  mat4x4_translate(mTranslate, -4.0f, 1.0f, 4.0f);
  mat4x4_rotate_Y(mRot, mRot, 60.0f*DEG_TO_RAD);
  mat4x4_mul(mRes2, mTranslate, mRot);
  mat4x4_transpose(matrixT, mRes2);

  hrMeshInstance(scnRef, cubeR, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);
  mat4x4_identity(mRot);

  mat4x4_translate(mTranslate, 4.0f, 1.0f, 4.5f);
  mat4x4_rotate_Y(mRot, mRot, -60.0f*DEG_TO_RAD);
  mat4x4_rotate_X(mRot, mRot, 80.0f*DEG_TO_RAD);
  mat4x4_mul(mRes2, mTranslate, mRot);
  mat4x4_transpose(matrixT, mRes2); //swap rows and columns

  hrMeshInstance(scnRef, torusB, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, 2.0f, -1.0f);
  mat4x4_mul(mRes2, mTranslate, mRes);
  mat4x4_transpose(matrixT, mRes2); //swap rows and columns

  hrMeshInstance(scnRef, sphereG, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_translate(mTranslate, 0, 12.0f, 0);
  mat4x4_transpose(matrixT, mTranslate);

  hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_156/z_out.png");

  return check_images("test_156", 1, 60);
}

bool MTL_TESTS::test_157_proc_checker_precomp_remap()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_157");

  hrSceneLibraryOpen(L"tests_f/test_157", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMaterialRef matR = hrMaterialCreate(L"matR");
  HRMaterialRef matG = hrMaterialCreate(L"matG");
  HRMaterialRef matB = hrMaterialCreate(L"matB");
  HRMaterialRef matV = hrMaterialCreate(L"matV");
  HRMaterialRef matC = hrMaterialCreate(L"matC");
  HRMaterialRef matGray = hrMaterialCreate(L"matGray");

  int rep1 = 2;
  int rep2 = 16;
  int rep3 = 32;

  HRTextureNodeRef testTex1 = hrTexture2DCreateFromProcLDR(&procTexCheckerLDR, (void*)(&rep1), sizeof(int), -1, -1);
  HRTextureNodeRef testTex2 = hrTexture2DCreateFromProcLDR(&procTexCheckerLDR, (void*)(&rep2), sizeof(int), -1, -1);
  HRTextureNodeRef testTex3 = hrTexture2DCreateFromProcLDR(&procTexCheckerLDR, (void*)(&rep3), sizeof(int), -1, -1);


  hrMaterialOpen(matR, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matR);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.95 0.05 0.05");
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

    hrTextureBind(testTex1, color);

    auto texNode = color.child(L"texture");
    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"clamp");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"clamp");
    texNode.append_attribute(L"input_gamma").set_value(2.2f);
    texNode.append_attribute(L"input_alpha").set_value(L"rgb");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matR);

  hrMaterialOpen(matG, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matG);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.05 0.95 0.05");
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

    hrTextureBind(testTex2, color);

    auto texNode = color.child(L"texture");
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

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matG);

  hrMaterialOpen(matB, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matB);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.05 0.05 0.95");
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

    hrTextureBind(testTex2, color);

    auto texNode = color.child(L"texture");
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

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matB);

  hrMaterialOpen(matV, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matV);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.95 0.05 0.95");
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

    hrTextureBind(testTex3, color);

    auto texNode = color.child(L"texture");
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

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matV);

  hrMaterialOpen(matC, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matC);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.05 0.95 0.95");
    color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

    hrTextureBind(testTex3, color);

    auto texNode = color.child(L"texture");
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

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matC);


  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");
    color.append_attribute(L"tex_apply_mode").set_value(L"replace");

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matGray);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef sphereR = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matR.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(10.0f), matGray.id);

  SimpleMesh torus = CreateTorus(0.2f, 0.5f, 32, 32);
  SimpleMesh cube2 = CreateCube(0.25f);
  HRMeshRef torusRef = hrMeshCreate(L"my_torus");

  hrMeshOpen(torusRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(torusRef, L"pos", &torus.vPos[0]);
    hrMeshVertexAttribPointer4f(torusRef, L"norm", &torus.vNorm[0]);
    hrMeshVertexAttribPointer2f(torusRef, L"texcoord", &torus.vTexCoord[0]);

    for (size_t i = 0; i < torus.matIndices.size() / 3; i++)
      torus.matIndices[i] = matR.id;

    for (size_t i = 1 * torus.matIndices.size() / 3; i < 2 * torus.matIndices.size() / 3; i++)
      torus.matIndices[i] = matG.id;

    for (size_t i = 2 * torus.matIndices.size() / 3; i < torus.matIndices.size(); i++)
      torus.matIndices[i] = matB.id;

    //hrMeshMaterialId(torusRef, mat0.id);
    hrMeshPrimitiveAttribPointer1i(torusRef, L"mind", &torus.matIndices[0]);
    hrMeshAppendTriangles3(torusRef, int32_t(torus.triIndices.size()), &torus.triIndices[0]);

    // test second call of hrMeshAppendTriangles3
    //
    hrMeshVertexAttribPointer4f(torusRef, L"pos", &cube2.vPos[0]);
    hrMeshVertexAttribPointer4f(torusRef, L"norm", &cube2.vNorm[0]);
    hrMeshVertexAttribPointer2f(torusRef, L"texcoord", &cube2.vTexCoord[0]);

    int cubeMatIndices[12] = { 0, 0, 1, 1, 2, 2, 1, 1, 2, 2, 3, 3 };

    hrMeshPrimitiveAttribPointer1i(torusRef, L"mind", cubeMatIndices);
    hrMeshAppendTriangles3(torusRef, int(cube2.triIndices.size()), &cube2.triIndices[0]);
  }
  hrMeshClose(torusRef);
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

    sizeNode.append_attribute(L"half_length").set_value(L"8.0");
    sizeNode.append_attribute(L"half_width").set_value(L"8.0");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"4.0");
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
    camNode.append_child(L"position").text().set(L"0 13 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 1024);

  hrRenderOpen(renderRef, HR_OPEN_EXISTING);
  auto node = hrRenderParamNode(renderRef);
  node.append_child(L"scenePrepass").text()  = 1;
  hrRenderClose(renderRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Create scene
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  float matrixT[4][4];
  float mTranslate[4][4];
  float mRes[4][4];
  float mRes2[4][4];
  float mRot[4][4];
  float mScale[4][4];

  const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 0.0f, -1.0f, 0.0f);
  mat4x4_mul(mRes2, mTranslate, mRes);
  mat4x4_transpose(matrixT, mRes2);

  hrMeshInstance(scnRef, planeRef, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);
  mat4x4_identity(mRot);
  mat4x4_identity(mScale);

  mat4x4_scale(mScale, mScale, 0.75);
  mat4x4_translate(mTranslate, -4.0f, 1.0f, 4.0f);
  mat4x4_rotate_Y(mRot, mRot, 60.0f*DEG_TO_RAD);
  mat4x4_mul(mRes2, mRot, mScale);
  mat4x4_mul(mRes, mTranslate, mRes2);
  mat4x4_transpose(matrixT, mRes);

  int32_t remap[2] = {matR.id, matG.id};
  hrMeshInstance(scnRef, sphereR, &matrixT[0][0], remap, 2);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);
  mat4x4_identity(mRes2);
  mat4x4_identity(mRot);
  mat4x4_identity(mScale);

  mat4x4_scale(mScale, mScale, 0.75);
  mat4x4_translate(mTranslate, 4.0f, 1.0f, 4.5f);
  mat4x4_rotate_Y(mRot, mRot, -60.0f*DEG_TO_RAD);
  mat4x4_rotate_X(mRot, mRot, 80.0f*DEG_TO_RAD);
  mat4x4_mul(mRes2, mRot, mScale);
  mat4x4_mul(mRes, mTranslate, mRes2);
  mat4x4_transpose(matrixT, mRes);

  hrMeshInstance(scnRef, sphereR, &matrixT[0][0], remap, 2);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);
  mat4x4_identity(mRes2);
  mat4x4_identity(mRot);
  mat4x4_identity(mScale);

  mat4x4_scale(mScale, mScale, 0.75);
  mat4x4_translate(mTranslate, 0.0f, 2.0f, -1.0f);
  mat4x4_mul(mRes2, mTranslate, mScale);
  mat4x4_transpose(matrixT, mRes2); //swap rows and columns

  hrMeshInstance(scnRef, sphereR, &matrixT[0][0]);

  ///////////

  int32_t remap2[6] = { matR.id, matV.id,
                        matG.id, matC.id,
                        matB.id, matG.id};

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);
  mat4x4_identity(mScale);

  mat4x4_scale(mScale, mScale, 2.0);
  mat4x4_translate(mTranslate, 0.0f, 6.0f, -1.0f);
  mat4x4_mul(mRes2, mTranslate, mScale);
  mat4x4_transpose(matrixT, mRes2); //swap rows and columns

  hrMeshInstance(scnRef, torusRef, &matrixT[0][0], remap2, 6);


  int32_t remap3[4] = { matR.id, matB.id,
                        matG.id, matB.id};

  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, 4.0f, 6.0f, -1.0f);
  mat4x4_mul(mRes2, mTranslate, mRes);
  mat4x4_mul(mRes2, mTranslate, mScale);
  mat4x4_transpose(matrixT, mRes2); //swap rows and columns

  hrMeshInstance(scnRef, torusRef, &matrixT[0][0], remap3, 4);


  mat4x4_identity(mTranslate);
  mat4x4_identity(mRes);

  mat4x4_translate(mTranslate, -4.0f, 6.0f, -1.0f);
  mat4x4_mul(mRes2, mTranslate, mRes);
  mat4x4_mul(mRes2, mTranslate, mScale);
  mat4x4_transpose(matrixT, mRes2); //swap rows and columns

  hrMeshInstance(scnRef, torusRef, &matrixT[0][0]);

  ///////////

  mat4x4_identity(mTranslate);
  mat4x4_translate(mTranslate, 0, 12.0f, 0);
  mat4x4_transpose(matrixT, mTranslate);

  hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_157/z_out.png");

  return check_images("test_157", 1, 60);
}