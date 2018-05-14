#include "tests.h"
#include <math.h>
#include <fstream>
#include <vector>
#include <string.h>
#include <sstream>
#include "LiteMath.h"

#include "../hydra_api/HR_HDRImageTool.h"
using HDRImage4f = HydraRender::HDRImage4f;
using HydraRender::LoadImageFromFile;
using HydraRender::SaveImageToFile;
using namespace HydraLiteMath;

#pragma warning(disable:4838)

namespace TEST_UTILS
{
  
  void show_me_texture_ldr(const std::string& a_inFleName, const std::string& a_outFleName)
  {
    int32_t wh[2];

    std::ifstream fin(a_inFleName, std::iostream::binary);
    fin.read((char*)wh, sizeof(int) * 2);

    if (wh[0] <= 0 || wh[1] <= 0)
      return;

    std::vector<uint32_t> sdata(wh[0] * wh[1]);
    fin.read((char*)&sdata[0], sdata.size() * sizeof(uint32_t));
    fin.close();

    SaveImageToFile(a_outFleName, wh[0], wh[1], &sdata[0]);
  }

  void show_me_texture_hdr(const std::string& a_inFleName, const std::string& a_outFleName)
  {
    int32_t wh[2];

    std::ifstream fin(a_inFleName, std::iostream::binary);
    fin.read((char*)wh, sizeof(int) * 2);

    if (wh[0] <= 0 || wh[1] <= 0)
      return;

    HDRImage4f colorImg(wh[0], wh[1]);
    fin.read((char*)colorImg.data(), wh[0] * wh[1] * 4 * sizeof(float));
    fin.close();

    SaveImageToFile(a_outFleName, colorImg, 2.2f);
  }

  std::vector<unsigned int> CreateStripedImageData(unsigned int* a_colors, int a_stripsNum, int w, int h)
  {
    std::vector<unsigned int> imageData(w*h);

    int currH = 0;
    int strideH = (h / a_stripsNum);

    for (int stripId = 0; stripId < a_stripsNum; stripId++)
    {
      unsigned int color = a_colors[stripId];

#pragma omp parallel for
      for (int y = currH; y < currH + strideH; y++)
      {
        for (int x = 0; x < w; x++)
          imageData[y*w + x] = color;
      }

      currH += strideH;
    }

    return imageData;
  }


  HRTextureNodeRef AddRandomTextureFromMemory(size_t& memTotal)
  {
    int choice = rand() % 3;

    int w = rand() % 2048 + 1 + 128;
    int h = rand() % 2048 + 1 + 128;

    if (choice == 0)      // add LDR texture
    {
      std::vector<int> data(w*h);
      for (size_t i = 0; i < data.size(); i++)
        data[i] = 0xFFFF00FF;

      memTotal += w*h * 4;
      return hrTexture2DCreateFromMemory(w, h, 4, &data[0]);
    }
    else if (choice == 1)      // add LDR texture
    {
      std::vector<int> data(w*h);
      for (size_t i = 0; i < data.size(); i++)
        data[i] = 0xFF7F0060;

      memTotal += w*h * 4;
      return hrTexture2DCreateFromMemory(w, h, 4, &data[0]);
    }
    else // add HDR texture
    {
      std::vector<int> data(w*h);
      for (size_t i = 0; i < data.size(); i++)
        data[i] = 0xFFA070F0;

      memTotal += w*h * 4;
      return hrTexture2DCreateFromMemory(w, h, 4, &data[0]);
    }
  }

  static inline float clamp(float u, float a, float b) { return fminf(fmaxf(a, u), b); }

  void procTexCheckerLDR(unsigned char* a_buffer, int w, int h, void* a_repeat)
  {
    if (a_repeat == nullptr)
      return;

    int* data = (int*)a_repeat;
    int repeats = *data;

    repeats *= 2;

#pragma omp parallel for
    for (int y = 0; y < h; y++)
    {
      for (int x = 0; x < w; x++)
      {
        float cx = floor(repeats * (float(x) / w));
        float cy = floor(repeats * (float(y) / h));
        float result = fmod(cx + cy, 2.0f);
        clamp(result, 0.0, 1.0);

        a_buffer[(y*w + x) * 4 + 0] = (unsigned char)(result * 255);
        a_buffer[(y*w + x) * 4 + 1] = (unsigned char)(result * 255);
        a_buffer[(y*w + x) * 4 + 2] = (unsigned char)(result * 255);
        a_buffer[(y*w + x) * 4 + 3] = 255;
      }
    }
  }

