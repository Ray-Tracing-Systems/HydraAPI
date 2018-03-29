#version 330 core
layout (location = 0) in vec2 vertex;

out vec2 fragmentTexCoord;
out vec2 vFragPosition;

const float DEG_TO_RAD = 3.14159265358979323846f / 180.0f;

out vec2 ViewRay;

void main(void)
{
  fragmentTexCoord = vertex * 0.5 + 0.5; 

  vFragPosition = vertex;
  
  ViewRay.x = vertex.x * (16.0f/9.0f) * tan(45.0f * DEG_TO_RAD / 2.0f);
  ViewRay.y = vertex.y * tan(45.0f * DEG_TO_RAD / 2.0f);

  gl_Position = vec4(vertex, 0.0, 1.0);
}
