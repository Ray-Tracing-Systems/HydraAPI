// procedure texture test

float4 userProc(const SurfaceInfo* sHit, sampler2D tex1, sampler2D tex2, float2 texOffset, float4 inColor)
{
  const float2 texCoord = readAttr(sHit,"TexCoord0");
  const float4 texColor = inColor*texture2D(tex1, texCoord + texOffset, TEX_CLAMP_U | TEX_CLAMP_V)*texture2D(tex2, texCoord + texOffset, TEX_CLAMP_U | TEX_CLAMP_V);
  return make_float4(texCoord.x, texCoord.y, 0.0f, 0.0f)*texColor;
}