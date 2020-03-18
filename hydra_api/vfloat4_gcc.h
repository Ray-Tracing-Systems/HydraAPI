//
// Created by frol on 30.10.18.
//

#ifndef TEST_GL_TOP_VFLOAT4_GCC_H
#define TEST_GL_TOP_VFLOAT4_GCC_H

#ifdef __x86_64
  #include <immintrin.h>
#endif

#if __GNUC__
#define CVEX_ALIGNED(X) __attribute__((__aligned__(X)))
#elif _MSC_VER
#define CVEX_ALIGNED(X) __declspec(align(X))
#else
#error "Unsupported compiler"
#endif

#include <cmath>

namespace cvex
{ 
  typedef unsigned           int _uint32_t;
  typedef unsigned long long int _uint64_t;
  typedef          long long int _sint64_t;

  typedef _uint32_t vuint4  __attribute__((vector_size(16)));
  typedef int       vint4   __attribute__((vector_size(16)));
  typedef float     vfloat4 __attribute__((vector_size(16)));

  typedef _uint32_t _vuint4u  __attribute__((vector_size(16), aligned(1)));
  typedef int       _vint4u   __attribute__((vector_size(16), aligned(1)));
  typedef float     _vfloat4u __attribute__((vector_size(16), aligned(1)));

  // load/store, splat ... 
  //
  static inline vuint4  load_u(const _uint32_t* p) { return *((_vuint4u*)p);  }
  static inline vint4   load_u(const int* p)       { return *((_vint4u*)p);   }
  static inline vfloat4 load_u(const float* p)     { return *((_vfloat4u*)p); }

  static inline vuint4  load  (const _uint32_t* p) { return *((vuint4*)p);  }
  static inline vint4   load  (const int* p)       { return *((vint4*)p);   }
  static inline vfloat4 load  (const float* p)     { return *((vfloat4*)p); }

  static inline void store_u(int* p,       vint4   a_val)   { *((_vint4u*)(p))   = a_val; }
  static inline void store_u(_uint32_t* p, vuint4  a_val)   { *((_vuint4u*)(p))  = a_val; }
  static inline void store_u(float* p,     vfloat4 a_val)   { *((_vfloat4u*)(p)) = a_val; }

  static inline void store_s(float* data, vfloat4 a_val)    { (*data) = a_val[0]; }

  static inline void store(int* p,       vint4   a_val)   { *((vint4*)(p))   = a_val; }
  static inline void store(_uint32_t* p, vuint4  a_val)   { *((vuint4*)(p))  = a_val; }
  static inline void store(float* p,     vfloat4 a_val)   { *((vfloat4*)(p)) = a_val; }

  static inline vint4   splat(int x)       { return vint4  {x,x,x,x}; }
  static inline vuint4  splat(_uint32_t x) { return vuint4 {x,x,x,x}; }
  static inline vfloat4 splat(float x)     { return vfloat4{x,x,x,x}; }

  
  static inline vfloat4 to_float32(const vint4 a)    { return vfloat4{(float)a[0], (float)a[1], (float)a[2], (float)a[3]}; }
  static inline vfloat4 to_float32(const vuint4 a)   { return vfloat4{(float)a[0], (float)a[1], (float)a[2], (float)a[3]}; }
  
  static inline vfloat4 as_float32(const vint4 a_val)   { return reinterpret_cast<vfloat4>(a_val); }
  static inline vfloat4 as_float32(const vuint4 a_val)  { return reinterpret_cast<vfloat4>(a_val); }
  static inline vint4   as_int32  (const vfloat4 a_val) { return reinterpret_cast<vint4>(a_val);   }
  static inline vuint4  as_uint32(const vfloat4 a_val)  { return reinterpret_cast<vuint4>(a_val);  }

  // math; all basic operators should be implemented by gcc, so we don't define them here
  //
  #ifdef __x86_64
  static inline vfloat4 rcp_e(vfloat4 a)       { return _mm_rcp_ps(a); }
  #else
  static inline vfloat4 rcp_e(vfloat4 a)       { return 1.0f/a; }
  #endif

  static inline vfloat4 min  (const vfloat4 a, const vfloat4 b) { return a < b ? a : b; }
  static inline vfloat4 max  (const vfloat4 a, const vfloat4 b) { return a > b ? a : b; }
  static inline vfloat4 clamp(const vfloat4 x, const vfloat4 minVal, const vfloat4 maxVal) { return max(min(x, maxVal), minVal); }
  static inline vfloat4 lerp (const vfloat4 u, const vfloat4 v, const float t) { return u + t * (v - u); }

