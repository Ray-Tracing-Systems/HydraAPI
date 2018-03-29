#version 330 core
layout (location = 0) in vec4 vertex;


uniform mat4 model;

layout (std140) uniform matrixBuffer
{
	mat4 view;
	mat4 projection;
};

void main()
{
	gl_Position = projection * view * model * vertex;
}
