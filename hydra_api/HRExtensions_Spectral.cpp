#include "HRExtensions_Spectral.h"
#include "HRExtensions_SpectralConstants.h"

#include "HR_HDRImageTool.h"
#include "HydraObjectManager.h"
#include "HydraXMLHelpers.h"
extern HRObjectManager g_objManager;

#include <filesystem>
#include <algorithm>



bool SaveImagesToMultilayerEXR(const float** data, int width, int height, const char** outchannelnames, int n_images, const char* outfilename, bool a_invertY = false);

namespace hr_spectral
{
  //from PBRT-v3
  float AverageSpectrumSamples(const float *lambda, const float *vals, int n, float lambdaStart, float lambdaEnd)
  {
//    for (int i = 0; i < n - 1; ++i) CHECK_GT(lambda[i + 1], lambda[i]);
//    CHECK_LT(lambdaStart, lambdaEnd);
    // Handle cases with out-of-bounds range or single sample only
    if (lambdaEnd <= lambda[0]) return vals[0];
    if (lambdaStart >= lambda[n - 1]) return vals[n - 1];
    if (n == 1) return vals[0];
    float sum = 0;
    // Add contributions of constant segments before/after samples
    if (lambdaStart < lambda[0]) sum += vals[0] * (lambda[0] - lambdaStart);
    if (lambdaEnd > lambda[n - 1])
      sum += vals[n - 1] * (lambdaEnd - lambda[n - 1]);

    // Advance to first relevant wavelength segment
    int i = 0;
    while (lambdaStart > lambda[i + 1]) ++i;
//    CHECK_LT(i + 1, n);

    // Loop over wavelength sample segments and add contributions
    auto interp = [lambda, vals](float w, int i) {
      return LiteMath::lerp(vals[i], vals[i + 1], (w - lambda[i]) / (lambda[i + 1] - lambda[i]));
    };

    for (; i + 1 < n && lambdaEnd >= lambda[i]; ++i)
    {
      float segLambdaStart = std::max(lambdaStart, lambda[i]);
      float segLambdaEnd = std::min(lambdaEnd, lambda[i + 1]);
      sum += 0.5f * (interp(segLambdaStart, i) + interp(segLambdaEnd, i)) *
             (segLambdaEnd - segLambdaStart);
    }
    return sum / (lambdaEnd - lambdaStart);
  }

  // Given a piecewise-linear SPD with values in vIn[] at corresponding
// wavelengths lambdaIn[], where lambdaIn is assumed to be sorted but may
// be irregularly spaced, resample the spectrum over the range of
// wavelengths [lambdaMin, lambdaMax], with a total of nOut wavelength
// samples between lambdaMin and lamdbaMax (including those at
// endpoints). The resampled spectrum values are written to vOut.
//from PBRT-v3
  void ResampleLinearSpectrum(const float* lambdaIn, const float* vIn, int nIn, 
    float lambdaMin, float lambdaMax, int nOut, float* vOut) 
  {
    assert(nOut > 2);
    
    //for (int i = 0; i < nIn - 1; ++i) CHECK_GT(lambdaIn[i + 1], lambdaIn[i]);
    //CHECK_LT(lambdaMin, lambdaMax);


    float delta = (lambdaMax - lambdaMin) / (nOut - 1);

    auto lambdaInClamped = [&](int index) {
      assert(index >= -1 && index <= nIn);
      if (index == -1) 
      {
        //CHECK_LT(lambdaMin - delta, lambdaIn[0]);
        return lambdaMin - delta;
      }
      else if (index == nIn) 
      {
        //CHECK_GT(lambdaMax + delta, lambdaIn[nIn - 1]);
        return lambdaMax + delta;
      }
      return lambdaIn[index];
    };

    
    auto vInClamped = [&](int index) {
      assert(index >= -1 && index <= nIn);
      return vIn[LiteMath::clamp(index, 0, nIn - 1)];
    };


    auto resample = [&](float lambda) -> float {
      if (lambda + delta / 2 <= lambdaIn[0]) return vIn[0];
      if (lambda - delta / 2 >= lambdaIn[nIn - 1]) return vIn[nIn - 1];
      
      if (nIn == 1) return vIn[0];

      int start, end;
      if (lambda - delta < lambdaIn[0])
      {
        start = -1;
      }
      else 
      {
        start = FindInterval(nIn, [&](int i) { return lambdaIn[i] <= lambda - delta; });
        assert(start >= 0 && start < nIn);
      }

      if (lambda + delta > lambdaIn[nIn - 1])
      {
        end = nIn;
      }
      else 
      {
        end = start > 0 ? start : 0;
        while (end < nIn && lambda + delta > lambdaIn[end]) ++end;
      }

      if (end - start == 2 && lambdaInClamped(start) <= lambda - delta &&
          lambdaIn[start + 1] == lambda &&
          lambdaInClamped(end) >= lambda + delta) 
      {
        return vIn[start + 1];
      }
      else if (end - start == 1) 
      {
        float t = (lambda - lambdaInClamped(start)) /
          (lambdaInClamped(end) - lambdaInClamped(start));
        assert(t >= 0 && t <= 1);
        return LiteMath::lerp(vInClamped(start), vInClamped(end), t);
      }
      else 
      {
        return AverageSpectrumSamples(lambdaIn, vIn, nIn, lambda - delta / 2, lambda + delta / 2);
      }
    };


    for (int outOffset = 0; outOffset < nOut; ++outOffset) 
    {
      float lambda = LiteMath::lerp(lambdaMin, lambdaMax, float(outOffset) / (nOut - 1));
      vOut[outOffset] = resample(lambda);
    }
  }