  static inline vint4 min  (const vint4 a, const vint4 b) { return a < b ? a : b; }
  static inline vint4 max  (const vint4 a, const vint4 b) { return a > b ? a : b; }
  static inline vint4 clamp(const vint4 x, const vint4 minVal, const vint4 maxVal) { return max(min(x, maxVal), minVal); }

  static inline vuint4 min  (const vuint4 a, const vuint4 b) { return a < b ? a : b; }
  static inline vuint4 max  (const vuint4 a, const vuint4 b) { return a > b ? a : b; }
  static inline vuint4 clamp(const vuint4 x, const vuint4 minVal, const vuint4 maxVal) { return max(min(x, maxVal), minVal); }

  // convert and cast
  //
  static inline vint4   to_int32(const vfloat4 a)  { return vint4  { (int)a[0], (int)a[1], (int)a[2], (int)a[3]}; }
  static inline vint4   to_int32(const vuint4  a)  { return (vint4)a; }

  static inline vuint4  to_uint32(const vint4   a)     { return (vuint4)a; }
  static inline vuint4  to_uint32(const vfloat4 a_val) { return (vuint4)(max(to_int32(a_val),vint4{0,0,0,0})); }

  // shuffle operations ...
  //
  static inline vint4   blend(const vint4 a,  const vint4 b,  const vint4  mask) { return ((mask & a) | (~mask & b)); }
  static inline vuint4  blend(const vuint4 a, const vuint4 b, const vuint4 mask) { return ((mask & a) | (~mask & b)); }
  static inline vint4   blend(const vuint4 a, const vint4 b,  const vint4  mask) { return ((mask & (vint4)a) | (~mask & b)); }

  static inline vfloat4 blend(const vfloat4 a, const vfloat4 b, const vint4  mask)
  {
    const vint4 ia = reinterpret_cast<vint4>(a);
    const vint4 ib = reinterpret_cast<vint4>(b);
    return reinterpret_cast<vfloat4>((mask & ia) | (~mask & ib));
  }

  static inline vfloat4 blend(const vfloat4 a, const vfloat4 b, const vuint4 mask)
  {
    const vuint4 ia = reinterpret_cast<vuint4>(a);
    const vuint4 ib = reinterpret_cast<vuint4>(b);
    return reinterpret_cast<vfloat4>((mask & ia) | (~mask & ib));
  }

  static inline bool test_bits_any(const vint4 a)
  {
    const _sint64_t a2 = ( *(_sint64_t*)&a ) | ( *(_sint64_t*)&a[2] );
    return (a2 != 0);
  }

  static inline bool test_bits_all(const vint4 a)
  {
    const _uint64_t* p1 = (const _uint64_t*)&a;
    const _uint64_t* p2 = p1 + 1;
    const _uint64_t a2 = (*p1) & (*p2);
    return (a2 == _uint64_t(0xFFFFFFFFFFFFFFFF));
  }

  static inline bool test_bits_any(const vuint4 a)  { return test_bits_any(reinterpret_cast<vint4>(a)); }
  static inline bool test_bits_any(const vfloat4 a) { return test_bits_any(reinterpret_cast<vint4>(a)); }
  static inline bool test_bits_all(const vuint4 a)  { return test_bits_all(reinterpret_cast<vint4>(a)); }
  static inline bool test_bits_all(const vfloat4 a) { return test_bits_all(reinterpret_cast<vint4>(a)); }

  static inline void prefetch(const float* ptr) {  __builtin_prefetch(ptr); }
  static inline void prefetch(const int* ptr)   {  __builtin_prefetch(ptr); }

  static inline vfloat4 shuffle_zyxw(vfloat4 a_src) { return __builtin_shuffle(a_src, vint4{2,1,0,3}); }
  static inline vfloat4 shuffle_yzxw(vfloat4 a_src) { return __builtin_shuffle(a_src, vint4{1,2,0,3}); }
  static inline vfloat4 shuffle_zxyw(vfloat4 a_src) { return __builtin_shuffle(a_src, vint4{2,0,1,3}); }

  static inline vfloat4 shuffle_xyxy(vfloat4 a_src) { return __builtin_shuffle(a_src, vint4{0,1,0,1});  }
  static inline vfloat4 shuffle_zwzw(vfloat4 a_src) { return __builtin_shuffle(a_src, vint4{2,3,2,3}); }

  static inline vfloat4 cross3(const vfloat4 a, const vfloat4 b) 
  {
    const vfloat4 a_yzx = shuffle_yzxw(a);
    const vfloat4 b_yzx = shuffle_yzxw(b);
    return shuffle_yzxw(a*b_yzx - a_yzx*b);
  }