  void procTexCheckerHDR(float* a_buffer, int w, int h, void* a_repeat)
  {
    if (a_repeat == nullptr)
      return;

    int* data = (int*)a_repeat;
    int repeats = *data;

    repeats *= 2;

#pragma omp parallel for
    for (int y = 0; y < h; y++)
    {
      for (int x = 0; x < w; x++)
      {
        float cx = floor(repeats * (float(x) / w));
        float cy = floor(repeats * (float(y) / h));
        float result = fmod(cx + cy, 2.0f);
        if (result < 0.001f)
          result = 0.0f;
        else
          result = 10.0f;

        a_buffer[(y*w + x) * 4 + 0] = result;
        a_buffer[(y*w + x) * 4 + 1] = result;
        a_buffer[(y*w + x) * 4 + 2] = result;
        a_buffer[(y*w + x) * 4 + 3] = 1.0;
      }
    }
  }

  void CreateTestBigTexturesFilesIfNeeded()
  {
    const int TXSZ = 4096;

    if (!FileExists("data/textures_gen/texture_big_red.png"))
    {
      std::cout << "creating test texture and saving it to disk ... " << std::endl;
      unsigned int color = 0xFF0000FF;
      CreateStripedImageFile("data/textures_gen/texture_big_red.png", &color, 1, TXSZ, TXSZ);
    }

    if (!FileExists("data/textures_gen/texture_green.png"))
    {
      std::cout << "creating test texture and saving it to disk ... " << std::endl;
      unsigned int color = 0xFF00FF00;
      CreateStripedImageFile("data/textures_gen/texture_green.png", &color, 1, TXSZ, TXSZ);
    }

    if (!FileExists("data/textures_gen/texture_blue.png"))
    {
      std::cout << "creating test texture and saving it to disk ... " << std::endl;
      unsigned int color = 0xFFFF0000;
      CreateStripedImageFile("data/textures_gen/texture_blue.png", &color, 1, TXSZ, TXSZ);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if (!FileExists("data/textures_gen/texture_big_z01.png"))
    {
      std::cout << "creating test texture and saving it to disk ... " << std::endl;
      unsigned int colors[2] = { 0xFF0000FF, 0xFF00FF00 };
      CreateStripedImageFile("data/textures_gen/texture_big_z01.png", colors, 2, TXSZ, TXSZ);
    }

    if (!FileExists("data/textures_gen/texture_big_z02.png"))
    {
      std::cout << "creating test texture and saving it to disk ... " << std::endl;
      unsigned int colors[2] = { 0xFF0000FF, 0xFFFF0000 };
      CreateStripedImageFile("data/textures_gen/texture_big_z02.png", colors, 2, TXSZ, TXSZ);
    }

    if (!FileExists("data/textures_gen/texture_big_z03.png"))
    {
      std::cout << "creating test texture and saving it to disk ... " << std::endl;
      unsigned int colors[2] = { 0xFF00FFFF, 0xFF0000FF };
      CreateStripedImageFile("data/textures_gen/texture_big_z03.png", colors, 2, TXSZ, TXSZ);
    }

    if (!FileExists("data/textures_gen/texture_big_z04.png"))
    {
      std::cout << "creating test texture and saving it to disk ... " << std::endl;
      unsigned int colors[2] = { 0xFFFF0000, 0xFF00FF00 };
      CreateStripedImageFile("data/textures_gen/texture_big_z04.png", colors, 2, TXSZ, TXSZ);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if (!FileExists("data/textures_gen/texture_big_z05.png"))
    {
      std::cout << "creating test texture and saving it to disk ... " << std::endl;
      unsigned int colors[4] = { 0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFFFF00FF };
      CreateStripedImageFile("data/textures_gen/texture_big_z05.png", colors, 4, TXSZ, TXSZ);
    }

    if (!FileExists("data/textures_gen/texture_big_z06.png"))
    {
      std::cout << "creating test texture and saving it to disk ... " << std::endl;
      unsigned int colors[4] = { 0xFF00FF00, 0xFFFF0000, 0xFFFF00FF, 0xFFFFFFFF };
      CreateStripedImageFile("data/textures_gen/texture_big_z06.png", colors, 4, TXSZ, TXSZ);
    }

    if (!FileExists("data/textures_gen/texture_big_z07.png"))
    {
      std::cout << "creating test texture and saving it to disk ... " << std::endl;
      unsigned int colors[4] = { 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF, 0xFF000000 };
      CreateStripedImageFile("data/textures_gen/texture_big_z07.png", colors, 4, TXSZ, TXSZ);
    }
  }

  HRTextureNodeRef CreateRandomStrippedTextureFromMemory(size_t& a_byteSize)
  {
    int TXSZ = 1024;
    int choice1 = rand() % 3;
    if (choice1 == 0)
    {
      a_byteSize += size_t(2048 * 2048 * 4);
      TXSZ = 2048;
    }
    else if (choice1 == 2)
    {
      a_byteSize += size_t(512 * 512 * 4);
      TXSZ = 512;
    }
    else
    {
      a_byteSize += size_t(1024 * 1024 * 4);
      TXSZ = 1024;
    }

    int choice = rand() % 10;

    if (choice == 0)
    {
      unsigned int color = 0xFF0000FF;
      auto data = CreateStripedImageData(&color, 1, TXSZ, TXSZ);
      return hrTexture2DCreateFromMemory(TXSZ, TXSZ, 4, &data[0]);
    }
    else if (choice == 1)
    {
      unsigned int color = 0xFF00FF00;
      auto data = CreateStripedImageData(&color, 1, TXSZ, TXSZ);
      return hrTexture2DCreateFromMemory(TXSZ, TXSZ, 4, &data[0]);
    }
    else if (choice == 2)
    {
      unsigned int color = 0xFFFF0000;
      auto data = CreateStripedImageData(&color, 1, TXSZ, TXSZ);
      return hrTexture2DCreateFromMemory(TXSZ, TXSZ, 4, &data[0]);
    }
    else if (choice == 3)
    {
      unsigned int colors[2] = { 0xFF0000FF, 0xFF00FF00 };
      auto data = CreateStripedImageData(colors, 2, TXSZ, TXSZ);
      return hrTexture2DCreateFromMemory(TXSZ, TXSZ, 4, &data[0]);
    }
    else if (choice == 4)
    {
      unsigned int colors[2] = { 0xFF0000FF, 0xFFFF0000 };
      auto data = CreateStripedImageData(colors, 2, TXSZ, TXSZ);
      return hrTexture2DCreateFromMemory(TXSZ, TXSZ, 4, &data[0]);
    }
    else if (choice == 5)
    {
      unsigned int colors[2] = { 0xFF00FFFF, 0xFF0000FF };
      auto data = CreateStripedImageData(colors, 2, TXSZ, TXSZ);
      return hrTexture2DCreateFromMemory(TXSZ, TXSZ, 4, &data[0]);
    }
    else if (choice == 6)
    {
      unsigned int colors[2] = { 0xFFFF0000, 0xFF00FF00 };
      auto data = CreateStripedImageData(colors, 2, TXSZ, TXSZ);
      return hrTexture2DCreateFromMemory(TXSZ, TXSZ, 4, &data[0]);
    }
    else if (choice == 7)
    {
      unsigned int colors[4] = { 0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFFFF00FF };
      auto data = CreateStripedImageData(colors, 4, TXSZ, TXSZ);
      return hrTexture2DCreateFromMemory(TXSZ, TXSZ, 4, &data[0]);
    }
    else if (choice == 8)
    {
      unsigned int colors[4] = { 0xFF00FF00, 0xFFFF0000, 0xFFFF00FF, 0xFFFFFFFF };
      auto data = CreateStripedImageData(colors, 4, TXSZ, TXSZ);
      return hrTexture2DCreateFromMemory(TXSZ, TXSZ, 4, &data[0]);
    }
    else
    {
      unsigned int colors[4] = { 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF, 0xFF000000 };
      auto data = CreateStripedImageData(colors, 4, TXSZ, TXSZ);
      return hrTexture2DCreateFromMemory(TXSZ, TXSZ, 4, &data[0]);
    }
  }

  bool FileExists(const char* a_fileName)
  {
    std::ifstream fin(a_fileName);
    bool res = fin.is_open();
    fin.close();
    return res;
  }

  void CreateStripedImageFile(const char* a_fileName, unsigned int* a_colors, int a_stripsNum, int w, int h)
  {
    std::vector<unsigned int> imageData = CreateStripedImageData(a_colors, a_stripsNum, w, h);
    SaveImageToFile(a_fileName, w, h, (unsigned int*)&imageData[0]);
  }

  std::vector<HRMeshRef> CreateRandomMeshesArray(int a_size)
  {
    std::vector<HRMeshRef> meshes(a_size);

    for (size_t i = 0; i < meshes.size(); i++)
    {
      int choice = rand() % 3;

      if (choice == 0)
        meshes[i] = HRMeshFromSimpleMesh(L"my_cube", CreateCube(0.5f), rand() % 50);
      else if (choice == 1)
        meshes[i] = HRMeshFromSimpleMesh(L"my_sphere", CreateSphere(0.5f, 128), rand() % 50);
      else
        meshes[i] = HRMeshFromSimpleMesh(L"my_torus2", CreateTorus(0.2f, 0.5f, 128, 128), rand() % 50);

      if (i % 20 == 0)
        std::cout << "[test_mbm]: MB, total meshes = " << i << "\r";
    }

    std::cout << std::endl;

    return meshes;
  }

  HRMeshRef HRMeshFromSimpleMesh(const wchar_t* a_name, const SimpleMesh& a_mesh, int a_matId)
  {
    HRMeshRef meshRef = hrMeshCreate(a_name);

    hrMeshOpen(meshRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(meshRef, L"pos", &a_mesh.vPos[0]);
      hrMeshVertexAttribPointer4f(meshRef, L"norm", &a_mesh.vNorm[0]);
      hrMeshVertexAttribPointer2f(meshRef, L"texcoord", &a_mesh.vTexCoord[0]);

      hrMeshMaterialId(meshRef, a_matId);
      hrMeshAppendTriangles3(meshRef, int(a_mesh.triIndices.size()), &a_mesh.triIndices[0]);
    }
    hrMeshClose(meshRef);

    return meshRef;
  }


  HRRenderRef CreateBasicTestRenderPT(int deviceId, int w, int h, int minRays, int maxRays, const wchar_t* a_drvName)
  {
    HRRenderRef renderRef = hrRenderCreate(a_drvName);
    hrRenderEnableDevice(renderRef, deviceId, true);

    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      auto node = hrRenderParamNode(renderRef);

      node.append_child(L"width").text()  = w;
      node.append_child(L"height").text() = h;

      node.append_child(L"method_primary").text()   = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text()  = L"pathtracing";
      node.append_child(L"method_caustic").text()   = L"pathtracing";
      node.append_child(L"shadows").text()          = L"1";

      node.append_child(L"trace_depth").text()      = L"6";
      node.append_child(L"diff_trace_depth").text() = L"3";

      node.append_child(L"pt_error").text()        = 1.0f; // 1.0%
      node.append_child(L"minRaysPerPixel").text() = minRays;
      node.append_child(L"maxRaysPerPixel").text() = maxRays;

    }
    hrRenderClose(renderRef);

    return renderRef;
  }

  HRRenderRef CreateBasicTestRenderPTNoCaust(int deviceId, int w, int h, int minRays, int maxRays)
  {
    HRRenderRef renderRef = hrRenderCreate(L"HydraModern");
    hrRenderEnableDevice(renderRef, deviceId, true);

    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      auto node = hrRenderParamNode(renderRef);

      node.append_child(L"width").text() = w;
      node.append_child(L"height").text() = h;

      node.append_child(L"method_primary").text()   = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text()  = L"pathtracing";
      node.append_child(L"method_caustic").text()   = L"none";
      node.append_child(L"shadows").text() = L"1";

      node.append_child(L"trace_depth").text() = L"5";
      node.append_child(L"diff_trace_depth").text() = L"3";

      node.append_child(L"pt_error").text() = L"2";
      node.append_child(L"minRaysPerPixel").text() = minRays;
      node.append_child(L"maxRaysPerPixel").text() = maxRays;

    }
    hrRenderClose(renderRef);

    return renderRef;
  }


