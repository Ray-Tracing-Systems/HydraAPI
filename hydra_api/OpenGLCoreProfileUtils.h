//
// Created by hikawa on 18.08.17.
//

#ifndef HYDRAAPI_COREPROFILEUTILS_H
#define HYDRAAPI_COREPROFILEUTILS_H

#include <iostream>
#include <unordered_map>
#include <fstream>
#include <sstream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"


#include "LiteMath.h"
#include "pugixml.hpp"
#include "HydraRenderDriverAPI.h"



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

    default:
      std::cerr << "Unknown error @ file " << file << " line " << line << std::endl;
      break;
  }

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

      ShaderProgram(const std::unordered_map<GLenum, std::string> &inputShaders, bool a_fromStrings = false);

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
      static GLuint LoadShaderObject(GLenum type, const std::string &filename, bool a_fromStrings = false);

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


  struct LODBuffer
  {
      enum LODBUF_TEXTURE_TYPE
      {
          LODBUF_TEX_1, //emission, diffuse, reflection, reflection glossiness
          LODBUF_TEX_2, //transparency, opacity, translucence, bump
          LODBUF_TEX_3, //tesselation factor v1, v2, meshID, 0
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

}

#endif