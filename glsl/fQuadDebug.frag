#version 330

in vec2 fragmentTexCoord;

out vec4 fragColor;


uniform isampler2D debugTex;


vec3 colorMap(float f)
{
  const float dx = 0.8;
  f = clamp(f, 0.0, 1.0f);
  float g = (6.0f - 2.0f * dx) * f + dx;

  float red = max(0.0f, (3.0f - abs(g - 4.0f) - abs(g - 5.0f))/2.0f);
  float green = max(0.0f, (4.0f - abs(g - 2.0f) - abs(g - 4.0f))/2.0f);
  float blue = max(0.0f, (3.0f - abs(g - 1.0f) - abs(g - 2.0f))/2.0f);

  return vec3(red, green, blue);
}


const int texIdBits = 0x00FFFFFF;
const int mipLevelBits = 0xFF000000;


void main(void)
{
  ivec4 val = texture(debugTex, fragmentTexCoord);

  int diffuseMipLevel = val.g >> 24;
  int diffuseTexId = val.g & texIdBits;

  //fragColor = vec4(colorMap(color.x/10.0f), 1.0f);
  fragColor = vec4(colorMap(diffuseMipLevel/10.0f), 1.0f); //color.rgb
}
