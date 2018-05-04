float3 abs3(float3 a)
{
  return make_float3(fabs(a.x), fabs(a.y), fabs(a.z));
}

float4 abs4(float4 a)
{
  return make_float4(fabs(a.x), fabs(a.y), fabs(a.z), fabs(a.w));
}

float3 floor3(float3 v)
{
  return make_float3(floor(v.x), floor(v.y), floor(v.z));
}

float4 floor4(float4 v)
{
  return make_float4(floor(v.x), floor(v.y), floor(v.z), floor(v.w));
}

float fract(float v)
{
  return v - floor(v);
}

float3 fract3(float3 v)
{
  return v - floor3(v);
}

float4 fract4(float4 v)
{
  return v - floor4(v);
}

float3 mod289f3(float3 x)
{
  return x - floor3(x * (1.0 / 289.0)) * 289.0;
}

float4 mod289f4(float4 x)
{
  return x - floor4(x * (1.0 / 289.0)) * 289.0;
}

float4 permute(float4 x)
{
  return mod289f4(((x*34.0) + 1.0)*x);
}

float4 taylorInvSqrt(float4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float3 fade(float3 t) 
{
  return t*t*t*(t*(t*6.0 - 15.0) + 10.0);
}

float mix(float x, float y, float a)
{
  return x*(1.0f - a) + y*a;
}

float2 mix2(float2 x, float2 y, float a)
{
  return make_float2(mix(x.x, y.x, a), mix(x.y, y.y, a));
}

float3 mix3(float3 x, float3 y, float a)
{
  return make_float3(mix(x.x, y.x, a), mix(x.y, y.y, a), mix(x.z, y.z, a));
}

float4 mix4(float4 x, float4 y, float a)
{
  return make_float4(mix(x.x, y.x, a), mix(x.y, y.y, a), mix(x.z, y.z, a), mix(x.w, y.w, a));
}

float step(float edge, float x)
{
  return x < edge ? 0.0f : 1.0f;
}

float4 step4(float edge, float4 x)
{
  return make_float4(x.x < edge ? 0.0f : 1.0f, x.y < edge ? 0.0f : 1.0f,
                     x.z < edge ? 0.0f : 1.0f, x.w < edge ? 0.0f : 1.0f);
}

float4 step4_(float4 edge, float4 x)
{
  return make_float4(x.x < edge.x ? 0.0f : 1.0f, x.y < edge.y ? 0.0f : 1.0f,
                     x.z < edge.z ? 0.0f : 1.0f, x.w < edge.w ? 0.0f : 1.0f);
}


float rand(float n){return fract(sin(n) * 43758.5453123);}
/*float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
*/

// Classic Perlin noise
float cnoise(float3 P)
{
  float3 Pi0 = floor3(P); // Integer part for indexing
  float3 Pi1 = Pi0 + make_float3(1.0, 1.0, 1.0); // Integer part + 1
  Pi0 = mod289f3(Pi0);
  Pi1 = mod289f3(Pi1);
  float3 Pf0 = fract3(P); // Fractional part for interpolation
  float3 Pf1 = Pf0 - make_float3(1.0, 1.0, 1.0); // Fractional part - 1.0
  float4 ix = make_float4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
  float4 iy = make_float4(Pi0.y, Pi0.y, Pi1.y, Pi1.y);
  float4 iz0 = make_float4(Pi0.z, Pi0.z, Pi0.z, Pi0.z);
  float4 iz1 = make_float4(Pi1.z, Pi1.z, Pi1.z, Pi1.z);

  float4 ixy = permute(permute(ix) + iy);
  float4 ixy0 = permute(ixy + iz0);
  float4 ixy1 = permute(ixy + iz1);

  float4 gx0 = ixy0 * (1.0 / 7.0);
  float4 gy0 = fract4(floor4(gx0) * (1.0 / 7.0)) - 0.5;
  gx0 = fract4(gx0);
  float4 gz0 = make_float4(0.5, 0.5, 0.5, 0.5) - abs4(gx0) - abs4(gy0);
  float4 sz0 = step4_(gz0, make_float4(0.0, 0.0, 0.0, 0.0));
  gx0 -= sz0 * (step4(0.0, gx0) - 0.5);
  gy0 -= sz0 * (step4(0.0, gy0) - 0.5);

  float4 gx1 = ixy1 * (1.0 / 7.0);
  float4 gy1 = fract4(floor4(gx1) * (1.0 / 7.0)) - 0.5;
  gx1 = fract4(gx1);
  float4 gz1 = make_float4(0.5, 0.5, 0.5, 0.5) - abs4(gx1) - abs4(gy1);
  float4 sz1 = step4_(gz1, make_float4(0.0, 0.0, 0.0, 0.0));
  gx1 -= sz1 * (step4_(0.0, gx1) - 0.5);
  gy1 -= sz1 * (step4_(0.0, gy1) - 0.5);

  float3 g000 = make_float3(gx0.x, gy0.x, gz0.x);
  float3 g100 = make_float3(gx0.y, gy0.y, gz0.y);
  float3 g010 = make_float3(gx0.z, gy0.z, gz0.z);
  float3 g110 = make_float3(gx0.w, gy0.w, gz0.w);
  float3 g001 = make_float3(gx1.x, gy1.x, gz1.x);
  float3 g101 = make_float3(gx1.y, gy1.y, gz1.y);
  float3 g011 = make_float3(gx1.z, gy1.z, gz1.z);
  float3 g111 = make_float3(gx1.w, gy1.w, gz1.w);

  float4 norm0 = taylorInvSqrt(make_float4(dot(g000, g000), dot(g010, g010), dot(g100, g100), dot(g110, g110)));
  g000 *= norm0.x;
  g010 *= norm0.y;
  g100 *= norm0.z;
  g110 *= norm0.w;
  float4 norm1 = taylorInvSqrt(make_float4(dot(g001, g001), dot(g011, g011), dot(g101, g101), dot(g111, g111)));
  g001 *= norm1.x;
  g011 *= norm1.y;
  g101 *= norm1.z;
  g111 *= norm1.w;

  float n000 = dot(g000, Pf0);
  float n100 = dot(g100, make_float3(Pf1.x, Pf0.y, Pf0.z));
  float n010 = dot(g010, make_float3(Pf0.x, Pf1.y, Pf0.z));
  float n110 = dot(g110, make_float3(Pf1.x, Pf1.y, Pf0.z));
  float n001 = dot(g001, make_float3(Pf0.x, Pf0.y, Pf1.z));
  float n101 = dot(g101, make_float3(Pf1.x, Pf0.y, Pf1.z));
  float n011 = dot(g011, make_float3(Pf0.x, Pf1.y, Pf1.z));
  float n111 = dot(g111, Pf1);

  float3 fade_xyz = fade(Pf0);
  float4 n_z = mix4(make_float4(n000, n100, n010, n110), make_float4(n001, n101, n011, n111), fade_xyz.z);
  float2 n_yz = mix2(make_float2(n_z.x, n_z.y), make_float2(n_z.z, n_z.w), fade_xyz.y);
  float n_xyz = mix(n_yz.x, n_yz.y, fade_xyz.x);
  return 2.2 * n_xyz;
}

float octave(float3 pos, int octaves, float persistence)
{
  float total = 0.0f;
  float frequency = 10.0f;
  float amplitude = 3.0f;
  float maxValue = 0.0f;

  for (int i = 0; i < octaves; i++)
  {
    total += cnoise(pos * frequency) * amplitude;

    maxValue += amplitude;

    amplitude *= persistence;
    frequency *= 2;
  }

  return total / maxValue;
}


float3 ramp(float3 color1, float3 color2, float3 colorramp, float pos,float pos_r )
{
	if (pos <= pos_r)
	{
		float r = mix(color1.x, colorramp.x, pos/pos_r);
		float g = mix(color1.y, colorramp.y, pos/pos_r);
		float b = mix(color1.z, colorramp.z, pos/pos_r);
		return make_float3(r, g, b);
	}
	else
	{
		float r = mix(colorramp.x, color2.x, (pos-pos_r)/(1-pos_r));
		float g = mix(colorramp.y, color2.y, (pos-pos_r)/(1-pos_r));
		float b = mix(colorramp.z, color2.z, (pos-pos_r)/(1-pos_r));
		return make_float3(r, g, b);
	}

}


float4 main(const SurfaceInfo* sHit, float3 color1, float3 color2)
{
  const float3 pos = readAttr(sHit,"WorldPos");
  
  float n = octave(1.0f * pos, 4, 0.7f);
  float n2 = octave(1.0f * pos, 6, 2.5f);
  
//  float3 color1 = make_float3(0.20f, 0.4f, 1.0f);
//  float3 color2 = make_float3(0.0f, 0.0f, 0.0f);
  float3 color3 = make_float3(1.0f, 1.0f, 1.0f);
  
//  float3 color_res = ramp(color1, color2, color3, n, pos.y);

//  float3 color_res = mix3(mix3(color2, color1, n), color1, step(0.25, pos.y)); // 2.0*pos.y
//  float3 color_res = ramp(mix3(color2, color1, n), color1, color2, pos.y, 0.2);

  n = mix(n, 1.0, step(1.25f * rand(pos.x * pos.z), pos.y));
  //n = mix(n, 1.0, step(1.25f * (n2 + 0.25f), pos.y));
  float3 color_res = mix3(color2, color1, pow(n, 0.04f));

//  return make_float4(n, n, n, 0.0f);
  return make_float4(color_res.x, color_res.y, color_res.z, 0.0f);
}



