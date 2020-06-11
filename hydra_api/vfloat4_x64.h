//
// Created by Vladimir Frolov on 14.10.18.
//
// This library is created as lightweight and more optimised replacement for famous libsimdpp
// In the case if no implementation for tarhet architecture presents, you should include
// file (vfloat4_your_arch.h) that defines all operations and types via orger implementation
// so it can work ok different platforms anyway

#ifndef TEST_GL_TOP_VFLOAT4_H
#define TEST_GL_TOP_VFLOAT4_H

#include <xmmintrin.h> // SSE
#include <emmintrin.h> // SSE2
#include <smmintrin.h> // SSE4.1 // #TODO: optionally
#include <immintrin.h> // AVX    // #TODO: optionally

#if __GNUC__
#define ALIGN(X) __attribute__((__aligned__(X)))
#elif _MSC_VER
#define ALIGN(X) __declspec(align(X))
#else
#error "Unsupported compiler"
#endif

#if defined(_MSC_VER)
#define CVEX_ALIGNED(x) __declspec(align(x))
#else
#if defined(__GNUC__)
#define CVEX_ALIGNED(x) __attribute__ ((aligned(x)))
#endif
#endif

#include <initializer_list>

#ifdef WIN32
#undef min
#undef max
#endif

namespace cvex
{
  typedef unsigned           int _uint32_t;
  typedef unsigned long long int _uint64_t;
  typedef          long long int _sint64_t;

  struct vfloat4
  {
    vfloat4() {}
    vfloat4(const __m128& rhs) { data = rhs; }
    vfloat4(const std::initializer_list<float> v) { data =_mm_loadu_ps(v.begin()); }

    inline operator __m128() const { return data; }

    __m128 data;
  };

  struct vint4
  {
    vint4() {}
    vint4(const __m128i& rhs) { data = rhs; }
    vint4(const std::initializer_list<int> v) { data = _mm_castps_si128(_mm_loadu_ps((const float*)v.begin())); } 
    inline operator __m128i() const { return data; }

    __m128i data;
  };

  struct vuint4
  {
    vuint4() {}
    vuint4(const __m128i& rhs) { data = rhs; }
    vuint4(const std::initializer_list<_uint32_t> v) { data = _mm_castps_si128(_mm_loadu_ps((const float*)v.begin())); }
    inline operator __m128i() const { return data; }

    __m128i data;
  };

  static inline void set_ftz() { _MM_SET_ROUNDING_MODE(_MM_ROUND_TOWARD_ZERO);}

  static inline void store(float* data, vfloat4 a_val)    { _mm_store_ps(data, a_val);  }
  static inline void store(int*   data, vint4 a_val)      { _mm_store_ps((float*)data, _mm_castsi128_ps(a_val)); }
  static inline void store(_uint32_t* data, vuint4 a_val) { _mm_store_ps((float*)data, _mm_castsi128_ps(a_val)); }

  static inline void store_u(float* data, vfloat4 a_val)    { _mm_storeu_ps(data, a_val); }
  static inline void store_u(int*   data,  vint4 a_val)     { _mm_storeu_ps((float*)data, _mm_castsi128_ps(a_val)); }
  static inline void store_u(_uint32_t* data, vuint4 a_val) { _mm_storeu_ps((float*)data, _mm_castsi128_ps(a_val)); }

  static inline void store_s(float* data, vfloat4 a_val)    { _mm_store_ss(data, a_val); }

  static inline vfloat4 load(const float *data)     { return _mm_load_ps(data);  }
  static inline vint4   load(const int *data)       { return _mm_castps_si128(_mm_load_ps((float*)data)); }
  static inline vuint4  load(const _uint32_t *data) { return _mm_castps_si128(_mm_load_ps((float*)data)); }

