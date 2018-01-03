//
// Created by hikawa on 18.08.17.
//

#include <cmath>
#include "RenderDriverOpenGL32Deferred.h"



RD_OGL32_Deferred::RD_OGL32_Deferred()
{
  m_msg = L"";

  camFov       = 45.0f;
  camNearPlane = 0.05f;
  camFarPlane  = 10000.0f;

  camPos[0] = 0.0f; camPos[1] = 0.0f; camPos[2] = 0.0f;
  camLookAt[0] = 0.0f; camLookAt[1] = 0.0f; camLookAt[2] = -1.0f;

  m_width  = 1024;
  m_height = 1024;

  m_drawLightsMode = DRAW_LIGHT_GEO_MODE::LIGHT_GEO;

  m_enableSSAO = true;
  m_doSSAOPass = false;

  m_quad = std::make_unique<FullScreenQuad>();
  //m_fullScreenTexture = std::make_unique<RenderTexture2D>(GL_RGBA, GL_RGBA32F, m_width, m_height);
  m_gBuffer = std::make_unique<GBuffer>(m_width, m_height);
  m_ssaoBuffer = std::make_unique<SSAOBuffer>(m_width, m_height, 32, 0.75f);

  m_matricesUBOBindingPoint = 0;
  m_materialUBOBindingPoint = 1;
  m_lightUBOBindingPoint    = 2;
}


void RD_OGL32_Deferred::ClearAll()
{
  m_gBufferProgram.Release();
  m_lightPassProgram.Release();
  m_stencilProgram.Release();
  m_quadProgram.Release();
  //m_ssaoProgram.Release();
  //m_ssaoBlurProgram.Release();

  m_objects.clear();
  m_texturesList.clear();
  m_diffColors.clear();
  m_reflColors.clear();
  m_reflGloss.clear();
  m_diffTexId.clear();
  m_reflTexId.clear();
  m_normalTexId.clear();
  m_texMatrices.clear();
  m_lightsMatrices.clear();
  m_lightsColors.clear();
}

HRDriverAllocInfo RD_OGL32_Deferred::AllocAll(HRDriverAllocInfo a_info)
{
  //m_objects.resize(a_info.geomNum);

  m_libPath = std::wstring(a_info.libraryPath);

  std::unordered_map<GLenum, std::string> gBufferShaders;
  gBufferShaders[GL_VERTEX_SHADER] = "../glsl/vGBuffer.glsl";
  gBufferShaders[GL_FRAGMENT_SHADER] = "../glsl/fGBuffer.glsl";
  m_gBufferProgram = ShaderProgram(gBufferShaders);

  std::unordered_map<GLenum, std::string> defferedShaders;
  defferedShaders[GL_VERTEX_SHADER] = "../glsl/vDeferredShading.glsl";
  defferedShaders[GL_FRAGMENT_SHADER] = "../glsl/fDeferredShading.glsl";
  m_lightPassProgram = ShaderProgram(defferedShaders);

  std::unordered_map<GLenum, std::string> stencilShaders;
  stencilShaders[GL_VERTEX_SHADER] = "../glsl/vNothing.glsl";
  stencilShaders[GL_FRAGMENT_SHADER] = "../glsl/fNothing.glsl";
  m_stencilProgram = ShaderProgram(stencilShaders);

  std::unordered_map<GLenum, std::string> quadShaders;
  quadShaders[GL_VERTEX_SHADER] = "../glsl/vQuad.glsl";
  quadShaders[GL_FRAGMENT_SHADER] = "../glsl/fQuad.glsl";
  m_quadProgram = ShaderProgram(quadShaders);

  std::unordered_map<GLenum, std::string> lightShaders;
  lightShaders[GL_VERTEX_SHADER] = "../glsl/vLights.glsl";
  lightShaders[GL_FRAGMENT_SHADER] = "../glsl/fLights.glsl";
  m_lightsProgram = ShaderProgram(lightShaders);
/*  std::unordered_map<GLenum, std::string> ssaoShaders;
  ssaoShaders[GL_VERTEX_SHADER] = "../glsl/vSSAO.glsl";
  ssaoShaders[GL_FRAGMENT_SHADER] = "../glsl/fSSAO.glsl";
  m_ssaoProgram = ShaderProgram(ssaoShaders);

  std::unordered_map<GLenum, std::string> ssaoBlurShaders;
  ssaoBlurShaders[GL_VERTEX_SHADER] = "../glsl/vSSAOBlur.glsl";
  ssaoBlurShaders[GL_FRAGMENT_SHADER] = "../glsl/fSSAOBlur.glsl";
  m_ssaoBlurProgram = ShaderProgram(ssaoBlurShaders);
  */

  m_diffColors.resize(a_info.matNum * 3);
  m_reflColors.resize(a_info.matNum * 3);
  m_reflGloss.resize(a_info.matNum);
  m_texMatrices.resize(a_info.matNum);

  m_texturesList.resize(a_info.imgNum);
  glGenTextures(GLsizei(a_info.imgNum), &m_texturesList[0]);

  m_diffTexId.resize(a_info.matNum);
  for (size_t i = 0; i < m_diffTexId.size(); i++)
    m_diffTexId[i] = -1;

  m_reflTexId.resize(a_info.matNum);
  for (size_t i = 0; i < m_reflTexId.size(); i++)
    m_reflTexId[i] = -1;

  m_normalTexId.resize(a_info.matNum);
  for (size_t i = 0; i < m_normalTexId.size(); i++)
    m_normalTexId[i] = -1;

  glGenTextures(1, &m_whiteTex);
  CreatePlaceholderWhiteTexture(m_whiteTex);
  m_lightBoundingSphere = CreateSphere(1.0f, 16, 0, -1, -1, 4, 3, boundingSphereIndices, m_lightBoundingSphereInstVBO, m_lightBoundingSphereColorVBO);

  CreateMaterialsUBO(a_info.matNum);
  CreateMatricesUBO();
  CreateLightSettingsUBO();
  //Temporary random lights
  /*numLights = 128;
  CreateRandomLights(numLights, m_lightPos, m_lightColor);

  AssignRandomIESFiles(numLights, m_lightTextures);*/

  //CreateTextureFromFile("data/ies/ies5.png", iesTexture);

  return a_info;
}

