//
// Created by vsan on 14.05.18.
//

#include <iostream>
#include "HydraTextureUtils.h"
#include "HydraAPI.h"

namespace HRTextureUtils
{
    float fitRange(float x, float src_a, float src_b, float dest_a, float dest_b)
    {
      x = x > src_b ? src_b : x;
      x = x < src_a ? src_a : x;
      float range = src_b - src_a;
      float tmp = (x - src_a) / range;

      float range2 = dest_b - dest_a;

      return tmp * range2 + dest_a;
    }

    float clampf(float x, float minval, float maxval)
    {
      return fmaxf(fminf(x, maxval), minval);
    }

    int clampi(int x, int minval, int maxval)
    {
      return max(min(x, maxval), minval);
    }

    float3 abs3(float3 a)
    {
      return make_float3(fabsf(a.x), fabsf(a.y), fabsf(a.z));
    }

    float4 abs4(float4 a)
    {
      return make_float4(fabsf(a.x), fabsf(a.y), fabsf(a.z), fabsf(a.w));
    }

    float3 floor3(float3 v)
    {
      return make_float3(floorf(v.x), floorf(v.y), floorf(v.z));
    }

    float4 floor4(float4 v)
    {
      return make_float4(floorf(v.x), floorf(v.y), floorf(v.z), floorf(v.w));
    }

    float fract(float v)
    {
      return v - floorf(v);
    }

    float3 fract3(float3 v)
    {
      return v - floor3(v);
    }

    float4 fract4(float4 v)
    {
      return v - floor4(v);
    }

    float3 mod289f3(float3 x)
    {
      return x - floor3(x * (1.0 / 289.0)) * 289.0;
    }

    float4 mod289f4(float4 x)
    {
      return x - floor4(x * (1.0 / 289.0)) * 289.0;
    }

    float4 permute(float4 x)
    {
      return mod289f4((x * 34.0f + 1.0f) * x);
    }

    float4 taylorInvSqrt(float4 r)
    {
      return 1.79284291400159f - 0.85373472095314f * r;
    }

    float3 fade(float3 t)
    {
      return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
    }

    float mix(float x, float y, float a)
    {
      return x * (1.0f - a) + y * a;
    }

    float2 mix2(float2 x, float2 y, float a)
    {
      return make_float2(mix(x.x, y.x, a), mix(x.y, y.y, a));
    }

    float3 mix3(float3 x, float3 y, float a)
    {
      return make_float3(mix(x.x, y.x, a), mix(x.y, y.y, a), mix(x.z, y.z, a));
    }

    float4 mix4(float4 x, float4 y, float a)
    {
      return make_float4(mix(x.x, y.x, a), mix(x.y, y.y, a), mix(x.z, y.z, a), mix(x.w, y.w, a));
    }

    float step(float edge, float x)
    {
      return x < edge ? 0.0f : 1.0f;
    }

    float4 step4(float edge, float4 x)
    {
      return make_float4(x.x < edge ? 0.0f : 1.0f, x.y < edge ? 0.0f : 1.0f,
                         x.z < edge ? 0.0f : 1.0f, x.w < edge ? 0.0f : 1.0f);
    }

    float4 step4_(float4 edge, float4 x)
    {
      return make_float4(x.x < edge.x ? 0.0f : 1.0f, x.y < edge.y ? 0.0f : 1.0f,
                         x.z < edge.z ? 0.0f : 1.0f, x.w < edge.w ? 0.0f : 1.0f);
    }