  static inline vfloat4 load_u(const float *data)     { return _mm_loadu_ps(data); }
  static inline vint4   load_u(const int *data)       { return _mm_castps_si128(_mm_loadu_ps((float*)data)); }
  static inline vuint4  load_u(const _uint32_t* data) { return _mm_castps_si128(_mm_loadu_ps((float*)data)); }
  static inline vfloat4 load_s(const float *data)     { return _mm_load_ss(data);  }

  //static inline void stream(void *data, vint4 a_val) { _mm_stream_si128((vint4 *) data, a_val); }

  static inline vint4   splat(const int i)       { return _mm_set1_epi32(i); }
  static inline vuint4  splat(const _uint32_t i) { return _mm_set1_epi32(i); }
  static inline vfloat4 splat(const float i)     { return _mm_set1_ps   (i); }

  static inline vfloat4 as_float32(const vint4 a_val)   { return _mm_castsi128_ps(a_val); }
  static inline vfloat4 as_float32(const vuint4 a_val)  { return _mm_castsi128_ps(a_val); }
  static inline vint4   as_int32  (const vfloat4 a_val) { return _mm_castps_si128(a_val); }
  static inline vuint4  as_uint32 (const vfloat4 a_val) { return _mm_castps_si128(a_val); }

  static inline vfloat4 to_float32(const vint4 a_val) { return _mm_cvtepi32_ps(a_val);}
  static inline vfloat4 to_float32(const vuint4 a_val)
  { 
    CVEX_ALIGNED(16) unsigned int temp_a[4];
    cvex::store(temp_a, a_val);
    return {float(temp_a[0]), float(temp_a[1]), float(temp_a[2]), float(temp_a[3]) };
  }

  static inline vint4 to_int32(const vfloat4 a_val) { return _mm_cvtps_epi32(a_val); }
  static inline vint4 to_int32(const vuint4  a_val) 
  { 
    vint4 res;
    res.data = a_val.data;
    return res; 
  }

  static inline vuint4 to_uint32(const vfloat4 a_val) // _mm_cvtps_epu32 presents only in AXV512!!!
  {
    CVEX_ALIGNED(16) float temp_a[4];
    cvex::store(temp_a, a_val);
    return { _uint32_t(temp_a[0]), _uint32_t(temp_a[1]), _uint32_t(temp_a[2]), _uint32_t(temp_a[3]) };
  }

  static inline vuint4 to_uint32(const vint4  a_val)
  {
    vuint4 res;
    res.data = a_val.data;
    return res;
  }

  static inline vfloat4 min(const vfloat4 a, const vfloat4 b) {return _mm_min_ps(a, b);}
  static inline vfloat4 max(const vfloat4 a, const vfloat4 b) {return _mm_max_ps(a, b);}
  static inline vfloat4 clamp(const vfloat4 x, const vfloat4 minVal, const vfloat4 maxVal) { return _mm_max_ps(_mm_min_ps(x, maxVal), minVal); }
  static inline vfloat4 rcp_e(const vfloat4 a) { return _mm_rcp_ps(a); }

  static inline vint4 min(const vint4 a, const vint4 b) { return _mm_min_epi32(a, b); }
  static inline vint4 max(const vint4 a, const vint4 b) { return _mm_max_epi32(a, b); }
  static inline vint4 clamp(const vint4 x, const vint4 minVal, const vint4 maxVal) { return _mm_max_epi32(_mm_min_epi32(x, maxVal), minVal); }

  static inline vuint4 min(const vuint4 a, const vuint4 b) { return _mm_min_epu32(a, b); }
  static inline vuint4 max(const vuint4 a, const vuint4 b) { return _mm_max_epu32(a, b); }
  static inline vuint4 clamp(const vuint4 x, const vuint4 minVal, const vuint4 maxVal) { return _mm_max_epu32(_mm_min_epu32(x, maxVal), minVal); }