HRDriverInfo RD_OGL32_Deferred::Info()
{
  HRDriverInfo info;

  info.supportHDRFrameBuffer        = false;
  info.supportHDRTextures           = false;
  info.supportMultiMaterialInstance = false;

  info.supportImageLoadFromInternalFormat = false;
  info.supportImageLoadFromExternalFormat = false;
  info.supportMeshLoadFromInternalFormat  = false;
  info.supportLighting                    = false;

  info.memTotal = int64_t(8) * int64_t(1024 * 1024 * 1024);

  return info;
}

void RD_OGL32_Deferred::GetLastErrorW(wchar_t *a_msg)
{
  wcscpy(a_msg, m_msg.c_str());
}

bool RD_OGL32_Deferred::UpdateImage(int32_t a_texId, int32_t w, int32_t h, int32_t bpp, const void *a_data,
                                   pugi::xml_node a_texNode)
{
  if (a_data == nullptr)
    return false;

  //CreateTextureFromData

  glBindTexture(GL_TEXTURE_2D, m_texturesList[a_texId]);
  if (bpp > 4)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, a_data);
  else
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, a_data);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0);

  //TODO: texture matrices

  return true;
}


bool RD_OGL32_Deferred::UpdateMaterial(int32_t a_matId, pugi::xml_node a_materialNode)
{
  auto diffuseColor = a_materialNode.child(L"diffuse").child(L"color");
  auto diffuseTex = diffuseColor.child(L"texture");

  auto ambient = a_materialNode.child(L"ambient");

  auto reflectColor = a_materialNode.child(L"reflectivity").child(L"color");
  auto reflectGloss = a_materialNode.child(L"reflectivity").child(L"glossiness");
  auto reflectTex = reflectColor.child(L"texture");

  auto normalTex = a_materialNode.child(L"displacement").child(L"normal_map").child(L"texture");

  LoadFloat3FromXMLNode(diffuseColor, m_diffColors, a_matId);
  LoadFloat3FromXMLNode(reflectColor, m_reflColors, a_matId);
  LoadFloatFromXMLNode(reflectGloss, m_reflGloss, a_matId, true);

  if (diffuseTex != nullptr)
  {
    m_diffTexId[a_matId] = diffuseTex.attribute(L"id").as_int();
    m_texMatrices[a_matId] = GetFloat4x4FromXMLNode(diffuseTex);
  }
  else
    m_diffTexId[a_matId] = -1;

  if (reflectTex != nullptr)
  {
    m_reflTexId[a_matId] = reflectTex.attribute(L"id").as_int();
    m_texMatrices[a_matId] = GetFloat4x4FromXMLNode(reflectTex);
  }
  else
    m_reflTexId[a_matId] = -1;

  if (normalTex != nullptr)
  {
    m_normalTexId[a_matId] = normalTex.attribute(L"id").as_int();
    m_texMatrices[a_matId] = GetFloat4x4FromXMLNode(normalTex);
  }
  else
    m_normalTexId[a_matId] = -1;

  struct mat
  {
    float3 diffuseColor;//16 0
    float useBump; // 4 16
    float3 reflectColor; //16 28 	
    float shininess; //4 4
    float4x4 texMatrix; //16*4  32 48 64 80
  };

  mat M;
  M.diffuseColor = m_diffColors.at(a_matId);
  M.useBump = (normalTex != nullptr) ? 1.0f : 0.0f;
  M.reflectColor = m_reflColors.at(a_matId);
  M.shininess = m_reflGloss.at(a_matId);
  M.texMatrix = m_texMatrices.at(a_matId);

  glBindBuffer(GL_UNIFORM_BUFFER, m_materialUBO);
  glBufferSubData(GL_UNIFORM_BUFFER, a_matId * m_materialStructSize, m_materialStructSize, &M);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);


  return true;
}