  HRMeshRef CreateTriStrip(int rows, int cols, float size)
  {
    //int numIndices = 2 * cols*(rows - 1) + rows - 1;

    std::vector<float> vertices_vec;
    vertices_vec.reserve(rows * cols * 4);

    std::vector<float> normals_vec;
    normals_vec.reserve(rows * cols * 4);

    std::vector<float> texcoords_vec;
    texcoords_vec.reserve(rows * cols * 2);

    std::vector<float3> normals_vec_tmp(rows * cols, float3(0.0f, 0.0f, 0.0f));

    std::vector<int> indices_vec;
    std::vector<int> mind_vec;

    for (int z = 0; z < rows; ++z)
    {
      for (int x = 0; x < cols; ++x)
      {
        float xx = -size / 2 + x*size / cols;
        float zz = -size / 2 + z*size / rows;
        float yy = 0.0f;
        //float r = sqrt(xx*xx + zz*zz);
        //float yy = 5.0f * (r != 0.0f ? sin(r) / r : 1.0f);

        vertices_vec.push_back(xx);
        vertices_vec.push_back(yy);
        vertices_vec.push_back(zz);
        vertices_vec.push_back(1.0f);

        texcoords_vec.push_back(x / float(cols - 1));
        texcoords_vec.push_back(z / float(rows - 1));
      }
    }

    int numTris = 0;
    for (int x = 0; x < cols - 1; ++x)
    {
      for (int z = 0; z < rows - 1; ++z)
      {
        unsigned int offset = x*cols + z;

        indices_vec.push_back(offset + 0);
        indices_vec.push_back(offset + rows);
        indices_vec.push_back(offset + 1);
        indices_vec.push_back(offset + rows);
        indices_vec.push_back(offset + rows + 1);
        indices_vec.push_back(offset + 1);

        float3 A(vertices_vec.at(4 * offset + 0), vertices_vec.at(4 * offset + 1),
                 vertices_vec.at(4 * offset + 2));
        float3 B(vertices_vec.at(4 * (offset + rows) + 0), vertices_vec.at(4 * (offset + rows) + 1),
                 vertices_vec.at(4 * (offset + rows) + 2));
        float3 C(vertices_vec.at(4 * (offset + 1) + 0), vertices_vec.at(4 * (offset + 1) + 1),
                 vertices_vec.at(4 * (offset + 1) + 2));
        float3 D(vertices_vec.at(4 * (offset + rows + 1) + 0), vertices_vec.at(4 * (offset + rows + 1) + 1),
                 vertices_vec.at(4 * (offset + rows + 1) + 2));

        float3 edge1B(normalize(B - A));
        float3 edge1C(normalize(C - A));

        float3 face_normal1 = cross(edge1B, edge1C);

        float3 edge2D(normalize(D - B));
        float3 edge2C(normalize(C - B));

        float3 face_normal2 = cross(edge2D, edge2C);

        normals_vec_tmp.at(offset) += face_normal1;
        normals_vec_tmp.at(offset + rows) += face_normal1 + face_normal2;
        normals_vec_tmp.at(offset + 1) += face_normal1 + face_normal2;
        normals_vec_tmp.at(offset + rows + 1) += face_normal2;

        numTris += 2;
      }
    }


    ///////////////////////
    for (auto& N : normals_vec_tmp)
    {
      N = normalize(N);
      normals_vec.push_back(N.x);
      normals_vec.push_back(N.y);
      normals_vec.push_back(N.z);
      normals_vec.push_back(1.0f);
    }

    ///////////////////////
    for(int i = 0; i < indices_vec.size() / 3; ++i)
    {
      mind_vec.push_back(i % 5 == 0 ? 0 : 1);
    }


    HRMeshRef meshRef = hrMeshCreate(L"tri_strip");

    hrMeshOpen(meshRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(meshRef, L"pos", &vertices_vec[0]);
      hrMeshVertexAttribPointer4f(meshRef, L"norm", &normals_vec[0]);
      hrMeshVertexAttribPointer2f(meshRef, L"texcoord", &texcoords_vec[0]);

      //hrMeshMaterialId(cubeRef, 0);
      hrMeshPrimitiveAttribPointer1i(meshRef, L"mind", &mind_vec[0]);
      hrMeshAppendTriangles3(meshRef, int(indices_vec.size()), &indices_vec[0]);
    }
    hrMeshClose(meshRef);

    return meshRef;
  }

}