  void ComputeXYZConversionCurves(float firstWavelength, float lastWavelength, uint32_t totalWavelengths,
                                  std::vector<float> &X, std::vector<float> &Y, std::vector<float> &Z)
  {
    X.resize(totalWavelengths);
    Y.resize(totalWavelengths);
    Z.resize(totalWavelengths);
    for (int i = 0; i < totalWavelengths; ++i)
    {
      auto wl0 = LiteMath::lerp(firstWavelength, lastWavelength, float(i) / float(totalWavelengths));
      auto wl1 = LiteMath::lerp(firstWavelength, lastWavelength, float(i + 1) / float(totalWavelengths));
      X[i] = AverageSpectrumSamples(CIE_lambda, CIE_X, nCIESamples, wl0, wl1);
      Y[i] = AverageSpectrumSamples(CIE_lambda, CIE_Y, nCIESamples, wl0, wl1);
      Z[i] = AverageSpectrumSamples(CIE_lambda, CIE_Z, nCIESamples, wl0, wl1);
    }
  }

  // temporary solution
  void ComputeXYZConversionCurvesUneven(const std::vector<float> wavelengths, std::vector<float>& X, std::vector<float>& Y, std::vector<float>& Z,
    float step = 10.0f)
  {
    X.resize(wavelengths.size());
    Y.resize(wavelengths.size());
    Z.resize(wavelengths.size());
    float firstWavelength = wavelengths.front();
    float lastWavelength = wavelengths.back();
    float diff = (lastWavelength - firstWavelength);
    int totalSteps = int(diff / step) + 1;

    std::vector<float> XX;
    std::vector<float> YY;
    std::vector<float> ZZ;
    XX.resize(totalSteps); YY.resize(totalSteps); ZZ.resize(totalSteps);
    for (int i = 0; i < totalSteps; ++i)
    {
      auto wl0 = LiteMath::lerp(firstWavelength, lastWavelength, float(i) / float(totalSteps));
      auto wl1 = LiteMath::lerp(firstWavelength, lastWavelength, float(i + 1) / float(totalSteps));
      XX[i] = AverageSpectrumSamples(CIE_lambda, CIE_X, nCIESamples, wl0, wl1);
      YY[i] = AverageSpectrumSamples(CIE_lambda, CIE_Y, nCIESamples, wl0, wl1);
      ZZ[i] = AverageSpectrumSamples(CIE_lambda, CIE_Z, nCIESamples, wl0, wl1);
    }

    for (int i = 0; i < wavelengths.size(); ++i)
    {
      int steps = int((wavelengths[i] - firstWavelength) / step);
      X[i] = XX[steps];
      Y[i] = YY[steps];
      Z[i] = ZZ[steps];
    }

  }

  LiteMath::float3 ToXYZ(const std::vector<float> &spec, const std::vector<float> &wavelengths,
                    const std::vector<float> &Xconv, const std::vector<float> &Yconv, const std::vector<float> &Zconv)
  {
    LiteMath::float3 xyz {0.0f, 0.0f, 0.0f};
    for (int i = 0; i < spec.size(); ++i)
    {
      xyz[0] += Xconv[i] * spec[i];
      xyz[1] += Yconv[i] * spec[i];
      xyz[2] += Zconv[i] * spec[i];
    }

    float scale = (wavelengths.back() - wavelengths.front()) / float(CIE_Y_integral * spec.size());
    xyz[0] *= scale;
    xyz[1] *= scale;
    xyz[2] *= scale;

    return xyz;
  }

  float ToY(const std::vector<float> &spec, const std::vector<float> &wavelengths, const std::vector<float> &Yconv)
  {
    float yy = 0.f;
    for (int i = 0; i < spec.size(); ++i) yy += Yconv[i] * spec[i];
    return yy * (wavelengths.back() - wavelengths.front()) / float(CIE_Y_integral * spec.size());
  }

