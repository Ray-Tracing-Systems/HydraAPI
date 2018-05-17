#include "HR_HDRImage.h"

#include "ssemath.h"


#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>

#include <string.h> // memcpy in linux
#include <math.h>   // sqrt, exp, fmax, fmin

#ifndef _MM_DENORMALS_ZERO_MASK
  #define _MM_DENORMALS_ZERO_MASK	0x0040
#endif

#ifndef _MM_DENORMALS_ZERO_ON
  #define _MM_DENORMALS_ZERO_ON		0x0040
#endif

#ifndef _MM_SET_DENORMALS_ZERO_MODE
#define _MM_SET_DENORMALS_ZERO_MODE(mode) _mm_setcsr((_mm_getcsr() & ~_MM_DENORMALS_ZERO_MASK) | (mode))
#endif

namespace HydraRender
{

  HDRImage4f::HDRImage4f() : m_width(0), m_height(0)
  {

  }

  HDRImage4f::HDRImage4f(int w, int h, const float* data) : m_width(w), m_height(h)
  {
    const int channels = 4;
    m_data.resize(m_width*m_height * 4);
    if (data != nullptr)
      memcpy(&m_data[0], data, m_width*m_height * 4 * sizeof(float));
  }

  void HDRImage4f::resize(int w, int h)
  {
    m_width = w;
    m_height = h;
    m_data.resize(m_width*m_height * 4);
  }


