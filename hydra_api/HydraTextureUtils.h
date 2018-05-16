//
// Created by vsan on 14.05.18.
//

#ifndef HYDRAAPI_EX_HYDRATEXTUREUTILS_H
#define HYDRAAPI_EX_HYDRATEXTUREUTILS_H

#include "LiteMath.h"
#include "pugixml.hpp"

using namespace HydraLiteMath;

float sampleGrayscaleTextureLDR(const std::vector<int> &imageData, int w, int h, float2 uv);
float  sampleTextureHDR(pugi::xml_node textureNode, float2 uv);


#endif //HYDRAAPI_EX_HYDRATEXTUREUTILS_H