  LiteMath::float3 ToRGB(const std::vector<float> &spec, const std::vector<float> &wavelengths,
                    const std::vector<float> &Xconv, const std::vector<float> &Yconv, const std::vector<float> &Zconv)
  {
    auto xyz = ToXYZ(spec, wavelengths, Xconv, Yconv, Zconv);
    return XYZToRGB(xyz);
  }

  void SpectralDataToRGB(std::filesystem::path &a_filePath, int a_width, int a_height, const std::vector<std::vector<float>> &spec,
                         const std::vector<float> &wavelengths, bool need_resample)
  {
    if(spec.empty() || wavelengths.empty())
    {
      HrError(L"[hr_spectral::SpectralDataToRGB] Empty spectral vector or wavelengths vector");
      return;
    }

    if(a_width * a_height != spec[0].size())
    {
      HrError(L"[hr_spectral::SpectralDataToRGB] Size of a single image from spectral vector is not equal to width * height");
      return;
    }

    if(spec.size() != wavelengths.size())
    {
      HrError(L"[hr_spectral::SpectralDataToRGB] Number of spectral samples is not equal to size of wavelengths vector");
      return;
    }

    std::vector<float> X;
    std::vector<float> Y;
    std::vector<float> Z;

    if (need_resample)
    {
      // TODO: proper resample
      /*ResampleLinearSpectrum(wavelengths.data(), spec.data(), spec.size(), wavelengths.front(), wavelengths.back(),
        wavelengths.size(), spectrum_new.data());*/
      ComputeXYZConversionCurvesUneven(wavelengths, X, Y, Z);
    }
    else
    {
      ComputeXYZConversionCurves(wavelengths.front(), wavelengths.back(), wavelengths.size(), X, Y, Z);
    }

//    auto& imgData = g_objManager.m_tempBuffer;
//    if (imgData.size() < a_width * a_height)
//      imgData.resize(a_width * a_height);
//    for(size_t i = 0; i < a_width * a_height; ++i)
//    {
//      std::vector<float> spectralPixel(spec.size());
//      for(size_t j = 0; j < spec.size(); ++j)
//        spectralPixel[j] = spec[j][i];
//
//      auto rgb = ToRGB(spectralPixel, wavelengths, X, Y, Z);
//      imgData[i] = RealColorToUint32(rgb.x, rgb.y, rgb.z, 1.0f);
//    }

    int channels = 4;
    std::vector<float> imgData(a_width * a_height * channels);

    #pragma omp parallel for
    for(int i = 0; i < a_width * a_height; ++i)
    {
      std::vector<float> spectralPixel(spec.size());
      for(int j = 0; j < spec.size(); ++j) // TODO: bad memory access, worth it to fix?
        spectralPixel[(size_t)(j)] = spec[(size_t)(j)][(size_t)(i)];

      auto rgb = ToRGB(spectralPixel, wavelengths, X, Y, Z);
      imgData[i * channels + 0] = rgb.x;
      imgData[i * channels + 1] = rgb.y;
      imgData[i * channels + 2] = rgb.z;
      imgData[i * channels + 3] = 1.0f;
    }


    auto pImgTool = g_objManager.m_pImgTool;
    if(a_filePath.extension().string() == std::string(".exr")) 
      pImgTool->SaveHDRImageToFileHDR(a_filePath.wstring().c_str(), a_width, a_height, 4, imgData.data());
    else // if(a_filePath.extension().string() == std::string(".png")) 
      pImgTool->SaveHDRImageToFileLDR(a_filePath.wstring().c_str(), a_width, a_height, 4, imgData.data());
    

//    if (imgData.size() > TEMP_BUFFER_MAX_SIZE_DONT_FREE)
//      imgData = g_objManager.EmptyBuffer();
  }

