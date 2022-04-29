float clamp(float x, float minVal, float maxVal)
{
    return min(max(x, minVal), maxVal);
}

float fract(float x)
{
    return x - floor(x);
}


float smoothstep(float edge0, float edge1, float x)
{
  const float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
  return t * t * (3.0f - 2.0f * t);
}

float mix(float x, float y, float a)
{
  return x * (1.0f - a) + y * a;
}

float4 mix4(float4 x, float4 y, float a)
{
  return make_float4(mix(x.x, y.x, a), mix(x.y, y.y, a), mix(x.z, y.z, a), mix(x.w, y.w, a));
}

/*
float4 main_1(const SurfaceInfo* sHit, sampler2D sdfTexture, float4 inColor)
{
  const float4 transparent = make_float4(0.0f, 0.0f, 0.0f, 0.0f);
  const float2 texCoord = readAttr(sHit,"TexCoord0");
  const float4 texColor = texture2D(sdfTexture, texCoord, TEX_CLAMP_U | TEX_CLAMP_V);
  const float alpha = texColor.w;
  
  //const float4 out_color = (alpha < 0.5f) ? inColor : transparent;
  
  return (alpha < 0.5f) ? inColor : transparent;
}*/

float4 main(const SurfaceInfo* sHit, sampler2D sdfTexture, float2 texScale, float4 inColor)
{
  const float4 transparent_black = make_float4(0.0f, 0.0f, 0.0f, 0.0f);
  const float4 transparent_white = make_float4(1.0f, 1.0f, 1.0f, 0.0f);
  const float2 texCoord = readAttr(sHit,"TexCoord0");
  float2 texCoord_adj = texCoord;
  texCoord_adj.x = fract(texCoord_adj.x * texScale.x);
  texCoord_adj.y = fract(texCoord_adj.y * texScale.y);
  const float4 texColor = texture2D(sdfTexture, texCoord_adj, TEX_CLAMP_U | TEX_CLAMP_V);
  const float distance = texColor.x;
  
  const float smoothing = 1.0f/64.0f;
  float alpha = smoothstep(0.5f - smoothing, 0.5f + smoothing, distance);
  
  //const float4 out_color = (alpha < 0.5f) ? inColor : transparent;
  const float4 out_color = mix4(transparent_white, inColor, alpha);
  return out_color;
}



