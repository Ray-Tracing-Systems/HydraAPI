#pragma once

#include <vector>
#include <string>

#if defined WIN32
#include <windows.h>
#endif

#include <math.h>
#include <cstdint>
#include "FreeImage.h"

#include "HR_HDRImage.h"

namespace HydraRender
{
  void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message);

  void SaveHDRImageToFileHDR(const std::string& a_fileName, int w, int h, const float* a_data);
  void SaveImageToFile(const std::string& a_fileName, int w, int h, unsigned int* data);
  void SaveImageToFile(const std::string& a_fileName, const HydraRender::HDRImage4f& image, const float a_gamma = 2.2f);

  void SaveHDRImageToFileHDR(const std::wstring& a_fileName, int w, int h, const float* a_data);
  void SaveImageToFile      (const std::wstring& a_fileName, int w, int h, const unsigned int* data);
  void SaveImageToFile      (const std::wstring& a_fileName, const HydraRender::HDRImage4f& image, const float a_gamma = 2.2f);


  void LoadImageFromFile(const std::string& a_fileName, std::vector<float>& data, int& w, int& h);
  void LoadImageFromFile(const std::string& a_fileName, HydraRender::HDRImage4f& image);

  void LoadImageFromFile(const std::wstring& a_fileName, std::vector<float>& data, int& w, int& h);
  void LoadImageFromFile(const std::wstring& a_fileName, HydraRender::HDRImage4f& image);

  template<typename ContainerT>
  float MSE(const ContainerT& image1, const ContainerT& image2)
  {
    if (image1.size() != image2.size())
      return 100000.0f;

    double summ = 0;

    #pragma omp parallel for reduction(+:summ)
    for (int i = 0; i < image1.size(); i++)
    {
      auto c1 = image1[i];
      auto c2 = image2[i];

      summ += double((c1 - c2)*(c1 - c2));
    }

    return float(summ) / float(image1.size());
  }

  template<typename ContainerT>
  float MSE3(const ContainerT& image1, const ContainerT& image2)
  {
    if (image1.size() != image2.size())
      return 100000.0f;

    double summ = 0;

    #pragma omp parallel for reduction(+:summ)
    for (int i = 0; i < image1.size(); i+=4)
    {
      auto c1x = image1[i + 0];
      auto c2x = image2[i + 0];

      auto c1y = image1[i + 1];
      auto c2y = image2[i + 1];

      auto c1z = image1[i + 2];
      auto c2z = image2[i + 2];

      summ += double((c1x - c2x)*(c1x - c2x) + (c1y - c2y)*(c1y - c2y) + (c1z - c2z)*(c1z - c2z));
    }

    return float(summ) / float(image1.size());
  }

};
