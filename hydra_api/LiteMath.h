#ifndef VFLOAT4_ALL_H
#define VFLOAT4_ALL_H

#ifdef WIN32
  #include "vfloat4_x64.h"
#else
  #include "vfloat4_gcc.h"
#endif 

// This is just and example. 
// In practise you may take any of these files that you prefer for your platform.  
// Or may use one of them or make yourself impl

//#include "vfloat4_gcc.h"
//#include "vfloat4_x64.h"

// __mips__
// __ppc__ 

#include <cmath>
#include <initializer_list>

namespace LiteMath
{ 
  const float EPSILON    = 1e-6f;
  #ifndef DEG_TO_RAD
  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;
  #endif
  
  typedef unsigned int uint;

  struct uint4
  {
    inline uint4()                               : x(0), y(0), z(0), w(0) {}
    inline uint4(uint a, uint b, uint c, uint d) : x(a), y(b), z(c), w(d) {}
    inline explicit uint4(uint a[4])             : x(a[0]), y(a[1]), z(a[2]), w(a[3]) {}

    inline uint4(const std::initializer_list<uint> a_v) { v = cvex::load_u(a_v.begin()); }
    inline uint4(cvex::vuint4 rhs) { v = rhs; }
    inline uint4 operator=(cvex::vuint4 rhs) { v = rhs; return *this; }

    inline uint& operator[](uint i)       { return M[i]; }
    inline uint  operator[](uint i) const { return M[i]; }

    inline uint4 operator+(const uint4& b) const { return v + b.v; }
    inline uint4 operator-(const uint4& b) const { return v - b.v; }
    inline uint4 operator*(const uint4& b) const { return v * b.v; }
    inline uint4 operator/(const uint4& b) const { return v / b.v; }

    inline uint4 operator+(const uint rhs) const { return v + rhs; }
    inline uint4 operator-(const uint rhs) const { return v - rhs; }
    inline uint4 operator*(const uint rhs) const { return v * rhs; }
    inline uint4 operator/(const uint rhs) const { return v / rhs; }

    union
    {
      struct { uint x, y, z, w; };
      uint  M[4];
      cvex::vuint4 v;
    };
  };

  static inline uint4 operator+(const uint a, const uint4 b) 
  { 
    const cvex::vuint4 res = (a + b.v);
    return uint4(res); 
  }

  static inline uint4 operator-(const uint a, const uint4 b) 
  { 
    const cvex::vuint4 res = (a - b.v);
    return uint4(res); 
  }

  static inline uint4 operator*(const uint a, const uint4 b) 
  { 
    const cvex::vuint4 res = (a * b.v);
    return uint4(res); 
  }

  static inline uint4 operator/(const uint a, const uint4 b) 
  { 
    const cvex::vuint4 res = (a / b.v);
    return uint4(res); 
  }

  static inline uint4 load   (const uint* p)       { return cvex::load(p);      }
  static inline uint4 load_u (const uint* p)       { return cvex::load_u(p);    }
  static inline void store   (uint* p, uint4 a_val) { cvex::store  (p, a_val.v); }
  static inline void store_u (uint* p, uint4 a_val) { cvex::store_u(p, a_val.v); }

  static inline uint4 operator&(const uint4 a, const uint4 b) { return uint4(a.v & b.v); }
  static inline uint4 operator|(const uint4 a, const uint4 b) { return uint4(a.v | b.v); }
  static inline uint4 operator~(const uint4 a)                { return uint4(~a.v); }

 //static inline uint4 operator>>(const uint4 a, const uint b) { return uint4(a.v >> b); }
 //static inline uint4 operator<<(const uint4 a, const uint b) { return uint4(a.v << b); }

  static inline uint4 min  (const uint4& a,   const uint4& b) { return cvex::min(a.v, b.v); }
  static inline uint4 max  (const uint4& a,   const uint4& b) { return cvex::max(a.v, b.v); }
  static inline uint4 clamp(const uint4& a_x, const uint4& a_min, const uint4& a_max) { return cvex::clamp(a_x.v, a_min.v, a_max.v); }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  struct int4
  {
    inline int4()                           : x(0), y(0), z(0), w(0) {}
    inline int4(int a, int b, int c, int d) : x(a), y(b), z(c), w(d) {}
    inline explicit int4(int a[4])          : x(a[0]), y(a[1]), z(a[2]), w(a[3]) {}

    inline int4(const std::initializer_list<int> a_v) { v = cvex::load_u(a_v.begin()); }
    inline int4(cvex::vint4 rhs) { v = rhs; }
    inline int4 operator=(cvex::vint4 rhs) { v = rhs; return *this; }

    inline int& operator[](int i)       { return M[i]; }
    inline int  operator[](int i) const { return M[i]; }

    inline int4 operator+(const int4& b) const { return v + b.v; }
    inline int4 operator-(const int4& b) const { return v - b.v; }
    inline int4 operator*(const int4& b) const { return v * b.v; }
    inline int4 operator/(const int4& b) const { return v / b.v; }

    inline int4 operator+(const int rhs) const { return v + rhs; }
    inline int4 operator-(const int rhs) const { return v - rhs; }
    inline int4 operator*(const int rhs) const { return v * rhs; }
    inline int4 operator/(const int rhs) const { return v / rhs; }

    union
    {
      struct { int x, y, z, w; };
      int  M[4];
      cvex::vint4 v;
    };
  };

  static inline int4 operator+(const int a, const int4 b) 
  { 
    const cvex::vint4 res = (a + b.v);
    return int4(res); 
  }

