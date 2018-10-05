float3 abs3(float3 a)
{
  return make_float3(fabs(a.x), fabs(a.y), fabs(a.z));
}

float to_gray(float3 col)
{
  return col.x * 0.333f + col.y * 0.333f + col.z * 0.333f;
}

float3 blend_weight(float3 norm, float sharpness)
{
  float3 w = abs3(norm);
  w.x = pow(w.x, sharpness);
  w.y = pow(w.y, sharpness);
  w.z = pow(w.z, sharpness);
  w = max(w, 0.00001) / dot(w, w);

  float b = (w.x + w.y + w.z);
  w.x = w.x / b;
  w.y = w.y / b;
  w.z = w.z / b;

  return w;
}

float3 sample_tri_norm(float3 pos, float3 normal, sampler2D texX, sampler2D texY, sampler2D texZ, float tex_scale,
    __global const float4* restrict in_texStorage1, __global const EngineGlobals* restrict in_globals)
{
  float2 y_uv = make_float2(pos.x * tex_scale, pos.z * tex_scale);
  float2 x_uv = make_float2(pos.z * tex_scale, pos.y * tex_scale);
  float2 z_uv = make_float2(pos.x * tex_scale, pos.y * tex_scale);

  float4 texColX = texture2D(texX, x_uv, 0);
  float4 texColY = texture2D(texY, y_uv, 0);
  float4 texColZ = texture2D(texZ, z_uv, 0);

  float sharpness = 10.0f;
  float3 w = blend_weight(normal, sharpness);

  float3 res = make_float3(texColX.x * w.x + texColY.x * w.y + texColZ.x * w.z,
                           texColX.y * w.x + texColY.y * w.y + texColZ.y * w.z,
                           texColX.z * w.x + texColY.z * w.y + texColZ.z * w.z);

  //res = res * 2.0f - 1.0f;
  //res = normalize(res);

  return res;
}

float3 bumpNormal(float3 normal, float3 position, float3 tang, float3 btan, sampler2D texX, sampler2D texY, sampler2D texZ,
    __global const float4* restrict in_texStorage1, __global const EngineGlobals* restrict in_globals)
{
  float scale = 1.0;
  float3 pos = scale * position;

  float tex_scale = 5.0f;

  const float2 offs0 = make_float2(-1.0f, -1.0f);
  const float2 offs1 = make_float2(0.0f,  -1.0f);
  const float2 offs2 = make_float2(+1.0f, -1.0f);
  const float2 offs3 = make_float2(-1.0f, 0.0f);
  const float2 offs4 = make_float2(+1.0f, 0.0f);
  const float2 offs5 = make_float2(-1.0f, +1.0f);
  const float2 offs6 = make_float2(0.0f,  +1.0f);
  const float2 offs7 = make_float2(+1.0f, +1.0f);

  float3 col = sample_tri_norm(pos, normal, texX, texY, texZ, tex_scale, in_texStorage1, in_globals);
  const float h_origin = to_gray(col);

  float diff[8];
  float3 tmp_pos =  pos + scale*(tang*offs0.x + btan*offs0.y);
  diff[0] = h_origin - to_gray(sample_tri_norm(tmp_pos, normal, texX, texY, texZ, tex_scale, in_texStorage1, in_globals));
  tmp_pos = pos + scale*(tang*offs1.x + btan*offs1.y);
  diff[1] = h_origin - to_gray(sample_tri_norm(tmp_pos, normal, texX, texY, texZ, tex_scale, in_texStorage1, in_globals));
  tmp_pos = pos + scale*(tang*offs2.x + btan*offs2.y);
  diff[2] = h_origin - to_gray(sample_tri_norm(tmp_pos, normal, texX, texY, texZ, tex_scale, in_texStorage1, in_globals));
  tmp_pos = pos + scale*(tang*offs3.x + btan*offs3.y);
  diff[3] = h_origin - to_gray(sample_tri_norm(tmp_pos, normal, texX, texY, texZ, tex_scale, in_texStorage1, in_globals));
  tmp_pos = pos + scale*(tang*offs4.x + btan*offs4.y);
  diff[4] = h_origin - to_gray(sample_tri_norm(tmp_pos, normal, texX, texY, texZ, tex_scale, in_texStorage1, in_globals));
  tmp_pos = pos + scale*(tang*offs5.x + btan*offs5.y);
  diff[5] = h_origin - to_gray(sample_tri_norm(tmp_pos, normal, texX, texY, texZ, tex_scale, in_texStorage1, in_globals));
  tmp_pos = pos + scale*(tang*offs6.x + btan*offs6.y);
  diff[6] = h_origin - to_gray(sample_tri_norm(tmp_pos, normal, texX, texY, texZ, tex_scale, in_texStorage1, in_globals));
  tmp_pos = pos + scale*(tang*offs7.x + btan*offs7.y);
  diff[7] = h_origin - to_gray(sample_tri_norm(tmp_pos, normal, texX, texY, texZ, tex_scale, in_texStorage1, in_globals));

  const float kScale = 0.00190625f;
  float3 res = (1.0f / 8.0f)*(make_float3(-diff[0], -diff[0], kScale) + make_float3(0.f, -diff[1], kScale)     +
                              make_float3(diff[2], -diff[2], kScale)  + make_float3(-diff[3], 0.f, kScale)     +
                              make_float3(diff[4], 0.f, kScale)       + make_float3(-diff[5], diff[5], kScale) +
                              make_float3(0.f, diff[6], kScale)       + make_float3(diff[7], diff[7],  kScale));

  res.x = -1.0f * res.x;
  res.y = -1.0f * res.y;
  res.z = +1.0f * res.z;

  return 0.01f * make_float3(0.0f, 0.0f, 1.0f) + 0.99f * res;
}

float4 main(const SurfaceInfo* sHit, sampler2D texX1, sampler2D texY1, sampler2D texZ1, sampler2D texX2, sampler2D texY2, sampler2D texZ2)
{
  const float3 norm = readAttr(sHit,"Normal");
  const float3 pos = readAttr(sHit,"WorldPos");
  const float3 tang = readAttr(sHit,"Tangent");
  const float3 btan = readAttr(sHit,"Bitangent");

  float sharpness = 10.0f;
  sampler2D texX = norm.x < 0 ? texX1 : texX2;
  sampler2D texY = norm.y < 0 ? texY1 : texY2;
  sampler2D texZ = norm.z < 0 ? texZ1 : texZ2;

  float3 N = bumpNormal(norm, pos, tang, btan, texX, texY, texZ, in_texStorage1, in_globals);

  return StoreNormal(N, NORMAL_IN_TANGENT_SPACE);
}

