#version 330 core

layout (std140) uniform matrixBuffer
{
	mat4 view;
	mat4 projection;
};

uniform sampler2D gVertex;
uniform sampler2D gNormal;
uniform sampler2D gDiffuse;
uniform sampler2D gReflect;
//uniform sampler2D ssao;
uniform sampler2D intensityTex;

 
struct Light
{
    vec3 pos;    //16 0
	float mult;  //4 12
	
    vec3 color; //16 16
    // 1 - point, 2 - ies, ...
	int type; //4 28
	
    vec3 normal; //16 32
	
    vec3 right; //16 48
};

layout (std140) uniform lightBuffer
{
	Light light;
};

uniform ivec2 screenSize;

out vec4 outColor;

const float PI       = 3.14159265358979323846f;
const float INV_PI   = 1.0f/PI;
const int Point     = 1;
const int IES       = 2;


vec2 getTexCoords()
{
  return gl_FragCoord.xy / screenSize;
}

float CalcLinearZ(float depth) {
    const float zFar = 1000000.0f;
    const float zNear = 0.1f;

    // bias it from [0, 1] to [-1, 1]
    float linear = zNear / (zFar - depth * (zFar - zNear)) * zFar;

    return (linear * 2.0) - 1.0;
}

vec3 CalcEyeFromWindow(vec3 windowSpace)
{
	vec3 ndcPos;
	ndcPos.xy = ((2.0 * windowSpace.xy)) / (screenSize) - 1;
	ndcPos.z = (2.0 * windowSpace.z - 1.0f);

	vec4 clipPos;
	clipPos.w = projection[3][2] / (ndcPos.z - (projection[2][2] / projection[2][3]));
	clipPos.xyz = ndcPos * clipPos.w;

	return (inverse(projection) * clipPos).xyz;
}


float sampleLDT(vec3 d)
{
// calculate s-coordinate
  float cosi = -dot(light.normal, d);
  float phii = -min(acos(cosi) - PI, 0) / PI;

// calculate t-coordinate
  vec3 up = cross(light.normal , light.right);
  vec3 dn = normalize(dot(-d, light.right) * light.right + dot(-d, up) * up);

  float cosr = dot(light.right, dn);
  float W = 0.5 * (acos(cosr) * INV_PI);
  float cosu = -dot(up, d);
  float f = max(0, sign(cosu));
  float texV = (1 - W) * f + W * (1 - f);


  return texture(intensityTex, vec2(texV, 1 - phii)).r;
}

/*
float CalcViewZ(vec2 Coords)
{
    float Depth = texture(gDepth, Coords).x;
    float ViewZ = projection[3][2] / (2 * Depth -1 - projection[2][2]);
    return ViewZ;
}*/




void main()
{
  vec2 fragmentTexCoord = getTexCoords();

   /*float ViewZ = CalcViewZ(fragmentTexCoord);

    float ViewX = ViewRay.x * ViewZ;
    float ViewY = ViewRay.y * ViewZ;
*/
    //vec3 fragPos = vec3(ViewX, ViewY, ViewZ);
  vec3 fragPos = texture(gVertex, fragmentTexCoord).rgb;
 // vec3 fragPos = CalcEyeFromWindow(vec3(gl_FragCoord.xy, texture(gDepth, fragmentTexCoord).x)).xyz;//texture(gVertex, fragmentTexCoord).rgb;
  //vec3 fragPos = calculate_view_position(fragmentTexCoord, texture(gDepth, fragmentTexCoord).x);
  vec3 normal = texture(gNormal, fragmentTexCoord).rgb;
  
  vec3 diffuseColor = texture(gDiffuse, fragmentTexCoord).rgb;
 
  vec3 reflectColor = texture(gReflect, fragmentTexCoord).rgb;
  float shininess = texture(gReflect, fragmentTexCoord).a;
  

  vec3 lightPos = vec3(view * vec4(light.pos, 1.0f));
  vec3 viewDir  = normalize(-fragPos);

  vec3 lightDir = lightPos - fragPos;
  float distance = length(lightDir);

  lightDir = normalize(lightDir);
  vec3 halfwayDir = normalize(lightDir + viewDir);

  float kd = max(dot(normal, lightDir), 0.0f);
  float ks = pow(max(dot(normal, halfwayDir), 0.0f), shininess);

  float attenuation = distance * distance;
  attenuation = max(1.0, attenuation);

  float intensity = 1.0f;

  if(light.type == IES)
    intensity = sampleLDT(lightDir);

  vec3 lightColor = (intensity * light.color * light.mult) / (attenuation);

  vec3 diffuse = lightColor * kd * diffuseColor;
  vec3 specular = lightColor * ks * reflectColor;
  vec3 color = diffuse + specular;

  outColor = vec4(color, 1.0f);
}
