#version 330 core
layout (location = 0) out vec3 gVertex;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gDiffuse;
layout (location = 3) out vec4 gReflect;


in VS_OUT
{
  vec2 TexCoords;
  vec3 FragPos;
  vec3 Normal;
  mat3 TBN;
} fs_in;

struct simpleMaterial
{
	vec3 diffuseColor;//16 0
	float useBump; // 4 16
    vec3 reflectColor; //16 28 	
	float shininess; //4 4
	mat4 texMatrix; //16*4  32 48 64 80
}; //size: 24 * sizeof(float)


uniform sampler2D diffuseTex;

#define MAX_MATERIALS 256
layout (std140) uniform materialBuffer
{
	simpleMaterial material[MAX_MATERIALS];
};



uniform sampler2D reflectTex;
uniform sampler2D normalTex;
uniform int matID;

void main()
{     
    gVertex = fs_in.FragPos;
    
	if(material[matID].useBump > 0.0f)
	{
		vec3 bumpNormal = texture(normalTex, mat2(material[matID].texMatrix) * fs_in.TexCoords).xyz;
		bumpNormal = 2.0f * bumpNormal - 1.0f;
		gNormal = normalize(fs_in.TBN * bumpNormal);	
	}
	else
		gNormal = normalize(fs_in.Normal);
		
    gDiffuse.rgb = material[matID].diffuseColor * texture(diffuseTex, mat2(material[matID].texMatrix) * fs_in.TexCoords).rgb;
	
    gReflect.rgb = material[matID].reflectColor * texture(reflectTex, mat2(material[matID].texMatrix) * fs_in.TexCoords).rgb;
    gReflect.a = material[matID].shininess;
}