  void SpectralDataToY(std::filesystem::path &a_filePath, int a_width, int a_height, const std::vector<std::vector<float>> &spec,
                       const std::vector<float> &wavelengths, bool need_resample)
  {
    if(spec.empty() || wavelengths.empty())
    {
      HrError(L"[hr_spectral::SpectralDataToY] Empty spectral vector or wavelengths vector");
      return;
    }

    if(a_width * a_height != spec[0].size())
    {
      HrError(L"[hr_spectral::SpectralDataToY] Size of a single image from spectral vector is not equal to width * height");
      return;
    }

    if(spec.size() != wavelengths.size())
    {
      HrError(L"[hr_spectral::SpectralDataToY] Number of spectral samples is not equal to size of wavelengths vector");
      return;
    }

    std::vector<float> X;
    std::vector<float> Y;
    std::vector<float> Z;
    if (need_resample)
    {
      // TODO: proper resample
      /*ResampleLinearSpectrum(wavelengths.data(), spec.data(), spec.size(), wavelengths.front(), wavelengths.back(),
        wavelengths.size(), spectrum_new.data());*/
      ComputeXYZConversionCurvesUneven(wavelengths, X, Y, Z);
    }
    else
    {
      ComputeXYZConversionCurves(wavelengths.front(), wavelengths.back(), wavelengths.size(), X, Y, Z);
    }

    std::vector<float> imgData(a_width * a_height);

#pragma omp parallel for
    for(int i = 0; i < a_width * a_height; ++i)
    {
      std::vector<float> spectralPixel(spec.size());
      for(int j = 0; j < spec.size(); ++j) // TODO: bad memory access, worth it to fix?
        spectralPixel[(size_t)(j)] = spec[(size_t)(j)][(size_t)(i)];

      imgData[i] =  ToY(spectralPixel, wavelengths, Y);
    }

    auto pImgTool = g_objManager.m_pImgTool;
    pImgTool->SaveHDRImageToFileHDR(a_filePath.wstring().c_str(), a_width, a_height, 1, imgData.data());
  }

  void AddLoadedBufferToSingleChannelImg(std::vector<float> &imgData, const std::vector<float> &buffer, const int32_t channels)
  {
    int dataChannels = (channels == 4) ? 3 : channels; // alpha channel does not store spectral data
    // add all channels from buf to imgData
    #pragma omp parallel for
    for(int i = 0; i < imgData.size(); ++i)
    {
      for(int k = 0; k < dataChannels; ++k)
      {
        imgData[(size_t)(i)] += buffer[(size_t)(i) * channels + (size_t)(k)];
      }
    }
  }

  void AppendLoadedBufferToSpectralData(std::vector<std::vector<float>> &spectralData, std::vector<float> &buffer,
                                        const int32_t a_width, const int32_t a_height, const int32_t channels)
  {
    int dataChannels = (channels == 4) ? 3 : channels; // alpha channel does not store spectral data
    if(channels == 1)
      spectralData.push_back(std::move(buffer));
    else
    {
      for (int k = 0; k < dataChannels; ++k)
      {
        std::vector<float> oneChannel(a_width * a_height);
#pragma omp parallel for
        for (int j = 0; j < a_width * a_height; ++j)
        {
          oneChannel[(size_t)(j)] = buffer[(size_t)(j) * channels + (size_t)(k)];
        }
        spectralData.push_back(std::move(oneChannel));
      }
    }
  }

  std::vector<std::vector<float>> LoadSpectralDataFromFiles(const std::vector<std::filesystem::path> &a_specPaths,
                                                            int32_t &a_width, int32_t &a_height)
  {
    auto pImgTool = g_objManager.m_pImgTool;

    std::vector<std::vector<float>> spectralData;
    spectralData.reserve(a_specPaths.size());

    int channels;
    std::vector<float> tempBuf;
    pImgTool->LoadImageFromFile(a_specPaths[0].wstring().c_str(), a_width, a_height, channels, tempBuf);
    AppendLoadedBufferToSpectralData(spectralData, tempBuf, a_width, a_height, channels);

    for(int i = 1; i < a_specPaths.size(); ++i)
    {
      int w, h;
      pImgTool->LoadImageFromFile(a_specPaths[i].wstring().c_str(), w, h, channels, tempBuf);
      if(w != a_width || h != a_height)
      {
        HrError(L"[hr_spectral::LoadSpectralDataFromFiles] Spectral image does not match the first spectral image in size. Image index: ", i);
        return {};
      }
      AppendLoadedBufferToSpectralData(spectralData, tempBuf, a_width, a_height, channels);
    }

    return spectralData;
  }

  void AverageSpectralImages(std::filesystem::path &a_filePath, const std::vector<std::filesystem::path> &a_specPaths)
  {
    auto pImgTool = g_objManager.m_pImgTool;

    int width, height, channels;
    std::vector<float> tempBuf;
    pImgTool->LoadImageFromFile(a_specPaths[0].wstring().c_str(), width, height, channels, tempBuf);

    std::vector<float> imgData(width * height);
    AddLoadedBufferToSingleChannelImg(imgData, tempBuf, channels);
    int totalSpectralBands = channels;

    for(int i = 1; i < a_specPaths.size(); ++i)
    {
      int w, h;
      pImgTool->LoadImageFromFile(a_specPaths[i].wstring().c_str(), w, h, channels, tempBuf);
      if(w != width || h != height)
      {
        HrError(L"[hr_spectral::AverageSpectralImages] Spectral image does not match the first spectral image in size. Image index: ", i);
        return;
      }
      AddLoadedBufferToSingleChannelImg(imgData, tempBuf, channels);
      totalSpectralBands += channels;
    }

    const float divisor = 1.0f / float(totalSpectralBands);
    #pragma omp parallel for
    for(int j = 0; j < imgData.size(); ++j)
    {
      imgData[(size_t)(j)] *= divisor;
    }

    pImgTool->SaveHDRImageToFileHDR(a_filePath.wstring().c_str(), width, height, 1, imgData.data());
  }

