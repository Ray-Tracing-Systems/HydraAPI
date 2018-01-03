#include "tests.h"
#include <math.h>
#include <iomanip>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sstream>

#include "../hydra_api/HR_HDRImageTool.h"
#include "tests.h"


#pragma warning(disable:4838)

using namespace TEST_UTILS;


bool test31_procedural_texture_LDR()
{
	initGLIfNeeded();

	hrErrorCallerPlace(L"test31");

	HRCameraRef    camRef;
	HRSceneInstRef scnRef;
	HRRenderRef    settingsRef;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	hrSceneLibraryOpen(L"tests/test_31", HR_WRITE_DISCARD);

	// geometry
	//
	HRMeshRef cubeRef  = HRMeshFromSimpleMesh(L"my_cube", CreateCube(0.75f), 0);
	HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(2.0f), 1);
	HRMeshRef sphRef   = HRMeshFromSimpleMesh(L"my_sphere", CreateSphere(0.5f, 32), 2);
	HRMeshRef torRef   = HRMeshFromSimpleMesh(L"my_torus", CreateTorus(0.2f, 0.5f, 32, 32), 3);

	// material and textures
	//

	int rep1 = 16;
	int rep2 = 8;
	int rep3 = 4;
	int rep4 = 2;

	HRTextureNodeRef testTex1 = hrTexture2DCreateFromProcLDR(&procTexCheckerLDR, (void*)(&rep1), 128, 128);
	HRTextureNodeRef testTex2 = hrTexture2DCreateFromProcLDR(&procTexCheckerLDR, (void*)(&rep2), 64, 64);
	HRTextureNodeRef testTex3 = hrTexture2DCreateFromProcLDR(&procTexCheckerLDR, (void*)(&rep3), 32, 32);
	HRTextureNodeRef testTex4 = hrTexture2DCreateFromProcLDR(&procTexCheckerLDR, (void*)(&rep4), 16, 16);

	//CreateStripedImageFile("tests_images/test_23/TexFromMemory.png", colors, 4, 128, 128);

	HRMaterialRef mat1 = hrMaterialCreate(L"mysimplemat");
	HRMaterialRef mat2 = hrMaterialCreate(L"mysimplemat2");
	HRMaterialRef mat3 = hrMaterialCreate(L"mysimplemat3");
	HRMaterialRef mat4 = hrMaterialCreate(L"mysimplemat4");

	hrMaterialOpen(mat1, HR_WRITE_DISCARD);
	{
		xml_node matNode = hrMaterialParamNode(mat1);

		xml_node diff = matNode.append_child(L"diffuse");

		diff.append_attribute(L"brdf_type").set_value(L"lambert");
		diff.append_child(L"color").text().set(L"0.75 0.75 0.5");

		hrTextureBind(testTex1, diff);
	}
	hrMaterialClose(mat1);

	hrMaterialOpen(mat2, HR_WRITE_DISCARD);
	{
		xml_node matNode = hrMaterialParamNode(mat2);

		xml_node diff = matNode.append_child(L"diffuse");

		diff.append_attribute(L"brdf_type").set_value(L"lambert");
		diff.append_child(L"color").text().set(L"1 1 1");

		hrTextureBind(testTex2, diff);
	}
	hrMaterialClose(mat2);

	hrMaterialOpen(mat3, HR_WRITE_DISCARD);
	{
		xml_node matNode = hrMaterialParamNode(mat3);

		xml_node diff = matNode.append_child(L"diffuse");

		diff.append_attribute(L"brdf_type").set_value(L"lambert");
		diff.append_child(L"color").text().set(L"0.95 0.95 0.95");

		hrTextureBind(testTex3, diff);
	}
	hrMaterialClose(mat3);

	hrMaterialOpen(mat4, HR_WRITE_DISCARD);
	{
		xml_node matNode = hrMaterialParamNode(mat4);

		xml_node diff = matNode.append_child(L"diffuse");

		diff.append_attribute(L"brdf_type").set_value(L"lambert");
		diff.append_child(L"color").text().set(L"0.95 0.95 0.95");

		hrTextureBind(testTex4, diff);
	}
	hrMaterialClose(mat4);


	// camera
	//
	camRef = hrCameraCreate(L"my camera");

	hrCameraOpen(camRef, HR_WRITE_DISCARD);
	{
		xml_node camNode = hrCameraParamNode(camRef);

		camNode.append_child(L"fov").text().set(L"45");
		camNode.append_child(L"nearClipPlane").text().set(L"0.01");
		camNode.append_child(L"farClipPlane").text().set(L"100.0");

		camNode.append_child(L"up").text().set(L"0 1 0");
		camNode.append_child(L"position").text().set(L"0 0 0");
		camNode.append_child(L"look_at").text().set(L"0 0 -1");
	}
	hrCameraClose(camRef);

	// set up render settings
	//
	settingsRef = hrRenderCreate(L"opengl1");

	hrRenderOpen(settingsRef, HR_WRITE_DISCARD);
	{
		pugi::xml_node node = hrRenderParamNode(settingsRef);

		wchar_t temp[256];
		swprintf(temp, 256, L"%d", 1024);
		node.append_child(L"width").text().set(temp);
		swprintf(temp, 256, L"%d", 768);
		node.append_child(L"height").text().set(temp);
	}
	hrRenderClose(settingsRef);

	// create scene
	//
	scnRef = hrSceneCreate(L"my scene");

	float	rtri = 25.0f; // Angle For The Triangle ( NEW )
	float	rquad = 40.0f;
	float g_FPS = 60.0f;
	int   frameCounter = 0;

	const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

	float matrixT[4][4], matrixT2[4][4], matrixT3[4][4], matrixT4[4][4];
	float mRot1[4][4], mTranslate[4][4], mRes[4][4];

	float mTranslateDown[4][4], mRes2[4][4];

	mat4x4_identity(mRot1);
	mat4x4_identity(mTranslate);
	mat4x4_identity(mRes);

	mat4x4_translate(mTranslate, 0.0f, 0.25f, -5.0f);
	mat4x4_rotate_X(mRot1, mRot1, -rquad*DEG_TO_RAD);
	mat4x4_rotate_Y(mRot1, mRot1, -rquad*DEG_TO_RAD*0.5f);
	mat4x4_mul(mRes, mTranslate, mRot1);
	mat4x4_transpose(matrixT, mRes); // this fucking math library swap rows and columns

	mat4x4_identity(mRes);
	mat4x4_translate(mTranslateDown, 0.0f, -1.0f, -5.0f);
	mat4x4_mul(mRes2, mTranslateDown, mRes);
	mat4x4_transpose(matrixT2, mRes2);

	mat4x4_identity(mRot1);
	mat4x4_identity(mRes);
	mat4x4_rotate_Y(mRes, mRot1, rquad*DEG_TO_RAD);
	mat4x4_translate(mTranslateDown, -1.5f, -0.5f, -4.0f);
	mat4x4_mul(mRes2, mTranslateDown, mRes);
	mat4x4_transpose(matrixT3, mRes2);

	mat4x4_identity(mRot1);
	mat4x4_identity(mTranslate);
	mat4x4_identity(mRes);

	mat4x4_translate(mTranslate, 2.0f, 0.25f, -5.0f);
	mat4x4_rotate_X(mRot1, mRot1, rquad*DEG_TO_RAD);
	mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD*0.5f);
	mat4x4_mul(mRes, mTranslate, mRot1);
	mat4x4_transpose(matrixT4, mRes); // this fucking math library swap rows and columns

	// draw scene
	//
	hrSceneOpen(scnRef, HR_WRITE_DISCARD);
	{
		hrMeshInstance(scnRef, cubeRef, &matrixT[0][0]);
		hrMeshInstance(scnRef, planeRef, &matrixT2[0][0]);
		hrMeshInstance(scnRef, sphRef, &matrixT3[0][0]);
		hrMeshInstance(scnRef, torRef, &matrixT4[0][0]);
	}
	hrSceneClose(scnRef);

	hrFlush(scnRef, settingsRef);

	hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_31/z_out.png");

	return check_images("test_31");
}