    float rand(float n)
    { return fract(sinf(n) * 43758.5453123f); }
/*float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
*/

// Classic Perlin noise
    float cnoise(float3 P)
    {
      float3 Pi0 = floor3(P); // Integer part for indexing
      float3 Pi1 = Pi0 + make_float3(1.0, 1.0, 1.0); // Integer part + 1
      Pi0 = mod289f3(Pi0);
      Pi1 = mod289f3(Pi1);
      float3 Pf0 = fract3(P); // Fractional part for interpolation
      float3 Pf1 = Pf0 - make_float3(1.0, 1.0, 1.0); // Fractional part - 1.0
      float4 ix = make_float4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
      float4 iy = make_float4(Pi0.y, Pi0.y, Pi1.y, Pi1.y);
      float4 iz0 = make_float4(Pi0.z, Pi0.z, Pi0.z, Pi0.z);
      float4 iz1 = make_float4(Pi1.z, Pi1.z, Pi1.z, Pi1.z);

      float4 ixy = permute(permute(ix) + iy);
      float4 ixy0 = permute(ixy + iz0);
      float4 ixy1 = permute(ixy + iz1);

      float4 gx0 = ixy0 * (1.0 / 7.0);
      float4 gy0 = fract4(floor4(gx0) * (1.0 / 7.0)) - 0.5;
      gx0 = fract4(gx0);
      float4 gz0 = make_float4(0.5, 0.5, 0.5, 0.5) - abs4(gx0) - abs4(gy0);
      float4 sz0 = step4_(gz0, make_float4(0.0, 0.0, 0.0, 0.0));
      gx0 -= sz0 * (step4(0.0, gx0) - 0.5);
      gy0 -= sz0 * (step4(0.0, gy0) - 0.5);

      float4 gx1 = ixy1 * (1.0 / 7.0);
      float4 gy1 = fract4(floor4(gx1) * (1.0 / 7.0)) - 0.5;
      gx1 = fract4(gx1);
      float4 gz1 = make_float4(0.5, 0.5, 0.5, 0.5) - abs4(gx1) - abs4(gy1);
      float4 sz1 = step4_(gz1, make_float4(0.0, 0.0, 0.0, 0.0));
      gx1 -= sz1 * (step4(0.0f, gx1) - 0.5);
      gy1 -= sz1 * (step4(0.0f, gy1) - 0.5);

      float3 g000 = make_float3(gx0.x, gy0.x, gz0.x);
      float3 g100 = make_float3(gx0.y, gy0.y, gz0.y);
      float3 g010 = make_float3(gx0.z, gy0.z, gz0.z);
      float3 g110 = make_float3(gx0.w, gy0.w, gz0.w);
      float3 g001 = make_float3(gx1.x, gy1.x, gz1.x);
      float3 g101 = make_float3(gx1.y, gy1.y, gz1.y);
      float3 g011 = make_float3(gx1.z, gy1.z, gz1.z);
      float3 g111 = make_float3(gx1.w, gy1.w, gz1.w);

      float4 norm0 = taylorInvSqrt(make_float4(dot(g000, g000), dot(g010, g010), dot(g100, g100), dot(g110, g110)));
      g000 *= norm0.x;
      g010 *= norm0.y;
      g100 *= norm0.z;
      g110 *= norm0.w;
      float4 norm1 = taylorInvSqrt(make_float4(dot(g001, g001), dot(g011, g011), dot(g101, g101), dot(g111, g111)));
      g001 *= norm1.x;
      g011 *= norm1.y;
      g101 *= norm1.z;
      g111 *= norm1.w;

      float n000 = dot(g000, Pf0);
      float n100 = dot(g100, make_float3(Pf1.x, Pf0.y, Pf0.z));
      float n010 = dot(g010, make_float3(Pf0.x, Pf1.y, Pf0.z));
      float n110 = dot(g110, make_float3(Pf1.x, Pf1.y, Pf0.z));
      float n001 = dot(g001, make_float3(Pf0.x, Pf0.y, Pf1.z));
      float n101 = dot(g101, make_float3(Pf1.x, Pf0.y, Pf1.z));
      float n011 = dot(g011, make_float3(Pf0.x, Pf1.y, Pf1.z));
      float n111 = dot(g111, Pf1);

      float3 fade_xyz = fade(Pf0);
      float4 n_z = mix4(make_float4(n000, n100, n010, n110), make_float4(n001, n101, n011, n111), fade_xyz.z);
      float2 n_yz = mix2(make_float2(n_z.x, n_z.y), make_float2(n_z.z, n_z.w), fade_xyz.y);
      float n_xyz = mix(n_yz.x, n_yz.y, fade_xyz.x);
      return n_xyz;
    }

    float octave(float3 pos, int octaves, float persistence, float frequency, float lacunarity)
    {
      float total = 0.0f;
      float amplitude = 1.0f;
      float maxValue = 0.0f;

      for (int i = 0; i < octaves; i++)
      {
        total += cnoise(pos * frequency) * amplitude;

        maxValue += amplitude;

        amplitude *= persistence;
        frequency *= lacunarity;
      }

      return total / maxValue;
    }

    float noise_musgrave_fBm(float3 p, float H, float lacunarity, float octaves)
    {
      float rmd;
      float value = 0.0;
      float pwr = 1.0;
      float pwHL = powf(lacunarity, -H);
      int i;

      for (i = 0; i < (int) octaves; i++)
      {
        value += cnoise(p) * pwr;
        pwr *= pwHL;
        p *= lacunarity;
      }

      rmd = octaves - floorf(octaves);
      if (rmd != 0.0)
        value += rmd * cnoise(p) * pwr;

      return value;
    }


    float unsigned_perlin(float3 pos)
    {
      float n = cnoise(pos);

      return 0.5f * n + 0.5f;
    }

    float noise_turbulence(float3 p, float details, int hard)
    {
      float fscale = 1.0;
      float amp = 1.0;
      float sum = 0.0;
      int i, n;

      float octaves = clamp(details, 0.0, 16.0);
      n = (int) octaves;

      for (i = 0; i <= n; i++)
      {
        float t = unsigned_perlin(fscale * p);

        if (hard)
          t = fabsf(2.0f * t - 1.0f);

        sum += t * amp;
        amp *= 0.5;
        fscale *= 2.0;
      }

      float rmd = octaves - floorf(octaves);

      if (rmd != 0.0)
      {
        float t = unsigned_perlin(fscale * p);

        if (hard)
          t = fabsf(2.0f * t - 1.0f);

        float sum2 = sum + t * amp;

        sum *= ((float) (1 << n) / (float) ((1 << (n + 1)) - 1));
        sum2 *= ((float) (1 << (n + 1)) / (float) ((1 << (n + 2)) - 1));

        return (1.0f - rmd) * sum + rmd * sum2;
      } else
      {
        sum *= ((float) (1 << n) / (float) ((1 << (n + 1)) - 1));
        return sum;
      }
    }