  void AverageSpectralImagesV2(std::filesystem::path &a_filePath, const std::vector<std::filesystem::path> &a_specPaths,
                               const std::vector<float> &wavelengths)
  {
    auto pImgTool = g_objManager.m_pImgTool;

    int width, height;
    auto spectralData = LoadSpectralDataFromFiles(a_specPaths, width, height);

    std::vector<float> imgData(width * height);
    #pragma omp parallel for
    for(int i = 0; i < width * height; ++i)
    {
      std::vector<float> spectralPixel(spectralData.size());
      for(int j = 0; j < spectralData.size(); ++j) // TODO: bad memory access, worth it to fix?
        spectralPixel[(size_t)(j)] = spectralData[(size_t)(j)][(size_t)(i)];

      imgData[i] = AverageSpectrumSamples(wavelengths.data(), spectralPixel.data(), spectralPixel.size(),
                                          wavelengths.front(), wavelengths.back());
    }

    pImgTool->SaveHDRImageToFileHDR(a_filePath.wstring().c_str(), width, height, 1, imgData.data());
  }


  void SpectralImagesToRGB(std::filesystem::path &a_filePath, const std::vector<std::filesystem::path> &a_specPaths,
                           const std::vector<float> &wavelengths, bool need_resample)
  {
    int width, height;
    auto spectralData = LoadSpectralDataFromFiles(a_specPaths, width, height);

    SpectralDataToRGB(a_filePath, width, height, spectralData, wavelengths, need_resample);
  }


  void SpectralImagesToY(std::filesystem::path &a_filePath, const std::vector<std::filesystem::path> &a_specPaths,
                         const std::vector<float> &wavelengths, bool need_resample)
  {
    int width, height;
    auto spectralData = LoadSpectralDataFromFiles(a_specPaths, width, height);

    SpectralDataToY(a_filePath, width, height, spectralData, wavelengths, need_resample);
  }

  void SpectralImagesToMultilayerEXR(std::filesystem::path& a_filePath, const std::vector<std::filesystem::path>& a_specPaths, 
    const std::vector<float>& wavelengths)
  {
    int width, height;
    auto spectralData = LoadSpectralDataFromFiles(a_specPaths, width, height);
    std::vector<const float*> spectralDataPlain;
    spectralDataPlain.reserve(spectralData.size());
    for (const auto& spec : spectralData)
    {
      spectralDataPlain.push_back(spec.data());
    }

    std::vector<std::string> channel_names_tmp(wavelengths.size());
    std::vector<const char*> channel_names(wavelengths.size());
    for (size_t i = 0; i < wavelengths.size(); i++)
    {
      channel_names_tmp[i] = std::to_string(int(wavelengths[i])) + ".Y";
      channel_names[i] = channel_names_tmp[i].c_str();
    }
    SaveImagesToMultilayerEXR(spectralDataPlain.data(), width, height, channel_names.data(), wavelengths.size(), a_filePath.string().c_str(), true);
  }

  std::vector<HRMaterialRef> CreateSpectralDiffuseMaterials(const std::vector<float> &wavelengths,
                                                            const std::vector<float> &spd,
                                                            const std::wstring& name)
  {
    assert(wavelengths.size() == spd.size());
    std::vector<HRMaterialRef> result;
    result.reserve(spd.size());

    for(int i = 0 ; i < spd.size(); ++i)
    {
      std::wstringstream ws;
      ws << name << L"_" << wavelengths[i];
      HRMaterialRef mat = hrMaterialCreate(ws.str().c_str());
      hrMaterialOpen(mat, HR_WRITE_DISCARD);
      {
        auto matNode = hrMaterialParamNode(mat);

        auto diff = matNode.append_child(L"diffuse");
        diff.append_attribute(L"brdf_type").set_value(L"lambert");

        auto color = diff.append_child(L"color");
        auto val = color.append_attribute(L"val");
        HydraXMLHelpers::WriteFloat3(val, {spd[i], spd[i], spd[i]});
      }
      hrMaterialClose(mat);

      result.push_back(mat);
    }

    return result;
  }

