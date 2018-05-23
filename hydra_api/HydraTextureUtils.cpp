//
// Created by vsan on 14.05.18.
//

#include <iostream>
#include "HydraTextureUtils.h"
#include "HydraAPI.h"


float sampleHeightMapLDR(const std::vector<int> &imageData, int w, int h, float2 uv)
{
  float acc = 0.0f;
  int samples = 0;
  for(int i = 0; i <= 1; i += 1)
  {
    for(int j = 0; j <= 1; j += 1)
    {
      int x = int(uv.x * (w - 1)) + i;
      int y = int(uv.y * (h - 1)) + j;

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

float sampleHeightMapHDR(const std::vector<float> &imageData, int w, int h, float2 uv)
{
  float acc = 0.0f;
  int samples = 0;
  for(int i = 0; i <= 1; i += 1)
  {
    for(int j = 0; j <= 1; j += 1)
    {
      int x = int(uv.x * (w - 1)) + i;
      int y = int(uv.y * (h - 1)) + j;

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