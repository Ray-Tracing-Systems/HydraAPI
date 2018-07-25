// procedure texture test

float4 userProc(const SurfaceInfo* sHit, float4 inColor)
{
  const float2 texCoord = readAttr(sHit,"TexCoord0");
  return make_float4(texCoord.x, texCoord.y, 0.0f, 0.0f)*inColor;
}
