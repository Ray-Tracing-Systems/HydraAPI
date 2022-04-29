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

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

float mix(float x, float y, float a)
{
  return x * (1.0f - a) + y * a;
}

float4 mix4(float4 x, float4 y, float a)
{
  return make_float4(mix(x.x, y.x, a), mix(x.y, y.y, a), mix(x.z, y.z, a), mix(x.w, y.w, a));
}

float screenPxRange()
{
//  const float pxRange  = 4.0f;
//  float2 unitRange     = float2(pxRange)/float2(481, 249);
//  float2 screenTexSize = float2(1.0f)/fwidth(texCoord);
//  return max(0.5*dot(unitRange, screenTexSize), 1.0);
  return 2.0f;
}

float4 main(const SurfaceInfo* sHit, sampler2D sdfTexture, float2 texScale, float4 inColor)
{
  const float4 transparent_black = make_float4(0.0f, 0.0f, 0.0f, 0.0f);
  const float4 transparent_white = make_float4(1.0f, 1.0f, 1.0f, 0.0f);
  const float2 texCoord = readAttr(sHit,"TexCoord0");
  float2 texCoord_adj = texCoord;
  texCoord_adj.x = fract(texCoord_adj.x * texScale.x);
  texCoord_adj.y = fract(texCoord_adj.y * texScale.y);
  const float4 texColor = texture2D(sdfTexture, texCoord_adj, TEX_CLAMP_U | TEX_CLAMP_V);
  const float distance = median(texColor.x, texColor.y, texColor.z);

//  float screenPxDistance = screenPxRange() * (distance - 0.5);
//  float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
  
  const float smoothing = 1.0f/64.0f;
  float alpha = smoothstep(0.5f - smoothing, 0.5f + smoothing, distance);
  
  //const float4 out_color = (alpha < 0.5f) ? inColor : transparent;
  const float4 out_color = mix4(transparent_white, inColor, alpha);
  return out_color;
}



