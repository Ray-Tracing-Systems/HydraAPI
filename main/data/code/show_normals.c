// procedure texture test

float4 mainNorm(const SurfaceInfo* sHit)
{
  const float3 norm = readAttr(sHit,"Normal");
  return make_float4(fabs(norm.x), fabs(norm.y), fabs(norm.z), 0.0f);
}