  static inline vfloat4 floor(const vfloat4 a_val) { return _mm_floor_ps(a_val); }
  static inline vfloat4 ceil (const vfloat4 a_val) { return _mm_ceil_ps(a_val);  }
  static inline vfloat4 fabs (const vfloat4 a_val)
  {
    const __m128 absmask = _mm_castsi128_ps(_mm_set1_epi32((1<<31)));
    return _mm_andnot_ps(absmask, a_val);
  }

  static inline vfloat4 blend(const vfloat4 a, const vfloat4 b, const vint4 mask)
  {
    return _mm_or_ps(_mm_and_ps(as_float32(mask), a),
                     _mm_andnot_ps(as_float32(mask), b));
  }

  static inline vfloat4 blend(const vfloat4 a, const vfloat4 b, const vuint4 mask)
  {
    return _mm_or_ps(_mm_and_ps(as_float32(mask), a),
                     _mm_andnot_ps(as_float32(mask), b));
  }

  static inline vint4 blend(const vint4 a, const vint4 b, const vint4 mask)
  {
    return as_int32(_mm_or_ps(_mm_and_ps   (as_float32(mask), as_float32(a)),
                              _mm_andnot_ps(as_float32(mask), as_float32(b))));
  }

  static inline vuint4 blend(const vuint4 a, const vuint4 b, const vuint4 mask)
  {
    return as_uint32(_mm_or_ps(_mm_and_ps(as_float32(mask), as_float32(a)),
                     _mm_andnot_ps(as_float32(mask), as_float32(b))));
  }


  static inline void transpose4(const vfloat4 a[4], vfloat4 RES[4])
  {
    const vint4 b0 = _mm_unpacklo_epi32(as_int32(a[0]), as_int32(a[1]));
    const vint4 b1 = _mm_unpackhi_epi32(as_int32(a[0]), as_int32(a[1]));
    const vint4 b2 = _mm_unpacklo_epi32(as_int32(a[2]), as_int32(a[3]));
    const vint4 b3 = _mm_unpackhi_epi32(as_int32(a[2]), as_int32(a[3]));

    RES[0] = _mm_castsi128_ps(_mm_unpacklo_epi64(b0, b2));
    RES[1] = _mm_castsi128_ps(_mm_unpackhi_epi64(b0, b2));
    RES[2] = _mm_castsi128_ps(_mm_unpacklo_epi64(b1, b3));
    RES[3] = _mm_castsi128_ps(_mm_unpackhi_epi64(b1, b3));
  }

  static inline bool test_bits_any(const vint4 a)
  {
    const _sint64_t* p1 = (const _sint64_t*)&a;
    const _sint64_t* p2 = p1 + 1;
    const _sint64_t a2 = (*p1) | (*p2);
    return (a2 != 0);
  }

  static inline bool test_bits_any(const vuint4 a)
  {
    const _uint64_t* p1 = (const _uint64_t*)&a;
    const _uint64_t* p2 = p1 + 1;
    const _uint64_t a2 = (*p1) | (*p2);
    return (a2 != 0);
  }

  static inline bool test_bits_any(const vfloat4 a) 
  { 
    const _sint64_t* p1 = (const _sint64_t*)&a;
    const _sint64_t* p2 = p1 + 1;
    const _sint64_t a2  = (*p1) | (*p2);
    return (a2 != 0);
  }

  static inline bool test_bits_all(const vint4 a)
  {
    const _uint64_t* p1 = (const _uint64_t*)&a;
    const _uint64_t* p2 = p1 + 1;
    const _uint64_t a2 = (*p1) & (*p2);
    return (a2 == _uint64_t(0xFFFFFFFFFFFFFFFF));
  }

  static inline bool test_bits_all(const vuint4 a)
  {
    const _uint64_t* p1 = (const _uint64_t*)&a;
    const _uint64_t* p2 = p1 + 1;
    const _uint64_t a2 = (*p1) & (*p2);
    return (a2 == _uint64_t(0xFFFFFFFFFFFFFFFF));
  }