  static inline int4 operator-(const int a, const int4 b) 
  { 
    const cvex::vint4 res = (a - b.v);
    return int4(res); 
  }

  static inline int4 operator*(const int a, const int4 b) 
  { 
    const cvex::vint4 res = (a * b.v);
    return int4(res); 
  }

  static inline int4 operator/(const int a, const int4 b) 
  { 
    const cvex::vint4 res = (a / b.v);
    return int4(res); 
  }

  static inline int4 load   (const int* p)       { return cvex::load(p);      }
  static inline int4 load_u (const int* p)       { return cvex::load_u(p);    }
  static inline void store  (int* p, int4 a_val) { cvex::store  (p, a_val.v); }
  static inline void store_u(int* p, int4 a_val) { cvex::store_u(p, a_val.v); }

  static inline int4 operator&(const int4 a, const int4 b) { return int4(a.v & b.v); }
  static inline int4 operator|(const int4 a, const int4 b) { return int4(a.v | b.v); }
  static inline int4 operator~(const int4 a)               { return int4(~a.v); }

  static inline int4 operator>>(const int4 a, const int b) { return int4(a.v >> b); }
  static inline int4 operator<<(const int4 a, const int b) { return int4(a.v << b); }

  static inline cvex::vuint4 to_uint32 (const int4& a)  { return cvex::to_uint32(a.v); }
  static inline int4         to_uint32 (const uint4& a) { return int4(cvex::to_int32(a.v)); }

  static inline int4 min(const int4& a, const int4& b) { return cvex::min(a.v, b.v); }
  static inline int4 max(const int4& a, const int4& b) { return cvex::max(a.v, b.v); }
  static inline int4 clamp(const int4& a_x, const int4& a_min, const int4& a_max) { return cvex::clamp(a_x.v, a_min.v, a_max.v); }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  struct float4
  {
    inline float4() : x(0), y(0), z(0), w(0) {}
    inline float4(float a, float b, float c, float d) : x(a), y(b), z(c), w(d) {}
    inline explicit float4(float a[4]) : x(a[0]), y(a[1]), z(a[2]), w(a[3]) {}

    inline float4(const std::initializer_list<float> a_v) { v = cvex::load_u(a_v.begin()); }
    inline float4          (cvex::vfloat4 rhs)            { v = rhs; }
    inline float4 operator=(cvex::vfloat4 rhs)            { v = rhs; return *this; }
    
    inline float& operator[](int i)       { return M[i]; }
    inline float  operator[](int i) const { return M[i]; }

    inline float4 operator+(const float4& b) const { return v + b.v; }
    inline float4 operator-(const float4& b) const { return v - b.v; }
    inline float4 operator*(const float4& b) const { return v * b.v; }
    inline float4 operator/(const float4& b) const { return v / b.v; }

    inline float4& operator+=(const float4& b) { v = v + b.v; return *this; }
    inline float4& operator-=(const float4& b) { v = v - b.v; return *this; }
    inline float4& operator*=(const float4& b) { v = v * b.v; return *this; }
    inline float4& operator/=(const float4& b) { v = v / b.v; return *this; }

    inline float4 operator+(const float rhs) const { return v + rhs; }
    inline float4 operator-(const float rhs) const { return v - rhs; }
    inline float4 operator*(const float rhs) const { return v * rhs; }
    inline float4 operator/(const float rhs) const { return v / rhs; }

    inline float4& operator+=(const float rhs) { v = v + rhs; return *this; }
    inline float4& operator-=(const float rhs) { v = v - rhs; return *this; }
    inline float4& operator*=(const float rhs) { v = v * rhs; return *this; }
    inline float4& operator/=(const float rhs) { v = v / rhs; return *this; }

    inline cvex::vint4 operator> (const float4& b) const { return (v > b.v); }
    inline cvex::vint4 operator< (const float4& b) const { return (v < b.v); }
    inline cvex::vint4 operator>=(const float4& b) const { return (v >= b.v); }
    inline cvex::vint4 operator<=(const float4& b) const { return (v <= b.v); }
    inline cvex::vint4 operator==(const float4& b) const { return (v == b.v); }
    inline cvex::vint4 operator!=(const float4& b) const { return (v != b.v); }

    union
    {
      struct {float x, y, z, w; };
      float  M[4];
      cvex::vfloat4 v;
    };
  };

  static inline float4 operator+(const float a, const float4 b) 
  { 
    const cvex::vfloat4 res = (a + b.v);
    return float4(res); 
  }

  static inline float4 operator-(const float a, const float4 b) 
  { 
    const cvex::vfloat4 res = (a - b.v);
    return float4(res); 
  }

  static inline float4 operator*(const float a, const float4 b) 
  { 
    const cvex::vfloat4 res = (a * b.v);
    return float4(res); 
  }

  static inline float4 operator/(const float a, const float4 b) 
  { 
    const cvex::vfloat4 res = (a / b.v);
    return float4(res); 
  }

  static inline float4 load   (const float* p)         { return cvex::load(p); }
  static inline float4 load_u (const float* p)         { return cvex::load_u(p); }
  static inline void   store  (float* p, float4 a_val) { cvex::store  (p, a_val.v); }
  static inline void   store_u(float* p, float4 a_val) { cvex::store_u(p, a_val.v); }

  static inline int4   to_int32  (const float4& a) { return cvex::to_int32(a.v); }
  static inline uint4  to_uint32 (const float4& a) { return cvex::to_uint32(a.v); }
  static inline float4 to_float32(const  int4& a)  { return cvex::to_float32(a.v); }
  static inline float4 to_float32(const uint4& a)  { return cvex::to_float32(a.v); }

