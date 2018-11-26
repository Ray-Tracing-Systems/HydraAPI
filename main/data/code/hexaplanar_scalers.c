float3 abs3(float3 a)
{
  return make_float3(fabs(a.x), fabs(a.y), fabs(a.z));
}

float4 main(const SurfaceInfo* sHit, sampler2D texX1, sampler2D texY1, sampler2D texZ1, sampler2D texX2, sampler2D texY2, sampler2D texZ2, float3 tex_scale, float3 minXYZ, float3 maxXYZ)
{
  const float3 norm = readAttr(sHit,"Normal");
  const float3 pos = readAttr(sHit,"WorldPos");

  float3 normPos = make_float3((pos.x - minXYZ.x) / (maxXYZ.x - minXYZ.x), (pos.y - minXYZ.y) / (maxXYZ.y - minXYZ.y), (pos.z - minXYZ.z) / (maxXYZ.z - minXYZ.z));
  
  float sharpness = 10.0f;
  sampler2D texX = norm.x < 0 ? texX1 : texX2;
  sampler2D texY = norm.y < 0 ? texY1 : texY2;
  sampler2D texZ = norm.z < 0 ? texZ1 : texZ2;


  float3 w = abs3(norm);
  w.x = pow(w.x, sharpness);
  w.y = pow(w.y, sharpness);
  w.z = pow(w.z, sharpness);
  w = max(w, 0.00001) / dot(w, w); 

  float b = (w.x + w.y + w.z);
  w.x = w.x / b;
  w.y = w.y / b;
  w.z = w.z / b;

  //float tex_scale = 1.0f;

  float2 y_uv = make_float2(normPos.x * -1.0f, normPos.z);
  float2 x_uv = make_float2(normPos.z * -1.0f, normPos.y);
  float2 z_uv = make_float2(normPos.x * -1.0f, normPos.y);

  float4 texColX = texture2D(texX, x_uv * tex_scale.x, 0);
  float4 texColY = texture2D(texY, y_uv * tex_scale.y, 0);
  float4 texColZ = texture2D(texZ, z_uv * tex_scale.z, 0);

  float4 res = texColX * w.x + texColY * w.y + texColZ * w.z;

  return res;
}

