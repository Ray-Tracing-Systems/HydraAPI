//
// Created by hikawa on 18.08.17.
//

#pragma once

#include <iostream>
#include <unordered_map>
#include <fstream>
#include <sstream>


#include "LiteMath.h"
#include "pugixml.hpp"
#include "HydraRenderDriverAPI.h"
#include "glad/glad.h" 

using namespace HydraLiteMath;

#define GL_CHECK_ERRORS ThrowExceptionOnGLError(__LINE__,__FILE__);

static void ThrowExceptionOnGLError(int line, const char *file)
{

  static char errMsg[512];

  //вызывается функция glGetError, проверяющая не произошла ли ошибка
  //в каком-то вызове opengl и если произошла, то какой код у ошибки
  GLenum gl_error = glGetError();

  if (gl_error == GL_NO_ERROR)
    return;

  switch (gl_error)
  {
    case GL_INVALID_ENUM:
      std::cerr << "GL_INVALID_ENUM file " << file << " line " << line << std::endl;
      break;

    case GL_INVALID_VALUE:
      std::cerr << "GL_INVALID_VALUE file " << file << " line " << line << std::endl;
      break;

    case GL_INVALID_OPERATION:
      std::cerr << "GL_INVALID_OPERATION file " << file << " line " << line << std::endl;
      break;

    case GL_STACK_OVERFLOW:
      std::cerr << "GL_STACK_OVERFLOW file " << file << " line " << line << std::endl;
      break;

    case GL_STACK_UNDERFLOW:
      std::cerr << "GL_STACK_UNDERFLOW file " << file << " line " << line << std::endl;
      break;

    case GL_OUT_OF_MEMORY:
      std::cerr << "GL_OUT_OF_MEMORY file " << file << " line " << line << std::endl;
      break;

    case GL_NO_ERROR:
      break;

    default:
      std::cerr << "Unknown error @ file " << file << " line " << line << std::endl;
      break;
  }

  if (gl_error != GL_NO_ERROR)
    throw std::runtime_error(errMsg);
}

std::string getexepath();

namespace GL_RENDER_DRIVER_UTILS {

  inline static int find_interval(float x)
  {
    if (fabs(x - 1.0f) < 1e-5f)
      return 10;
    else
      return (int) (x * 10);
  }

  inline static float GetCosPowerFromGlossiness(float glossiness)
  {
    float cMin = 1.0f;
    float cMax = 1000000.0f;

    float x = glossiness;

    float coeff[10][4] = {
            {8.88178419700125e-14f, -1.77635683940025e-14f, 5.0f,              1.0f}, //0-0.1
            {357.142857142857f,     -35.7142857142857f,     5.0f,              1.5f}, //0.1-0.2
            {-2142.85714285714f,    428.571428571429f,      8.57142857142857f, 2.0f}, //0.2-0.3
            {428.571428571431f,     -42.8571428571432f,     30.0f,             5.0f}, //0.3-0.4
            {2095.23809523810f,     -152.380952380952f,     34.2857142857143f, 8.0f}, //0.4-0.5
            {-4761.90476190476f,    1809.52380952381f,      66.6666666666667f, 12.0f},//0.5-0.6
            {9914.71215351811f,     1151.38592750533f,      285.714285714286f, 32.0f}, //0.6-0.7
            {45037.7068059246f,     9161.90096119855f,      813.432835820895f, 82.0f}, //0.7-0.8
            {167903.678757035f,     183240.189801913f,      3996.94423223835f, 300.0f}, //0.8-0.9
            {-20281790.7444668f,    6301358.14889336f,      45682.0925553320f, 2700.0f} //0.9-1.0
    };

    int k = find_interval(x);

    if (k == 10 || x >= 0.99f)
      return cMax;
    else
      return coeff[k][3] + coeff[k][2] * (x - k * 0.1f) + coeff[k][1] * powf((x - k * 0.1f), 2.0f) +
             coeff[k][0] * powf((x - k * 0.1f), 3.0f);
  }

