#version 330 core

in VS_OUT
{
  vec2 TexCoords;
  vec3 FragPos;
  vec3 Normal;
} fs_in;


out vec4 outColor;


struct pointLight
{
    vec3 pos;
    vec3 color;
    float mult;
};

struct simpleMaterial
{
    vec3 diffuse;
    vec3 reflect;

    float shininess;

    sampler2D diffuseTex;
    sampler2D reflectTex;
};

#define MAX_LIGHTS 128

uniform vec3 viewPos;
uniform pointLight pointLights[MAX_LIGHTS];
uniform int numLights;
uniform simpleMaterial material;


float checker(vec2 pos, float repeats)
{
  float cx = floor(repeats * pos.x);
  float cy = floor(repeats * pos.y);
  float result = mod(cx + cy, 2.0f);

  return sign(result);
}


void main()
{
  vec3 color;

  /*float n = mix(0.0f, 1.0f, checker(fs_in.TexCoords.xy, 36.0f));
  color = vec3(n);*/

  vec3 ambient = vec3(0.15f);

  vec3 viewDir = normalize(viewPos - fs_in.FragPos);
  color = ambient;
  for(int i = 0; i < numLights; ++i)
  {
    vec3 lightDir = normalize(pointLights[i].pos - fs_in.FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float kd = max(dot(fs_in.Normal, lightDir), 0.0f);

    float ks = pow(max(dot(fs_in.Normal, halfwayDir), 0.0f), material.shininess);

    vec3 diffuse = pointLights[i].color * pointLights[i].mult * kd * material.diffuse * texture(material.diffuseTex, fs_in.TexCoords).xyz;

    vec3 specular = pointLights[i].color * pointLights[i].mult * ks * material.reflect * texture(material.reflectTex, fs_in.TexCoords).x;

    color += diffuse + specular;

  }


  outColor = vec4(color, 1.0f);
}