bool RD_OGL32_Deferred::UpdateLight(int32_t a_lightId, pugi::xml_node a_lightNode)
{
  lightData data(float3(0.0f, 0.0f, 0.0f), 0.0f, 1, 0);

  auto intensityNode = a_lightNode.child(L"intensity");
  //auto lightNode.attribute(L"type").set_value(L"point");
  //auto lightNode.attribute(L"distribution").set_value(L"ies");
  auto distribution = std::wstring(a_lightNode.attribute(L"distribution").as_string());
  auto type = std::wstring(a_lightNode.attribute(L"type").as_string());

  std::get<LIGHT_DATA::COLOR>(data) = GetFloat3FromXMLNode(intensityNode.child(L"color"));
  std::get<LIGHT_DATA::MULT>(data) = intensityNode.child(L"multiplier").attribute(L"val").as_float();


  if(distribution.compare(L"ies") == 0)
  {
    auto path = std::wstring(a_lightNode.child(L"ies").attribute(L"data").as_string());
    std::string path_s(path.begin(), path.end());

    GLuint texId;
    glGenTextures(1, &texId);

    int res = CreateGLSphereMapFromIES(path_s, texId);

    if(res != 0) //failed to load IES file
      return false;

    std::get<LIGHT_DATA::TYPE>(data) = 2;
    std::get<LIGHT_DATA::TEX_ID>(data) = texId;
  }
  m_lights[a_lightId] = data;

  return true;
}

bool RD_OGL32_Deferred::UpdateMesh(int32_t a_meshId, pugi::xml_node a_meshNode, const HRMeshDriverInput &a_input,
                                  const HRBatchInfo *a_batchList, int32_t a_listSize)
{
  if (a_input.triNum == 0)
  {
    return true;
  }
  //TODO: maybe try MultiDrawIndirect

  meshData batchMeshData;
  for (int32_t batchId = 0; batchId < a_listSize; ++batchId)
  {
    HRBatchInfo batch = a_batchList[batchId];

    GLuint vertexPosBufferObject;
    GLuint vertexNormBufferObject;
    GLuint vertexTangentBufferObject;
    GLuint vertexTexCoordsBufferObject;
 //   GLuint matIDBufferObject;

    GLuint indexBufferObject;
    GLuint vertexArrayObject;

    std::vector<float> batchPos;
    std::vector<float> batchNorm; 
    std::vector<float> batchTangent;
    std::vector<float> batchTexCoord;
    std::vector<int>   batchIndices;

    CreateGeometryFromBatch(batch, a_input, batchPos, batchNorm, batchTangent, batchTexCoord, batchIndices);

 //   std::vector<int>   matIDs(batchPos.size() / 4, batch.matId);

    // vertex positions
    glGenBuffers(1, &vertexPosBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexPosBufferObject);
    glBufferData(GL_ARRAY_BUFFER, batchPos.size() * sizeof(GLfloat), &batchPos[0], GL_STATIC_DRAW);

    // vertex normals
    glGenBuffers(1, &vertexNormBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexNormBufferObject);
    glBufferData(GL_ARRAY_BUFFER, batchNorm.size() * sizeof(GLfloat), &batchNorm[0], GL_STATIC_DRAW);

    // vertex tangents
    glGenBuffers(1, &vertexTangentBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexTangentBufferObject);
    glBufferData(GL_ARRAY_BUFFER, batchTangent.size() * sizeof(GLfloat), &batchTangent[0], GL_STATIC_DRAW);

    // vertex texture coordinates
    glGenBuffers(1, &vertexTexCoordsBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexTexCoordsBufferObject);
    glBufferData(GL_ARRAY_BUFFER, batchTexCoord.size() * sizeof(GLfloat), &batchTexCoord[0], GL_STATIC_DRAW);

    //matID
/*    glGenBuffers(1, &matIDBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, matIDBufferObject);
    glBufferData(GL_ARRAY_BUFFER, matIDs.size() * sizeof(GLint), &matIDs[0], GL_STATIC_DRAW);
*/
    // index buffer
    glGenBuffers(1, &indexBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, batchIndices.size() * sizeof(GLuint), &batchIndices[0], GL_STATIC_DRAW);


    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);

    glBindBuffer(GL_ARRAY_BUFFER, vertexPosBufferObject);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, vertexNormBufferObject);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, vertexTexCoordsBufferObject);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, vertexTangentBufferObject);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

 /*   glBindBuffer(GL_ARRAY_BUFFER, matIDBufferObject);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_INT, GL_FALSE, 0, nullptr);

    */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);

    glBindVertexArray(0);

    std::pair<GLuint, int> tmp;
    tmp.first = vertexArrayObject;
    tmp.second = int(batchIndices.size());

    batchMeshData[batch.matId] = tmp;
  }

  m_objects[a_meshId] = batchMeshData;

  return true;
}


