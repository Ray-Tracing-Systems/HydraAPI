//
// Created by Vladimir Frolov on 14.10.18.
//
// This library is created as lightweight and more optimised replacement for famous libsimdpp
// In the case if no implementation for tarhet architecture presents, you should include
// file (vfloat4_your_arch.h) that defines all operations and types via orger implementation
// so it can work ok different platforms anyway

#ifndef TEST_GL_TOP_VFLOAT4_H
#define TEST_GL_TOP_VFLOAT4_H

#ifdef WIN32
#include <intrin.h>
#else
#include <xmmintrin.h>
#endif


#if __GNUC__
#define ALIGN(X) __attribute__((__aligned__(X)))
#elif _MSC_VER
#define ALIGN(X) __declspec(align(X))
#else
#error "Unsupported compiler"
#endif

#ifndef _MM_DENORMALS_ZERO_MASK
#define _MM_DENORMALS_ZERO_MASK	0x0040
#endif

#ifndef _MM_DENORMALS_ZERO_ON
#define _MM_DENORMALS_ZERO_ON		0x0040
#endif

#ifndef _MM_SET_DENORMALS_ZERO_MODE
#define _MM_SET_DENORMALS_ZERO_MODE(mode) _mm_setcsr((_mm_getcsr() & ~_MM_DENORMALS_ZERO_MASK) | (mode))
#endif

namespace cvex
{
  typedef __m128  vfloat4;
  typedef __m128i vint4;

  #ifdef WIN32 // MVSC does not define operators !!!

  static inline vfloat4 operator+(vfloat4 a, vfloat4 b) { return _mm_add_ps(a, b); }
  static inline vfloat4 operator-(vfloat4 a, vfloat4 b) { return _mm_sub_ps(a, b); }
  static inline vfloat4 operator*(vfloat4 a, vfloat4 b) { return _mm_mul_ps(a, b); }
  static inline vfloat4 operator/(vfloat4 a, vfloat4 b) { return _mm_div_ps(a, b); }

  #endif

  static inline void set_ftz() { _MM_SET_ROUNDING_MODE(_MM_ROUND_TOWARD_ZERO);}

  static inline void store(float* data, vfloat4 a_val)   { _mm_store_ps(data, a_val);  }
  static inline void store(int*   data, vint4 a_val)     { _mm_store_ps((float*)data, _mm_castsi128_ps(a_val));  }

  static inline void store_u(float* data, vfloat4 a_val) { _mm_storeu_ps(data, a_val); }
  static inline void store_u(int*   data,  vint4 a_val)  { _mm_storeu_ps((float*)data, _mm_castsi128_ps(a_val)); }
  static inline void store_s(float* data, vfloat4 a_val) { _mm_store_ss(data, a_val);  } // store single ...

  static inline vfloat4 load (const float *data) { return _mm_load_ps(data);  }
  static inline vint4 load   (const int *data)   { return _mm_castps_si128(_mm_load_ps((float*)data));  }

  static inline vfloat4 load_u(const float *data)   { return _mm_loadu_ps(data); }
  static inline vint4   load_u(const int *data)     { return _mm_castps_si128(_mm_loadu_ps((float*)data)); }
  static inline vfloat4 load_s(const float *data)   { return _mm_load_ss(data);  }

  static inline int extract_0(const vint4 a_val)    { return _mm_cvtsi128_si32(a_val); }
  static inline int extract_1(const vint4 a_val)    { return _mm_cvtsi128_si32( _mm_shuffle_epi32(a_val, _MM_SHUFFLE(1,1,1,1)) ); }
  static inline int extract_2(const vint4 a_val)    { return _mm_cvtsi128_si32( _mm_shuffle_epi32(a_val, _MM_SHUFFLE(2,2,2,2)) ); }
  static inline int extract_3(const vint4 a_val)    { return _mm_cvtsi128_si32( _mm_shuffle_epi32(a_val, _MM_SHUFFLE(3,3,3,3)) ); }

