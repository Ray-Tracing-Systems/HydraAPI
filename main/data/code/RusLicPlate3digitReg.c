// procedure texture test

float4 userProc(const SurfaceInfo* sHit, sampler2D tex1, __global sampler2D tex2[9], float2 texOffset, float4 inColor)
{
  
  const float2 texCoord = readAttr(sHit,"TexCoord0");
  
  float4 texColorBase = texture2D(tex1, texCoord + texOffset, TEX_CLAMP_U | TEX_CLAMP_V);
  
  float offsetX = 0;
  float offsetY = 0;
  float4 texColorLayout = 1.0f;
  float4 texColorFont = 1.0f;

  if(texCoord.x > 0.062f && texCoord.x < 0.157f && texCoord.y > 0.17f && texCoord.y < 0.67f) // width 0.095
  {
    offsetX = -0.062f;
    offsetY = -0.17f;
    texColorFont = texture2D(tex2[6], make_float2((texCoord.x + offsetX) * 10.52f, (texCoord.y + offsetY) * 2.0f), TEX_CLAMP_U | TEX_CLAMP_V);
    
    // texColorLayout = make_float4(1.0, 0.5, 0.5, 0);
  }
  else if(texCoord.x > 0.17f && texCoord.x < 0.258f && texCoord.y > 0.17f && texCoord.y < 0.81f) // width 0.088
  {
    offsetX = -0.17f;
    offsetY = -0.17f;
    texColorFont = texture2D(tex2[0], make_float2((texCoord.x + offsetX) * 11.36f, (texCoord.y + offsetY) * 1.5625f), TEX_CLAMP_U | TEX_CLAMP_V);

    // texColorLayout = make_float4(1.0, 0.5, 0.5, 0);
  }
  else if(texCoord.x > 0.279f && texCoord.x < 0.367f && texCoord.y > 0.17f && texCoord.y < 0.81f) 
  {
    offsetX = -0.279f;
    offsetY = -0.17f;
    texColorFont = texture2D(tex2[1], make_float2((texCoord.x + offsetX) * 11.36f, (texCoord.y + offsetY) * 1.5625f), TEX_CLAMP_U | TEX_CLAMP_V);

    // texColorLayout = make_float4(1.0, 0.5, 0.5, 0);
  }
  else if(texCoord.x > 0.388f && texCoord.x < 0.476f && texCoord.y > 0.17f && texCoord.y < 0.81f) 
  {
    offsetX = -0.388f;
    offsetY = -0.17f;
    texColorFont = texture2D(tex2[2], make_float2((texCoord.x + offsetX) * 11.36f, (texCoord.y + offsetY) * 1.5625f), TEX_CLAMP_U | TEX_CLAMP_V);

    // texColorLayout = make_float4(1.0, 0.5, 0.5, 0);
  }
  else if(texCoord.x > 0.49f && texCoord.x < 0.585f && texCoord.y > 0.17f && texCoord.y < 0.675f)
  {
    offsetX = -0.49f;
    offsetY = -0.17f;
    texColorFont = texture2D(tex2[7], make_float2((texCoord.x + offsetX) * 10.52, (texCoord.y + offsetY) * 2), TEX_CLAMP_U | TEX_CLAMP_V);
    
    // texColorLayout = make_float4(1.0, 0.5, 0.5, 0);
  }
  else if(texCoord.x > 0.597f && texCoord.x < 0.692f && texCoord.y > 0.17f && texCoord.y < 0.675f) 
  {
    offsetX = -0.597f;
    offsetY = -0.17f;
    texColorFont = texture2D(tex2[8], make_float2((texCoord.x + offsetX) * 10.52, (texCoord.y + offsetY) * 2), TEX_CLAMP_U | TEX_CLAMP_V);
    
    // texColorLayout = make_float4(1.0, 0.5, 0.5, 0);
  }
  else if(texCoord.x > 0.72f && texCoord.x < 0.795f && texCoord.y > 0.365f && texCoord.y < 0.87f) // width 0.075
  {
    offsetX = -0.72f;
    offsetY = -0.365f;
    texColorFont = texture2D(tex2[3], make_float2((texCoord.x + offsetX) * 13.88f, (texCoord.y + offsetY) * 1.98f), TEX_CLAMP_U | TEX_CLAMP_V);

    // texColorLayout = make_float4(1.0, 0.5, 0.5, 0);
  }
  else if(texCoord.x > 0.797f && texCoord.x < 0.872f && texCoord.y > 0.365f && texCoord.y < 0.87f)       
  {
    offsetX = -0.797f;
    offsetY = -0.365f;
    texColorFont = texture2D(tex2[4], make_float2((texCoord.x + offsetX) * 13.88f, (texCoord.y + offsetY) * 1.98f), TEX_CLAMP_U | TEX_CLAMP_V);

    // texColorLayout = make_float4(1.0, 0.5, 0.5, 0);
  }
  else if(texCoord.x > 0.875f && texCoord.x < 0.95f && texCoord.y > 0.365f && texCoord.y < 0.87f)       
  {
    offsetX = -0.875f;
    offsetY = -0.365f;
    texColorFont = texture2D(tex2[5], make_float2((texCoord.x + offsetX) * 13.88f, (texCoord.y + offsetY) * 1.98f), TEX_CLAMP_U | TEX_CLAMP_V);

    // texColorLayout = make_float4(1.0, 0.5, 0.5, 0);
  }


  const float4 texColorOut = texColorBase * texColorFont;// * texColorLayout;
  return texColorOut;
}