bool RD_OGL32_Deferred::UpdateCamera(pugi::xml_node a_camNode)
{
  if (a_camNode == nullptr)
    return true;

  const wchar_t* camPosStr = a_camNode.child(L"position").text().as_string();
  const wchar_t* camLAtStr = a_camNode.child(L"look_at").text().as_string();
  const wchar_t* camUpStr  = a_camNode.child(L"up").text().as_string();

  if (!a_camNode.child(L"fov").text().empty())
    camFov = a_camNode.child(L"fov").text().as_float();

  if (!a_camNode.child(L"nearClipPlane").text().empty())
    camNearPlane = 0.1f;// a_camNode.child(L"nearClipPlane").text().as_float();

  if (!a_camNode.child(L"farClipPlane").text().empty())
    camFarPlane = 1000000.0f;//a_camNode.child(L"farClipPlane").text().as_float();

  if (std::wstring(camPosStr) != L"")
  {
    std::wstringstream input(camPosStr);
    input >> camPos[0] >> camPos[1] >> camPos[2];
  }

  if (std::wstring(camLAtStr) != L"")
  {
    std::wstringstream input(camLAtStr);
    input >> camLookAt[0] >> camLookAt[1] >> camLookAt[2];
  }

  if (std::wstring(camUpStr) != L"")
  {
    std::wstringstream input(camUpStr);
    input >> camUp[0] >> camUp[1] >> camUp[2];
  }

  return true;
}

bool RD_OGL32_Deferred::UpdateSettings(pugi::xml_node a_settingsNode)
{
  int new_w, new_h;
  if (a_settingsNode.child(L"width") != nullptr)
    new_w = a_settingsNode.child(L"width").text().as_int();

  if (a_settingsNode.child(L"height") != nullptr)
    new_h = a_settingsNode.child(L"height").text().as_int();

  if (new_w < 0 || new_h < 0)
  {
    m_msg = L"RD_OGL32_Deferred::UpdateSettings, bad input resolution";
    return false;
  }

  if(new_w != m_width || new_h != m_height)
  {
    m_width = new_w;
    m_height = new_h;
    m_gBuffer->ResizeAttachments(m_width, m_height);
    m_ssaoBuffer->ResizeAttachments(m_width, m_height);
    //m_fullScreenTexture->ResizeAttachments(m_width, m_height);
  }

  if (a_settingsNode.child(L"SSAO") != nullptr)
    m_enableSSAO = a_settingsNode.child(L"SSAO").text().as_bool();

  if (a_settingsNode.child(L"lightgeo_mode") != nullptr)
    m_drawLightsMode = static_cast<DRAW_LIGHT_GEO_MODE>(a_settingsNode.child(L"lightgeo_mode").text().as_int());

  return true;
}

