float3 abs3(float3 a)
{
  return make_float3(fabs(a.x), fabs(a.y), fabs(a.z));
}

float4 main(const SurfaceInfo* sHit, sampler2D texX, sampler2D texY, sampler2D texZ)
{
  const float3 norm = readAttr(sHit,"Normal");
  const float3 pos = readAttr(sHit,"WorldPos");

  float sharpness = 10.0f;

  float3 w = abs3(norm);
  w.x = pow(w.x, sharpness);
  w.y = pow(w.y, sharpness);
  w.z = pow(w.z, sharpness);
  w = max(w, 0.00001) / dot(w, w); 

  float b = (w.x + w.y + w.z);
  w.x = w.x / b;
  w.y = w.y / b;
  w.z = w.z / b;

  float tex_scale = 10.0f;

  float2 y_uv = make_float2(pos.x * tex_scale, pos.z * tex_scale);
  float2 x_uv = make_float2(pos.z * tex_scale, pos.y * tex_scale);
  float2 z_uv = make_float2(pos.x * tex_scale, pos.y * tex_scale);

  float4 texColX = texture2D(texX, x_uv, 0);
  float4 texColY = texture2D(texY, y_uv, 0);
  float4 texColZ = texture2D(texZ, z_uv, 0);

  float4 res = texColX * w.x + texColY * w.y + texColZ * w.z;

  return res;
}

