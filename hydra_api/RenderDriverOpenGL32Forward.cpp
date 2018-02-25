//
// Created by hikawa on 04.08.17.
//
#include "RenderDriverOpenGL32Forward.h"



RD_OGL32_Forward::RD_OGL32_Forward()
{
  m_msg = L"";

  camFov       = 45.0f;
  camNearPlane = 0.1f;
  camFarPlane  = 1000.0f;

  camPos[0] = 0.0f; camPos[1] = 0.0f; camPos[2] = 0.0f;
  camLookAt[0] = 0.0f; camLookAt[1] = 0.0f; camLookAt[2] = -1.0f;

  m_width  = 1024;
  m_height = 1024;

  m_quad = std::make_unique<FullScreenQuad>();
  m_fullScreenTexture = std::make_unique<RenderTexture2D>(GL_RGBA, GL_RGBA32F, m_width, m_height);
}


void RD_OGL32_Forward::ClearAll()
{
  m_quadProgram.Release();
  m_matProgram.Release();
  m_programs.clear();
  m_objects.clear();

}

HRDriverAllocInfo RD_OGL32_Forward::AllocAll(HRDriverAllocInfo a_info)
{
  //m_objects.resize(a_info.geomNum);

  m_libPath = std::wstring(a_info.libraryPath);

  std::cout << getexepath();

  std::unordered_map<GLenum, std::string> simpleRender;
  simpleRender[GL_VERTEX_SHADER] = "../glsl/vSimple.glsl";
  simpleRender[GL_FRAGMENT_SHADER] = "../glsl/fSimple.glsl";

  m_matProgram = ShaderProgram(simpleRender);


  std::unordered_map<GLenum, std::string> quadShaders;
  quadShaders[GL_VERTEX_SHADER] = "../glsl/vQuad.glsl";
  quadShaders[GL_FRAGMENT_SHADER] = "../glsl/fQuad.glsl";

  m_quadProgram = ShaderProgram(quadShaders);

  m_diffColors.resize(a_info.matNum * 3);
  m_reflColors.resize(a_info.matNum * 3);
  m_reflGloss.resize(a_info.matNum);

  m_texturesList.resize(a_info.imgNum);
  glGenTextures(GLsizei(a_info.imgNum), &m_texturesList[0]);

  m_diffTexId.resize(a_info.matNum);
  for (size_t i = 0; i < m_diffTexId.size(); i++)
    m_diffTexId[i] = -1;

  m_reflTexId.resize(a_info.matNum);
  for (size_t i = 0; i < m_reflTexId.size(); i++)
    m_reflTexId[i] = -1;

  CreatePlaceholderWhiteTexture(m_whiteTex);

  return a_info;
}

HRDriverInfo RD_OGL32_Forward::Info()
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

bool RD_OGL32_Forward::UpdateImage(int32_t a_texId, int32_t w, int32_t h, int32_t bpp, const void *a_data,
                                 pugi::xml_node a_texNode)
{
  if (a_data == nullptr)
    return false;

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

  //TODO: texture matrices

  return true;
}


bool RD_OGL32_Forward::UpdateMaterial(int32_t a_matId, pugi::xml_node a_materialNode)
{
  pugi::xml_node diffuseColor = a_materialNode.child(L"diffuse").child(L"color");
  pugi::xml_node diffuseTex = diffuseColor.child(L"texture");

  pugi::xml_node reflectColor = a_materialNode.child(L"reflectivity").child(L"color");
  pugi::xml_node reflectTex = reflectColor.child(L"texture");
  pugi::xml_node reflectGloss = a_materialNode.child(L"reflectivity").child(L"glossiness");

  LoadFloat3FromXMLNode(diffuseColor, m_diffColors, a_matId);
  LoadFloat3FromXMLNode(reflectColor, m_reflColors, a_matId);
  LoadFloatFromXMLNode(reflectGloss, m_reflGloss, a_matId, true);

  if (diffuseTex != nullptr)
    m_diffTexId[a_matId] = diffuseTex.attribute(L"id").as_int();
  else
    m_diffTexId[a_matId] = -1;

  if (reflectTex != nullptr)
    m_reflTexId[a_matId] = reflectTex.attribute(L"id").as_int();
  else
    m_reflTexId[a_matId] = -1;

  return true;
}

bool RD_OGL32_Forward::UpdateLight(int32_t a_lightIdId, pugi::xml_node a_lightNode)
{
  return false;
}