void RD_OGL32_Deferred::BeginScene()
{
 // std::cout << "BeginScene" <<std::endl;
  glViewport(0, 0, (GLint)m_width, (GLint)m_height);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  m_gBuffer->FrameInit();
  m_gBufferProgram.StartUseShader();
  m_gBuffer->StartGeoPass();

  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_DEPTH_CLAMP);
  glClear(GL_DEPTH_BUFFER_BIT);

  m_doSSAOPass = true;

  const float aspect = float(m_width) / float(m_height);
  projection = projectionMatrixTransposed(camFov, aspect, camNearPlane, camFarPlane);
  //m_gBufferProgram.SetUniform("projection", projection);

  float3 eye(camPos[0], camPos[1], camPos[2]);
  float3 center(camLookAt[0], camLookAt[1], camLookAt[2]);
  float3 up(camUp[0], camUp[1], camUp[2]);
  lookAt = lookAtTransposed(eye, center, up);
  //m_gBufferProgram.SetUniform("view", lookAt);

  std::vector<float4x4> matrices{lookAt , projection };

  glBindBuffer(GL_UNIFORM_BUFFER, m_matricesUBO);
  glBufferData(GL_UNIFORM_BUFFER, 32 * sizeof(GL_FLOAT), &matrices[0], GL_STATIC_DRAW);
  //glBufferSubData(GL_UNIFORM_BUFFER, 0, 32 * sizeof(GL_FLOAT), &matrices[0]);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

float RD_OGL32_Deferred::DrawOneLight(int i, float4x4 &&sphModel)
{
  float3 lightColor = std::get<LIGHT_DATA::COLOR>(m_lights[i]);
  float lightMult = std::get<LIGHT_DATA::MULT>(m_lights[i]);
  unsigned int lightType = std::get<LIGHT_DATA::TYPE>(m_lights[i]);
  GLuint lightTexId = std::get<LIGHT_DATA::TEX_ID>(m_lights[i]);

  float3 lightNormal = float3(0.0f, -1.0f, 0.0f);
  float3 lightRight  = float3(1.0f, 0.0f, 0.0f);

  lightNormal = normalize(mul3x3(mul(lookAt, sphModel), lightNormal));
  lightRight  = normalize(mul3x3(mul(lookAt, sphModel), lightRight));

  float sphRadius = CalcLightBoundingSphereRadius(lightColor, lightMult);

  float4x4 sphScale;
  sphScale.identity();

  sphScale = scale4x4(float3(sphRadius, sphRadius, sphRadius));
  sphModel = mul(sphModel, sphScale);

  float4x4 sphT = transpose4x4(sphModel);

  float3 sphPos = float3(sphModel.row[0].w, sphModel.row[1].w, sphModel.row[2].w);


  m_stencilProgram.StartUseShader();

  // Stencil prepass
  glDrawBuffer(GL_NONE);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glClear(GL_STENCIL_BUFFER_BIT);

  glStencilFunc(GL_ALWAYS, 0, 0);
  glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
  glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

 // m_stencilProgram.SetUniform("view", lookAt);
 // m_stencilProgram.SetUniform("projection", projection);
  m_stencilProgram.SetUniform("model", sphT);

  glBindVertexArray(m_lightBoundingSphere);
  glDrawElements(GL_TRIANGLES, boundingSphereIndices, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);

 // m_stencilProgram.StopUseShader();

  // Light pass
  m_gBuffer->StartLightPass();

  m_lightPassProgram.StartUseShader();

//  m_lightPassProgram.SetUniform("view", lookAt);
//  m_lightPassProgram.SetUniform("projection", projection);
  //m_lightPassProgram.SetUniform("viewPos", lookAt);
  m_lightPassProgram.SetUniform("screenSize", int2(m_width, m_height));

  bindTexture(m_lightPassProgram, GBuffer::GBUF_POSITION, "gVertex", m_gBuffer->GetTextureId(GBuffer::GBUF_POSITION));
  bindTexture(m_lightPassProgram, GBuffer::GBUF_NORMAL, "gNormal", m_gBuffer->GetTextureId(GBuffer::GBUF_NORMAL));
  bindTexture(m_lightPassProgram, GBuffer::GBUF_DIFFUSE, "gDiffuse", m_gBuffer->GetTextureId(GBuffer::GBUF_DIFFUSE));
  bindTexture(m_lightPassProgram, GBuffer::GBUF_REFLECTION, "gReflect", m_gBuffer->GetTextureId(GBuffer::GBUF_REFLECTION));
  //bindTexture(m_lightPassProgram, GBuffer::GBUF_NUM_TEXTURES, "ssao", m_ssaoBuffer->GetTextureId(SSAOBuffer::SSAO_BLUR));

 /* m_lightPassProgram.SetUniform("light.pos", sphPos);
  m_lightPassProgram.SetUniform("light.color", lightColor);
  m_lightPassProgram.SetUniform("light.mult", lightMult);
  m_lightPassProgram.SetUniform("light.type", lightType);*/

  m_lightPassProgram.SetUniform("model", sphT);

/*  m_lightPassProgram.SetUniform("light.normal", lightNormal);
  m_lightPassProgram.SetUniform("light.right", lightRight);*/

  struct lightSettings
  {
    float3 pos;    //16 0
    float mult;  //4 12
    float3 color; //16 16
                // 1 - point, 2 - ies, ...
    int type; //4 28
    float3 normal; //16 32
    float3 right; //16 48
  };

  lightSettings ls;
  ls.pos = sphPos;
  ls.mult = lightMult;
  ls.color = lightColor;
  ls.type = lightType;
  ls.normal = lightNormal;
  ls.right = lightRight;

  glBindBuffer(GL_UNIFORM_BUFFER, m_lightUBO);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, m_lightStructSize, &ls);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  bindTexture(m_lightPassProgram, GBuffer::GBUF_NUM_TEXTURES, "intensityTex", lightTexId);

  glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_ONE, GL_ONE);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_FRONT);
  
  glBindVertexArray(m_lightBoundingSphere);
    glDrawElements(GL_TRIANGLES, boundingSphereIndices, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);

  //glCullFace(GL_BACK);
  glDisable(GL_BLEND);
  glDisable(GL_CULL_FACE);

  //m_lightPassProgram.StopUseShader();

  return sphRadius;
}

