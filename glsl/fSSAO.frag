#version 330 core

in vec2 fragmentTexCoord;
in vec2 vFragPosition;

out float color;


layout (std140) uniform matrixBuffer
{
	mat4 view;
	mat4 projection;
};

uniform sampler2D texNoise;
uniform samplerBuffer randomSamples;

uniform sampler2D gVertex;
uniform sampler2D gNormal;


layout (std140) uniform ssaoSettingsBuffer
{
	ivec2 screenSize;
	int kernelSize;
	float radius;
};

const float bias = 0.075;

in vec2 ViewRay;

float CalcViewZ(vec2 Coords)
{
    float Depth = texture(gVertex, Coords).x;
    float ViewZ = projection[3][2] / (2 * Depth -1 - projection[2][2]);
    return ViewZ;
}

void main()
{
	vec2 noiseScale = vec2(screenSize)/4.0f; 
	
	
	float ViewZ = CalcViewZ(fragmentTexCoord);

    float ViewX = ViewRay.x * ViewZ;
    float ViewY = ViewRay.y * ViewZ;

    vec3 fragPos = vec3(ViewX, ViewY, ViewZ);
	
	//vec3 fragPos = texture(gVertex, fragmentTexCoord).xyz;
	vec3 normal = normalize(texture(gNormal, fragmentTexCoord).rgb);
	
	vec3 randomVec = normalize(texture(texNoise, fragmentTexCoord * noiseScale).xyz);
	
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = normalize(cross(normal, tangent));
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	for(int i = 0; i < kernelSize; ++i)
	{ 
		float s = float(i) / kernelSize;
		//vec3 sample = TBN * texture(randomSamples, s).rgb;
		vec3 sample = vec3(texelFetch(randomSamples, i * 3 + 0).r, texelFetch(randomSamples, i * 3 + 1).r, texelFetch(randomSamples, i * 3 + 2).r);
		sample = TBN * sample;
		
		//vec3 sample = TBN * gKernel[i];
		sample = fragPos + sample * radius; 
		
		vec4 offset = vec4(sample, 1.0);
		offset = projection * offset; 
		offset.xyz /= offset.w; 
		offset.xyz = offset.xyz * 0.5 + 0.5; 
		
		//float sampleDepth = texture(gVertex, offset.xy).z;
		float sampleDepth = CalcViewZ(offset.xy);

		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;  
	}
	
	occlusion = 1.0 - (occlusion / kernelSize);

	color = occlusion;
}