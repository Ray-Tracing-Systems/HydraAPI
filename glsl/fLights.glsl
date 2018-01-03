#version 330 core

in vec3 fragColor;
out vec4 outColor;

void main()
{
  outColor = vec4(fragColor, 1.0f);
  //outColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
