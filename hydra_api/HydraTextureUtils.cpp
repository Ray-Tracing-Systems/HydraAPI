//
// Created by vsan on 14.05.18.
//

#include <iostream>
#include "HydraTextureUtils.h"
#include "HydraAPI.h"

float sampleGrayscaleTextureLDR(const std::vector<int> &imageData, int w, int h, float2 uv)
{
  int x = uv.x * (w - 1);
  int y = uv.y * (h - 1);
  int ind = y * w + x;
  if(ind < w *h)
  {
    int val = imageData.at(ind);
    unsigned char ch1 = (val & 0x00FF0000) >> 16;
    unsigned char ch2 = (val & 0x0000FF00) >> 8;
    unsigned char ch3 = (val & 0x000000FF);

    float gray = 0.2126f * (ch3 / 255.0f) + 0.7152f * (ch2 / 255.0f) + 0.0722f * (ch1 / 255.0f);

    return gray;
  }
  else
  {
    std::cout << "sample tex out of bounds, uv: " << uv.x << ", " << uv.y << " ind :" << ind << std::endl;
    return 0.0f;
  }
}

float sampleTextureHDR(pugi::xml_node textureNode, float2 uv)
{
  return 0.0f;
}