// procedure texture test

float4 userProc(const SurfaceInfo* sHit, __global sampler2D texArr[2], float4 inColor)
{
  const float2 texCoord  = readAttr(sHit,"TexCoord0");
  const sampler2D texSam = (texCoord.x < 0.5f) ? texArr[0] : texArr[1];
  
  return inColor*texture2D(texSam, texCoord, TEX_CLAMP_U | TEX_CLAMP_V);
}