  std::vector<HRLightRef> CreateSpectralLights(const HRLightRef &baseLight, const std::vector<float> &wavelengths,
                                               const std::vector<float> &spd, const std::wstring& name)
  {
    assert(wavelengths.size() == spd.size());
    std::vector<HRLightRef> result;
    result.reserve(spd.size());

    hrLightOpen(baseLight, HR_OPEN_READ_ONLY);
    auto baselightNode = hrLightParamNode(baseLight);

    for(size_t i = 0 ; i < spd.size(); ++i)
    {
      std::wstringstream ws;
      ws << name << L"_" << wavelengths[i];
      HRLightRef light = hrLightCreate(ws.str().c_str());
      hrLightOpen(light, HR_WRITE_DISCARD);
      {
        auto lightNode = hrLightParamNode(light);

        HydraXMLHelpers::forceAttributes(baselightNode, lightNode, {L"id", L"name", L"mat_id"});
        HydraXMLHelpers::copyChildNodes(baselightNode, lightNode);

        auto colorNode = lightNode.force_child(L"intensity").force_child(L"color");
        auto val = colorNode.force_attribute(L"val");
        HydraXMLHelpers::WriteFloat3(val, {spd[i], spd[i], spd[i]});
      }
      hrLightClose(light);

      result.push_back(light);
    }

    hrLightClose(baseLight);

    return result;
  }

  std::vector<HRLightRef> CreateSpectralLightsD65(const HRLightRef &baseLight, const std::vector<float> &wavelengths,
                                                  const std::wstring& name)
  {
    std::vector<HRLightRef> result;
    result.reserve(wavelengths.size());

    hrLightOpen(baseLight, HR_OPEN_READ_ONLY);
    auto baselightNode = hrLightParamNode(baseLight);

    for(size_t i = 0 ; i < wavelengths.size(); ++i)
    {
      std::wstringstream ws;
      ws << name << L"_" << wavelengths[i];
      HRLightRef light = hrLightCreate(ws.str().c_str());
      hrLightOpen(light, HR_WRITE_DISCARD);
      {
        auto lightNode = hrLightParamNode(light);

        HydraXMLHelpers::forceAttributes(baselightNode, lightNode, {L"id", L"name", L"mat_id"});
        HydraXMLHelpers::copyChildNodes(baselightNode, lightNode);

        auto colorNode = lightNode.force_child(L"intensity").force_child(L"color");
        auto val = colorNode.force_attribute(L"val");

        if(uint32_t(wavelengths[i]) < D_65_FIRST_WAVELENGTH ||
           uint32_t(wavelengths[i]) > D_65_FIRST_WAVELENGTH + D_65_ARRAY_SIZE * D_65_PRECISION )
        {
          HrError(L"[hr_spectral::CreateSpectralLightsD65] Wavelength out of known range for d65", wavelengths[i]);
          return {};
        }

        float d65_val = D_65_SPD[uint32_t(wavelengths[i]) - D_65_FIRST_WAVELENGTH] / D_65_MAX_VAL;

        HydraXMLHelpers::WriteFloat3(val, {d65_val, d65_val, d65_val});
      }
      hrLightClose(light);

      result.push_back(light);
    }

    hrLightClose(baseLight);

    return result;
  }

  std::vector<HRMaterialRef> CreateSpectralDiffuseMaterialsFromSPDFile(const std::filesystem::path &spd_file,
                                                                       const std::vector<float> &wavelengths,
                                                                       const std::wstring& name)
  {
    if(!std::filesystem::exists(spd_file))
    {
      HrError(L".spd file does not exist");
      return {};
    }

    std::vector<float> file_wavelengths;
    std::vector<float> file_spd;

    std::ifstream in(spd_file);
    std::string line;
    while(std::getline(in, line))
    {
      auto split_pos = line.find_first_of(' ');
      float wavelength = std::stof(line.substr(0, split_pos));
      float power      = std::stof(line.substr(split_pos + 1, (line.size() - split_pos)));

      file_spd.push_back(power);
      file_wavelengths.push_back(wavelength);
    }

    std::vector<float> spd;
    spd.reserve(wavelengths.size());
    for(const auto& w: wavelengths)
    {
      auto found = std::find_if(file_wavelengths.begin(), file_wavelengths.end(),
                                [w](float x){return fabsf(x - w) < LiteMath::EPSILON;});
      if(found != file_wavelengths.end())
      {
        auto idx = found - file_wavelengths.begin();
        spd.push_back(file_spd[idx]);
      }
      else
      {
        // TODO: interpolate values
      }
    }

    std::vector<HRMaterialRef> result;
    result.reserve(spd.size());
    for(int i = 0 ; i < spd.size(); ++i)
    {
      std::wstringstream ws;
      ws << name << L"_" << wavelengths[i];
      HRMaterialRef mat = hrMaterialCreate(ws.str().c_str());
      hrMaterialOpen(mat, HR_WRITE_DISCARD);
      {
        auto matNode = hrMaterialParamNode(mat);

        auto diff = matNode.append_child(L"diffuse");
        diff.append_attribute(L"brdf_type").set_value(L"lambert");

        auto color = diff.append_child(L"color");
        auto val = color.append_attribute(L"val");
        HydraXMLHelpers::WriteFloat3(val, {spd[i], spd[i], spd[i]});
      }
      hrMaterialClose(mat);

      result.push_back(mat);
    }

    return result;
  }