  static inline vfloat4 splat_0(const vfloat4 v) { return __builtin_shuffle(v, vint4{0,0,0,0}); }
  static inline vfloat4 splat_1(const vfloat4 v) { return __builtin_shuffle(v, vint4{1,1,1,1}); }
  static inline vfloat4 splat_2(const vfloat4 v) { return __builtin_shuffle(v, vint4{2,2,2,2}); }
  static inline vfloat4 splat_3(const vfloat4 v) { return __builtin_shuffle(v, vint4{3,3,3,3}); }

  static inline vint4 splat_0(const vint4 v) { return __builtin_shuffle(v, vint4{0,0,0,0}); }
  static inline vint4 splat_1(const vint4 v) { return __builtin_shuffle(v, vint4{1,1,1,1}); }
  static inline vint4 splat_2(const vint4 v) { return __builtin_shuffle(v, vint4{2,2,2,2}); }
  static inline vint4 splat_3(const vint4 v) { return __builtin_shuffle(v, vint4{3,3,3,3}); }

  static inline vuint4 splat_0(const vuint4 v) { return __builtin_shuffle(v, vint4{0,0,0,0}); }
  static inline vuint4 splat_1(const vuint4 v) { return __builtin_shuffle(v, vint4{1,1,1,1}); }
  static inline vuint4 splat_2(const vuint4 v) { return __builtin_shuffle(v, vint4{2,2,2,2}); }
  static inline vuint4 splat_3(const vuint4 v) { return __builtin_shuffle(v, vint4{3,3,3,3}); }

  static inline int extract_0(const vint4 a_val) { return a_val[0]; }
  static inline int extract_1(const vint4 a_val) { return a_val[1]; }
  static inline int extract_2(const vint4 a_val) { return a_val[2]; }
  static inline int extract_3(const vint4 a_val) { return a_val[3]; }

  static inline unsigned int extract_0(const vuint4 a_val) { return a_val[0]; }
  static inline unsigned int extract_1(const vuint4 a_val) { return a_val[1]; }
  static inline unsigned int extract_2(const vuint4 a_val) { return a_val[2]; }
  static inline unsigned int extract_3(const vuint4 a_val) { return a_val[3]; }

  static inline float extract_0(const vfloat4 a_val) { return a_val[0]; }
  static inline float extract_1(const vfloat4 a_val) { return a_val[1]; }
  static inline float extract_2(const vfloat4 a_val) { return a_val[2]; }
  static inline float extract_3(const vfloat4 a_val) { return a_val[3]; }

  static inline __m128i as_m128i(const vfloat4& a) { return reinterpret_cast<__m128i>(a);  } 

  #ifdef __x86_64

  static inline float   dot3f(const vfloat4 a, const vfloat4 b) { return _mm_cvtss_f32(_mm_dp_ps(a, b, 0x7f)); }
  static inline vfloat4 dot3v(const vfloat4 a, const vfloat4 b) { return _mm_dp_ps(a, b, 0x7f); }
  static inline vfloat4 dot4v(const vfloat4 a, const vfloat4 b) { return _mm_dp_ps(a, b, 0xff); }
  static inline float   dot4f(const vfloat4 a, const vfloat4 b) { return _mm_cvtss_f32(_mm_dp_ps(a, b, 0xff)); }

  static inline float   length3f(const vfloat4 a) { return _mm_cvtss_f32(_mm_sqrt_ss(dot3v(a,a)) ); }
  static inline float   length4f(const vfloat4 a) { return _mm_cvtss_f32(_mm_sqrt_ss(dot4v(a,a)) ); }
  static inline vfloat4 length3v(const vfloat4 a) { return _mm_sqrt_ps(dot3v(a,a)); }
  static inline vfloat4 length4v(const vfloat4 a) { return _mm_sqrt_ps(dot4v(a,a)); }

  static inline vfloat4 floor(const vfloat4 a_val) { return _mm_floor_ps(a_val); }
  static inline vfloat4 ceil (const vfloat4 a_val) { return _mm_ceil_ps(a_val);  }
  static inline vfloat4 fabs (const vfloat4 a_val)
  {
    const __m128 absmask = _mm_castsi128_ps(_mm_set1_epi32((1<<31)));
    return _mm_andnot_ps(absmask, a_val);
  }

  static inline bool any_of (const vint4 a) { return _mm_movemask_ps(as_float32(a)) != 0; }
  static inline bool all_of (const vint4 a) { return _mm_movemask_ps(as_float32(a)) == 15; }