  static inline float4 as_float32(const int4 a_val)   { return cvex::as_float32(a_val.v); }
  static inline float4 as_float32(const uint4 a_val)  { return cvex::as_float32(a_val.v); }
  static inline int4   as_int32  (const float4 a_val) { return cvex::as_int32  (a_val.v); }
  static inline uint4  as_uint32 (const float4 a_val) { return cvex::as_uint32 (a_val.v); }

  static inline float4 min  (const float4& a, const float4& b)                            { return cvex::min(a.v, b.v); }
  static inline float4 max  (const float4& a, const float4& b)                            { return cvex::max(a.v, b.v); }
  static inline float4 clamp(const float4& x, const float4& minVal, const float4& maxVal) { return cvex::clamp(x.v, minVal.v, maxVal.v); }
  static inline float4 clamp(const float4& x, const float minVal, const float maxVal)     { return cvex::clamp(x.v, cvex::splat(minVal), cvex::splat(maxVal)); }
  static inline float4 lerp (const float4& u, const float4& v, const float t)             { return cvex::lerp(u.v, v.v, t); }

  static inline float  dot3f(const float4& a, const float4& b) { return cvex::dot3f(a.v, b.v); }
  static inline float4 dot3v(const float4& a, const float4& b) { return cvex::dot3v(a.v, b.v); }
  static inline float  dot4f(const float4& a, const float4& b) { return cvex::dot4f(a.v, b.v); }
  static inline float  dot  (const float4& a, const float4& b) { return cvex::dot4f(a.v, b.v); }
  static inline float4 dot4v(const float4& a, const float4& b) { return cvex::dot4v(a.v, b.v); }
  static inline float4 cross3(const float4& a, const float4& b){ return cvex::cross3(a.v, b.v);} 

  static inline float  length3f(const float4& a) { return cvex::length3f(a.v); }
  static inline float  length4f(const float4& a) { return cvex::length4f(a.v); }
  static inline float4 length3v(const float4& a) { return cvex::length3v(a.v); }
  static inline float4 length4v(const float4& a) { return cvex::length4v(a.v); }

  static inline float4 floor(const float4 a_val) { return cvex::floor(a_val.v); }
  static inline float4 ceil (const float4 a_val) { return cvex::ceil(a_val.v);  }
  static inline float4 fabs (const float4& a)    { return cvex::fabs (a.v);} 
  static inline float4 rcp_e(const float4& a)    { return cvex::rcp_e(a.v);     }
  
  static inline unsigned int color_pack_rgba(const float4 rel_col) { return cvex::color_pack_rgba(rel_col.v); }
  static inline unsigned int color_pack_bgra(const float4 rel_col) { return cvex::color_pack_bgra(rel_col.v); }

  static inline float extract_0(const float4& a_val) { return cvex::extract_0(a_val.v); }
  static inline float extract_1(const float4& a_val) { return cvex::extract_1(a_val.v); }
  static inline float extract_2(const float4& a_val) { return cvex::extract_2(a_val.v); }
  static inline float extract_3(const float4& a_val) { return cvex::extract_3(a_val.v); }

  static inline float4 splat_0(const float4& v)      { return cvex::splat_0(v.v); }
  static inline float4 splat_1(const float4& v)      { return cvex::splat_1(v.v); }
  static inline float4 splat_2(const float4& v)      { return cvex::splat_2(v.v); }
  static inline float4 splat_3(const float4& v)      { return cvex::splat_3(v.v); }  

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  struct float3
  {
    inline float3() : x(0), y(0), z(0) {}
    inline float3(float a, float b, float c) : x(a), y(b), z(c) {}
    inline explicit float3(const float* ptr) : x(ptr[0]), y(ptr[1]), z(ptr[2]) {}

    inline float& operator[](int i)       { return M[i]; }
    inline float  operator[](int i) const { return M[i]; }

    union
    {
      struct {float x, y, z; };
      float M[3];
    };
  };

  struct float2
  {
    inline float2() : x(0), y(0) {}
    inline float2(float a, float b) : x(a), y(b) {}
    inline explicit float2(float a[2]) : x(a[0]), y(a[1]) {}

    inline float& operator[](int i)       { return M[i]; }
    inline float  operator[](int i) const { return M[i]; }

    union
    {
      struct {float x, y; };
      float M[2];
    };
  };
  
  static inline int4   make_int4  (int a, int b, int c, int d)         { return int4{a,b,c,d};   }
  static inline float4 make_float4(float a, float b, float c, float d) { return float4{a,b,c,d}; }
  static inline float3 make_float3(float a, float b, float c)          { return float3{a,b,c};   }
  static inline float2 make_float2(float a, float b)                   { return float2{a,b};     }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  /**
  \brief this class use colmajor memory layout for effitient vector-matrix operations
  */
  struct float4x4
  {
    inline float4x4()  { identity(); }

    inline explicit float4x4(const float A[16])
    {
      m_col[0] = float4{ A[0], A[4], A[8],  A[12] };
      m_col[1] = float4{ A[1], A[5], A[9],  A[13] };
      m_col[2] = float4{ A[2], A[6], A[10], A[14] };
      m_col[3] = float4{ A[3], A[7], A[11], A[15] };
    }

