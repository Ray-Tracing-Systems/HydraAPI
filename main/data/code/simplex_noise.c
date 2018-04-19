// procedure texture test

float3 floor3(float3 v)
{
  return make_float3(floor(v.x), floor(v.y), floor(v.z));
}

float4 floor4(float4 v)
{
  return make_float4(floor(v.x), floor(v.y), floor(v.z), floor(v.w));
}

float4 abs4(float4 v)
{
  return make_float4(fabs(v.x), fabs(v.y), fabs(v.z), fabs(v.w));
}

// step generates a step function by comparing x to edge.
// For element i of the return value, 0.0 is returned if x[i] < edge[i], and 1.0 is returned otherwise. 
//

float3 step3(float3 edge, float3 v)
{
  const float x = (v.x < edge.x) ? 0.0f : 1.0f;
  const float y = (v.y < edge.y) ? 0.0f : 1.0f;
  const float z = (v.z < edge.z) ? 0.0f : 1.0f;
  
  return make_float3(x, y, z);
}

float4 step4(float4 edge, float4 v)
{
  const float x = (v.x < edge.x) ? 0.0f : 1.0f;
  const float y = (v.y < edge.y) ? 0.0f : 1.0f;
  const float z = (v.z < edge.z) ? 0.0f : 1.0f;
  const float w = (v.w < edge.w) ? 0.0f : 1.0f;
  
  return make_float4(x, y, z, w);
}

float3 mod289(float3 x) {
  return x - floor3(x * (1.0f / 289.0f)) * 289.0f;
}

float4 mod289_4(float4 x) {
  return x - floor4(x * (1.0f / 289.0f)) * 289.0f;
}

float4 permute(float4 x) {
     return mod289_4(((x*34.0f)+1.0f)*x);
}

float4 taylorInvSqrt(float4 r)
{
  return 1.79284291400159f - 0.85373472095314f * r;
}



float snoise(float3 v)
{ 
  const float2 C  = make_float2(1.0f/6.0f, 1.0f/3.0f);
  const float4 D  = make_float4(0.0f, 0.5f, 1.0f, 2.0f);

  const float dt1 = dot(v, make_float3(C.y, C.y, C.y));
  // First corner
  float3 i  = floor(v + make_float3(dt1,dt1,dt1) );
  
  const float dt2 = dot(i, make_float3(C.x, C.x, C.x));
  float3 x0 =   v - i + make_float3(dt2,dt2,dt2);

  // Other corners
  float3 g  = step3(x0.yzx, x0.xyz);
  float3 l  = make_float3(1.0f,1.0f,1.0f) - g;
  float3 i1 = min( g.xyz, l.zxy );
  float3 i2 = max( g.xyz, l.zxy );

  //   x0 = x0 - 0.0 + 0.0 * C.xxx;
  //   x1 = x0 - i1  + 1.0 * C.xxx;
  //   x2 = x0 - i2  + 2.0 * C.xxx;
  //   x3 = x0 - 1.0 + 3.0 * C.xxx;
  float3 x1 = x0 - i1 + C.xxx;
  float3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  float3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

  // Permutations
  i = mod289(i); 
  float4 p = permute( permute( permute( 
             i.z + make_float4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + make_float4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + make_float4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float  n_ = 0.142857142857f; // 1.0/7.0
  float3 ns = n_ * D.wyz - D.xzx;

  float4 j  = p - 49.0f * floor(p * ns.z * ns.z);  //  mod(p,7*7)

  float4 x_ = floor(j * ns.z);
  float4 y_ = floor(j - 7.0f * x_ );    // mod(j,N)

  float4 x = x_ *ns.x + ns.yyyy;
  float4 y = y_ *ns.x + ns.yyyy;
  float4 h = make_float4(1.0f,1.0f,1.0f,1.0f) - abs4(x) - abs4(y);

  float4 b0 = make_float4( x.x, x.y, y.x, x.y );
  float4 b1 = make_float4( x.z, x.w, y.z, x.w );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  float4 s0 = floor(b0)*2.0f + 1.0f;
  float4 s1 = floor(b1)*2.0f + 1.0f;
  float4 sh = -step4(h, make_float4(0.0f,0.0f,0.0f,0.0f));

  float4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  float4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  float3 p0 = make_float3(a0.x, a0.y, h.x);
  float3 p1 = make_float3(a0.z, a0.w, h.y);
  float3 p2 = make_float3(a1.x, a1.y, h.z);
  float3 p3 = make_float3(a1.z, a1.w, h.w);

//Normalise gradients
  float4 norm = taylorInvSqrt(make_float4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  float4 m = max(0.6f - make_float4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0f);
  m = m * m;
  return 42.0f * dot( m*m, make_float4( dot(p0,x0), dot(p1,x1), 
                                        dot(p2,x2), dot(p3,x3) ) );
}


float4 main(const SurfaceInfo* sHit)
{
  const float3 pos     = readAttr(sHit,"WorldPos");
  const float noiseVal = snoise(pos*2.0f);
  return make_float4(noiseVal, noiseVal, noiseVal, 0.0f);
}

