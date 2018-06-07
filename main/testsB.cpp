#include "tests.h"
#include <math.h>
#include <iomanip>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include <math.h>
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
#include "../hydra_api/HydraPostProcessAPI.h"
#include "../hydra_api/LiteMath.h"
#include "../hydra_api/HydraXMLHelpers.h"

using namespace TEST_UTILS;

extern GLFWwindow* g_window;

using HydraLiteMath::float2;
using HydraLiteMath::float3;
using HydraLiteMath::float4;



bool test81_custom_attributes()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test81");

  HRCameraRef    camRef;
  HRSceneInstRef scnRef;
  HRRenderRef    settingsRef;

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_81", HR_WRITE_DISCARD);

  // geometry
  //
  auto cubeMeshData  = CreateCube(0.75f);
  auto torusMeshData = CreateTorus(0.2f, 0.5f, 32, 32);


  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(2.0f), 1);
  HRMeshRef sphRef   = HRMeshFromSimpleMesh(L"my_sphere", CreateSphere(0.5f, 32), 2);

  HRMeshRef cubeRef  = hrMeshCreate(L"my_cube");

  std::vector<HydraLiteMath::float3> cubeColors(cubeMeshData.vPos.size() / 4);
  for (size_t i = 0; i < cubeColors.size(); i++)
    cubeColors[i] = float3(1, 1, 1);

  for (size_t i = 0; i < cubeColors.size()/2 + 1; i++)
    cubeColors[i] = float3(1, 0, 0);
  
  hrMeshOpen(cubeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeRef, L"pos",      &cubeMeshData.vPos[0]);
    hrMeshVertexAttribPointer4f(cubeRef, L"norm",     &cubeMeshData.vNorm[0]);
    hrMeshVertexAttribPointer2f(cubeRef, L"texcoord", &cubeMeshData.vTexCoord[0]);

    hrMeshVertexAttribPointer3f(cubeRef, L"color",    (const float*)&cubeColors[0]);      // custom attribute

    hrMeshMaterialId      (cubeRef, 0);
    hrMeshAppendTriangles3(cubeRef, int(cubeMeshData.triIndices.size()), &cubeMeshData.triIndices[0]);
  }
  hrMeshClose(cubeRef);


  HRMeshRef torRef = hrMeshCreate(L"my_torus");

  std::vector<float> torusDarkness(torusMeshData.vPos.size() / 4);
  for (size_t i = 0; i < torusDarkness.size(); i++)
    torusDarkness[i] = sinf(  100.0f*float(i)/float(torusDarkness.size())  );

  hrMeshOpen(torRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(torRef, L"pos",      &torusMeshData.vPos[0]);
    hrMeshVertexAttribPointer4f(torRef, L"norm",     &torusMeshData.vNorm[0]);
    hrMeshVertexAttribPointer2f(torRef, L"texcoord", &torusMeshData.vTexCoord[0]);
                           
    hrMeshVertexAttribPointer1f(torRef, L"darkness", (const float*)&torusDarkness[0]);      // custom attribute

    hrMeshMaterialId(torRef, 0);
    hrMeshAppendTriangles3(torRef, int(torusMeshData.triIndices.size()), &torusMeshData.triIndices[0]);
  }
  hrMeshClose(torRef);


  // material and textures
  //
  HRTextureNodeRef testTex2 = hrTexture2DCreateFromFile(L"data/textures/chess_red.bmp");

  HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
  HRMaterialRef mat1 = hrMaterialCreate(L"mysimplemat2");
  HRMaterialRef mat2 = hrMaterialCreate(L"wood");

  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat0);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"0.75 0.75 0.5");

    HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
    hrTextureBind(testTex, diff);
  }
  hrMaterialClose(mat0);

  hrMaterialOpen(mat1, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat1);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"1 1 1");

    hrTextureBind(testTex2, diff);
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
  settingsRef = hrRenderCreate(L"opengl1TestCustomAttributes");

  hrRenderOpen(settingsRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(settingsRef);
    node.append_child(L"width").text()  = 1024;
    node.append_child(L"height").text() = 768;
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
  mat4x4_rotate_X(mRot1, mRot1, -rquad * DEG_TO_RAD);
  mat4x4_rotate_Y(mRot1, mRot1, +rquad * DEG_TO_RAD*0.5f);
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

  hrRenderSaveFrameBufferLDR(settingsRef, L"tests_images/test_81/z_out.png");

  return check_images("test_81");
}


namespace hlm = HydraLiteMath;


