#pragma once

//// (powf4, exp2f4, log2f4): http://jrfonseca.blogspot.ru/2008/09/fast-sse2-pow-tables-or-polynomials.html 

#include <math.h>
#include <cstdint>

#include <emmintrin.h>
#include <smmintrin.h> /// for _mm_packus_epi32
#include <xmmintrin.h>

#define EXP2_TABLE_SIZE_LOG2 9
#define EXP2_TABLE_SIZE (1 << EXP2_TABLE_SIZE_LOG2)
#define EXP2_TABLE_OFFSET (EXP2_TABLE_SIZE/2)
#define EXP2_TABLE_SCALE ((float) ((EXP2_TABLE_SIZE/2)-1))

#define LOG2_TABLE_SIZE_LOG2 8
#define LOG2_TABLE_SIZE (1 << LOG2_TABLE_SIZE_LOG2)
#define LOG2_TABLE_SCALE ((float) ((LOG2_TABLE_SIZE)-1))

namespace HydraSSE
{
  constexpr bool g_useSSE = true;

  /* 2 ^ x, for x in [-1.0, 1.0) */
  static float exp2_table[2 * EXP2_TABLE_SIZE];
  /* log2(x), for x in [1.0, 2.0) */
  static float log2_table[2 * LOG2_TABLE_SIZE];

  void exp2_init(void);
  void log2_init(void);
 
  union f4
  {
    int32_t  i[4];
    uint32_t u[4];
    float    f[4];
    __m128   m;
    __m128i  mi;
  };

  /**
  * Fast approximation to exp2(x).
  * Let ipart = int(x)
  * Let fpart = x - ipart;
  * So, exp2(x) = exp2(ipart) * exp2(fpart)
  * Compute exp2(ipart) with i << ipart
  * Compute exp2(fpart) with lookup table.
  */
  __m128 exp2f4(__m128 x);
  __m128 log2f4(__m128 x);


  static inline __m128 powf4(__m128 x, __m128 y)
  {
    return exp2f4(_mm_mul_ps(log2f4(x), y));
  }

  // those are is self-implemented
  //
  static const __m128 const_255 = _mm_set_ps1(255.0f);

  /**
  \brief implement realcolor (float4) to rgb256(int) conversion with gamma correction and clamping (min(color, 1.0f)).
  \param a_pointer - input address from which float4(__m128) color well be read
  \param normc     - normalisation constant; will be multiplied with read color
  \param gammaInv  - inverse gamma (usually 1.0f/2.2f)
  \param RGBA color packed in single int

  */
  static inline int gammaCorr(const float* a_pointer, const __m128 normc, const __m128 gammaInv)
  {
    const __m128 color1 = _mm_mul_ps(normc, _mm_load_ps(a_pointer));
    const __m128 color2 = powf4(color1, gammaInv);
    const __m128i rgba  = _mm_cvtps_epi32(_mm_min_ps(_mm_mul_ps(color2, const_255), const_255));
    const __m128i out   = _mm_packus_epi32(rgba, _mm_setzero_si128());
    const __m128i out2  = _mm_packus_epi16(out, _mm_setzero_si128());
    return _mm_cvtsi128_si32(out2);
  }

};