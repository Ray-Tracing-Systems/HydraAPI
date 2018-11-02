#pragma once

#include <vector>
#include <string>

#include <cstring>
#include "alloc16.h"

#include <tuple>

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
      if (m_data.empty()) return nullptr;
      else                return m_data.data();
    }

    inline float* data()
    {
      if (m_data.empty()) return nullptr;
      else                return m_data.data();
    }

    void loadFromImage4f(const std::string& a_fileName);
    void saveToImage4f(const std::string& a_fileName);

    void sample(float x, float y, float* out_vec4f) const;
    void resampleTo(HDRImage4f& a_outImage);

    void convertToLDR(float a_gamma, std::vector<unsigned int>& outData);
    void convertFromLDR(float a_gamma, const unsigned int* inData, int a_size);

    void medianFilter(float a_thresholdValue, HDRImage4f &a_outImage);
    void gaussBlur(int BLUR_RADIUS2, float a_sigma);

  private:

    int m_width;
    int m_height;
    std::vector<float, aligned16<float> > m_data;

  };

  struct LDRImage1i
  {
  public:

    LDRImage1i()                                        : m_width(0), m_height(0) {}
    LDRImage1i(int w, int h, const int* data = nullptr) : m_width(0), m_height(0)
    {
      resize(w, h);
      if(data != nullptr)
        memcpy(&m_data[0], data, w*h * sizeof(int));
    }
    virtual ~LDRImage1i() = default;

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
      if (m_data.empty()) return nullptr;
      else                return m_data.data();
    }

    inline int* data()
    {
      if (m_data.empty()) return nullptr;
      else                return m_data.data();
    }

    inline       std::vector<int>& dataVector()       { return m_data; }
    inline const std::vector<int>& dataVector() const { return m_data; }

  private:

    int m_width;
    int m_height;
    std::vector<int> m_data;

  };

  std::tuple<double, double, double> ColorSummImage4f(const float* a_image4f, int a_width, int a_height);
};

/**
\brief abstract interface to image Load/Save operations
*/
struct IHRImageTool
{
  IHRImageTool()          = default;
  virtual ~IHRImageTool() = default;

  /**
  \brief load image from file to a_data; load both LDR and HDR images
  \param a_fileName -- input file name
  \param w          -- out image width
  \param h          -- out image height
  \param bpp        -- out image bytes per pixel (4 for LDR or 16 for HDR images)
  \param a_data     -- we pass container to allow its resize by the loader impl; if bpp = 16 then loader must increase size 4 times.
  */
  virtual bool LoadImageFromFile(const wchar_t* a_fileName, 
                                 int& w, int& h, int& bpp, std::vector<int>& a_data) = 0;

  /**
  \brief load image and force convert it to HDR (linear color space).
  \param a_fileName -- input file name
  \param w          -- out image width
  \param h          -- out image height
  \param a_data     -- we pass container to allow its resize by the loader impl; if bpp = 16 then loader must increase size 4 times.
  */
  virtual bool LoadImageFromFile(const wchar_t* a_fileName, 
                                 int& w, int& h, std::vector<float>& a_data) = 0;

  virtual void SaveHDRImageToFileHDR(const wchar_t* a_fileName, int w, int h, const float* a_data) = 0;
  virtual void SaveLDRImageToFileLDR(const wchar_t* a_fileName, int w, int h, const int*   a_data) = 0;

private:
  IHRImageTool(const IHRImageTool& a_rhs)            = default;
  IHRImageTool& operator=(const IHRImageTool& a_rhs) = default;
};


class InternalImageTool : public IHRImageTool
{
public:

  InternalImageTool() = default;

  bool LoadImageFromFile(const wchar_t* a_fileName,
                         int& w, int& h, int& bpp, std::vector<int>& a_data) override;

  bool LoadImageFromFile(const wchar_t* a_fileName,
                         int& w, int& h, std::vector<float>& a_data) override;

  void SaveHDRImageToFileHDR(const wchar_t* a_fileName, int w, int h, const float* a_data) override;
  void SaveLDRImageToFileLDR(const wchar_t* a_fileName, int w, int h, const int*   a_data) override;
};