  inline static unsigned int color_pack_rgba(const vfloat4 rel_col)
  {
    static constexpr vfloat4 const_255 = { 255.0f, 255.0f, 255.0f, 255.0f };
  
    const __m128i rgba = _mm_cvtps_epi32(_mm_mul_ps(rel_col, const_255));
    const __m128i out  = _mm_packus_epi32(rgba, _mm_setzero_si128());
    const __m128i out2 = _mm_packus_epi16(out, _mm_setzero_si128());
  
    return _mm_cvtsi128_si32(out2);
  }

  inline static unsigned int color_pack_bgra(const vfloat4 rel_col) { return color_pack_rgba(cvex::shuffle_zyxw(rel_col)); }

  static inline void transpose4(const vfloat4 __restrict in_rows[4], vfloat4 __restrict out_rows[4])
  {
    const auto a0 = as_m128i(in_rows[0]);
    const auto a1 = as_m128i(in_rows[1]);
    const auto a2 = as_m128i(in_rows[2]);
    const auto a3 = as_m128i(in_rows[3]);

    const auto b0 = _mm_unpacklo_epi32(a0, a1);
    const auto b1 = _mm_unpackhi_epi32(a0, a1);
    const auto b2 = _mm_unpacklo_epi32(a2, a3);
    const auto b3 = _mm_unpackhi_epi32(a2, a3);
  
    out_rows[0] = _mm_castsi128_ps(_mm_unpacklo_epi64(b0, b2));
    out_rows[1] = _mm_castsi128_ps(_mm_unpackhi_epi64(b0, b2));
    out_rows[2] = _mm_castsi128_ps(_mm_unpacklo_epi64(b1, b3));
    out_rows[3] = _mm_castsi128_ps(_mm_unpackhi_epi64(b1, b3));
  }

  static inline void set_ftz() { _MM_SET_ROUNDING_MODE(_MM_ROUND_TOWARD_ZERO); }

  static inline vint4 gather(const int* a_data, const vint4 offset) { return (vint4)_mm_i32gather_epi32(a_data, (__m128i)offset, 4); }

  #else

  static inline float   dot3f(const vfloat4 a, const vfloat4 b) 
  {
    const vfloat4 mres = a*b; 
    return mres[0] + mres[1] + mres[2]; 
  }

  static inline vfloat4 dot3v(const vfloat4 a, const vfloat4 b) 
  {
    const vfloat4 mres = a*b;
    const float res    = mres[0] + mres[1] + mres[2];
    return vfloat4{res,res,res,res}; 
  }

  static inline vfloat4 length3v(const vfloat4 a) 
  {
    const vfloat4 mres = a*a;
    const float res    = sqrtf(mres[0] + mres[1] + mres[2]);
    return vfloat4{res,res,res,res}; 
  }

  static inline float   dot4f(const vfloat4 a, const vfloat4 b) 
  {
    const vfloat4 mres = a*b; 
    return mres[0] + mres[1] + mres[2] + mres[3]; 
  }

  static inline vfloat4 dot4v(const vfloat4 a, const vfloat4 b) 
  {
    const vfloat4 mres = a*b;
    const float res    = mres[0] + mres[1] + mres[2] + mres[3];
    return vfloat4{res,res,res,res}; 
  }

  static inline vfloat4 length4v(const vfloat4 a) 
  {
    const vfloat4 mres = a*a;
    const float res    = sqrtf(mres[0] + mres[1] + mres[2] + mres[3]);
    return vfloat4{res,res,res,res}; 
  }
  
  static inline float length3f(const vfloat4 a) { return sqrtf(dot3f(a,a)); }
  static inline float length4f(const vfloat4 a) { return sqrtf(dot4f(a,a)); }

  static inline vfloat4 ceil(const vfloat4 a)
  {
    const vfloat4 res = {::ceilf(a[0]), ::ceilf(a[1]), ::ceilf(a[2]), ::ceilf(a[3])};
    return res;
  }

  static inline vfloat4 floor(const vfloat4 a)
  {
    const vfloat4 res = {::floorf(a[0]), ::floorf(a[1]), ::floorf(a[2]), ::floorf(a[3])};
    return res;
  }

  static inline vfloat4 fabs(const vfloat4 a)
  {
    const vfloat4 res = {::fabs(a[0]), ::fabs(a[1]), ::fabs(a[2]), ::fabs(a[3])};
    return res;
  }

  static inline bool any_of (const vint4 a) { return (a[0] != 0) || (a[1] != 0) || (a[2] != 0) || (a[3] != 0); }
  static inline bool all_of (const vint4 a) { return (a[0] != 0) && (a[1] != 0) && (a[2] != 0) && (a[3] != 0); }


