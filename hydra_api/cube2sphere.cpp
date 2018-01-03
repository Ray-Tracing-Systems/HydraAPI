#include "HydraAPI.h"

#include <iomanip>
#include <cstring>
#include <stdlib.h>

#if defined(WIN32)
#include <FreeImage.h>
#include <GLFW/glfw3.h>
#pragma comment(lib, "glfw3dll.lib")
#pragma comment(lib, "FreeImage.lib")

#else
#include <FreeImage.h>
#include <GLFW/glfw3.h>
#endif


#include "../hydra_api/HR_HDRImageTool.h"
#include "../hydra_api/HydraXMLHelpers.h"

#pragma warning(disable:4996)
#pragma warning(disable:4838)
#pragma warning(disable:4244)

namespace HydraRender // for test purpose only
{
  void SaveImageToFile(const std::string& a_fileName, int w, int h, unsigned int* data); // for test purpose only
};

namespace HRUtils
{
  using HydraLiteMath::float4;
  using HydraLiteMath::float3;
  using HydraLiteMath::float2;

  static inline float myclamp(float u, float a, float b) { float r = fmax(a, u); return fmin(r, b); }

  static inline int RealColorToUint32(const float real_color[4])
  {
    float  r = myclamp(real_color[0] * 255.0f, 0.0f, 255.0f);
    float  g = myclamp(real_color[1] * 255.0f, 0.0f, 255.0f);
    float  b = myclamp(real_color[2] * 255.0f, 0.0f, 255.0f);
    float  a = myclamp(real_color[3] * 255.0f, 0.0f, 255.0f);

    unsigned char red = (unsigned char)r;
    unsigned char green = (unsigned char)g;
    unsigned char blue = (unsigned char)b;
    unsigned char alpha = (unsigned char)a;

    return red | (green << 8) | (blue << 16) | (alpha << 24);
  }

  struct uchar4
  {
    uchar4() :x(0), y(0), z(0), w(0) {}
    uchar4(unsigned char a, unsigned char b, unsigned char c, unsigned char d) : x(a), y(b), z(c), w(d) {}

    unsigned char x, y, z, w;
  };

  struct int4
  {
    int4() :x(0), y(0), z(0), w(0) {}
    int4(int a, int b, int c, int d) : x(a), y(b), z(c), w(d) {}

    int x, y, z, w;
  };

  static inline float4 read_array_uchar4(const uchar4* a_data, int offset)
  {
    const float mult = 0.003921568f; // (1.0f/255.0f);
    const uchar4 c0 = a_data[offset];
    return mult*float4((float)c0.x, (float)c0.y, (float)c0.z, (float)c0.w);
  }

  static inline int4 bilinearOffsets(const float ffx, const float ffy, const int w, const int h)
  {
    const int sx = (ffx > 0.0f) ? 1 : -1;
    const int sy = (ffy > 0.0f) ? 1 : -1;

    const int px = (int)(ffx);
    const int py = (int)(ffy);

    int px_w0, px_w1, py_w0, py_w1;

    // clamp tex coord
    //
    px_w0 = (px >= w) ? w - 1 : px;
    px_w1 = (px + 1 >= w) ? w - 1 : px + 1;

    px_w0 = (px_w0 < 0) ? 0 : px_w0;
    px_w1 = (px_w1 < 0) ? 0 : px_w1;

    // clamp tex coord
    //
    py_w0 = (py >= h) ? h - 1 : py;
    py_w1 = (py + 1 >= h) ? h - 1 : py + 1;

    py_w0 = (py_w0 < 0) ? 0 : py_w0;
    py_w1 = (py_w1 < 0) ? 0 : py_w1;

    const int offset0 = py_w0*w + px_w0;
    const int offset1 = py_w0*w + px_w1;
    const int offset2 = py_w1*w + px_w0;
    const int offset3 = py_w1*w + px_w1;

    return int4(offset0, offset1, offset2, offset3);
  }


  static inline float4 read_imagef_sw4(const float2 a_texCoord, const int* a_data, const int w, const int h)
  {
    const float fw = (float)(w);
    const float fh = (float)(h);

    float ffx = a_texCoord.x*fw - 0.5f;
    float ffy = a_texCoord.y*fh - 0.5f;

    if (ffx < 0) ffx = 0.0f;
    if (ffy < 0) ffy = 0.0f;

    // Calculate the weights for each pixel
    //
    const int   px = (int)(ffx);
    const int   py = (int)(ffy);

    const float fx = fabs(ffx - (float)px);
    const float fy = fabs(ffy - (float)py);
    const float fx1 = 1.0f - fx;
    const float fy1 = 1.0f - fy;

    const float w1 = fx1 * fy1;
    const float w2 = fx  * fy1;
    const float w3 = fx1 * fy;
    const float w4 = fx  * fy;

    const int4 offsets = bilinearOffsets(ffx, ffy, w, h);

    // fetch pixels
    //
    const float4 f1 = read_array_uchar4((const uchar4*)a_data, offsets.x);
    const float4 f2 = read_array_uchar4((const uchar4*)a_data, offsets.y);
    const float4 f3 = read_array_uchar4((const uchar4*)a_data, offsets.z);
    const float4 f4 = read_array_uchar4((const uchar4*)a_data, offsets.w);

    // Calculate the weighted sum of pixels (for each color channel)
    //
    const float outr = f1.x * w1 + f2.x * w2 + f3.x * w3 + f4.x * w4;
    const float outg = f1.y * w1 + f2.y * w2 + f3.y * w3 + f4.y * w4;
    const float outb = f1.z * w1 + f2.z * w2 + f3.z * w3 + f4.z * w4;
    const float outa = f1.w * w1 + f2.w * w2 + f3.w * w3 + f4.w * w4;

    return float4(outr, outg, outb, outa);
  }