  static inline bool test_bits_all(const vfloat4 a)
  {
    const _uint64_t* p1 = (const _uint64_t*)&a;
    const _uint64_t* p2 = p1 + 1;
    const _uint64_t a2 = (*p1) & (*p2);
    return (a2 == _uint64_t(0xFFFFFFFFFFFFFFFF));
  }

  static inline vfloat4 dot3v(const vfloat4 a, const vfloat4 b) { return _mm_dp_ps(a, b, 0x7f); }
  static inline float   dot3f(const vfloat4 a, const vfloat4 b) { return _mm_cvtss_f32(_mm_dp_ps(a, b, 0x7f)); }
  static inline vfloat4 dot4v(const vfloat4 a, const vfloat4 b) { return _mm_dp_ps(a, b, 0xff); }
  static inline float   dot4f(const vfloat4 a, const vfloat4 b) { return _mm_cvtss_f32(_mm_dp_ps(a, b, 0xff)); }

  static inline float   length3f(const vfloat4 a) { return _mm_cvtss_f32(_mm_sqrt_ss(dot3v(a, a))); }
  static inline float   length4f(const vfloat4 a) { return _mm_cvtss_f32(_mm_sqrt_ss(dot4v(a, a))); }
  static inline vfloat4 length3v(const vfloat4 a) { return _mm_sqrt_ps(dot3v(a, a)); }
  static inline vfloat4 length4v(const vfloat4 a) { return _mm_sqrt_ps(dot4v(a, a)); }

