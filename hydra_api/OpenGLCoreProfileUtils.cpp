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

  void
  CreateTextureFromData(GLuint &texId, int32_t w, int32_t h, int32_t bpp, const void *a_data, GLenum interpolation)
  {
    if (a_data == nullptr)
      CreatePlaceholderWhiteTexture(texId);

    glBindTexture(GL_TEXTURE_2D, texId);
    if (bpp > 4)
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, a_data);
    else if (bpp == 1)
    {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, a_data);
      GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_RED};
      glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
    } else
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, a_data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interpolation);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interpolation);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenerateMipmap(GL_TEXTURE_2D);


    glBindTexture(GL_TEXTURE_2D, 0);

    //TODO: texture matrices
  }
  /*
  void CreateTextureFromFile(const std::string &file_name, GLuint &tex)
  {
    FreeImage_Initialise(true);
    FREE_IMAGE_FORMAT format = FreeImage_GetFileType(file_name.c_str(), 0);
    bool isHDR = false;

    if (format == -1)
    {
      std::cerr << "can't find texture " << file_name << std::endl;
    } else
    {
      if (format == FIF_EXR || format == FIF_HDR)
        isHDR = true;

      FIBITMAP *bitmap = FreeImage_Load(format, file_name.c_str());
      int bitsPerPixel = FreeImage_GetBPP(bitmap);
      int nWidth = FreeImage_GetWidth(bitmap);
      int nHeight = FreeImage_GetHeight(bitmap);
      FIBITMAP *bitmap_new = nullptr;

      if (!isHDR)
      {
        if (bitsPerPixel == 32)
          bitmap_new = bitmap;
        else
          bitmap_new = FreeImage_ConvertTo32Bits(bitmap);

        GLubyte *textureData = FreeImage_GetBits(bitmap_new);

        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nWidth, nHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, textureData);
      } else
      {
        FIBITMAP *bitmap_new = FreeImage_ConvertToRGBAF(bitmap);
        int texSize = 4 * nWidth * nHeight;
        FIRGBAF *textureData = (FIRGBAF *) FreeImage_GetBits(bitmap_new);
        GLfloat *pixels = new GLfloat[texSize];

        memcpy(pixels, textureData, texSize * sizeof(GLfloat));

        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, nWidth, nHeight, 0, GL_RGBA, GL_FLOAT, pixels);

        delete[] pixels;
      }

      glGenerateMipmap(GL_TEXTURE_2D);

      if (bitmap_new) FreeImage_Unload(bitmap_new);
      if (bitmap) FreeImage_Unload(bitmap);
    }

    FreeImage_DeInitialise();
  }
  */

  GLuint CreateSphere(float radius, int numberSlices, int posLocation, int normLocation, int texLocation,
                      int instanceMatLocation, int colorLocation, int &boundingSphereIndices, GLuint &instanceVBO,
                      GLuint &colorVBO)
  {
    int i, j;

    int numberParallels = numberSlices;
    int numberVertices = (numberParallels + 1) * (numberSlices + 1);
    int numberIndices = numberParallels * numberSlices * 3;

    boundingSphereIndices = numberIndices;

    float angleStep = (2.0f * 3.14159265358979323846f) / ((float) numberSlices);
    //float helpVector[3] = {0.0f, 1.0f, 0.0f};

    std::vector<float> pos(numberVertices * 4, 0.0f);
    std::vector<float> norm(numberVertices * 4, 0.0f);
    std::vector<float> texcoords(numberVertices * 2, 0.0f);

    std::vector<int> indices(numberIndices, -1);

    for (i = 0; i < numberParallels + 1; i++)
    {
      for (j = 0; j < numberSlices + 1; j++)
      {
        int vertexIndex = (i * (numberSlices + 1) + j) * 4;
        int normalIndex = (i * (numberSlices + 1) + j) * 4;
        int texCoordsIndex = (i * (numberSlices + 1) + j) * 2;

        pos.at(vertexIndex + 0) = radius * sinf(angleStep * (float) i) * sinf(angleStep * (float) j);
        pos.at(vertexIndex + 1) = radius * cosf(angleStep * (float) i);
        pos.at(vertexIndex + 2) = radius * sinf(angleStep * (float) i) * cosf(angleStep * (float) j);
        pos.at(vertexIndex + 3) = 1.0f;

        norm.at(normalIndex + 0) = pos.at(vertexIndex + 0) / radius;
        norm.at(normalIndex + 1) = pos.at(vertexIndex + 1) / radius;
        norm.at(normalIndex + 2) = pos.at(vertexIndex + 2) / radius;
        norm.at(normalIndex + 3) = 1.0f;

        texcoords.at(texCoordsIndex + 0) = (float) j / (float) numberSlices;
        texcoords.at(texCoordsIndex + 1) = (1.0f - (float) i) / (float) (numberParallels - 1);
      }
    }

    int *indexBuf = &indices[0];

    for (i = 0; i < numberParallels; i++)
    {
      for (j = 0; j < numberSlices; j++)
      {
        *indexBuf++ = i * (numberSlices + 1) + j;
        *indexBuf++ = (i + 1) * (numberSlices + 1) + j;
        *indexBuf++ = (i + 1) * (numberSlices + 1) + (j + 1);

        *indexBuf++ = i * (numberSlices + 1) + j;
        *indexBuf++ = (i + 1) * (numberSlices + 1) + (j + 1);
        *indexBuf++ = i * (numberSlices + 1) + (j + 1);

        int diff = int(indexBuf - &indices[0]);
        if (diff >= numberIndices)
          break;
      }
      int diff = int(indexBuf - &indices[0]);
      if (diff >= numberIndices)
        break;
    }

    GLuint vboVertices, vboIndices, vboNormals, vboTexCoords;
    GLuint vao;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vboIndices);

    glBindVertexArray(vao);

    if (posLocation >= 0)
    {
      glGenBuffers(1, &vboVertices);
      glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
      glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(GLfloat), &pos[0], GL_STATIC_DRAW);
      glVertexAttribPointer(posLocation, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid *) 0);
      glEnableVertexAttribArray(posLocation);
    }

    if (normLocation >= 0)
    {
      glGenBuffers(1, &vboNormals);
      glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
      glBufferData(GL_ARRAY_BUFFER, norm.size() * sizeof(GLfloat), &norm[0], GL_STATIC_DRAW);
      glVertexAttribPointer(normLocation, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid *) 0);
      glEnableVertexAttribArray(normLocation);
    }

    if (texLocation >= 0)
    {
      glGenBuffers(1, &vboTexCoords);
      glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords);
      glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(GLfloat), &texcoords[0], GL_STATIC_DRAW);
      glVertexAttribPointer(texLocation, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid *) 0);
      glEnableVertexAttribArray(texLocation);
    }

    if(instanceMatLocation >= 0)
    {
      glGenBuffers(1, &instanceVBO);
      glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    }

    if (colorLocation >= 0)
    {
      glGenBuffers(1, &colorVBO);
      glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    }


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);

    return vao;
  }


  void LoadFloat3FromXMLNode(pugi::xml_node node, std::vector<float3> &attrib, int a_matId)
  {
    float3 color(0.0f, 0.0f, 0.0f);

    if (node != nullptr)
    {
      const wchar_t *str = nullptr;

      if (node.attribute(L"val") != nullptr)
        str = node.attribute(L"val").as_string();
      else
        str = node.text().as_string();

      if (!std::wstring(str).empty())
      {
        std::wstringstream input(str);
        input >> color.x >> color.y >> color.z;
      }
    }
    attrib[a_matId] = color;
  }

  float3 GetFloat3FromXMLNode(pugi::xml_node node)
  {
    float3 color(0.0f, 0.0f, 0.0f);

    if (node != nullptr)
    {
      const wchar_t *str = nullptr;

      if (node.attribute(L"val") != nullptr)
        str = node.attribute(L"val").as_string();
      else
        str = node.text().as_string();

      if (!std::wstring(str).empty())
      {
        std::wstringstream input(str);
        input >> color.x >> color.y >> color.z;
      }
    }
    return color;
  }

  float4x4 GetFloat4x4FromXMLNode(pugi::xml_node node)
  {
    float mat[16];

    if (node != nullptr)
    {
      const wchar_t *str = nullptr;

      if (node.attribute(L"matrix") != nullptr)
        str = node.attribute(L"matrix").as_string();
      else
        str = node.text().as_string();

      if (!std::wstring(str).empty())
      {
        std::wstringstream input(str);
        input >> mat[0] >> mat[1] >> mat[2] >> mat[3] >>
                 mat[4] >> mat[5] >> mat[6] >> mat[7] >>
                 mat[8] >> mat[9] >> mat[10] >> mat[11] >>
                 mat[12] >> mat[13] >> mat[14] >> mat[15];
      }
    }
    return float4x4(mat);
  }




  void LoadFloatFromXMLNode(pugi::xml_node node, std::vector<float> &attrib, int a_matId, bool isShininess)
  {
    float value;
    if (isShininess)
      value = 256.0f;
    else
      value = 0.0f;
    if (node != nullptr)
    {
      const wchar_t *str = nullptr;

      if (node.attribute(L"val") != nullptr)
        str = node.attribute(L"val").as_string();
      else
        str = node.text().as_string();

      if (!std::wstring(str).empty())
      {
        value = std::stof(str);

        if (isShininess)
        {
          value = GetCosPowerFromGlossiness(value);
          value = sqrtf(value);
        }
      }
    }
    attrib[a_matId] = value;
  }


  void CreateRandomLights(int num, std::vector<float3> &pos, std::vector<float3> &color)
  {
    std::random_device rand;
    std::mt19937 rng(rand());
    std::uniform_real_distribution<float> xdist(-75.0f, 75.0f);
    std::uniform_real_distribution<float> ydist(-8.0f, 5.0f);
    std::uniform_real_distribution<float> zdist(-75.0f, 75.0f);
    std::uniform_real_distribution<float> colorDistrib(0.1f, 1.0f);


    for (int i = 0; i < num; i++)
    {
      float xPos = xdist(rng);
      float yPos = ydist(rng);
      float zPos = zdist(rng);
      pos.emplace_back(float3(xPos, yPos, zPos));

      float r = colorDistrib(rng);
      float g = colorDistrib(rng);
      float b = colorDistrib(rng);
      color.emplace_back(float3(r, g, b));
    }
  }

  void AssignRandomIESFiles(int num, std::vector<GLuint> &iesTextures)
  {
    iesTextures.resize(num);
    glGenTextures(num, &iesTextures[0]);

    std::vector<std::string> iesFileNames = {"data/ies/ies_5.ies", "data/ies/ies_10.ies", "data/ies/ies_11.ies",
                                             "data/ies/ies_12.ies", "data/ies/ies_16.ies", "data/ies/111621PN.IES",
                                             "data/ies/APWP3T8.IES"};

    for(int i = 0; i < num; ++i)
    {
      int j = i;
      if(i >= iesFileNames.size())
        j = i % iesFileNames.size();

      CreateGLSphereMapFromIES(iesFileNames.at(j), iesTextures.at(i));
    }
  }

  GLuint ChooseRandomTexture(std::vector<GLuint> &textures)
  {
    std::random_device rand;
    std::mt19937 rng(rand());
    std::uniform_int_distribution<unsigned int> gen(0, (unsigned int)textures.size());

    return gen(rng);
  }

  int CreateGLSphereMapFromIES(std::string filename, GLuint &texId)
  {
    enum IES_REFLECT
    {
      REFLECT4, REFLECT2, REFLECT0
    };

    oldies::IE_DATA iesData;
    memset(&iesData, 0, sizeof(oldies::IE_DATA));
    bool read = (oldies::IE_ReadFile((char *) filename.c_str(), &iesData) != 0);

    if (!read || iesData.photo.vert_angles == nullptr)
    {
      std::cerr << "IE_ReadFile error" << std::endl;
      texId = 0;
      return 1;
    }

    int w = 0; // phi, 0-360
    int h = 0; // theta, 0-180

    float verticalStart = iesData.photo.vert_angles[0];
    float verticalEnd = iesData.photo.vert_angles[iesData.photo.num_vert_angles - 1]; // 0-90, 90-180, 0-180

    float horizontStart = iesData.photo.horz_angles[0];
    float horizontEnd = iesData.photo.horz_angles[iesData.photo.num_horz_angles -
                                                  1];   // 0-0, 0-90, 0-180, 180-360, 0-360

    float eps = 1e-5f;

    //
    //
    if (fabs(verticalStart) < eps && fabs(verticalEnd - 90.0f) < eps) // 0-90
      h = iesData.photo.num_vert_angles * 2;
    else if (fabs(verticalStart - 90.0f) < eps && fabs(verticalEnd - 180.0f) < eps) // 90-180
      h = iesData.photo.num_vert_angles * 2;
    else // 0-180
      h = iesData.photo.num_vert_angles;

    IES_REFLECT reflectType = REFLECT0;

    // The set of horizontal angles, listed in increasing order.The first angle must be 0°.
    // The last angle determines the degree of lateral symmetry displayed by the intensity distribution.If it is 0°, the distribution is axially symmetric.
    // If it is 90°, the distribution is symmetric in each quadrant.
    // If it is 180°, the distribution is symmetric about a vertical plane.
    // If it is greater than 180° and less than or equal to 360°, the distribution exhibits no lateral symmetries.
    // All other values are invalid.
    //
    if (fabs(horizontStart) < eps && fabs(horizontEnd) < eps) // 0-0
      w = 1;
    else if (fabs(horizontStart) < eps &&
             fabs(horizontEnd - 90.0f) < eps) // 0-90, If it is 90°, the distribution is symmetric in each quadrant.
    {
      w = iesData.photo.num_horz_angles * 4;
      reflectType = REFLECT4;
    } else if (fabs(horizontStart) < eps && fabs(horizontEnd - 180.0f) <
                                            eps) // 0-180,  If it is 180°, the distribution is symmetric about a vertical plane.
    {
      w = iesData.photo.num_horz_angles * 4;
      reflectType = REFLECT4;
    } else if (fabs(horizontStart - 90.0f) < eps && fabs(horizontEnd - 180.0f) < eps) // 90-180
    {
      w = iesData.photo.num_horz_angles * 2;
      reflectType = REFLECT2;
    } else // if (fabs(horizontStart) < eps && fabs(horizontEnd - 360.0f) < eps) // 0-360
      w = iesData.photo.num_horz_angles;

    if (horizontEnd > 180.0f && horizontEnd < 360.0f) // crappy values like 357.0
      horizontEnd = 360.0f;


    std::vector<float> resultData(w * h);
    for (auto i = 0; i < resultData.size(); i++)
      resultData[i] = 0.0f;

    // init initial data
    //
    float stepTheta = (verticalEnd - verticalStart) / float(iesData.photo.num_vert_angles);
    float stepPhi = (horizontEnd - horizontStart) / float(iesData.photo.num_horz_angles);
    if (fabs(stepPhi) < eps)
      stepPhi = 360.0f;

    int thetaIndex = 0;
    for (float thetaGrad = verticalStart; (thetaIndex <
                                           iesData.photo.num_vert_angles); thetaGrad += stepTheta, thetaIndex++)
    {
      float theta = DEG_TO_RAD * thetaGrad;
      int iY = int((theta * INV_PI) * float(h) + 0.5f);
      if (iY >= h)
        iY = h - 1;

      int phiIndex = 0;
      for (float phiGrad = horizontStart; (phiIndex <
                                           iesData.photo.num_horz_angles); phiGrad += stepPhi, phiIndex++)
      {
        float phi = DEG_TO_RAD * phiGrad;
        int iX = int((phi * 0.5f * INV_PI) * float(w) + 0.5f);
        if (iX >= w)
          iX = w - 1;

        resultData[iY*w + iX] = iesData.photo.pcandela[phiIndex][thetaIndex];
      }
    }

    // reflect the rest of surrounding sphere
    //
    if (reflectType == REFLECT4)
    {
      for (float thetaGrad = verticalStart; (thetaGrad <= verticalEnd); thetaGrad += stepTheta)
      {
        float theta = DEG_TO_RAD * thetaGrad;
        int iY = int((theta * INV_PI) * float(h) + 0.5f);
        if (iY >= h)
          iY = h - 1;

        for (float phiGrad = 0.0f; (phiGrad <= 90.0f); phiGrad += stepPhi)
        {
          float phi = DEG_TO_RAD * (phiGrad);
          float phi2 = DEG_TO_RAD * (180.0f - phiGrad - stepPhi);
          float phi3 = DEG_TO_RAD * (180.0f + phiGrad);
          float phi4 = DEG_TO_RAD * (360.0f - phiGrad - stepPhi);

          int iX1 = int((phi * 0.5f * INV_PI) * float(w) + 0.5f);
          int iX2 = int((phi2 * 0.5f * INV_PI) * float(w) + 0.5f);
          int iX3 = int((phi3 * 0.5f * INV_PI) * float(w) + 0.5f);
          int iX4 = int((phi4 * 0.5f * INV_PI) * float(w) + 0.5f);

          if (iX1 >= w) iX1 = w - 1;
          if (iX2 >= w) iX2 = w - 1;
          if (iX3 >= w) iX3 = w - 1;
          if (iX4 >= w) iX4 = w - 1;

          resultData[iY * w + iX2] = resultData[iY * w + iX1];
          resultData[iY * w + iX3] = resultData[iY * w + iX1];
          resultData[iY * w + iX4] = resultData[iY * w + iX1];
        }
      }
    } else if (reflectType == REFLECT2)
    {

      for (float thetaGrad = verticalStart; (thetaGrad <= verticalEnd); thetaGrad += stepTheta)
      {
        float theta = DEG_TO_RAD * thetaGrad;
        int iY = int((theta * INV_PI) * float(h) + 0.5f);
        if (iY >= h)
          iY = h - 1;

        for (float phiGrad = 0.0f; (phiGrad <= 180.0f); phiGrad += stepPhi)
        {
          float phi = DEG_TO_RAD * phiGrad;
          float phi2 = DEG_TO_RAD * (360.0f - phiGrad - stepPhi);

          int iX1 = int((phi * 0.5f * INV_PI) * float(w) + 0.5f);
          int iX2 = int((phi2 * 0.5f * INV_PI) * float(w) + 0.5f);

          if (iX1 >= w) iX1 = w - 1;
          if (iX2 >= w) iX2 = w - 1;

          resultData[iY * w + iX2] = resultData[iY * w + iX1];
        }
      }
    }

    oldies::IE_Flush(&iesData); // release resources

    auto maxVal = *(std::max_element(resultData.begin(), resultData.end()));

    auto minVal = *(std::min_element(resultData.begin(), resultData.end()));

    //normalize intensity values to control it later with multiplier
    std::for_each(resultData.begin(), resultData.end(),
                  [=](float &x) { x = (x - minVal) / (maxVal - minVal); });

    std::vector<unsigned char> convertedData(resultData.size(), 0);

    for (int i = 0; i < resultData.size(); ++i)
      convertedData.at(i) = (unsigned char) (std::min<float>(float(1), resultData.at(i)) * 255);


    CreateTextureFromData(texId, w, h, 1, &convertedData[0], GL_LINEAR);

    return 0;
  }

  void GenerateSamplesInHemisphere(std::vector<float> &samples, int numSamples)
  {
    std::random_device rand;
    std::mt19937 rng(rand());
    std::uniform_real_distribution<float> smp(0.0f, 1.0f);

    for (int i = 0; i < numSamples; ++i)
    {
      float3 sample(smp(rng) * 2.0f - 1.0f, smp(rng) * 2.0f - 1.0f, smp(rng));
      sample = normalize(sample);
      sample *= smp(rng);
      float scale = float(i) / numSamples;
      scale = lerp(0.1f, 1.0f, scale * scale);
      sample *= scale;
      samples.push_back(sample.x);  
      samples.push_back(sample.y);
      samples.push_back(sample.z);
    }
  }

  void GenerateSamplesInHemisphere(std::vector<float3> &samples, int numSamples)
  {
    std::random_device rand;
    std::mt19937 rng(rand());
    std::uniform_real_distribution<float> smp(0.0f, 1.0f);

    for (int i = 0; i < numSamples; ++i)
    {
      float3 sample(smp(rng) * 2.0f - 1.0f, smp(rng) * 2.0f - 1.0f, smp(rng));
      sample = normalize(sample);
      sample *= smp(rng);
      float scale = float(i) / numSamples;
      scale = lerp(0.1f, 1.0f, scale * scale);
      sample *= scale;
      samples.push_back(sample);
    }
  }

  void GenerateRandom2DRotVectors(std::vector<float3> &samples, int numSamples)
  {
    std::random_device rand;
    std::mt19937 rng(rand());
    std::uniform_real_distribution<float> smp(0.0f, 1.0f);

    for (int i = 0; i < numSamples; i++)
    {
      float3 sample(smp(rng) * 2.0f - 1.0f, smp(rng) * 2.0f - 1.0f, 0.0f);
      samples.push_back(sample);
    }
  }

  GLuint CreateRandomRot2DVectorsTexture(int numSamples)
  {
    std::vector<float3> samples;
    GenerateRandom2DRotVectors(samples, numSamples);

    int size = int(std::sqrt(numSamples));

    GLuint texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, size, size, 0, GL_RGB, GL_FLOAT, &samples[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return texId;
  }

  GLuint CreateRandomSamples1DTexture(int numSamples)
  {
    std::vector<float3> samples;
    GenerateSamplesInHemisphere(samples, numSamples);

    GLuint texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_1D, texId);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB16F, numSamples, 0, GL_RGB, GL_FLOAT, &samples[0]);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAX_LEVEL, 0);

    return texId;
  }

  void CreateRandomSamplesTBO(int numSamples, GLuint &texId, GLuint &tbo)
  {
    std::vector<float> samples;
    GenerateSamplesInHemisphere(samples, numSamples);

    glGenBuffers(1, &tbo);
    glBindBuffer(GL_TEXTURE_BUFFER, tbo);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(float) * samples.size(), &samples[0], GL_STATIC_DRAW);

    glGenTextures(1, &texId);

    glBindBuffer(GL_TEXTURE_BUFFER, 0);
  }

  void CreateGeometryFromBatch(const HRBatchInfo &batch, const HRMeshDriverInput &a_input, std::vector<float> &a_pos,
                               std::vector<float> &a_norm, std::vector<float> &a_batchTangent, std::vector<float> &a_texcoords,
                               std::vector<int> &a_indices)
  {

    const int drawElementsNum = batch.triEnd - batch.triBegin;

    int newIndex = 0;
    std::unordered_map<int, int> indexMap;
    a_indices.reserve(drawElementsNum * 3);
    for (int32_t i = batch.triBegin; i < batch.triEnd; ++i) //convert from mesh indices to batch indices
    {
      int A = a_input.indices[i * 3 + 0];
      int B = a_input.indices[i * 3 + 1];
      int C = a_input.indices[i * 3 + 2];

      if (indexMap.find(A) == indexMap.end())
      {
        indexMap[A] = newIndex;
        a_indices.push_back(newIndex);
        newIndex++;
      } else
      {
        a_indices.push_back(indexMap[A]);
      }
      if (indexMap.find(B) == indexMap.end())
      {
        indexMap[B] = newIndex;
        a_indices.push_back(newIndex);
        newIndex++;
      } else
      {
        a_indices.push_back(indexMap[B]);
      }
      if (indexMap.find(C) == indexMap.end())
      {
        indexMap[C] = newIndex;
        a_indices.push_back(newIndex);
        newIndex++;
      } else
      {
        a_indices.push_back(indexMap[C]);
      }
    }

    const int newUniqueIndexNum = int(indexMap.size());
    a_pos.resize(newUniqueIndexNum * 4, 0.0f);
    a_norm.resize(newUniqueIndexNum * 4, 0.0f);
    a_batchTangent.resize(newUniqueIndexNum * 4, 0.0f);
    a_texcoords.resize(newUniqueIndexNum * 2, 0.0f);

    for (int32_t i = batch.triBegin; i < batch.triEnd; ++i)
    {
      int A = a_input.indices[i * 3 + 0];
      int B = a_input.indices[i * 3 + 1];
      int C = a_input.indices[i * 3 + 2];

      int newA = indexMap[a_input.indices[i * 3 + 0]];
      int newB = indexMap[a_input.indices[i * 3 + 1]];
      int newC = indexMap[a_input.indices[i * 3 + 2]];

      a_pos.at(newA * 4 + 0) = (a_input.pos4f[A * 4 + 0]);
      a_pos.at(newA * 4 + 1) = (a_input.pos4f[A * 4 + 1]);
      a_pos.at(newA * 4 + 2) = (a_input.pos4f[A * 4 + 2]);
      a_pos.at(newA * 4 + 3) = (a_input.pos4f[A * 4 + 3]);

      a_pos.at(newB * 4 + 0) = (a_input.pos4f[B * 4 + 0]);
      a_pos.at(newB * 4 + 1) = (a_input.pos4f[B * 4 + 1]);
      a_pos.at(newB * 4 + 2) = (a_input.pos4f[B * 4 + 2]);
      a_pos.at(newB * 4 + 3) = (a_input.pos4f[B * 4 + 3]);

      a_pos.at(newC * 4 + 0) = (a_input.pos4f[C * 4 + 0]);
      a_pos.at(newC * 4 + 1) = (a_input.pos4f[C * 4 + 1]);
      a_pos.at(newC * 4 + 2) = (a_input.pos4f[C * 4 + 2]);
      a_pos.at(newC * 4 + 3) = (a_input.pos4f[C * 4 + 3]);


      a_norm.at(newA * 4 + 0) = (a_input.norm4f[A * 4 + 0]);
      a_norm.at(newA * 4 + 1) = (a_input.norm4f[A * 4 + 1]);
      a_norm.at(newA * 4 + 2) = (a_input.norm4f[A * 4 + 2]);
      a_norm.at(newA * 4 + 3) = (a_input.norm4f[A * 4 + 3]);

      a_norm.at(newB * 4 + 0) = (a_input.norm4f[B * 4 + 0]);
      a_norm.at(newB * 4 + 1) = (a_input.norm4f[B * 4 + 1]);
      a_norm.at(newB * 4 + 2) = (a_input.norm4f[B * 4 + 2]);
      a_norm.at(newB * 4 + 3) = (a_input.norm4f[B * 4 + 3]);

      a_norm.at(newC * 4 + 0) = (a_input.norm4f[C * 4 + 0]);
      a_norm.at(newC * 4 + 1) = (a_input.norm4f[C * 4 + 1]);
      a_norm.at(newC * 4 + 2) = (a_input.norm4f[C * 4 + 2]);
      a_norm.at(newC * 4 + 3) = (a_input.norm4f[C * 4 + 3]);


      a_batchTangent.at(newA * 4 + 0) = (a_input.tan4f[A * 4 + 0]);
      a_batchTangent.at(newA * 4 + 1) = (a_input.tan4f[A * 4 + 1]);
      a_batchTangent.at(newA * 4 + 2) = (a_input.tan4f[A * 4 + 2]);
      a_batchTangent.at(newA * 4 + 3) = (a_input.tan4f[A * 4 + 3]);

      a_batchTangent.at(newB * 4 + 0) = (a_input.tan4f[B * 4 + 0]);
      a_batchTangent.at(newB * 4 + 1) = (a_input.tan4f[B * 4 + 1]);
      a_batchTangent.at(newB * 4 + 2) = (a_input.tan4f[B * 4 + 2]);
      a_batchTangent.at(newB * 4 + 3) = (a_input.tan4f[B * 4 + 3]);

      a_batchTangent.at(newC * 4 + 0) = (a_input.tan4f[C * 4 + 0]);
      a_batchTangent.at(newC * 4 + 1) = (a_input.tan4f[C * 4 + 1]);
      a_batchTangent.at(newC * 4 + 2) = (a_input.tan4f[C * 4 + 2]);
      a_batchTangent.at(newC * 4 + 3) = (a_input.tan4f[C * 4 + 3]);


      a_texcoords.at(newA * 2 + 0) = (a_input.texcoord2f[A * 2 + 0]);
      a_texcoords.at(newA * 2 + 1) = (a_input.texcoord2f[A * 2 + 1]);

      a_texcoords.at(newB * 2 + 0) = (a_input.texcoord2f[B * 2 + 0]);
      a_texcoords.at(newB * 2 + 1) = (a_input.texcoord2f[B * 2 + 1]);

      a_texcoords.at(newC * 2 + 0) = (a_input.texcoord2f[C * 2 + 0]);
      a_texcoords.at(newC * 2 + 1) = (a_input.texcoord2f[C * 2 + 1]);
    }
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

  GBuffer::GBuffer(GLsizei width, GLsizei height) : width(width), height(height)
  {
    glGenFramebuffers(1, &frameBufferObject);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);

    for (int i = 0; i < GBUF_NUM_TEXTURES; ++i)
    {
      if (i == GBUF_REFLECTION)
        renderTextures[i] = CreateEmptyTex(GL_RGBA, GL_RGBA16F, width, height);
      else
        renderTextures[i] = CreateEmptyTex(GL_RGB, GL_RGB16F, width, height);

      glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, renderTextures[i], 0);
    }

    finalTex = CreateEmptyTex(GL_RGBA, GL_RGBA16F, width, height);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + GBUF_NUM_TEXTURES, finalTex, 0);

    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, width, height, 0, GL_DEPTH_STENCIL,
                 GL_FLOAT_32_UNSIGNED_INT_24_8_REV, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);


    GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
      std::cout << "Framebuffer is not complete" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void GBuffer::ResizeAttachments(GLsizei width, GLsizei height)
  {
    for (int i = 0; i < GBUF_NUM_TEXTURES; ++i)
    {
      glBindTexture(GL_TEXTURE_2D, renderTextures[i]);
      if (i == GBUF_REFLECTION)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
      else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    }

    glBindTexture(GL_TEXTURE_2D, finalTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, width, height, 0, GL_DEPTH_STENCIL,
                 GL_FLOAT_32_UNSIGNED_INT_24_8_REV, NULL);
  }


  void GBuffer::FrameInit()
  {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBufferObject);
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + GBUF_NUM_TEXTURES); //finalTex
    glClear(GL_COLOR_BUFFER_BIT);
  }

  GBuffer::~GBuffer()
  {
    glDeleteTextures(GBUF_NUM_TEXTURES, renderTextures);
    glDeleteTextures(1, &depthTex);
    for (int i = 0; i < GBUF_NUM_TEXTURES; ++i)
      renderTextures[i] = GLuint(-1);

    glDeleteFramebuffers(1, &frameBufferObject);
  }

  void GBuffer::StartGeoPass()
  {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBufferObject);

    GLenum attachments[GBUF_NUM_TEXTURES];

    for (int i = 0; i < GBUF_NUM_TEXTURES; ++i)
      attachments[i] = GL_COLOR_ATTACHMENT0 + i;

    glDrawBuffers(GBUF_NUM_TEXTURES, attachments);
  }

  void GBuffer::StartFinalPass(const GLuint &targetFBO)
  {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBufferObject);
    glReadBuffer(GL_COLOR_ATTACHMENT0 + GBUF_NUM_TEXTURES);
  }

  void GBuffer::StartLightPass()
  {
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + GBUF_NUM_TEXTURES);
  }

  void GBuffer::EndRendering()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  GLuint GBuffer::GetTextureId(GBUF_TEXTURE_TYPE type)
  {
    if (type == GBUF_NUM_TEXTURES)
      return finalTex;
    else
      return renderTextures[type];
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




  SSAOBuffer::SSAOBuffer(GLsizei width, GLsizei height, int samples, float radius) : width(width), height(height), numSamples(samples), radius(radius)
  {

    std::unordered_map<GLenum, std::string> ssaoShaders;
    ssaoShaders[GL_VERTEX_SHADER] = "../glsl/vSSAO.vert";
    ssaoShaders[GL_FRAGMENT_SHADER] = "../glsl/fSSAO.frag";
    ssaoProgram = ShaderProgram(ssaoShaders);

    std::unordered_map<GLenum, std::string> ssaoBlurShaders;
    ssaoBlurShaders[GL_VERTEX_SHADER] = "../glsl/vSSAOBlur.vert";
    ssaoBlurShaders[GL_FRAGMENT_SHADER] = "../glsl/fSSAOBlur.frag";
    ssaoBlurProgram = ShaderProgram(ssaoBlurShaders);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    renderTextures[0] = CreateEmptyTex(GL_RGB, GL_RED, width, height);
    GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_RED };
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderTextures[0], 0);
    GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
      std::cout << "Framebuffer is not complete" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &fboBlur);
    glBindFramebuffer(GL_FRAMEBUFFER, fboBlur);

    renderTextures[1] = CreateEmptyTex(GL_RGB, GL_RED, width, height);
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderTextures[1], 0);
    fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
      std::cout << "Framebuffer is not complete" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //mplesTexture = CreateRandomSamples1DTexture(64);
    rotTexture = CreateRandomRot2DVectorsTexture(16);

    CreateRandomSamplesTBO(64, samplesTexture, tbo);

    GLuint uboIndex = glGetUniformBlockIndex(ssaoProgram.GetProgram(), "ssaoSettingsBuffer");
  
    glUniformBlockBinding(ssaoProgram.GetProgram(), uboIndex, 3);

    GLsizei settingsUBOSize = 3 * sizeof(GLint) + sizeof(GLfloat);


    struct settings
    {
      int2 screenSize;
      int kernelSize;
      float radius;
    };

    settings s;
    s.screenSize = int2(width, height);
    s.kernelSize = numSamples;
    s.radius = radius;

    glGenBuffers(1, &settingsUBO);

    glBindBuffer(GL_UNIFORM_BUFFER, settingsUBO);
    glBufferData(GL_UNIFORM_BUFFER, settingsUBOSize, &s, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 3, settingsUBO, 0, settingsUBOSize);

  }

  SSAOBuffer::~SSAOBuffer()
  {
    glDeleteTextures(SSAO_NUM_TEXTURES, renderTextures);
    for (int i = 0; i < SSAO_NUM_TEXTURES; ++i)
      renderTextures[i] = GLuint(-1);

    glDeleteTextures(1, &samplesTexture);
    samplesTexture = GLuint(-1);

    glDeleteTextures(1, &rotTexture);
    rotTexture = GLuint(-1);

    glDeleteFramebuffers(1, &fbo);
    glDeleteFramebuffers(1, &fboBlur);

    ssaoProgram.Release();
    ssaoBlurProgram.Release();
  }

  GLuint SSAOBuffer::GetTextureId(SSAO_TEXTURE_TYPE type)
  {
    return renderTextures[type];
  }


  void SSAOBuffer::StartSSAOPass(const float4x4 &projection, const GLuint positionTex, const GLuint normalTex)
  {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClear(GL_COLOR_BUFFER_BIT);
    ssaoProgram.StartUseShader();
    //ssaoProgram.SetUniform("projection", projection);
  /*  ssaoProgram.SetUniform("kernelSize", numSamples);
    ssaoProgram.SetUniform("radius", radius);
    ssaoProgram.SetUniform("screenSize", int2(width, height));*/
    bindTexture(ssaoProgram, 0, "texNoise", rotTexture);
    bindTextureBuffer(ssaoProgram, 1, "randomSamples", tbo, samplesTexture);
    bindTexture(ssaoProgram, 2, "gVertex", positionTex);
    bindTexture(ssaoProgram, 3, "gNormal", normalTex);
  }

  void SSAOBuffer::StartSSAOBlurPass()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, fboBlur);
    glClear(GL_COLOR_BUFFER_BIT);
    ssaoBlurProgram.StartUseShader();
    bindTexture(ssaoBlurProgram, 0, "ssaoRaw", renderTextures[SSAOBuffer::SSAO_RAW]);
  }

  void SSAOBuffer::EndSSAOPass()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //ssaoProgram.StopUseShader();
  }

  void SSAOBuffer::EndSSAOBlurPass()
  {
    //ssaoBlurProgram.StopUseShader();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void SSAOBuffer::ResizeAttachments(GLsizei width, GLsizei height)
  {
    for (int i = 0; i < SSAO_NUM_TEXTURES; ++i)
    {
      glBindTexture(GL_TEXTURE_2D, renderTextures[i]);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    }

    int2 screenSize(width, height);

    glBindBuffer(GL_UNIFORM_BUFFER, settingsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER,0, 2 * sizeof(GLint), &screenSize);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
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