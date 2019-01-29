//
// Created by vsan on 23.03.18.
//

#include <cmath>
#include "RenderDriverOpenGL3_Utility.h"
#include "HydraObjectManager.h"
#include "HydraXMLHelpers.h"


RD_OGL32_Utility::RD_OGL32_Utility()
{
  camFov       = 45.0f;
  camNearPlane = 0.05f;
  camFarPlane  = 10000.0f;

  camPos[0] = 0.0f; camPos[1] = 0.0f; camPos[2] = 0.0f;
  camLookAt[0] = 0.0f; camLookAt[1] = 0.0f; camLookAt[2] = -1.0f;

  m_width  = 1024;
  m_height = 1024;

  m_texNum = 0;

  m_quad = std::make_unique<FullScreenQuad>();
  //m_fullScreenTexture = std::make_unique<RenderTexture2D>(GL_RGBA, GL_RGBA32F, m_width, m_height);
  m_lodBuffer = std::make_unique<LODBuffer>(m_width, m_height);

  m_matricesUBOBindingPoint = 0;
}


void RD_OGL32_Utility::ClearAll()
{
  m_lodBufferProgram.Release();
  m_quadProgram.Release();

  for(auto& vbo : m_allVBOs)
  {
    glDeleteBuffers(1, &vbo);
  }

  m_allVBOs.clear();

  for(auto& obj : m_objects)
  {
    glDeleteVertexArrays(1, &obj.second.first);
  }

  m_objects.clear();

  m_materials_pt1.clear();
  m_materials_pt2.clear();
  m_materials_matrix.clear();
  m_remapLists.clear();

}

// C++11 raw string literals

const std::string lod_vs = R"END(
#version 330 core
layout (location = 0) in vec4 vertex;
layout (location = 1) in vec4 normal;
layout (location = 2) in vec2 texcoords;

out VS_OUT
{
	vec2 TexCoords;
	vec3 FragPos;
	vec3 Normal;
} vs_out;

layout (std140) uniform matrixBuffer
{
	mat4 view;
	mat4 projection;
};

uniform mat4 model;
uniform bool invertNormals;	

void main()
{
	vs_out.TexCoords  = texcoords;
	vec4 viewPos      = view * model * vertex;
	vs_out.FragPos    = viewPos.xyz;
	mat3 normalMatrix = mat3(transpose(inverse(view * model)));

	vs_out.Normal = normalMatrix * normal.xyz;
	vs_out.Normal = invertNormals ? -1.0f * vs_out.Normal : vs_out.Normal;
	gl_Position   = projection * viewPos;
}
)END";

const std::string lod_fs = R"END(
#version 330 core
layout (location = 0) out uvec4 lodbuf_1;
layout (location = 1) out uvec4 lodbuf_2;

in VS_OUT
{
  vec2 TexCoords;
  vec3 FragPos;
  vec3 Normal;
} fs_in;

uniform usamplerBuffer materials1;
uniform usamplerBuffer materials2;
uniform samplerBuffer materials_matrix;

uniform int matID;
uniform ivec2 max_tex_res;
uniform ivec2 render_res;
uniform ivec2 rasterization_res;

#define MAX_MIP_LEVEL 4

float mip_map_level(vec2 texture_coordinate)
{
  vec2  dx_vtc        = (rasterization_res.x / float(render_res.x)) * max_tex_res.x * dFdx(texture_coordinate);
  vec2  dy_vtc        = (rasterization_res.y / float(render_res.y)) * max_tex_res.y * dFdy(texture_coordinate);

  float dx2           = dot(dx_vtc, dx_vtc);
  float dy2           = dot(dy_vtc, dy_vtc);  
  float delta_max_sqr = max(dx2, dy2);

  const float maxClamp = pow(2.0f, MAX_MIP_LEVEL * 2.0f);
  delta_max_sqr = clamp(delta_max_sqr, 1.0f, maxClamp);

  return 0.5f * log2(delta_max_sqr);
}

const uint texIdBits    = 0x00FFFFFFu;
const uint mipLevelBits = 0xFF000000u;

