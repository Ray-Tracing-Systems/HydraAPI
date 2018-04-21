// procedure texture test

// https://www.shadertoy.com/view/4sfGzS

float3 floor3(float3 v)
{
  return make_float3(floor(v.x), floor(v.y), floor(v.z));
}

float fract(float v)
{
  return v - floor(v);
}

float3 fract3(float3 v)
{
  return v - floor(v);
}

float mix(float x, float y, float a)
{
  return x*(1.0f - a) + y*a;
}

float hash(float3 p)  // replace this by something better
{
    p  = fract3( p*0.3183099f + make_float3(0.1f,0.1f,0.1f) );
	p *= 17.0;
    return fract( p.x*p.y*p.z*(p.x+p.y+p.z) );
}

float noise(float3 x )
{
    float3 p = floor3(x);
    float3 f = fract3(x);
    f = f*f*(make_float3(3.0f,3.0f,3.0f) - 2.0f*f);
	
    return mix(mix(mix( hash(p + make_float3(0,0,0)), 
                        hash(p + make_float3(1,0,0)),f.x),
                   mix( hash(p + make_float3(0,1,0)), 
                        hash(p + make_float3(1,1,0)),f.x),f.y),
               mix(mix( hash(p + make_float3(0,0,1)), 
                        hash(p + make_float3(1,0,1)),f.x),
                   mix( hash(p + make_float3(0,1,1)), 
                        hash(p + make_float3(1,1,1)),f.x),f.y),f.z);
}


float4 main(const SurfaceInfo* sHit)
{
  const float3 pos     = readAttr(sHit,"WorldPos");
  //const float noiseVal = noise(pos*10.0f);
 
  const float3x3 m = make_float3x3(make_float3(0.00f,  0.80f,  0.60f),
                                   make_float3(-0.80f,  0.36f, -0.48f),
                                   make_float3(-0.60f, -0.48f,  0.64f));
   
  float f = 0.0f;
  
  float3 q = 10.0f*pos;
  f  = 0.5000f*noise( q ); q = mul3x3x3(m, q*2.01f);
  f += 0.2500f*noise( q ); q = mul3x3x3(m, q*2.02f);
  f += 0.1250f*noise( q ); q = mul3x3x3(m, q*2.03f);
  f += 0.0625f*noise( q ); q = mul3x3x3(m, q*2.01f);
  
  return make_float4(f, f, f, 0.0f);
}

