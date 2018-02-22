//
// Created by hikawa on 18.08.17.
//

#pragma once

#include "RenderDriverOpenGL32Forward.h"

using namespace GL_RENDER_DRIVER_UTILS;

struct RD_OGL32_Deferred : IHRRenderDriver
{
  RD_OGL32_Deferred();

  void ClearAll() override;
  HRDriverAllocInfo AllocAll(HRDriverAllocInfo a_info) override;

  void GetLastErrorW(wchar_t a_msg[256]) override;

  bool UpdateImage(int32_t a_texId, int32_t w, int32_t h, int32_t bpp, const void* a_data, pugi::xml_node a_texNode) override;
  bool UpdateMaterial(int32_t a_matId, pugi::xml_node a_materialNode) override;
  bool UpdateLight(int32_t a_lightIdId, pugi::xml_node a_lightNode) override;

  bool UpdateMesh(int32_t a_meshId, pugi::xml_node a_meshNode, const HRMeshDriverInput &a_input, const HRBatchInfo *a_batchList, int32_t a_listSize) override;

  bool UpdateImageFromFile(int32_t a_texId, const wchar_t* a_fileName, pugi::xml_node a_texNode) override {return false;}
  bool UpdateMeshFromFile(int32_t a_meshId, pugi::xml_node a_meshNode, const wchar_t* a_fileName) override {return false;}


  bool UpdateCamera(pugi::xml_node a_camNode) override;
  bool UpdateSettings(pugi::xml_node a_settingsNode) override;

  /////////////////////////////////////////////////////////////////////////////////////////////

  void BeginScene(pugi::xml_node a_sceneNode) override;
  void EndScene() override;
  void InstanceMeshes(int32_t a_mesh_id, const float* a_matrices, int32_t a_instNum, const int* a_lightInstId, const int* a_remapId) override;
  void InstanceLights(int32_t a_light_id, const float* a_matrix, pugi::xml_node* a_custAttrArray, int32_t a_instNum, int32_t a_lightGroupId) override;

  void Draw() override;

  HRRenderUpdateInfo HaveUpdateNow(int a_maxRaysPerPixel) override;

  void GetFrameBufferHDR(int32_t w, int32_t h, float*   a_out, const wchar_t* a_layerName) override;
  void GetFrameBufferLDR(int32_t w, int32_t h, int32_t* a_out) override;

  void GetGBufferLine(int32_t a_lineNumber, HRGBufferPixel* a_lineData, int32_t a_startX, int32_t a_endX) override {}

  HRDriverInfo Info() override;
  const HRRenderDeviceInfoListElem* DeviceList() const override { return nullptr; }
  bool EnableDevice(int32_t id, bool a_enable) override { return true; }

protected:

  enum class DRAW_LIGHT_GEO_MODE
  {
    LIGHT_GEO,
    LIGHT_VOLUME,
    NOTHING
  };

  DRAW_LIGHT_GEO_MODE m_drawLightsMode;

  bool m_enableSSAO;

  float CalcLightBoundingSphereRadius(float3 a_color, float a_intensity);

  float DrawOneLight(int i, float4x4 &&sphModel);

  void DrawLightGeoInstances(GLuint &vao, GLuint &vboMat, GLuint &vboColor, int numIndices);

  void SSAOPass() const;

  void CreateMaterialsUBO(int numMat);
  void CreateMatricesUBO();
  void CreateLightSettingsUBO();

  std::wstring m_libPath;
  std::wstring m_msg;

  using meshData = std::unordered_map<int, std::pair<GLuint, int>>;

  std::unordered_map<int32_t, meshData> m_objects; //meshId -> {matId -> vao, indicesNum}
  ShaderProgram m_gBufferProgram;
  ShaderProgram m_lightPassProgram;
  ShaderProgram m_stencilProgram;
  ShaderProgram m_quadProgram;
  ShaderProgram m_lightsProgram;
  /* ShaderProgram m_ssaoProgram;
  ShaderProgram m_ssaoBlurProgram;*/

  bool m_doSSAOPass;

  std::unique_ptr<RenderTexture2D> m_fullScreenTexture;
  std::unique_ptr<GBuffer> m_gBuffer;
  std::unique_ptr<SSAOBuffer> m_ssaoBuffer;
  std::unique_ptr<FullScreenQuad>  m_quad;

  std::vector<GLuint> m_texturesList;

  std::vector<GLuint> m_lightTextures;

  std::vector<float3> m_diffColors;
  std::vector<float3> m_reflColors;
  std::vector<float>  m_reflGloss;

  GLuint m_materialUBO;
  GLuint m_materialUBOBindingPoint;
  GLsizei m_materialStructSize;

  GLuint m_matricesUBO;
  GLuint m_matricesUBOBindingPoint;

  GLuint  m_lightUBO;
  GLuint  m_lightUBOBindingPoint;
  GLsizei m_lightStructSize;

  std::vector<int> m_diffTexId;
  std::vector<int> m_reflTexId;
  std::vector<int> m_normalTexId;

  std::vector<float4x4> m_texMatrices;

  std::vector<float4x4> m_lightsMatrices;
  std::vector<float3> m_lightsColors;

  int numLights;

  enum LIGHT_DATA
  {
    COLOR = 0,
    MULT = 1,
    TYPE = 2,
    TEX_ID = 3
  };

  using lightData = std::tuple<float3, float, unsigned int, GLuint>; //{color, multiplier, type, textureId}
  std::unordered_map<int32_t, lightData> m_lights;

  GLuint m_whiteTex;

  GLuint m_lightBoundingSphere;
  GLuint m_lightBoundingSphereInstVBO;
  GLuint m_lightBoundingSphereColorVBO;
  int boundingSphereIndices;

  float4x4 lookAt;
  float4x4 projection;

  // camera parameters
  //
  float camPos[3];
  float camLookAt[3];
  float camUp[3];

  float camFov;
  float camNearPlane;
  float camFarPlane;
  int   m_width;
  int   m_height;
};


