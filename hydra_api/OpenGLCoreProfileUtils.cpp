//
// Created by hikawa on 18.08.17.
//

#include <iostream>
#include <random>
#include "OpenGLCoreProfileUtils.h"

namespace oldies
{
  extern "C" {
  #include "../ies_parser/IESNA.H"
  };
};

#include "FreeImage.h"
#include <algorithm>

#ifndef WIN32
#include <unistd.h>
#include <climits>

std::string getexepath()
{
  char result[PATH_MAX];
  ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
  return std::string(result, (count > 0) ? count : 0);
}

#undef min
#undef max

#else
std::string getexepath()
{
  char NPath[512];
  GetCurrentDirectoryA(512, NPath);
  return std::string(NPath);
}
#endif

namespace GL_RENDER_DRIVER_UTILS
{
  #ifdef M_PI
  #undef M_PI
  #endif
  static const float M_PI = 3.14159265358979323846f;
  static const float INV_PI = 1.0f / M_PI;
  static const float DEG_TO_RAD = M_PI / 180.0f;


  void bindTexture(const ShaderProgram &program, int unit, const std::string &name, RenderTexture2D &texture)
  {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture.GetTextureId(RenderTexture2D::RTEX_COLOR));

    program.SetUniform(name, unit);
  }

  void bindTexture(const ShaderProgram &program, int unit, const std::string &name, GLuint textureId)
  {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, textureId);

    program.SetUniform(name, unit);
  }

  void bindTextureBuffer(const ShaderProgram &program, int unit, const std::string &name, GLuint tbo, GLuint textureId, GLenum format)
  {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_BUFFER, textureId);
    glTexBuffer(GL_TEXTURE_BUFFER, format, tbo);

    program.SetUniform(name, unit);
  }


  void CreatePlaceholderWhiteTexture(GLuint &a_whiteTex)
  {
    glBindTexture(GL_TEXTURE_2D, a_whiteTex);

    float white[4] = {1.0f, 1.0f, 1.0f, 1.0f};

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1, 1, 0, GL_RGBA, GL_FLOAT, &white);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenerateMipmap(GL_TEXTURE_2D);
  }


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  ShaderProgram::ShaderProgram(const std::unordered_map<GLenum, std::string> &inputShaders, bool a_fromStrings)
  {

    shaderProgram = glCreateProgram();

    if (inputShaders.find(GL_VERTEX_SHADER) != inputShaders.end())
    {
      shaderObjects[GL_VERTEX_SHADER] = LoadShaderObject(GL_VERTEX_SHADER, inputShaders.at(GL_VERTEX_SHADER), a_fromStrings);
      glAttachShader(shaderProgram, shaderObjects[GL_VERTEX_SHADER]);
    }

    if (inputShaders.find(GL_FRAGMENT_SHADER) != inputShaders.end())
    {
      shaderObjects[GL_FRAGMENT_SHADER] = LoadShaderObject(GL_FRAGMENT_SHADER, inputShaders.at(GL_FRAGMENT_SHADER), a_fromStrings);
      glAttachShader(shaderProgram, shaderObjects[GL_FRAGMENT_SHADER]);
    }
    if (inputShaders.find(GL_GEOMETRY_SHADER) != inputShaders.end())
    {
      shaderObjects[GL_GEOMETRY_SHADER] = LoadShaderObject(GL_GEOMETRY_SHADER, inputShaders.at(GL_GEOMETRY_SHADER), a_fromStrings);
      glAttachShader(shaderProgram, shaderObjects[GL_GEOMETRY_SHADER]);
    }
    if (inputShaders.find(GL_TESS_CONTROL_SHADER) != inputShaders.end())
    {
      shaderObjects[GL_TESS_CONTROL_SHADER] = LoadShaderObject(GL_TESS_CONTROL_SHADER,
                                                               inputShaders.at(GL_TESS_CONTROL_SHADER), a_fromStrings);
      glAttachShader(shaderProgram, shaderObjects[GL_TESS_CONTROL_SHADER]);
    }
    if (inputShaders.find(GL_TESS_EVALUATION_SHADER) != inputShaders.end())
    {
      shaderObjects[GL_TESS_EVALUATION_SHADER] = LoadShaderObject(GL_TESS_EVALUATION_SHADER,
                                                                  inputShaders.at(GL_TESS_EVALUATION_SHADER), a_fromStrings);
      glAttachShader(shaderProgram, shaderObjects[GL_TESS_EVALUATION_SHADER]);
    }
    if (inputShaders.find(GL_COMPUTE_SHADER) != inputShaders.end())
    {
      shaderObjects[GL_COMPUTE_SHADER] = LoadShaderObject(GL_COMPUTE_SHADER, inputShaders.at(GL_COMPUTE_SHADER), a_fromStrings);
      glAttachShader(shaderProgram, shaderObjects[GL_COMPUTE_SHADER]);
    }

    glLinkProgram(shaderProgram);

    GLint linkStatus;


    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE)
    {
      GLchar infoLog[512];
      glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
      std::cerr << "Shader program linking failed\n" << infoLog << std::endl;
      shaderProgram = 0;
    }

  }


  void ShaderProgram::Release()
  {
    if (shaderObjects.find(GL_VERTEX_SHADER) != shaderObjects.end())
    {
      glDetachShader(shaderProgram, shaderObjects[GL_VERTEX_SHADER]);
      glDeleteShader(shaderObjects[GL_VERTEX_SHADER]);
    }

    if (shaderObjects.find(GL_FRAGMENT_SHADER) != shaderObjects.end())
    {
      glDetachShader(shaderProgram, shaderObjects[GL_FRAGMENT_SHADER]);
      glDeleteShader(shaderObjects[GL_FRAGMENT_SHADER]);
    }

    if (shaderObjects.find(GL_GEOMETRY_SHADER) != shaderObjects.end())
    {
      glDetachShader(shaderProgram, shaderObjects[GL_GEOMETRY_SHADER]);
      glDeleteShader(shaderObjects[GL_GEOMETRY_SHADER]);
    }

    if (shaderObjects.find(GL_TESS_CONTROL_SHADER) != shaderObjects.end())
    {
      glDetachShader(shaderProgram, shaderObjects[GL_TESS_CONTROL_SHADER]);
      glDeleteShader(shaderObjects[GL_TESS_CONTROL_SHADER]);
    }

    if (shaderObjects.find(GL_TESS_EVALUATION_SHADER) != shaderObjects.end())
    {
      glDetachShader(shaderProgram, shaderObjects[GL_TESS_EVALUATION_SHADER]);
      glDeleteShader(shaderObjects[GL_TESS_EVALUATION_SHADER]);
    }
    if (shaderObjects.find(GL_COMPUTE_SHADER) != shaderObjects.end())
    {
      glDetachShader(shaderProgram, shaderObjects[GL_COMPUTE_SHADER]);
      glDeleteShader(shaderObjects[GL_COMPUTE_SHADER]);
    }

    glDeleteProgram(shaderProgram);
  }

  bool ShaderProgram::reLink()
  {
    GLint linked;

    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);

    if (!linked)
    {
      GLint logLength, charsWritten;
      glGetProgramiv(this->shaderProgram, GL_INFO_LOG_LENGTH, &logLength);

      auto log = new char[logLength];
      glGetProgramInfoLog(this->shaderProgram, logLength, &charsWritten, log);

      std::cerr << "Shader program link error: " << std::endl << log << std::endl;

      delete[] log;
      shaderProgram = 0;
      return false;
    }

    return true;
  }


  GLuint ShaderProgram::LoadShaderObject(GLenum type, const std::string &filename, bool a_fromStrings)
  {
    std::string shaderText;

    if (!a_fromStrings)
    {
      std::ifstream fs(filename);
      if (!fs.is_open())
      {
        std::cerr << "ERROR: Could not read shader from " << filename << std::endl;
        return 0;
      }
      shaderText = std::string((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
    }
    else
      shaderText = filename;

    GLuint newShaderObject = glCreateShader(type);

    const char *shaderSrc = shaderText.c_str();
    glShaderSource(newShaderObject, 1, &shaderSrc, nullptr);

    glCompileShader(newShaderObject);

    GLint compileStatus;
    glGetShaderiv(newShaderObject, GL_COMPILE_STATUS, &compileStatus);

    if (compileStatus != GL_TRUE)
    {
      GLchar infoLog[512];
      glGetShaderInfoLog(newShaderObject, 512, nullptr, infoLog);
      std::cerr << "Shader compilation failed : " << std::endl << infoLog << std::endl;
      return 0;
    }

    return newShaderObject;
  }

  void ShaderProgram::StartUseShader() const
  {
    glUseProgram(shaderProgram);
  }

  void ShaderProgram::StopUseShader() const
  {
    glUseProgram(0);
  }

  void ShaderProgram::SetUniform(const std::string &location, const float4x4 &value) const
  {
    GLint uniformLocation = glGetUniformLocation(shaderProgram, location.c_str());
    if (uniformLocation == -1)
    {
      std::cerr << "Uniform  " << location << " not found" << std::endl;
      return;
    }

    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, value.L());
  }

  void ShaderProgram::SetUniform(const std::string &location, int value) const
  {
    GLint uniformLocation = glGetUniformLocation(shaderProgram, location.c_str());
    if (uniformLocation == -1)
    {
      std::cerr << "Uniform  " << location << " not found" << std::endl;
      return;
    }
    glUniform1i(uniformLocation, value);
  }

  void ShaderProgram::SetUniform(const std::string &location, unsigned int value) const
  {
    GLint uniformLocation = glGetUniformLocation(shaderProgram, location.c_str());
    if (uniformLocation == -1)
    {
      std::cerr << "Uniform  " << location << " not found" << std::endl;
      return;
    }
    glUniform1ui(uniformLocation, value);
  }

  void ShaderProgram::SetUniform(const std::string &location, float value) const
  {
    GLint uniformLocation = glGetUniformLocation(shaderProgram, location.c_str());
    if (uniformLocation == -1)
    {
      std::cerr << "Uniform  " << location << " not found" << std::endl;
      return;
    }
    glUniform1f(uniformLocation, value);
  }

  void ShaderProgram::SetUniform(const std::string &location, double value) const
  {
    GLint uniformLocation = glGetUniformLocation(shaderProgram, location.c_str());
    if (uniformLocation == -1)
    {
      std::cerr << "Uniform  " << location << " not found" << std::endl;
      return;
    }
    glUniform1d(uniformLocation, value);
  }

  void ShaderProgram::SetUniform(const std::string &location, const float4 &value) const
  {
    GLint uniformLocation = glGetUniformLocation(shaderProgram, location.c_str());
    if (uniformLocation == -1)
    {
      std::cerr << "Uniform  " << location << " not found" << std::endl;
      return;
    }
    float val[] = {value.x, value.y, value.z, value.w};
    glUniform4fv(uniformLocation, 1, val);
  }

  void ShaderProgram::SetUniform(const std::string &location, const float3 &value) const
  {
    GLint uniformLocation = glGetUniformLocation(shaderProgram, location.c_str());
    if (uniformLocation == -1)
    {
      std::cerr << "Uniform  " << location << " not found" << std::endl;
      return;
    }
    float val[] = {value.x, value.y, value.z};
    glUniform3fv(uniformLocation, 1, val);
  }

  void ShaderProgram::SetUniform(const std::string &location, const float2 &value) const
  {
    GLint uniformLocation = glGetUniformLocation(shaderProgram, location.c_str());
    if (uniformLocation == -1)
    {
      std::cerr << "Uniform  " << location << " not found" << std::endl;
      return;
    }
    float val[] = {value.x, value.y};
    glUniform2fv(uniformLocation, 1, val);
  }

  void ShaderProgram::SetUniform(const std::string &location, const int2 &value) const
  {
    GLint uniformLocation = glGetUniformLocation(shaderProgram, location.c_str());
    if (uniformLocation == -1)
    {
      std::cerr << "Uniform  " << location << " not found" << std::endl;
      return;
    }
    int val[] = {value.x, value.y};
    glUniform2iv(uniformLocation, 1, val);
  }

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
  GLuint CreateEmptyTex(GLenum format, GLenum internalFormat, GLsizei width, GLsizei height, GLenum type)
  {
    GLuint texture;

    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);

    return texture;
  }

  RenderTexture2D::RenderTexture2D() : frameBufferObject(-1), width(0), height(0)
  {
    for (int i = 0; i < RTEX_NUM_TEXTURES; ++i)
      renderTextures[i] = GLuint(-1);
  }

  RenderTexture2D::RenderTexture2D(GLenum format, GLenum internal_format, GLsizei width, GLsizei height) : width(
      width), height(height), m_format(format), m_internal_format(internal_format)
  {
    glGenFramebuffers(1, &frameBufferObject);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);

    for (int i = 0; i < RTEX_NUM_TEXTURES; ++i)
    {
      renderTextures[i] = CreateEmptyTex(format, internal_format, width, height);
      glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, renderTextures[i], 0);
    }

    depthTex = CreateEmptyTex(GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT24, width, height, GL_UNSIGNED_BYTE);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTex, 0);

    GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
      std::cout << "Framebuffer is not complete" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void RenderTexture2D::ResizeAttachments(GLsizei width, GLsizei height)
  {
    for (int i = 0; i < RTEX_NUM_TEXTURES; ++i)
    {
      glBindTexture(GL_TEXTURE_2D, renderTextures[i]);
      glTexImage2D(GL_TEXTURE_2D, 0, m_internal_format, width, height, 0, m_format, GL_FLOAT, NULL);
    }

    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE,
                 NULL);
  }

  RenderTexture2D::~RenderTexture2D()
  {
    glDeleteTextures(RTEX_NUM_TEXTURES, renderTextures);
    glDeleteTextures(1, &depthTex);
    for (int i = 0; i < RTEX_NUM_TEXTURES; ++i)
      renderTextures[i] = GLuint(-1);

    glDeleteFramebuffers(1, &frameBufferObject);
  }


  void RenderTexture2D::StartRendering()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void RenderTexture2D::EndRendering()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////


  LODBuffer::LODBuffer(GLsizei width, GLsizei height) : width(width), height(height)
  {
    glGenFramebuffers(1, &frameBufferObject);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);

    for (int i = 0; i < LODBUF_NUM_TEXTURES; ++i)
    {
      renderTextures[i] = CreateEmptyTex(GL_RGBA_INTEGER, GL_RGBA32UI, width, height, GL_UNSIGNED_INT);//GL_RGBA32I
      glFramebufferTexture(GL_FRAMEBUFFER, GLenum(GL_COLOR_ATTACHMENT0 + i), renderTextures[i], 0);
    }

    depthTex = CreateEmptyTex(GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT24, width, height, GL_UNSIGNED_BYTE);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTex, 0);

    GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
      std::cout << "(LODBuffer) Framebuffer is not complete" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void LODBuffer::ResizeAttachments(GLsizei width, GLsizei height)
  {
    for (int i = 0; i < LODBUF_NUM_TEXTURES; ++i)
    {
      glBindTexture(GL_TEXTURE_2D, renderTextures[i]);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32UI, width, height, 0, GL_RGBA, GL_UNSIGNED_INT, NULL);
    }
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
  }


  LODBuffer::~LODBuffer()
  {
    glDeleteTextures(LODBUF_NUM_TEXTURES, renderTextures);
    glDeleteTextures(1, &depthTex);
    for (int i = 0; i < LODBUF_NUM_TEXTURES; ++i)
      renderTextures[i] = GLuint(-1);

    glDeleteFramebuffers(1, &frameBufferObject);
  }

  void LODBuffer::StartRendering()
  {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBufferObject);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLenum attachments[LODBUF_NUM_TEXTURES];

    for (int i = 0; i < LODBUF_NUM_TEXTURES; ++i)
      attachments[i] = GLenum(GL_COLOR_ATTACHMENT0 + i);

    glDrawBuffers(LODBUF_NUM_TEXTURES, attachments);
  }


  void LODBuffer::EndRendering()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  GLuint LODBuffer::GetTextureId(LODBUF_TEXTURE_TYPE type)
  {
    if (type == LODBUF_NUM_TEXTURES)
      return GLuint(-1);
    else
      return renderTextures[type];
  }

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

  FullScreenQuad::FullScreenQuad()
  {
    float quadPos[] = {-1.0f, 1.0f,
                       -1.0f, -1.0f,
                       1.0f, 1.0f,
                       1.0f, -1.0f};

    vertexPosBufferObject = 0;
    vertexPosLocation = 0;

    glGenBuffers(1, &vertexPosBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexPosBufferObject);
    glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), (GLfloat *) quadPos, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);

    glBindBuffer(GL_ARRAY_BUFFER, vertexPosBufferObject);
    glEnableVertexAttribArray(vertexPosLocation);
    glVertexAttribPointer(vertexPosLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);
  }

  FullScreenQuad::~FullScreenQuad()
  {
    if (vertexPosBufferObject)
    {
      glDeleteBuffers(1, &vertexPosBufferObject);
      vertexPosBufferObject = 0;
    }

    if (vertexArrayObject)
    {
      glDeleteVertexArrays(1, &vertexArrayObject);
      vertexArrayObject = 0;
    }

  }

  void FullScreenQuad::Draw()
  {
    glBindVertexArray(vertexArrayObject);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 8);
  }

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

}