bool test38_licence_plate()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_38");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_38", HR_WRITE_DISCARD);

  SimpleMesh cubeOpen = CreateCubeOpen(4.0f);
  SimpleMesh plane = CreatePlane(4.0f);

  // textures
  //
  srand(time(NULL));
  int stateLicPlate = 2;// rand() % 2;
  enum { RusLicPlate2digitReg, RusLicPlate3digitReg, GerLicPlate };

  const int sizeTexBitmap2 = 9;

  HRTextureNodeRef texProc; 
  HRTextureNodeRef texBitmap1Base;  
  HRTextureNodeRef texBitmap2[sizeTexBitmap2];
  HRTextureNodeRef texBitmapRusFont[22] = {
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Russia/Digits/RusFontLicensePlate_0.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Russia/Digits/RusFontLicensePlate_1.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Russia/Digits/RusFontLicensePlate_2.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Russia/Digits/RusFontLicensePlate_3.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Russia/Digits/RusFontLicensePlate_4.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Russia/Digits/RusFontLicensePlate_5.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Russia/Digits/RusFontLicensePlate_6.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Russia/Digits/RusFontLicensePlate_7.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Russia/Digits/RusFontLicensePlate_8.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Russia/Digits/RusFontLicensePlate_9.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Russia/Letters/RusFontLicensePlate_A.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Russia/Letters/RusFontLicensePlate_B.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Russia/Letters/RusFontLicensePlate_C.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Russia/Letters/RusFontLicensePlate_E.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Russia/Letters/RusFontLicensePlate_H.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Russia/Letters/RusFontLicensePlate_K.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Russia/Letters/RusFontLicensePlate_M.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Russia/Letters/RusFontLicensePlate_O.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Russia/Letters/RusFontLicensePlate_P.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Russia/Letters/RusFontLicensePlate_T.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Russia/Letters/RusFontLicensePlate_X.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Russia/Letters/RusFontLicensePlate_Y.png") };

  HRTextureNodeRef texBitmapGerFont[61] = {
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Digits/GerFontLicensePlate_0.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Digits/GerFontLicensePlate_1.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Digits/GerFontLicensePlate_2.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Digits/GerFontLicensePlate_3.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Digits/GerFontLicensePlate_4.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Digits/GerFontLicensePlate_5.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Digits/GerFontLicensePlate_6.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Digits/GerFontLicensePlate_7.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Digits/GerFontLicensePlate_8.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Digits/GerFontLicensePlate_9.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_A.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_A2.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_B.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_C.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_D.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_E.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_F.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_G.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_H.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_I.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_J.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_K.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_L.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_M.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_N.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_O.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_O2.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_P.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_Q.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_R.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_S.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_T.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_U.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_U2.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_V.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_W.png"),    
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_X.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_Y.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Letters/GerFontLicensePlate_Z.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Emblems/GerEmblemsReg_01.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Emblems/GerEmblemsReg_02.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Emblems/GerEmblemsReg_03.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Emblems/GerEmblemsReg_04.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Emblems/GerEmblemsReg_05.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Emblems/GerEmblemsReg_06.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Emblems/GerEmblemsReg_07.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Emblems/GerEmblemsReg_08.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Emblems/GerEmblemsReg_09.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Emblems/GerEmblemsReg_10.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Emblems/GerEmblemsReg_11.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Emblems/GerEmblemsReg_12.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Emblems/GerEmblemsReg_13.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Emblems/GerEmblemsReg_14.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Emblems/GerEmblemsReg_15.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Emblems/GerEmblemsReg_16.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Emblems/GerSafetyCheckSticker_01.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Emblems/GerSafetyCheckSticker_02.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Emblems/GerSafetyCheckSticker_03.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Emblems/GerSafetyCheckSticker_04.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Emblems/GerSafetyCheckSticker_05.png"),
    hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/Emblems/GerSafetyCheckSticker_06.png") };

  switch (stateLicPlate)
  {
  case(RusLicPlate2digitReg):
  {
    texProc = hrTextureCreateAdvanced(L"proc", L"RusLicPlate2digitReg");
    texBitmap1Base = hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Russia/RusLicensePlate2digitRegionBase.png");
    break;
  }
  case(RusLicPlate3digitReg):
  {
    texProc = hrTextureCreateAdvanced(L"proc", L"RusLicPlate3digitReg");
    texBitmap1Base = hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Russia/RusLicensePlate3digitRegionBase.png");
    break;
  }
  case(GerLicPlate):
  {
    texProc = hrTextureCreateAdvanced(L"proc", L"GerLicPlate");
    texBitmap1Base = hrTexture2DCreateFromFile(L"d:/Samsung/Textures/license_plate/Germany/GerLicensePlateBase.png");
    break;
  }
  default:
    break;
  }


  switch (stateLicPlate)
  {
  case(RusLicPlate2digitReg):
  {
    for (int i = 0; i < 5; i++) // 5 digits
      texBitmap2[i] = texBitmapRusFont[rand() % 10];

    for (int i = 5; i < sizeTexBitmap2; i++)  // 3 letters
      texBitmap2[i] = texBitmapRusFont[(rand() % 12) + 10];
    break;
  }
  case(RusLicPlate3digitReg):
  {
    for (int i = 0; i < 6; i++) // 5 digits
      texBitmap2[i] = texBitmapRusFont[rand() % 10];

    for (int i = 6; i < sizeTexBitmap2; i++)  // 3 letters
      texBitmap2[i] = texBitmapRusFont[(rand() % 29) + 10];
    break;
  }
  case(GerLicPlate):
  {
    for (int i = 0; i < 4; i++) // 3 digits
      texBitmap2[i] = texBitmapGerFont[rand() % 10];

    for (int i = 4; i < sizeTexBitmap2; i++)  // 3 letters
      texBitmap2[i] = texBitmapGerFont[(rand() % 29) + 10];

    texBitmap2[7] = texBitmapGerFont[(rand() % 16) + 39];   // emblems regions
    texBitmap2[8] = texBitmapGerFont[(rand() % 6) + 55];   // safety check sticker

    break;
  }
  default:
    break;
  }

    


  hrTextureNodeOpen(texProc, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc);

    xml_node code_node = texNode.append_child(L"code");


    switch (stateLicPlate)
    {
    case(RusLicPlate2digitReg):
    {
      code_node.append_attribute(L"file") = L"data/code/RusLicPlate2digitReg.c";
      break;
    }
    case(RusLicPlate3digitReg):
    {
      code_node.append_attribute(L"file") = L"data/code/RusLicPlate3digitReg.c";
      break;
    }
    case(GerLicPlate):
    {
      code_node.append_attribute(L"file") = L"data/code/GerLicPlate.c";
      break;
    }
    default:
      break;
    }      
    
    code_node.append_attribute(L"main") = L"userProc";
  }
  hrTextureNodeClose(texProc);


  // other as usual in this test
  //
  HRMaterialRef mat0 = hrMaterialCreate(L"myProcMat");
  HRMaterialRef mat1 = hrMaterialCreate(L"red");
  HRMaterialRef mat2 = hrMaterialCreate(L"green");
  HRMaterialRef mat3 = hrMaterialCreate(L"white");

  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat0);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val") = L"0.6 0.6 0.6";
    colorNode.append_attribute(L"tex_apply_mode") = L"replace";

    auto texNode = hrTextureBind(texProc, colorNode);

    // not used currently
    //
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


    // proc texture sampler settings
    //
    xml_node p1 = texNode.append_child(L"arg");
    xml_node p2 = texNode.append_child(L"arg");
    xml_node p3 = texNode.append_child(L"arg");
    xml_node p4 = texNode.append_child(L"arg");

    p1.append_attribute(L"id") = 0;
    p1.append_attribute(L"name") = L"texId1";
    p1.append_attribute(L"type") = L"sampler2D";
    p1.append_attribute(L"size") = 1;
    p1.append_attribute(L"val") = texBitmap1Base.id;

    p2.append_attribute(L"id") = 1;
    p2.append_attribute(L"name") = L"texId2";
    p2.append_attribute(L"type") = L"sampler2D";
    p2.append_attribute(L"size") = sizeTexBitmap2;

    std::wstringstream strOut;
    for (int i = 0; i < sizeTexBitmap2; i++)
      strOut << L" " << texBitmap2[i].id;  
    auto val = strOut.str();
    p2.append_attribute(L"val") = val.c_str();


    p3.append_attribute(L"id") = 2;
    p3.append_attribute(L"name") = L"offset";
    p3.append_attribute(L"type") = L"float2";
    p3.append_attribute(L"size") = 1;
    p3.append_attribute(L"val") = L"0 0";

    p4.append_attribute(L"id") = 3;
    p4.append_attribute(L"name") = L"color";
    p4.append_attribute(L"type") = L"float4";
    p4.append_attribute(L"size") = 1;
    p4.append_attribute(L"val") = L"1 1 1 1";
  }
  hrMaterialClose(mat0);

  hrMaterialOpen(mat1, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat1);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.1 0.1");
  }
  hrMaterialClose(mat1);

  hrMaterialOpen(mat2, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat2);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.1 0.5 0.1");
  }
  hrMaterialClose(mat2);

  hrMaterialOpen(mat3, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat3);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.6 0.6 0.6");
  }
  hrMaterialClose(mat3);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
  HRMeshRef planeRef = hrMeshCreate(L"my_plane");

  hrMeshOpen(cubeOpenRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeOpenRef, L"pos", &cubeOpen.vPos[0]);
    hrMeshVertexAttribPointer4f(cubeOpenRef, L"norm", &cubeOpen.vNorm[0]);
    hrMeshVertexAttribPointer2f(cubeOpenRef, L"texcoord", &cubeOpen.vTexCoord[0]);

    int cubeMatIndices[10] = { mat3.id, mat3.id, mat3.id, mat3.id, mat3.id, mat3.id, mat2.id, mat2.id, mat1.id, mat1.id };

    //hrMeshMaterialId(cubeRef, 0);
    hrMeshPrimitiveAttribPointer1i(cubeOpenRef, L"mind", cubeMatIndices);
    hrMeshAppendTriangles3(cubeOpenRef, int(cubeOpen.triIndices.size()), &cubeOpen.triIndices[0]);
  }
  hrMeshClose(cubeOpenRef);

  hrMeshOpen(planeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(planeRef, L"pos", &plane .vPos[0]);
    hrMeshVertexAttribPointer4f(planeRef, L"norm", &plane.vNorm[0]);
    hrMeshVertexAttribPointer2f(planeRef, L"texcoord", &plane.vTexCoord[0]);

    int planeMatIndices[1] = { mat0.id };

    //hrMeshMaterialId(cubeRef, 0);
    hrMeshPrimitiveAttribPointer1i(planeRef, L"mind", planeMatIndices);
    hrMeshAppendTriangles3(planeRef, int(plane.triIndices.size()), &plane.triIndices[0]);
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
    sizeNode.append_attribute(L"half_width") = 1.0f;

    pugi::xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").text().set(L"1 1 1");
    intensityNode.append_child(L"multiplier").text().set(L"8.0");
  }
  hrLightClose(rectLight);

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
    camNode.append_child(L"position").text().set(L"0 0 14");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  // set up render settings
  //
  HRRenderRef renderRef = hrRenderCreate(L"HydraModern"); // opengl1
  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
  hrRenderLogDir(renderRef, L"C:/[Hydra]/logs/", true);   

  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);

    node.append_child(L"width").text() = L"512";
    node.append_child(L"height").text() = L"512";

    node.append_child(L"method_primary").text() = L"pathtracing";
    node.append_child(L"method_secondary").text() = L"pathtracing";
    node.append_child(L"method_tertiary").text() = L"pathtracing";
    node.append_child(L"method_caustic").text() = L"pathtracing";
    node.append_child(L"shadows").text() = L"1";

    node.append_child(L"trace_depth").text() = L"8";
    node.append_child(L"diff_trace_depth").text() = L"4";
    node.append_child(L"pt_error").text() = L"2.0";
    node.append_child(L"minRaysPerPixel").text() = L"256";
    node.append_child(L"maxRaysPerPixel").text() = L"2048";
  }
  hrRenderClose(renderRef);


  // create scene
  //
  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f; 

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    // instance plane and cornell box
    //
    auto mscale = hlm::scale4x4(float3(1.0f, 1.0f, 0.2311f));
    auto mrot = hlm::rotate_X_4x4(-90.0f*DEG_TO_RAD);
    auto mtranslate = hlm::translate4x4(hlm::float3(0, 0.0f, -1));
    auto mtransform = mul(mtranslate, mul(mrot, mscale));
    hrMeshInstance(scnRef, planeRef, mtransform.L());

    std::cout << "mtransform = " << std::endl;
    for (int y = 0; y < 4; y++)
    {
      for (int x = 0; x < 4; x++)
      {
        std::cout << mtransform.M(x, y) << "  " << "\t";
      }
      std::cout << std::endl;
    }

    mrot = hlm::rotate_Y_4x4(180.0f*DEG_TO_RAD);
    hrMeshInstance(scnRef, cubeOpenRef, mrot.L());

    //// instance light (!!!)
    //
    mtranslate = hlm::translate4x4(hlm::float3(0, 3.85f, 0));
    hrLightInstance(scnRef, rectLight, mtranslate.L());
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef, camRef);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_38/z_out.png");

  return check_images("test_38", 1, 10);
}


bool test82_proc_texture()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_82");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_82", HR_WRITE_DISCARD);

  SimpleMesh sphere   = CreateSphere(2.0f, 128);
  SimpleMesh cubeOpen = CreateCubeOpen(4.0f);

  // textures
  //
  HRTextureNodeRef texBitmap1 = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
  HRTextureNodeRef texBitmap2 = hrTexture2DCreateFromFile(L"data/textures/300px-Bump2.jpg");
  HRTextureNodeRef texProc    = hrTextureCreateAdvanced(L"proc", L"my_custom_faloff"); 
  HRTextureNodeRef texProc2   = hrTextureCreateAdvanced(L"proc", L"my_custom_faloff");

  hrTextureNodeOpen(texProc, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc);
   
    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/mul_tex_coord.c";
    code_node.append_attribute(L"main") = L"userProc";
  }
  hrTextureNodeClose(texProc);

  hrTextureNodeOpen(texProc2, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc2);

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/blue_water.c";
    code_node.append_attribute(L"main") = L"main";
  }
  hrTextureNodeClose(texProc2);


  // other as usual in this test
  //
  HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
  HRMaterialRef mat1 = hrMaterialCreate(L"red");
  HRMaterialRef mat2 = hrMaterialCreate(L"green");
  HRMaterialRef mat3 = hrMaterialCreate(L"white");

  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat0);

    auto emission = matNode.append_child(L"emission");
    {
      emission.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);

      auto color = emission.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.25 0.25 0.25");
      color.append_attribute(L"tex_apply_mode ").set_value(L"multiply");

      auto texNode = hrTextureBind(texProc2, color);
    }

    xml_node diff = matNode.append_child(L"diffuse");
    {
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      auto colorNode = diff.append_child(L"color");

      colorNode.append_attribute(L"val") = L"0.5 0.5 0.5";

      auto texNode = hrTextureBind(texBitmap1, colorNode);

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
    }

  }
  hrMaterialClose(mat0);

  hrMaterialOpen(mat1, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat1);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val") = L"0.5 0.0 0.0";
    colorNode.append_attribute(L"tex_apply_mode") = L"replace";

    auto texNode = hrTextureBind(texProc, colorNode);

    // not used currently
    //
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


    // proc texture sampler settings
    //
    xml_node p1 = texNode.append_child(L"arg"); 
    xml_node p2 = texNode.append_child(L"arg");
    xml_node p3 = texNode.append_child(L"arg");
    xml_node p4 = texNode.append_child(L"arg");
   
    p1.append_attribute(L"id")   = 0;
    p1.append_attribute(L"name") = L"texId1";
    p1.append_attribute(L"type") = L"sampler2D";
    p1.append_attribute(L"size") = 1;
    p1.append_attribute(L"val")  = texBitmap1.id;
     
    p2.append_attribute(L"id")   = 1;
    p2.append_attribute(L"name") = L"texId2";
    p2.append_attribute(L"type") = L"sampler2D";
    p2.append_attribute(L"size") = 1;
    p2.append_attribute(L"val")  = texBitmap2.id;

    p3.append_attribute(L"id")   = 3;
    p3.append_attribute(L"name") = L"offset";
    p3.append_attribute(L"type") = L"float2";
    p3.append_attribute(L"size") = 1;
    p3.append_attribute(L"val")  = L"0 0";

    p4.append_attribute(L"id")   = 4;
    p4.append_attribute(L"name") = L"color";
    p4.append_attribute(L"type") = L"float4";
    p4.append_attribute(L"size") = 1;
    p4.append_attribute(L"val")  = L"1 1 1 1";
  }
  hrMaterialClose(mat1);

  hrMaterialOpen(mat2, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat2);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.5 0.0");
  }
  hrMaterialClose(mat2);

  hrMaterialOpen(mat3, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat3);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(mat3);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
  HRMeshRef planeRef = hrMeshCreate(L"my_plane");
  HRMeshRef sphereRef = hrMeshCreate(L"my_sphere");

  hrMeshOpen(cubeOpenRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeOpenRef, L"pos", &cubeOpen.vPos[0]);
    hrMeshVertexAttribPointer4f(cubeOpenRef, L"norm", &cubeOpen.vNorm[0]);
    hrMeshVertexAttribPointer2f(cubeOpenRef, L"texcoord", &cubeOpen.vTexCoord[0]);

    int cubeMatIndices[10] = { mat3.id, mat3.id, mat3.id, mat3.id, mat3.id, mat3.id, mat2.id, mat2.id, mat1.id, mat1.id };

    //hrMeshMaterialId(cubeRef, 0);
    hrMeshPrimitiveAttribPointer1i(cubeOpenRef, L"mind", cubeMatIndices);
    hrMeshAppendTriangles3(cubeOpenRef, int(cubeOpen.triIndices.size()), &cubeOpen.triIndices[0]);
  }
  hrMeshClose(cubeOpenRef);

  hrMeshOpen(sphereRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(sphereRef, L"pos", &sphere.vPos[0]);
    hrMeshVertexAttribPointer4f(sphereRef, L"norm", &sphere.vNorm[0]);
    hrMeshVertexAttribPointer2f(sphereRef, L"texcoord", &sphere.vTexCoord[0]);

    for (size_t i = 0; i < sphere.matIndices.size(); i++)
      sphere.matIndices[i] = mat0.id;

    hrMeshPrimitiveAttribPointer1i(sphereRef, L"mind", &sphere.matIndices[0]);
    hrMeshAppendTriangles3(sphereRef, int32_t(sphere.triIndices.size()), &sphere.triIndices[0]);
  }
  hrMeshClose(sphereRef);

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
    sizeNode.append_attribute(L"half_width") = 1.0f;

    pugi::xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").text().set(L"1 1 1");
    intensityNode.append_child(L"multiplier").text().set(L"8.0");
  }
  hrLightClose(rectLight);

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
    camNode.append_child(L"position").text().set(L"0 0 14");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  // set up render settings
  //
  HRRenderRef renderRef = hrRenderCreate(L"HydraModern"); // opengl1
  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);

  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);

    node.append_child(L"width").text()  = L"512";
    node.append_child(L"height").text() = L"512";

    node.append_child(L"method_primary").text()   = L"pathtracing";
    node.append_child(L"method_secondary").text() = L"pathtracing";
    node.append_child(L"method_tertiary").text()  = L"pathtracing";
    node.append_child(L"method_caustic").text()   = L"pathtracing";
    node.append_child(L"shadows").text()          = L"1";

    node.append_child(L"trace_depth").text()      = L"8";
    node.append_child(L"diff_trace_depth").text() = L"4";
    node.append_child(L"maxRaysPerPixel").text()  = L"2048";

    node.append_child(L"evalgbuffer").text() = 1;
  }
  hrRenderClose(renderRef);

  // create scene
  //
  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    // instance sphere and cornell box
    //
    auto mtranslate = hlm::translate4x4(hlm::float3(0, -2, 1));
    hrMeshInstance(scnRef, sphereRef, mtranslate.L());

    auto mrot = hlm::rotate_Y_4x4(180.0f*DEG_TO_RAD);
    hrMeshInstance(scnRef, cubeOpenRef, mrot.L());

    //// instance light (!!!)
    //
    mtranslate = hlm::translate4x4(hlm::float3(0, 3.85f, 0));
    hrLightInstance(scnRef, rectLight, mtranslate.L());
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef, camRef);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_82/z_out.png");
  hrRenderSaveGBufferLayerLDR(renderRef, L"tests_images/test_82/z_out2.png", L"diffcolor");

  return check_images("test_82", 1, 10);
}