  inline static unsigned int color_pack_rgba(const vfloat4 rel_col)
  {
    static constexpr vfloat4 const_255 = { 256.0f, 256.0f, 256.0f, 256.0f };
    const vuint4 rgba = to_uint32(_mm_mul_ps(rel_col, const_255));
    return (rgba[3] << 24) | (rgba[2] << 16) | (rgba[1] << 8) | rgba[0];
  }

  inline static unsigned int color_pack_bgra(const vfloat4 rel_col)
  {
    static constexpr vfloat4 const_255 = { 256.0f, 256.0f, 256.0f, 256.0f };
    const vuint4 rgba = to_uint32(_mm_mul_ps(cvex::shuffle_zyxw(rel_col), const_255));
    return (rgba[3] << 24) | (rgba[2] << 16) | (rgba[1] << 8) | rgba[0];
  }

  inline static void set_ftz() {}

  static inline vint4 gather(const int* a_data, const vint4 offset)
  {
    CVEX_ALIGNED(16) int myOffsets[4];
    store(myOffsets, offset);

    const int d01 = a_data[myOffsets[0]];
    const int d11 = a_data[myOffsets[1]];
    const int d21 = a_data[myOffsets[2]];
    const int d31 = a_data[myOffsets[3]];

    return vint4{d01, d11, d21, d31};
  }

  #endif

 
  static inline void mat4_rowmajor_mul_mat4(float* __restrict M, const float* __restrict A, const float* __restrict B) // modern gcc compiler succesfuly vectorize such implementation!
  {
  	M[ 0] = A[ 0] * B[ 0] + A[ 1] * B[ 4] + A[ 2] * B[ 8] + A[ 3] * B[12];
  	M[ 1] = A[ 0] * B[ 1] + A[ 1] * B[ 5] + A[ 2] * B[ 9] + A[ 3] * B[13];
  	M[ 2] = A[ 0] * B[ 2] + A[ 1] * B[ 6] + A[ 2] * B[10] + A[ 3] * B[14];
  	M[ 3] = A[ 0] * B[ 3] + A[ 1] * B[ 7] + A[ 2] * B[11] + A[ 3] * B[15];
  	M[ 4] = A[ 4] * B[ 0] + A[ 5] * B[ 4] + A[ 6] * B[ 8] + A[ 7] * B[12];
  	M[ 5] = A[ 4] * B[ 1] + A[ 5] * B[ 5] + A[ 6] * B[ 9] + A[ 7] * B[13];
  	M[ 6] = A[ 4] * B[ 2] + A[ 5] * B[ 6] + A[ 6] * B[10] + A[ 7] * B[14];
  	M[ 7] = A[ 4] * B[ 3] + A[ 5] * B[ 7] + A[ 6] * B[11] + A[ 7] * B[15];
  	M[ 8] = A[ 8] * B[ 0] + A[ 9] * B[ 4] + A[10] * B[ 8] + A[11] * B[12];
  	M[ 9] = A[ 8] * B[ 1] + A[ 9] * B[ 5] + A[10] * B[ 9] + A[11] * B[13];
  	M[10] = A[ 8] * B[ 2] + A[ 9] * B[ 6] + A[10] * B[10] + A[11] * B[14];
  	M[11] = A[ 8] * B[ 3] + A[ 9] * B[ 7] + A[10] * B[11] + A[11] * B[15];
  	M[12] = A[12] * B[ 0] + A[13] * B[ 4] + A[14] * B[ 8] + A[15] * B[12];
  	M[13] = A[12] * B[ 1] + A[13] * B[ 5] + A[14] * B[ 9] + A[15] * B[13];
  	M[14] = A[12] * B[ 2] + A[13] * B[ 6] + A[14] * B[10] + A[15] * B[14];
  	M[15] = A[12] * B[ 3] + A[13] * B[ 7] + A[14] * B[11] + A[15] * B[15];
  }

  static inline void mat4_colmajor_mul_vec4(float* __restrict RES, const float* __restrict B, const float* __restrict V) // modern gcc compiler succesfuly vectorize such implementation!
  {
  	RES[0] = V[0] * B[0] + V[1] * B[4] + V[2] * B[ 8] + V[3] * B[12];
  	RES[1] = V[0] * B[1] + V[1] * B[5] + V[2] * B[ 9] + V[3] * B[13];
  	RES[2] = V[0] * B[2] + V[1] * B[6] + V[2] * B[10] + V[3] * B[14];
  	RES[3] = V[0] * B[3] + V[1] * B[7] + V[2] * B[11] + V[3] * B[15];
  }

};

#endif //TEST_GL_TOP_VFLOAT4_GCC_H