    inline void identity()
    {
      m_col[0] = float4{ 1.0f, 0.0f, 0.0f, 0.0f };
      m_col[1] = float4{ 0.0f, 1.0f, 0.0f, 0.0f };
      m_col[2] = float4{ 0.0f, 0.0f, 1.0f, 0.0f };
      m_col[3] = float4{ 0.0f, 0.0f, 0.0f, 1.0f };
    }

    inline float4x4 operator*(const float4x4& rhs) const
    {
      // transpose will change multiplication order (due to in fact we use column major)
      //
      float4x4 res;
      cvex::mat4_rowmajor_mul_mat4((float*)res.m_col, (const float*)rhs.m_col, (const float*)m_col); 
      return res;
    }

    inline float4 get_col(int i) const                { return m_col[i]; }
    inline void   set_col(int i, const float4& a_col) { m_col[i] = a_col; }

    inline float4 get_row(int i) const { return float4{ m_col[0][i], m_col[1][i], m_col[2][i], m_col[3][i] }; }
    inline void   set_row(int i, const float4& a_col)
    {
      m_col[0][i] = a_col[0];
      m_col[1][i] = a_col[1];
      m_col[2][i] = a_col[2];
      m_col[3][i] = a_col[3];
    }

    inline float4& col(int i)       { return m_col[i]; }
    inline float4  col(int i) const { return m_col[i]; }

    inline float& operator()(int row, int col)       { return m_col[col][row]; }
    inline float  operator()(int row, int col) const { return m_col[col][row]; }

    inline void StoreRowMajor(float A[16]) const
    {
      float4x4 temp;
      cvex::transpose4((const cvex::vfloat4*)m_col, (cvex::vfloat4*)&temp);

      store_u(A + 0,  temp.col(0) );
      store_u(A + 4,  temp.col(1) );
      store_u(A + 8,  temp.col(2) );
      store_u(A + 12, temp.col(3) );
    }

    inline void StoreColMajor(float A[16]) const
    {
      store_u(A + 0,  m_col[0]);
      store_u(A + 4,  m_col[1]);
      store_u(A + 8,  m_col[2]);
      store_u(A + 12, m_col[3]);
    }

    inline const float* L() const // #WARNING: THIS IS LEGACY FUNCTION, PLEASE DO NOT USE IT!!!
    {
      static constexpr int TEMPDATASIZE = 16;
      static float4x4 g_tempLinearData[TEMPDATASIZE];
      static int tempCounter = 0;

      const float* resAddr = (const float*)(g_tempLinearData + tempCounter);

      cvex::transpose4((const cvex::vfloat4*)this, (cvex::vfloat4*)resAddr);
        
      tempCounter++;
      if(tempCounter >= TEMPDATASIZE)
        tempCounter = 0;

      return resAddr;
    }

    inline float* L() // #WARNING: THIS IS LEGACY FUNCTION, PLEASE DO NOT USE IT!!!
    {
      static constexpr int TEMPDATASIZE = 16;
      static float4x4 g_tempLinearData[TEMPDATASIZE];
      static int tempCounter = 0;

      float* resAddr = (float*)(g_tempLinearData + tempCounter);

      cvex::transpose4((const cvex::vfloat4*)this, (cvex::vfloat4*)resAddr);
        
      tempCounter++;
      if(tempCounter >= TEMPDATASIZE)
        tempCounter = 0;

      return resAddr;
    }

    float4 m_col[4];
  };

  static inline float2 to_float2(float4 v)          { return float2{v.x, v.y}; }
  static inline float2 to_float2(float3 v)          { return float2{v.x, v.y}; }
  static inline float3 to_float3(float4 v)          { return float3{v.x, v.y, v.z}; }
  static inline float4 to_float4(float3 v, float w) { return float4{v.x, v.y, v.z, w}; }

  static inline float4 operator*(const float4x4& m, const float4& v)
  {
    float4 res;
    cvex::mat4_colmajor_mul_vec4((float*)&res, (const float*)&m, (const float*)&v);
    return res;
  }

  static inline float4x4 mul(const float4x4& m1, const float4x4& m2) { return m1*m2; }
  static inline float3   mul(const float4x4& m1, const float3& v)    { return to_float3(m1*to_float4(v,1.0f)); }
  static inline float3   mul4x3(const float4x4& m1, const float3& v) { return to_float3(m1*to_float4(v,1.0f)); }
  static inline float3   mul3x3(float4x4 m, float3 v)
  {
    float4x4 m2 = m;
    m2.set_col(3, float4{0,  0,  0, 1.0f });
    return to_float3(m2*to_float4(v, 1.0f));
  }

  static inline float4x4 transpose(const float4x4& rhs)
  {
    float4x4 res;
    cvex::transpose4((const cvex::vfloat4*)&rhs, (cvex::vfloat4*)&res);
    return res;
  }

  static inline float4x4 translate4x4(float3 t)
  {
    float4x4 res;
    res.set_col(3, float4{t.x,  t.y,  t.z, 1.0f });
    return res;
  }

  static inline float4x4 scale4x4(float3 t)
  {
    float4x4 res;
    res.set_col(0, float4{t.x, 0.0f, 0.0f,  0.0f});
    res.set_col(1, float4{0.0f, t.y, 0.0f,  0.0f});
    res.set_col(2, float4{0.0f, 0.0f,  t.z, 0.0f});
    res.set_col(3, float4{0.0f, 0.0f, 0.0f, 1.0f});
    return res;
  }