bool test83_proc_texture2()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_83");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_83", HR_WRITE_DISCARD);

  SimpleMesh sphere   = CreateSphere(2.0f, 128);
  SimpleMesh cubeOpen = CreateCubeOpen(4.0f);

  // textures
  //
  HRTextureNodeRef texBitmap1 = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
  HRTextureNodeRef texBitmap2 = hrTexture2DCreateFromFile(L"data/textures/300px-Bump2.jpg");
  HRTextureNodeRef texProc    = hrTextureCreateAdvanced  (L"proc", L"my_custom_faloff");
  HRTextureNodeRef texProc2   = hrTextureCreateAdvanced  (L"proc", L"my_show_normals");

  hrTextureNodeOpen(texProc, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc);

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/test_tex_array.c"; 
    code_node.append_attribute(L"main") = L"userProc";
  }
  hrTextureNodeClose(texProc);

  hrTextureNodeOpen(texProc2, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc2);

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/show_normals.c";
    code_node.append_attribute(L"main") = L"mainNorm";
  }
  hrTextureNodeClose(texProc2);


  // other as usual in this test
  //
  HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
  HRMaterialRef mat1 = hrMaterialCreate(L"red");
  HRMaterialRef mat2 = hrMaterialCreate(L"green");
  HRMaterialRef mat3 = hrMaterialCreate(L"white");

  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat0);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val") = L"0.5 0.5 0.5";
    colorNode.append_attribute(L"tex_apply_mode") = L"replace";

    auto texNode = hrTextureBind(texProc2, colorNode);

    // not used currently #TODO: figure out what of theese we needed!
    //
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
  }
  hrMaterialClose(mat0);

  hrMaterialOpen(mat1, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat1);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val") = L"0.5 0.0 0.0";
    colorNode.append_attribute(L"tex_apply_mode") = L"replace";

    auto texNode = hrTextureBind(texProc, colorNode);

    // not used currently #TODO: figure out what of theese we needed!
    //
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

    // proc texture sampler settings
    //
    xml_node p1 = texNode.append_child(L"arg");
    xml_node p2 = texNode.append_child(L"arg");
    xml_node p3 = texNode.append_child(L"arg");
    xml_node p4 = texNode.append_child(L"arg");

    std::wstringstream strOut;
    strOut << texBitmap1.id << L" " << texBitmap2.id;
    auto val = strOut.str();

    p1.append_attribute(L"id")   = 0;
    p1.append_attribute(L"name") = L"texId1";
    p1.append_attribute(L"type") = L"sampler2D";
    p1.append_attribute(L"size") = 2;
    p1.append_attribute(L"val")  = val.c_str();

    p4.append_attribute(L"id")   = 1;
    p4.append_attribute(L"name") = L"color";
    p4.append_attribute(L"type") = L"float4";
    p4.append_attribute(L"size") = 1;
    p4.append_attribute(L"val")  = L"1 1 1 1";
  }
  hrMaterialClose(mat1);

  hrMaterialOpen(mat2, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat2);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.5 0.0");
  }
  hrMaterialClose(mat2);

  hrMaterialOpen(mat3, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat3);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(mat3);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
  HRMeshRef planeRef = hrMeshCreate(L"my_plane");
  HRMeshRef sphereRef = hrMeshCreate(L"my_sphere");

  hrMeshOpen(cubeOpenRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeOpenRef, L"pos", &cubeOpen.vPos[0]);
    hrMeshVertexAttribPointer4f(cubeOpenRef, L"norm", &cubeOpen.vNorm[0]);
    hrMeshVertexAttribPointer2f(cubeOpenRef, L"texcoord", &cubeOpen.vTexCoord[0]);

    int cubeMatIndices[10] = { mat3.id, mat3.id, mat3.id, mat3.id, mat3.id, mat3.id, mat2.id, mat2.id, mat1.id, mat1.id };

    //hrMeshMaterialId(cubeRef, 0);
    hrMeshPrimitiveAttribPointer1i(cubeOpenRef, L"mind", cubeMatIndices);
    hrMeshAppendTriangles3(cubeOpenRef, int(cubeOpen.triIndices.size()), &cubeOpen.triIndices[0]);
  }
  hrMeshClose(cubeOpenRef);

  hrMeshOpen(sphereRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(sphereRef, L"pos", &sphere.vPos[0]);
    hrMeshVertexAttribPointer4f(sphereRef, L"norm", &sphere.vNorm[0]);
    hrMeshVertexAttribPointer2f(sphereRef, L"texcoord", &sphere.vTexCoord[0]);

    for (size_t i = 0; i < sphere.matIndices.size(); i++)
      sphere.matIndices[i] = mat0.id;

    hrMeshPrimitiveAttribPointer1i(sphereRef, L"mind", &sphere.matIndices[0]);
    hrMeshAppendTriangles3(sphereRef, int32_t(sphere.triIndices.size()), &sphere.triIndices[0]);
  }
  hrMeshClose(sphereRef);

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
    sizeNode.append_attribute(L"half_width") = 1.0f;

    pugi::xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").text().set(L"1 1 1");
    intensityNode.append_child(L"multiplier").text().set(L"8.0");
  }
  hrLightClose(rectLight);

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
    camNode.append_child(L"position").text().set(L"0 0 14");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  // set up render settings
  //
  HRRenderRef renderRef = hrRenderCreate(L"HydraModern"); // opengl1
  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
  hrRenderLogDir(renderRef, L"C:/[Hydra]/logs/", false);

  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);

    node.append_child(L"width").text() = L"512";
    node.append_child(L"height").text() = L"512";

    node.append_child(L"method_primary").text() = L"pathtracing";
    node.append_child(L"method_secondary").text() = L"pathtracing";
    node.append_child(L"method_tertiary").text() = L"pathtracing";
    node.append_child(L"method_caustic").text() = L"pathtracing";
    node.append_child(L"shadows").text() = L"1";

    node.append_child(L"trace_depth").text() = L"8";
    node.append_child(L"diff_trace_depth").text() = L"4";
    node.append_child(L"pt_error").text() = L"2.0";
    node.append_child(L"minRaysPerPixel").text() = L"256";
    node.append_child(L"maxRaysPerPixel").text() = L"2048";
  }
  hrRenderClose(renderRef);

  // create scene
  //
  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    // instance sphere and cornell box
    //
    auto mtranslate = hlm::translate4x4(hlm::float3(0, -2, 1));
    hrMeshInstance(scnRef, sphereRef, mtranslate.L());

    auto mrot = hlm::rotate_Y_4x4(180.0f*DEG_TO_RAD);
    hrMeshInstance(scnRef, cubeOpenRef, mrot.L());

    //// instance light (!!!)
    //
    mtranslate = hlm::translate4x4(hlm::float3(0, 3.85f, 0));
    hrLightInstance(scnRef, rectLight, mtranslate.L());
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef, camRef);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_83/z_out.png");

  return check_images("test_83", 1, 10);
}


