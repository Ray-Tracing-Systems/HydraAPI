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


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vertex;

	vs_out.TexCoords = texcoords;
	vs_out.FragPos = vec3(model * vertex);
	vs_out.Normal = mat3(transpose(inverse(model))) * normal.xyz;
}