bool RD_OGL32_Forward::UpdateMesh(int32_t a_meshId, pugi::xml_node a_meshNode, const HRMeshDriverInput &a_input,
                                const HRBatchInfo *a_batchList, int32_t a_listSize)
{

  if (a_input.triNum == 0)
  {
    return true;
  }
  //TODO: maybe try MultiDrawIndirect


  GLuint programId = m_matProgram.GetProgram();

  meshData batchMeshData;
  for (int32_t batchId = 0; batchId < a_listSize; ++batchId)
  {
    HRBatchInfo batch = a_batchList[batchId];

    GLuint vertexPosBufferObject;
    GLuint vertexNormBufferObject;
    GLuint vertexTangentsBufferObject;
    GLuint vertexTexCoordsBufferObject;

    GLuint indexBufferObject;
    GLuint vertexArrayObject;

    std::vector<float> batchPos;
    std::vector<float> batchNorm;
    std::vector<float> batchTangents;
    std::vector<float> batchTexCoord;
    std::vector<int>   batchIndices;

    CreateGeometryFromBatch(batch, a_input, batchPos, batchNorm, batchTangents, batchTexCoord, batchIndices);

    // vertex positions
    glGenBuffers(1, &vertexPosBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexPosBufferObject);
    glBufferData(GL_ARRAY_BUFFER, batchPos.size() * sizeof(GLfloat), &batchPos[0], GL_STATIC_DRAW);

    // vertex normals
    glGenBuffers(1, &vertexNormBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexNormBufferObject);
    glBufferData(GL_ARRAY_BUFFER, batchNorm.size() * sizeof(GLfloat), &batchNorm[0], GL_STATIC_DRAW);

    // vertex tangents
    glGenBuffers(1, &vertexTangentsBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexTangentsBufferObject);
    glBufferData(GL_ARRAY_BUFFER, batchTangents.size() * sizeof(GLfloat), &batchTangents[0], GL_STATIC_DRAW);

    // vertex texture coordinates
    glGenBuffers(1, &vertexTexCoordsBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexTexCoordsBufferObject);
    glBufferData(GL_ARRAY_BUFFER, batchTexCoord.size() * sizeof(GLfloat), &batchTexCoord[0], GL_STATIC_DRAW);

    // index buffer
    glGenBuffers(1, &indexBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, batchIndices.size() * sizeof(GLuint), &batchIndices[0], GL_STATIC_DRAW);


    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);

    glBindBuffer(GL_ARRAY_BUFFER, vertexPosBufferObject);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vertexNormBufferObject);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vertexTexCoordsBufferObject);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vertexTangentsBufferObject);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, 0);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);

    glBindVertexArray(0);

    std::pair<GLuint, int> tmp;
    tmp.first  = vertexArrayObject;
    tmp.second = int(batchIndices.size());

    batchMeshData[batch.matId] = tmp;
  }

  m_objects[a_meshId] = batchMeshData;

  return true;
}