bool test84_proc_texture2()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_84");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_84", HR_WRITE_DISCARD);

  SimpleMesh sphere   = CreateSphere(2.0f, 128);
  SimpleMesh cubeOpen = CreateCubeOpen(4.0f);

  // textures
  //
  HRTextureNodeRef texBitmap1 = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
  HRTextureNodeRef texBitmap2 = hrTexture2DCreateFromFile(L"data/textures/300px-Bump2.jpg");
  HRTextureNodeRef texProc    = hrTextureCreateAdvanced(L"proc", L"my_custom_faloff");
  HRTextureNodeRef texProc2   = hrTextureCreateAdvanced(L"proc", L"my_show_normals");

  hrTextureNodeOpen(texProc, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc);

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/mul_tex_coord.c";
    code_node.append_attribute(L"main") = L"userProc";
  }
  hrTextureNodeClose(texProc);

  hrTextureNodeOpen(texProc2, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc2);

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/faloff_example.c";
    code_node.append_attribute(L"main") = L"main";
  }
  hrTextureNodeClose(texProc2);


  // other as usual in this test
  //
  HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
  HRMaterialRef mat1 = hrMaterialCreate(L"red");
  HRMaterialRef mat2 = hrMaterialCreate(L"green");
  HRMaterialRef mat3 = hrMaterialCreate(L"white");

  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat0);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val") = L"0.5 0.5 0.5";
    colorNode.append_attribute(L"tex_apply_mode") = L"replace";

    auto texNode = hrTextureBind(texProc2, colorNode);
    {
      xml_node p1 = texNode.append_child(L"arg");
      xml_node p2 = texNode.append_child(L"arg");

      p1.append_attribute(L"id") = 0;
      p1.append_attribute(L"name") = L"color1";
      p1.append_attribute(L"type") = L"float4";
      p1.append_attribute(L"size") = 1;
      p1.append_attribute(L"val") = L"1 0 0 0";

      p2.append_attribute(L"id") = 1;
      p2.append_attribute(L"name") = L"color2";
      p2.append_attribute(L"type") = L"float4";
      p2.append_attribute(L"size") = 1;
      p2.append_attribute(L"val") = L"0 0 1 0";
    }

    // not used currently #TODO: figure out what of theese we needed!
    //
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
  }
  hrMaterialClose(mat0);

  hrMaterialOpen(mat1, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat1);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val") = L"0.5 0.0 0.0";
    colorNode.append_attribute(L"tex_apply_mode") = L"replace";

    auto texNode = hrTextureBind(texProc, colorNode);

    // not used currently #TODO: figure out what of theese we needed!
    //
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


    // proc texture sampler settings
    //
    xml_node p1 = texNode.append_child(L"arg");
    xml_node p2 = texNode.append_child(L"arg");
    xml_node p3 = texNode.append_child(L"arg");
    xml_node p4 = texNode.append_child(L"arg");

    p1.append_attribute(L"id") = 0;
    p1.append_attribute(L"name") = L"texId1";
    p1.append_attribute(L"type") = L"sampler2D";
    p1.append_attribute(L"size") = 1;
    p1.append_attribute(L"val") = texBitmap1.id;

    p2.append_attribute(L"id") = 1;
    p2.append_attribute(L"name") = L"texId2";
    p2.append_attribute(L"type") = L"sampler2D";
    p2.append_attribute(L"size") = 1;
    p2.append_attribute(L"val") = texBitmap2.id;

    p3.append_attribute(L"id") = 3;
    p3.append_attribute(L"name") = L"offset";
    p3.append_attribute(L"type") = L"float2";
    p3.append_attribute(L"size") = 1;
    p3.append_attribute(L"val") = L"0 0";

    p4.append_attribute(L"id") = 4;
    p4.append_attribute(L"name") = L"color";
    p4.append_attribute(L"type") = L"float4";
    p4.append_attribute(L"size") = 1;
    p4.append_attribute(L"val") = L"1 1 1 1";
  }
  hrMaterialClose(mat1);

  hrMaterialOpen(mat2, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat2);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.5 0.0");
  }
  hrMaterialClose(mat2);

  hrMaterialOpen(mat3, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat3);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(mat3);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
  HRMeshRef planeRef = hrMeshCreate(L"my_plane");
  HRMeshRef sphereRef = hrMeshCreate(L"my_sphere");

  hrMeshOpen(cubeOpenRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeOpenRef, L"pos", &cubeOpen.vPos[0]);
    hrMeshVertexAttribPointer4f(cubeOpenRef, L"norm", &cubeOpen.vNorm[0]);
    hrMeshVertexAttribPointer2f(cubeOpenRef, L"texcoord", &cubeOpen.vTexCoord[0]);

    int cubeMatIndices[10] = { mat3.id, mat3.id, mat3.id, mat3.id, mat3.id, mat3.id, mat2.id, mat2.id, mat1.id, mat1.id };

    //hrMeshMaterialId(cubeRef, 0);
    hrMeshPrimitiveAttribPointer1i(cubeOpenRef, L"mind", cubeMatIndices);
    hrMeshAppendTriangles3(cubeOpenRef, int(cubeOpen.triIndices.size()), &cubeOpen.triIndices[0]);
  }
  hrMeshClose(cubeOpenRef);

  hrMeshOpen(sphereRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(sphereRef, L"pos", &sphere.vPos[0]);
    hrMeshVertexAttribPointer4f(sphereRef, L"norm", &sphere.vNorm[0]);
    hrMeshVertexAttribPointer2f(sphereRef, L"texcoord", &sphere.vTexCoord[0]);

    for (size_t i = 0; i < sphere.matIndices.size(); i++)
      sphere.matIndices[i] = mat0.id;

    hrMeshPrimitiveAttribPointer1i(sphereRef, L"mind", &sphere.matIndices[0]);
    hrMeshAppendTriangles3(sphereRef, int32_t(sphere.triIndices.size()), &sphere.triIndices[0]);
  }
  hrMeshClose(sphereRef);

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
    sizeNode.append_attribute(L"half_width") = 1.0f;

    pugi::xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").text().set(L"1 1 1");
    intensityNode.append_child(L"multiplier").text().set(L"8.0");
  }
  hrLightClose(rectLight);

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
    camNode.append_child(L"position").text().set(L"0 0 14");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  // set up render settings
  //
  HRRenderRef renderRef = hrRenderCreate(L"HydraModern"); // opengl1
  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
  //hrRenderLogDir(renderRef, L"/home/frol/hydra/logs/", false);


  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);

    node.append_child(L"width").text() = L"512";
    node.append_child(L"height").text() = L"512";

    node.append_child(L"method_primary").text() = L"pathtracing";
    node.append_child(L"method_secondary").text() = L"pathtracing";
    node.append_child(L"method_tertiary").text() = L"pathtracing";
    node.append_child(L"method_caustic").text() = L"pathtracing";
    node.append_child(L"shadows").text() = L"1";

    node.append_child(L"trace_depth").text() = L"8";
    node.append_child(L"diff_trace_depth").text() = L"4";
    node.append_child(L"pt_error").text() = L"2.0";
    node.append_child(L"minRaysPerPixel").text() = L"256";
    node.append_child(L"maxRaysPerPixel").text() = L"256";
  }
  hrRenderClose(renderRef);

  // create scene
  //
  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    // instance sphere and cornell box
    //
    auto mtranslate = hlm::translate4x4(hlm::float3(0, -2, 1));
    hrMeshInstance(scnRef, sphereRef, mtranslate.L());

    auto mrot = hlm::rotate_Y_4x4(180.0f*DEG_TO_RAD);
    hrMeshInstance(scnRef, cubeOpenRef, mrot.L());

    //// instance light (!!!)
    //
    mtranslate = hlm::translate4x4(hlm::float3(0, 3.85f, 0));
    hrLightInstance(scnRef, rectLight, mtranslate.L());
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef, camRef);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_84/z_out.png");

  return check_images("test_84", 1, 10);
}