mat2 getTexMatrix(int matId, int slotId)
{
  vec4 tex_mat = texelFetch(materials_matrix, matId * 8 + slotId);
  mat2 res;
  res[0][0] = tex_mat.x;
  res[0][1] = tex_mat.y;
  res[1][0] = tex_mat.z;
  res[1][1] = tex_mat.w;

  return res;
}

void main()
{     
  uvec4 texIds1 = uvec4(0, 0, 0, 0);
  uvec4 texIds2 = uvec4(0, 0, 0, 0);
  uint mipLevel = 0u;

  if(matID >= 0)
  {
    texIds1 = texelFetch(materials1, matID);
    texIds2 = texelFetch(materials2, matID);

    mipLevel  = uint(floor(mip_map_level(fs_in.TexCoords * getTexMatrix(matID, 0))));
    texIds1.r = (texIds1.r & texIdBits) | (mipLevel << 24);

    mipLevel  = uint(floor(mip_map_level(fs_in.TexCoords * getTexMatrix(matID, 1))));
    texIds1.g = (texIds1.g & texIdBits) | (mipLevel << 24);

    mipLevel  = uint(floor(mip_map_level(fs_in.TexCoords * getTexMatrix(matID, 2))));
    texIds1.b = (texIds1.b & texIdBits) | (mipLevel << 24);

    mipLevel  = uint(floor(mip_map_level(fs_in.TexCoords * getTexMatrix(matID, 3))));
    texIds1.a = (texIds1.a & texIdBits) | (mipLevel << 24);

    mipLevel  = uint(floor(mip_map_level(fs_in.TexCoords * getTexMatrix(matID, 4))));
    texIds2.r = (texIds2.r & texIdBits) | (mipLevel << 24);

    mipLevel  = uint(floor(mip_map_level(fs_in.TexCoords * getTexMatrix(matID, 5))));
    texIds2.g = (texIds2.g & texIdBits) | (mipLevel << 24);

    mipLevel  = uint(floor(mip_map_level(fs_in.TexCoords * getTexMatrix(matID, 6))));
    texIds2.b = (texIds2.b & texIdBits) | (mipLevel << 24);

    mipLevel  = uint(floor(mip_map_level(fs_in.TexCoords * getTexMatrix(matID, 7))));
    texIds2.a = (texIds2.a & texIdBits) | (mipLevel << 24);
  }

  lodbuf_1 = texIds1;
  lodbuf_2 = texIds2;
}
)END";

const std::string debug_vs = R"END(
#version 330 core
layout (location = 0) in vec2 vertex;

out vec2 fragmentTexCoord;
out vec2 vFragPosition;

void main(void)
{
  fragmentTexCoord = vertex * 0.5 + 0.5; 
  vFragPosition    = vertex;
  gl_Position      = vec4(vertex, 0.0, 1.0);
}

)END";

const std::string debug_fs = R"END(
#version 330

in vec2 fragmentTexCoord;
out vec4 fragColor;

uniform usampler2D debugTex;

vec3 colorMap(float f)
{
  const float dx = 0.8;
  f              = clamp(f, 0.0, 1.0f);
  float g        = (6.0f - 2.0f * dx) * f + dx;
  float red      = max(0.0f, (3.0f - abs(g - 4.0f) - abs(g - 5.0f))/2.0f);
  float green    = max(0.0f, (4.0f - abs(g - 2.0f) - abs(g - 4.0f))/2.0f);
  float blue     = max(0.0f, (3.0f - abs(g - 1.0f) - abs(g - 2.0f))/2.0f);
  return vec3(red, green, blue);
}

const uint texIdBits    = 0x00FFFFFFu;
const uint mipLevelBits = 0xFF000000u;

void main(void)
{
  uvec4 val            = texture(debugTex, fragmentTexCoord);
  uint diffuseMipLevel = val.g >> 24;
  uint diffuseTexId    = val.g & texIdBits;

  //fragColor = vec4(colorMap(color.x/10.0f), 1.0f);
  fragColor = vec4(colorMap(diffuseMipLevel/10.0f), 1.0f); //color.rgb
}
)END";