  static inline vfloat4 splat_0(const vfloat4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0)); }
  static inline vfloat4 splat_1(const vfloat4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1)); }
  static inline vfloat4 splat_2(const vfloat4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2)); }
  static inline vfloat4 splat_3(const vfloat4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3)); }

  static inline vint4 splat_0(const vint4 v) { return _mm_shuffle_epi32(v, _MM_SHUFFLE(0, 0, 0, 0)); }
  static inline vint4 splat_1(const vint4 v) { return _mm_shuffle_epi32(v, _MM_SHUFFLE(1, 1, 1, 1)); }
  static inline vint4 splat_2(const vint4 v) { return _mm_shuffle_epi32(v, _MM_SHUFFLE(2, 2, 2, 2)); }
  static inline vint4 splat_3(const vint4 v) { return _mm_shuffle_epi32(v, _MM_SHUFFLE(3, 3, 3, 3)); }

  static inline vuint4 splat_0(const vuint4 v) { return _mm_shuffle_epi32(v, _MM_SHUFFLE(0, 0, 0, 0)); }
  static inline vuint4 splat_1(const vuint4 v) { return _mm_shuffle_epi32(v, _MM_SHUFFLE(1, 1, 1, 1)); }
  static inline vuint4 splat_2(const vuint4 v) { return _mm_shuffle_epi32(v, _MM_SHUFFLE(2, 2, 2, 2)); }
  static inline vuint4 splat_3(const vuint4 v) { return _mm_shuffle_epi32(v, _MM_SHUFFLE(3, 3, 3, 3)); }

  static inline int extract_0(const vint4 a_val) { return _mm_cvtsi128_si32(a_val); }
  static inline int extract_1(const vint4 a_val) { return _mm_extract_epi32(a_val, 1); }
  static inline int extract_2(const vint4 a_val) { return _mm_extract_epi32(a_val, 2); }
  static inline int extract_3(const vint4 a_val) { return _mm_extract_epi32(a_val, 3); }

  static inline unsigned int extract_0(const vuint4 a_val) { return (unsigned int)_mm_cvtsi128_si32(a_val); }
  static inline unsigned int extract_1(const vuint4 a_val) { return (unsigned int)_mm_extract_epi32(a_val, 1); }
  static inline unsigned int extract_2(const vuint4 a_val) { return (unsigned int)_mm_extract_epi32(a_val, 2); }
  static inline unsigned int extract_3(const vuint4 a_val) { return (unsigned int)_mm_extract_epi32(a_val, 3); }

  static inline float extract_0(const vfloat4 a_val) { return _mm_cvtss_f32(a_val); }
  static inline float extract_1(const vfloat4 a_val) { return _mm_cvtss_f32(splat_1(a_val)); }
  static inline float extract_2(const vfloat4 a_val) { return _mm_cvtss_f32(splat_2(a_val)); } // #TODO: stop pressing Port 5
  static inline float extract_3(const vfloat4 a_val) { return _mm_cvtss_f32(splat_3(a_val)); } // #TODO: please consider _mm_cvtsi128_si32( _mm_srli_si128(a_val, 3*4) ); ??? 

  static inline vfloat4 shuffle_zyxw(vfloat4 a_src) { return _mm_shuffle_ps(a_src, a_src, _MM_SHUFFLE(3, 0, 1, 2)); }
  static inline vfloat4 shuffle_yzxw(vfloat4 a_src) { return _mm_shuffle_ps(a_src, a_src, _MM_SHUFFLE(3, 0, 2, 1)); }
  static inline vfloat4 shuffle_zxyw(vfloat4 a_src) { return _mm_shuffle_ps(a_src, a_src, _MM_SHUFFLE(3, 1, 0, 2)); }

  static inline vfloat4 shuffle_xyxy(vfloat4 a_src) { return _mm_shuffle_ps(a_src, a_src, _MM_SHUFFLE(1, 0, 1, 0)); }
  static inline vfloat4 shuffle_zwzw(vfloat4 a_src) { return _mm_shuffle_ps(a_src, a_src, _MM_SHUFFLE(3, 2, 3, 2)); }

  static inline vfloat4 cross3(const vfloat4 a, const vfloat4 b) 
  { 
    const __m128 a_yzx = shuffle_yzxw(a);
    const __m128 b_yzx = shuffle_yzxw(b);
    const __m128 c     = _mm_sub_ps(_mm_mul_ps(a, b_yzx), _mm_mul_ps(a_yzx, b));
    return shuffle_yzxw(c);
  }

  inline static unsigned int color_pack_rgba(const vfloat4 rel_col)
  {
    static const vfloat4 const_255 = { 255.0f, 255.0f, 255.0f, 255.0f };
  
    const __m128i rgba = _mm_cvtps_epi32(_mm_mul_ps(rel_col, const_255));
    const __m128i out  = _mm_packus_epi32(rgba, _mm_setzero_si128());
    const __m128i out2 = _mm_packus_epi16(out, _mm_setzero_si128());
  
    return _mm_cvtsi128_si32(out2);
  }

  inline static unsigned int color_pack_bgra(const vfloat4 rel_col) { return color_pack_rgba(cvex::shuffle_zyxw(rel_col)); }

  static inline bool any_of (const vint4 a) { return _mm_movemask_ps(as_float32(a)) != 0; }
  static inline bool all_of (const vint4 a) { return _mm_movemask_ps(as_float32(a)) == 15; }

  static inline void prefetch(const float* ptr) {  _mm_prefetch((const char*)ptr, _MM_HINT_T0); }
  static inline void prefetch(const int* ptr)   {  _mm_prefetch((const char*)ptr, _MM_HINT_T0); }

  static inline void mat4_rowmajor_mul_mat4(float* __restrict M, const float* __restrict A, const float* __restrict B) // modern gcc compiler succesfuly vectorize such implementation!
  {
    M[0] = A[0] * B[0] + A[1] * B[4] + A[2] * B[8] + A[3] * B[12];
    M[1] = A[0] * B[1] + A[1] * B[5] + A[2] * B[9] + A[3] * B[13];
    M[2] = A[0] * B[2] + A[1] * B[6] + A[2] * B[10] + A[3] * B[14];
    M[3] = A[0] * B[3] + A[1] * B[7] + A[2] * B[11] + A[3] * B[15];
    M[4] = A[4] * B[0] + A[5] * B[4] + A[6] * B[8] + A[7] * B[12];
    M[5] = A[4] * B[1] + A[5] * B[5] + A[6] * B[9] + A[7] * B[13];
    M[6] = A[4] * B[2] + A[5] * B[6] + A[6] * B[10] + A[7] * B[14];
    M[7] = A[4] * B[3] + A[5] * B[7] + A[6] * B[11] + A[7] * B[15];
    M[8] = A[8] * B[0] + A[9] * B[4] + A[10] * B[8] + A[11] * B[12];
    M[9] = A[8] * B[1] + A[9] * B[5] + A[10] * B[9] + A[11] * B[13];
    M[10] = A[8] * B[2] + A[9] * B[6] + A[10] * B[10] + A[11] * B[14];
    M[11] = A[8] * B[3] + A[9] * B[7] + A[10] * B[11] + A[11] * B[15];
    M[12] = A[12] * B[0] + A[13] * B[4] + A[14] * B[8] + A[15] * B[12];
    M[13] = A[12] * B[1] + A[13] * B[5] + A[14] * B[9] + A[15] * B[13];
    M[14] = A[12] * B[2] + A[13] * B[6] + A[14] * B[10] + A[15] * B[14];
    M[15] = A[12] * B[3] + A[13] * B[7] + A[14] * B[11] + A[15] * B[15];
  }

  static inline void mat4_colmajor_mul_vec4(float* __restrict RES, const float* __restrict B, const float* __restrict V) // modern gcc compiler succesfuly vectorize such implementation!
  {
    RES[0] = V[0] * B[0] + V[1] * B[4] + V[2] * B[8] + V[3] * B[12];
    RES[1] = V[0] * B[1] + V[1] * B[5] + V[2] * B[9] + V[3] * B[13];
    RES[2] = V[0] * B[2] + V[1] * B[6] + V[2] * B[10] + V[3] * B[14];
    RES[3] = V[0] * B[3] + V[1] * B[7] + V[2] * B[11] + V[3] * B[15];
  }


