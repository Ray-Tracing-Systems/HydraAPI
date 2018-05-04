float4 main(const SurfaceInfo* sHit, float3 colorHit, sampler2D texHit, float3 colorMiss, sampler2D texMiss, float faloffPower)
{
  const float2 texCoord = readAttr(sHit,"TexCoord0");
  float  ao             = readAttr(sHit,"AO");
 
  const float4 col1     = to_float4(colorHit, 1.0f)*texture2D(texHit, texCoord, 0);
  const float4 col2     = to_float4(colorMiss,1.0f)*texture2D(texMiss, texCoord, 0);
  
  ao = 1.0f - pow(ao, faloffPower);
  
  return ao*col1 + (1.0f-ao)*col2;
}