bool test32_procedural_texture_HDR()
{
	initGLIfNeeded();

	hrErrorCallerPlace(L"test32");

	HRCameraRef    camRef;
	HRSceneInstRef scnRef;
	HRRenderRef    settingsRef;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	hrSceneLibraryOpen(L"tests/test_32", HR_WRITE_DISCARD);

	// geometry
	//
	HRMeshRef cubeRef  = HRMeshFromSimpleMesh(L"my_cube", CreateCube(0.75f), 0);
	HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(2.0f), 1);
	HRMeshRef sphRef   = HRMeshFromSimpleMesh(L"my_sphere", CreateSphere(0.5f, 32), 2);
	HRMeshRef torRef   = HRMeshFromSimpleMesh(L"my_torus", CreateTorus(0.2f, 0.5f, 32, 32), 3);

	// material and textures
	//

	int rep1 = 16;
	int rep2 = 8;
	int rep3 = 4;
	int rep4 = 2;

	HRTextureNodeRef testTex1 = hrTexture2DCreateFromProcHDR(&procTexCheckerHDR, (void*)(&rep1), 128, 128);
	HRTextureNodeRef testTex2 = hrTexture2DCreateFromProcHDR(&procTexCheckerHDR, (void*)(&rep2), 64, 64);
	HRTextureNodeRef testTex3 = hrTexture2DCreateFromProcHDR(&procTexCheckerHDR, (void*)(&rep3), 32, 32);
	HRTextureNodeRef testTex4 = hrTexture2DCreateFromProcHDR(&procTexCheckerHDR, (void*)(&rep4), 16, 16);

	//CreateStripedImageFile("tests_images/test_23/TexFromMemory.png", colors, 4, 128, 128);

	HRMaterialRef mat1 = hrMaterialCreate(L"mysimplemat");
	HRMaterialRef mat2 = hrMaterialCreate(L"mysimplemat2");
	HRMaterialRef mat3 = hrMaterialCreate(L"mysimplemat3");
	HRMaterialRef mat4 = hrMaterialCreate(L"mysimplemat4");

	hrMaterialOpen(mat1, HR_WRITE_DISCARD);
	{
		xml_node matNode = hrMaterialParamNode(mat1);

		xml_node diff = matNode.append_child(L"diffuse");

		diff.append_attribute(L"brdf_type").set_value(L"lambert");
		diff.append_child(L"color").text().set(L"0.75 0.75 0.5");

		hrTextureBind(testTex1, diff);
	}
	hrMaterialClose(mat1);

	hrMaterialOpen(mat2, HR_WRITE_DISCARD);
	{
		xml_node matNode = hrMaterialParamNode(mat2);

		xml_node diff = matNode.append_child(L"diffuse");

		diff.append_attribute(L"brdf_type").set_value(L"lambert");
		diff.append_child(L"color").text().set(L"1 1 1");

		hrTextureBind(testTex2, diff);
	}
	hrMaterialClose(mat2);

	hrMaterialOpen(mat3, HR_WRITE_DISCARD);
	{
		xml_node matNode = hrMaterialParamNode(mat3);

		xml_node diff = matNode.append_child(L"diffuse");

		diff.append_attribute(L"brdf_type").set_value(L"lambert");
		diff.append_child(L"color").text().set(L"0.95 0.95 0.95");

		hrTextureBind(testTex3, diff);
	}
	hrMaterialClose(mat3);

	hrMaterialOpen(mat4, HR_WRITE_DISCARD);
	{
		xml_node matNode = hrMaterialParamNode(mat4);

		xml_node diff = matNode.append_child(L"diffuse");

		diff.append_attribute(L"brdf_type").set_value(L"lambert");
		diff.append_child(L"color").text().set(L"0.95 0.95 0.95");

		hrTextureBind(testTex4, diff);
	}
	hrMaterialClose(mat4);


	// camera
	//
	camRef = hrCameraCreate(L"my camera");

	hrCameraOpen(camRef, HR_WRITE_DISCARD);
	{
		xml_node camNode = hrCameraParamNode(camRef);

		camNode.append_child(L"fov").text().set(L"45");
		camNode.append_child(L"nearClipPlane").text().set(L"0.01");
		camNode.append_child(L"farClipPlane").text().set(L"100.0");

		camNode.append_child(L"up").text().set(L"0 1 0");
		camNode.append_child(L"position").text().set(L"0 0 0");
		camNode.append_child(L"look_at").text().set(L"0 0 -1");
	}
	hrCameraClose(camRef);

	// set up render settings
	//
	settingsRef = hrRenderCreate(L"opengl1");

	hrRenderOpen(settingsRef, HR_WRITE_DISCARD);
	{
		pugi::xml_node node = hrRenderParamNode(settingsRef);

		wchar_t temp[256];
		swprintf(temp, 256, L"%d", 1024);
		node.append_child(L"width").text().set(temp);
		swprintf(temp, 256, L"%d", 768);
		node.append_child(L"height").text().set(temp);
	}
	hrRenderClose(settingsRef);

	// create scene
	//
	scnRef = hrSceneCreate(L"my scene");

	float	rtri = 25.0f; // Angle For The Triangle ( NEW )
	float	rquad = 40.0f;
	float g_FPS = 60.0f;
	int   frameCounter = 0;

	const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

	float matrixT[4][4], matrixT2[4][4], matrixT3[4][4], matrixT4[4][4];
	float mRot1[4][4], mTranslate[4][4], mRes[4][4];

	float mTranslateDown[4][4], mRes2[4][4];

	mat4x4_identity(mRot1);
	mat4x4_identity(mTranslate);
	mat4x4_identity(mRes);

	mat4x4_translate(mTranslate, 0.0f, 0.25f, -5.0f);
	mat4x4_rotate_X(mRot1, mRot1, -rquad*DEG_TO_RAD);
	mat4x4_rotate_Y(mRot1, mRot1, -rquad*DEG_TO_RAD*0.5f);
	mat4x4_mul(mRes, mTranslate, mRot1);
	mat4x4_transpose(matrixT, mRes); // this fucking math library swap rows and columns

	mat4x4_identity(mRes);
	mat4x4_translate(mTranslateDown, 0.0f, -1.0f, -5.0f);
	mat4x4_mul(mRes2, mTranslateDown, mRes);
	mat4x4_transpose(matrixT2, mRes2);

	mat4x4_identity(mRot1);
	mat4x4_identity(mRes);
	mat4x4_rotate_Y(mRes, mRot1, rquad*DEG_TO_RAD);
	mat4x4_translate(mTranslateDown, -1.5f, -0.5f, -4.0f);
	mat4x4_mul(mRes2, mTranslateDown, mRes);
	mat4x4_transpose(matrixT3, mRes2);

	mat4x4_identity(mRot1);
	mat4x4_identity(mTranslate);
	mat4x4_identity(mRes);

	mat4x4_translate(mTranslate, 2.0f, 0.25f, -5.0f);
	mat4x4_rotate_X(mRot1, mRot1, rquad*DEG_TO_RAD);
	mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD*0.5f);
	mat4x4_mul(mRes, mTranslate, mRot1);
	mat4x4_transpose(matrixT4, mRes); // this fucking math library swap rows and columns

	// draw scene
	//
	hrSceneOpen(scnRef, HR_WRITE_DISCARD);
	{
		hrMeshInstance(scnRef, cubeRef, &matrixT[0][0]);
		hrMeshInstance(scnRef, planeRef, &matrixT2[0][0]);
		hrMeshInstance(scnRef, sphRef, &matrixT3[0][0]);
		hrMeshInstance(scnRef, torRef, &matrixT4[0][0]);
	}
	hrSceneClose(scnRef);

	hrFlush(scnRef, settingsRef);

	hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_32/z_out.png");

	return check_images("test_32");
}

