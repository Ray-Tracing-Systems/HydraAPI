#version 330 core
layout (location = 0) in vec4 vertex;


layout (std140) uniform matrixBuffer
{
	mat4 view;
	mat4 projection;
};
uniform mat4 model;

void main(void)
{
  gl_Position = projection * view * model * vertex;
}
