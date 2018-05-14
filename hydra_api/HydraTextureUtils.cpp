//
// Created by vsan on 14.05.18.
//

#include "HydraTextureUtils.h"

char   sampleTextureLDR(pugi::xml_node textureNode, float2 uv)
{
 /* auto texId = textureNode.attribute(L"id").as_int();
  auto location = textureNode.attribute(L"loc").as_string();
  auto w = textureNode.attribute(L"width").as_int();
  auto h = textureNode.attribute(L"height").as_int();
  auto bpp = textureNode.attribute(L"offset").as_int() / 2;

  auto *imageData = new unsigned char[w * h * bpp];

  if()

  texture.ldrCallback(imageData, w, h, texture.customData);

  auto pTextureImpl = g_objManager.m_pFactory->CreateTexture2DFromMemory(&texture, w, h, bpp, imageData);
  texture.pImpl = pTextureImpl;

  delete[] imageData;

  isProc = true;*/

}

float4 sampleTextureHDR(pugi::xml_node textureNode, float2 uv)
{

}