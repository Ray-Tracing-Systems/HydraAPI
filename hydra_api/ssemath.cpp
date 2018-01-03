#include "ssemath.h"

//// (powf4, exp2f4, log2f4): http://jrfonseca.blogspot.ru/2008/09/fast-sse2-pow-tables-or-polynomials.html 

void HydraSSE::exp2_init(void)
{
  int i;
  for (i = 0; i < EXP2_TABLE_SIZE; i++)
    exp2_table[i] = (float)pow(2.0, (i - EXP2_TABLE_OFFSET) / EXP2_TABLE_SCALE);
}


void HydraSSE::log2_init(void)
{
  unsigned i;
  for (i = 0; i < LOG2_TABLE_SIZE; i++)
    log2_table[i] = (float)log2(1.0 + i * (1.0 / (LOG2_TABLE_SIZE - 1)));
}

__m128 HydraSSE::exp2f4(__m128 x)
{
  __m128i ipart;
  __m128 fpart, expipart;
  union f4 index, expfpart;

  x = _mm_min_ps(x, _mm_set1_ps(129.00000f));
  x = _mm_max_ps(x, _mm_set1_ps(-126.99999f));

  /* ipart = int(x) */
  ipart = _mm_cvtps_epi32(x);

  /* fpart = x - ipart */
  fpart = _mm_sub_ps(x, _mm_cvtepi32_ps(ipart));

  /* expipart = (float) (1 << ipart) */
  expipart = _mm_castsi128_ps(_mm_slli_epi32(_mm_add_epi32(ipart, _mm_set1_epi32(127)), 23));

  /* index = EXP2_TABLE_OFFSET + (int)(fpart * EXP2_TABLE_SCALE) */
  index.mi = _mm_add_epi32(_mm_cvtps_epi32(_mm_mul_ps(fpart, _mm_set1_ps(EXP2_TABLE_SCALE))), _mm_set1_epi32(EXP2_TABLE_OFFSET));

  expfpart.f[0] = exp2_table[index.u[0]];
  expfpart.f[1] = exp2_table[index.u[1]];
  expfpart.f[2] = exp2_table[index.u[2]];
  expfpart.f[3] = exp2_table[index.u[3]];

  return _mm_mul_ps(expipart, expfpart.m);
}

__m128 HydraSSE::log2f4(__m128 x)
{
  union f4 index, p;

  __m128i exp  = _mm_set1_epi32(0x7F800000);
  __m128i mant = _mm_set1_epi32(0x007FFFFF);
  __m128i i    = _mm_castps_si128(x);
  __m128 e     = _mm_cvtepi32_ps(_mm_sub_epi32(_mm_srli_epi32(_mm_and_si128(i, exp), 23), _mm_set1_epi32(127)));
  index.mi     = _mm_srli_epi32(_mm_and_si128(i, mant), 23 - LOG2_TABLE_SIZE_LOG2);

  p.f[0] = log2_table[index.u[0]];
  p.f[1] = log2_table[index.u[1]];
  p.f[2] = log2_table[index.u[2]];
  p.f[3] = log2_table[index.u[3]];

  return _mm_add_ps(p.m, e);
}