void RD_OGL32_Deferred::SSAOPass() const
{
 // std::cout << "SSAOPass" << std::endl;
  m_ssaoBuffer->StartSSAOPass(projection, m_gBuffer->GetDepthTex(), m_gBuffer->GetTextureId(GBuffer::GBUF_NORMAL));
  m_quad->Draw();
  m_ssaoBuffer->EndSSAOPass();

 // glBindFramebuffer(GL_FRAMEBUFFER, 0);
  m_ssaoBuffer->StartSSAOBlurPass();
  m_quad->Draw();
  m_ssaoBuffer->EndSSAOBlurPass();
}


void RD_OGL32_Deferred::EndScene()
{
 
 // std::cout << "EndScene" <<std::endl;
  //SSAOPass();
  //glBindFramebuffer(GL_FRAMEBUFFER, 0);
  if (m_drawLightsMode != DRAW_LIGHT_GEO_MODE::NOTHING)
  {
     glEnable(GL_DEPTH_TEST);

     m_lightsProgram.StartUseShader();
   //  m_lightsProgram.SetUniform("view", lookAt);
   //  m_lightsProgram.SetUniform("projection", projection);

     DrawLightGeoInstances(m_lightBoundingSphere, m_lightBoundingSphereInstVBO, m_lightBoundingSphereColorVBO, boundingSphereIndices);
     m_lightsMatrices.clear();
     m_lightsColors.clear();
     glDisable(GL_DEPTH_TEST);    
  }

  /* Simple tone mapping */
  m_quadProgram.StartUseShader();
  m_gBuffer->StartFinalPass(0);
  m_quadProgram.SetUniform("exposure", 2.0f);
  //bindTexture(m_quadProgram, 0, "colorTexture", m_ssaoBuffer->GetTextureId(SSAOBuffer::SSAO_BLUR));
  bindTexture(m_quadProgram, 0, "colorTexture", m_gBuffer->GetTextureId(GBuffer::GBUF_NUM_TEXTURES));

  if(m_enableSSAO)
    bindTexture(m_quadProgram, 1, "ssaoTexture", m_ssaoBuffer->GetTextureId(SSAOBuffer::SSAO_BLUR));
  else
    bindTexture(m_quadProgram, 1, "ssaoTexture", m_whiteTex);
 // bindTexture(m_quadProgram, 2, "gDiffuse", m_gBuffer->GetTextureId(GBuffer::GBUF_DIFFUSE));
  m_quad->Draw();
  //m_quadProgram.StopUseShader();
  
  /* Direct copy of render texture to the default framebuffer */
  /*
  m_gBuffer->StartFinalPass(0);
  glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
  */

  glFlush();
}


