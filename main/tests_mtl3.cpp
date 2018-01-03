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