bool RD_OGL32_Forward::UpdateCamera(pugi::xml_node a_camNode)
{
  if (a_camNode == nullptr)
    return true;

  const wchar_t* camPosStr = a_camNode.child(L"position").text().as_string();
  const wchar_t* camLAtStr = a_camNode.child(L"look_at").text().as_string();
  const wchar_t* camUpStr  = a_camNode.child(L"up").text().as_string();

  if (!a_camNode.child(L"fov").text().empty())
    camFov = a_camNode.child(L"fov").text().as_float();

  if (!a_camNode.child(L"nearClipPlane").text().empty())
    camNearPlane = a_camNode.child(L"nearClipPlane").text().as_float();

  if (!a_camNode.child(L"farClipPlane").text().empty())
    camFarPlane = a_camNode.child(L"farClipPlane").text().as_float();

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

bool RD_OGL32_Forward::UpdateSettings(pugi::xml_node a_settingsNode)
{
  if (a_settingsNode.child(L"width") != nullptr)
    m_width = a_settingsNode.child(L"width").text().as_int();

  if (a_settingsNode.child(L"height") != nullptr)
    m_height = a_settingsNode.child(L"height").text().as_int();

  if (m_width < 0 || m_height < 0)
  {
    m_msg = L"RD_OGL32_Forward::UpdateSettings, bad input resolution";
    return false;
  }

  m_fullScreenTexture = std::make_unique<RenderTexture2D>(GL_BGRA, GL_RGBA32F, m_width, m_height);

  return true;
}

void RD_OGL32_Forward::BeginScene(pugi::xml_node a_sceneNode)
{
  glViewport(0, 0, (GLint)m_width, (GLint)m_height);
  glClearColor(0.0f, 0.0f, 0.20f, 1.0f);
  glClearDepth(1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  //glEnable(GL_CULL_FACE);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer

  m_matProgram.StartUseShader();

  const float aspect = float(m_width) / float(m_height);
  float4x4 projMatrixInv = projectionMatrixTransposed(camFov, aspect, camNearPlane, camFarPlane);
  m_matProgram.SetUniform("projection", projMatrixInv);


  float3 eye(camPos[0], camPos[1], camPos[2]);
  float3 center(camLookAt[0], camLookAt[1], camLookAt[2]);
  float3 up(camUp[0], camUp[1], camUp[2]);
  float4x4 lookAtMatrix = lookAtTransposed(eye, center, up);
  m_matProgram.SetUniform("view", lookAtMatrix);
  m_matProgram.SetUniform("viewPos", eye);
  m_fullScreenTexture->StartRendering();

//////Temporary hard-coded point lights
  float3 lightPos(0.0f, 10.0f, -4.0f);
  float3 lightPos2(8.0f, 10.0f, 0.0f);
  float3 lightPos3(-8.0f, 10.0f, 0.0f);
  float3 lightColor(1.0f, 1.0f, 1.0f);
  float lightMult(2.0f);

  m_matProgram.SetUniform("numLights", 3);
  m_matProgram.SetUniform("pointLights[0].pos", lightPos);
  m_matProgram.SetUniform("pointLights[0].color", lightColor);
  m_matProgram.SetUniform("pointLights[0].mult", lightMult);

  m_matProgram.SetUniform("pointLights[1].pos", lightPos2);
  m_matProgram.SetUniform("pointLights[1].color", lightColor);
  m_matProgram.SetUniform("pointLights[1].mult", lightMult);

  m_matProgram.SetUniform("pointLights[2].pos", lightPos3);
  m_matProgram.SetUniform("pointLights[2].color", lightColor);
  m_matProgram.SetUniform("pointLights[2].mult", lightMult);

}

void RD_OGL32_Forward::EndScene()
{
  m_matProgram.StopUseShader();

  m_fullScreenTexture->EndRendering();

  m_quadProgram.StartUseShader();

  bindTexture(m_quadProgram, 1, "colorTexture", (*m_fullScreenTexture));
  glDisable(GL_DEPTH_TEST);

  m_quad->Draw();

  glFlush();
}

void RD_OGL32_Forward::InstanceMeshes(int32_t a_mesh_id, const float *a_matrices, int32_t a_instNum,
                                      const int *a_lightInstId, const int* a_remapId)
{
  for (int32_t i = 0; i < a_instNum; i++)
  {
    float modelM[16];
    mat4x4_transpose(modelM, (float*)(a_matrices + i*16));

    m_matProgram.SetUniform("model", float4x4(modelM));

    for(auto batch : m_objects[a_mesh_id])
    {
      int matId = batch.first;
      if (m_diffTexId[matId] >= 0)
      {
        int texId = m_diffTexId[matId];
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, m_texturesList[texId]);
      }
      else
      {
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, m_whiteTex);
      }

      if (m_diffTexId[matId] >= 0)
      {
        int texId = m_reflTexId[matId];
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, m_texturesList[texId]);
      }
      else
      {
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, m_whiteTex);
      }

      m_matProgram.SetUniform("material.diffuse", m_diffColors[matId]);
      m_matProgram.SetUniform("material.reflect", m_reflColors[matId]);
      m_matProgram.SetUniform("material.shininess", m_reflGloss[matId]);

      glBindVertexArray(batch.second.first);
      glDrawElements(GL_TRIANGLES, batch.second.second, GL_UNSIGNED_INT, nullptr);
      glBindVertexArray(0);
    }

  }
}

void RD_OGL32_Forward::InstanceLights(int32_t a_light_id, const float *a_matrix, pugi::xml_node* a_custAttrArray, int32_t a_instNum, int32_t a_lightGroupId)
{

}

void RD_OGL32_Forward::Draw()
{

}

HRRenderUpdateInfo RD_OGL32_Forward::HaveUpdateNow(int a_maxRaysPerPixel)
{
  HRRenderUpdateInfo res;
  res.finalUpdate   = true;
  res.haveUpdateFB  = true;
  res.progress      = 100.0f;
  return res;
}

void RD_OGL32_Forward::GetFrameBufferHDR(int32_t w, int32_t h, float *a_out, const wchar_t *a_layerName)
{
  glReadPixels(0, 0, w, h, GL_RGBA, GL_FLOAT, (GLvoid*)a_out);
}

void RD_OGL32_Forward::GetFrameBufferLDR(int32_t w, int32_t h, int32_t *a_out)
{
  glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)a_out);
}

IHRRenderDriver* CreateOpenGL32Forward_RenderDriver()
{
  return new RD_OGL32_Forward;
}