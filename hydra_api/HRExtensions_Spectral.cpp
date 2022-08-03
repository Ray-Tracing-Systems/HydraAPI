#include "HRExtensions_Spectral.h"
#include "HRExtensions_SpectralConstants.h"

#include "HR_HDRImageTool.h"
#include "HydraObjectManager.h"
#include "HydraXMLHelpers.h"
extern HRObjectManager g_objManager;

#include <filesystem>
#include <algorithm>

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
                         const std::vector<float> &wavelengths)
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
    ComputeXYZConversionCurves(wavelengths.front(), wavelengths.back(), wavelengths.size(), X, Y, Z);

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
    for(size_t i = 0; i < a_width * a_height; ++i)
    {
      std::vector<float> spectralPixel(spec.size());
      for(size_t j = 0; j < spec.size(); ++j) // TODO: bad memory access, worth it to fix?
        spectralPixel[j] = spec[j][i];

      auto rgb = ToRGB(spectralPixel, wavelengths, X, Y, Z);
      imgData[i * channels + 0] = rgb.x;
      imgData[i * channels + 1] = rgb.y;
      imgData[i * channels + 2] = rgb.z;
      imgData[i * channels + 3] = 1.0f;
    }


    auto pImgTool = g_objManager.m_pImgTool;
//    pImgTool->SaveLDRImageToFileLDR(a_filePath.wstring().c_str(), a_width, a_height, imgData.data());
    pImgTool->SaveHDRImageToFileHDR(a_filePath.wstring().c_str(), a_width, a_height, 4, imgData.data());

//    if (imgData.size() > TEMP_BUFFER_MAX_SIZE_DONT_FREE)
//      imgData = g_objManager.EmptyBuffer();
  }

  void SpectralDataToY(std::filesystem::path &a_filePath, int a_width, int a_height, const std::vector<std::vector<float>> &spec,
                       const std::vector<float> &wavelengths)
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
    ComputeXYZConversionCurves(wavelengths.front(), wavelengths.back(), wavelengths.size(), X, Y, Z);

    std::vector<float> imgData(a_width * a_height);

#pragma omp parallel for
    for(size_t i = 0; i < a_width * a_height; ++i)
    {
      std::vector<float> spectralPixel(spec.size());
      for(size_t j = 0; j < spec.size(); ++j) // TODO: bad memory access, worth it to fix?
        spectralPixel[j] = spec[j][i];

      imgData[i] =  ToY(spectralPixel, wavelengths, Y);
    }

    auto pImgTool = g_objManager.m_pImgTool;
    pImgTool->SaveHDRImageToFileHDR(a_filePath.wstring().c_str(), a_width, a_height, 1, imgData.data());
  }

  void AddLoadedBufferToSingleChannelImg(std::vector<float> &imgData, const std::vector<float> &buffer, const int32_t channels)
  {
    // add all channels from buf to imgData
    #pragma omp parallel for
    for(size_t i = 0; i < imgData.size(); ++i)
    {
      for(int k = 0; k < channels; ++k)
      {
        imgData[i] += buffer[i * channels + k];
      }
    }
  }

  void AppendLoadedBufferToSpectralData(std::vector<std::vector<float>> &spectralData, std::vector<float> &buffer,
                                        const int32_t a_width, const int32_t a_height, const int32_t channels)
  {
    if(channels == 1)
      spectralData.push_back(std::move(buffer));
    else
    {
      for (int k = 0; k < channels; ++k)
      {
        std::vector<float> oneChannel(a_width * a_height);
#pragma omp parallel for
        for (size_t j = 0; j < a_width * a_height; ++j)
        {
          oneChannel[j] = buffer[j * channels + k];
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
    for(size_t j = 0; j < imgData.size(); ++j)
    {
      imgData[j] *= divisor;
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
    for(size_t i = 0; i < width * height; ++i)
    {
      std::vector<float> spectralPixel(spectralData.size());
      for(size_t j = 0; j < spectralData.size(); ++j) // TODO: bad memory access, worth it to fix?
        spectralPixel[j] = spectralData[j][i];

      imgData[i] = AverageSpectrumSamples(wavelengths.data(), spectralPixel.data(), spectralPixel.size(),
                                          wavelengths.front(), wavelengths.back());
    }

    pImgTool->SaveHDRImageToFileHDR(a_filePath.wstring().c_str(), width, height, 1, imgData.data());
  }


  void SpectralImagesToRGB(std::filesystem::path &a_filePath, const std::vector<std::filesystem::path> &a_specPaths,
                           const std::vector<float> &wavelengths)
  {
    if(a_specPaths.size() != wavelengths.size())
    {
      HrError(L"[hr_spectral::SpectralImagesToRGB] Number of spectral images is not equal to size of wavelengths vector");
      return;
    }

    int width, height;
    auto spectralData = LoadSpectralDataFromFiles(a_specPaths, width, height);

    SpectralDataToRGB(a_filePath, width, height, spectralData, wavelengths);
  }

  void SpectralImagesToY(std::filesystem::path &a_filePath, const std::vector<std::filesystem::path> &a_specPaths,
                         const std::vector<float> &wavelengths)
  {
    if(a_specPaths.size() != wavelengths.size())
    {
      HrError(L"[hr_spectral::SpectralImagesToY] Number of spectral images is not equal to size of wavelengths vector");
      return;
    }

    int width, height;
    auto spectralData = LoadSpectralDataFromFiles(a_specPaths, width, height);

    SpectralDataToY(a_filePath, width, height, spectralData, wavelengths);
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

  std::vector<HRMaterialRef> CreateSpectralTexturedDiffuseMaterials(const std::vector<float> &wavelengths,
                                                                    const std::vector<std::filesystem::path> &texPaths,
                                                                    const LiteMath::float4x4 &texMatrix,
                                                                    const std::wstring& name)
  {
    assert(wavelengths.size() == texPaths.size());
    std::vector<HRMaterialRef> result;
    result.reserve(texPaths.size());

    for(int i = 0 ; i < wavelengths.size(); ++i)
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