  static inline float4x4 rotate4x4X(float phi)
  {
    float4x4 res;
    res.set_col(0, float4{1.0f,      0.0f,       0.0f, 0.0f  });
    res.set_col(1, float4{0.0f, +cosf(phi),  +sinf(phi), 0.0f});
    res.set_col(2, float4{0.0f, -sinf(phi),  +cosf(phi), 0.0f});
    res.set_col(3, float4{0.0f,      0.0f,       0.0f, 1.0f  });
    return res;
  }

  static inline float4x4 rotate4x4Y(float phi)
  {
    float4x4 res;
    res.set_col(0, float4{+cosf(phi), 0.0f, -sinf(phi), 0.0f});
    res.set_col(1, float4{     0.0f, 1.0f,      0.0f, 0.0f  });
    res.set_col(2, float4{+sinf(phi), 0.0f, +cosf(phi), 0.0f});
    res.set_col(3, float4{     0.0f, 0.0f,      0.0f, 1.0f  });
    return res;
  }

  static inline float4x4 rotate4x4Z(float phi)
  {
    float4x4 res;
    res.set_col(0, float4{+cosf(phi), sinf(phi), 0.0f, 0.0f});
    res.set_col(1, float4{-sinf(phi), cosf(phi), 0.0f, 0.0f});
    res.set_col(2, float4{     0.0f,     0.0f, 1.0f, 0.0f  });
    res.set_col(3, float4{     0.0f,     0.0f, 0.0f, 1.0f  });
    return res;
  }
  
