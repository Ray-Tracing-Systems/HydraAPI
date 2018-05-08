// procedure texture test

float4 userProc(const SurfaceInfo* sHit, sampler2D tex1, __global sampler2D tex2[5], float2 texOffset, float4 inColor)
{
  
  const float2 texCoord = readAttr(sHit,"TexCoord0");
  float4 texColorBase = texture2D(tex1, texCoord + texOffset, TEX_CLAMP_U | TEX_CLAMP_V);


  if(texCoord.x > 0.075f && texCoord.x < 0.17f)
  {
    texColorFont = make_float4(1.0, 0.5, 0.5, 0);
  }
  else if(texCoord.x > 0.2f && texCoord.x < 0.286f)
  {
    texColorFont = make_float4(0.5, 1.0, 0.5, 0);
  }
  
  return texColorFont;
}