  std::vector<HRMaterialRef> CreateSpectralDiffuseMaterialsFromSPDFile3channel(const std::filesystem::path &spd_file,
                                                                               const std::vector<float> &wavelengths,
                                                                               const std::wstring& name)
  {
    if(!std::filesystem::exists(spd_file))
    {
      HrError(L".spd file does not exist");
      return {};
    }

    std::vector<float> file_wavelengths;
    std::vector<float> file_spd;

    std::ifstream in(spd_file);
    std::string line;
    while(std::getline(in, line))
    {
      auto split_pos = line.find_first_of(' ');
      float wavelength = std::stof(line.substr(0, split_pos));
      float power      = std::stof(line.substr(split_pos + 1, (line.size() - split_pos)));

      file_spd.push_back(power);
      file_wavelengths.push_back(wavelength);
    }

    std::vector<float> spd;
    spd.reserve(wavelengths.size());
    for(const auto& w: wavelengths)
    {
      auto found = std::find_if(file_wavelengths.begin(), file_wavelengths.end(),
                                [w](float x){return fabsf(x - w) < LiteMath::EPSILON;});
      if(found != file_wavelengths.end())
      {
        auto idx = found - file_wavelengths.begin();
        spd.push_back(file_spd[idx]);
      }
      else
      {
        // TODO: interpolate values
      }
    }

    std::vector<HRMaterialRef> result;
    result.reserve(spd.size());
    int i = 0;
    while(i < spd.size())
    {
      std::wstringstream ws;
      ws << name << L"_" << wavelengths[i];
      HRMaterialRef mat = hrMaterialCreate(ws.str().c_str());
      hrMaterialOpen(mat, HR_WRITE_DISCARD);
      {
        auto matNode = hrMaterialParamNode(mat);

        auto diff = matNode.append_child(L"diffuse");
        diff.append_attribute(L"brdf_type").set_value(L"lambert");

        auto color = diff.append_child(L"color");
        auto val = color.append_attribute(L"val");

        if(spd.size() - i == 1 || spd.size() - i == 2)
        {
          HydraXMLHelpers::WriteFloat3(val, {spd[i], spd[i], spd[i]});
          i += 1;
        }
        else
        {
          HydraXMLHelpers::WriteFloat3(val, {spd[i + 0], spd[i + 1], spd[i + 2]});
          i += 3;
        }

      }
      hrMaterialClose(mat);

      result.push_back(mat);
    }

    return result;
  }

