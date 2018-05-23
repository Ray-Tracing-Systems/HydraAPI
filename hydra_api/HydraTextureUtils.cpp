//
// Created by vsan on 14.05.18.
//

#include <iostream>
#include "HydraTextureUtils.h"
#include "HydraAPI.h"


static const uint32_t permutation[] = { 151, 160, 137, 91, 90, 15,
                                   131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
                                   190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
                                   88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
                                   77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
                                   102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
                                   135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
                                   5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
                                   223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
                                   129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
                                   251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
                                   49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
                                   138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
};

float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }
float normalize01(float x, float min, float max) { return (x - min) / (max - min); }


struct PerlinNoise
{
    uint32_t p[512];

    void initPerlin()
    {
      for (int x = 0; x<512; x++)
      {
        p[x] = permutation[x % 256];
      }
    }

    PerlinNoise()
    {
      initPerlin();
    }

    float grad(uint32_t hash, float x, float y, float z)
    {
      switch (hash & 0xF)
      {
        case 0x0: return  x + y;
        case 0x1: return -x + y;
        case 0x2: return  x - y;
        case 0x3: return -x - y;
        case 0x4: return  x + z;
        case 0x5: return -x + z;
        case 0x6: return  x - z;
        case 0x7: return -x - z;
        case 0x8: return  y + z;
        case 0x9: return -y + z;
        case 0xA: return  y - z;
        case 0xB: return -y - z;
        case 0xC: return  y + x;
        case 0xD: return -y + z;
        case 0xE: return  y - x;
        case 0xF: return -y - z;
        default: return 0;
      }
    };

    float noise(float x, float y, float z)
    {
      int X = (uint32_t)floorf(x) & 255;
      int Y = (uint32_t)floorf(y) & 255;
      int Z = (uint32_t)floorf(z) & 255;

      x -= floorf(x);
      y -= floorf(y);
      z -= floorf(z);

      float u = fade(x);
      float v = fade(y);
      float w = fade(z);


      int A = p[X] + Y;
      int AA = p[A] + Z;
      int AB = p[A + 1] + Z;
      int B = p[X + 1] + Y;
      int BA = p[B] + Z;
      int BB = p[B + 1] + Z;

      float res = lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z),
                                        grad(p[BA], x - 1, y, z)),
                                lerp(u, grad(p[AB], x, y - 1, z),
                                     grad(p[BB], x - 1, y - 1, z))),
                        lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1),
                                     grad(p[BA + 1], x - 1, y, z - 1)),
                             lerp(u, grad(p[AB + 1], x, y - 1, z - 1),
                                  grad(p[BB + 1], x - 1, y - 1, z - 1))));

      return normalize01(res, -1, 1);
    }

    float octave(float x, float y, float z, int octaves, float base_frequency, float persistence, float lacunarity)
    {
      float total = 0;
      float frequency = base_frequency;
      float amplitude = 1;
      float maxValue = 0;

      for (int i = 0; i<octaves; i++)
      {
        total += noise(x * frequency, y * frequency, z * frequency) * amplitude;

        maxValue += amplitude;

        amplitude *= persistence;
        frequency *= lacunarity;
      }

      return total / maxValue;
    }
};



float sampleHeightMapLDR(const std::vector<int> &imageData, int w, int h, float2 uv, float4x4 matrix)
{
  float acc = 0.0f;
  int samples = 0;
  for(int i = 0; i <= 1; i += 1)
  {
    for(int j = 0; j <= 1; j += 1)
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
      if(ind < w *h)
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
  for(int i = 0; i <= 1; i += 1)
  {
    for(int j = 0; j <= 1; j += 1)
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
      if(ind < w *h)
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
  static PerlinNoise pn;

  if(noiseXMLNode != nullptr)
  {
    auto base_freq   = noiseXMLNode.attribute(L"base_freq").as_float();
    auto num_octaves = noiseXMLNode.attribute(L"octaves").as_int();
    auto persistance = noiseXMLNode.attribute(L"persistance").as_float();
    auto lacunarity  = noiseXMLNode.attribute(L"lacunarity").as_float();

    return pn.octave(attrib.x, attrib.y, attrib.z, num_octaves, base_freq, persistance, lacunarity);

  }
  else
  {
    return 0.0f;
  }
}