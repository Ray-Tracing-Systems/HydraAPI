//
// Created by vsan on 14.05.18.
//

#ifndef HYDRAAPI_EX_HYDRATEXTUREUTILS_H
#define HYDRAAPI_EX_HYDRATEXTUREUTILS_H

#include "LiteMath.h"
#include "pugixml.hpp"

using namespace HydraLiteMath;

namespace HRTextureUtils
{
    float sampleHeightMapLDR(const std::vector<int> &imageData, int w, int h, float2 uv, float4x4 matrix);

    float sampleHeightMapHDR(const std::vector<float> &imageData, int w, int h, float2 uv, float4x4 matri);

    float sampleNoise(pugi::xml_node noiseXMLNode, float3 attrib);

    float noise(float3 p, float distortion, float detail);

    float noise_musgrave_fBm(float3 p, float H, float lacunarity, float octaves);

    float fitRange(float x, float src_a, float src_b, float dest_a, float dest_b);
}


#endif //HYDRAAPI_EX_HYDRATEXTUREUTILS_H