HRDriverAllocInfo RD_OGL32_Utility::AllocAll(HRDriverAllocInfo a_info)
{
  //m_objects.resize(a_info.geomNum);

  m_libPath = std::wstring(a_info.libraryPath);

  auto resources_pathW = std::wstring(a_info.resourcesPath);
  auto resources_path = std::string(resources_pathW.begin(), resources_pathW.end());

  std::unordered_map<GLenum, std::string> lodBufferShaders;
  lodBufferShaders[GL_VERTEX_SHADER]   = lod_vs; // resources_path + "/glsl/LODBuffer.vert"; //D:/!repos_new/!hydra/HydraAPI
  lodBufferShaders[GL_FRAGMENT_SHADER] = lod_fs; // resources_path + "/glsl/LODBuffer.frag";
  m_lodBufferProgram = ShaderProgram(lodBufferShaders, true);

  std::unordered_map<GLenum, std::string> quadShaders;
  quadShaders[GL_VERTEX_SHADER]   = debug_vs; // resources_path  + "/glsl/vQuad.vert";
  quadShaders[GL_FRAGMENT_SHADER] = debug_fs; // resources_path + "/glsl/fQuadDebug.frag";
  m_quadProgram = ShaderProgram(quadShaders, true);

  m_texNum = (unsigned int)a_info.imgNum;

  m_materials_pt1.resize((unsigned long)(a_info.matNum), int4(-1, -1, -1, -1));
  m_materials_pt2.resize((unsigned long)(a_info.matNum), int4(-1, -1, -1, -1));
  m_materials_matrix.resize((unsigned long)(a_info.matNum * 8), float4(1.0f, 0.0f, 0.0f, 1.0f));


  glGenTextures(1, &m_whiteTex);
  CreatePlaceholderWhiteTexture(m_whiteTex);

  CreateMaterialsTBO();
  CreateMatricesUBO();

  return a_info;
}

HRDriverInfo RD_OGL32_Utility::Info()
{
  HRDriverInfo info;

  info.supportHDRFrameBuffer        = false;
  info.supportHDRTextures           = true;
  info.supportMultiMaterialInstance = false;

  info.supportImageLoadFromInternalFormat = false;
  info.supportImageLoadFromExternalFormat = false;
  info.supportMeshLoadFromInternalFormat  = false;
  info.supportLighting                    = false;

  info.memTotal = int64_t(8) * int64_t(1024 * 1024 * 1024); //TODO: ?

  return info;
}

bool RD_OGL32_Utility::UpdateImage(int32_t a_texId, int32_t w, int32_t h, int32_t bpp, const void *a_data,
                                    pugi::xml_node a_texNode)
{
  return true;
}


