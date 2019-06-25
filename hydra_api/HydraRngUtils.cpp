//
// Created by frol on 25.06.19.
//

#include "HydraAPI.h"
#include <cmath>

namespace hr_prng
{
  static inline unsigned int NextState(RandomGen *gen)
  {
    const unsigned int x = (gen->state).x * 17 + (gen->state).y * 13123;
    (gen->state).x       = (x << 13) ^ x;
    (gen->state).y      ^= (x << 7);
    return x;
  }

  static inline int mapRndFloatToInt(float a_val, int a, int b)
  {
    const float fa = (float) (a + 0);
    const float fb = (float) (b + 1);
    const float fR = fa + a_val * (fb - fa);

    const int res = (int) (fR);

    if (res > b)
      return b;
    else
      return res;
  }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float hr_prng::rndFloat(RandomGen *gen)
{
  const unsigned int x   = NextState(gen);
  const unsigned int tmp = (x * (x * x * 15731 + 74323) + 871483);
  const float scale      = (1.0f / 4294967296.0f);
  return ((float) (tmp)) * scale;
}

hr_prng::RandomGen hr_prng::RandomGenInit(const int a_seed)
{
  RandomGen gen;
  gen.state.x = (a_seed * (a_seed * a_seed * 15731 + 74323) + 871483);
  gen.state.y = (a_seed * (a_seed * a_seed * 13734 + 37828) + 234234);

  for (int i = 0; i < (a_seed % 7); i++)
    NextState(&gen);

  return gen;
}

int hr_prng::rndIntUniform(RandomGen& gen, int a, int b)
{
  return mapRndFloatToInt(rndFloat(&gen), a, b);
}

float hr_prng::rndFloatUniform(RandomGen &gen, float s, float e)
{
  const float t = rndFloat(&gen);
  return s + t * (e - s);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef long long int INT64;

using hr_qmc::QRNG_DIMENSIONS;
using hr_qmc::QRNG_RESOLUTION;

////////////////////////////////////////////////////////////////////////////////
// Table generation functions
////////////////////////////////////////////////////////////////////////////////
// Internal 64(63)-bit table
static INT64 cjn[63][QRNG_DIMENSIONS];

static int GeneratePolynomials(int buffer[QRNG_DIMENSIONS], bool primitive)
{
    int i, j, n, p1, p2, l;
    int e_p1, e_p2, e_b;

    //generate all polynomials to buffer
    for(n = 1, buffer[0] = 0x2, p2 = 0, l = 0; n < QRNG_DIMENSIONS; ++n){
        //search for the next irreducable polynomial
        for(p1 = buffer[n - 1] + 1; ; ++p1){
            //find degree of polynomial p1
            for(e_p1 = 30; (p1 & (1 << e_p1)) == 0; --e_p1) {}

            // try to divide p1 by all polynomials in buffer
            for(i = 0; i < n; ++i){
                // find the degree of buffer[i]
                for(e_b = e_p1; (buffer[i] & (1 << e_b)) == 0; --e_b) {}

                // divide p2 by buffer[i] until the end
                for(p2 = (buffer[i] << ((e_p2 = e_p1) - e_b)) ^ p1; p2 >= buffer[i]; p2 = (buffer[i] << (e_p2 - e_b)) ^ p2){
                    for( ; (p2 & (1 << e_p2)) == 0; --e_p2) {}
                }// compute new degree of p2

                // division without remainder!!! p1 is not irreducable
                if(p2 == 0){
                    break;
                }
            }

            //all divisions were with remainder - p1 is irreducable
            if(p2 != 0){
                e_p2 = 0;
                if(primitive){
                    //check that p1 has only one cycle (i.e. is monic, or primitive)
                    j = ~(0xffffffff << (e_p1 + 1));
                    e_b = (1 << e_p1) | 0x1;
                    for(p2 = e_b, e_p2 = (1 << e_p1) - 2; e_p2 > 0; --e_p2){
                        p2 <<= 1;
                        i = p2 & p1;
                        i = (i & 0x55555555) + ((i >> 1) & 0x55555555);
                        i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
                        i = (i & 0x07070707) + ((i >> 4) & 0x07070707);
                        p2 |= (i % 255) & 1;
                        if ((p2 & j) == e_b) break;
                    }
                }
                //it is monic - add it to the list of polynomials
                if(e_p2 == 0){
                    buffer[n] = p1;
                    l += e_p1;
                    break;
                }
            }
        }
    }
    return l + 1;
}


////////////////////////////////////////////////////////////////////////////////
//  @misc{Bratley92:LDS,
//    author = "B. Fox and P. Bratley and H. Niederreiter",
//    title = "Implementation and test of low discrepancy sequences",
//    text = "B. L. Fox, P. Bratley, and H. Niederreiter. Implementation and test of
//      low discrepancy sequences. ACM Trans. Model. Comput. Simul., 2(3):195--213,
//      July 1992.",
//    year = "1992" }
////////////////////////////////////////////////////////////////////////////////
static void GenerateCJ(){
    int buffer[QRNG_DIMENSIONS];
    int *polynomials;
    int n, p1, l, e_p1;

    // Niederreiter (in contrast to Sobol) allows to use not primitive, but just irreducable polynomials
    l = GeneratePolynomials(buffer, false);

    // convert all polynomials from buffer to polynomials table
    polynomials = new int[l + 2 * QRNG_DIMENSIONS + 1];
    for(n = 0, l = 0; n < QRNG_DIMENSIONS; ++n){
        //find degree of polynomial p1
        for(p1 = buffer[n], e_p1 = 30; (p1 & (1 << e_p1)) == 0; --e_p1) {}

        //fill polynomials table with values for this polynomial
        polynomials[l++] = 1;
        for(--e_p1; e_p1 >= 0; --e_p1){
            polynomials[l++] = (p1 >> e_p1) & 1;
        }
        polynomials[l++] = -1;
    }
    polynomials[l] = -1;

    // irreducable polynomial p
    int *p = polynomials, e, d;
    // polynomial b
    int b_arr[1024], *b, m;
    // v array
    int v_arr[1024], *v;
    // temporary polynomial, required to do multiplication of p and b
    int t_arr[1024], *t;
    // subsidiary variables
    int i, j, u, m1, ip, it;

    // cycle over monic irreducible polynomials
    for(d = 0; p[0] != -1; p += e + 2){
        // allocate memory for cj array for dimention (ip + 1)
        for(i = 0; i < 63; ++i){
            cjn[i][d] = 0;
        }

        // determine the power of irreducable polynomial
        for(e = 0; p[e + 1] != -1; ++e) {}
        // polynomial b in the beginning is just '1'
        (b = b_arr + 1023)[m = 0] = 1;
        // v array needs only (63 + e - 2) length
        v = v_arr + 1023 - (63 + e - 2);

        // cycle over all coefficients
        for(j = 63 - 1, u = e; j >= 0; --j, ++u){
            if(u == e){
                u = 0;
                // multiply b by p (polynomials multiplication)
                for(i = 0, t = t_arr + 1023 - (m1 = m); i <= m; ++i){
                    t[i] = b[i];
                }
                b = b_arr + 1023 - (m += e);

                for(i = 0; i <= m; ++i){
                    b[i] = 0;
                    for(ip = e - (m - i), it = m1; ip <= e && it >= 0; ++ip, --it){
                        if(ip >= 0){
                            b[i] ^= p[ip] & t[it];
                        }
                    }
                }
                // multiplication of polynomials finished

                // calculate v
                for(i = 0; i < m1; ++i){
                    v[i] = 0;
                }
                for(; i < m; ++i){
                    v[i] = 1;
                }
                for(; i <= 63 + e - 2; ++i){
                    v[i] = 0;
                    for (it = 1; it <= m; ++it){
                        v[i] ^= v[i - it] & b[it];
                    }
                }
            }

            // copy calculated v to cj
            for(i = 0; i < 63; i++){
                cjn[i][d] |= (INT64)v[i + u] << j;
            }
        }
        ++d;
    }

    delete []polynomials;
}


////////////////////////////////////////////////////////////////////////////////
// Initialization (table setup)
////////////////////////////////////////////////////////////////////////////////
void hr_qmc::init(unsigned int table[QRNG_DIMENSIONS][QRNG_RESOLUTION])
{
  GenerateCJ();

  for(int dim = 0; dim < QRNG_DIMENSIONS; dim++)
    for(int bit = 0; bit < QRNG_RESOLUTION; bit++)
      table[dim][bit] = (int)((cjn[bit][dim] >> 32) & 0x7FFFFFFF);
}


float hr_qmc::rndFloat(unsigned int pos, int dim, unsigned int *c_Table)
{
  unsigned int result = 0;
  unsigned int data   = pos;
  for (int bit = 0; bit < QRNG_RESOLUTION; bit++, data >>= 1)
    if (data & 1) result ^= c_Table[bit + dim*QRNG_RESOLUTION];
  return (float)(result + 1) * INT_SCALE;
}

float hr_qmc::rndFloatUniform(unsigned int pos, int dim, unsigned int *c_Table, float s, float e)
{
  const float t = rndFloat(pos,dim, c_Table);
  return s + t * (e - s);
}

int hr_qmc::rndIntUniform(unsigned int pos, int dim, unsigned int *c_Table, int a, int b)
{
  return hr_prng::mapRndFloatToInt(rndFloat(pos, dim, c_Table), a, b);
}