bool test85_proc_texture_ao()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_85"); 

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_85", HR_WRITE_DISCARD);

  SimpleMesh sphere   = CreateSphere(2.0f, 128);
  SimpleMesh cubeOpen = CreateCubeOpen(4.0f);

  // textures
  //
  HRTextureNodeRef texBitmap1 = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp"); 
  HRTextureNodeRef texBitmap2 = hrTexture2DCreateFromFile(L"data/textures/300px-Bump2.jpg");
  HRTextureNodeRef texProc    = hrTextureCreateAdvanced(L"proc", L"my_custom_faloff");
  HRTextureNodeRef texProc2   = hrTextureCreateAdvanced(L"proc", L"my_show_normals");
  HRTextureNodeRef texProc3   = hrTextureCreateAdvanced(L"proc", L"my_ao_test");

  hrTextureNodeOpen(texProc, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc);

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/mul_tex_coord.c";
    code_node.append_attribute(L"main") = L"userProc";
  }
  hrTextureNodeClose(texProc);

  hrTextureNodeOpen(texProc2, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc2);

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/noise3D.c";
    code_node.append_attribute(L"main") = L"main";
  }
  hrTextureNodeClose(texProc2);

  hrTextureNodeOpen(texProc3, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc3);

    // Occluded color().       // put them in source code later! this test is only for AO.
    // Unoccluded color().     // put them in source code later! this test is only for AO.
    // Falloff(0.1f - 10.0f) -.     // put them in source code later! this test is only for AO.
    // 

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/show_ao.c";
    code_node.append_attribute(L"main") = L"main";
  }
  hrTextureNodeClose(texProc3);


  // other as usual in this test
  //
  HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
  HRMaterialRef mat1 = hrMaterialCreate(L"red");
  HRMaterialRef mat2 = hrMaterialCreate(L"green");
  HRMaterialRef mat3 = hrMaterialCreate(L"white");

  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat0);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val") = L"0.5 0.5 0.5";
    colorNode.append_attribute(L"tex_apply_mode") = L"replace";

    auto texNode = hrTextureBind(texProc2, colorNode);
  }
  hrMaterialClose(mat0);

  hrMaterialOpen(mat1, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat1);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val") = L"0.5 0.0 0.0";
    colorNode.append_attribute(L"tex_apply_mode") = L"replace";

    auto texNode = hrTextureBind(texProc, colorNode);

    // not used currently #TODO: figure out what of theese we needed!
    //
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


    // proc texture sampler settings
    //
    xml_node p1 = texNode.append_child(L"arg");
    xml_node p2 = texNode.append_child(L"arg");
    xml_node p3 = texNode.append_child(L"arg");
    xml_node p4 = texNode.append_child(L"arg");

    p1.append_attribute(L"id") = 0;
    p1.append_attribute(L"name") = L"texId1";
    p1.append_attribute(L"type") = L"sampler2D";
    p1.append_attribute(L"size") = 1;
    p1.append_attribute(L"val") = texBitmap1.id;

    p2.append_attribute(L"id") = 1;
    p2.append_attribute(L"name") = L"texId2";
    p2.append_attribute(L"type") = L"sampler2D";
    p2.append_attribute(L"size") = 1;
    p2.append_attribute(L"val") = texBitmap2.id;

    p3.append_attribute(L"id") = 3;
    p3.append_attribute(L"name") = L"offset";
    p3.append_attribute(L"type") = L"float2";
    p3.append_attribute(L"size") = 1;
    p3.append_attribute(L"val") = L"0 0";

    p4.append_attribute(L"id") = 4;
    p4.append_attribute(L"name") = L"color";
    p4.append_attribute(L"type") = L"float4";
    p4.append_attribute(L"size") = 1;
    p4.append_attribute(L"val") = L"1 1 1 1";
  }
  hrMaterialClose(mat1);

  hrMaterialOpen(mat2, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat2);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.5 0.0");
  }
  hrMaterialClose(mat2);

  hrMaterialOpen(mat3, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat3);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val").set_value(L"0.5 0.5 0.5");

    auto texNode = hrTextureBind(texProc3, colorNode);
    texNode.append_attribute(L"input_gamma") = 1.0f;

    xml_node aoNode = texNode.append_child(L"ao");

    // Distribution: Corner, Edge, Both(Corner and Edge).
    //
    aoNode.append_attribute(L"length") = 1.25f;

    auto texAoLengthNode = hrTextureBind(texBitmap1, aoNode);
    {
      // set ao length texture params
    }

    // Distribution: Corner/Up, Edge/Down, Both/Both(Corner and Edge).
    //
    aoNode.append_attribute(L"hemisphere") = L"up";

    // Only for this object(on / off) - �������� ������ ��� ����� �������.
    //
    aoNode.append_attribute(L"local") = 0;
  }
  hrMaterialClose(mat3);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
  HRMeshRef planeRef    = hrMeshCreate(L"my_plane");
  HRMeshRef sphereRef   = hrMeshCreate(L"my_sphere");

  hrMeshOpen(cubeOpenRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeOpenRef, L"pos", &cubeOpen.vPos[0]);
    hrMeshVertexAttribPointer4f(cubeOpenRef, L"norm", &cubeOpen.vNorm[0]);
    hrMeshVertexAttribPointer2f(cubeOpenRef, L"texcoord", &cubeOpen.vTexCoord[0]);

    int cubeMatIndices[10] = { mat3.id, mat3.id, mat3.id, mat3.id, mat3.id, mat3.id, mat2.id, mat2.id, mat1.id, mat1.id };

    //hrMeshMaterialId(cubeRef, 0);
    hrMeshPrimitiveAttribPointer1i(cubeOpenRef, L"mind", cubeMatIndices);
    hrMeshAppendTriangles3(cubeOpenRef, int(cubeOpen.triIndices.size()), &cubeOpen.triIndices[0]);
  }
  hrMeshClose(cubeOpenRef);

  hrMeshOpen(sphereRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(sphereRef, L"pos", &sphere.vPos[0]);
    hrMeshVertexAttribPointer4f(sphereRef, L"norm", &sphere.vNorm[0]);
    hrMeshVertexAttribPointer2f(sphereRef, L"texcoord", &sphere.vTexCoord[0]);

    for (size_t i = 0; i < sphere.matIndices.size(); i++)
      sphere.matIndices[i] = mat0.id;

    hrMeshPrimitiveAttribPointer1i(sphereRef, L"mind", &sphere.matIndices[0]);
    hrMeshAppendTriangles3(sphereRef, int32_t(sphere.triIndices.size()), &sphere.triIndices[0]);
  }
  hrMeshClose(sphereRef);

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
    sizeNode.append_attribute(L"half_width") = 1.0f;

    pugi::xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").text().set(L"1 1 1");
    intensityNode.append_child(L"multiplier").text().set(L"8.0");
  }
  hrLightClose(rectLight);

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
    camNode.append_child(L"position").text().set(L"0 0 14");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  // set up render settings
  //
  HRRenderRef renderRef = hrRenderCreate(L"HydraModern"); // opengl1
  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
  hrRenderLogDir(renderRef, L"C:/[Hydra]/logs/", true);


  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);

    node.append_child(L"width").text() = L"512";
    node.append_child(L"height").text() = L"512";

    node.append_child(L"method_primary").text() = L"pathtracing";
    node.append_child(L"method_secondary").text() = L"pathtracing";
    node.append_child(L"method_tertiary").text() = L"pathtracing";
    node.append_child(L"method_caustic").text() = L"pathtracing";
    node.append_child(L"shadows").text() = L"1";

    node.append_child(L"trace_depth").text() = L"8";
    node.append_child(L"diff_trace_depth").text() = L"4";
    node.append_child(L"pt_error").text() = L"2.0";
    node.append_child(L"minRaysPerPixel").text() = L"256";
    node.append_child(L"maxRaysPerPixel").text() = L"2048";
  }
  hrRenderClose(renderRef);

  // create scene
  //
  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    // instance sphere and cornell box
    //
    auto mtranslate = hlm::translate4x4(hlm::float3(0, -2, 1));
    hrMeshInstance(scnRef, sphereRef, mtranslate.L());

    auto mrot = hlm::rotate_Y_4x4(180.0f*DEG_TO_RAD);
    hrMeshInstance(scnRef, cubeOpenRef, mrot.L());

    //// instance light (!!!)
    //
    mtranslate = hlm::translate4x4(hlm::float3(0, 3.85f, 0));
    hrLightInstance(scnRef, rectLight, mtranslate.L());
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef, camRef);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_85/z_out.png");

  return check_images("test_85", 1, 10);
}


bool test86_proc_texture_ao_dirt()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_86"); 

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_86", HR_WRITE_DISCARD);

  SimpleMesh sphere   = CreateSphere(1.25f, 128);
  SimpleMesh cubeOpen = CreateCubeOpen(4.0f);

  // textures
  //
  HRTextureNodeRef texBitmap1 = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
  HRTextureNodeRef texBitmap2 = hrTexture2DCreateFromFile(L"data/textures/300px-Bump2.jpg");
  HRTextureNodeRef texProc    = hrTextureCreateAdvanced(L"proc", L"my_custom_faloff");
  HRTextureNodeRef texProc2   = hrTextureCreateAdvanced(L"proc", L"my_show_normals");
  HRTextureNodeRef texProc3   = hrTextureCreateAdvanced(L"proc", L"my_ao_test");

  hrTextureNodeOpen(texProc, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc);

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/mul_tex_coord.c";
    code_node.append_attribute(L"main") = L"userProc";
  }
  hrTextureNodeClose(texProc);

  hrTextureNodeOpen(texProc2, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc2);

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/blue_water2.c";
    code_node.append_attribute(L"main") = L"main";
  }
  hrTextureNodeClose(texProc2);

  hrTextureNodeOpen(texProc3, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc3);

    // Occluded color().       // put them in source code later! this test is only for AO.
    // Unoccluded color().     // put them in source code later! this test is only for AO.
    // Falloff(0.1f - 10.0f).     // put them in source code later! this test is only for AO.
    // 
    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/noise3D_mul_ao.c";
    code_node.append_attribute(L"main") = L"main";
  }
  hrTextureNodeClose(texProc3);


  // other as usual in this test
  //
  HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
  HRMaterialRef mat1 = hrMaterialCreate(L"red");
  HRMaterialRef mat2 = hrMaterialCreate(L"green");
  HRMaterialRef mat3 = hrMaterialCreate(L"white");

  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat0);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val")            = L"0.5 0.5 0.5";
    colorNode.append_attribute(L"tex_apply_mode") = L"replace";

    auto texNode = hrTextureBind(texProc2, colorNode);
  }
  hrMaterialClose(mat0);

  hrMaterialOpen(mat1, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat1);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val")            = L"0.5 0.0 0.0";
    colorNode.append_attribute(L"tex_apply_mode") = L"replace";

    auto texNode = hrTextureBind(texProc, colorNode);

    // not used currently #TODO: figure out what of theese we needed!
    //
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


    // proc texture sampler settings
    //
    xml_node p1 = texNode.append_child(L"arg");
    xml_node p2 = texNode.append_child(L"arg");
    xml_node p3 = texNode.append_child(L"arg");
    xml_node p4 = texNode.append_child(L"arg");

    p1.append_attribute(L"id") = 0;
    p1.append_attribute(L"name") = L"texId1";
    p1.append_attribute(L"type") = L"sampler2D";
    p1.append_attribute(L"size") = 1;
    p1.append_attribute(L"val") = texBitmap1.id;

    p2.append_attribute(L"id") = 1;
    p2.append_attribute(L"name") = L"texId2";
    p2.append_attribute(L"type") = L"sampler2D";
    p2.append_attribute(L"size") = 1;
    p2.append_attribute(L"val") = texBitmap2.id;

    p3.append_attribute(L"id") = 3;
    p3.append_attribute(L"name") = L"offset";
    p3.append_attribute(L"type") = L"float2";
    p3.append_attribute(L"size") = 1;
    p3.append_attribute(L"val") = L"0 0";

    p4.append_attribute(L"id") = 4;
    p4.append_attribute(L"name") = L"color";
    p4.append_attribute(L"type") = L"float4";
    p4.append_attribute(L"size") = 1;
    p4.append_attribute(L"val") = L"1 1 1 1";
  }
  hrMaterialClose(mat1);

  hrMaterialOpen(mat2, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat2);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.5 0.0");
  }
  hrMaterialClose(mat2);

  hrMaterialOpen(mat3, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat3);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val").set_value(L"0.5 0.5 0.5");

    auto texNode = hrTextureBind(texProc3, colorNode);
    texNode.append_attribute(L"input_gamma") = 1.0f;

    xml_node aoNode = texNode.append_child(L"ao");

    // Distribution: Corner, Edge, Both(Corner and Edge).
    //
    aoNode.append_attribute(L"length") = 1.25f;

    // Distribution: Corner/Up, Edge/Down, Both/Both(Corner and Edge).
    //
    aoNode.append_attribute(L"hemisphere") = L"up";

    // Only for this object(on / off) - �������� ������ ��� ����� �������.
    //
    aoNode.append_attribute(L"local") = 1;
  }
  hrMaterialClose(mat3);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
  HRMeshRef planeRef    = hrMeshCreate(L"my_plane");
  HRMeshRef sphereRef   = hrMeshCreate(L"my_sphere");

  hrMeshOpen(cubeOpenRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeOpenRef, L"pos", &cubeOpen.vPos[0]);
    hrMeshVertexAttribPointer4f(cubeOpenRef, L"norm", &cubeOpen.vNorm[0]);
    hrMeshVertexAttribPointer2f(cubeOpenRef, L"texcoord", &cubeOpen.vTexCoord[0]);

    int cubeMatIndices[10] = { mat3.id, mat3.id, mat3.id, mat3.id, mat3.id, mat3.id, mat2.id, mat2.id, mat1.id, mat1.id };

    //hrMeshMaterialId(cubeRef, 0);
    hrMeshPrimitiveAttribPointer1i(cubeOpenRef, L"mind", cubeMatIndices);
    hrMeshAppendTriangles3(cubeOpenRef, int(cubeOpen.triIndices.size()), &cubeOpen.triIndices[0]);
  }
  hrMeshClose(cubeOpenRef);

  hrMeshOpen(sphereRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(sphereRef, L"pos", &sphere.vPos[0]);
    hrMeshVertexAttribPointer4f(sphereRef, L"norm", &sphere.vNorm[0]);
    hrMeshVertexAttribPointer2f(sphereRef, L"texcoord", &sphere.vTexCoord[0]);

    for (size_t i = 0; i < sphere.matIndices.size(); i++)
      sphere.matIndices[i] = mat0.id;

    hrMeshPrimitiveAttribPointer1i(sphereRef, L"mind", &sphere.matIndices[0]);
    hrMeshAppendTriangles3(sphereRef, int32_t(sphere.triIndices.size()), &sphere.triIndices[0]);
  }
  hrMeshClose(sphereRef);

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
    sizeNode.append_attribute(L"half_width") = 1.0f;

    pugi::xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").text().set(L"1 1 1");
    intensityNode.append_child(L"multiplier").text().set(L"8.0");
  }
  hrLightClose(rectLight);

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
    camNode.append_child(L"position").text().set(L"0 0 14");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  // set up render settings
  //
  HRRenderRef renderRef = hrRenderCreate(L"HydraModern"); // opengl1
  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
  hrRenderLogDir(renderRef, L"C:/[Hydra]/logs/", true);


  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);

    node.append_child(L"width").text() = L"512";
    node.append_child(L"height").text() = L"512";

    node.append_child(L"method_primary").text() = L"pathtracing";
    node.append_child(L"method_secondary").text() = L"pathtracing";
    node.append_child(L"method_tertiary").text() = L"pathtracing";
    node.append_child(L"method_caustic").text() = L"pathtracing";
    node.append_child(L"shadows").text() = L"1";

    node.append_child(L"trace_depth").text() = L"8";
    node.append_child(L"diff_trace_depth").text() = L"4";
    node.append_child(L"pt_error").text() = L"2.0";
    node.append_child(L"minRaysPerPixel").text() = L"256";
    node.append_child(L"maxRaysPerPixel").text() = L"2048";
  }
  hrRenderClose(renderRef);

  // create scene
  //
  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    // instance sphere and cornell box
    //
    auto mtranslate = hlm::translate4x4(hlm::float3(1.5f, -2, 1)); 
    hrMeshInstance(scnRef, sphereRef, mtranslate.L());

    auto mtranslate2 = hlm::translate4x4(hlm::float3(-1.5f, -2, 1)); 
    hrMeshInstance(scnRef, sphereRef, mtranslate2.L());

    auto mrot = hlm::rotate_Y_4x4(180.0f*DEG_TO_RAD);
    hrMeshInstance(scnRef, cubeOpenRef, mrot.L());

    //// instance light (!!!)
    //
    mtranslate = hlm::translate4x4(hlm::float3(0, 3.85f, 0));
    hrLightInstance(scnRef, rectLight, mtranslate.L());
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef, camRef);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_86/z_out.png");

  return check_images("test_86", 1, 10);
}