bool RD_OGL32_Utility::UpdateMaterial(int32_t a_matId, pugi::xml_node a_materialNode)
{
  auto mat_id = (unsigned int)(a_matId);

  int32_t emissionTexId = -1;
  int32_t diffuseTexId = -1;
  int32_t reflectTexId = -1;
  int32_t reflectGlossTexId = -1;

  int32_t transparencyTexId = -1;
  int32_t opacityTexId = -1;
  int32_t translucencyTexId = -1;
  int32_t bumpTexId = -1;

  float emissionTexMat[4] = {1.0f, 0.0f, 0.0f, 1.0f};
  float diffuseTexMat[4] = {1.0f, 0.0f, 0.0f, 1.0f};
  float reflectTexMat[4] = {1.0f, 0.0f, 0.0f, 1.0f};
  float reflectGlossTexMat[4] = {1.0f, 0.0f, 0.0f, 1.0f};

  float transparencyTexMat[4] = {1.0f, 0.0f, 0.0f, 1.0f};
  float opacityTexMat[4] = {1.0f, 0.0f, 0.0f, 1.0f};
  float translucencyTexMat[4] = {1.0f, 0.0f, 0.0f, 1.0f};
  float bumpTexMat[4] = {1.0f, 0.0f, 0.0f, 1.0f};

  auto emissionTex = a_materialNode.child(L"emission").child(L"color").child(L"texture");
  if (emissionTex  != nullptr)
  {
    emissionTexId = emissionTex.attribute(L"id").as_int();
    HydraXMLHelpers::ReadMatrix2x2From4x4(emissionTex, L"matrix", emissionTexMat);
  }

  auto diffuseTex = a_materialNode.child(L"diffuse").child(L"color").child(L"texture");
  if (diffuseTex  != nullptr)
  {
    diffuseTexId = diffuseTex.attribute(L"id").as_int();
    HydraXMLHelpers::ReadMatrix2x2From4x4(diffuseTex, L"matrix", diffuseTexMat);
  }

  auto reflectTex = a_materialNode.child(L"reflectivity").child(L"color").child(L"texture");
  if (reflectTex  != nullptr)
  {
    reflectTexId = reflectTex.attribute(L"id").as_int();
    HydraXMLHelpers::ReadMatrix2x2From4x4(reflectTex, L"matrix", reflectTexMat);
  }

  
  auto reflectGlossTex = a_materialNode.child(L"reflectivity").child(L"glossiness").child(L"texture");
  if (reflectGlossTex  != nullptr)
  {
    reflectGlossTexId = reflectGlossTex.attribute(L"id").as_int();
    HydraXMLHelpers::ReadMatrix2x2From4x4(reflectGlossTex, L"matrix", reflectGlossTexMat);
  }

  auto transparencyTex = a_materialNode.child(L"transparency").child(L"color").child(L"texture");
  if (transparencyTex  != nullptr)
  {
    transparencyTexId = transparencyTex.attribute(L"id").as_int();
    HydraXMLHelpers::ReadMatrix2x2From4x4(transparencyTex, L"matrix", transparencyTexMat);
  }
  
  auto opacityTex = a_materialNode.child(L"opacity").child(L"texture");
  if (opacityTex  != nullptr)
  {
    opacityTexId = opacityTex.attribute(L"id").as_int();
    HydraXMLHelpers::ReadMatrix2x2From4x4(opacityTex, L"matrix", opacityTexMat);
  }

  auto translucencyTex = a_materialNode.child(L"translucency").child(L"color").child(L"texture");
  if (translucencyTex  != nullptr)
  {
    translucencyTexId = translucencyTex.attribute(L"id").as_int();
    HydraXMLHelpers::ReadMatrix2x2From4x4(translucencyTex, L"matrix", translucencyTexMat);
  }

  auto bumpTex = a_materialNode.child(L"displacement").child(L"normal_map").child(L"texture");
  if (bumpTex  != nullptr)
  {
    bumpTexId = bumpTex.attribute(L"id").as_int();
    HydraXMLHelpers::ReadMatrix2x2From4x4(bumpTex, L"matrix", bumpTexMat);
  }


  int4 mat_pt1 = int4(emissionTexId, diffuseTexId, reflectTexId, reflectGlossTexId);
  int4 mat_pt2 = int4(transparencyTexId, opacityTexId, translucencyTexId, bumpTexId);

//  int4 mat_pt1 = int4(3, 4, 5, 6);
//  int4 mat_pt2 = int4(7, 8, 9, 10);

  m_materials_pt1.at(mat_id) = mat_pt1;
  m_materials_pt2.at(mat_id) = mat_pt2;

  m_materials_matrix.at(mat_id * 8 + 0) = float4(emissionTexMat);
  m_materials_matrix.at(mat_id * 8 + 1) = float4(diffuseTexMat);
  m_materials_matrix.at(mat_id * 8 + 2) = float4(reflectTexMat);
  m_materials_matrix.at(mat_id * 8 + 3) = float4(reflectGlossTexMat);

  m_materials_matrix.at(mat_id * 8 + 4) = float4(translucencyTexMat);
  m_materials_matrix.at(mat_id * 8 + 5) = float4(opacityTexMat);
  m_materials_matrix.at(mat_id * 8 + 6) = float4(translucencyTexMat);
  m_materials_matrix.at(mat_id * 8 + 7) = float4(bumpTexMat);

  return true;
}

bool RD_OGL32_Utility::UpdateLight(int32_t a_lightId, pugi::xml_node a_lightNode)
{
  return true;
}

