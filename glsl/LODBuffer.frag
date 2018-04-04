#version 330 core
layout (location = 0) out uvec4 lodbuf_1;
layout (location = 1) out uvec4 lodbuf_2;


in VS_OUT
{
  vec2 TexCoords;
  vec3 FragPos;
  vec3 Normal;
} fs_in;


uniform usamplerBuffer materials1;
uniform usamplerBuffer materials2;

uniform int matID;
uniform ivec2 window_res;
//ivec2 window_res = ivec2(1024, 1024);

#define MAX_MIP_LEVEL 10

float mip_map_level(vec2 texture_coordinate)
{
  vec2  dx_vtc        = dFdx(4 * window_res.x * texture_coordinate);
  vec2  dy_vtc        = dFdy(4 * window_res.y * texture_coordinate);
  float delta_max_sqr = max(dot(dx_vtc, dx_vtc), dot(dy_vtc, dy_vtc));

  const float maxClamp = pow(2, MAX_MIP_LEVEL * 2);
  delta_max_sqr = clamp(delta_max_sqr, 0.0, maxClamp);

  return 0.5 * log2(delta_max_sqr);
}


const uint texIdBits    = 0x00FFFFFFu;
const uint mipLevelBits = 0xFF000000u;


void main()
{     

  uvec4 texIds1 = uvec4(0, 0, 0, 0);
  uvec4 texIds2 = uvec4(0, 0, 0, 0);

  if(matID >= 0)
  {
    uint mipLevel = uint(floor(mip_map_level(fs_in.TexCoords)));

    texIds1 = texelFetch(materials1, matID);
    texIds2 = texelFetch(materials2, matID);

    texIds1.r = (texIds1.r & texIdBits) | (mipLevel << 24);
    texIds1.g = (texIds1.g & texIdBits) | (mipLevel << 24);
    texIds1.b = (texIds1.b & texIdBits) | (mipLevel << 24);
    texIds1.a = (texIds1.a & texIdBits) | (mipLevel << 24);

    texIds2.r = (texIds2.r & texIdBits) | (mipLevel << 24);
    texIds2.g = (texIds2.g & texIdBits) | (mipLevel << 24);
    texIds2.b = (texIds2.b & texIdBits) | (mipLevel << 24);
    texIds2.a = (texIds2.a & texIdBits) | (mipLevel << 24);

  }

  lodbuf_1 = texIds1;
  lodbuf_2 = texIds2;

}