static inline cvex::vfloat4 operator+(const cvex::vfloat4 a, const cvex::vfloat4 b) { return _mm_add_ps(a, b); }
static inline cvex::vfloat4 operator-(const cvex::vfloat4 a, const cvex::vfloat4 b) { return _mm_sub_ps(a, b); }
static inline cvex::vfloat4 operator*(const cvex::vfloat4 a, const cvex::vfloat4 b) { return _mm_mul_ps(a, b); }
static inline cvex::vfloat4 operator/(const cvex::vfloat4 a, const cvex::vfloat4 b) { return _mm_div_ps(a, b); }

static inline cvex::vfloat4 operator+(const cvex::vfloat4 a, const float b) { return _mm_add_ps(a, _mm_broadcast_ss(&b)); }
static inline cvex::vfloat4 operator-(const cvex::vfloat4 a, const float b) { return _mm_sub_ps(a, _mm_broadcast_ss(&b)); }
static inline cvex::vfloat4 operator*(const cvex::vfloat4 a, const float b) { return _mm_mul_ps(a, _mm_broadcast_ss(&b)); }
static inline cvex::vfloat4 operator/(const cvex::vfloat4 a, const float b) { return _mm_div_ps(a, _mm_broadcast_ss(&b)); }

static inline cvex::vfloat4 operator+(const float a, const cvex::vfloat4 b) { return _mm_add_ps(_mm_broadcast_ss(&a), b); }
static inline cvex::vfloat4 operator-(const float a, const cvex::vfloat4 b) { return _mm_sub_ps(_mm_broadcast_ss(&a), b); }
static inline cvex::vfloat4 operator*(const float a, const cvex::vfloat4 b) { return _mm_mul_ps(_mm_broadcast_ss(&a), b); }
static inline cvex::vfloat4 operator/(const float a, const cvex::vfloat4 b) { return _mm_div_ps(_mm_broadcast_ss(&a), b); }

