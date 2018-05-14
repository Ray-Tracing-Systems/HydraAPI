//
// Created by vsan on 14.05.18.
//

#ifndef HYDRAAPI_EX_HYDRATEXTUREUTILS_H
#define HYDRAAPI_EX_HYDRATEXTUREUTILS_H

#include "LiteMath.h"
#include "pugixml.hpp"

using namespace HydraLiteMath;

char   sampleTextureLDR(pugi::xml_node textureNode, float2 uv);
float4 sampleTextureHDR(pugi::xml_node textureNode, float2 uv);


#endif //HYDRAAPI_EX_HYDRATEXTUREUTILS_H
