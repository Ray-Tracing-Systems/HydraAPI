float4 main(const SurfaceInfo* sHit)
{
  const float ao = readAttr(sHit,"AO");  
  return make_float4(ao, ao, ao, 0.0f);
}