bool test87_proc_texture_reflect()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_87");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_87", HR_WRITE_DISCARD);

  SimpleMesh sphere   = CreateSphere(2.0f, 128);
  SimpleMesh cubeOpen = CreateCubeOpen(4.0f);

  // textures
  //
  HRTextureNodeRef texBitmap1 = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
  HRTextureNodeRef texBitmap2 = hrTexture2DCreateFromFile(L"data/textures/300px-Bump2.jpg");
  HRTextureNodeRef texProc    = hrTextureCreateAdvanced(L"proc", L"my_custom_faloff");
  HRTextureNodeRef texProc2   = hrTextureCreateAdvanced(L"proc", L"my_show_normals");
  HRTextureNodeRef texProc3   = hrTextureCreateAdvanced(L"proc", L"my_ao_test");
  HRTextureNodeRef texProc4   = hrTextureCreateAdvanced(L"proc", L"my_voronoi_test");

  hrTextureNodeOpen(texProc, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc);

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/mul_tex_coord.c";
    code_node.append_attribute(L"main") = L"userProc";
  }
  hrTextureNodeClose(texProc);

  hrTextureNodeOpen(texProc2, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc2);

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/noise3D.c";
    code_node.append_attribute(L"main") = L"main";
  }
  hrTextureNodeClose(texProc2);

  hrTextureNodeOpen(texProc3, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc3);

    // Occluded color().       // put them in source code later! this test is only for AO.
    // Unoccluded color().     // put them in source code later! this test is only for AO.
    // Falloff(0.1f - 10.0f).     // put them in source code later! this test is only for AO.
    // 
    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/noise3D_mul_ao.c";
    code_node.append_attribute(L"main") = L"main";

    xml_node aoNode = texNode.append_child(L"ao");
  }
  hrTextureNodeClose(texProc3);

  hrTextureNodeOpen(texProc4, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc4);

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/blue_water.c";
    code_node.append_attribute(L"main") = L"main";
  }
  hrTextureNodeClose(texProc4);


  // other as usual in this test
  //
  HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
  HRMaterialRef mat1 = hrMaterialCreate(L"red");
  HRMaterialRef mat2 = hrMaterialCreate(L"green");
  HRMaterialRef mat3 = hrMaterialCreate(L"white");

  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat0);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val") = L"0.5 0.5 0.5";
    colorNode.append_attribute(L"tex_apply_mode") = L"replace";

    auto texNode = hrTextureBind(texProc4, colorNode); 
    texNode.append_attribute(L"input_gamma") = 1.0f;

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"phong");

    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(0);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(2.5);

    auto reflColor = refl.append_child(L"color");
    reflColor.append_attribute(L"val").set_value(L"0.5 0.5 0.5");

    auto texNodeR = hrTextureBind(texProc2, reflColor);
  }
  hrMaterialClose(mat0);

  hrMaterialOpen(mat1, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat1);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val") = L"0.5 0.0 0.0";
    colorNode.append_attribute(L"tex_apply_mode") = L"replace";

    auto texNode = hrTextureBind(texProc, colorNode);

    // not used currently #TODO: figure out what of theese we needed!
    //
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


    // proc texture sampler settings
    //
    xml_node p1 = texNode.append_child(L"arg");
    xml_node p2 = texNode.append_child(L"arg");
    xml_node p3 = texNode.append_child(L"arg");
    xml_node p4 = texNode.append_child(L"arg");

    p1.append_attribute(L"id") = 0;
    p1.append_attribute(L"name") = L"texId1";
    p1.append_attribute(L"type") = L"sampler2D";
    p1.append_attribute(L"size") = 1;
    p1.append_attribute(L"val") = texBitmap1.id;

    p2.append_attribute(L"id") = 1;
    p2.append_attribute(L"name") = L"texId2";
    p2.append_attribute(L"type") = L"sampler2D";
    p2.append_attribute(L"size") = 1;
    p2.append_attribute(L"val") = texBitmap2.id;

    p3.append_attribute(L"id") = 3;
    p3.append_attribute(L"name") = L"offset";
    p3.append_attribute(L"type") = L"float2";
    p3.append_attribute(L"size") = 1;
    p3.append_attribute(L"val") = L"0 0";

    p4.append_attribute(L"id") = 4;
    p4.append_attribute(L"name") = L"color";
    p4.append_attribute(L"type") = L"float4";
    p4.append_attribute(L"size") = 1;
    p4.append_attribute(L"val") = L"1 1 1 1";
  }
  hrMaterialClose(mat1);

  hrMaterialOpen(mat2, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat2);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.5 0.0");
  }
  hrMaterialClose(mat2);

  hrMaterialOpen(mat3, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat3);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(mat3);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
  HRMeshRef planeRef    = hrMeshCreate(L"my_plane");
  HRMeshRef sphereRef   = hrMeshCreate(L"my_sphere");

  hrMeshOpen(cubeOpenRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeOpenRef, L"pos", &cubeOpen.vPos[0]);
    hrMeshVertexAttribPointer4f(cubeOpenRef, L"norm", &cubeOpen.vNorm[0]);
    hrMeshVertexAttribPointer2f(cubeOpenRef, L"texcoord", &cubeOpen.vTexCoord[0]);

    int cubeMatIndices[10] = { mat3.id, mat3.id, mat3.id, mat3.id, mat3.id, mat3.id, mat2.id, mat2.id, mat1.id, mat1.id };

    //hrMeshMaterialId(cubeRef, 0);
    hrMeshPrimitiveAttribPointer1i(cubeOpenRef, L"mind", cubeMatIndices);
    hrMeshAppendTriangles3(cubeOpenRef, int(cubeOpen.triIndices.size()), &cubeOpen.triIndices[0]);
  }
  hrMeshClose(cubeOpenRef);

  hrMeshOpen(sphereRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(sphereRef, L"pos", &sphere.vPos[0]);
    hrMeshVertexAttribPointer4f(sphereRef, L"norm", &sphere.vNorm[0]);
    hrMeshVertexAttribPointer2f(sphereRef, L"texcoord", &sphere.vTexCoord[0]);

    for (size_t i = 0; i < sphere.matIndices.size(); i++)
      sphere.matIndices[i] = mat0.id;

    hrMeshPrimitiveAttribPointer1i(sphereRef, L"mind", &sphere.matIndices[0]);
    hrMeshAppendTriangles3(sphereRef, int32_t(sphere.triIndices.size()), &sphere.triIndices[0]);
  }
  hrMeshClose(sphereRef);

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
    sizeNode.append_attribute(L"half_width") = 1.0f;

    pugi::xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").text().set(L"1 1 1");
    intensityNode.append_child(L"multiplier").text().set(L"8.0");
  }
  hrLightClose(rectLight);

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
    camNode.append_child(L"position").text().set(L"0 0 14");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  // set up render settings
  //
  HRRenderRef renderRef = hrRenderCreate(L"HydraModern"); // opengl1
  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
  hrRenderLogDir(renderRef, L"C:/[Hydra]/logs/", true);


  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);

    node.append_child(L"width").text() = L"512";
    node.append_child(L"height").text() = L"512";

    node.append_child(L"method_primary").text() = L"pathtracing";
    node.append_child(L"method_secondary").text() = L"pathtracing";
    node.append_child(L"method_tertiary").text() = L"pathtracing";
    node.append_child(L"method_caustic").text() = L"pathtracing";
    node.append_child(L"shadows").text() = L"1";

    node.append_child(L"trace_depth").text() = L"8";
    node.append_child(L"diff_trace_depth").text() = L"4";
    node.append_child(L"pt_error").text() = L"2.0";
    node.append_child(L"minRaysPerPixel").text() = L"256";
    node.append_child(L"maxRaysPerPixel").text() = L"2048";
  }
  hrRenderClose(renderRef);

  // create scene
  //
  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    // instance sphere and cornell box
    //
    auto mtranslate = hlm::translate4x4(hlm::float3(0, -2, 1));
    hrMeshInstance(scnRef, sphereRef, mtranslate.L());

    auto mrot = hlm::rotate_Y_4x4(180.0f*DEG_TO_RAD);
    hrMeshInstance(scnRef, cubeOpenRef, mrot.L());

    //// instance light (!!!)
    //
    mtranslate = hlm::translate4x4(hlm::float3(0, 3.85f, 0));
    hrLightInstance(scnRef, rectLight, mtranslate.L());
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef, camRef);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_87/z_out.png");

  return check_images("test_87", 1, 10);
}


