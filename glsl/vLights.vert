#version 330 core
layout (location = 0) in vec4 vertex;
layout (location = 3) in vec3 color;
layout (location = 4) in mat4 instance_mat;

layout (std140) uniform matrixBuffer
{
	mat4 view;
	mat4 projection;
};

out vec3 fragColor;

void main()
{
	fragColor = color;
	gl_Position = projection * view * instance_mat * vertex;
}
