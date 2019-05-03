/***************************************************************************
# Copyright (c) 2018, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************/
RaytracingAccelerationStructure gRtScene : register(t0, space0);
RWTexture2D<float4> gOutput : register(u0);

struct Vertex {
  float3 pos;
  float3 normal;
  float3 tangent;
  float2 tex_coord;
};

StructuredBuffer<Vertex> Vertices : register(t0, space1);
StructuredBuffer<uint> Indices : register(t0, space2);

cbuffer PerInstance : register(b0, space0)
{
    float4x4 model;
    float3x3 normM;
    float3 color;
}

float3 linearToSrgb(float3 c)
{
    // Based on http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
    float3 sq1 = sqrt(c);
    float3 sq2 = sqrt(sq1);
    float3 sq3 = sqrt(sq2);
    float3 srgb = 0.662002687 * sq1 + 0.684122060 * sq2 - 0.323583601 * sq3 - 0.0225411470 * c;
    return srgb;
}

struct RayPayload
{
    float3 raydir;
    float3 color;
    float3 normal, pos, tangent;
    int depth;
    float2 dims;
};

cbuffer Camera : register(b0, space1)
{
    matrix projI;
    matrix viewI;
    int frame;
}

float2 rand_2_0004(in float2 uv)
{
    float noiseX = (frac(sin(dot(uv, float2(12.9898,78.233)      )) * 43758.5453));
    float noiseY = (frac(sin(dot(uv, float2(12.9898,78.233) * 2.0)) * 43758.5453));
    return float2(noiseX, noiseY);
}

static const float PI = 3.14159265f;

float3 UniformSampleHemisphere(float u1, float u2)
{
    const float r = sqrt(1.0f - u1 * u1);
    const float phi = 2 * PI * u2;
 
    return normalize(float3(cos(phi) * r, sin(phi) * r, u1));
}

float3 RandomHemiDir(in float3 n, in float3 t , in float2 seed) {
    float3 ret;
    seed = abs(seed);
    float3 vhemi = UniformSampleHemisphere(seed.x, seed.y);

    float3 b = cross(t, n);
    float3x3 rot = float3x3(t.x, b.x, n.x,
                            t.y, b.y, n.y,
                            t.z, b.z, n.z);
    ret = mul(rot, vhemi);
    return ret;
}


[shader("raygeneration")]
void rayGen()
{
    uint3 launchIndex = DispatchRaysIndex();
    uint3 launchDim = DispatchRaysDimensions();

    float2 crd = float2(launchIndex.xy);
    float2 dims = float2(launchDim.xy);

    float2 d = ((crd/dims) * 2.f - 1.f);
    float aspectRatio = dims.x / dims.y;

    //RayDesc ray;
  //  ray.Origin = float3(0, 0, -2);
//    ray.Direction = normalize(float3(d.x * aspectRatio, -d.y, 1));

    // #DXR Extra: Perspective Camera
    // Perspective
    RayDesc ray;
    ray.Origin = mul(viewI, float4(0, 0, 0, 1));
    float4 target = mul(projI, float4(d.x, -d.y, 1, 1));
    ray.Direction = mul(viewI, float4(target.xyz, 0));

    ray.TMin = 0;
    ray.TMax = 100000;

    RayPayload payload;
    payload.raydir = normalize(ray.Direction); 
    payload.color = 0;
    
//    for (int i = 0; i < MAX_DEPTH; i++) {
//        stack[i].nrays = 0;
//    }

    payload.dims = rand_2_0004(crd / (float2)dims);
    payload.dims += rand_2_0004(frame * float2(3.12354, 2.1232));
    payload.depth = 1;
    TraceRay( gRtScene, 0 /*rayFlags*/, 0xFF, 0 /* ray index*/, 2, 0, ray, payload );

    
    float3 col = linearToSrgb(payload.color);
    /*
    if (view[0][0] == 1.0 &&
        view[1][0] == 2.0 &&
        view[2][0] == 3.0 &&
        view[3][0] == 4.0) {
        gOutput[launchIndex.xy] = float4(0,1,0,1);    
    } else {
        gOutput[launchIndex.xy] = float4(1,0,0,1);    
    }*/
    //gOutput[launchIndex.xy] = float4(1,0,0,1);
    
    gOutput[launchIndex.xy] = (gOutput[launchIndex.xy] * (frame - 1) + float4(col, 1)) / frame;
}

