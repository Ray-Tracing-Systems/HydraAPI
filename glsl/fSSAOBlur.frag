#version 330 core

in vec2 fragmentTexCoord;

uniform sampler2D ssaoRaw;

out float color;

void main() 
{
    vec2 texelSize = 1.0f / vec2(textureSize(ssaoRaw, 0));
    float result = 0.0f;
    for (int x = -3; x <= 3; ++x) 
    {
        for (int y = -3; y <= 3; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssaoRaw, fragmentTexCoord + offset).r;
        }
    }
	result = result / (7.0f * 7.0f);
    color = result;
}  