bool RD_OGL32_Utility::UpdateMesh(int32_t a_meshId, pugi::xml_node a_meshNode, const HRMeshDriverInput &a_input,
                                   const HRBatchInfo *a_batchList, int32_t a_listSize)
{
  if (a_input.triNum == 0)
  {
    return true;
  }
  //TODO: maybe try MultiDrawIndirect

  GLuint vertexPosBufferObject;
  GLuint vertexNormBufferObject;
  GLuint vertexTexCoordsBufferObject;

  GLuint indexBufferObject;
  GLuint vertexArrayObject;

// vertex positions
  glGenBuffers(1, &vertexPosBufferObject);
  glBindBuffer(GL_ARRAY_BUFFER, vertexPosBufferObject);
  glBufferData(GL_ARRAY_BUFFER, a_input.vertNum * 4 * sizeof(GLfloat), a_input.pos4f, GL_STATIC_DRAW);

  // vertex normals
  glGenBuffers(1, &vertexNormBufferObject);
  glBindBuffer(GL_ARRAY_BUFFER, vertexNormBufferObject);
  glBufferData(GL_ARRAY_BUFFER, a_input.vertNum * 4 * sizeof(GLfloat), a_input.norm4f, GL_STATIC_DRAW);

  // vertex texture coordinates
  glGenBuffers(1, &vertexTexCoordsBufferObject);
  glBindBuffer(GL_ARRAY_BUFFER, vertexTexCoordsBufferObject);
  glBufferData(GL_ARRAY_BUFFER, a_input.vertNum * 2 * sizeof(GLfloat), a_input.texcoord2f, GL_STATIC_DRAW);

  // index buffer
  glGenBuffers(1, &indexBufferObject);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, a_input.triNum * 3 * sizeof(GLuint), a_input.indices, GL_STATIC_DRAW);

  m_allVBOs.push_back(vertexPosBufferObject);
  m_allVBOs.push_back(vertexNormBufferObject);
  m_allVBOs.push_back(vertexTexCoordsBufferObject);

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

//     glBindBuffer(GL_ARRAY_BUFFER, matIDBufferObject);
//     glEnableVertexAttribArray(4);
//     glVertexAttribPointer(4, 1, GL_INT, GL_FALSE, 0, nullptr);


  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);

  glBindVertexArray(0); GL_CHECK_ERRORS;

  meshData batchMeshData;
  batchMeshData.first = vertexArrayObject;
  for (int32_t batchId = 0; batchId < a_listSize; ++batchId)
  {
    HRBatchInfo batch = a_batchList[batchId];
    std::pair<int, int> tmp;
    tmp.first = batch.triBegin;
    tmp.second = batch.triEnd;
    batchMeshData.second[batch.matId] = tmp;
  }

  m_objects[a_meshId] = batchMeshData;

  return true;
}


bool RD_OGL32_Utility::UpdateCamera(pugi::xml_node a_camNode)
{
  if (a_camNode == nullptr)
    return true;

  const std::wstring camPosStr = a_camNode.child(L"position").text().as_string();
  const std::wstring camLAtStr = a_camNode.child(L"look_at").text().as_string();
  const std::wstring camUpStr  = a_camNode.child(L"up").text().as_string();

  if (!a_camNode.child(L"fov").text().empty())
    camFov = a_camNode.child(L"fov").text().as_float();

  if (!a_camNode.child(L"nearClipPlane").text().empty())
    camNearPlane = 0.1f;// a_camNode.child(L"nearClipPlane").text().as_float();

  if (!a_camNode.child(L"farClipPlane").text().empty())
    camFarPlane = 1000000.0f;//a_camNode.child(L"farClipPlane").text().as_float();

  if (!camPosStr.empty())
  {
    std::wstringstream input(camPosStr);
    input >> camPos[0] >> camPos[1] >> camPos[2];
  }

  if (!camLAtStr.empty())
  {
    std::wstringstream input(camLAtStr);
    input >> camLookAt[0] >> camLookAt[1] >> camLookAt[2];
  }

  if (!camUpStr.empty())
  {
    std::wstringstream input(camUpStr);
    input >> camUp[0] >> camUp[1] >> camUp[2];
  }

  return true;
}