bool test33_update_from_file()
{
	initGLIfNeeded();

	hrErrorCallerPlace(L"test_33");

	HRCameraRef    camRef;
	HRSceneInstRef scnRef;
	HRRenderRef    settingsRef;

	bool id_ok1 = false;
	bool id_ok2 = false;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	hrSceneLibraryOpen(L"tests/test_33", HR_WRITE_DISCARD);

	// geometry
	//
	HRMeshRef cubeRef = HRMeshFromSimpleMesh(L"my_cube", CreateCube(0.75f), 0);
	HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(2.0f), 1);
	HRMeshRef sphRef = HRMeshFromSimpleMesh(L"my_sphere", CreateSphere(0.5f, 32), 2);
	HRMeshRef torRef = HRMeshFromSimpleMesh(L"my_torus", CreateTorus(0.2f, 0.5f, 32, 32), 0);

	// material and textures
	//
	HRTextureNodeRef testTex2 = hrTexture2DCreateFromFile(L"data/textures/163.jpg");

	HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
	HRMaterialRef mat1 = hrMaterialCreate(L"mysimplemat2");
	HRMaterialRef mat2 = hrMaterialCreate(L"wood");

	hrMaterialOpen(mat0, HR_WRITE_DISCARD);
	{
		xml_node matNode = hrMaterialParamNode(mat0);

		xml_node diff = matNode.append_child(L"diffuse");

		diff.append_attribute(L"brdf_type").set_value(L"lambert");
		diff.append_child(L"color").text().set(L"0.75 0.75 0.5");

		hrTextureBind(testTex2, diff);
	}
	hrMaterialClose(mat0);

	hrMaterialOpen(mat1, HR_WRITE_DISCARD);
	{
		xml_node matNode = hrMaterialParamNode(mat1);

		xml_node diff = matNode.append_child(L"diffuse");

		diff.append_attribute(L"brdf_type").set_value(L"lambert");
		diff.append_child(L"color").text().set(L"1 1 1");

		HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
		hrTextureBind(testTex, diff);
	}
	hrMaterialClose(mat1);

	hrMaterialOpen(mat2, HR_WRITE_DISCARD);
	{
		xml_node matNode = hrMaterialParamNode(mat2);

		xml_node diff = matNode.append_child(L"diffuse");

		diff.append_attribute(L"brdf_type").set_value(L"lambert");
		diff.append_child(L"color").text().set(L"0.75 0.75 0.75");

		HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/relief_wood.jpg");
		hrTextureBind(testTex, diff);
	}
	hrMaterialClose(mat2);

	// camera
	//
	camRef = hrCameraCreate(L"my camera");

	hrCameraOpen(camRef, HR_WRITE_DISCARD);
	{
		xml_node camNode = hrCameraParamNode(camRef);

		camNode.append_child(L"fov").text().set(L"45");
		camNode.append_child(L"nearClipPlane").text().set(L"0.01");
		camNode.append_child(L"farClipPlane").text().set(L"100.0");

		camNode.append_child(L"up").text().set(L"0 1 0");
		camNode.append_child(L"position").text().set(L"0 0 0");
		camNode.append_child(L"look_at").text().set(L"0 0 -1");
	}
	hrCameraClose(camRef);

	// set up render settings
	//
	settingsRef = hrRenderCreate(L"opengl1");

	hrRenderOpen(settingsRef, HR_WRITE_DISCARD);
	{
		pugi::xml_node node = hrRenderParamNode(settingsRef);

		wchar_t temp[256];
		swprintf(temp, 256, L"%d", 1024);
		node.append_child(L"width").text().set(temp);
		swprintf(temp, 256, L"%d", 768);
		node.append_child(L"height").text().set(temp);
	}
	hrRenderClose(settingsRef);

	// create scene
	//
	scnRef = hrSceneCreate(L"my scene");

	float	rtri = 25.0f; // Angle For The Triangle ( NEW )
	float	rquad = 40.0f;
	float g_FPS = 60.0f;
	int   frameCounter = 0;

	const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

	float matrixT[4][4], matrixT2[4][4], matrixT3[4][4], matrixT4[4][4];
	float mRot1[4][4], mTranslate[4][4], mRes[4][4];

	float mTranslateDown[4][4], mRes2[4][4];

	mat4x4_identity(mRot1);
	mat4x4_identity(mTranslate);
	mat4x4_identity(mRes);

	mat4x4_translate(mTranslate, 0.0f, 0.25f, -5.0f);
	mat4x4_rotate_X(mRot1, mRot1, -rquad*DEG_TO_RAD);
	mat4x4_rotate_Y(mRot1, mRot1, -rquad*DEG_TO_RAD*0.5f);
	mat4x4_mul(mRes, mTranslate, mRot1);
	mat4x4_transpose(matrixT, mRes); // this fucking math library swap rows and columns

	mat4x4_identity(mRes);
	mat4x4_translate(mTranslateDown, 0.0f, -1.0f, -5.0f);
	mat4x4_mul(mRes2, mTranslateDown, mRes);
	mat4x4_transpose(matrixT2, mRes2);

	mat4x4_identity(mRot1);
	mat4x4_identity(mRes);
	mat4x4_rotate_Y(mRes, mRot1, rquad*DEG_TO_RAD);
	mat4x4_translate(mTranslateDown, -1.5f, -0.5f, -4.0f);
	mat4x4_mul(mRes2, mTranslateDown, mRes);
	mat4x4_transpose(matrixT3, mRes2);

	mat4x4_identity(mRot1);
	mat4x4_identity(mTranslate);
	mat4x4_identity(mRes);

	mat4x4_translate(mTranslate, 2.0f, 0.25f, -5.0f);
	mat4x4_rotate_X(mRot1, mRot1, rquad*DEG_TO_RAD);
	mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD*0.5f);
	mat4x4_mul(mRes, mTranslate, mRot1);
	mat4x4_transpose(matrixT4, mRes); // this fucking math library swap rows and columns

	// draw scene
	//
	hrSceneOpen(scnRef, HR_WRITE_DISCARD);
	{
		hrMeshInstance(scnRef, cubeRef, &matrixT[0][0]);
		hrMeshInstance(scnRef, planeRef, &matrixT2[0][0]);
		hrMeshInstance(scnRef, sphRef, &matrixT3[0][0]);
		hrMeshInstance(scnRef, torRef, &matrixT4[0][0]);
	}
	hrSceneClose(scnRef);

	hrFlush(scnRef, settingsRef);
	hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_33/z_out.png");


	HRTextureNodeRef new_testTex2 = hrTexture2DUpdateFromFile(testTex2, L"data/textures/chess_red.bmp");

	hrMaterialOpen(mat0, HR_OPEN_EXISTING);
	{
		xml_node matNode = hrMaterialParamNode(mat0);

		xml_node diff = matNode.append_child(L"diffuse");
		
		hrTextureBind(new_testTex2, diff);
	}
	hrMaterialClose(mat0);

	hrFlush(scnRef, settingsRef);
	hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_33/z_out2.png");
	
	id_ok1 = (new_testTex2.id == testTex2.id);
	// frame 3
	//

	new_testTex2 = hrTexture2DUpdateFromFile(new_testTex2, L"data/textures/163.jpg");

	hrMaterialOpen(mat1, HR_OPEN_EXISTING);
	{
		xml_node matNode = hrMaterialParamNode(mat1);

		xml_node diff = matNode.append_child(L"diffuse");

		hrTextureBind(new_testTex2, diff);
	}
	hrMaterialClose(mat1);

	hrFlush(scnRef, settingsRef);
	hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_33/z_out3.png");

	id_ok2 = (new_testTex2.id == testTex2.id);

	hrErrorCallback(ErrorCallBack);

  bool noDups1 = check_all_duplicates(L"tests/test_33/statex_00002.xml");
  bool noDups2 = check_all_duplicates(L"tests/test_33/statex_00003.xml");

	return check_images("test_33") && id_ok1 && id_ok2 && noDups1 && noDups2;
}

