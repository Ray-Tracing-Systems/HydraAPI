#version 330

in vec2 fragmentTexCoord;

out vec4 fragColor;


uniform sampler2D colorTexture; 
uniform sampler2D ssaoTexture; 
//uniform sampler2D gDiffuse;
uniform float exposure;

void main(void)
{	
  vec3 color = textureLod(colorTexture, fragmentTexCoord, 0).rgb;
  float ssao = pow(textureLod(ssaoTexture, fragmentTexCoord, 0).r, 8.0f);

  float luminance = dot(color.rgb, vec3(0.2126f, 0.7152f, 0.0722f));
  color *= mix(ssao, 1.0f, luminance);
  color = color + vec3(0.0f, 0.0f, 0.001f);

  const float gamma = 2.2f;
  vec3 toneMapped = vec3(1.0f) - exp(-color * exposure); 
  toneMapped = pow(toneMapped, vec3(1.0f / gamma));

  fragColor = vec4(toneMapped, 1.0f);
}