bool RD_OGL32_Utility::UpdateSettings(pugi::xml_node a_settingsNode)
{
  if (a_settingsNode.child(L"width") != nullptr)
    m_settingsWidth = a_settingsNode.child(L"width").text().as_int();

  if (a_settingsNode.child(L"height") != nullptr)
    m_settingsHeight = a_settingsNode.child(L"height").text().as_int();

  return true;
}

void RD_OGL32_Utility::BeginScene(pugi::xml_node a_sceneNode)
{
  // std::cout << "BeginScene" <<std::endl;

  if(a_sceneNode.child(L"remap_lists") != nullptr)
  {
    for(auto listNode = a_sceneNode.child(L"remap_lists").first_child(); listNode != nullptr; listNode = listNode.next_sibling())
    {
      int listSize = listNode.attribute(L"size").as_int();

      std::unordered_map<uint32_t, uint32_t> remapList;
      const wchar_t* listStr = listNode.attribute(L"val").as_string();
      if(listStr != nullptr)
      {
        std::wstringstream inputStream(listStr);
        for(int i = 0; i < listSize; i += 2)
        {
          uint32_t a = 0;
          uint32_t b = 0;

          inputStream >> a;
          inputStream >> b;

          remapList[a] = b;
        }
      }
      m_remapLists.emplace_back(remapList);
    }
  }

  glViewport(0, 0, (GLint)m_width, (GLint)m_height);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  m_lodBufferProgram.StartUseShader();
  m_lodBuffer->StartRendering();

  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_DEPTH_CLAMP);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  const float aspect = float(m_width) / float(m_height);
  projection = projectionMatrixTransposed(camFov, aspect, camNearPlane, camFarPlane);

  float3 eye(camPos[0], camPos[1], camPos[2]);
  float3 center(camLookAt[0], camLookAt[1], camLookAt[2]);
  float3 up(camUp[0], camUp[1], camUp[2]);
  lookAt = lookAtTransposed(eye, center, up);

  std::vector<float4x4> matrices{lookAt , projection };

  glBindBuffer(GL_UNIFORM_BUFFER, m_matricesUBO);
  //glBufferData(GL_UNIFORM_BUFFER, 32 * sizeof(GLfloat), &matrices[0], GL_STATIC_DRAW);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, 32 * sizeof(GLfloat), &matrices[0]);
  glBindBuffer(GL_UNIFORM_BUFFER, 0); GL_CHECK_ERRORS;


  SetMaterialsTBO();

  //m_lodBufferProgram.SetUniform("window_res", int2(m_width, m_height));
  m_lodBufferProgram.SetUniform("max_tex_res", int2(MAX_TEXTURE_RESOLUTION, MAX_TEXTURE_RESOLUTION));
  m_lodBufferProgram.SetUniform("render_res", int2(m_settingsWidth, m_settingsHeight));

  int2 rasterization_res = int2(m_width, m_height);
  m_lodBufferProgram.SetUniform("rasterization_res", rasterization_res);

}



void RD_OGL32_Utility::EndScene()
{
  m_lodBuffer->EndRendering();

  FillMipLevelsDict();

  m_quadProgram.StartUseShader();
  bindTexture(m_quadProgram, 0, "debugTex", m_lodBuffer->GetTextureId(LODBuffer::LODBUF_TEX_1));

  m_quad->Draw();

  m_quadProgram.StopUseShader();

  /* Direct copy of render texture to the default framebuffer */
  /*
  m_gBuffer->StartFinalPass(0);
  glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
  */

  /*for (std::pair<int32_t, int32_t> elem : dict)
    std::cout << " " << elem.first << ":" << elem.second << std::endl;*/

  glFlush();
}