bool test36_update_from_memory()
{
	initGLIfNeeded();

	hrErrorCallerPlace(L"test_36");

	HRCameraRef    camRef;
	HRSceneInstRef scnRef;
	HRRenderRef    settingsRef;

	bool id_ok1 = false;
	bool id_ok2 = false;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	hrSceneLibraryOpen(L"tests/test_36", HR_WRITE_DISCARD);

	// geometry
	//
	HRMeshRef cubeRef = HRMeshFromSimpleMesh(L"my_cube", CreateCube(0.75f), 0);
	HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(2.0f), 1);
	HRMeshRef sphRef = HRMeshFromSimpleMesh(L"my_sphere", CreateSphere(0.5f, 32), 1);
	HRMeshRef torRef = HRMeshFromSimpleMesh(L"my_torus", CreateTorus(0.2f, 0.5f, 32, 32), 0);

	// material and textures
	//
	unsigned int colors1[4] = { 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF, 0xFF000000 };
	unsigned int colors2[4] = { 0xFF00FF00, 0xFFFF0000, 0xFFFF00FF, 0xFFFFFFFF };

	std::vector<unsigned int> imageData1 = CreateStripedImageData(colors1, 4, 128, 128);
	std::vector<unsigned int> imageData2 = CreateStripedImageData(colors2, 4, 300, 300);

	HRTextureNodeRef testTex2 = hrTexture2DCreateFromMemory(128, 128, 4, &imageData1[0]);
	HRTextureNodeRef testTex3 = hrTexture2DCreateFromMemory(300, 300, 4, &imageData2[0]);

	HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
	HRMaterialRef mat1 = hrMaterialCreate(L"mysimplemat2");
	HRMaterialRef mat2 = hrMaterialCreate(L"wood");

	hrMaterialOpen(mat0, HR_WRITE_DISCARD);
	{
		xml_node matNode = hrMaterialParamNode(mat0);

		xml_node diff = matNode.append_child(L"diffuse");

		diff.append_attribute(L"brdf_type").set_value(L"lambert");
		diff.append_child(L"color").text().set(L"0.75 0.75 0.5");

		hrTextureBind(testTex2, diff);
	}
	hrMaterialClose(mat0);

	hrMaterialOpen(mat1, HR_WRITE_DISCARD);
	{
		xml_node matNode = hrMaterialParamNode(mat1);

		xml_node diff = matNode.append_child(L"diffuse");

		diff.append_attribute(L"brdf_type").set_value(L"lambert");
		diff.append_child(L"color").text().set(L"1 1 1");

		hrTextureBind(testTex3, diff);
	}
	hrMaterialClose(mat1);

	// camera
	//
	camRef = hrCameraCreate(L"my camera");

	hrCameraOpen(camRef, HR_WRITE_DISCARD);
	{
		xml_node camNode = hrCameraParamNode(camRef);

		camNode.append_child(L"fov").text().set(L"45");
		camNode.append_child(L"nearClipPlane").text().set(L"0.01");
		camNode.append_child(L"farClipPlane").text().set(L"100.0");

		camNode.append_child(L"up").text().set(L"0 1 0");
		camNode.append_child(L"position").text().set(L"0 0 0");
		camNode.append_child(L"look_at").text().set(L"0 0 -1");
	}
	hrCameraClose(camRef);

	// set up render settings
	//
	settingsRef = hrRenderCreate(L"opengl1");

	hrRenderOpen(settingsRef, HR_WRITE_DISCARD);
	{
		pugi::xml_node node = hrRenderParamNode(settingsRef);

		wchar_t temp[256];
		swprintf(temp, 256, L"%d", 1024);
		node.append_child(L"width").text().set(temp);
		swprintf(temp, 256, L"%d", 768);
		node.append_child(L"height").text().set(temp);
	}
	hrRenderClose(settingsRef);

	// create scene
	//
	scnRef = hrSceneCreate(L"my scene");

	float	rtri = 25.0f; // Angle For The Triangle ( NEW )
	float	rquad = 40.0f;
	float g_FPS = 60.0f;
	int   frameCounter = 0;

	const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

	float matrixT[4][4], matrixT2[4][4], matrixT3[4][4], matrixT4[4][4];
	float mRot1[4][4], mTranslate[4][4], mRes[4][4];

	float mTranslateDown[4][4], mRes2[4][4];

	mat4x4_identity(mRot1);
	mat4x4_identity(mTranslate);
	mat4x4_identity(mRes);

	mat4x4_translate(mTranslate, 0.0f, 0.25f, -5.0f);
	mat4x4_rotate_X(mRot1, mRot1, -rquad*DEG_TO_RAD);
	mat4x4_rotate_Y(mRot1, mRot1, -rquad*DEG_TO_RAD*0.5f);
	mat4x4_mul(mRes, mTranslate, mRot1);
	mat4x4_transpose(matrixT, mRes); // this fucking math library swap rows and columns

	mat4x4_identity(mRes);
	mat4x4_translate(mTranslateDown, 0.0f, -1.0f, -5.0f);
	mat4x4_mul(mRes2, mTranslateDown, mRes);
	mat4x4_transpose(matrixT2, mRes2);

	mat4x4_identity(mRot1);
	mat4x4_identity(mRes);
	mat4x4_rotate_Y(mRes, mRot1, rquad*DEG_TO_RAD);
	mat4x4_translate(mTranslateDown, -1.5f, -0.5f, -4.0f);
	mat4x4_mul(mRes2, mTranslateDown, mRes);
	mat4x4_transpose(matrixT3, mRes2);

	mat4x4_identity(mRot1);
	mat4x4_identity(mTranslate);
	mat4x4_identity(mRes);

	mat4x4_translate(mTranslate, 2.0f, 0.25f, -5.0f);
	mat4x4_rotate_X(mRot1, mRot1, rquad*DEG_TO_RAD);
	mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD*0.5f);
	mat4x4_mul(mRes, mTranslate, mRot1);
	mat4x4_transpose(matrixT4, mRes); // this fucking math library swap rows and columns

	// draw scene
	//
	hrSceneOpen(scnRef, HR_WRITE_DISCARD);
	{
		hrMeshInstance(scnRef, cubeRef, &matrixT[0][0]);
		hrMeshInstance(scnRef, planeRef, &matrixT2[0][0]);
		hrMeshInstance(scnRef, sphRef, &matrixT3[0][0]);
		hrMeshInstance(scnRef, torRef, &matrixT4[0][0]);
	}
	hrSceneClose(scnRef);

	hrFlush(scnRef, settingsRef);
	hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_36/z_out.png");


	HRTextureNodeRef new_testTex2 = hrTexture2DUpdateFromMemory(testTex2, 300, 300, 4, &imageData2[0]);

	hrMaterialOpen(mat0, HR_OPEN_EXISTING);
	{
		xml_node matNode = hrMaterialParamNode(mat0);

		xml_node diff = matNode.append_child(L"diffuse");

		hrTextureBind(new_testTex2, diff);
	}
	hrMaterialClose(mat0);

	hrFlush(scnRef, settingsRef);
	hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_36/z_out2.png");

	id_ok1 = (new_testTex2.id == testTex2.id);
	// frame 3
	//
	
	HRTextureNodeRef new_testTex3 = hrTexture2DUpdateFromMemory(testTex3, 128, 128, 4, &imageData1[0]);

	hrMaterialOpen(mat1, HR_OPEN_EXISTING);
	{
		xml_node matNode = hrMaterialParamNode(mat1);

		xml_node diff = matNode.append_child(L"diffuse");

		hrTextureBind(new_testTex3, diff);
	}
	hrMaterialClose(mat1);

	hrFlush(scnRef, settingsRef);
	hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_36/z_out3.png");

	id_ok2 = (new_testTex3.id == testTex3.id);

	hrErrorCallback(ErrorCallBack);

	return check_images("test_36", 3) && id_ok1 && id_ok2;
}