void RD_OGL32_Deferred::InstanceMeshes(int32_t a_mesh_id, const float *a_matrices, int32_t a_instNum,
                                      const int *a_lightInstId)
{
 // std::cout << "InstanceMeshes" <<std::endl;
  for (int32_t i = 0; i < a_instNum; i++)
  {
    float modelM[16];
    mat4x4_transpose(modelM, (float*)(a_matrices + i*16));

    m_gBufferProgram.SetUniform("model", float4x4(modelM));

    for(auto batch : m_objects[a_mesh_id])
    {
      int matId = batch.first;
      if (m_diffTexId[matId] >= 0)
      {
        int texId = m_diffTexId[matId];
        bindTexture(m_gBufferProgram, 0, "diffuseTex", m_texturesList[texId]);
      }
      else
      {
        bindTexture(m_gBufferProgram, 0, "diffuseTex", m_whiteTex);
      }

      if (m_reflTexId[matId] >= 0)
      {
        int texId = m_reflTexId[matId];
        bindTexture(m_gBufferProgram, 1, "reflectTex", m_texturesList[texId]);
      }
      else
      {
        bindTexture(m_gBufferProgram, 1, "reflectTex", m_whiteTex);
      }

      if (m_normalTexId[matId] >= 0)
      {
        int texId = m_normalTexId[matId];
        bindTexture(m_gBufferProgram, 2, "normalTex", m_texturesList[texId]);
      }

      m_gBufferProgram.SetUniform("matID", matId); 
      m_gBufferProgram.SetUniform("invertNormals", true); //TODO: find a way to check meshes?

      glBindVertexArray(batch.second.first);
      glDrawElements(GL_TRIANGLES, batch.second.second, GL_UNSIGNED_INT, nullptr);
      glBindVertexArray(0);
    }

  }
}


void RD_OGL32_Deferred::InstanceLights(int32_t a_light_id, const float *a_matrix, pugi::xml_node* a_custAttrArray, int32_t a_instNum, int32_t a_lightGroupId)
{
  //m_gBufferProgram.StopUseShader();
 
  if(m_enableSSAO && m_doSSAOPass)
  {
    m_doSSAOPass = false;
    SSAOPass();
  }
  
  m_gBuffer->StartGeoPass(); //reset states 

  float lightVolRadius = 0.0f;
 // std::cout << "InstanceLights" <<std::endl;
  glDepthMask(GL_FALSE);
  glEnable(GL_STENCIL_TEST);
  for (int32_t i = 0; i < a_instNum; i++)
  {
    //float modelM[16];
    //mat4x4_transpose(modelM, (float *) (a_matrix + i * 16));

    lightVolRadius = DrawOneLight(a_light_id, float4x4((float *) (a_matrix + i * 16)));

    if (m_drawLightsMode != DRAW_LIGHT_GEO_MODE::NOTHING)
    {
      float3 color = std::get<LIGHT_DATA::COLOR>(m_lights[a_light_id]);

      float4x4 model((float *)(a_matrix + i * 16));
      float4x4 sphScale;

      if (m_drawLightsMode == DRAW_LIGHT_GEO_MODE::LIGHT_GEO)
        sphScale = scale4x4(float3(0.05f, 0.05f, 0.05f));
      else if (m_drawLightsMode == DRAW_LIGHT_GEO_MODE::LIGHT_VOLUME)
        sphScale = scale4x4(float3(lightVolRadius, lightVolRadius, lightVolRadius));
      model = mul(model, sphScale);
      m_lightsMatrices.push_back(transpose4x4(model));
      m_lightsColors.push_back(color);
    }
  }
  glDisable(GL_STENCIL_TEST);
}

void RD_OGL32_Deferred::DrawLightGeoInstances(GLuint &vao, GLuint &vboMat, GLuint &vboColor, int numIndices)
{ 
  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vboColor);
  glBufferData(GL_ARRAY_BUFFER, m_lightsColors.size() * 3 * sizeof(float), &m_lightsColors[0], GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);

  glVertexAttribDivisor(3, 1);


  glBindBuffer(GL_ARRAY_BUFFER, vboMat);
  glBufferData(GL_ARRAY_BUFFER, m_lightsMatrices.size() * 16 * sizeof(float), &m_lightsMatrices[0], GL_DYNAMIC_DRAW);

  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (GLvoid*)0);
  glEnableVertexAttribArray(5);
  glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (GLvoid*)(4 * sizeof(float)));
  glEnableVertexAttribArray(6);
  glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (GLvoid*)(8 * sizeof(float)));
  glEnableVertexAttribArray(7);
  glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (GLvoid*)(12 * sizeof(float)));

  glVertexAttribDivisor(4, 1);
  glVertexAttribDivisor(5, 1);
  glVertexAttribDivisor(6, 1);
  glVertexAttribDivisor(7, 1);

  glDrawElementsInstanced(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0, m_lightsMatrices.size());

  glBindVertexArray(0);
 
}

float RD_OGL32_Deferred::CalcLightBoundingSphereRadius(float3 a_color, float a_intensity)
{
  float maxColor = std::fmax(std::fmax(a_color.x, a_color.y), a_color.z);

  float ret = std::sqrt((4.0f * (256.0f / 2.0f) * maxColor * a_intensity)) / 2.0f;

  return ret;
}

