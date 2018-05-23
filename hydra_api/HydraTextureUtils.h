//
// Created by vsan on 14.05.18.
//

#ifndef HYDRAAPI_EX_HYDRATEXTUREUTILS_H
#define HYDRAAPI_EX_HYDRATEXTUREUTILS_H

#include "LiteMath.h"
#include "pugixml.hpp"

using namespace HydraLiteMath;

float sampleHeightMapLDR(const std::vector<int> &imageData, int w, int h, float2 uv);
float sampleHeightMapHDR(const std::vector<float> &imageData, int w, int h, float2 uv);


#endif //HYDRAAPI_EX_HYDRATEXTUREUTILS_H
