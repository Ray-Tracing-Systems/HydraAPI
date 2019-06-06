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


namespace MTL_TESTS
{
  bool test_131_blend_simple()
  {
    initGLIfNeeded();

    hrErrorCallerPlace(L"test_131");

    hrSceneLibraryOpen(L"tests_f/test_131", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMaterialRef matGold    = hrMaterialCreate(L"matGold");
    HRMaterialRef matSilver  = hrMaterialCreate(L"matSilver");
    HRMaterialRef matLacquer = hrMaterialCreate(L"matLacquer");
    HRMaterialRef matGlass   = hrMaterialCreate(L"matGlass");
    HRMaterialRef matBricks1 = hrMaterialCreate(L"matBricks1");
    HRMaterialRef matBricks2 = hrMaterialCreate(L"matBricks2");


    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
    HRTextureNodeRef texYinYang = hrTexture2DCreateFromFile(L"data/textures/yinyang.png");

    hrMaterialOpen(matGold, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGold);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.88 0.61 0.05");

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      //refl.append_child(L"color").append_attribute(L"val").set_value(L"0.88 0.61 0.05");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.0 0.0");
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
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matSilver);

    hrMaterialOpen(matLacquer, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matLacquer);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.05 0.05 0.05");

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(1.5);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matLacquer);

    hrMaterialOpen(matGlass, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGlass);

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.1 0.1 0.1 ");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(1.5f);

      auto transp = matNode.append_child(L"transparency");
      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"0.9 0.9 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"IOR").append_attribute(L"val").set_value(1.5f);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGlass);

    hrMaterialOpen(matBricks1, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBricks1);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.2 0.2 0.75");
      color.append_attribute(L"tex_apply_mode ").set_value(L"multiply");

			auto texNode = hrTextureBind(texChecker, diff.child(L"color"));

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
    hrMaterialClose(matBricks1);

    hrMaterialOpen(matBricks2, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBricks2);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.1 0.1 0.1");
      color.append_attribute(L"tex_apply_mode ").set_value(L"multiply");

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

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.9 0.9 0.9");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(1.5);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matBricks2);

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    HRMaterialRef matBlend1 = hrMaterialCreateBlend(L"matBlend1", matGold, matSilver);
    HRMaterialRef matBlend2 = hrMaterialCreateBlend(L"matBlend2", matLacquer, matGlass);
    HRMaterialRef matBlend3 = hrMaterialCreateBlend(L"matBlend3", matBricks1, matBricks2);

    hrMaterialOpen(matBlend1, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBlend1);

      auto blend = matNode.append_child(L"blend");
      blend.append_attribute(L"type").set_value(L"mask_blend");

      auto mask = blend.append_child(L"mask");
      mask.append_attribute(L"val").set_value(1.0f);

      auto texNode = mask.append_child(L"texture");
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