  static inline vfloat4 shuffle_zyxw(vfloat4 a_src) { return _mm_shuffle_ps(a_src, a_src, _MM_SHUFFLE(3, 0, 1, 2)); }
  static inline vfloat4 shuffle_yzxw(vfloat4 a_src) { return _mm_shuffle_ps(a_src, a_src, _MM_SHUFFLE(3, 0, 2, 1)); }
  static inline vfloat4 shuffle_zxyw(vfloat4 a_src) { return _mm_shuffle_ps(a_src, a_src, _MM_SHUFFLE(3, 1, 0, 2)); }
  static inline vfloat4 shuffle_zwzw(vfloat4 a_src) { return _mm_shuffle_ps(a_src, a_src, _MM_SHUFFLE(3, 2, 3, 2)); }

  static inline void stream(void *data, vint4 a_val) { _mm_stream_si128((vint4 *) data, a_val); }

  static inline auto splat(const int i)   -> vint4   { return _mm_set_epi32(i, i, i, i); }
  static inline auto splat(const float i) -> vfloat4 { return _mm_set_ps   (i, i, i, i); }

  static inline vfloat4 splat_0(const vfloat4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0)); }
  static inline vfloat4 splat_1(const vfloat4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1)); }
  static inline vfloat4 splat_2(const vfloat4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2)); }
  static inline vfloat4 splat_3(const vfloat4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3)); }

  static inline vint4 make_vint(const int a, const int b, const int c, const int d) { return _mm_set_epi32(d, c, b, a); }

  static inline vfloat4 as_vfloat(const vint4 a_val) { return _mm_castsi128_ps(a_val); }
  static inline vint4   as_vint(const vfloat4 a_val) { return _mm_castps_si128(a_val); }

  static inline vint4   to_int32(const vfloat4 a_val) { return _mm_cvtps_epi32(a_val);}
  static inline vfloat4 to_float32(const vint4 a_val) { return _mm_cvtepi32_ps(a_val);}

  static inline vfloat4 floor(const vfloat4 a_val) { return _mm_floor_ps(a_val); }

  static inline vfloat4 min(const vfloat4 a, const vfloat4 b) {return _mm_min_ps(a, b);}
  static inline vfloat4 max(const vfloat4 a, const vfloat4 b) {return _mm_max_ps(a, b);}

  static inline vfloat4 rcp_e(const vfloat4 a) { return _mm_rcp_ps(a); }

  static inline vfloat4 blend(const vfloat4 a, const vfloat4 b, const vint4 mask)
  {
    return _mm_or_ps(_mm_and_ps(as_vfloat(mask), a),
                     _mm_andnot_ps(as_vfloat(mask), b));
  }

  static inline vint4 blend(const vint4 a, const vint4 b, const vint4 mask)
  {
    return as_vint(_mm_or_ps(_mm_and_ps   (as_vfloat(mask), as_vfloat(a)),
                             _mm_andnot_ps(as_vfloat(mask), as_vfloat(b))));
  }

  static inline void transpose4(vfloat4& a0, vfloat4& a1, vfloat4& a2, vfloat4& a3)
  {
    const vint4 b0 = _mm_unpacklo_epi32(as_vint(a0), as_vint(a1));
    const vint4 b1 = _mm_unpackhi_epi32(as_vint(a0), as_vint(a1));
    const vint4 b2 = _mm_unpacklo_epi32(as_vint(a2), as_vint(a3));
    const vint4 b3 = _mm_unpackhi_epi32(as_vint(a2), as_vint(a3));

    a0 = as_vfloat(_mm_unpacklo_epi64(b0, b2));
    a1 = as_vfloat(_mm_unpackhi_epi64(b0, b2));
    a2 = as_vfloat(_mm_unpacklo_epi64(b1, b3));
    a3 = as_vfloat(_mm_unpackhi_epi64(b1, b3));
  }

  inline static int color_compress_bgra(const vfloat4 rel_col)
  {
    static const vfloat4 const_255 = {255.0f, 255.0f, 255.0f, 255.0f};

    const __m128i rgba = _mm_cvtps_epi32(_mm_mul_ps(cvex::shuffle_zyxw(rel_col), const_255));
    const __m128i out  = _mm_packus_epi32(rgba, _mm_setzero_si128());
    const __m128i out2 = _mm_packus_epi16(out, _mm_setzero_si128());

    return _mm_cvtsi128_si32(out2);
  }

  static inline bool test_bits_any(const vint4 a) { return (_mm_movemask_ps(_mm_castsi128_ps(a)) & 15) != 0; }

  static inline bool test_all(const vfloat4 a) { return (_mm_movemask_ps(a) & 15) == 15; }
  static inline bool test_any(const vfloat4 a) { return (_mm_movemask_ps(a) & 15) != 0; }
  static inline bool test_all(const vint4 a)   { return (_mm_movemask_ps(_mm_castsi128_ps(a)) & 15) == 15; }
  static inline bool test_any(const vint4 a)   { return (_mm_movemask_ps(_mm_castsi128_ps(a)) & 15) != 0; }


  // it is strongly not recommended to use these functions because their general implementation could be slow
  //
  static inline vfloat4 shuffle2_xy_xy(const vfloat4 a, const vfloat4 b) { return _mm_shuffle_ps(a, b, _MM_SHUFFLE(1, 0, 1, 0)); }
  static inline vfloat4 shuffle2_xy_zw(const vfloat4 a, const vfloat4 b) { return _mm_shuffle_ps(a, b, _MM_SHUFFLE(3, 2, 1, 0)); }

  static inline __m128 sse_dot3(__m128 a, __m128 b) // please don't use _mm_dp_ps! It is from SSE 4.1 !!! Don't work on AMD Phenom
  {
    const __m128 mult  = _mm_mul_ps(a, b);
    const __m128 shuf1 = _mm_shuffle_ps(mult, mult, _MM_SHUFFLE(0, 3, 2, 1));
    const __m128 shuf2 = _mm_shuffle_ps(mult, mult, _MM_SHUFFLE(1, 0, 3, 2));
    return _mm_add_ss(_mm_add_ss(mult, shuf1), shuf2);
  }

  static inline vfloat4 dot3v(const vfloat4 a, const vfloat4 b) { return sse_dot3(a, b); } // _mm_dp_ps(a, b, 0x7f);
  static inline float   dot3f(const vfloat4 a, const vfloat4 b) { return _mm_cvtss_f32(sse_dot3(a, b)); }

  //static inline bool cmpgt_all_xyzw(const vfloat4 a, const vfloat4 b) { return (_mm_movemask_ps(_mm_cmpgt_ps(a, b)) & 15) == 15; } // #TODO: UNTESTED!
  static inline bool cmpgt_all_xyz (const vfloat4 a, const vfloat4 b) { return (_mm_movemask_ps(_mm_cmpgt_ps(a, b)) & 7)  == 7; }
  static inline bool cmpgt_all_x   (const vfloat4 a, const vfloat4 b) { return (_mm_movemask_ps(_mm_cmpgt_ss(a, b)) & 1)  == 1; }
  static inline bool cmpgt_any     (const vfloat4 a, const vfloat4 b) { return  _mm_movemask_ps(_mm_cmpgt_ps(a, b))       != 0; }

  // it is not recommended to use these functions because they are not general, but more hw specific
  // due to _mm_***_ss is the x64 only feature, so, when using these functions you must guarantee that
  // only first vector component must be used further. Other components are undefined!
  //
  static inline vfloat4 add_s(vfloat4 a, vfloat4 b) { return _mm_add_ss(a,b); } // #NOTE: assume you will never use .yzw coordinates!; only .x is valid!
  static inline vfloat4 sub_s(vfloat4 a, vfloat4 b) { return _mm_sub_ss(a,b); } // #NOTE: assume you will never use .yzw coordinates!; only .x is valid!
  static inline vfloat4 mul_s(vfloat4 a, vfloat4 b) { return _mm_mul_ss(a,b); } // #NOTE: assume you will never use .yzw coordinates!; only .x is valid!
  static inline vfloat4 div_s(vfloat4 a, vfloat4 b) { return _mm_div_ss(a,b); } // #NOTE: assume you will never use .yzw coordinates!; only .x is valid!
  static inline vfloat4 rcp_s(vfloat4 a)            { return _mm_rcp_ss(a);   } // #NOTE: assume you will never use .yzw coordinates!; only .x is valid!


};

#endif //TEST_GL_TOP_VFLOAT4_H
