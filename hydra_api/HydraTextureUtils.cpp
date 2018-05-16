//
// Created by vsan on 14.05.18.
//

#include "HydraTextureUtils.h"
#include "HydraAPI.h"

char sampleTextureLDR(pugi::xml_node textureNode, float2 uv)
{
  auto texId = textureNode.attribute(L"id").as_int();
  auto location = textureNode.attribute(L"loc").as_string();
  auto w = textureNode.attribute(L"width").as_int();
  auto h = textureNode.attribute(L"height").as_int();
  auto bpp = textureNode.attribute(L"offset").as_int() / 2;

  if(location != L"")
  {
    auto *imageData = new int[w * h];

    HRTextureNodeRef texRef;
    texRef.id = texId;

    hrTextureNodeOpen(texRef, HR_OPEN_READ_ONLY);
    {
      hrTexture2DGetDataLDR(texRef, &w, &h, imageData);
    }
    hrTextureNodeClose(texRef);

    int val = imageData[int(uv.y * w + uv.x)];
    unsigned char ch1 = (val & 0x00FF0000) >> 16;
    unsigned char ch2 = (val & 0x0000FF00) >> 8;
    unsigned char ch3 = (val & 0x000000FF);

    float gray = 0.2126f * (ch3 / 255.0f) + 0.7152f * (ch2 / 255.0f) + 0.0722f * (ch1 / 255.0f);


    delete[] imageData;

  }
}

float4 sampleTextureHDR(pugi::xml_node textureNode, float2 uv)
{

}