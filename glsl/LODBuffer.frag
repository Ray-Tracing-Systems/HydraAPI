#version 330 core
layout (location = 0) out ivec4 lodbuf_1;
layout (location = 1) out ivec4 lodbuf_2;


in VS_OUT
{
  vec2 TexCoords;
  vec3 FragPos;
  vec3 Normal;
} fs_in;


uniform isamplerBuffer materials1;
uniform isamplerBuffer materials2;

uniform int matID;
//uniform ivec2 window_res;
ivec2 window_res = ivec2(1024, 1024);

float mip_map_level(vec2 texture_coordinate)
{
  vec2  dx_vtc        = dFdx(window_res.x * texture_coordinate);
  vec2  dy_vtc        = dFdy(window_res.y * texture_coordinate);
  float delta_max_sqr = max(dot(dx_vtc, dx_vtc), dot(dy_vtc, dy_vtc));

 return 0.5 * log2(delta_max_sqr); 
}


const int texIdBits = 0x00FFFFFF;
const int mipLevelBits = 0xFF000000;


void main()
{     

  ivec4 texIds1 = ivec4(0, 0, 0, 0);
  ivec4 texIds2 = ivec4(0, 0, 0, 0);

  if(matID >= 0)
  {
    int mipLevel = int(floor(mip_map_level(fs_in.TexCoords)));

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
