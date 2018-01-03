#pragma once

#include <vector>
#include <string>

#if _MSC_VER >= 1400
#include <intrin.h>
#else
#include <xmmintrin.h>
#endif

#include <cstring>

#include "alloc16.h"

namespace HydraRender
{

  struct HDRImage4f
  {
  public:

    HDRImage4f();
    HDRImage4f(int w, int h, const float* data = nullptr);
    virtual ~HDRImage4f();

    HDRImage4f(const HDRImage4f& rhs) = default;
    HDRImage4f(HDRImage4f&& lhs)      = default;

    HDRImage4f& operator=(HDRImage4f&& rhs)      = default;
    HDRImage4f& operator=(const HDRImage4f& rhs) = default;

    void resize(int w, int h);

    inline int width()    const { return m_width; }
    inline int height()   const { return m_height; }
    inline int channels() const { return 4; }

    inline const float* data() const
    {
      if (m_data.size() == 0) return nullptr;
      else                    return &m_data[0];
    }

    inline float* data()
    {
      if (m_data.size() == 0) return nullptr;
      else                    return &m_data[0];
    }

    void loadFromImage4f(const std::string& a_fileName);
    void saveToImage4f(const std::string& a_fileName);

    void   sample(float x, float y, float* out_vec4f) const;
    __m128 sample(float x, float y) const;

    void resampleTo(HDRImage4f& a_outImage);

    void convertToLDR(float a_gamma, std::vector<unsigned int>& outData);

    void medianFilterInPlace(float a_thresholdValue, float avgB);
    void gaussBlur(const int BLUR_RADIUS2, float a_sigma);

  private:

    int m_width;
    int m_height;
    std::vector<float, aligned16<float> > m_data;

  };


  struct LDRImage1i
  {
  public:

    LDRImage1i() {}
    LDRImage1i(int w, int h, const int* data = nullptr)
    {
      resize(w, h);
      if(data != nullptr)
        memcpy(&m_data[0], data, w*h * sizeof(int));
    }
    virtual ~LDRImage1i() {}

    LDRImage1i(const LDRImage1i& rhs) = default;
    LDRImage1i(LDRImage1i&& lhs)      = default;

    LDRImage1i& operator=(LDRImage1i&& rhs)      = default;
    LDRImage1i& operator=(const LDRImage1i& rhs) = default;

    void resize(int w, int h) { m_data.resize(w*h); m_width = w; m_height = h; }

    inline int width()    const { return m_width; }
    inline int height()   const { return m_height; }
    inline int channels() const { return 4; }

    inline const int* data() const
    {
      if (m_data.size() == 0) return nullptr;
      else                    return &m_data[0];
    }

    inline int* data()
    {
      if (m_data.size() == 0) return nullptr;
      else                    return &m_data[0];
    }

    inline       std::vector<int>& dataVector()       { return m_data; }
    inline const std::vector<int>& dataVector() const { return m_data; }

  private:

    int m_width;
    int m_height;
    std::vector<int> m_data;

  };

};



