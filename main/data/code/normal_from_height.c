float getHeight(const float4 inVal) 
{ 
  const float4 res = 255.0f*(make_float4(1,1,1,1) - inVal);
  return fmax(res.x, fmax(res.y, res.z));
}

float2 toNormalizedCoord(const float3 input, const float2 multInv)
{
  const float x1 = input.x + 0.5f;
  const float y1 = input.y + 0.5f;
  return make_float2(x1, y1)*multInv;
}

float4 main(const SurfaceInfo* sHit, sampler2D texHeight, float2 invTexRes)
{
  const float2 texCoord  = readAttr(sHit,"TexCoord0");

  const float kScale     = 2000.0f * fmax(invTexRes.x, invTexRes.y);
  const float hThisPixel = getHeight(texture2D(texHeight, texCoord, TEX_CLAMP_U | TEX_CLAMP_V));

  //return make_float4(hThisPixel,-hThisPixel,hThisPixel,0.0f);

  float diff[8];

  diff[0] = hThisPixel - getHeight( texture2D(texHeight, texCoord + make_float2(-1.0f, -1.0f)*invTexRes, TEX_CLAMP_U | TEX_CLAMP_V) ); 
  diff[1] = hThisPixel - getHeight( texture2D(texHeight, texCoord + make_float2(0.0f,  -1.0f)*invTexRes, TEX_CLAMP_U | TEX_CLAMP_V) ); 
  diff[2] = hThisPixel - getHeight( texture2D(texHeight, texCoord + make_float2(+1.0f, -1.0f)*invTexRes, TEX_CLAMP_U | TEX_CLAMP_V) ); 
  diff[3] = hThisPixel - getHeight( texture2D(texHeight, texCoord + make_float2(-1.0f, 0.0f) *invTexRes, TEX_CLAMP_U | TEX_CLAMP_V) ); 
  diff[4] = hThisPixel - getHeight( texture2D(texHeight, texCoord + make_float2(+1.0f, 0.0f) *invTexRes, TEX_CLAMP_U | TEX_CLAMP_V) ); 
  diff[5] = hThisPixel - getHeight( texture2D(texHeight, texCoord + make_float2(-1.0f, +1.0f)*invTexRes, TEX_CLAMP_U | TEX_CLAMP_V) ); 
  diff[6] = hThisPixel - getHeight( texture2D(texHeight, texCoord + make_float2(0.0f,  +1.0f)*invTexRes, TEX_CLAMP_U | TEX_CLAMP_V) ); 
  diff[7] = hThisPixel - getHeight( texture2D(texHeight, texCoord + make_float2(+1.0f, +1.0f)*invTexRes, TEX_CLAMP_U | TEX_CLAMP_V) ); 

  const float allDiffSumm = fabs(diff[0]) + fabs(diff[1]) + fabs(diff[2]) + fabs(diff[3]) + fabs(diff[4]) + fabs(diff[5]) + fabs(diff[6]) + fabs(diff[7]);

  float3 res = (1.0f / 8.0f)*(make_float3(-diff[0], -diff[0], kScale) + make_float3(0.f, -diff[1], kScale)     + 
                              make_float3(diff[2], -diff[2], kScale)  + make_float3(-diff[3], 0.f, kScale)     + 
                              make_float3(diff[4], 0.f, kScale)       + make_float3(-diff[5], diff[5], kScale) + 
                              make_float3(0.f, diff[6], kScale)       + make_float3(diff[7], diff[7],  kScale));

  res.x *= -1.0f;
  res.y *= -1.0f;
  res.z *= +1.0f;

  res = normalize(res);

  if (res.z < 0.65f)
  {
    res.z = 0.65f;
    res = normalize(res);
  }

  if (allDiffSumm < 1.0f)
  {
    res.x = 0.0f;
    res.y = 0.0f;
    res.z = 1.0f;
  }

  return StoreNormal(res, NORMAL_IN_TANGENT_SPACE);
}