void RD_OGL32_Utility::InstanceMeshes(int32_t a_mesh_id, const float *a_matrices, int32_t a_instNum,
                                       const int *a_lightInstId, const int* a_remapId, const int* a_realInstId)
{
  // std::cout << "InstanceMeshes" <<std::endl;

  auto batchMeshData = m_objects[a_mesh_id];
  glBindVertexArray(batchMeshData.first);
  for (int32_t i = 0; i < a_instNum; i++)
  {
    float modelM[16];
    mat4x4_transpose(modelM, (float*)(a_matrices + i*16));

    int remapId = *(a_remapId + i);

    m_lodBufferProgram.SetUniform("model", float4x4(modelM));
    for(auto batch : batchMeshData.second)
    {
      int matId = batch.first;

      if(remapId != -1)
        matId = m_remapLists.at(remapId)[matId];

      m_lodBufferProgram.SetUniform("matID", matId);

      bindTextureBuffer(m_lodBufferProgram, 0, "materials1", m_materialsTBOs[0], m_materialsTBOTexIds[0], GL_RGBA32I);
      bindTextureBuffer(m_lodBufferProgram, 1, "materials2", m_materialsTBOs[1], m_materialsTBOTexIds[1], GL_RGBA32I);
      bindTextureBuffer(m_lodBufferProgram, 2, "materials_matrix", m_materialsTBOs[2], m_materialsTBOTexIds[2], GL_RGBA32F);

      auto triBegin = batch.second.first;
      auto triEnd   = batch.second.second;
      auto indices  = 3 * int(triEnd - triBegin);


      glDrawElements(GL_TRIANGLES, indices, GL_UNSIGNED_INT, (void*)(3 * triBegin * sizeof(GLuint)));
      m_lodBufferProgram.SetUniform("matID", -1);
    }
  }
  glBindVertexArray(0); GL_CHECK_ERRORS;
}


void RD_OGL32_Utility::InstanceLights(int32_t a_light_id, const float *a_matrix, pugi::xml_node* a_custAttrArray, int32_t a_instNum, int32_t a_lightGroupId)
{

}

void RD_OGL32_Utility::CreateMaterialsTBO()
{
  glGenBuffers(3, m_materialsTBOs);

  glBindBuffer(GL_TEXTURE_BUFFER, m_materialsTBOs[0]);
  glBufferData(GL_TEXTURE_BUFFER, sizeof(int32_t) * 4 * m_materials_pt1.size(), nullptr, GL_STATIC_DRAW);
  glGenTextures(1, &m_materialsTBOTexIds[0]);

  glBindBuffer(GL_TEXTURE_BUFFER, m_materialsTBOs[1]);
  glBufferData(GL_TEXTURE_BUFFER, sizeof(int32_t) * 4 * m_materials_pt2.size(), nullptr, GL_STATIC_DRAW);
  glGenTextures(1, &m_materialsTBOTexIds[1]);

  glBindBuffer(GL_TEXTURE_BUFFER, m_materialsTBOs[2]);
  glBufferData(GL_TEXTURE_BUFFER, sizeof(float) * 4 * m_materials_matrix.size(), nullptr, GL_STATIC_DRAW);
  glGenTextures(1, &m_materialsTBOTexIds[2]);

  glBindBuffer(GL_TEXTURE_BUFFER, 0);
}

void RD_OGL32_Utility::SetMaterialsTBO()
{
  glBindBuffer(GL_TEXTURE_BUFFER, m_materialsTBOs[0]);
  glBufferSubData(GL_TEXTURE_BUFFER, 0,  sizeof(int32_t) * 4 * m_materials_pt1.size(), &m_materials_pt1[0]);

  glBindBuffer(GL_TEXTURE_BUFFER, m_materialsTBOs[1]);
  glBufferSubData(GL_TEXTURE_BUFFER, 0,  sizeof(int32_t) * 4 * m_materials_pt2.size(), &m_materials_pt2[0]);

  glBindBuffer(GL_TEXTURE_BUFFER, m_materialsTBOs[2]);
  glBufferSubData(GL_TEXTURE_BUFFER, 0,  sizeof(float) * 4 * m_materials_matrix.size(), &m_materials_matrix[0]);

  glBindBuffer(GL_TEXTURE_BUFFER, 0); GL_CHECK_ERRORS;
}