bool test88_proc_texture_convex_rust()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_88");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_88", HR_WRITE_DISCARD);

  SimpleMesh sphere   = CreateSphere(2.0f, 128);
  SimpleMesh cubeOpen = CreateCubeOpen(4.0f);
  SimpleMesh cube     = CreateCube(1.0f);

  // textures
  //
  HRTextureNodeRef texBitmap1 = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
  HRTextureNodeRef texBitmap2 = hrTexture2DCreateFromFile(L"data/textures/300px-Bump2.jpg");
  HRTextureNodeRef texProc    = hrTextureCreateAdvanced  (L"proc", L"my_custom_faloff");
  HRTextureNodeRef texProc2   = hrTextureCreateAdvanced  (L"proc", L"my_show_normals");
  HRTextureNodeRef texProc3   = hrTextureCreateAdvanced  (L"proc", L"my_ao_test");
  HRTextureNodeRef texProc4   = hrTextureCreateAdvanced  (L"proc", L"my_voronoi_test");
  HRTextureNodeRef texProc5   = hrTextureCreateAdvanced  (L"proc", L"my_cavity_test");

  hrTextureNodeOpen(texProc, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc);

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/mul_tex_coord.c";
    code_node.append_attribute(L"main") = L"userProc";
  }
  hrTextureNodeClose(texProc);

  hrTextureNodeOpen(texProc2, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc2);

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/noise3D.c";
    code_node.append_attribute(L"main") = L"main";
  }
  hrTextureNodeClose(texProc2);

  hrTextureNodeOpen(texProc3, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc3);

    // Occluded color().       // put them in source code later! this test is only for AO.
    // Unoccluded color().     // put them in source code later! this test is only for AO.
    // Falloff(0.1f - 10.0f) -.     // put them in source code later! this test is only for AO.
    // 
    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/noise3D_mul_ao.c";
    code_node.append_attribute(L"main") = L"main";
  }
  hrTextureNodeClose(texProc3);

  hrTextureNodeOpen(texProc4, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc4);

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/blue_water.c";
    code_node.append_attribute(L"main") = L"main";
  }
  hrTextureNodeClose(texProc4);


  hrTextureNodeOpen(texProc5, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc5);

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/voronoi.c";
    code_node.append_attribute(L"main") = L"main";

    xml_node aoNode = texNode.append_child(L"ao");

    // Distribution: Corner, Edge, Both(Corner and Edge).
    //
    aoNode.append_attribute(L"length") = 0.5f;

    // Distribution: Corner/Up, Edge/Down, Both/Both(Corner and Edge).
    //
    aoNode.append_attribute(L"hemisphere") = L"down";

    // Only for this object(on / off) -
    //
    aoNode.append_attribute(L"local") = 1;
  }
  hrTextureNodeClose(texProc5);


  // other as usual in this test
  //
  HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
  HRMaterialRef mat1 = hrMaterialCreate(L"red");
  HRMaterialRef mat2 = hrMaterialCreate(L"green");
  HRMaterialRef mat3 = hrMaterialCreate(L"white");
  HRMaterialRef mat4 = hrMaterialCreate(L"cubemat1");

  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat0);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val") = L"0.5 0.5 0.5";
    colorNode.append_attribute(L"tex_apply_mode") = L"replace";

    auto texNode = hrTextureBind(texProc4, colorNode); 
    texNode.append_attribute(L"input_gamma") = 1.0f;

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"phong");

    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(0);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(2.5);

    auto reflColor = refl.append_child(L"color");
    reflColor.append_attribute(L"val").set_value(L"0.5 0.5 0.5");

    auto texNodeR = hrTextureBind(texProc2, reflColor);
    //texNodeR.append_attribute(L"input_gamma") = 1.0f;
  }
  hrMaterialClose(mat0);

  hrMaterialOpen(mat1, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat1);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val") = L"0.5 0.0 0.0";
    colorNode.append_attribute(L"tex_apply_mode") = L"replace";

    auto texNode = hrTextureBind(texProc, colorNode);

    // not used currently #TODO: figure out what of theese we needed!
    //
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


    // proc texture sampler settings
    //
    xml_node p1 = texNode.append_child(L"arg");
    xml_node p2 = texNode.append_child(L"arg");
    xml_node p3 = texNode.append_child(L"arg");
    xml_node p4 = texNode.append_child(L"arg");

    p1.append_attribute(L"id") = 0;
    p1.append_attribute(L"name") = L"texId1";
    p1.append_attribute(L"type") = L"sampler2D";
    p1.append_attribute(L"size") = 1;
    p1.append_attribute(L"val") = texBitmap1.id;

    p2.append_attribute(L"id") = 1;
    p2.append_attribute(L"name") = L"texId2";
    p2.append_attribute(L"type") = L"sampler2D";
    p2.append_attribute(L"size") = 1;
    p2.append_attribute(L"val") = texBitmap2.id;

    p3.append_attribute(L"id") = 3;
    p3.append_attribute(L"name") = L"offset";
    p3.append_attribute(L"type") = L"float2";
    p3.append_attribute(L"size") = 1;
    p3.append_attribute(L"val") = L"0 0";

    p4.append_attribute(L"id") = 4;
    p4.append_attribute(L"name") = L"color";
    p4.append_attribute(L"type") = L"float4";
    p4.append_attribute(L"size") = 1;
    p4.append_attribute(L"val") = L"1 1 1 1";
  }
  hrMaterialClose(mat1);

  hrMaterialOpen(mat2, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat2);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.5 0.0");
  }
  hrMaterialClose(mat2);

  hrMaterialOpen(mat3, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat3);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val").set_value(L"0.5 0.5 0.5");

    auto texNode = hrTextureBind(texProc3, colorNode);
    texNode.append_attribute(L"input_gamma") = 1.0f;

    xml_node aoNode = texNode.append_child(L"ao");

    aoNode.append_attribute(L"length")     = 1.25f;
    aoNode.append_attribute(L"hemisphere") = L"up";
    aoNode.append_attribute(L"local")      = 1;
  }
  hrMaterialClose(mat3);

  hrMaterialOpen(mat4, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat4);
    xml_node diff    = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val").set_value(L"0.5 0.5 0.5");

    auto texNode = hrTextureBind(texProc5, colorNode);
    texNode.append_attribute(L"input_gamma") = 1.0f;

    xml_node p1 = texNode.append_child(L"arg");
    xml_node p2 = texNode.append_child(L"arg");
    xml_node p3 = texNode.append_child(L"arg");
     
    p1.append_attribute(L"id")   = 0;
    p1.append_attribute(L"name") = L"a";
    p1.append_attribute(L"type") = L"float4";
    p1.append_attribute(L"size") = 1;
    p1.append_attribute(L"val")  = L"0.01 0.1 0.1 1.0";
    
    p2.append_attribute(L"id")   = 1;
    p2.append_attribute(L"name") = L"b";
    p2.append_attribute(L"type") = L"float4";
    p2.append_attribute(L"size") = 1;
    p2.append_attribute(L"val")  = L"0.75 0.05 0.05 1.0";
   
    p3.append_attribute(L"id")   = 2;
    p3.append_attribute(L"name") = L"power";
    p3.append_attribute(L"type") = L"float";
    p3.append_attribute(L"size") = 1;
    p3.append_attribute(L"val")  = 0.5f;

  }
  hrMaterialClose(mat4);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
  HRMeshRef planeRef    = hrMeshCreate(L"my_plane");
  HRMeshRef sphereRef   = hrMeshCreate(L"my_sphere");
  HRMeshRef cubeRef     = hrMeshCreate(L"my_cube");

  hrMeshOpen(cubeOpenRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeOpenRef, L"pos", &cubeOpen.vPos[0]);
    hrMeshVertexAttribPointer4f(cubeOpenRef, L"norm", &cubeOpen.vNorm[0]);
    hrMeshVertexAttribPointer2f(cubeOpenRef, L"texcoord", &cubeOpen.vTexCoord[0]);

    int cubeMatIndices[10] = { mat3.id, mat3.id, mat3.id, mat3.id, mat3.id, mat3.id, mat2.id, mat2.id, mat1.id, mat1.id };

    //hrMeshMaterialId(cubeRef, 0);
    hrMeshPrimitiveAttribPointer1i(cubeOpenRef, L"mind", cubeMatIndices);
    hrMeshAppendTriangles3(cubeOpenRef, int(cubeOpen.triIndices.size()), &cubeOpen.triIndices[0]);
  }
  hrMeshClose(cubeOpenRef);

  hrMeshOpen(sphereRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(sphereRef, L"pos", &sphere.vPos[0]);
    hrMeshVertexAttribPointer4f(sphereRef, L"norm", &sphere.vNorm[0]);
    hrMeshVertexAttribPointer2f(sphereRef, L"texcoord", &sphere.vTexCoord[0]);

    for (size_t i = 0; i < sphere.matIndices.size(); i++)
      sphere.matIndices[i] = mat0.id;

    hrMeshPrimitiveAttribPointer1i(sphereRef, L"mind", &sphere.matIndices[0]);
    hrMeshAppendTriangles3(sphereRef, int32_t(sphere.triIndices.size()), &sphere.triIndices[0]);
  }
  hrMeshClose(sphereRef);

  hrMeshOpen(cubeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeRef, L"pos", &cube.vPos[0]);
    hrMeshVertexAttribPointer4f(cubeRef, L"norm", &cube.vNorm[0]);
    hrMeshVertexAttribPointer2f(cubeRef, L"texcoord", &cube.vTexCoord[0]);

    hrMeshMaterialId(cubeRef, mat4.id);
    hrMeshAppendTriangles3(cubeRef, int(cube.triIndices.size()), &cube.triIndices[0]);
  }
  hrMeshClose(cubeRef);

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
    sizeNode.append_attribute(L"half_width") = 1.0f;

    pugi::xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").text().set(L"1 1 1");
    intensityNode.append_child(L"multiplier").text().set(L"8.0");
  }
  hrLightClose(rectLight);

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
    camNode.append_child(L"position").text().set(L"0 1 10");
    camNode.append_child(L"look_at").text().set(L"0 -1 0");
  }
  hrCameraClose(camRef);

  // set up render settings
  //
  HRRenderRef renderRef = hrRenderCreate(L"HydraModern"); // opengl1
  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
  hrRenderLogDir(renderRef, L"C:/[Hydra]/logs/", true);


  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);

    node.append_child(L"width").text() = L"512";
    node.append_child(L"height").text() = L"512";

    node.append_child(L"method_primary").text() = L"pathtracing";
    node.append_child(L"method_secondary").text() = L"pathtracing";
    node.append_child(L"method_tertiary").text() = L"pathtracing";
    node.append_child(L"method_caustic").text() = L"pathtracing";
    node.append_child(L"shadows").text() = L"1";

    node.append_child(L"trace_depth").text() = L"8";
    node.append_child(L"diff_trace_depth").text() = L"4";
    node.append_child(L"pt_error").text() = L"2.0";
    node.append_child(L"minRaysPerPixel").text() = L"256";
    node.append_child(L"maxRaysPerPixel").text() = L"2048";
  }
  hrRenderClose(renderRef);

  // create scene
  //
  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    // instance sphere and cornell box
    //
    auto mscale     = hlm::scale4x4(float3(1.5f, 1.5f, 1.5f));
    auto mrot1      = hlm::rotate_Y_4x4(-25.0f*DEG_TO_RAD); 
    auto mtranslate = hlm::translate4x4(hlm::float3(0, -2.5f, 1));
    auto mtransform = mul(mtranslate, mul(mrot1, mscale));
    hrMeshInstance(scnRef, cubeRef, mtransform.L());

    auto mrot = hlm::rotate_Y_4x4(180.0f*DEG_TO_RAD);
    hrMeshInstance(scnRef, cubeOpenRef, mrot.L());

    //// instance light (!!!)
    //
    mtranslate = hlm::translate4x4(hlm::float3(0, 3.85f, 0));
    hrLightInstance(scnRef, rectLight, mtranslate.L());
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef, camRef);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_88/z_out.png");

  return check_images("test_88", 1, 30);
}