  static inline float3 TexCoord2DToSphereMap(float2 a_texCoord) // reverse to sphereMapTo2DTexCoord 
  {
    const float phi = a_texCoord.x * 2.f * (3.14159265358979323846f); // see PBRT coords:  Float phi = uv[0] * 2.f * Pi;
    const float theta = a_texCoord.y * (3.14159265358979323846f);       // see PBRT coords:  Float theta = uv[1] * Pi

    const float sinTheta = sin(theta);

    const float x = sinTheta*cos(phi);           // see PBRT coords: (Vector3f(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta)
    const float y = sinTheta*sin(phi);
    const float z = cos(theta);

    return float3(y, -z, x);
  }

  static inline float2 cube2uv(float3 a_dir, int* pPlaneNumber)
  {
    float2 uv;
    float3 dirCopy = a_dir;

    if (dirCopy.x<0)
    {
      dirCopy.x = -dirCopy.x;
    }
    if (dirCopy.y<0)
    {
      dirCopy.y = -dirCopy.y;
    }
    if (dirCopy.z<0)
    {
      dirCopy.z = -dirCopy.z;
    }

    uv.x = 0.5f;
    uv.y = 0.5f;

    if (dirCopy.x >= dirCopy.y && dirCopy.x >= dirCopy.z)
    {
      if (a_dir.x>0)
      {
        *pPlaneNumber = 0;
        uv.x += a_dir.z / a_dir.x*0.5f;
        uv.y -= a_dir.y / a_dir.x*0.5f;
      }
      else
      {
        *pPlaneNumber = 1;
        uv.x += a_dir.z / a_dir.x*0.5f;
        uv.y += a_dir.y / a_dir.x*0.5f;
      }
    }

    if (dirCopy.y>dirCopy.x && dirCopy.y >= dirCopy.z)
    {
      if (a_dir.y>0)
      {
        *pPlaneNumber = 2;
        uv.x -= a_dir.x / a_dir.y*0.5f;
        uv.y += a_dir.z / a_dir.y*0.5f;
      }
      else
      {
        *pPlaneNumber = 3;
        uv.x += a_dir.x / a_dir.y*0.5f;
        uv.y += a_dir.z / a_dir.y*0.5f;
      }
    }
    if (dirCopy.z>dirCopy.x && dirCopy.z>dirCopy.y)
    {
      if (a_dir.z>0)
      {
        *pPlaneNumber = 4;
        uv.x -= a_dir.x / a_dir.z*0.5f;
        uv.y -= a_dir.y / a_dir.z*0.5f;
      }
      else
      {
        *pPlaneNumber = 5;
        uv.x -= a_dir.x / a_dir.z*0.5f;
        uv.y += a_dir.y / a_dir.z*0.5f;
      }
    }
    return uv;
  }

  HRTextureNodeRef Cube2SphereLDR(HRTextureNodeRef a_cube[6])
  {
    int w, h, bpp;
    hrTexture2DGetSize(a_cube[0], &w, &h, &bpp);

    // extract row LDR data
    //
    std::vector<int> data[6];
    for (int i = 0; i < 6; i++)
    {
      data[i].resize(w*h);
      hrTexture2DGetDataLDR(a_cube[i], &w, &h, &data[i][0]);
    }

    const int sphW = 4 * w;
    const int sphH = 2 * h;

    // iterate (phi,theta) to get spheremap. Don't forget about gamma transform.
    // 
    std::vector<int32_t> finalData(sphW*sphH);

    #pragma omp parallel for
    for (int y = 0; y < sphH; y++)
    {
      const float ty = (float(y) + 0.5f) / float(sphH);
      for (int x = 0; x < sphW; x++)
      {
        const float tx = (float(x) + 0.5f) / float(sphW);

        const float3 dir = TexCoord2DToSphereMap(float2(tx, ty));

        int face;
        const float2 uv = float2(1, 1) - cube2uv(dir, &face);

        const float4 fpix = read_imagef_sw4(uv, &data[face][0], w, h);

        float rgba[4] = { fpix.x, fpix.y, fpix.z, fpix.w };
        finalData[y * sphW + x] = RealColorToUint32(rgba);
      }
    }

    // for test purpose use
    // HydraRender::SaveImageToFile(std::string("z_spheremap.png"), sphW, sphH, (unsigned int*)&finalData[0]);

    return hrTexture2DCreateFromMemory(sphW, sphH, 4, &finalData[0]);
  }


};