  inline void mat4x4_transpose(float M[16], const float N[16])
  {
    for (int j = 0; j < 4; j++) {
      for (int i = 0; i < 4; i++)
        M[i * 4 + j] = N[j * 4 + i];
    }
  }


  class ShaderProgram
  {
  public:

      ShaderProgram() {};

      ShaderProgram(const std::unordered_map<GLenum, std::string> &inputShaders);

      virtual ~ShaderProgram() {};

      void Release(); //actual destructor

      virtual void StartUseShader() const;

      virtual void StopUseShader() const;

      GLuint GetProgram() const { return shaderProgram; }


      bool reLink();

      void SetUniform(const std::string &location, const float4x4 &value) const;

      void SetUniform(const std::string &location, float value) const;

      void SetUniform(const std::string &location, double value) const;

      void SetUniform(const std::string &location, int value) const;

      void SetUniform(const std::string &location, unsigned int value) const;

      void SetUniform(const std::string &location, const float4 &value) const;

      void SetUniform(const std::string &location, const float3 &value) const;

      void SetUniform(const std::string &location, const float2 &value) const;

      void SetUniform(const std::string &location, const int2 &value) const;

  private:
      static GLuint LoadShaderObject(GLenum type, const std::string &filename);

      GLuint shaderProgram;
      std::unordered_map<GLenum, GLuint> shaderObjects;
  };

  struct RenderTexture2D
  {
      enum RENDER_TEXTURE_TYPE
      {
          RTEX_COLOR,
          RTEX_NUM_TEXTURES
      };

      RenderTexture2D();

      RenderTexture2D(GLenum format, GLenum internal_format, GLsizei width, GLsizei height);

      ~RenderTexture2D();

      void StartRendering();

      void EndRendering();

      void ResizeAttachments(GLsizei width, GLsizei height);

      GLuint GetTextureId(RENDER_TEXTURE_TYPE type) { return renderTextures[type]; }

  protected:
      GLenum m_format;
      GLenum m_internal_format;
      GLsizei width;
      GLsizei height;
      GLuint depthTex;
      GLuint renderTextures[RTEX_NUM_TEXTURES];
      GLuint frameBufferObject;
  };

  struct GBuffer
  {
      enum GBUF_TEXTURE_TYPE
      {
          GBUF_POSITION,
          GBUF_NORMAL,
          GBUF_DIFFUSE,
          GBUF_REFLECTION,
          GBUF_NUM_TEXTURES
      };

      virtual ~GBuffer();

      GBuffer(GLsizei width, GLsizei height);

      GLuint GetTextureId(GBUF_TEXTURE_TYPE type);

      GLuint GetDepthTex() { return depthTex; }

      GLuint GetFrameBuffer() { return frameBufferObject; }

      void FrameInit();

      void StartGeoPass();

      void StartLightPass();

      void StartFinalPass(const GLuint &targetFBO);

      void EndRendering();

      void ResizeAttachments(GLsizei width, GLsizei height);

  protected:
      GLsizei width;
      GLsizei height;
      GLuint depthTex;
      GLuint finalTex;
      GLuint renderTextures[GBUF_NUM_TEXTURES];
      GLuint frameBufferObject;
  };


  struct LODBuffer
  {
      enum LODBUF_TEXTURE_TYPE
      {
          LODBUF_TEX_1, //emission, diffuse, reflection, reflection glossiness
          LODBUF_TEX_2, //transparency, opacity, translucence, bump
          LODBUF_NUM_TEXTURES
      };

      virtual ~LODBuffer();

      LODBuffer(GLsizei width, GLsizei height);

      GLuint GetTextureId(LODBUF_TEXTURE_TYPE type);

      GLuint GetFrameBuffer() { return frameBufferObject; }

      void StartRendering();

      void EndRendering();

      void ResizeAttachments(GLsizei width, GLsizei height);

  protected:
      GLsizei width;
      GLsizei height;
      GLuint renderTextures[LODBUF_NUM_TEXTURES];
      GLuint depthTex;
      GLuint frameBufferObject;
  };

  struct SSAOBuffer
  {
    enum SSAO_TEXTURE_TYPE
    {
      SSAO_RAW,
      SSAO_BLUR,
      SSAO_NUM_TEXTURES
    };

