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
uniform samplerBuffer materials_matrix;

uniform int matID;
uniform ivec2 max_tex_res;
uniform ivec2 render_res;
uniform ivec2 rasterization_res;

#define MAX_MIP_LEVEL 4

float mip_map_level(vec2 texture_coordinate)
{
  vec2  dx_vtc        = (rasterization_res.x / float(render_res.x)) * max_tex_res.x * dFdx(texture_coordinate);
  vec2  dy_vtc        = (rasterization_res.y / float(render_res.y)) * max_tex_res.y * dFdy(texture_coordinate);

  float dx2           = dot(dx_vtc, dx_vtc);
  float dy2           = dot(dy_vtc, dy_vtc);  
  float delta_max_sqr = max(dx2, dy2);

  const float maxClamp = pow(2.0f, MAX_MIP_LEVEL * 2.0f);
  delta_max_sqr = clamp(delta_max_sqr, 1.0f, maxClamp);

  return 0.5f * log2(delta_max_sqr);
}


const uint texIdBits    = 0x00FFFFFFu;
const uint mipLevelBits = 0xFF000000u;

mat2 getTexMatrix(int matId, int slotId)
{
  vec4 tex_mat = texelFetch(materials_matrix, matId * 8 + slotId);
  mat2 res;
  res[0][0] = tex_mat.x;
  res[0][1] = tex_mat.y;
  res[1][0] = tex_mat.z;
  res[0][1] = tex_mat.w;

  return res;
}


void main()
{     

  uvec4 texIds1 = uvec4(0, 0, 0, 0);
  uvec4 texIds2 = uvec4(0, 0, 0, 0);
  uint mipLevel = 0u;

  if(matID >= 0)
  {
    texIds1 = texelFetch(materials1, matID);
    texIds2 = texelFetch(materials2, matID);

    mipLevel = uint(floor(mip_map_level(fs_in.TexCoords * getTexMatrix(matID, 0))));
    texIds1.r = (texIds1.r & texIdBits) | (mipLevel << 24);

    mipLevel = uint(floor(mip_map_level(fs_in.TexCoords * getTexMatrix(matID, 1))));
    texIds1.g = (texIds1.g & texIdBits) | (mipLevel << 24);

    mipLevel = uint(floor(mip_map_level(fs_in.TexCoords * getTexMatrix(matID, 2))));
    texIds1.b = (texIds1.b & texIdBits) | (mipLevel << 24);

    mipLevel = uint(floor(mip_map_level(fs_in.TexCoords * getTexMatrix(matID, 3))));
    texIds1.a = (texIds1.a & texIdBits) | (mipLevel << 24);

    mipLevel = uint(floor(mip_map_level(fs_in.TexCoords * getTexMatrix(matID, 4))));
    texIds2.r = (texIds2.r & texIdBits) | (mipLevel << 24);

    mipLevel = uint(floor(mip_map_level(fs_in.TexCoords * getTexMatrix(matID, 5))));
    texIds2.g = (texIds2.g & texIdBits) | (mipLevel << 24);

    mipLevel = uint(floor(mip_map_level(fs_in.TexCoords * getTexMatrix(matID, 6))));
    texIds2.b = (texIds2.b & texIdBits) | (mipLevel << 24);

    mipLevel = uint(floor(mip_map_level(fs_in.TexCoords * getTexMatrix(matID, 7))));
    texIds2.a = (texIds2.a & texIdBits) | (mipLevel << 24);

  }

  lodbuf_1 = texIds1;
  lodbuf_2 = texIds2;

}
