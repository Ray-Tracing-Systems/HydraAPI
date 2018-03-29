#version 330 core
layout (location = 0) in vec4 vertex;
layout (location = 1) in vec4 normal;
layout (location = 2) in vec2 texcoords;

out VS_OUT
{
	vec2 TexCoords;
	vec3 FragPos;
	vec3 Normal;
} vs_out;


layout (std140) uniform matrixBuffer
{
	mat4 view;
	mat4 projection;
};

uniform mat4 model;

uniform bool invertNormals;	

void main()
{
	vs_out.TexCoords = texcoords;
	vec4 viewPos = view * model * vertex;
	vs_out.FragPos = viewPos.xyz;
	
	mat3 normalMatrix = mat3(transpose(inverse(view * model)));
	
	vs_out.Normal = normalMatrix * normal.xyz;
	
	vs_out.Normal = invertNormals ? -1.0f * vs_out.Normal : vs_out.Normal;
	
	gl_Position = projection * viewPos;
}