    SSAOBuffer(GLsizei width, GLsizei height, int samples = 64, float radius = 0.5f);

    ~SSAOBuffer();

    GLuint GetTextureId(SSAO_TEXTURE_TYPE type);

    void StartSSAOPass(const float4x4 &projection, const GLuint positionTex, const GLuint normalTex);

    void StartSSAOBlurPass();

    void EndSSAOPass();

    void EndSSAOBlurPass();

    void ResizeAttachments(GLsizei width, GLsizei height);

    GLuint GetSSAOProgramID()     const { return ssaoProgram.GetProgram(); };
    GLuint GetSSAOBlurProgramID() const { return ssaoBlurProgram.GetProgram(); };

  protected:
    GLsizei width;
    GLsizei height;
    int numSamples;
    float radius;

    ShaderProgram ssaoProgram;
    ShaderProgram ssaoBlurProgram;

    GLuint settingsUBO;

    GLuint rotTexture;
    GLuint samplesTexture;
    GLuint tbo;
    GLuint renderTextures[SSAO_NUM_TEXTURES];
    GLuint fbo;
    GLuint fboBlur;
  };

  struct FullScreenQuad
  {
      FullScreenQuad();

      virtual ~FullScreenQuad();

      void Draw();

      GLuint GetVBO() { return vertexPosBufferObject; }

  protected:

      FullScreenQuad(const FullScreenQuad &rhs) {}

      FullScreenQuad &operator=(const FullScreenQuad &rhs) { return *this; }

      GLuint vertexPosBufferObject;
      GLuint vertexPosLocation;
      GLuint vertexArrayObject;
  };

  GLuint CreateEmptyTex(GLenum internalFormat, GLenum format, GLsizei width, GLsizei height, GLenum type = GL_FLOAT);

  void bindTexture(const ShaderProgram &program, int unit, const std::string &name, RenderTexture2D &texture);

  void bindTexture(const ShaderProgram &program, int unit, const std::string &name, GLuint textureId);

  void bindTextureBuffer(const ShaderProgram &program, int unit, const std::string &name, GLuint tbo, GLuint textureId, GLenum format = GL_R32F);

  void CreatePlaceholderWhiteTexture(GLuint &a_whiteTex);

  void LoadFloat3FromXMLNode(pugi::xml_node node, std::vector<float3> &attrib, int a_matId);

  void LoadFloatFromXMLNode(pugi::xml_node node, std::vector<float> &attrib, int a_matId, bool isShininess);

  float3 GetFloat3FromXMLNode(pugi::xml_node node);

  float4x4 GetFloat4x4FromXMLNode(pugi::xml_node node);

  void CreateGeometryFromBatch(const HRBatchInfo &batch, const HRMeshDriverInput &a_input, std::vector<float> &a_pos,
                               std::vector<float> &a_norm, std::vector<float> &a_batchTangent, std::vector<float> &a_texcoords,
                               std::vector<int> &a_indices);

  void CreateRandomLights(int num, std::vector<float3> &pos, std::vector<float3> &color);

  void AssignRandomIESFiles(int num, std::vector<GLuint> &iesTextures);

  GLuint ChooseRandomTexture(std::vector<GLuint> &textures);

  //void CreateTextureFromFile(const std::string &file_name, GLuint &tex);

  void CreateTextureFromData(GLuint &texId, int32_t w, int32_t h, int32_t bpp, const void *a_data,
                             GLenum interpolation = GL_NEAREST);

  GLuint CreateSphere(float radius, int numberSlices, int posLocation, int normLocation, int texLocation,
                      int instanceMatLocation, int colorLocation, int &boundingSphereIndices, GLuint &instanceVBO,
                      GLuint &colorVBO);

  int CreateGLSphereMapFromIES(std::string filename, GLuint &texId);

  void GenerateSamplesInHemisphere(std::vector<float3> &samples, int numSamples);

  void CreateRandomSamplesTBO(int numSamples, GLuint &texId, GLuint &tbo);
}