[shader("miss")]
void miss(inout RayPayload payload)
{
    float ndotl = dot(payload.raydir, normalize(float3(0, 1, 2)));
    if (ndotl >= 0.95 && ndotl <= 1.0) {
        payload.color = float3(1, 1, 1);
    } else {
        payload.color = 0.1 * float3(0.1, 0.2, 0.4);//0.2 * lerp(float3(1,0,0), float3(0,1,0), float3(1,1,1) * (0.5 + 0.5 * sin(ndotl* 20)));//float3(0.2, 0.3, 0.5);
    }

    payload.color *= 200;
}

void ComputeVertexAttribs(in float3 barycentrics, out float3 normal, out float3 tangent, out float3 pos) {
    uint vertId = 3 * PrimitiveIndex();
    normal =        Vertices[Indices[vertId + 0]].normal * barycentrics.x +
                    Vertices[Indices[vertId + 1]].normal * barycentrics.y +
                    Vertices[Indices[vertId + 2]].normal * barycentrics.z;
    tangent =       Vertices[Indices[vertId + 0]].tangent * barycentrics.x +
                    Vertices[Indices[vertId + 1]].tangent * barycentrics.y +
                    Vertices[Indices[vertId + 2]].tangent * barycentrics.z;
    pos    =        Vertices[Indices[vertId + 0]].pos * barycentrics.x +
                    Vertices[Indices[vertId + 1]].pos * barycentrics.y +
                    Vertices[Indices[vertId + 2]].pos * barycentrics.z;
    
    tangent = mul(normM, tangent);
    normal = mul(normM, normal);
    float4 pos4 = mul(model, float4(pos, 1));
    pos = pos4.xyz / pos4.w;

    tangent = normalize(tangent);
    normal = normalize(normal);
}

[shader("closesthit")]
void triangleChs(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    const int MAX_DEPTH = 5;
    const int PER_DEPTH_RAYS = 2;
    
    if (payload.depth >= MAX_DEPTH) {
        return;
    }
    
    float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);
    
    float3 normal, tangent, pos;
    
    ComputeVertexAttribs(barycentrics, normal, tangent, pos);
    

    RayPayload npayload;
    npayload.tangent = tangent;
    npayload.normal = normal;
    npayload.pos = pos;
    npayload.depth = payload.depth + 1;
    npayload.color = 0;
    //normal = normalize(normal);
    float3 dir = float3(-1, 1, 1);
    dir = normalize(dir);
    float3 accum = 0;

    npayload.dims = payload.dims;
        
    for (int nrays = 0; nrays < PER_DEPTH_RAYS; nrays++) {
        RayDesc ray;
        
        ray.Origin = pos + normal * 0.00001;

        float2 seed = rand_2_0004(npayload.dims);
        ray.Direction = RandomHemiDir(normal, tangent, seed);
        npayload.dims += seed * float2(1.12312, -3.78645);
        ray.TMin = 0;
        ray.TMax = 10000000;
        npayload.raydir = ray.Direction;
        npayload.color = 0;
    
        TraceRay( gRtScene, 0 , 0xFF, 0 , 2, 0, ray, npayload );
        
        //accum += abs(min(dot(normal, ray.Direction), 0)) * float3(1, 1, 1);
        accum += npayload.color *  max(dot(ray.Direction, normal), 0);
    }
    payload.color += accum / PER_DEPTH_RAYS;
    payload.dims = npayload.dims;

    //payload.color = pos;
    //payload.color = tangents * 0.5 + float3(0.5, 0.5, 0.5);    
    //payload.color = float3(1, 1, 1) * dot(normal, dir);
}

struct ShadowPayload
{
    bool hit;
};

[shader("closesthit")]
void planeChs(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    float hitT = RayTCurrent();
    float3 rayDirW = WorldRayDirection();
    float3 rayOriginW = WorldRayOrigin();

    // Find the world-space hit position
    float3 posW = rayOriginW + hitT * rayDirW;

    // Fire a shadow ray. The direction is hard-coded here, but can be fetched from a constant-buffer
    RayDesc ray;
    ray.Origin = posW;
    ray.Direction = normalize(float3(0.5, 0.5, -0.5));
    ray.TMin = 0.01;
    ray.TMax = 100000;
    ShadowPayload shadowPayload;
    TraceRay(gRtScene, 0  /*rayFlags*/, 0xFF, 1 /* ray index*/, 0, 1, ray, shadowPayload);

    float factor = shadowPayload.hit ? 0.1 : 1.0;
    payload.color = 0;//float4(0.9f, 0.9f, 0.9f, 1.0f) * factor;
}

[shader("closesthit")]
void shadowChs(inout ShadowPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    payload.hit = true;
}

[shader("miss")]
void shadowMiss(inout ShadowPayload payload)
{
    payload.hit = false;
}
