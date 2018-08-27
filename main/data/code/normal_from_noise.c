float  clampf(float x, float minval, float maxval) { return max(min(x,maxval),minval); }
int    clampi (int x, int minval, int maxval) { return max(min(x,maxval),minval); }

float smoothstep(float edge0, float edge1, float x)
{
  float  tVal = (x - edge0) / (edge1 - edge0);
  float  t    = fmin(fmax(tVal, 0.0f), 1.0f); 
  return t * t * (3.0f - 2.0f * t);
}

float2 fract2(float2 v)
{
  return v - floor(v);
}

float fract(float v)
{
  return v - floor(v);
}

float2 hash2(float2 p ) 
{
  const float2 p1 = make_float2(dot(p, make_float2(123.4f, 748.6f)), dot(p, make_float2(547.3f, 659.3f)));
  return fract2 (make_float2(sin(p1.x)*5232.85324f, sin(p1.y)*5232.85324f));   
}


//Based off of iq's described here: http://www.iquilezles.org/www/articles/voronoilin
//

float voronoi(float2 p, float iTime) 
{
    float2 n = floor (p);
    float2 f = fract2(p);
    float md = 5.0f;
    float2 m = make_float2(0.0f, 0.0f);
    for (int i = -1;i<=1;i++) {
        for (int j = -1;j<=1;j++) {
            float2 g = make_float2(i, j);
            float2 o = hash2(n+g);
            o = 0.5f+0.5f*sin(iTime+5.038f*o);
            float2 r = g + o - f;
            float d = dot(r, r);
            if (d<md) {
              md = d;
              m = n+g+o;
            }
        }
    }
    return md;
}

float ov(float3 p) 
{
    float v = 0.0f;
    float a = 0.4f;
    for (int i = 0;i<3;i++) {
        v+= voronoi(p.xy, p.z)*a;
        p*=2.0;
        a*=0.5;
    }
    return v;
}

/*

float3 floor3(float3 v)
{
  return make_float3(floor(v.x), floor(v.y), floor(v.z));
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
*/


static inline void CoordinateSystem(float3 v1, __private float3* v2, __private float3* v3)
{
  float invLen = 1.0f;

  if (fabs(v1.x) > fabs(v1.y))
  {
    invLen = 1.0f / sqrt(v1.x*v1.x + v1.z*v1.z);
    (*v2) = make_float3(-v1.z * invLen, 0.0f, v1.x * invLen);
  }
  else
  {
    invLen = 1.0f / sqrt(v1.y*v1.y + v1.z*v1.z);
    (*v2) = make_float3(0.0f, v1.z * invLen, -v1.y * invLen);
  }

  (*v3) = cross(v1, (*v2));
}

float4 main(const SurfaceInfo* sHit, sampler2D texHeight, float2 invTexRes)
{
  const float3 pos  = readAttr(sHit,"WorldPos");
  const float3 norm = readAttr(sHit,"Normal");
  const float3 tang = readAttr(sHit,"Tangent");
  const float3 btan = readAttr(sHit,"Bitangent");
  
  //float3 tang, btan;
  //CoordinateSystem(norm, &tang, &btan);

  const float2 offs0 = make_float2(-1.0f, -1.0f);
  const float2 offs1 = make_float2(0.0f,  -1.0f);
  const float2 offs2 = make_float2(+1.0f, -1.0f);
  const float2 offs3 = make_float2(-1.0f, 0.0f);
  const float2 offs4 = make_float2(+1.0f, 0.0f);
  const float2 offs5 = make_float2(-1.0f, +1.0f);
  const float2 offs6 = make_float2(0.0f,  +1.0f);
  const float2 offs7 = make_float2(+1.0f, +1.0f);

  
  const float h_origin = ov(pos);
  const float scale    = 1.0f;

  float diff[8];

  diff[0] = h_origin - ov(pos + scale*(tang*offs0.x + btan*offs0.y));  
  diff[1] = h_origin - ov(pos + scale*(tang*offs1.x + btan*offs1.y));
  diff[2] = h_origin - ov(pos + scale*(tang*offs2.x + btan*offs2.y));
  diff[3] = h_origin - ov(pos + scale*(tang*offs3.x + btan*offs3.y));
  diff[4] = h_origin - ov(pos + scale*(tang*offs4.x + btan*offs4.y));
  diff[5] = h_origin - ov(pos + scale*(tang*offs5.x + btan*offs5.y));
  diff[6] = h_origin - ov(pos + scale*(tang*offs6.x + btan*offs6.y));
  diff[7] = h_origin - ov(pos + scale*(tang*offs7.x + btan*offs7.y));
 

  const float kScale = 0.1f;
  float3 res = (1.0f / 8.0f)*(make_float3(-diff[0], -diff[0], kScale) + make_float3(0.f, -diff[1], kScale)     + 
                              make_float3(diff[2], -diff[2], kScale)  + make_float3(-diff[3], 0.f, kScale)     + 
                              make_float3(diff[4], 0.f, kScale)       + make_float3(-diff[5], diff[5], kScale) + 
                              make_float3(0.f, diff[6], kScale)       + make_float3(diff[7], diff[7],  kScale));
  
  res.x *= -1.0f;
  res.y *= -1.0f;
  res.z *= +1.0f;

  return StoreNormal(res, NORMAL_IN_TANGENT_SPACE); 
}