  std::vector<HRMaterialRef> CreateSpectralTexturedDiffuseRoughSpecMaterials(const std::vector<float>& wavelengths,
                                                                             const std::vector<std::filesystem::path>& diffTexs,
                                                                             const std::vector<std::filesystem::path>& glossTexs,
                                                                             const LiteMath::float4x4& diffTexMatrix,
                                                                             const LiteMath::float4x4& glossTexMatrix,
                                                                             const float ior,
                                                                             const std::wstring& name)
  {
    assert(wavelengths.size() == diffTexs.size());
    assert(glossTexs.size() == diffTexs.size());
    std::vector<HRMaterialRef> result;
    result.reserve(diffTexs.size());

    for(int i = 0 ; i < wavelengths.size(); ++i)
    {
      std::wstringstream ws;
      ws << name << L"_" << wavelengths[i];

      auto diffTexRef = hrTexture2DCreateFromFile(diffTexs[i].wstring().c_str());
      auto glossTexRef = hrTexture2DCreateFromFile(glossTexs[i].wstring().c_str());

      HRMaterialRef mat = hrMaterialCreate(ws.str().c_str());
      hrMaterialOpen(mat, HR_WRITE_DISCARD);
      {
        auto matNode = hrMaterialParamNode(mat);

        auto diff = matNode.append_child(L"diffuse");
        diff.append_attribute(L"brdf_type").set_value(L"lambert");

        auto color = diff.append_child(L"color");
        auto val = color.append_attribute(L"val");
        val.set_value(L"1.0 1.0 1.0");

        color.append_attribute(L"tex_apply_mode") = L"replace";
        {
          auto texNode = hrTextureBind(diffTexRef, color);

          texNode.append_attribute(L"matrix");
          float samplerMatrix[16] = { diffTexMatrix(0,0), diffTexMatrix(0,1), diffTexMatrix(0,2), diffTexMatrix(0,3),
                                      diffTexMatrix(1,0), diffTexMatrix(1,1), diffTexMatrix(1,2), diffTexMatrix(1,3),
                                      diffTexMatrix(2,0), diffTexMatrix(2,1), diffTexMatrix(2,2), diffTexMatrix(2,3),
                                      diffTexMatrix(3,0), diffTexMatrix(3,1), diffTexMatrix(3,2), diffTexMatrix(3,3), };

          texNode.append_attribute(L"addressing_mode_u").set_value(L"clamp");
          texNode.append_attribute(L"addressing_mode_v").set_value(L"clamp");
          texNode.append_attribute(L"input_gamma").set_value(1.0f);
          texNode.append_attribute(L"input_alpha").set_value(L"rgb");
          HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
        }

        auto refl = matNode.append_child(L"reflectivity");
        refl.append_attribute(L"brdf_type").set_value(L"ggx");
        refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
        refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
        refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(ior);
        auto gloss = refl.append_child(L"glossiness");
        gloss.append_attribute(L"val").set_value(L"1.0");
        {
          auto texNode = hrTextureBind(glossTexRef, gloss);

          texNode.append_attribute(L"matrix");
          float samplerMatrix[16] = { glossTexMatrix(0,0), glossTexMatrix(0,1), glossTexMatrix(0,2), glossTexMatrix(0,3),
                                      glossTexMatrix(1,0), glossTexMatrix(1,1), glossTexMatrix(1,2), glossTexMatrix(1,3),
                                      glossTexMatrix(2,0), glossTexMatrix(2,1), glossTexMatrix(2,2), glossTexMatrix(2,3),
                                      glossTexMatrix(3,0), glossTexMatrix(3,1), glossTexMatrix(3,2), glossTexMatrix(3,3), };

          texNode.append_attribute(L"addressing_mode_u").set_value(L"clamp");
          texNode.append_attribute(L"addressing_mode_v").set_value(L"clamp");
          texNode.append_attribute(L"input_gamma").set_value(1.0f);
          texNode.append_attribute(L"input_alpha").set_value(L"rgb");
          HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
        }
      }
      hrMaterialClose(mat);

      result.push_back(mat);
    }

    return result;
  }

  std::vector<HRMaterialRef> CreateSpectralTexturedDiffuseMaterials(const std::vector<float>& wavelengths,
    const std::vector<std::filesystem::path>& texPaths,
    const LiteMath::float4x4& texMatrix,
    const std::wstring& name)
  {
    assert(wavelengths.size() == texPaths.size());
    std::vector<HRMaterialRef> result;
    result.reserve(texPaths.size());

    for (int i = 0; i < wavelengths.size(); ++i)
    {
      std::wstringstream ws;
      ws << name << L"_" << wavelengths[i];

      auto texRef = hrTexture2DCreateFromFile(texPaths[i].wstring().c_str());

      HRMaterialRef mat = hrMaterialCreate(ws.str().c_str());
      hrMaterialOpen(mat, HR_WRITE_DISCARD);
      {
        auto matNode = hrMaterialParamNode(mat);

        auto diff = matNode.append_child(L"diffuse");
        diff.append_attribute(L"brdf_type").set_value(L"lambert");

        auto color = diff.append_child(L"color");
        auto val = color.append_attribute(L"val");
        val.set_value(L"1.0 1.0 1.0");

        color.append_attribute(L"tex_apply_mode") = L"replace";

        auto texNode = hrTextureBind(texRef, color);

        texNode.append_attribute(L"matrix");
        float samplerMatrix[16] = { texMatrix(0,0), texMatrix(0,1), texMatrix(0,2), texMatrix(0,3),
                                    texMatrix(1,0), texMatrix(1,1), texMatrix(1,2), texMatrix(1,3),
                                    texMatrix(2,0), texMatrix(2,1), texMatrix(2,2), texMatrix(2,3),
                                    texMatrix(3,0), texMatrix(3,1), texMatrix(3,2), texMatrix(3,3), };

        texNode.append_attribute(L"addressing_mode_u").set_value(L"clamp");
        texNode.append_attribute(L"addressing_mode_v").set_value(L"clamp");
        texNode.append_attribute(L"input_gamma").set_value(1.0f);
        texNode.append_attribute(L"input_alpha").set_value(L"rgb");

        HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
      }
      hrMaterialClose(mat);

      result.push_back(mat);
    }

    return result;
  }
}