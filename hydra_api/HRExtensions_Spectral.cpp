#include "HRExtensions_Spectral.h"

#include "HR_HDRImageTool.h"
#include "HydraObjectManager.h"
#include "HydraXMLHelpers.h"
extern HRObjectManager g_objManager;

#include <filesystem>
#include <algorithm>

namespace hr_spectral
{
  //from PBRT-v3
  float AverageSpectrumSamples(const float *lambda, const float *vals, int n,
                               float lambdaStart, float lambdaEnd) {
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

  LiteMath::float3 ToXYZ(const std::vector<float> &spec, const std::vector<int> &wavelengths,
                    const std::vector<float> &Xconv, const std::vector<float> &Yconv, const std::vector<float> &Zconv)
  {
    LiteMath::float3 xyz {0.0f, 0.0f, 0.0f};
    for (int i = 0; i < spec.size(); ++i)
    {
      xyz[0] += Xconv[i] * spec[i];
      xyz[1] += Yconv[i] * spec[i];
      xyz[2] += Zconv[i] * spec[i];
    }

    float scale = float(wavelengths.back() - wavelengths.front()) / float(CIE_Y_integral * spec.size());
    xyz[0] *= scale;
    xyz[1] *= scale;
    xyz[2] *= scale;

    return xyz;
  }

  LiteMath::float3 ToRGB(const std::vector<float> &spec, const std::vector<int> &wavelengths,
                    const std::vector<float> &Xconv, const std::vector<float> &Yconv, const std::vector<float> &Zconv)
  {
    auto xyz = ToXYZ(spec, wavelengths, Xconv, Yconv, Zconv);
    return XYZToRGB(xyz);
  }

  void SpectralDataToRGB(std::filesystem::path &a_filePath, int a_width, int a_height, const std::vector<std::vector<float>> &spec, const std::vector<int> &wavelengths)
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

  void SpectralImagesToRGB(std::filesystem::path &a_filePath, const std::vector<std::filesystem::path> &a_specPaths, const std::vector<int> &wavelengths)
  {
    if(a_specPaths.size() != wavelengths.size())
    {
      HrError(L"[hr_spectral::SpectralImagesToRGB] Number of spectral images is not equal to size of wavelengths vector");
      return;
    }

    auto pImgTool = g_objManager.m_pImgTool;

    std::vector<std::vector<float>> spectralData;
    spectralData.resize(wavelengths.size());

    int width, height, channels;
    pImgTool->LoadImageFromFile(a_specPaths[0].wstring().c_str(), width, height, channels, spectralData[0]);
    for(size_t i = 1; i < a_specPaths.size(); ++i)
    {
      int w, h, chan;
      pImgTool->LoadImageFromFile(a_specPaths[i].wstring().c_str(), w, h, chan, spectralData[i]);
      if(w != width || h != height || chan != channels)
      {
        HrError(L"[hr_spectral::SpectralImagesToRGB] Spectral image does not match the first spectral image in size. Image index: ", i);
        return;
      }
    }

    SpectralDataToRGB(a_filePath, width, height, spectralData, wavelengths);
  }

  std::vector<HRMaterialRef> CreateSpectralDiffuseMaterials(const std::vector<int> &wavelengths,
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

  std::vector<HRLightRef> CreateSpectralLights(const HRLightRef &baseLight, const std::vector<int> &wavelengths,
                                               const std::vector<float> &spd, const std::wstring& name)
  {
    assert(wavelengths.size() == spd.size());
    std::vector<HRLightRef> result;
    result.reserve(spd.size());

    hrLightOpen(baseLight, HR_OPEN_READ_ONLY);
    auto baselightNode = hrLightParamNode(baseLight);

    for(int i = 0 ; i < spd.size(); ++i)
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

  std::vector<HRMaterialRef> CreateSpectralDiffuseMaterialsFromSPDFile(const std::filesystem::path &spd_file,
                                                                       const std::vector<int> &wavelengths,
                                                                       const std::wstring& name)
  {
    if(!std::filesystem::exists(spd_file))
    {
      HrError(L".spd file does not exist");
      return {};
    }

    std::vector<int> file_wavelengths;
    std::vector<float> file_spd;

    std::ifstream in(spd_file);
    std::string line;
    while(std::getline(in, line))
    {
      auto split_pos = line.find_first_of(' ');
      int wavelength = std::stoi(line.substr(0, split_pos));
      float power    = std::stof(line.substr(split_pos + 1, (line.size() - split_pos)));

      file_spd.push_back(power);
      file_wavelengths.push_back(wavelength);
    }

    std::vector<float> spd;
    spd.reserve(wavelengths.size());
    for(const auto& w: wavelengths)
    {
      auto found = std::find_if(file_wavelengths.begin(), file_wavelengths.end(), [w](int x){return x == w;});
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

  std::vector<HRMaterialRef> CreateSpectralTexturedDiffuseMaterials(const std::vector<int> &wavelengths,
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