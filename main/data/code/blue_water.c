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

float4 mix(float4 x, float4 y, float a)
{
  return x*(1.0f - a) + y*a;
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

float ov(float2 p, float iTime) 
{
    float v = 0.0f;
    float a = 0.4f;
    for (int i = 0;i<3;i++) {
        v+= voronoi(p, iTime)*a;
        p*=2.0;
        a*=0.5;
    }
    return v;
}


float4 main(const SurfaceInfo* sHit)
{
  const float3 pos  = readAttr(sHit,"WorldPos");
  
  float4 a  = make_float4(0.20f, 0.4f, 1.0f, 1.0f);
  float4 b  = make_float4(0.85f, 0.9f, 1.0f, 1.0f);
  
  return mix(a, b, smoothstep(0.0f, 0.35f, ov(pos.xy, pos.z)));
}