  static inline float4x4 inverse4x4(float4x4 m1)
  {
    CVEX_ALIGNED(16) float tmp[12]; // temp array for pairs
    float4x4 m;

    // calculate pairs for first 8 elements (cofactors)
    //
    tmp[0]  = m1(2,2) * m1(3,3);
    tmp[1]  = m1(2,3) * m1(3,2);
    tmp[2]  = m1(2,1) * m1(3,3);
    tmp[3]  = m1(2,3) * m1(3,1);
    tmp[4]  = m1(2,1) * m1(3,2);
    tmp[5]  = m1(2,2) * m1(3,1);
    tmp[6]  = m1(2,0) * m1(3,3);
    tmp[7]  = m1(2,3) * m1(3,0);
    tmp[8]  = m1(2,0) * m1(3,2);
    tmp[9]  = m1(2,2) * m1(3,0);
    tmp[10] = m1(2,0) * m1(3,1);
    tmp[11] = m1(2,1) * m1(3,0);

    // calculate first 8 m1.rowents (cofactors)
    //
    m(0,0) = tmp[0]  * m1(1,1) + tmp[3] * m1(1,2) + tmp[4]  * m1(1,3);
    m(0,0) -= tmp[1] * m1(1,1) + tmp[2] * m1(1,2) + tmp[5]  * m1(1,3);
    m(1,0) = tmp[1]  * m1(1,0) + tmp[6] * m1(1,2) + tmp[9]  * m1(1,3);
    m(1,0) -= tmp[0] * m1(1,0) + tmp[7] * m1(1,2) + tmp[8]  * m1(1,3);
    m(2,0) = tmp[2]  * m1(1,0) + tmp[7] * m1(1,1) + tmp[10] * m1(1,3);
    m(2,0) -= tmp[3] * m1(1,0) + tmp[6] * m1(1,1) + tmp[11] * m1(1,3);
    m(3,0) = tmp[5]  * m1(1,0) + tmp[8] * m1(1,1) + tmp[11] * m1(1,2);
    m(3,0) -= tmp[4] * m1(1,0) + tmp[9] * m1(1,1) + tmp[10] * m1(1,2);
    m(0,1) = tmp[1]  * m1(0,1) + tmp[2] * m1(0,2) + tmp[5]  * m1(0,3);
    m(0,1) -= tmp[0] * m1(0,1) + tmp[3] * m1(0,2) + tmp[4]  * m1(0,3);
    m(1,1) = tmp[0]  * m1(0,0) + tmp[7] * m1(0,2) + tmp[8]  * m1(0,3);
    m(1,1) -= tmp[1] * m1(0,0) + tmp[6] * m1(0,2) + tmp[9]  * m1(0,3);
    m(2,1) = tmp[3]  * m1(0,0) + tmp[6] * m1(0,1) + tmp[11] * m1(0,3);
    m(2,1) -= tmp[2] * m1(0,0) + tmp[7] * m1(0,1) + tmp[10] * m1(0,3);
    m(3,1) = tmp[4]  * m1(0,0) + tmp[9] * m1(0,1) + tmp[10] * m1(0,2);
    m(3,1) -= tmp[5] * m1(0,0) + tmp[8] * m1(0,1) + tmp[11] * m1(0,2);

    // calculate pairs for second 8 m1.rowents (cofactors)
    //
    tmp[0]  = m1(0,2) * m1(1,3);
    tmp[1]  = m1(0,3) * m1(1,2);
    tmp[2]  = m1(0,1) * m1(1,3);
    tmp[3]  = m1(0,3) * m1(1,1);
    tmp[4]  = m1(0,1) * m1(1,2);
    tmp[5]  = m1(0,2) * m1(1,1);
    tmp[6]  = m1(0,0) * m1(1,3);
    tmp[7]  = m1(0,3) * m1(1,0);
    tmp[8]  = m1(0,0) * m1(1,2);
    tmp[9]  = m1(0,2) * m1(1,0);
    tmp[10] = m1(0,0) * m1(1,1);
    tmp[11] = m1(0,1) * m1(1,0);

    // calculate second 8 m1 (cofactors)
    //
    m(0,2) = tmp[0]   * m1(3,1) + tmp[3]  * m1(3,2) + tmp[4]  * m1(3,3);
    m(0,2) -= tmp[1]  * m1(3,1) + tmp[2]  * m1(3,2) + tmp[5]  * m1(3,3);
    m(1,2) = tmp[1]   * m1(3,0) + tmp[6]  * m1(3,2) + tmp[9]  * m1(3,3);
    m(1,2) -= tmp[0]  * m1(3,0) + tmp[7]  * m1(3,2) + tmp[8]  * m1(3,3);
    m(2,2) = tmp[2]   * m1(3,0) + tmp[7]  * m1(3,1) + tmp[10] * m1(3,3);
    m(2,2) -= tmp[3]  * m1(3,0) + tmp[6]  * m1(3,1) + tmp[11] * m1(3,3);
    m(3,2) = tmp[5]   * m1(3,0) + tmp[8]  * m1(3,1) + tmp[11] * m1(3,2);
    m(3,2) -= tmp[4]  * m1(3,0) + tmp[9]  * m1(3,1) + tmp[10] * m1(3,2);
    m(0,3) = tmp[2]   * m1(2,2) + tmp[5]  * m1(2,3) + tmp[1]  * m1(2,1);
    m(0,3) -= tmp[4]  * m1(2,3) + tmp[0]  * m1(2,1) + tmp[3]  * m1(2,2);
    m(1,3) = tmp[8]   * m1(2,3) + tmp[0]  * m1(2,0) + tmp[7]  * m1(2,2);
    m(1,3) -= tmp[6]  * m1(2,2) + tmp[9]  * m1(2,3) + tmp[1]  * m1(2,0);
    m(2,3) = tmp[6]   * m1(2,1) + tmp[11] * m1(2,3) + tmp[3]  * m1(2,0);
    m(2,3) -= tmp[10] * m1(2,3) + tmp[2]  * m1(2,0) + tmp[7]  * m1(2,1);
    m(3,3) = tmp[10]  * m1(2,2) + tmp[4]  * m1(2,0) + tmp[9]  * m1(2,1);
    m(3,3) -= tmp[8]  * m1(2,1) + tmp[11] * m1(2,2) + tmp[5]  * m1(2,0);

    // calculate matrix inverse
    //
    const float k = 1.0f / (m1(0,0) * m(0,0) + m1(0,1) * m(1,0) + m1(0,2) * m(2,0) + m1(0,3) * m(3,0));
    const float4 vK{k,k,k,k};

    m.set_col(0, m.get_col(0)*vK);
    m.set_col(1, m.get_col(1)*vK);
    m.set_col(2, m.get_col(2)*vK);
    m.set_col(3, m.get_col(3)*vK);

    return m;
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  static inline float clamp(float u, float a, float b) { const float r = fmax(a, u);      return fmin(r, b); }

  static inline int max  (int a, int b)        { return a > b ? a : b; }                                    
  static inline int min  (int a, int b)        { return a < b ? a : b; }                                    
  static inline int clamp(int u, int a, int b) { const int   r = (a > u) ? a : u; return (r < b) ? r : b; } 

  inline float rnd(float s, float e)
  {
    const float t = (float)(rand()) / (float)RAND_MAX;
    return s + t*(e - s);
  }

  template<typename T> inline T SQR(T x) { return x * x; }

  inline bool isfinite(float x) { return std::isfinite(x); }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  //**********************************************************************************
  // float3 operators and functions
  //**********************************************************************************
  static inline float3 operator * (const float3 & u, float v) { return float3{u.x * v, u.y * v, u.z * v}; }
  static inline float3 operator / (const float3 & u, float v) { return float3{u.x / v, u.y / v, u.z / v}; }
  static inline float3 operator + (const float3 & u, float v) { return float3{u.x + v, u.y + v, u.z + v}; }
  static inline float3 operator - (const float3 & u, float v) { return float3{u.x - v, u.y - v, u.z - v}; }
  static inline float3 operator * (float v, const float3 & u) { return float3{v * u.x, v * u.y, v * u.z}; }
  static inline float3 operator / (float v, const float3 & u) { return float3{v / u.x, v / u.y, v / u.z}; }
  static inline float3 operator + (float v, const float3 & u) { return float3{u.x + v, u.y + v, u.z + v}; }
  static inline float3 operator - (float v, const float3 & u) { return float3{u.x - v, u.y - v, u.z - v}; }

  static inline float3 operator + (const float3 & u, const float3 & v) { return float3{u.x + v.x, u.y + v.y, u.z + v.z}; }
  static inline float3 operator - (const float3 & u, const float3 & v) { return float3{u.x - v.x, u.y - v.y, u.z - v.z}; }
  static inline float3 operator * (const float3 & u, const float3 & v) { return float3{u.x * v.x, u.y * v.y, u.z * v.z}; }
  static inline float3 operator / (const float3 & u, const float3 & v) { return float3{u.x / v.x, u.y / v.y, u.z / v.z}; }

  static inline float3 operator - (const float3 & u) { return {-u.x, -u.y, -u.z}; }

  static inline float3 & operator += (float3 & u, const float3 & v) { u.x += v.x; u.y += v.y; u.z += v.z; return u; }
  static inline float3 & operator -= (float3 & u, const float3 & v) { u.x -= v.x; u.y -= v.y; u.z -= v.z; return u; }
  static inline float3 & operator *= (float3 & u, const float3 & v) { u.x *= v.x; u.y *= v.y; u.z *= v.z; return u; }
  static inline float3 & operator /= (float3 & u, const float3 & v) { u.x /= v.x; u.y /= v.y; u.z /= v.z; return u; }

  static inline float3 & operator += (float3 & u, float v) { u.x += v; u.y += v; u.z += v; return u; }
  static inline float3 & operator -= (float3 & u, float v) { u.x -= v; u.y -= v; u.z -= v; return u; }
  static inline float3 & operator *= (float3 & u, float v) { u.x *= v; u.y *= v; u.z *= v; return u; }
  static inline float3 & operator /= (float3 & u, float v) { u.x /= v; u.y /= v; u.z /= v; return u; }
  static inline bool     operator == (const float3 & u, const float3 & v) { return (::fabs(u.x - v.x) < EPSILON) && (::fabs(u.y - v.y) < EPSILON) && (::fabs(u.z - v.z) < EPSILON); }
  
  static inline float3 lerp(const float3 & u, const float3 & v, float t) { return u + t * (v - u); }
  static inline float  dot(const float3 & u, const float3 & v) { return (u.x*v.x + u.y*v.y + u.z*v.z); }
  static inline float3 cross(const float3 & u, const float3 & v) { return float3{u.y*v.z - u.z*v.y, u.z*v.x - u.x*v.z, u.x*v.y - u.y*v.x}; }
  static inline float3 clamp(const float3 & u, float a, float b) { return float3{clamp(u.x, a, b), clamp(u.y, a, b), clamp(u.z, a, b)}; }

  static inline float  length(const float3 & u) { return sqrtf(SQR(u.x) + SQR(u.y) + SQR(u.z)); }
  static inline float  lengthSquare(const float3 u) { return u.x*u.x + u.y*u.y + u.z*u.z; }
  static inline float3 normalize(const float3 & u) { return u / length(u); }

  static inline float  maxcomp(const float3 & u) { return fmax(u.x, fmax(u.y, u.z)); }
  static inline float  mincomp(const float3 & u) { return fmin(u.x, fmin(u.y, u.z)); }

  //**********************************************************************************
  // float2 operators and functions
  //**********************************************************************************

  static inline float2 operator * (const float2 & u, float v) { return float2{u.x * v, u.y * v}; }
  static inline float2 operator / (const float2 & u, float v) { return float2{u.x / v, u.y / v}; }
  static inline float2 operator * (float v, const float2 & u) { return float2{v * u.x, v * u.y}; }
  static inline float2 operator / (float v, const float2 & u) { return float2{v / u.x, v / u.y}; }

  static inline float2 operator + (const float2 & u, const float2 & v) { return float2{u.x + v.x, u.y + v.y}; }
  static inline float2 operator - (const float2 & u, const float2 & v) { return float2{u.x - v.x, u.y - v.y}; }
  static inline float2 operator * (const float2 & u, const float2 & v) { return float2{u.x * v.x, u.y * v.y}; }
  static inline float2 operator / (const float2 & u, const float2 & v) { return float2{u.x / v.x, u.y / v.y}; }
  static inline float2 operator - (const float2 & v) { return {-v.x, -v.y}; }

  static inline float2 & operator += (float2 & u, const float2 & v) { u.x += v.x; u.y += v.y; return u; }
  static inline float2 & operator -= (float2 & u, const float2 & v) { u.x -= v.x; u.y -= v.y; return u; }
  static inline float2 & operator *= (float2 & u, const float2 & v) { u.x *= v.x; u.y *= v.y; return u; }
  static inline float2 & operator /= (float2 & u, const float2 & v) { u.x /= v.x; u.y /= v.y; return u; }

  static inline float2 & operator += (float2 & u, float v) { u.x += v; u.y += v; return u; }
  static inline float2 & operator -= (float2 & u, float v) { u.x -= v; u.y -= v; return u; }
  static inline float2 & operator *= (float2 & u, float v) { u.x *= v; u.y *= v; return u; }
  static inline float2 & operator /= (float2 & u, float v) { u.x /= v; u.y /= v; return u; }
  static inline bool     operator == (const float2 & u, const float2 & v) { return (::fabs(u.x - v.x) < EPSILON) && (::fabs(u.y - v.y) < EPSILON); }

  static inline float2 lerp(const float2 & u, const float2 & v, float t) { return u + t * (v - u); }
  static inline float  dot(const float2 & u, const float2 & v)   { return (u.x*v.x + u.y*v.y); }
  static inline float2 clamp(const float2 & u, float a, float b) { return float2{clamp(u.x, a, b), clamp(u.y, a, b)}; }

  static inline float  length(const float2 & u)    { return sqrtf(SQR(u.x) + SQR(u.y)); }
  static inline float2 normalize(const float2 & u) { return u / length(u); }

  static inline float  lerp(float u, float v, float t) { return u + t * (v - u); }

  static inline float3 operator*(const float4x4& m, const float3& v)
  {
    float4 v2 = to_float4(v, 1.0f);
    float4 res;
    cvex::mat4_colmajor_mul_vec4((float*)&res, (const float*)&m, (const float*)&v2);
    return to_float3(res);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  struct int3
  {
      int3() :x(0), y(0), z(0) {}
      int3(int a, int b, int c) : x(a), y(b), z(c) {}

      int x, y, z;
  };

  struct uint3
  {
      uint3() :x(0), y(0), z(0) {}
      uint3(unsigned int a, unsigned int b, unsigned int c) : x(a), y(b), z(c) {}

      unsigned int x, y, z;
  };

  struct int2
  {
    int2() : x(0), y(0) {}
    int2(int a, int b) : x(a), y(b) {}

    int x, y;
  };

  struct uint2
  {
    uint2() : x(0), y(0) {}
    uint2(unsigned int a, unsigned int b) : x(a), y(b) {}

    bool operator==(const uint2 &other) const { return (x == other.x && y == other.y) || (x == other.y && y == other.x); }

    unsigned int x, y;
  };

  struct ushort2
  {
    ushort2() : x(0), y(0) {}
    ushort2(unsigned short a, unsigned short b) : x(a), y(b) {}

    unsigned short x, y;
  };

  struct ushort4
  {
    ushort4() :x(0), y(0), z(0), w(0) {}
    ushort4(unsigned short a, unsigned short b, unsigned short c, unsigned short d) : x(a), y(b), z(c), w(d) {}

    unsigned short x, y, z, w;
  };

  struct uchar4
  {
    uchar4() :x(0), y(0), z(0), w(0) {}
    uchar4(unsigned char a, unsigned char b, unsigned char c, unsigned char d) : x(a), y(b), z(c), w(d) {}

    unsigned char x, y, z, w;
  };

  
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  static inline bool IntersectBox2Box2(float2 box1Min, float2 box1Max, float2 box2Min, float2 box2Max)
  {
    return box1Min.x <= box2Max.x && box2Min.x <= box1Max.x &&
           box1Min.y <= box2Max.y && box2Min.y <= box1Max.y;
  }

  static inline bool IntersectBox2Box2(int2 box1Min, int2 box1Max, int2 box2Min, int2 box2Max)
  {
    return box1Min.x <= box2Max.x && box2Min.x <= box1Max.x &&
           box1Min.y <= box2Max.y && box2Min.y <= box1Max.y;
  }
 
  inline static float4 color_unpack_bgra(int packedColor)
  {
    const int red   = (packedColor & 0x00FF0000) >> 16;
    const int green = (packedColor & 0x0000FF00) >> 8;
    const int blue  = (packedColor & 0x000000FF) >> 0;
    const int alpha = (packedColor & 0xFF000000) >> 24;
  
    return float4((float)red, (float)green, (float)blue, (float)alpha)*(1.0f / 255.0f);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // Look At matrix creation
  // return the inverse view matrix
  //
  static inline float4x4 lookAt(float3 eye, float3 center, float3 up)
  {
    float3 x, y, z; // basis; will make a rotation matrix

    z.x = eye.x - center.x;
    z.y = eye.y - center.y;
    z.z = eye.z - center.z;
    z = normalize(z);

    y.x = up.x;
    y.y = up.y;
    y.z = up.z;

    x = cross(y, z); // X vector = Y cross Z
    y = cross(z, x); // Recompute Y = Z cross X

    // cross product gives area of parallelogram, which is < 1.0 for
    // non-perpendicular unit-length vectors; so normalize x, y here
    x = normalize(x);
    y = normalize(y);

    float4x4 M;
    M.set_col(0, float4{ x.x, y.x, z.x, 0.0f });
    M.set_col(1, float4{ x.y, y.y, z.y, 0.0f });
    M.set_col(2, float4{ x.z, y.z, z.z, 0.0f });
    M.set_col(3, float4{ -x.x * eye.x - x.y * eye.y - x.z*eye.z,
                         -y.x * eye.x - y.y * eye.y - y.z*eye.z,
                         -z.x * eye.x - z.y * eye.y - z.z*eye.z,
                         1.0f });
    return M;
  }

  static inline float4x4 perspectiveMatrix(float fovy, float aspect, float zNear, float zFar)
  {
    const float ymax = zNear * tanf(fovy * 3.14159265358979323846f / 360.0f);
    const float xmax = ymax * aspect;

    const float left = -xmax;
    const float right = +xmax;
    const float bottom = -ymax;
    const float top = +ymax;

    const float temp = 2.0f * zNear;
    const float temp2 = right - left;
    const float temp3 = top - bottom;
    const float temp4 = zFar - zNear;

    float4x4 res;
    res.set_col(0, float4{ temp / temp2, 0.0f, 0.0f, 0.0f });
    res.set_col(1, float4{ 0.0f, temp / temp3, 0.0f, 0.0f });
    res.set_col(2, float4{ (right + left) / temp2,  (top + bottom) / temp3, (-zFar - zNear) / temp4, -1.0 });
    res.set_col(3, float4{ 0.0f, 0.0f, (-temp * zFar) / temp4, 0.0f });
    return res;
  }

  static inline float4x4 ortoMatrix(const float l, const float r, const float b, const float t, const float n, const float f)
  {
    float4x4 res;
    res(0,0) = 2.0f / (r - l);
    res(0,1) = 0;
    res(0,2) = 0;
    res(0,3) = -(r + l) / (r - l);

    res(1,0) = 0;
    res(1,1) = -2.0f / (t - b);  // why minus ??? check it for OpenGL please
    res(1,2) = 0;
    res(1,3) = -(t + b) / (t - b);

    res(2,0) = 0;
    res(2,1) = 0;
    res(2,2) = -2.0f / (f - n);
    res(2,3) = -(f + n) / (f - n);

    res(3,0) = 0.0f;
    res(3,1) = 0.0f;
    res(3,2) = 0.0f;
    res(3,3) = 1.0f;
    return res;
  }

  // http://matthewwellings.com/blog/the-new-vulkan-coordinate-system/
  //
  static inline float4x4 OpenglToVulkanProjectionMatrixFix()
  {
    float4x4 res;
    res(1,1) = -1.0f;
    res(2,2) = 0.5f;
    res(2,3) = 0.5f;
    return res;
  }

};

#endif