void RD_OGL32_Utility::CreateMatricesUBO()
{
  GLuint uboIndexLODBuffer   = glGetUniformBlockIndex(m_lodBufferProgram.GetProgram(), "matrixBuffer");

  glUniformBlockBinding(m_lodBufferProgram.GetProgram(),    uboIndexLODBuffer,   m_matricesUBOBindingPoint);

  GLsizei matricesUBOSize = 32 * sizeof(GLfloat);

  glGenBuffers(1, &m_matricesUBO); 

  glBindBuffer(GL_UNIFORM_BUFFER, m_matricesUBO);
  glBufferData(GL_UNIFORM_BUFFER, matricesUBOSize, nullptr, GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  glBindBufferRange(GL_UNIFORM_BUFFER, m_matricesUBOBindingPoint, m_matricesUBO, 0, matricesUBOSize);

}

void RD_OGL32_Utility::Draw()
{
  //std::cout << "Draw" << std::endl;

}

void RD_OGL32_Utility::FillMipLevelsDict()
{
  std::vector<unsigned int> texture_data((unsigned long)(m_width * m_height* 4 * 2), 0);

  glBindTexture(GL_TEXTURE_2D, m_lodBuffer->GetTextureId(LODBuffer::LODBUF_TEX_1));
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, &texture_data[0]); GL_CHECK_ERRORS;

  glBindTexture(GL_TEXTURE_2D, m_lodBuffer->GetTextureId(LODBuffer::LODBUF_TEX_2));
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, &texture_data[m_width * m_height * 4]); GL_CHECK_ERRORS;

  const unsigned int texIdBits    = 0x00FFFFFFu;
  //const unsigned int mipLevelBits = 0xFF000000u;

  m_mipLevelDict.clear();

  for(auto pix : texture_data)
  {
    uint32_t mipLevel = pix >> 24u;
    uint32_t texId    = pix & texIdBits;

    if(texId <= m_texNum)
    {
      if (m_mipLevelDict.find(texId) != m_mipLevelDict.end())
      {
        if (mipLevel < m_mipLevelDict[texId])
          m_mipLevelDict[texId] = mipLevel;
      }
      else
        m_mipLevelDict[texId] = mipLevel;
    }
  }
}

HRRenderUpdateInfo RD_OGL32_Utility::HaveUpdateNow(int a_maxRaysPerPixel)
{
  HRRenderUpdateInfo res;
  res.finalUpdate   = true;
  res.haveUpdateFB  = true;
  res.progress      = 100.0f;
  return res;
}

void RD_OGL32_Utility::GetFrameBufferHDR(int32_t w, int32_t h, float *a_out, const wchar_t *a_layerName)
{
  glReadPixels(0, 0, w, h, GL_RGBA, GL_FLOAT, (GLvoid*)a_out);
}

void RD_OGL32_Utility::GetFrameBufferLDR(int32_t w, int32_t h, int32_t *a_out)
{
  glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)a_out);
}


IHRRenderDriver* CreateOpenGL3_Utilty_RenderDriver()
{
  return new RD_OGL32_Utility;
}

#ifndef WIN32
GLFWwindow * InitGLForUtilityDriver()
{
  GLFWwindow *offscreen_context = nullptr;

  //bool init_result = _init_GL_for_utility_driver(offscreen_context);

  if (!glfwInit())
  {
    HrError(L"Failed to initialize GLFW for Utility driver");
    return nullptr;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

  offscreen_context = glfwCreateWindow(1024, 1024, "", NULL, NULL);
  glfwMakeContextCurrent(offscreen_context);

  if (!offscreen_context)
  {
    HrError(L"Failed to create GLFW offscreen context for Utility driver");
    glfwTerminate();
    return nullptr;
  }

  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  return offscreen_context;
}
#endif
std::unordered_map<uint32_t, uint32_t> getMipLevelsFromUtilityDriver(IHRRenderDriver *driver)
{
  glFinish();
  RD_OGL32_Utility* utilityDrvRef = dynamic_cast<RD_OGL32_Utility *>(driver);
  if (utilityDrvRef == nullptr)
    return std::unordered_map<uint32_t, uint32_t>();

  return utilityDrvRef->GetMipLevelsDict();
};