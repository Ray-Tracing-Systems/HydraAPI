//
// Created by frol on 30.10.18.
//

#ifndef TEST_GL_TOP_VFLOAT4_ARM_H
#define TEST_GL_TOP_VFLOAT4_ARM_H

#include <arm_neon.h>

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

  struct vfloat4
  {
    inline vfloat4() {}
    inline vfloat4(float32x4_t a) {data = a;}
    inline operator float32x4_t() const {return data;}
    inline vfloat4(const std::initializer_list<float> v) { data = vld1q_f32(v.begin()); }

    float32x4_t data;   
  };

  struct vint4 
  {
    inline vint4() {}
    inline vint4(int32x4_t a) {data = a;}
    inline operator int32x4_t() const {return data;}
    inline vint4(const std::initializer_list<int> v) { data = vld1q_s32(v.begin()); }

    int32x4_t data;
  };

  struct vuint4 
  {
    inline vuint4() {}
    inline vuint4(uint32x4_t a) {data = a;}
    inline operator uint32x4_t() const { return data; }
    inline vuint4(const std::initializer_list<_uint32_t> v) { data = vld1q_u32(v.begin()); }

    uint32x4_t data;
  };  

  static inline vfloat4 load  (const float* p) { return vld1q_f32(p); }
  static inline vfloat4 load_u(const float* p) { return vld1q_f32(p); }

  static inline void store  (float* p, vfloat4 a_val) { vst1q_f32(p, a_val); }
  static inline void store_u(float* p, vfloat4 a_val) { vst1q_f32(p, a_val); }

 
  static inline vuint4  load  (const unsigned int* p) { return vld1q_u32(p); }
  static inline vuint4  load_u(const unsigned int* p) { return vld1q_u32(p); }

  static inline vint4   load  (const int* p)          { return vld1q_s32(p); }
  static inline vint4   load_u(const int* p)          { return vld1q_s32(p); }

  static inline void store  (unsigned int* p, vuint4 a_val) { vst1q_u32(p, a_val); }
  static inline void store_u(unsigned int* p, vuint4 a_val) { vst1q_u32(p, a_val); }

  static inline void store  (int* p, vint4 a_val)  { vst1q_s32(p, a_val); }
  static inline void store_u(int* p, vint4 a_val)  { vst1q_s32(p, a_val); }

  static inline vuint4  splat(unsigned int x)  { return vld1q_dup_u32(&x); }
  static inline vint4   splat(int x)           { return vld1q_dup_s32(&x); }
  static inline vfloat4 splat(float x)         { return vld1q_dup_f32(&x); }

  static inline vfloat4 to_float32(vint4 a)    { return vcvtq_f32_s32(a); }
  static inline vfloat4 to_float32(vuint4 a)   { return vcvtq_f32_u32(a); }
  
  static inline vint4   to_int32  (vuint4 a)   { return (int32x4_t)a.data;}
  static inline vint4   to_int32  (vfloat4 a)  { return vcvtq_s32_f32(a); }
  static inline vuint4  to_uint32 (vfloat4 a)  { return vcvtq_u32_f32(a); }
  static inline vuint4  to_uint32 (vint4 a)    { return (uint32x4_t)a.data; }

  static inline vfloat4 as_float32(const vint4 a_val)   { return vreinterpretq_f32_s32(a_val); }
  static inline vfloat4 as_float32(const vuint4 a_val)  { return vreinterpretq_f32_u32(a_val); }
  static inline vint4   as_int32  (const vfloat4 a_val) { return vreinterpretq_s32_f32(a_val); }
  static inline vuint4  as_uint32 (const vfloat4 a_val) { return vreinterpretq_u32_f32(a_val); }

  static inline vfloat4 rcp_e(vfloat4 a)                { return vrecpeq_f32(a); }

  static inline vfloat4 blend(const vfloat4 a, const vfloat4 b, const vint4 mask) { return vbslq_f32((uint32x4_t)mask.data, a.data, b.data); }
  static inline vint4   blend(const vint4 a,   const vint4 b,   const vint4 mask) { return vbslq_s32((uint32x4_t)mask.data, a.data, b.data); }
  static inline vuint4  blend(const vuint4 a,  const vuint4 b,  const vint4 mask) { return vbslq_u32((uint32x4_t)mask.data, a.data, b.data); }

  static inline vfloat4 blend(const vfloat4 a, const vfloat4 b, const vuint4 mask) { return vbslq_f32(mask.data, a.data, b.data); }
  static inline vint4   blend(const vint4 a,   const vint4 b,   const vuint4 mask) { return vbslq_s32(mask.data, a.data, b.data); }
  static inline vuint4  blend(const vuint4 a,  const vuint4 b,  const vuint4 mask) { return vbslq_u32(mask.data, a.data, b.data); }


  static inline bool test_bits_any(const vint4 a)
  {
    const uint32x2_t tmp = vorr_u32(vget_low_u32((uint32x4_t)a.data), 
                                    vget_high_u32((uint32x4_t)a.data));
    return (vget_lane_u32(vpmax_u32(tmp, tmp), 0) != 0);
  }

  static inline bool test_bits_any(const vuint4 a)
  {
    const uint32x2_t tmp = vorr_u32(vget_low_u32((uint32x4_t)a.data), 
                                    vget_high_u32((uint32x4_t)a.data));
    return (vget_lane_u32(vpmax_u32(tmp, tmp), 0) != 0);
  }

  static inline bool test_bits_any(const vfloat4 a_f)
  {
    const vint4 a = as_int32(a_f);
    const uint32x2_t tmp = vorr_u32(vget_low_u32((uint32x4_t)a.data), 
                                    vget_high_u32((uint32x4_t)a.data));
    return (vget_lane_u32(vpmax_u32(tmp, tmp), 0) != 0);
  }

  static inline bool test_bits_all(const vuint4 v)  { return !test_bits_any(vmvnq_u32(v)); }
  static inline bool test_bits_all(const vint4 v)   { return !test_bits_any(vmvnq_s32(v)); }
  static inline bool test_bits_all(const vfloat4 v) { return test_bits_all(as_int32(v)); }
 
  static inline vfloat4 min(const vfloat4 a, const vfloat4 b)                              { return vminq_f32(a.data,b.data); }
  static inline vfloat4 max(const vfloat4 a, const vfloat4 b)                              { return vmaxq_f32(a.data,b.data); }
  static inline vfloat4 clamp(const vfloat4 x, const vfloat4 minVal, const vfloat4 maxVal) { return max(min(x, maxVal), minVal);  }

  static inline vint4  min(const vint4 a,  const vint4 b)                           { return vminq_s32(a.data,b.data); }
  static inline vint4  max(const vint4 a,  const vint4 b)                           { return vmaxq_s32(a.data,b.data); }
  static inline vint4  clamp(const vint4 x, const vint4 minVal, const vint4 maxVal) { return vmaxq_s32(vminq_s32(x, maxVal), minVal);  }

  static inline vuint4 min(const vuint4 a, const vuint4 b)                             { return vminq_u32(a.data,b.data); }
  static inline vuint4 max(const vuint4 a, const vuint4 b)                             { return vmaxq_u32(a.data,b.data); }
  static inline vuint4 clamp(const vuint4 x, const vuint4 minVal, const vuint4 maxVal) { return vmaxq_u32(vminq_u32(x, maxVal), minVal);  }

  static inline float extract_0(const vfloat4 a_val) { return vget_lane_f32(vget_low_f32(a_val.data), 0);  } // performance alert: Don't mix NEON & non-NEON code!!!
  static inline float extract_1(const vfloat4 a_val) { return vget_lane_f32(vget_low_f32(a_val.data), 1);  } // don't use vget_lane (!!!)
  static inline float extract_2(const vfloat4 a_val) { return vget_lane_f32(vget_high_f32(a_val.data), 0); } // performance alert: Don't mix NEON & non-NEON code!!!
  static inline float extract_3(const vfloat4 a_val) { return vget_lane_f32(vget_high_f32(a_val.data), 1); } // don't use vget_lane (!!!)

  static inline int extract_0(const vint4 a_val)     { return vget_lane_s32(vget_low_s32(a_val.data), 0);  } // performance alert: Don't mix NEON & non-NEON code!!!
  static inline int extract_1(const vint4 a_val)     { return vget_lane_s32(vget_low_s32(a_val.data), 1);  } // don't use vget_lane (!!!)
  static inline int extract_2(const vint4 a_val)     { return vget_lane_s32(vget_high_s32(a_val.data), 0); } // performance alert: Don't mix NEON & non-NEON code!!!
  static inline int extract_3(const vint4 a_val)     { return vget_lane_s32(vget_high_s32(a_val.data), 1); } // don't use vget_lane (!!!)
  
  static inline unsigned int extract_0(const vuint4 a_val) { return vget_lane_u32(vget_low_u32(a_val.data), 0);  }
  static inline unsigned int extract_1(const vuint4 a_val) { return vget_lane_u32(vget_low_u32(a_val.data), 1);  }
  static inline unsigned int extract_2(const vuint4 a_val) { return vget_lane_u32(vget_high_u32(a_val.data), 0); }
  static inline unsigned int extract_3(const vuint4 a_val) { return vget_lane_u32(vget_high_u32(a_val.data), 1); }

  static inline vfloat4 splat_0(const vfloat4 v) { return vdupq_lane_f32(vget_low_f32 (v.data), 0); }
  static inline vfloat4 splat_1(const vfloat4 v) { return vdupq_lane_f32(vget_low_f32 (v.data), 1); }
  static inline vfloat4 splat_2(const vfloat4 v) { return vdupq_lane_f32(vget_high_f32(v.data), 0); }
  static inline vfloat4 splat_3(const vfloat4 v) { return vdupq_lane_f32(vget_high_f32(v.data), 1); }

  static inline vint4 splat_0(const vint4 v) { return vdupq_lane_s32(vget_low_s32 (v.data), 0); }
  static inline vint4 splat_1(const vint4 v) { return vdupq_lane_s32(vget_low_s32 (v.data), 1); }
  static inline vint4 splat_2(const vint4 v) { return vdupq_lane_s32(vget_high_s32(v.data), 0); }
  static inline vint4 splat_3(const vint4 v) { return vdupq_lane_s32(vget_high_s32(v.data), 1); }

  static inline vuint4 splat_0(const vuint4 v) { return vdupq_lane_u32(vget_low_u32 (v.data), 0); }
  static inline vuint4 splat_1(const vuint4 v) { return vdupq_lane_u32(vget_low_u32 (v.data), 1); }
  static inline vuint4 splat_2(const vuint4 v) { return vdupq_lane_u32(vget_high_u32(v.data), 0); }
  static inline vuint4 splat_3(const vuint4 v) { return vdupq_lane_u32(vget_high_u32(v.data), 1); }

  static inline vfloat4 dot4v(const vfloat4 a, const vfloat4 b)
  {
    const float32x4_t prod = vmulq_f32(a.data, b.data);
    const float32x4_t sum1 = vaddq_f32(prod, vrev64q_f32(prod));
    const float32x4_t sum2 = vaddq_f32(sum1, vcombine_f32(vget_high_f32(sum1), vget_low_f32(sum1)));
    return sum2;
  }

  static inline vfloat4 dot3v(const vfloat4 a, const vfloat4 b)
  {
    const float32x4_t com3 = {1.0f, 1.0f, 1.0f, 0.0f};
    const float32x4_t prd3 = vmulq_f32(a.data, com3);
    const float32x4_t prod = vmulq_f32(prd3, b.data);
    const float32x4_t sum1 = vaddq_f32(prod, vrev64q_f32(prod));
    const float32x4_t sum2 = vaddq_f32(sum1, vcombine_f32(vget_high_f32(sum1), vget_low_f32(sum1)));
    return sum2;
  }

  static inline float dot3f(const vfloat4 a, const vfloat4 b) 
  {
    CVEX_ALIGNED(16) float mres[4];
    const float32x4_t a2 = vmulq_f32(a.data, b.data);
    vst1q_f32(mres, a2);
    return mres[0] + mres[1] + mres[2];
  }

  static inline float dot4f(const vfloat4 a, const vfloat4 b) 
  {
    CVEX_ALIGNED(16) float mres[4];
    const float32x4_t a2 = vmulq_f32(a.data, b.data);
    vst1q_f32(mres, a2);
    return mres[0] + mres[1] + mres[2] + mres[3];
  }

  static inline float length3f(const vfloat4 a) 
  {
    CVEX_ALIGNED(16) float mres[4];
    const float32x4_t a2 = vmulq_f32(a.data, a.data);
    vst1q_f32(mres, a2);
    return sqrtf(mres[0] + mres[1] + mres[2]);
  }

  static inline float length4f(const vfloat4 a) 
  {
    CVEX_ALIGNED(16) float mres[4];
    const float32x4_t a2 = vmulq_f32(a.data, a.data);
    vst1q_f32(mres, a2);
    return sqrtf(mres[0] + mres[1] + mres[2] + mres[3]);
  }

  static inline vfloat4 length3v(const vfloat4 a) 
  {
    CVEX_ALIGNED(16) float mres[4];
    const float32x4_t a2 = vmulq_f32(a.data, a.data);
    vst1q_f32(mres, a2);

    const float res = sqrtf(mres[0] + mres[1] + mres[2]);
    return cvex::splat(res); 
  }

  static inline vfloat4 length4v(const vfloat4 a) 
  {
    CVEX_ALIGNED(16) float mres[4];
    const float32x4_t a2 = vmulq_f32(a.data, a.data);
    vst1q_f32(mres, a2);

    const float res    = sqrtf(mres[0] + mres[1] + mres[2] + mres[3]);
    return cvex::splat(res); 
  }
  
  inline vfloat4 VectorSwizzleGeneral(vfloat4 V, uint32_t E0, uint32_t E1, uint32_t E2, uint32_t E3)
  {
    static const uint32_t ControlElement[ 4 ] =
    {
        0x03020100, // XM_SWIZZLE_X
        0x07060504, // XM_SWIZZLE_Y
        0x0B0A0908, // XM_SWIZZLE_Z
        0x0F0E0D0C, // XM_SWIZZLE_W
    };
     
    struct TEMP
    {
      union 
      {
        float32x2x2_t tbl;
        uint8x8x2_t   tbl2;
      };
    } temp;
    temp.tbl.val[0] = vget_low_f32(V.data);
    temp.tbl.val[1] = vget_high_f32(V.data);

    uint32x2_t idx = vcreate_u32( ((uint64_t)ControlElement[E0]) | (((uint64_t)ControlElement[E1]) << 32) );
    const uint8x8_t rL = vtbl2_u8( temp.tbl2, vreinterpret_u8_u32(idx) );

    idx = vcreate_u32( ((uint64_t)ControlElement[E2]) | (((uint64_t)ControlElement[E3]) << 32) );
    const uint8x8_t rH = vtbl2_u8( temp.tbl2, vreinterpret_u8_u32(idx) );

    return vcombine_f32( vreinterpret_f32_u8(rL), vreinterpret_f32_u8(rH) );
  }

  static inline vfloat4 shuffle_zyxw(vfloat4 a) { return VectorSwizzleGeneral(a,2,1,0,3); }
  static inline vfloat4 shuffle_yzxw(vfloat4 a) { return VectorSwizzleGeneral(a,1,2,0,3); }
  static inline vfloat4 shuffle_zxyw(vfloat4 v) { return VectorSwizzleGeneral(v,2,0,1,3); }

  static inline vfloat4 shuffle_xyxy(vfloat4 a) 
  { 
    const float32x2_t a01 = vget_low_f32(a); // vrev64_f32 to get yxyx
	  return vcombine_f32(a01, a01);
  }

  static inline vfloat4 shuffle_zwzw(vfloat4 a) 
  { 
	  const float32x2_t b01 = vget_high_f32(a);
	  return vcombine_f32(b01, b01);
  }

  static inline void cross3_c(float v0[3], float v1[3], float d[3])
  {
  	d[0] = v0[1]*v1[2] - v0[2]*v1[1];
  	d[1] = v0[2]*v1[0] - v0[0]*v1[2];
  	d[2] = v0[0]*v1[1] - v0[1]*v1[0];
  }

  static inline vfloat4 cross3(const vfloat4 a, const vfloat4 b) 
  {
    CVEX_ALIGNED(16) float arr_a[4];
    CVEX_ALIGNED(16) float arr_b[4];
    CVEX_ALIGNED(16) float arr_c[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    cvex::store(arr_a, a);
    cvex::store(arr_b, b);

    cross3_c(arr_a, arr_b, arr_c);

    return cvex::load(arr_c);
  }

  static inline vfloat4 lerp(const vfloat4 u, const vfloat4 v, const float t) 
  {
    const float32x4_t vt   = vld1q_dup_f32(&t);
    const float32x4_t diff = vsubq_f32(u.data, v.data);
    return vmlaq_f32(u.data, vt, diff); // vmlaq_f32(a, b, c) | Multiply and add 	a + (b * c)
  }

  static inline vfloat4 ceil(const vfloat4 a)
  {
    CVEX_ALIGNED(16) float mres[4];
    vst1q_f32(mres, a);
    const vfloat4 res = {::ceilf(mres[0]), ::ceilf(mres[1]), ::ceilf(mres[2]), ::ceilf(mres[3])};
    return res;
  }

  static inline vfloat4 floor(const vfloat4 a)
  {
    CVEX_ALIGNED(16) float mres[4];
    vst1q_f32(mres, a);
    const vfloat4 res = {::floorf(mres[0]), ::floorf(mres[1]), ::floorf(mres[2]), ::floorf(mres[3])};
    return res;
  }

  static inline vfloat4 fabs(const vfloat4 a)
  {
    CVEX_ALIGNED(16) float mres[4];
    vst1q_f32(mres, a);
    const vfloat4 res = {::fabs(mres[0]), ::fabs(mres[1]), ::fabs(mres[2]), ::fabs(mres[3])};
    return res;
  }

  static inline bool cmp_gt3(const vfloat4 a, const vfloat4 b) 
  { 
    CVEX_ALIGNED(16) float aa[4];
    CVEX_ALIGNED(16) float ab[4];
    vst1q_f32(aa, a);
    vst1q_f32(ab, b);
    return (aa[0] > ab[0]) && (aa[1] > ab[1]) && (aa[2] > ab[2]); 
  }

  static inline bool cmp_lt3(const vfloat4 a, const vfloat4 b) 
  { 
    CVEX_ALIGNED(16) float aa[4];
    CVEX_ALIGNED(16) float ab[4];
    vst1q_f32(aa, a);
    vst1q_f32(ab, b);
    return (aa[0] < ab[0]) && (aa[1] < ab[1]) && (aa[2] < ab[2]); 
  }

  static inline bool cmp_ge3(const vfloat4 a, const vfloat4 b) 
  { 
    CVEX_ALIGNED(16) float aa[4];
    CVEX_ALIGNED(16) float ab[4];
    vst1q_f32(aa, a);
    vst1q_f32(ab, b);
    return (aa[0] >= ab[0]) && (aa[1] >= ab[1]) && (aa[2] >= ab[2]); 
  }

  static inline bool cmp_le3(const vfloat4 a, const vfloat4 b) 
  { 
    CVEX_ALIGNED(16) float aa[4];
    CVEX_ALIGNED(16) float ab[4];
    vst1q_f32(aa, a);
    vst1q_f32(ab, b);
    return (aa[0] <= ab[0]) && (aa[1] <= ab[1]) && (aa[2] <= ab[2]); 
  }

  inline static unsigned int color_pack_rgba(const vfloat4 rel_col)
  {
    const vfloat4 const_255 = { 255.0f, 255.0f, 255.0f, 255.0f };
    const vuint4 vrgba      = to_uint32(vmulq_f32(rel_col, const_255));

    CVEX_ALIGNED(16) unsigned int rgba[4];
    cvex::store(rgba, vrgba);

    //std::cout << "rgba: " << rgba[0] << "\t" << rgba[1] << "\t" << rgba[2] << "\t" << rgba[3] << std::endl;

    return (rgba[3] << 24) | (rgba[2] << 16) | (rgba[1] << 8) | rgba[0];
  }

  inline static unsigned int color_pack_bgra(const vfloat4 rel_col)
  {
    const vfloat4 const_255 = { 255.0f, 255.0f, 255.0f, 255.0f };
    const vuint4 vrgba      = to_uint32(vmulq_f32(cvex::shuffle_zyxw(rel_col), const_255));

    CVEX_ALIGNED(16) unsigned int rgba[4];
    cvex::store(rgba, vrgba);

    return (rgba[3] << 24) | (rgba[2] << 16) | (rgba[1] << 8) | rgba[0];
  }

  inline static void set_ftz() 
  {
  #ifdef __arm__
	static const unsigned int x = 0x04086060;
	static const unsigned int y = 0x03000000;
	int r;
	asm volatile (
		"fmrx	%0, fpscr			\n\t"	//r0 = FPSCR
		"and	%0, %0, %1		\n\t"	//r0 = r0 & 0x04086060
		"orr	%0, %0, %2		\n\t"	//r0 = r0 | 0x03000000
		"fmxr	fpscr, %0			\n\t"	//FPSCR = r0
		: "=r"(r)
		: "r"(x), "r"(y)
	);
  #endif 
  }
  
  inline void transpose4(const vfloat4 in_rows[4], vfloat4 out_rows[4])
  {
    CVEX_ALIGNED(16) float tmpdata[16];
    
    cvex::store(tmpdata + 0 , in_rows[0]);
    cvex::store(tmpdata + 4 , in_rows[1]);
    cvex::store(tmpdata + 8 , in_rows[2]);
    cvex::store(tmpdata + 12, in_rows[3]);

    out_rows[0] = vfloat4{tmpdata[0], tmpdata[4], tmpdata[8],  tmpdata[12]};
    out_rows[1] = vfloat4{tmpdata[1], tmpdata[5], tmpdata[9],  tmpdata[13]};
    out_rows[2] = vfloat4{tmpdata[2], tmpdata[6], tmpdata[10], tmpdata[14]};
    out_rows[3] = vfloat4{tmpdata[3], tmpdata[7], tmpdata[11], tmpdata[15]};
  }

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

  static inline void prefetch(const float* ptr) {  __builtin_prefetch(ptr); }
  static inline void prefetch(const int* ptr)   {  __builtin_prefetch(ptr); }


static inline cvex::vfloat4 operator+(cvex::vfloat4 a, cvex::vfloat4 b) { return vaddq_f32(a.data, b.data); }
static inline cvex::vfloat4 operator-(cvex::vfloat4 a, cvex::vfloat4 b) { return vsubq_f32(a.data, b.data); }
static inline cvex::vfloat4 operator*(cvex::vfloat4 a, cvex::vfloat4 b) { return vmulq_f32(a.data, b.data); }
static inline cvex::vfloat4 operator/(cvex::vfloat4 a, cvex::vfloat4 b) 
{ 
  CVEX_ALIGNED(16) float temp_a[4];
  CVEX_ALIGNED(16) float temp_b[4];
  cvex::store(temp_a, a);
  cvex::store(temp_b, b);
  return { temp_a[0] / temp_b[0], temp_a[1] / temp_b[1], temp_a[2] / temp_b[2], temp_a[3] / temp_b[3] };
}

static inline cvex::vfloat4 operator+(const cvex::vfloat4 a, const float b) { return vaddq_f32(a.data, vmovq_n_f32(b)); }
static inline cvex::vfloat4 operator-(const cvex::vfloat4 a, const float b) { return vsubq_f32(a.data, vmovq_n_f32(b)); }
static inline cvex::vfloat4 operator*(const cvex::vfloat4 a, const float b) { return vmulq_f32(a.data, vmovq_n_f32(b)); }
static inline cvex::vfloat4 operator/(const cvex::vfloat4 a, const float b) { return vmulq_f32(a.data, vmovq_n_f32(1.0f/b)); }

static inline cvex::vfloat4 operator+(const float b, const cvex::vfloat4 a) { return vaddq_f32(vmovq_n_f32(b), a.data); }
static inline cvex::vfloat4 operator-(const float b, const cvex::vfloat4 a) { return vsubq_f32(vmovq_n_f32(b), a.data); }
static inline cvex::vfloat4 operator*(const float b, const cvex::vfloat4 a) { return vmulq_f32(vmovq_n_f32(b), a.data); }
static inline cvex::vfloat4 operator/(const float b, const cvex::vfloat4 a) 
{ 
  CVEX_ALIGNED(16) float temp_a[4];
  cvex::store(temp_a, a);
  return { b / temp_a[0], b / temp_a[1], b / temp_a[2], b / temp_a[3] }; 
}

static inline cvex::vint4 operator+(const cvex::vint4 a, const cvex::vint4 b) { return vaddq_s32(a.data, b.data); }
static inline cvex::vint4 operator-(const cvex::vint4 a, const cvex::vint4 b) { return vsubq_s32(a.data, b.data);}
static inline cvex::vint4 operator*(const cvex::vint4 a, const cvex::vint4 b) { return vmulq_s32(a.data, b.data); }
static inline cvex::vint4 operator/(const cvex::vint4 a, const cvex::vint4 b) 
{ 
  CVEX_ALIGNED(16) int temp_a[4];
  CVEX_ALIGNED(16) int temp_b[4];
  cvex::store(temp_a, a);
  cvex::store(temp_b, b);
  return { temp_a[0] / temp_b[0], temp_a[1] / temp_b[1], temp_a[2] / temp_b[2], temp_a[3] / temp_b[3] };
} 

static inline cvex::vint4 operator+(const cvex::vint4 a, const int b) { return vaddq_s32(a, vmovq_n_s32(b)); }
static inline cvex::vint4 operator-(const cvex::vint4 a, const int b) { return vsubq_s32(a, vmovq_n_s32(b)); }
static inline cvex::vint4 operator*(const cvex::vint4 a, const int b) { return vmulq_s32(a, vmovq_n_s32(b)); }
static inline cvex::vint4 operator/(const cvex::vint4 a, const int b)
{
  CVEX_ALIGNED(16) int temp_a[4];
  cvex::store(temp_a, a);
  return { temp_a[0] / b, temp_a[1] / b, temp_a[2] / b, temp_a[3] / b };
}

static inline cvex::vint4 operator+(const int a, const cvex::vint4 b) { return vaddq_s32(vmovq_n_s32(a), b); }
static inline cvex::vint4 operator-(const int a, const cvex::vint4 b) { return vsubq_s32(vmovq_n_s32(a), b); }
static inline cvex::vint4 operator*(const int a, const cvex::vint4 b) { return vmulq_s32(vmovq_n_s32(a), b); }
static inline cvex::vint4 operator/(const int a, const cvex::vint4 b)
{
  CVEX_ALIGNED(16) int temp_b[4];
  cvex::store(temp_b, b);
  return { a / temp_b[0], a / temp_b[1], a / temp_b[2], a / temp_b[3] };
}

static inline cvex::vuint4 operator+(const cvex::vuint4 a, const cvex::vuint4 b) { return vaddq_u32(a.data, b.data); }
static inline cvex::vuint4 operator-(const cvex::vuint4 a, const cvex::vuint4 b) { return vsubq_u32(a.data, b.data);}
static inline cvex::vuint4 operator*(const cvex::vuint4 a, const cvex::vuint4 b) { return vmulq_u32(a.data, b.data); }
static inline cvex::vuint4 operator/(const cvex::vuint4 a, const cvex::vuint4 b)
{
  CVEX_ALIGNED(16) unsigned int temp_a[4];
  CVEX_ALIGNED(16) unsigned int temp_b[4];
  cvex::store(temp_a, a);
  cvex::store(temp_b, b);
  return { temp_a[0] / temp_b[0], temp_a[1] / temp_b[1], temp_a[2] / temp_b[2], temp_a[3] / temp_b[3] };
}

static inline cvex::vuint4 operator+(const cvex::vuint4 a, const unsigned int b) { return vaddq_u32(a, vmovq_n_u32(b)); }
static inline cvex::vuint4 operator-(const cvex::vuint4 a, const unsigned int b) { return vsubq_u32(a, vmovq_n_u32(b)); }
static inline cvex::vuint4 operator*(const cvex::vuint4 a, const unsigned int b) { return vmulq_u32(a, vmovq_n_u32(b)); }
static inline cvex::vuint4 operator/(const cvex::vuint4 a, const unsigned int b)
{
  CVEX_ALIGNED(16) unsigned int temp_a[4];
  cvex::store(temp_a, a);
  return { temp_a[0] / b, temp_a[1] / b, temp_a[2] / b, temp_a[3] / b };
}
static inline cvex::vuint4 operator+(const unsigned int a, const cvex::vuint4 b) { return vaddq_u32(vmovq_n_u32(a), b); }
static inline cvex::vuint4 operator-(const unsigned int a, const cvex::vuint4 b) { return vsubq_u32(vmovq_n_u32(a), b); }
static inline cvex::vuint4 operator*(const unsigned int a, const cvex::vuint4 b) { return vmulq_u32(vmovq_n_u32(a), b); }
static inline cvex::vuint4 operator/(const unsigned int a, const cvex::vuint4 b)
{
  CVEX_ALIGNED(16) unsigned int temp_b[4];
  cvex::store(temp_b, b);
  return { a / temp_b[0], a / temp_b[1], a / temp_b[2], a / temp_b[3] };
}

#ifdef NDEBUG

static inline cvex::vint4  operator<<(const cvex::vint4 a,  const int val) { return vshlq_n_s32(a, val); }
static inline cvex::vint4  operator>>(const cvex::vint4 a,  const int val) { return vshrq_n_s32(a, val); }
static inline cvex::vuint4 operator<<(const cvex::vuint4 a, const int val) { return vshlq_n_u32(a, val); }
static inline cvex::vuint4 operator>>(const cvex::vuint4 a, const int val) { return vshrq_n_u32(a, val); }

#else

static inline cvex::vint4  operator<<(const cvex::vint4 a,  const int val) 
{ 
  CVEX_ALIGNED(16) int t[4];
  cvex::store(t, a);
  return {t[0] << val, t[1] << val, t[2] << val, t[3] << val}; 
}
static inline cvex::vint4  operator>>(const cvex::vint4 a,  const int val)
{
  CVEX_ALIGNED(16) int t[4];
  cvex::store(t, a);
  return {t[0] >> val, t[1] >> val, t[2] >> val, t[3] >> val}; 
}

static inline cvex::vuint4 operator<<(const cvex::vuint4 a, const int val) 
{
  CVEX_ALIGNED(16) unsigned int t[4];
  cvex::store(t, a);
  return {t[0] << val, t[1] << val, t[2] << val, t[3] << val}; 
}

static inline cvex::vuint4 operator>>(const cvex::vuint4 a, const int val) 
{
  CVEX_ALIGNED(16) unsigned int t[4];
  cvex::store(t, a);
  return {t[0] >> val, t[1] >> val, t[2] >> val, t[3] >> val}; 
}

#endif

static inline cvex::vint4 operator|(const cvex::vint4 a, const cvex::vint4 b) { return vorrq_s32(a,b); }
static inline cvex::vint4 operator&(const cvex::vint4 a, const cvex::vint4 b) { return vandq_s32(a,b); }
static inline cvex::vint4 operator~(const cvex::vint4 a)                      { return vmvnq_s32(a);   }

static inline cvex::vuint4 operator|(const cvex::vuint4 a, const cvex::vuint4 b) { return vorrq_u32(a,b); }
static inline cvex::vuint4 operator&(const cvex::vuint4 a, const cvex::vuint4 b) { return vandq_u32(a,b); }

static inline cvex::vuint4 operator|(const cvex::vint4 a, const cvex::vuint4 b) { return vorrq_u32((uint32x4_t)a.data, b); }
static inline cvex::vuint4 operator&(const cvex::vint4 a, const cvex::vuint4 b) { return vandq_u32((uint32x4_t)a.data, b); }

static inline cvex::vuint4 operator|(const cvex::vuint4 a, const int b) { return vorrq_u32(a, vld1q_dup_u32((unsigned int*)&b)); }
static inline cvex::vuint4 operator&(const cvex::vuint4 a, const int b) { return vandq_u32(a, vld1q_dup_u32((unsigned int*)&b)); }
static inline cvex::vuint4 operator~(const cvex::vuint4 a)              { return vmvnq_u32(a);   }

static inline cvex::vint4 operator> (const cvex::vfloat4 a, const cvex::vfloat4 b) { return (int32x4_t)vcgtq_f32(a, b); }
static inline cvex::vint4 operator< (const cvex::vfloat4 a, const cvex::vfloat4 b) { return (int32x4_t)vcltq_f32(a, b); }
static inline cvex::vint4 operator>=(const cvex::vfloat4 a, const cvex::vfloat4 b) { return (int32x4_t)vcgeq_f32(a, b); }
static inline cvex::vint4 operator<=(const cvex::vfloat4 a, const cvex::vfloat4 b) { return (int32x4_t)vcleq_f32(a, b); }
static inline cvex::vint4 operator==(const cvex::vfloat4 a, const cvex::vfloat4 b) { return (int32x4_t)vceqq_f32(a, b); }
static inline cvex::vint4 operator!=(const cvex::vfloat4 a, const cvex::vfloat4 b) { return (int32x4_t)vmvnq_u32(vceqq_f32(a, b)); }

};

#endif //TEST_GL_TOP_VFLOAT4_GCC_H
