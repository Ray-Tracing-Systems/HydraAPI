// procedure texture test

float4 userProc(const SurfaceInfo* sHit, sampler2D tex1, __global sampler2D tex2[8], float2 texOffset, float4 inColor)
{
  
  const float2 texCoord = readAttr(sHit,"TexCoord0");
  
  float4 texColorBase = texture2D(tex1, texCoord + texOffset, TEX_CLAMP_U | TEX_CLAMP_V);
  
  float offsetX = 0;
  float offsetY = 0;
  float4 texColorLayout = 1.0f;
  float4 texColorFont = 1.0f;

  if(texCoord.x > 0.163f && texCoord.x < 0.256f && texCoord.y > 0.18f && texCoord.y < 0.815f) // width 0.093
  {
    offsetX = -0.163f;
    offsetY = -0.18f;
    texColorFont = texture2D(tex2[4], make_float2((texCoord.x + offsetX) * 10.7526f, (texCoord.y + offsetY) * 1.5748f), TEX_CLAMP_U | TEX_CLAMP_V);
    
    // texColorLayout = make_float4(1.0f, 0.5f, 0.5f, 0.0f);
  }
  else if(texCoord.x > 0.268f && texCoord.x < 0.361f && texCoord.y > 0.18f && texCoord.y < 0.815f) 
  {
    offsetX = -0.268f;
    offsetY = -0.18f;
    texColorFont = texture2D(tex2[5], make_float2((texCoord.x + offsetX) * 10.7526f, (texCoord.y + offsetY) * 1.5748f), TEX_CLAMP_U | TEX_CLAMP_V);

    // texColorLayout = make_float4(1.0f, 0.5f, 0.5f, 0.0f);
  }
  else if(texCoord.x > 0.379f && texCoord.x < 0.467f && texCoord.y > 0.128f && texCoord.y < 0.508f) // width 0.088f, emblems regions
  {
    offsetX = -0.379f;
    offsetY = -0.128f;
    texColorFont = texture2D(tex2[7], make_float2((texCoord.x + offsetX) * 11.3636f, (texCoord.y + offsetY) * 2.6315f), TEX_CLAMP_U | TEX_CLAMP_V);

    // texColorLayout = make_float4(1.0f, 0.5f, 0.5f, 0.0f);
  }  
    else if(texCoord.x > 0.384f && texCoord.x < 0.452f && texCoord.y > 0.584f && texCoord.y < 0.874f) // width 0.068f, safety check sticker
  {
    offsetX = -0.384f;
    offsetY = -0.584f;
    texColorFont = texture2D(tex2[8], make_float2((texCoord.x + offsetX) * 14.7058f, (texCoord.y + offsetY) * 3.4482f), TEX_CLAMP_U | TEX_CLAMP_V);

    // texColorLayout = make_float4(1.0f, 0.5f, 0.5f, 0.0f);
  }  
  else if(texCoord.x > 0.48f && texCoord.x < 0.573f && texCoord.y > 0.18f && texCoord.y < 0.815f) 
  {
    offsetX = -0.48f;
    offsetY = -0.18f;
    texColorFont = texture2D(tex2[6], make_float2((texCoord.x + offsetX) * 10.7526f, (texCoord.y + offsetY) * 1.5748f), TEX_CLAMP_U | TEX_CLAMP_V);

    // texColorLayout = make_float4(1.0f, 0.5f, 0.5f, 0.0f);
  }
  else if(texCoord.x > 0.619f && texCoord.x < 0.712f && texCoord.y > 0.18f && texCoord.y < 0.815f) 
  {
    offsetX = -0.619f;
    offsetY = -0.18f;
    texColorFont = texture2D(tex2[0], make_float2((texCoord.x + offsetX) * 10.7526f, (texCoord.y + offsetY) * 1.5748f), TEX_CLAMP_U | TEX_CLAMP_V);

    // texColorLayout = make_float4(1.0f, 0.5f, 0.5f, 0.0f);
  }
  else if(texCoord.x > 0.726f && texCoord.x < 0.819f && texCoord.y > 0.18f && texCoord.y < 0.815f) 
  {
    offsetX = -0.726f;
    offsetY = -0.18f;
    texColorFont = texture2D(tex2[1], make_float2((texCoord.x + offsetX) * 10.7526f, (texCoord.y + offsetY) * 1.5748f), TEX_CLAMP_U | TEX_CLAMP_V);
    
    // texColorLayout = make_float4(1.0, 0.5, 0.5, 0);
  }
  else if(texCoord.x > 0.834f && texCoord.x < 0.927f && texCoord.y > 0.18f && texCoord.y < 0.815f) 
  {
    offsetX = -0.834f;
    offsetY = -0.18f;
    texColorFont = texture2D(tex2[2], make_float2((texCoord.x + offsetX) * 10.7526f, (texCoord.y + offsetY) * 1.5748f), TEX_CLAMP_U | TEX_CLAMP_V);
    
    // texColorLayout = make_float4(1.0, 0.5, 0.5, 0);
  }
  

  const float4 texColorOut = texColorBase * texColorFont;// * texColorLayout;
  return texColorOut;
}