  HDRImage4f::~HDRImage4f()
  {

  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void HDRImage4f::loadFromImage4f(const std::string& a_fileName)
  {
    unsigned int wh[2];

    std::ifstream fin(a_fileName.c_str(), std::ios::binary);

    if (!fin.is_open())
    {
      std::cerr << "LoadImageFromImagef4: can't open file " << a_fileName.c_str() << std::endl;
      m_width = 0;
      m_height = 0;
      return;
    }

    fin.read((char*)wh, sizeof(int) * 2);

    m_width = wh[0];
    m_height = wh[1];

    m_data.resize(m_width*m_height * 4);

    fin.read((char*)&m_data[0], wh[0] * wh[1] * 4 * sizeof(float));
    fin.close();
  }


  void HDRImage4f::saveToImage4f(const std::string& a_fileName)
  {
    int wh[2] = { m_width, m_height };

    std::ofstream fout(a_fileName.c_str(), std::ios::binary);
    fout.write((char*)wh, sizeof(int) * 2);
    fout.write((char*)&m_data[0], wh[0] * wh[1] * 4 * sizeof(float));
    fout.close();
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  inline static float clamp(float u, float a, float b) { return fminf(fmaxf(a, u), b); }

  struct float4
  {
    float4() : x(0), y(0), z(0), w(0) {}
    float4(float a, float b, float c, float d) : x(a), y(b), z(c), w(d) {}

    float x, y, z, w;
  };

  static inline float4 read_imagef(const float4* f_data, int w, int h, float a_texCoordX, float a_texCoordY) // texture2d_t
  {
    const float fw = (float)(w);
    const float fh = (float)(h);

    const float ffx = clamp(a_texCoordX*fw - 0.5f, 0.0f, fw - 1.0f);
    const float ffy = clamp(a_texCoordY*fh - 0.5f, 0.0f, fh - 1.0f);

    const int px = (int)(ffx);
    const int py = (int)(ffy);

    // Calculate the weights for each pixel
    //
    float fx = ffx - (float)px;
    float fy = ffy - (float)py;
    float fx1 = 1.0f - fx;
    float fy1 = 1.0f - fy;

    float w1 = fx1 * fy1;
    float w2 = fx  * fy1;
    float w3 = fx1 * fy;
    float w4 = fx  * fy;

    float4 f1, f2, f3, f4;

    // fetch data
    //
    const float4* p0 = f_data + (py*w) + px;

    f1 = p0[0 + 0 * w];
    f2 = (px < w - 1) ? p0[1 + 0 * w] : f1;
    f3 = (py < h - 1) ? p0[0 + 1 * w] : f1;
    f4 = ((px < w - 1) && (py < h - 1)) ? p0[1 + 1 * w] : f1;


    // Calculate the weighted sum of pixels (for each color channel)
    //
    float outr = f1.x * w1 + f2.x * w2 + f3.x * w3 + f4.x * w4;
    float outg = f1.y * w1 + f2.y * w2 + f3.y * w3 + f4.y * w4;
    float outb = f1.z * w1 + f2.z * w2 + f3.z * w3 + f4.z * w4;
    float outa = f1.w * w1 + f2.w * w2 + f3.w * w3 + f4.w * w4;

    return float4(outr, outg, outb, outa);
  }


  void HDRImage4f::sample(float x, float y, float* out_vec4f) const
  {
    float4 res = read_imagef((const float4*)&m_data[0], m_width, m_height, x, y);

    out_vec4f[0] = res.x;
    out_vec4f[1] = res.y;
    out_vec4f[2] = res.z;
    out_vec4f[3] = res.w;
  }


  inline __m128 read_imagef_sse(const float* data, int w, int h, float a_texCoordX, float a_texCoordY)
  {
    const float fw = (float)(w);
    const float fh = (float)(h);

    const float ffx = clamp(a_texCoordX*fw - 0.5f, 0.0f, fw - 1.0f);
    const float ffy = clamp(a_texCoordY*fh - 0.5f, 0.0f, fh - 1.0f);

    const int px = (int)(ffx);
    const int py = (int)(ffy);

    const int stride = w;

    const int px1 = (px < w - 1) ? px + 1 : px;
    const int py1 = (py < h - 1) ? py + 1 : py;

    int offset0 = (px + py*stride) * 4;
    int offset1 = (px1 + py*stride) * 4;
    int offset2 = (px + py1*stride) * 4;
    int offset3 = (px1 + py1*stride) * 4;

    float  alpha = ffx - (float)px;
    float  beta = ffy - (float)py;
    float  gamma = 1.0f - alpha;
    float  delta = 1.0f - beta;

    __m128 alphaV = _mm_set_ps(alpha, alpha, alpha, alpha);
    __m128 betaV = _mm_set_ps(beta, beta, beta, beta);
    __m128 gammaV = _mm_set_ps(gamma, gamma, gamma, gamma);
    __m128 deltaV = _mm_set_ps(delta, delta, delta, delta);

    __m128 samplesA = _mm_mul_ps(_mm_load_ps(data + offset0), gammaV);
    __m128 samplesB = _mm_mul_ps(_mm_load_ps(data + offset1), alphaV);
    __m128 samplesC = _mm_mul_ps(_mm_load_ps(data + offset2), gammaV);
    __m128 samplesD = _mm_mul_ps(_mm_load_ps(data + offset3), alphaV);

    __m128 resultX0 = _mm_add_ps(samplesA, samplesB);
    __m128 resultX1 = _mm_add_ps(samplesC, samplesD);
    __m128 resultY0 = _mm_mul_ps(resultX0, deltaV);
    __m128 resultY1 = _mm_mul_ps(resultX1, betaV);

    return _mm_add_ps(resultY0, resultY1);
  }


  __m128 HDRImage4f::sample(float x, float y) const
  {
    return read_imagef_sse(&m_data[0], m_width, m_height, x, y);
  }


  void HDRImage4f::resampleTo(HDRImage4f& a_outImage)
  {
    int w = a_outImage.width();
    int h = a_outImage.height();

    if (w == width() && h == height()) // trivial case
    {
      memcpy(a_outImage.data(), data(), m_width*m_height * sizeof(float) * 4);
      return;
    }

    HDRImage4f* pInputImage = this;
    HDRImage4f tempImage2;

    if (w * 2 < m_width || h * 2 < m_height) // piramid bilinear filterig 
    {
      tempImage2.resize(w * 2, h * 2);
      this->resampleTo(tempImage2);
      pInputImage = &tempImage2;
    }
    else
    {
      pInputImage = this;
    }

    // now resample from pInputImage which can be this or tempImage

    const float fwInv = 1.0f / float(w);
    const float fhInv = 1.0f / float(h);
    float* dataNew = a_outImage.data();

    if (w * 2 == pInputImage->width() && h * 2 == pInputImage->height())    // special case, simple and fast
    {
      const __m128 oneQuater = _mm_set_ps(0.25f, 0.25f, 0.25f, 0.25f);
      float* dataOld = pInputImage->data();

      #pragma omp parallel for
      for (int j = 0; j < h; ++j)
      {
        const int offsetY = j*w * 4;

        const int offsetY0 = (j * 2 * pInputImage->width() + 0) * 4;
        const int offsetY1 = (j * 2 * pInputImage->width() + 1) * 4;

        for (int i = 0; i < w; ++i)
        {
          const int offsetX = i * 4;

          const int offsetX0 = (i * 2 + 0) * 4;
          const int offsetX1 = (i * 2 + 1) * 4;

          const __m128 x0 = _mm_load_ps(dataOld + offsetY0 + offsetX0);
          const __m128 x1 = _mm_load_ps(dataOld + offsetY0 + offsetX1);
          const __m128 x2 = _mm_load_ps(dataOld + offsetY1 + offsetX0);
          const __m128 x3 = _mm_load_ps(dataOld + offsetY1 + offsetX1);

          const  __m128 filtered = _mm_mul_ps(oneQuater, _mm_add_ps(_mm_add_ps(x0, x1), _mm_add_ps(x2, x3)));
          _mm_store_ps(dataNew + offsetY + offsetX, filtered);

        }

      }

    }
    else // general case
    {
      #pragma omp parallel for
      for (int j = 0; j < h; ++j)
      {
        float texCoordY = (float(j) + 0.5f)*fhInv;
        int   offsetY = j*w * 4;

        for (int i = 0; i < w; ++i)
        {
          float texCoordX = (float(i) + 0.5f)*fwInv;
          __m128 filtered = pInputImage->sample(texCoordX, texCoordY);
          _mm_store_ps(dataNew + offsetY + i * 4, filtered);
        }
      }

    }

  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  inline __m128 sse_dot3(__m128 a, __m128 b) // please don't use _mm_dp_ps! It is from SSE 4.1 !!! Don't work on AMD Phenom
  {
    const __m128 mult  = _mm_mul_ps(a, b);
    const __m128 shuf1 = _mm_shuffle_ps(mult, mult, _MM_SHUFFLE(0, 3, 2, 1));
    const __m128 shuf2 = _mm_shuffle_ps(mult, mult, _MM_SHUFFLE(1, 0, 3, 2));
    const __m128 shuf3 = _mm_shuffle_ps(mult, mult, _MM_SHUFFLE(2, 1, 0, 3));

    return _mm_add_ss(_mm_add_ss(mult, shuf1), shuf2);
  }

  void HDRImage4f::medianFilterInPlace(float a_thresholdValue, float avgB)
  {
    int w = width();
    int h = height();

    const float ts2       = a_thresholdValue*a_thresholdValue;
    const __m128 invEight = _mm_set_ps(1.0f / 8.0f, 1.0f / 8.0f, 1.0f / 8.0f, 1.0f / 8.0f);
    const __m128 eps      = _mm_set_ps(1e-5f, 1e-5f, 1e-5f, 1e-5f);

    const __m128 mthreshold    = _mm_set_ps(1000.0f, ts2, ts2, ts2);
    const __m128 mthresholdMin = _mm_set_ps(1000.0f, ts2*0.01f, ts2*0.01f, ts2*0.01f);
    const __m128 mthresholdMax = _mm_set_ps(1000.0f, ts2*4.0f, ts2*4.0f, ts2*4.0f);

    const __m128 mAvgB = _mm_set_ps(avgB, avgB, avgB, avgB);

    float* pData = data();

    for (int j = 0; j < h; ++j)
    {
      int offsetY0 = (j - 1)*w * 4;
      int offsetY1 = (j + 0)*w * 4;
      int offsetY2 = (j + 1)*w * 4;

      if (j - 1 < 0)
        offsetY0 = offsetY1;

      if (j + 1 >= h)
        offsetY2 = offsetY1;

      for (int i = 0; i < w; ++i)
      {
        int offsetX0 = (i - 1) * 4;
        int offsetX1 = (i + 0) * 4;
        int offsetX2 = (i + 1) * 4;

        if (i - 1 < 0)
          offsetX0 = offsetX1;

        if (i + 1 >= w)
          offsetX2 = offsetX1;

        const __m128 xm = _mm_load_ps(pData + offsetY1 + offsetX1);

        const __m128 x0 = _mm_load_ps(pData + offsetY1 + offsetX0);
        const __m128 x1 = _mm_load_ps(pData + offsetY1 + offsetX2);
        const __m128 x2 = _mm_load_ps(pData + offsetY0 + offsetX1);
        const __m128 x3 = _mm_load_ps(pData + offsetY2 + offsetX1);

        const __m128 x4 = _mm_load_ps(pData + offsetY0 + offsetX0);
        const __m128 x5 = _mm_load_ps(pData + offsetY0 + offsetX2);
        const __m128 x6 = _mm_load_ps(pData + offsetY2 + offsetX0);
        const __m128 x7 = _mm_load_ps(pData + offsetY2 + offsetX2);

        const __m128 sumColorOthers = _mm_add_ps(_mm_add_ps(_mm_add_ps(x0, x1), _mm_add_ps(x2, x3)), _mm_add_ps(_mm_add_ps(x4, x5), _mm_add_ps(x6, x7)));
        const __m128 avgColorOthers = _mm_mul_ps(invEight, sumColorOthers);

        __m128 threshold = mthreshold;

        if (avgB > 1e-6f) // change threshold value, tune it for MLT
        {
          const __m128 avgColorLenSquare = sse_dot3(avgColorOthers, avgColorOthers); // 
          if (_mm_movemask_ps(_mm_cmpgt_ss(avgColorLenSquare, eps)))                 // if color leng square > epsilon
          {
            const __m128 avgColorSquare = _mm_mul_ps(avgColorOthers, avgColorOthers);
            threshold = _mm_mul_ps(threshold, _mm_div_ps(avgColorSquare, mAvgB));
            threshold = _mm_min_ps(mthresholdMax, _mm_max_ps(mthresholdMin, threshold));
          }
        }
        else
          threshold = mthresholdMin;

        const __m128 diff0 = _mm_sub_ps(xm, x0);
        const __m128 diff1 = _mm_sub_ps(xm, x1);
        const __m128 diff2 = _mm_sub_ps(xm, x2);
        const __m128 diff3 = _mm_sub_ps(xm, x3);

        const __m128 diff0sq = _mm_mul_ps(diff0, diff0); // add _mm_and_ps with mask to discard w component ... 
        const __m128 diff1sq = _mm_mul_ps(diff1, diff1);
        const __m128 diff2sq = _mm_mul_ps(diff2, diff2);
        const __m128 diff3sq = _mm_mul_ps(diff3, diff3);

        int numFailed = 0;

        if (_mm_movemask_ps(_mm_cmpgt_ps(diff0sq, threshold)))
          numFailed++;

        if (_mm_movemask_ps(_mm_cmpgt_ps(diff1sq, threshold)))
          numFailed++;

        if (_mm_movemask_ps(_mm_cmpgt_ps(diff2sq, threshold)))
          numFailed++;

        if (_mm_movemask_ps(_mm_cmpgt_ps(diff3sq, threshold)))
          numFailed++;

        if (numFailed >= 3)
        {
          const __m128 x4 = _mm_load_ps(pData + offsetY0 + offsetX0);
          const __m128 x5 = _mm_load_ps(pData + offsetY0 + offsetX2);
          const __m128 x6 = _mm_load_ps(pData + offsetY2 + offsetX0);
          const __m128 x7 = _mm_load_ps(pData + offsetY2 + offsetX2);

          const __m128 xi[9] = { xm, x0, x1, x2, x3, x4, x5, x6, x7 };

          float red[9];
          float green[9];
          float blue[9];

          for (int i = 0; i < 9; i++)
          {
            const __m128 xc = xi[i];

            _mm_store_ss(red + i, xc);
            _mm_store_ss(green + i, _mm_shuffle_ps(xc, xc, _MM_SHUFFLE(1, 1, 1, 1)));
            _mm_store_ss(blue + i, _mm_shuffle_ps(xc, xc, _MM_SHUFFLE(2, 2, 2, 2)));
          }

          std::sort(red, red + 9);
          std::sort(green, green + 9);
          std::sort(blue, blue + 9);

          _mm_store_ps(pData + offsetY1 + offsetX1, _mm_set_ps(0.0f, blue[4], green[4], red[4]));
        }

      }

    }


  }


  std::vector<float> createGaussKernelWeights1D_HDRImage(int size, float a_sigma)
  {
    std::vector<float> gKernel;
    gKernel.resize(size);

    // set standard deviation to 1.0
    //
    float sigma = a_sigma;
    float s = 2.0f * sigma * sigma;

    // sum is for normalization
    float sum = 0.0;
    int halfSize = size / 2;

    for (int x = -halfSize; x <= halfSize; x++)
    {
      float r = sqrtf((float)(x*x));
      int index = x + halfSize;
      gKernel[index] = (exp(-(r) / s)) / (3.141592654f * s);
      sum += gKernel[index];
    }

    // normalize the Kernel
    for (int i = 0; i < size; ++i)
      gKernel[i] /= sum;

    return gKernel;
  }

  void HDRImage4f::gaussBlur(const int BLUR_RADIUS2, float a_sigma)
  {
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

    size_t imSize = size_t(m_width)*size_t(m_height) * sizeof(__m128);

    float sigma = a_sigma; // = 0.85f + 0.05f*float(BLUR_RADIUS2);
    std::vector<float> kernel = createGaussKernelWeights1D_HDRImage(BLUR_RADIUS2 * 2 + 1, sigma);

    __m128 one = _mm_set_ps(1.0f, 1.0f, 1.0f, 1.0f);

    // init weights
    //
    __m128 weights[64];

    for (int i = 0; i < 64; i++)
      weights[i] = _mm_set_ps(0, 0, 0, 0);

    for (int i = 0; i < kernel.size(); i++)
    {
      float w = kernel[i] * 1.0f;
      weights[i] = _mm_set_ps(w, w, w, w);
    }

    const float* m_dataInBrightPixels = this->data();
    float*       m_dataOut = this->data();
    HDRImage4f temp(this->width(), this->height());

    // horisontal blur pass
    //

    float* tmpData = temp.data();

    memset(tmpData, 0, imSize); // THIS IS IMPORTANT BECAUSE BOUNDARIES ARE NOT SET BY THE FINAL PASS

    for (int y = 0; y < m_height; y++)
    {
      int offset = y*m_width * 4;

      for (int x = BLUR_RADIUS2; x < m_width - BLUR_RADIUS2; x++)
      {
        __m128 summ = _mm_mul_ps(weights[BLUR_RADIUS2], _mm_load_ps(m_dataInBrightPixels + offset + (x + 0) * 4));

        for (int wid = 1; wid < BLUR_RADIUS2; wid++)
        {
          __m128 p0 = _mm_mul_ps(weights[wid + BLUR_RADIUS2], _mm_load_ps(m_dataInBrightPixels + offset + (x - wid) * 4));
          __m128 p1 = _mm_mul_ps(weights[wid + BLUR_RADIUS2], _mm_load_ps(m_dataInBrightPixels + offset + (x + wid) * 4));
          summ = _mm_add_ps(summ, _mm_add_ps(p0, p1));
        }

        _mm_store_ps(tmpData + offset + x * 4, summ);
      }

    }

    memset(m_dataOut, 0, imSize); // THIS IS IMPORTANT BECAUSE BOUNDARIES ARE NOT SET BY THE FINAL PASS

    // vertical blur pass and blend pass
    //
    for (int x = 0; x < m_width; x++)
    {
      for (int y = BLUR_RADIUS2; y < m_height - BLUR_RADIUS2; y++)
      {
        int offset2 = (y * 4)*m_width + x * 4;

        __m128 summ = _mm_mul_ps(weights[BLUR_RADIUS2], _mm_load_ps(tmpData + y*m_width * 4 + x * 4));

        for (int wid = 1; wid < BLUR_RADIUS2; wid++)
        {
          __m128 p0 = _mm_mul_ps(weights[wid + BLUR_RADIUS2], _mm_load_ps(tmpData + 4 * (y - wid)*m_width + x * 4));
          __m128 p1 = _mm_mul_ps(weights[wid + BLUR_RADIUS2], _mm_load_ps(tmpData + 4 * (y + wid)*m_width + x * 4));
          summ = _mm_add_ps(summ, _mm_add_ps(p0, p1));
        }

        _mm_store_ps(m_dataOut + y*m_width * 4 + x * 4, summ);
      }
    }


  }



  unsigned int HR_HDRImage4f_RealColorToUint32(float a_r, float a_g, float a_b, float a_alpha)
  {
    float  r = clamp(a_r*255.0f, 0.0f, 255.0f);
    float  g = clamp(a_g*255.0f, 0.0f, 255.0f);
    float  b = clamp(a_b*255.0f, 0.0f, 255.0f);
    float  a = clamp(a_alpha*255.0f, 0.0f, 255.0f);

    unsigned char red = (unsigned char)r;
    unsigned char green = (unsigned char)g;
    unsigned char blue = (unsigned char)b;
    unsigned char alpha = (unsigned char)a;

    return red | (green << 8) | (blue << 16) | (alpha << 24);
  }


  static void convertFloat4ToLDR2(const float* dataPtr, std::vector<unsigned int>& dataLDR, float a_gamma)
  {
    const float power = 1.0f / a_gamma;

    #pragma omp parallel for
    for (int i = 0; i < int(dataLDR.size()); i++)
    {
      float r = dataPtr[i * 4 + 0];
      float g = dataPtr[i * 4 + 1];
      float b = dataPtr[i * 4 + 2];
      float a = dataPtr[i * 4 + 3];

      r = pow(r, power);
      g = pow(g, power);
      b = pow(b, power);
      a = pow(a, power);

      dataLDR[i] = HR_HDRImage4f_RealColorToUint32(r, g, b, a);
    }
  }


  void HDRImage4f::convertToLDR(float a_gamma, std::vector<unsigned int>& outData)
  {
    if (outData.size() != m_data.size() / 4)
      outData.resize(m_data.size() / 4);

    convertFloat4ToLDR2(data(), outData, a_gamma);
  }


  static inline float4 unpackColor(unsigned int rgba)
  {
    const float mulInv = (1.0f / 255.0f);

    float4 res;
    res.x = ( rgba & 0x000000FF)       *mulInv;
    res.y = ((rgba & 0x0000FF00) >> 8) *mulInv;
    res.z = ((rgba & 0x00FF0000) >> 16)*mulInv;
    res.w = ((rgba & 0xFF000000) >> 24)*mulInv;
    return res;
  }

  static inline __m128 unpackColor(unsigned int rgba, const __m128& a_mulInv)
  {
    const __m128i intData = _mm_set_epi32((rgba & 0xFF000000) >> 24,
                                          (rgba & 0x00FF0000) >> 16, 
                                          (rgba & 0x0000FF00) >> 8, 
                                           rgba & 0x000000FF
                                         );
    return _mm_mul_ps(_mm_cvtepi32_ps(intData), a_mulInv);
  }

  void HDRImage4f::convertFromLDR(float a_gamma, const unsigned int* dataLDR, int a_size) //#TODO: accelerate with SSE !!!
  {
    const __m128 powps  = _mm_set_ps(a_gamma, a_gamma, a_gamma, a_gamma);
    const __m128 mulInv = _mm_set_ps((1.0f / 255.0f), (1.0f / 255.0f), (1.0f / 255.0f), (1.0f / 255.0f));

    float* out = data();

    #pragma omp parallel for
    for (int i = 0; i < a_size; i++)
    {
      const __m128 colorps  = unpackColor(dataLDR[i], mulInv);
      const __m128 colorps2 = HydraSSE::powf4(colorps, powps);
      _mm_store_ps(out + i*4, colorps2);
    }

  }


};