    float noise(float3 p, float distortion, float detail)
    {
      float3 r;
      int hard = 0;

      if (distortion > 0.000001f)
      {
        r.x = unsigned_perlin(p + make_float3(13.5f, 13.5f, 13.5f)) * distortion;
        r.y = unsigned_perlin(p) * distortion;
        r.z = unsigned_perlin(p - make_float3(13.5f, 13.5f, 13.5f)) * distortion;

        p += r;
      }

      float res = noise_turbulence(p, detail, hard);

      return res;
    }


    float sampleHeightMapLDR(const std::vector<int> &imageData, int w, int h, float2 uv, float4x4 matrix)
    {
      float acc = 0.0f;
      int samples = 0;
      for (int i = 0; i <= 1; i += 1)
      {
        for (int j = 0; j <= 1; j += 1)
        {
          float4 uv_ = float4(uv.x, uv.y, 0.0f, 1.0f);
          uv_ = mul(matrix, uv_);

          uv_.x = uv_.x - floorf(uv_.x);
          uv_.y = uv_.y - floorf(uv_.y);

          uv_.x = uv_.x > 1.0f ? 1.0f - uv_.x : uv_.x;
          uv_.x = uv_.x < 0.0f ? 1.0f + uv_.x : uv_.x;

          uv_.y = uv_.y > 1.0f ? 1.0f - uv_.y : uv_.y;
          uv_.y = uv_.y < 0.0f ? 1.0f + uv_.y : uv_.y;

          int x = int(uv_.x * (w - 1)) + i;
          int y = int(uv_.y * (h - 1)) + j;

          x = x > (w - 1) ? w - 1 : x;
          x = x < 0 ? 0 : x;

          y = y > (h - 1) ? h - 1 : y;
          y = y < 0 ? 0 : y;

          int ind = y * w + x;
          if (ind < w * h)
          {
            int val = imageData.at(ind);
            unsigned char ch1 = (val & 0x00FF0000) >> 16;
            unsigned char ch2 = (val & 0x0000FF00) >> 8;
            unsigned char ch3 = (val & 0x000000FF);

            float gray = 0.2126f * (ch3 / 255.0f) + 0.7152f * (ch2 / 255.0f) + 0.0722f * (ch1 / 255.0f);

            acc += gray;
            samples++;
          }
        }
      }
      return acc / samples;
    }

    float sampleHeightMapHDR(const std::vector<float> &imageData, int w, int h, float2 uv, float4x4 matrix)
    {
      float acc = 0.0f;
      int samples = 0;
      for (int i = 0; i <= 1; i += 1)
      {
        for (int j = 0; j <= 1; j += 1)
        {
          float4 uv_ = float4(uv.x, uv.y, 0.0f, 1.0f);
          uv_ = mul(matrix, uv_);

          uv_.x = uv_.x - floorf(uv_.x);
          uv_.y = uv_.y - floorf(uv_.y);

          uv_.x = uv_.x > 1.0f ? 1.0f - uv_.x : uv_.x;
          uv_.x = uv_.x < 0.0f ? 1.0f + uv_.x : uv_.x;

          uv_.y = uv_.y > 1.0f ? 1.0f - uv_.y : uv_.y;
          uv_.y = uv_.y < 0.0f ? 1.0f + uv_.y : uv_.y;

          int x = int(uv_.x * (w - 1)) + i;
          int y = int(uv_.y * (h - 1)) + j;

          x = x > (w - 1) ? w - 1 : x;
          x = x < 0 ? 0 : x;

          y = y > (h - 1) ? h - 1 : y;
          y = y < 0 ? 0 : y;

          int ind = y * w + x;
          if (ind < w * h)
          {
            float ch1 = imageData.at(ind * 4 + 0);
            float ch2 = imageData.at(ind * 4 + 1);
            float ch3 = imageData.at(ind * 4 + 2);

            float gray = 0.2126f * ch1 + 0.7152f * ch2 + 0.0722f * ch3;

            acc += gray;
            samples++;
          }
        }
      }
      return acc / samples;
    }

    float sampleNoise(pugi::xml_node noiseXMLNode, float3 attrib)
    {

      if (noiseXMLNode != nullptr)
      {
        auto base_freq = noiseXMLNode.attribute(L"base_freq").as_float();
        auto num_octaves = noiseXMLNode.attribute(L"octaves").as_int();
        auto persistence = noiseXMLNode.attribute(L"persistence").as_float();
        auto lacunarity = noiseXMLNode.attribute(L"lacunarity").as_float();

        return 0.5f * (octave(attrib, num_octaves, persistence, base_freq, lacunarity) + 1.0f);
      } else
      {
        return 0.0f;
      }
    }
}