      hrTextureBind(texChecker, mask);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matBlend1);

    hrMaterialOpen(matBlend2, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBlend2);

      auto blend = matNode.append_child(L"blend");
      blend.append_attribute(L"type").set_value(L"mask_blend");

      auto mask = blend.append_child(L"mask");
      mask.append_attribute(L"val").set_value(1.0f);

      auto texNode = mask.append_child(L"texture");
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

      hrTextureBind(texYinYang, mask);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matBlend2);

    hrMaterialOpen(matBlend3, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBlend3);

      auto blend = matNode.append_child(L"blend");
      blend.append_attribute(L"type").set_value(L"mask_blend");

      auto mask = blend.append_child(L"mask");
      mask.append_attribute(L"val").set_value(1.0f);

      auto texNode = mask.append_child(L"texture");
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

      hrTextureBind(texChecker, mask);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matBlend3);



    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matBlend1.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matBlend2.id);
    HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), matBlend3.id);
    HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(18.0f), matGray.id);
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

      sizeNode.append_attribute(L"half_length").set_value(L"10.0");
      sizeNode.append_attribute(L"half_width").set_value(L"10.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");
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
      camNode.append_child(L"position").text().set(L"0 10 8");
      camNode.append_child(L"look_at").text().set(L"0 0 0");
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 4096); // L"HydraModern"


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scene
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace HydraLiteMath;

    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mScale;
    float4x4 mRes;

    const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();


    mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
    mRot = rotate_Y_4x4(180.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrMeshInstance(scnRef, cubeOpen, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-4.25f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(4.25f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph3, mRes.L());


    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0, 16.0f, 0.0));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, rectLight, mRes.L());

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
        std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
        std::cout.precision(pres);

        glfwSwapBuffers(g_window);
        glfwPollEvents();
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_131/z_out.png");

    return check_images("test_131", 1, 25);
  }


  bool test_132_blend_recursive()
  {
    initGLIfNeeded();

    hrErrorCallerPlace(L"test_132");

    hrSceneLibraryOpen(L"tests_f/test_132", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMaterialRef matGold = hrMaterialCreate(L"matGold");
    HRMaterialRef matSilver = hrMaterialCreate(L"matSilver");
    HRMaterialRef matBricks1 = hrMaterialCreate(L"matBricks1");
    HRMaterialRef matBricks2 = hrMaterialCreate(L"matBricks2");


    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
    HRTextureNodeRef texYinYang = hrTexture2DCreateFromFile(L"data/textures/yinyang.png");

    hrMaterialOpen(matGold, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGold);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.88 0.61 0.05");

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      //refl.append_child(L"color").append_attribute(L"val").set_value(L"0.88 0.61 0.05");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.0 0.0");
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
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matSilver);


    hrMaterialOpen(matBricks1, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBricks1);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.2 0.2 0.75");
      color.append_attribute(L"tex_apply_mode ").set_value(L"multiply");

			auto texNode = hrTextureBind(texChecker, diff.child(L"color"));

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
    hrMaterialClose(matBricks1);

    hrMaterialOpen(matBricks2, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBricks2);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.1 0.1 0.1");
      color.append_attribute(L"tex_apply_mode ").set_value(L"multiply");

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

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.9 0.9 0.9");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(1.5);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matBricks2);

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    HRMaterialRef matBlend1  = hrMaterialCreateBlend(L"matBlend1", matGold, matSilver);
    HRMaterialRef matBlend2  = hrMaterialCreateBlend(L"matBlend2", matBricks1, matBricks2);
    HRMaterialRef matBlendR1 = hrMaterialCreateBlend(L"matBlendR1", matBlend1, matBlend2);
    HRMaterialRef matBlendR2 = hrMaterialCreateBlend(L"matBlendR2", matBlend1, matBricks1);
    HRMaterialRef matBlendR3 = hrMaterialCreateBlend(L"matBlendR3", matBlend1, matBricks2);

    hrMaterialOpen(matBlend1, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBlend1);

      auto blend = matNode.append_child(L"blend");
      blend.append_attribute(L"type").set_value(L"mask_blend");

      auto mask = blend.append_child(L"mask");
      mask.append_attribute(L"val").set_value(1.0f);

      auto texNode = mask.append_child(L"texture");
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

      hrTextureBind(texChecker, mask);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matBlend1);

    hrMaterialOpen(matBlend2, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBlend2);

      auto blend = matNode.append_child(L"blend");
      blend.append_attribute(L"type").set_value(L"mask_blend");

      auto mask = blend.append_child(L"mask");
      mask.append_attribute(L"val").set_value(1.0f);

      auto texNode = mask.append_child(L"texture");
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

      hrTextureBind(texYinYang, mask);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matBlend2);

    hrMaterialOpen(matBlendR1, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBlendR1);

      auto blend = matNode.append_child(L"blend");
      blend.append_attribute(L"type").set_value(L"mask_blend");

      auto mask = blend.append_child(L"mask");
      mask.append_attribute(L"val").set_value(1.0f);

      auto texNode = mask.append_child(L"texture");
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

      hrTextureBind(texChecker, mask);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matBlendR1);


    hrMaterialOpen(matBlendR2, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBlendR2);

      auto blend = matNode.append_child(L"blend");
      blend.append_attribute(L"type").set_value(L"mask_blend");

      auto mask = blend.append_child(L"mask");
      mask.append_attribute(L"val").set_value(1.0f);

      auto texNode = mask.append_child(L"texture");
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

      hrTextureBind(texYinYang, mask);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matBlendR2);

    hrMaterialOpen(matBlendR3, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBlendR3);

      auto blend = matNode.append_child(L"blend");
      blend.append_attribute(L"type").set_value(L"mask_blend");

      auto mask = blend.append_child(L"mask");
      mask.append_attribute(L"val").set_value(1.0f);

      auto texNode = mask.append_child(L"texture");
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

      hrTextureBind(texChecker, mask);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matBlendR3);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matBlendR1.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matBlendR2.id);
    HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), matBlendR3.id);
    HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(18.0f), matGray.id);
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

      sizeNode.append_attribute(L"half_length").set_value(L"10.0");
      sizeNode.append_attribute(L"half_width").set_value(L"10.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");
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
      camNode.append_child(L"position").text().set(L"0 10 8");
      camNode.append_child(L"look_at").text().set(L"0 0 0");
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 4096); // L"HydraModern"


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scene
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace HydraLiteMath;

    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mScale;
    float4x4 mRes;

    const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();


    mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
    mRot = rotate_Y_4x4(180.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrMeshInstance(scnRef, cubeOpen, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-4.25f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(4.25f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph3, mRes.L());


    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0, 16.0f, 0.0));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, rectLight, mRes.L());

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
        std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
        std::cout.precision(pres);

        glfwSwapBuffers(g_window);
        glfwPollEvents();
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_132/z_out.png");

    return check_images("test_132", 1, 25);
  }

  bool test_140_blend_emission()
  {
    initGLIfNeeded();

    hrErrorCallerPlace(L"test_140");

    hrSceneLibraryOpen(L"tests_f/test_140", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMaterialRef matEmission = hrMaterialCreate(L"matEmission");
    HRMaterialRef matOpacity = hrMaterialCreate(L"matOpacity");
    HRMaterialRef matPlastic = hrMaterialCreate(L"matPlastic");
    HRMaterialRef matGlassBump = hrMaterialCreate(L"matGlassBump");
    HRMaterialRef matBG = hrMaterialCreate(L"matBG");
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
    HRTextureNodeRef texNormal = hrTexture2DCreateFromFile(L"data/textures/normal_map.jpg");
    HRTextureNodeRef texOrnament = hrTexture2DCreateFromFile(L"data/textures/ornament.jpg");
    HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/building.hdr");
    HRTextureNodeRef texEmiss = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
    HRTextureNodeRef texGradient = hrTexture2DCreateFromFile(L"data/textures/gradient.png");
    HRTextureNodeRef texBigleaf3 = hrTexture2DCreateFromFile(L"data/textures/bigleaf3.tga");
    

    hrMaterialOpen(matEmission, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matEmission);

      auto emission = matNode.append_child(L"emission");
      emission.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);

      auto color = emission.append_child(L"color");
      color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      color.append_attribute(L"tex_apply_mode ").set_value(L"replace");

      auto texNode = hrTextureBind(texEmiss, emission.child(L"color"));

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
    hrMaterialClose(matEmission);

    hrMaterialOpen(matOpacity, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matOpacity);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.5 0.25");

      auto opacity = matNode.append_child(L"opacity");
      opacity.append_child(L"skip_shadow").append_attribute(L"val").set_value(0);

      auto texNode = hrTextureBind(texBigleaf3, opacity);

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
    hrMaterialClose(matOpacity);

    hrMaterialOpen(matPlastic, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matPlastic);

      auto diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.25 0.8 0.25");

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(0.9f);
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5f);

      auto displacement = matNode.append_child(L"displacement");
      auto heightNode = displacement.append_child(L"height_map");

      displacement.append_attribute(L"type").set_value(L"height_bump");
      heightNode.append_attribute(L"amount").set_value(1.0f);

      auto texNode2 = hrTextureBind(texOrnament, heightNode);

      texNode2.append_attribute(L"matrix");
      float samplerMatrix2[16] = { 2, 0, 0, 0,
                                   0, 2, 0, 0,
                                   0, 0, 1, 0,
                                   0, 0, 0, 1 };

      texNode2.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode2.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode2.append_attribute(L"input_gamma").set_value(2.2f);
      texNode2.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode2, L"matrix", samplerMatrix2);

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matPlastic);

    hrMaterialOpen(matGlassBump, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGlassBump);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5f);

      auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.5f);

      auto displacement = matNode.append_child(L"displacement");
      auto heightNode = displacement.append_child(L"height_map");

      displacement.append_attribute(L"type").set_value(L"height_bump");
      heightNode.append_attribute(L"amount").set_value(0.1f);
      heightNode.append_attribute(L"smooth_lvl").set_value(0.0f);

      auto texNode2 = hrTextureBind(texOrnament, heightNode);

      texNode2.append_attribute(L"matrix");
      float samplerMatrix2[16] = { 2, 0, 0, 0,
                                   0, 2, 0, 0,
                                   0, 0, 1, 0,
                                   0, 0, 0, 1 };

      texNode2.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode2.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode2.append_attribute(L"input_gamma").set_value(2.2f);
      texNode2.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode2, L"matrix", samplerMatrix2);

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matGlassBump);

    hrMaterialOpen(matBG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBG);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.25 0.25 0.8");
      color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

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
      color.append_attribute(L"val").set_value(L"0.7 0.7 0.7");
      color.append_attribute(L"tex_apply_mode").set_value(L"replace");

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

    HRMaterialRef matBlend1 = hrMaterialCreateBlend(L"matBlend1", matEmission, matOpacity);
    HRMaterialRef matBlend2 = hrMaterialCreateBlend(L"matBlend2", matEmission, matPlastic);
    HRMaterialRef matBlend3 = hrMaterialCreateBlend(L"matBlend3", matEmission, matGlassBump);

    hrMaterialOpen(matBlend1, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBlend1);

      auto blend = matNode.append_child(L"blend");
      blend.append_attribute(L"type").set_value(L"mask_blend");

      auto mask = blend.append_child(L"mask");
      mask.append_attribute(L"val").set_value(1.0f);

      auto texNode = mask.append_child(L"texture");
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

      hrTextureBind(texGradient, mask);
      VERIFY_XML(matNode);
    }
    hrMaterialClose(matBlend1);

    hrMaterialOpen(matBlend2, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBlend2);

      auto blend = matNode.append_child(L"blend");
      blend.append_attribute(L"type").set_value(L"mask_blend");

      auto mask = blend.append_child(L"mask");
      mask.append_attribute(L"val").set_value(1.0f);

      auto texNode = mask.append_child(L"texture");
      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 2, 0, 0, 0,
                                  0, 2, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

      hrTextureBind(texOrnament, mask);
      VERIFY_XML(matNode);
    }
    hrMaterialClose(matBlend2);

    hrMaterialOpen(matBlend3, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBlend3);

      auto blend = matNode.append_child(L"blend");
      blend.append_attribute(L"type").set_value(L"mask_blend");

      auto mask = blend.append_child(L"mask");
      mask.append_attribute(L"val").set_value(1.0f);

      auto texNode = mask.append_child(L"texture");
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

      hrTextureBind(texChecker, mask);
      VERIFY_XML(matNode);
    }
    hrMaterialClose(matBlend3);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1     = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matBlend1.id);
    HRMeshRef sph2     = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matBlend2.id);
    HRMeshRef sph3     = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), matBlend3.id);
    HRMeshRef boxBG    = HRMeshFromSimpleMesh(L"boxBG", CreateCube(1.0f), matBG.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRLightRef sky = hrLightCreate(L"sky");

    hrLightOpen(sky, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sky);

      lightNode.attribute(L"type").set_value(L"sky");
      lightNode.attribute(L"distribution").set_value(L"map");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"0.1");

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
      camNode.append_child(L"position").text().set(L"0 1 16");
      camNode.append_child(L"look_at").text().set(L"0 1 0");
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 2048);


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
    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, planeRef, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-4.25f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(4.25f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph3, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, 0.0f, 1.0f));
    mScale = scale4x4(float3(16.0f, 8.0f, 0.5f));
    mRes = mul(mScale, mRes);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, boxBG, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    mTranslate = translate4x4(float3(0, 10.0f, 0.0));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, rectLight, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    hrLightInstance(scnRef, sky, mRes.L());

    ///////////

    hrSceneClose(scnRef);
    hrFlush(scnRef, renderRef);

    while (true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

      if (info.haveUpdateFB)
      {
        
        auto pres = std::cout.precision(2);
        std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
        std::cout.precision(pres);

        glfwSwapBuffers(g_window);
        glfwPollEvents();
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_140/z_out.png");

    return check_images("test_140", 1, 30);
  }

  bool test_142_blend_normalmap_heightmap()
  {
    initGLIfNeeded();

    hrErrorCallerPlace(L"test_142");

    hrSceneLibraryOpen(L"tests_f/test_142", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMaterialRef matPlastic    = hrMaterialCreate(L"matPlastic");
    HRMaterialRef matNormalBump = hrMaterialCreate(L"matNormalBump");
    HRMaterialRef matBG         = hrMaterialCreate(L"matBG");
    HRMaterialRef matGray       = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker  = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
    HRTextureNodeRef texNormal   = hrTexture2DCreateFromFile(L"data/textures/normal_map.jpg");
    HRTextureNodeRef texOrnament = hrTexture2DCreateFromFile(L"data/textures/ornament.jpg");
    HRTextureNodeRef texEnv      = hrTexture2DCreateFromFile(L"data/textures/building.hdr");
    HRTextureNodeRef texGradient = hrTexture2DCreateFromFile(L"data/textures/gradient.png");
    

    hrMaterialOpen(matPlastic, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matPlastic);

      auto diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.1 0.6 0.1");

      auto displacement = matNode.append_child(L"displacement");
      auto heightNode = displacement.append_child(L"height_map");

      displacement.append_attribute(L"type").set_value(L"height_bump");
      heightNode.append_attribute(L"amount").set_value(1.0f);

      auto texNode2 = hrTextureBind(texOrnament, heightNode);

      texNode2.append_attribute(L"matrix");
      float samplerMatrix2[16] = { 4, 0, 0, 0,
                                   0, 4, 0, 0,
                                   0, 0, 1, 0,
                                   0, 0, 0, 1 };

      texNode2.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode2.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode2.append_attribute(L"input_gamma").set_value(2.2f);
      texNode2.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode2, L"matrix", samplerMatrix2);

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matPlastic);

    hrMaterialOpen(matNormalBump, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matNormalBump);

      auto diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.6 0.1 0.1");

      auto displacement = matNode.append_child(L"displacement");
      auto heightNode = displacement.append_child(L"normal_map");

      displacement.append_attribute(L"type").set_value(L"normal_bump");

      auto invert = heightNode.append_child(L"invert");
      invert.append_attribute(L"x").set_value(0);
      invert.append_attribute(L"y").set_value(0);

      auto texNode = hrTextureBind(texNormal, heightNode);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 5, 0, 0, 0,
                                  0, 5, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(1.0f);            // !!! this is important for normalmap !!!
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matNormalBump);

    hrMaterialOpen(matBG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBG);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.25 0.25 0.8");
      color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

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
      color.append_attribute(L"val").set_value(L"0.7 0.7 0.7");
      color.append_attribute(L"tex_apply_mode").set_value(L"replace");

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

    HRMaterialRef matBlend1 = hrMaterialCreateBlend(L"matBlend1", matPlastic, matNormalBump);

    hrMaterialOpen(matBlend1, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBlend1);

      auto blend = matNode.append_child(L"blend");
      blend.append_attribute(L"type").set_value(L"mask_blend");

      auto mask = blend.append_child(L"mask");
      mask.append_attribute(L"val").set_value(1.0f);

      auto texNode = mask.append_child(L"texture");
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

      hrTextureBind(texGradient, mask);
      VERIFY_XML(matNode);
    }
    hrMaterialClose(matBlend1);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph2     = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matBlend1.id);
    HRMeshRef boxBG    = HRMeshFromSimpleMesh(L"boxBG", CreateCube(1.0f), matBG.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRLightRef sky = hrLightCreate(L"sky");

    hrLightOpen(sky, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sky);

      lightNode.attribute(L"type").set_value(L"sky");
      lightNode.attribute(L"distribution").set_value(L"map");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"0.1");

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

      camNode.append_child(L"fov").text().set(L"23");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 1 16");
      camNode.append_child(L"look_at").text().set(L"0 1 0");
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

    using namespace HydraLiteMath;

    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mScale;
    float4x4 mRes;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    ///////////
    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, planeRef, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, 1.0f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, 0.0f, 1.0f));
    mScale = scale4x4(float3(16.0f, 8.0f, 0.5f));
    mRes = mul(mScale, mRes);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, boxBG, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    mTranslate = translate4x4(float3(0, 10.0f, 0.0));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, rectLight, mRes.L());

    ///////////

    mTranslate.identity();
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
        std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
        std::cout.precision(pres);

        glfwSwapBuffers(g_window);
        glfwPollEvents();
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_142/z_out.png");

    return check_images("test_142", 1, 50);
  }
}
