
float4 main(const SurfaceInfo* sHit, sampler2D texNorm)
{
  const float2 texCoord = readAttr(sHit,"TexCoord0");
  const float4 texColor = texture2D(texNorm, texCoord, TEX_CLAMP_U | TEX_CLAMP_V);

  if(texCoord.x < 0.5f)
    return texColor;
  else
    return StoreNormal(make_float3(0,0,1), NORMAL_IN_TANGENT_SPACE); 
}