static inline vfloat4 lerp(const vfloat4& u, const vfloat4& v, const float t) { return u + t * (v - u); }

static inline cvex::vint4 operator+(const cvex::vint4 a, const cvex::vint4 b) { return _mm_add_epi32(a, b);   }
static inline cvex::vint4 operator-(const cvex::vint4 a, const cvex::vint4 b) { return _mm_sub_epi32(a, b);   }
static inline cvex::vint4 operator*(const cvex::vint4 a, const cvex::vint4 b) { return _mm_mullo_epi32(a, b); }
static inline cvex::vint4 operator/(const cvex::vint4 a, const cvex::vint4 b) 
{ 
  CVEX_ALIGNED(16) int temp_a[4];
  CVEX_ALIGNED(16) int temp_b[4];
  cvex::store(temp_a, a);
  cvex::store(temp_b, b);
  return { temp_a[0] / temp_b[0], temp_a[1] / temp_b[1], temp_a[2] / temp_b[2], temp_a[3] / temp_b[3] };
} 

static inline cvex::vint4 operator+(const cvex::vint4 a, const int b) { return _mm_add_epi32(a, cvex::splat(b)); }
static inline cvex::vint4 operator-(const cvex::vint4 a, const int b) { return _mm_sub_epi32(a, cvex::splat(b)); }
static inline cvex::vint4 operator*(const cvex::vint4 a, const int b) { return _mm_mullo_epi32(a, cvex::splat(b)); }
static inline cvex::vint4 operator/(const cvex::vint4 a, const int b)
{
  CVEX_ALIGNED(16) int temp_a[4];
  cvex::store(temp_a, a);
  return { temp_a[0] / b, temp_a[1] / b, temp_a[2] / b, temp_a[3] / b };
}

static inline cvex::vint4 operator+(const int a, const cvex::vint4 b) { return _mm_add_epi32(cvex::splat(a), b); }
static inline cvex::vint4 operator-(const int a, const cvex::vint4 b) { return _mm_sub_epi32(cvex::splat(a), b); }
static inline cvex::vint4 operator*(const int a, const cvex::vint4 b) { return _mm_mullo_epi32(cvex::splat(a), b); }
static inline cvex::vint4 operator/(const int a, const cvex::vint4 b)
{
  CVEX_ALIGNED(16) int temp_b[4];
  cvex::store(temp_b, b);
  return { a / temp_b[0], a / temp_b[1], a / temp_b[2], a / temp_b[3] };
}

static inline cvex::vint4 operator<<(const cvex::vint4 a, const int val) { return _mm_slli_epi32(a, val); }
static inline cvex::vint4 operator>>(const cvex::vint4 a, const int val) { return _mm_srai_epi32(a, val); }

static inline cvex::vint4 operator|(const cvex::vint4 a, const cvex::vint4 b) { return _mm_or_si128(a,b); }
static inline cvex::vint4 operator&(const cvex::vint4 a, const cvex::vint4 b) { return _mm_and_si128(a, b); }
static inline cvex::vint4 operator~(const cvex::vint4 a)                      { return _mm_andnot_si128(a, _mm_set1_epi32(0xFFFFFFFF)); }