bool test89_proc_texture_dirty()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_89");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneLibraryOpen(L"tests/test_89", HR_WRITE_DISCARD);

  SimpleMesh sphere   = CreateSphere(2.0f, 128);
  SimpleMesh cubeOpen = CreateCubeOpen(4.0f);
  SimpleMesh cube     = CreateCube(1.0f);

  // textures
  //
  HRTextureNodeRef texProc2 = hrTextureCreateAdvanced(L"proc", L"noise_test");
  HRTextureNodeRef texProc4 = hrTextureCreateAdvanced(L"proc", L"my_voronoi_test");


  hrTextureNodeOpen(texProc2, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc2);

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/noise_v2.c";
    code_node.append_attribute(L"main") = L"main";

    xml_node p1 = texNode.append_child(L"arg");
    xml_node p2 = texNode.append_child(L"arg");

    p1.append_attribute(L"id")   = 0;
    p1.append_attribute(L"name") = L"color1";
    p1.append_attribute(L"type") = L"float3";
    p1.append_attribute(L"size") = 1;
    p1.append_attribute(L"val")  = L"0.2 0.4 1.0";

    p2.append_attribute(L"id")   = 1;
    p2.append_attribute(L"name") = L"color2";
    p2.append_attribute(L"type") = L"float3";
    p2.append_attribute(L"size") = 1;
    p2.append_attribute(L"val")  = L"0.65 0.33 0.0";
  }
  hrTextureNodeClose(texProc2);

  hrTextureNodeOpen(texProc4, HR_WRITE_DISCARD);
  {
    xml_node texNode = hrTextureParamNode(texProc4);

    xml_node code_node = texNode.append_child(L"code");
    code_node.append_attribute(L"file") = L"data/code/blue_water.c";
    code_node.append_attribute(L"main") = L"main";
  }
  hrTextureNodeClose(texProc4);


  // other as usual in this test
  //
  HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
  HRMaterialRef mat1 = hrMaterialCreate(L"proc");

  HRMaterialRef matA = hrMaterialCreate(L"matA");
  HRMaterialRef matB = hrMaterialCreate(L"matB");
  HRMaterialRef matBlend = hrMaterialCreateBlend(L"blend", matB, matA);

  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat0);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val") = L"0.5 0.5 0.5";
    colorNode.append_attribute(L"tex_apply_mode") = L"replace";
  }
  hrMaterialClose(mat0);

  hrMaterialOpen(mat1, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat1);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val") = L"0.5 0.5 0.5";
    colorNode.append_attribute(L"tex_apply_mode") = L"replace";

    auto texNode = hrTextureBind(texProc2, colorNode);
    texNode.append_attribute(L"input_gamma") = 1.0f;

    xml_node p1 = texNode.append_child(L"arg");
    xml_node p2 = texNode.append_child(L"arg");

    p1.append_attribute(L"id")   = 0;
    p1.append_attribute(L"name") = L"color1";
    p1.append_attribute(L"type") = L"float3";
    p1.append_attribute(L"size") = 1;
    p1.append_attribute(L"val")  = L"0.2 0.4 1.0";

    p2.append_attribute(L"id")   = 1;
    p2.append_attribute(L"name") = L"color2";
    p2.append_attribute(L"type") = L"float3";
    p2.append_attribute(L"size") = 1;
    p2.append_attribute(L"val")  = L"0.32 0.17 0.0";

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"phong");

    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(8.0f);

    auto reflColor = refl.append_child(L"color");
    reflColor.append_attribute(L"val").set_value(L"0.5 0.5 0.5");

    auto texNodeR = hrTextureBind(texProc2, reflColor);

    p1 = texNodeR.append_child(L"arg");
    p2 = texNodeR.append_child(L"arg");

    p1.append_attribute(L"id")   = 0;
    p1.append_attribute(L"name") = L"color1";
    p1.append_attribute(L"type") = L"float3";
    p1.append_attribute(L"size") = 1;
    p1.append_attribute(L"val")  = L"1.0 1.0 1.0";

    p2.append_attribute(L"id")   = 1;
    p2.append_attribute(L"name") = L"color2";
    p2.append_attribute(L"type") = L"float3";
    p2.append_attribute(L"size") = 1;
    p2.append_attribute(L"val")  = L"0.0 0.0 0.0";
  }
  hrMaterialClose(mat1);

  hrMaterialOpen(matA, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(matA);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val") = L"0.32 0.17 0.0";
  }
  hrMaterialClose(matA);

  hrMaterialOpen(matB, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(matB);
    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    auto colorNode = diff.append_child(L"color");

    colorNode.append_attribute(L"val") = L"0.2 0.4 1.0";

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"phong");

    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(8.0f);

    auto reflColor = refl.append_child(L"color");
    reflColor.append_attribute(L"val").set_value(L"0.2 0.4 1.0");
  }
  hrMaterialClose(matB);


  hrMaterialOpen(matBlend, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matBlend);

    auto blend = matNode.append_child(L"blend");
    blend.append_attribute(L"type").set_value(L"mask_blend");

    auto mask = blend.append_child(L"mask");
    mask.append_attribute(L"val").set_value(1.0f);

    auto texNodeR = hrTextureBind(texProc2, mask);

    auto p1 = texNodeR.append_child(L"arg");
    auto p2 = texNodeR.append_child(L"arg");
    texNodeR.append_attribute(L"input_gamma") = 1.0f;

    p1.append_attribute(L"id")   = 0;
    p1.append_attribute(L"name") = L"color1";
    p1.append_attribute(L"type") = L"float3";
    p1.append_attribute(L"size") = 1;
    p1.append_attribute(L"val")  = L"1.0 1.0 1.0";

    p2.append_attribute(L"id")   = 1;
    p2.append_attribute(L"name") = L"color2";
    p2.append_attribute(L"type") = L"float3";
    p2.append_attribute(L"size") = 1;
    p2.append_attribute(L"val")  = L"0.0 0.0 0.0";
  }
  hrMaterialClose(matBlend);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRMeshRef planeRef = hrMeshCreate(L"my_plane");
  HRMeshRef lucyRef  = hrMeshCreateFromFileDL(L"data/meshes/lucy.vsgf");

  SimpleMesh plane    = CreatePlane(10.0f);
  hrMeshOpen(planeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(planeRef, L"pos", &plane.vPos[0]);
    hrMeshVertexAttribPointer4f(planeRef, L"norm", &plane.vNorm[0]);
    hrMeshVertexAttribPointer2f(planeRef, L"texcoord", &plane.vTexCoord[0]);

    hrMeshMaterialId(planeRef, mat0.id);
    hrMeshAppendTriangles3(planeRef, int32_t(plane.triIndices.size()), &plane.triIndices[0]);
  }
  hrMeshClose(planeRef);

  hrMeshOpen(lucyRef, HR_TRIANGLE_IND3, HR_OPEN_EXISTING);
  {
    hrMeshMaterialId(lucyRef, matBlend.id);
    //hrMeshMaterialId(lucyRef, mat1.id);
  }
  hrMeshClose(lucyRef);

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
    sizeNode.append_attribute(L"half_width") = 1.0f;

    pugi::xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").text().set(L"1 1 1");
    intensityNode.append_child(L"multiplier").text().set(L"2.0");
  }
  hrLightClose(rectLight);

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

    camNode.append_child(L"fov").text().set(L"30");
    camNode.append_child(L"nearClipPlane").text().set(L"0.01");
    camNode.append_child(L"farClipPlane").text().set(L"100.0");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 1 4");
    camNode.append_child(L"look_at").text().set(L"0 1 0");
  }
  hrCameraClose(camRef);

  // set up render settings
  //
  HRRenderRef renderRef = hrRenderCreate(L"HydraModern"); // opengl1
  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
  hrRenderLogDir(renderRef, L"/tmp/hydra_logs", true);


  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);

    node.append_child(L"width").text() = L"1024";
    node.append_child(L"height").text() = L"1024";

    node.append_child(L"method_primary").text() = L"pathtracing";
    node.append_child(L"method_secondary").text() = L"pathtracing";
    node.append_child(L"method_tertiary").text() = L"pathtracing";
    node.append_child(L"method_caustic").text() = L"pathtracing";
    node.append_child(L"shadows").text() = L"1";

    node.append_child(L"trace_depth").text() = L"8";
    node.append_child(L"diff_trace_depth").text() = L"4";
    node.append_child(L"pt_error").text() = L"2.0";
    node.append_child(L"minRaysPerPixel").text() = L"256";
    node.append_child(L"maxRaysPerPixel").text() = L"1024";
  }
  hrRenderClose(renderRef);

  // create scene
  //
  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    // instance sphere and cornell box
    //
    auto mscale     = hlm::scale4x4(float3(0.25f, 0.25f, 0.25f));
    //auto mrot1      = hlm::rotate_Y_4x4(-25.0f*DEG_TO_RAD);
    //auto mtranslate = hlm::translate4x4(hlm::float3(0, -2.5f, 1));
   // auto mtransform = mul(mtranslate, mul(mrot1, mscale));

    hrMeshInstance(scnRef, lucyRef, mscale.L());

    //auto mrot = hlm::rotate_Y_4x4(180.0f*DEG_TO_RAD);
    hrMeshInstance(scnRef, planeRef, hlm::float4x4().L());

    //// instance light (!!!)
    //
    auto mtranslate = hlm::translate4x4(hlm::float3(0, 3.5f, 0));
    hrLightInstance(scnRef, rectLight, mtranslate.L());
  }
  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef, camRef);


  int w = 1024;
  int h = 1024;

  glViewport(0, 0, w , h);
  std::vector<int32_t> image(w * h);

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      hrRenderGetFrameBufferLDR1i(renderRef, w,h , &image[0]);

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

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_89/z_out.png");

  return check_images("test_89", 1, 10);
}


