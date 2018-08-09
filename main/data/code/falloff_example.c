
float4 mix(float4 x, float4 y, float a)
{
  return x*(1.0f - a) + y*a;
}

float4 main(const SurfaceInfo* sHit, float4 color1, float4 color2)
{
  const float3 pos  = readAttr(sHit,"LocalPos");
  const float3 norm = readAttr(sHit,"Normal");
  
  const float3 rayDir = hr_viewVectorHack;
  float cosAlpha      = fabs(dot(norm,rayDir));
  
  return mix(color1, color2, cosAlpha);
}