void RD_OGL32_Deferred::CreateMaterialsUBO(int numMat)
{
  GLuint uboIndex = glGetUniformBlockIndex(m_gBufferProgram.GetProgram(), "materialBuffer");

  //GLsizei uboSize = numMat * (sizeof(bool) + sizeof(float) + sizeof(float3) * 2 + sizeof(float4x4));
  m_materialStructSize = 24 * sizeof(GLfloat);

  glUniformBlockBinding(m_gBufferProgram.GetProgram(), uboIndex, m_materialUBOBindingPoint);

  glGenBuffers(1, &m_materialUBO);

  glBindBuffer(GL_UNIFORM_BUFFER, m_materialUBO);
  glBufferData(GL_UNIFORM_BUFFER, m_materialStructSize * numMat, NULL, GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  glBindBufferRange(GL_UNIFORM_BUFFER, m_materialUBOBindingPoint, m_materialUBO, 0, m_materialStructSize * numMat);
}

void RD_OGL32_Deferred::CreateLightSettingsUBO()
{
  GLuint uboIndex = glGetUniformBlockIndex(m_lightPassProgram.GetProgram(), "lightBuffer");

  m_lightStructSize = 16 * sizeof(GLfloat);

  glUniformBlockBinding(m_lightPassProgram.GetProgram(), uboIndex, m_lightUBOBindingPoint);

  glGenBuffers(1, &m_lightUBO);

  glBindBuffer(GL_UNIFORM_BUFFER, m_lightUBO);
  glBufferData(GL_UNIFORM_BUFFER, m_lightStructSize, NULL, GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  glBindBufferRange(GL_UNIFORM_BUFFER, m_lightUBOBindingPoint, m_lightUBO, 0, m_lightStructSize);

}

void RD_OGL32_Deferred::CreateMatricesUBO()
{
  GLuint uboIndexGBuffer   = glGetUniformBlockIndex(m_gBufferProgram.GetProgram(), "matrixBuffer");
  GLuint uboIndexLights    = glGetUniformBlockIndex(m_lightsProgram.GetProgram(), "matrixBuffer");
  GLuint uboIndexLightPass = glGetUniformBlockIndex(m_lightPassProgram.GetProgram(), "matrixBuffer");
  GLuint uboIndexSSAO      = glGetUniformBlockIndex(m_ssaoBuffer->GetSSAOProgramID(), "matrixBuffer");
  GLuint uboIndexStencil   = glGetUniformBlockIndex(m_stencilProgram.GetProgram(), "matrixBuffer");

  glUniformBlockBinding(m_gBufferProgram.GetProgram(),    uboIndexGBuffer,   m_matricesUBOBindingPoint);
  glUniformBlockBinding(m_lightsProgram.GetProgram(),     uboIndexLights,    m_matricesUBOBindingPoint);
  glUniformBlockBinding(m_lightPassProgram.GetProgram(),  uboIndexLightPass, m_matricesUBOBindingPoint);
  glUniformBlockBinding(m_ssaoBuffer->GetSSAOProgramID(), uboIndexSSAO,      m_matricesUBOBindingPoint);
  glUniformBlockBinding(m_stencilProgram.GetProgram(),    uboIndexStencil,   m_matricesUBOBindingPoint);


  GLsizei matricesUBOSize = 32 * sizeof(GLfloat);

  glGenBuffers(1, &m_matricesUBO);

  glBindBuffer(GL_UNIFORM_BUFFER, m_matricesUBO);
  glBufferData(GL_UNIFORM_BUFFER, matricesUBOSize, NULL, GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  glBindBufferRange(GL_UNIFORM_BUFFER, m_matricesUBOBindingPoint, m_matricesUBO, 0, matricesUBOSize);

}

void RD_OGL32_Deferred::Draw()
{
  //std::cout << "Draw" << std::endl;
}

HRRenderUpdateInfo RD_OGL32_Deferred::HaveUpdateNow(int a_maxRaysPerPixel)
{
  HRRenderUpdateInfo res;
  res.finalUpdate   = true;
  res.haveUpdateFB  = true;
  res.progress      = 100.0f;
  return res;
}

void RD_OGL32_Deferred::GetFrameBufferHDR(int32_t w, int32_t h, float *a_out, const wchar_t *a_layerName)
{
  glReadPixels(0, 0, w, h, GL_RGBA, GL_FLOAT, (GLvoid*)a_out);
}

void RD_OGL32_Deferred::GetFrameBufferLDR(int32_t w, int32_t h, int32_t *a_out)
{
  glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)a_out);
}


IHRRenderDriver* CreateOpenGL32Deferred_RenderDriver()
{
  return new RD_OGL32_Deferred;
}
