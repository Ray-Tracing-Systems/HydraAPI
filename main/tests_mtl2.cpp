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

	bool test_133_emissive_and_diffuse()
	{
		hrErrorCallerPlace(L"test_133");

		hrSceneLibraryOpen(L"tests_f/test_133", HR_WRITE_DISCARD);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Materials
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		HRMaterialRef matR = hrMaterialCreate(L"matR");
		HRMaterialRef matG = hrMaterialCreate(L"matG");
		HRMaterialRef matB = hrMaterialCreate(L"matB");
		HRMaterialRef matBG = hrMaterialCreate(L"matBG");
		HRMaterialRef matGray = hrMaterialCreate(L"matGray");

		HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
		HRTextureNodeRef tex = hrTexture2DCreateFromFile(L"data/textures/blur_pattern.bmp");

		hrMaterialOpen(matR, HR_WRITE_DISCARD);
		{
			auto matNode = hrMaterialParamNode(matR);

			matNode.append_child(L"diffuse").append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.25 0.0");

			auto emission = matNode.append_child(L"emission");
			emission.append_child(L"multiplier").append_attribute(L"val").set_value(0.75f);

			auto color = emission.append_child(L"color");
			color.append_attribute(L"val").set_value(L"1.0 0.0 0.0");
			color.append_attribute(L"tex_apply_mode ").set_value(L"multiply");

			auto texNode = hrTextureBind(tex, emission.child(L"color"));

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
			VERIFY_XML(matNode);
		}
		hrMaterialClose(matR);

		hrMaterialOpen(matG, HR_WRITE_DISCARD);
		{
			auto matNode = hrMaterialParamNode(matG);

			matNode.append_child(L"diffuse").append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.0");

			auto emission = matNode.append_child(L"emission");
			emission.append_child(L"multiplier").append_attribute(L"val").set_value(0.65f);

			auto color = emission.append_child(L"color");
			color.append_attribute(L"val").set_value(L"0.0 1.0 0.0");
			color.append_attribute(L"tex_apply_mode ").set_value(L"multiply");

			auto texNode = hrTextureBind(texChecker, emission.child(L"color"));

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
		hrMaterialClose(matG);

		hrMaterialOpen(matB, HR_WRITE_DISCARD);
		{
			auto matNode = hrMaterialParamNode(matB);

			matNode.append_child(L"diffuse").append_child(L"color").append_attribute(L"val").set_value(L"0.4 0.4 0.4");

			auto emission = matNode.append_child(L"emission");
			emission.append_child(L"multiplier").append_attribute(L"val").set_value(0.5f);

			auto color = emission.append_child(L"color");
			color.append_attribute(L"val").set_value(L"0.0 0.0 1.0");
			color.append_attribute(L"tex_apply_mode ").set_value(L"multiply");

			auto texNode = hrTextureBind(tex, emission.child(L"color"));

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
			color.append_attribute(L"tex_apply_mode ").set_value(L"multiply");

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
			color.append_attribute(L"tex_apply_mode ").set_value(L"replace");

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
		HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matR.id);
		HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matG.id);
		HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), matB.id);
		HRMeshRef boxBG = HRMeshFromSimpleMesh(L"boxBG", CreateCube(1.0f), matBG.id);
		HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

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

			sizeNode.append_attribute(L"half_length").set_value(L"5.0");
			sizeNode.append_attribute(L"half_width").set_value(L"5.0");

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
		mRot.identity();

		const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

		mTranslate = translate4x4(float3(0.0f, 1.0f, -4.0f));
		mScale = scale4x4(float3(16.0f, 8.0f, 0.5f));
		// mRot = rotate_Y_4x4(60.f*DEG_TO_RAD);
		mRes = mul(mScale, mRes);
		// mRes = mul(mRot, mRes);
		mRes = mul(mTranslate, mRes);

		hrMeshInstance(scnRef, boxBG, mRes.L());

		///////////

		mTranslate.identity();
		mRes.identity();

		mTranslate = translate4x4(float3(0, 15.0f, 0.0));
		mRes = mul(mTranslate, mRes);

		hrLightInstance(scnRef, rectLight, mRes.L());

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
				std::cout << "rendering progress = " << info.progress << "% \r";
				std::cout.precision(pres);
			}

			if (info.finalUpdate)
				break;
		}

		hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_133/z_out.png");

		return check_images("test_133", 1, 10);
	}

	bool test_134_diff_refl_transp()
	{
		hrErrorCallerPlace(L"test_134");

		hrSceneLibraryOpen(L"tests_f/test_134", HR_WRITE_DISCARD);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Materials
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		HRMaterialRef matMaxcolor  = hrMaterialCreate(L"matMaxcolor");
		HRMaterialRef matLuminance = hrMaterialCreate(L"matLuminance");
		HRMaterialRef matColored   = hrMaterialCreate(L"matColored");
		HRMaterialRef matGray      = hrMaterialCreate(L"matGray");

		hrMaterialOpen(matMaxcolor, HR_WRITE_DISCARD);
		{
			auto matNode = hrMaterialParamNode(matMaxcolor);

			auto diff = matNode.append_child(L"diffuse");

			diff.append_attribute(L"brdf_type").set_value(L"lambert");
			diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.0");

			auto refl = matNode.append_child(L"reflectivity");
			
			refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
			refl.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
			refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.95");
			refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
			refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
			refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(2.5);

			auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.75 0.75");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 0.0 0.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.75f);

			VERIFY_XML(matNode);
		}
		hrMaterialClose(matMaxcolor);

		hrMaterialOpen(matLuminance, HR_WRITE_DISCARD);
		{
			auto matNode = hrMaterialParamNode(matLuminance);

			auto refl = matNode.append_child(L"reflectivity");

			refl.append_attribute(L"brdf_type").set_value(L"phong");
			refl.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.75 0.75");
			refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
			refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
			refl.append_child(L"fresnel").append_attribute(L"val").set_value(0);

			VERIFY_XML(matNode);
		}
		hrMaterialClose(matLuminance);

		hrMaterialOpen(matColored, HR_WRITE_DISCARD);
		{
			auto matNode = hrMaterialParamNode(matColored);

			auto diff = matNode.append_child(L"diffuse");

			diff.append_attribute(L"brdf_type").set_value(L"lambert");
			diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.0 0.65");

			auto refl = matNode.append_child(L"reflectivity");

			refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
			refl.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.25 0.0");
			refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
			refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
			refl.append_child(L"fresnel").append_attribute(L"val").set_value(0);

			auto transp = matNode.append_child(L"transparency");

			transp.append_attribute(L"brdf_type").set_value(L"phong");
			transp.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");
			transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0);
			transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
			transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
			transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
			transp.append_child(L"ior").append_attribute(L"val").set_value(1.75f);

			VERIFY_XML(matNode);
		}
		hrMaterialClose(matColored);

		hrMaterialOpen(matGray, HR_WRITE_DISCARD);
		{
			auto matNode = hrMaterialParamNode(matGray);

			auto diff = matNode.append_child(L"diffuse");

			diff.append_attribute(L"brdf_type").set_value(L"lambert");
			diff.append_child(L"color").append_attribute(L"val").set_value(L"0.25 0.25 0.25");

			VERIFY_XML(matNode);
		}
		hrMaterialClose(matGray);

		HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
		HRMaterialRef matBG = hrMaterialCreate(L"matBG");

		hrMaterialOpen(matBG, HR_WRITE_DISCARD);
		{
			auto matNode = hrMaterialParamNode(matBG);

			auto diff = matNode.append_child(L"diffuse");
			diff.append_attribute(L"brdf_type").set_value(L"lambert");

			auto color = diff.append_child(L"color");
			color.append_attribute(L"val").set_value(L"0.25 0.25 1.0");
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

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Meshes
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		HRMeshRef sphereMaxcolor = HRMeshFromSimpleMesh(L"sphereMaxcolor", CreateSphere(2.0f, 64), matMaxcolor.id);
		HRMeshRef sphereLum      = HRMeshFromSimpleMesh(L"sphereLum",      CreateSphere(2.0f, 64), matLuminance.id);
		HRMeshRef sphereColored  = HRMeshFromSimpleMesh(L"sphereColored",  CreateSphere(2.0f, 64), matColored.id);
		HRMeshRef planeRef       = HRMeshFromSimpleMesh(L"my_plane",       CreatePlane(10.0f),     matGray.id);
		HRMeshRef boxBG          = HRMeshFromSimpleMesh(L"boxBG",          CreateCube(1.0f),       matBG.id);

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

			sizeNode.append_attribute(L"half_length").set_value(L"5");
			sizeNode.append_attribute(L"half_width").set_value(L"5");

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
			camNode.append_child(L"position").text().set(L"0 0 12");
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

		hrMeshInstance(scnRef, sphereMaxcolor, &matrixT[0][0]);

		///////////

		mat4x4_identity(mTranslate);
		mat4x4_identity(mRes);

		mat4x4_translate(mTranslate, 4.25f, 1.0f, 0.0f);
		mat4x4_mul(mRes2, mTranslate, mRes);
		mat4x4_transpose(matrixT, mRes2); //swap rows and columns

		hrMeshInstance(scnRef, sphereColored, &matrixT[0][0]);

		///////////

		mat4x4_identity(mTranslate);
		mat4x4_identity(mRes);

		mat4x4_translate(mTranslate, 0.0f, 1.0f, 0.0f);
		mat4x4_mul(mRes2, mTranslate, mRes);
		mat4x4_transpose(matrixT, mRes2); //swap rows and columns

		hrMeshInstance(scnRef, sphereLum, &matrixT[0][0]);

		///////////

		mat4x4_identity(mTranslate);
		mat4x4_translate(mTranslate, 0, 16.0f, 0.0);
		mat4x4_transpose(matrixT, mTranslate);

		hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

		///////////
		{
			HydraLiteMath::float4x4 mTranslate, mRes, mRot, mScale;

			const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

			mTranslate = HydraLiteMath::translate4x4(HydraLiteMath::float3(0.0f, 1.0f, -4.0f));
			mScale     = HydraLiteMath::scale4x4(HydraLiteMath::float3(16.0f, 8.0f, 0.5f));
			mRes       = mul(mScale, mRes);
			mRes       = mul(mTranslate, mRes);

			hrMeshInstance(scnRef, boxBG, mRes.L());
		}

		hrSceneClose(scnRef);

		hrFlush(scnRef, renderRef);
		
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(500));

			HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

			if (info.haveUpdateFB)
			{
				auto pres = std::cout.precision(2);
				std::cout << "rendering progress = " << info.progress << "% \r";
				std::cout.precision(pres);
			}

			if (info.finalUpdate)
				break;
		}

		hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_134/z_out.png");

		return check_images("test_134", 1, 60);
	}

  bool test_135_opacity_metal()
  {
    hrErrorCallerPlace(L"test_135");

    hrSceneLibraryOpen(L"tests_f/test_135", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMaterialRef matMirror = hrMaterialCreate(L"matMirror");
    HRMaterialRef matMetalPhong = hrMaterialCreate(L"matMetalPhong");
    HRMaterialRef matMetalMF = hrMaterialCreate(L"matMetalMF");
    HRMaterialRef matBG = hrMaterialCreate(L"matBG");
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
    HRTextureNodeRef texPattern = hrTexture2DCreateFromFile(L"data/textures/blur_pattern.bmp");

    hrMaterialOpen(matMirror, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matMirror);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(0);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5);

      auto opacity = matNode.append_child(L"opacity");
      opacity.append_child(L"skip_shadow").append_attribute(L"val").set_value(0);

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
    hrMaterialClose(matMirror);

    hrMaterialOpen(matMetalPhong, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matMetalPhong);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.8");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(50);

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
    hrMaterialClose(matMetalPhong);

    hrMaterialOpen(matMetalMF, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matMetalMF);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.8");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(50);

      auto opacity = matNode.append_child(L"opacity");
      opacity.append_child(L"skip_shadow").append_attribute(L"val").set_value(0);

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
    hrMaterialClose(matMetalMF);

    hrMaterialOpen(matBG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBG);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.25 0.25 1.0");
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
      color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");
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

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matMirror.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matMetalPhong.id);
    HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), matMetalMF.id);
    HRMeshRef boxBG = HRMeshFromSimpleMesh(L"boxBG", CreateCube(1.0f), matBG.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

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
    hrRenderOpen(renderRef, HR_OPEN_EXISTING);
    {
      auto settingsNode = hrRenderParamNode(renderRef);
      settingsNode.force_child(L"method_caustic").text() = L"none";
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
    mRot.identity();

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    mTranslate = translate4x4(float3(0.0f, 1.0f, -4.0f));
    mScale = scale4x4(float3(16.0f, 8.0f, 0.5f));
    // mRot = rotate_Y_4x4(60.f*DEG_TO_RAD);
    mRes = mul(mScale, mRes);
    // mRes = mul(mRot, mRes);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, boxBG, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0, 15.0f, 0.0));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, rectLight, mRes.L());

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
        std::cout << "rendering progress = " << info.progress << "% \r";
        std::cout.precision(pres);
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_135/z_out.png");

    return check_images("test_135", 1, 20);
  }

  bool test_136_opacity_glass()
  {
    hrErrorCallerPlace(L"test_136");

    hrSceneLibraryOpen(L"tests_f/test_136", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMaterialRef matGlass                 = hrMaterialCreate(L"matGlass");
    HRMaterialRef matGlassTransluc         = hrMaterialCreate(L"matGlassTransluc");
    HRMaterialRef matDiffGlassTranslucBump = hrMaterialCreate(L"matDiffGlassTranslucBump");
    HRMaterialRef matBG                    = hrMaterialCreate(L"matBG");
    HRMaterialRef matGray                  = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker  = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
    HRTextureNodeRef texPattern  = hrTexture2DCreateFromFile(L"data/textures/blur_pattern.bmp");
    HRTextureNodeRef texOrnament = hrTexture2DCreateFromFile(L"data/textures/ornament.jpg");


    hrMaterialOpen(matGlass, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGlass);

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

      auto opacity = matNode.append_child(L"opacity");
      opacity.append_child(L"skip_shadow").append_attribute(L"val").set_value(0);

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
    hrMaterialClose(matGlass);

    hrMaterialOpen(matGlassTransluc, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGlassTransluc);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5f);

      auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"0.6 0.7 0.8");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.5f);

      auto transl = matNode.append_child(L"translucency");
      transl.append_child(L"color").append_attribute(L"val").set_value(L"0.2 0.3 0.5");

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
    hrMaterialClose(matGlassTransluc);

    hrMaterialOpen(matDiffGlassTranslucBump, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matDiffGlassTranslucBump);

      auto diffuse = matNode.append_child(L"diffuse");
      diffuse.append_child(L"color").append_attribute(L"val").set_value(L"0.2 0.3 0.5");

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5f);

      auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"0.6 0.7 0.8");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.5f);

      auto transl = matNode.append_child(L"translucency");
      transl.append_child(L"color").append_attribute(L"val").set_value(L"0.2 0.3 0.5");

      auto opacity = matNode.append_child(L"opacity");
      opacity.append_child(L"skip_shadow").append_attribute(L"val").set_value(0);

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
    hrMaterialClose(matDiffGlassTranslucBump);

    hrMaterialOpen(matBG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBG);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.25 0.25 1.0");
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
      color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");
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

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matGlass.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matGlassTransluc.id);
    HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), matDiffGlassTranslucBump.id);
    HRMeshRef boxBG = HRMeshFromSimpleMesh(L"boxBG", CreateCube(1.0f), matBG.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

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

    hrRenderOpen(renderRef, HR_OPEN_EXISTING);
    {
      auto xmlNode = hrRenderParamNode(renderRef);
      xmlNode.force_child(L"method_caustic").text() = L"none";
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
    mRot.identity();

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    mTranslate = translate4x4(float3(0.0f, 1.0f, -4.0f));
    mScale = scale4x4(float3(16.0f, 8.0f, 0.5f));
    // mRot = rotate_Y_4x4(60.f*DEG_TO_RAD);
    mRes = mul(mScale, mRes);
    // mRes = mul(mRot, mRes);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, boxBG, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0, 15.0f, 0.0));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, rectLight, mRes.L());

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
        std::cout << "rendering progress = " << info.progress << "% \r";
        std::cout.precision(pres);
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_136/z_out.png");

    return check_images("test_136", 1, 20);
  }

  bool test_137_shadow_matte_emission()
  {
    hrErrorCallerPlace(L"test_137");
  
    hrSceneLibraryOpen(L"tests_f/test_137", HR_WRITE_DISCARD);
  
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
    HRMaterialRef matR    = hrMaterialCreate(L"matR");
    HRMaterialRef matG    = hrMaterialCreate(L"matG");
  
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
  
      auto emission = matNode.append_child(L"emission");
      emission.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);
  
      auto color = emission.append_child(L"color");
      color.append_attribute(L"val").set_value(L"1.0 0.0 0.0");
      color.append_attribute(L"tex_apply_mode ").set_value(L"multiply");
  
      auto tex     = hrTexture2DCreateFromFile(L"data/textures/blur_pattern.bmp");
      auto texNode = hrTextureBind(tex, emission.child(L"color"));
  
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
      
      VERIFY_XML(matNode);
    }
    hrMaterialClose(matG);
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph2      = HRMeshFromSimpleMesh(L"sph2",      CreateSphere(1.0f, 64), matR.id);
    HRMeshRef planeRef2 = HRMeshFromSimpleMesh(L"my_plane2", CreatePlane(4.0f),      matG.id);
  
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
  
    HRRenderRef renderRef = CreateBasicTestRenderPTNoCaust(CURR_RENDER_DEVICE, 512, 512, 256, 1024);
  
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
  
    while (true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
      HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);
    
      if (info.haveUpdateFB)
      {
        auto pres = std::cout.precision(2);
        std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
        std::cout.precision(pres);
      }
    
      if (info.finalUpdate)
        break;
    }
  
    hrRenderSaveFrameBufferLDR (renderRef, L"tests_images/test_137/z_out.png");
  
    hrRenderSaveGBufferLayerLDR(renderRef, L"tests_images/test_137/z_out2.png", L"depth");
    hrRenderSaveGBufferLayerLDR(renderRef, L"tests_images/test_137/z_out3.png", L"diffcolor");
    hrRenderSaveGBufferLayerLDR(renderRef, L"tests_images/test_137/z_out4.png", L"alpha");
    hrRenderSaveGBufferLayerLDR(renderRef, L"tests_images/test_137/z_out5.png", L"shadow");
  
    return check_images("test_137", 5, 25);
  }

  bool test_138_translucency_and_diffuse()
  {
    hrErrorCallerPlace(L"test_138");

    hrSceneLibraryOpen(L"tests_f/test_138", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMaterialRef matDiffTrans = hrMaterialCreate(L"matDiffTrans");
    HRMaterialRef matDiffReflTrans = hrMaterialCreate(L"matDiffReflTrans");
    HRMaterialRef matDiffReflTransOpac = hrMaterialCreate(L"matDiffReflTransOpac");
    HRMaterialRef matBG = hrMaterialCreate(L"matBG");
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
    HRTextureNodeRef texPattern = hrTexture2DCreateFromFile(L"data/textures/blur_pattern.bmp");
    HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/building.hdr");


    hrMaterialOpen(matDiffTrans, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matDiffTrans);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.4 0.4 0.4");

      auto transl = matNode.append_child(L"translucency");
      transl.append_child(L"color").append_attribute(L"val").set_value(L"0.4 0.4 0.4");

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matDiffTrans);

    hrMaterialOpen(matDiffReflTrans, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matDiffReflTrans);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.2 0.3 0.4");

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.9");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"luminance");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5);

      auto transl = matNode.append_child(L"translucency");
      transl.append_child(L"color").append_attribute(L"val").set_value(L"0.2 0.3 0.4");

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matDiffReflTrans);

    hrMaterialOpen(matDiffReflTransOpac, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matDiffReflTransOpac);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.4 0.1 0.1");

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.8");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5);

      auto transl = matNode.append_child(L"translucency");
      transl.append_child(L"color").append_attribute(L"val").set_value(L"0.1 0.4 0.1");

      auto opacity = matNode.append_child(L"opacity");
      opacity.append_child(L"skip_shadow").append_attribute(L"val").set_value(0);

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
    hrMaterialClose(matDiffReflTransOpac);

    hrMaterialOpen(matBG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBG);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.25 0.25 1.0");
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

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matDiffTrans.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matDiffReflTrans.id);
    HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), matDiffReflTransOpac.id);
    HRMeshRef boxBG = HRMeshFromSimpleMesh(L"boxBG", CreateCube(1.0f), matBG.id);
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

      sizeNode.append_attribute(L"half_length").set_value(L"1.0");
      sizeNode.append_attribute(L"half_width").set_value(L"1.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1000.0");

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
    mRot.identity();

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    mTranslate = translate4x4(float3(0.0f, -6.0f, 3.0f));
    mScale = scale4x4(float3(16.0f, 8.0f, 0.5f));
    mRot = rotate_Y_4x4(25.f * DEG_TO_RAD);
    mRes = mul(mScale, mRes);
    mRes = mul(mRot, mRes);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, boxBG, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    mTranslate = translate4x4(float3(10, 15.0f, -20.0));
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
        std::cout << "rendering progress = " << info.progress << "% \r";
        std::cout.precision(pres);
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_138/z_out.png");

    return check_images("test_138", 1, 15);
  }

  bool test_139_glass_and_bump()
  {
    hrErrorCallerPlace(L"test_139");

    hrSceneLibraryOpen(L"tests_f/test_139", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMaterialRef matGlass = hrMaterialCreate(L"matGlass");
    HRMaterialRef matGlassBump = hrMaterialCreate(L"matGlassBump");
    HRMaterialRef matGlassNormalBump = hrMaterialCreate(L"matDiffGlassTranslucBump");
    HRMaterialRef matBG = hrMaterialCreate(L"matBG");
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
    HRTextureNodeRef texNormal = hrTexture2DCreateFromFile(L"data/textures/normal_map.jpg");
    HRTextureNodeRef texOrnament = hrTexture2DCreateFromFile(L"data/textures/ornament.jpg");
    HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/building.hdr");


    hrMaterialOpen(matGlass, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGlass);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5f);

      auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"0.6 0.7 0.8");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.5f);

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matGlass);

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
      transp.append_child(L"color").append_attribute(L"val").set_value(L"0.6 0.7 0.8");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.5f);

      auto displacement = matNode.append_child(L"displacement");
      auto heightNode = displacement.append_child(L"height_map");

      displacement.append_attribute(L"type").set_value(L"height_bump");
      heightNode.append_attribute(L"amount").set_value(0.5f);

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

    hrMaterialOpen(matGlassNormalBump, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGlassNormalBump);

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
      auto heightNode = displacement.append_child(L"normal_map");

      displacement.append_attribute(L"type").set_value(L"normal_bump");

      auto invert = heightNode.append_child(L"invert");
      invert.append_attribute(L"x").set_value(0);
      invert.append_attribute(L"y").set_value(0);

      auto texNode = hrTextureBind(texNormal, heightNode);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 2, 0, 0, 0,
                                  0, 2, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(1.0f);            // !!! this is important for normalmap !!!
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matGlassNormalBump);

    hrMaterialOpen(matBG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBG);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.25 0.25 1.0");
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

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matGlass.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matGlassBump.id);
    HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), matGlassNormalBump.id);
    HRMeshRef boxBG = HRMeshFromSimpleMesh(L"boxBG", CreateCube(1.0f), matBG.id);
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

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 512, 4096);


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
        std::cout << "rendering progress = " << info.progress << "% \r";
        std::cout.precision(pres);
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_139/z_out.png");

    return check_images("test_139", 1, 65);
  }


  bool test_141_opacity_smooth()
  {
    hrErrorCallerPlace(L"test_141");

    hrSceneLibraryOpen(L"tests_f/test_141", HR_WRITE_DISCARD);

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
      diff.append_child(L"color").append_attribute(L"val").set_value(L"1.0 0.5 0.5");

      auto opacity = matNode.append_child(L"opacity");
      opacity.append_attribute(L"smooth").set_value(0);
      opacity.append_child(L"skip_shadow").append_attribute(L"val").set_value(0);

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
    hrMaterialClose(matR);

    hrMaterialOpen(matG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matG);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 1.0 0.5");

      auto opacity = matNode.append_child(L"opacity");
      opacity.append_attribute(L"smooth").set_value(1);
      opacity.append_child(L"skip_shadow").append_attribute(L"val").set_value(0);

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
      opacity.append_attribute(L"smooth").set_value(1);
      opacity.append_child(L"skip_shadow").append_attribute(L"val").set_value(0);

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
    hrMaterialClose(matB);

    hrMaterialOpen(matBG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBG);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.25 0.25 1.0");
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
      color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");
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

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matR.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matG.id);
    HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), matB.id);
    HRMeshRef boxBG = HRMeshFromSimpleMesh(L"boxBG", CreateCube(1.0f), matBG.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

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

      sizeNode.append_attribute(L"half_length").set_value(L"0.5");
      sizeNode.append_attribute(L"half_width").set_value(L"0.5");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"400.0");
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
    mRot.identity();

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    mTranslate = translate4x4(float3(0.0f, 1.0f, -4.0f));
    mScale = scale4x4(float3(16.0f, 8.0f, 0.5f));
    // mRot = rotate_Y_4x4(60.f*DEG_TO_RAD);
    mRes = mul(mScale, mRes);
    // mRes = mul(mRot, mRes);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, boxBG, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0, 20.0f, 0.0));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, rectLight, mRes.L());

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
        std::cout << "rendering progress = " << info.progress << "% \r";
        std::cout.precision(pres);
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_141/z_out.png");

    return check_images("test_141", 1, 10);
  }


};