static inline cvex::vint4 operator> (const cvex::vfloat4 a, const cvex::vfloat4 b) { return cvex::as_int32(_mm_cmpgt_ps(a, b)); }
static inline cvex::vint4 operator< (const cvex::vfloat4 a, const cvex::vfloat4 b) { return cvex::as_int32(_mm_cmplt_ps(a, b)); }
static inline cvex::vint4 operator>=(const cvex::vfloat4 a, const cvex::vfloat4 b) { return cvex::as_int32(_mm_cmpge_ps(a, b)); }
static inline cvex::vint4 operator<=(const cvex::vfloat4 a, const cvex::vfloat4 b) { return cvex::as_int32(_mm_cmple_ps(a, b)); }
static inline cvex::vint4 operator==(const cvex::vfloat4 a, const cvex::vfloat4 b) { return cvex::as_int32(_mm_cmpeq_ps(a, b)); }
static inline cvex::vint4 operator!=(const cvex::vfloat4 a, const cvex::vfloat4 b) { return cvex::as_int32(_mm_cmpneq_ps(a, b)); }

static inline cvex::vuint4 operator+(const cvex::vuint4 a, const cvex::vuint4 b) { return _mm_add_epi32(a, b); }
static inline cvex::vuint4 operator-(const cvex::vuint4 a, const cvex::vuint4 b) { return _mm_sub_epi32(a, b); }
static inline cvex::vuint4 operator*(const cvex::vuint4 a, const cvex::vuint4 b) { return _mm_mullo_epi32(a, b); }
static inline cvex::vuint4 operator/(const cvex::vuint4 a, const cvex::vuint4 b)
{
  CVEX_ALIGNED(16) unsigned int temp_a[4];
  CVEX_ALIGNED(16) unsigned int temp_b[4];
  cvex::store(temp_a, a);
  cvex::store(temp_b, b);
  return { temp_a[0] / temp_b[0], temp_a[1] / temp_b[1], temp_a[2] / temp_b[2], temp_a[3] / temp_b[3] };
}

static inline cvex::vuint4 operator+(const cvex::vuint4 a, const unsigned int b) { return _mm_add_epi32(a, cvex::splat(b)); }
static inline cvex::vuint4 operator-(const cvex::vuint4 a, const unsigned int b) { return _mm_sub_epi32(a, cvex::splat(b)); }
static inline cvex::vuint4 operator*(const cvex::vuint4 a, const unsigned int b) { return _mm_mullo_epi32(a, cvex::splat(b)); }
static inline cvex::vuint4 operator/(const cvex::vuint4 a, const unsigned int b)
{
  CVEX_ALIGNED(16) unsigned int temp_a[4];
  cvex::store(temp_a, a);
  return { temp_a[0] / b, temp_a[1] / b, temp_a[2] / b, temp_a[3] / b };
}

static inline cvex::vuint4 operator+(const unsigned int a, const cvex::vuint4 b) { return _mm_add_epi32(cvex::splat(a), b); }
static inline cvex::vuint4 operator-(const unsigned int a, const cvex::vuint4 b) { return _mm_sub_epi32(cvex::splat(a), b); }
static inline cvex::vuint4 operator*(const unsigned int a, const cvex::vuint4 b) { return _mm_mullo_epi32(cvex::splat(a), b); }
static inline cvex::vuint4 operator/(const unsigned int a, const cvex::vuint4 b)
{
  CVEX_ALIGNED(16) unsigned int temp_b[4];
  cvex::store(temp_b, b);
  return { a / temp_b[0], a / temp_b[1], a / temp_b[2], a / temp_b[3] };
}

static inline cvex::vuint4 operator<<(const cvex::vuint4 a, const int val) { return _mm_slli_epi32(a, val); }
static inline cvex::vuint4 operator>>(const cvex::vuint4 a, const int val) { return _mm_srli_epi32(a, val); }

static inline cvex::vuint4 operator|(const cvex::vuint4 a, const cvex::vuint4 b) { return _mm_or_si128(a, b); }
static inline cvex::vuint4 operator&(const cvex::vuint4 a, const cvex::vuint4 b) { return _mm_and_si128(a, b); }
static inline cvex::vuint4 operator~(const cvex::vuint4 a)                       { return _mm_andnot_si128(a, _mm_set1_epi32(0xFFFFFFFF)); }


};

#endif //TEST_GL_TOP_VFLOAT4_H
