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
  
  const float scale = kScale;

  float3 v38[8];
  v38[0] = make_float3(-diff[0], -diff[0], scale);
  v38[1] = make_float3(0.f, -diff[1], scale);
  v38[2] = make_float3(diff[2], -diff[2], scale);
  v38[3] = make_float3(-diff[3], 0.f, scale);
  v38[4] = make_float3(diff[4], 0.f, scale);
  v38[5] = make_float3(-diff[5], diff[5], scale);
  v38[6] = make_float3(0.f, diff[6], scale);
  v38[7] = make_float3(diff[7], diff[7], scale);

  float3 res = make_float3(0, 0, 0);
  for (int i = 0; i<8; i++)
    res += v38[i];

  res = res*(1.0f / 8.0f);

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

  const float zeroPointFive = 0.49995f; // this is due to on NV and AMD 0.5 saved to 8 bit textures in different way - 127 on NV and 128 on AMD

  const float resX = clamp(0.5f*res.x + zeroPointFive, 0.0f, 1.0f);
  const float resY = clamp(0.5f*res.y + zeroPointFive, 0.0f, 1.0f);
  const float resZ = clamp(1.0f*res.z + 0.0f, 0.0f, 1.0f);
  const float resW = clamp((255.0f - hThisPixel) / 255.0f, 0.0f, 1.0f);

  return make_float4(resX,resY,resZ,resW);
}

