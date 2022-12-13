#ifndef HYDRAAPI_EX_HREXTENSIONS_SPECTRAL_H
#define HYDRAAPI_EX_HREXTENSIONS_SPECTRAL_H

/*
    Contains code parts taken from pbrt-v3, license:

    pbrt source code is Copyright(c) 1998-2016
                        Matt Pharr, Greg Humphreys, and Wenzel Jakob.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "HydraAPI.h"
#include "LiteMath.h"

#include <filesystem>

namespace hr_spectral
{
  void SpectralImagesToRGB(std::filesystem::path &a_filePath, const std::vector<std::filesystem::path> &a_specPaths,
                           const std::vector<float> &wavelengths, bool need_resample = false);
  void SpectralImagesToY(std::filesystem::path &a_filePath, const std::vector<std::filesystem::path> &a_specPaths,
                         const std::vector<float> &wavelengths, bool need_resample = false);
  void SpectralImagesToMultilayerEXR(std::filesystem::path& a_filePath, const std::vector<std::filesystem::path>& a_specPaths,
    const std::vector<float>& wavelengths);
  void AverageSpectralImages(std::filesystem::path &a_filePath, const std::vector<std::filesystem::path> &a_specPaths);
  void AverageSpectralImagesV2(std::filesystem::path &a_filePath, const std::vector<std::filesystem::path> &a_specPaths,
                               const std::vector<float> &wavelengths);

  std::vector<HRMaterialRef> CreateSpectralDiffuseMaterials(const std::vector<float> &wavelengths,
                                                            const std::vector<float> &spd,
                                                            const std::wstring& name = L"mat");

  std::vector<HRMaterialRef> CreateSpectralTexturedDiffuseMaterials(const std::vector<float> &wavelengths,
                                                                    const std::vector<std::filesystem::path> &texPaths,
                                                                    const LiteMath::float4x4 &texMatrix,
                                                                    const std::wstring& name = L"mat");

  std::vector<HRMaterialRef> CreateSpectralTexturedDiffuseRoughSpecMaterials(const std::vector<float>& wavelengths,
                                                                             const std::vector<std::filesystem::path>& diffTexs,
                                                                             const std::vector<std::filesystem::path>& glossTexs,
                                                                             const LiteMath::float4x4& diffTexMatrix,
                                                                             const LiteMath::float4x4& glossTexMatrix,
                                                                             const float ior = 1.5f,
                                                                             const std::wstring& name = L"mat");

  std::vector<HRMaterialRef> CreateSpectralDiffuseMaterialsFromSPDFile(const std::filesystem::path &spd_file,
                                                                       const std::vector<float> &wavelengths,
                                                                       const std::wstring& name = L"mat");

  std::vector<HRMaterialRef> CreateSpectralDiffuseMaterialsFromSPDFile3channel(const std::filesystem::path &spd_file,
                                                                               const std::vector<float> &wavelengths,
                                                                               const std::wstring& name = L"mat");

  std::vector<HRLightRef> CreateSpectralLights(const HRLightRef &baseLight, const std::vector<float> &wavelengths,
                                               const std::vector<float> &spd, const std::wstring& name = L"light");

  std::vector<HRLightRef> CreateSpectralLightsD65(const HRLightRef &baseLight, const std::vector<float> &wavelengths,
                                                  const std::wstring& name = L"light");

  void ComputeXYZConversionCurves(float firstWavelength, float lastWavelength, uint32_t totalWavelengths,
                                  std::vector<float> &X, std::vector<float> &Y, std::vector<float> &Z);

  float Y(const std::vector<float> &spec, const std::vector<float> &wavelengths, const std::vector<float> &Yconv);

  LiteMath::float3 ToXYZ(const std::vector<float> &spec, const std::vector<float> &wavelengths,
                    const std::vector<float> &Xconv, const std::vector<float> &Yconv, const std::vector<float> &Zconv);

  LiteMath::float3 ToRGB(const std::vector<float> &spec, const std::vector<float> &wavelengths,
                    const std::vector<float> &Xconv, const std::vector<float> &Yconv, const std::vector<float> &Zconv);


  //from PBRT-v3
  inline LiteMath::float3 XYZToRGB(const LiteMath::float3 xyz)
  {
    LiteMath::float3 rgb;
    rgb[0] = +3.240479f * xyz[0] - 1.537150f * xyz[1] - 0.498535f * xyz[2];
    rgb[1] = -0.969256f * xyz[0] + 1.875991f * xyz[1] + 0.041556f * xyz[2];
    rgb[2] = +0.055648f * xyz[0] - 0.204043f * xyz[1] + 1.057311f * xyz[2];

    return rgb;
  }

  inline LiteMath::float3 RGBToXYZ(const LiteMath::float3 rgb)
  {
    LiteMath::float3 xyz;
    xyz[0] = 0.412453f * rgb[0] + 0.357580f * rgb[1] + 0.180423f * rgb[2];
    xyz[1] = 0.212671f * rgb[0] + 0.715160f * rgb[1] + 0.072169f * rgb[2];
    xyz[2] = 0.019334f * rgb[0] + 0.119193f * rgb[1] + 0.950227f * rgb[2];

    return xyz;
  }

  //from PBRT-v3
  template <typename Predicate>
  int FindInterval(int size, const Predicate& pred) {
    int first = 0, len = size;
    while (len > 0) {
      int half = len >> 1, middle = first + half;
      // Bisect range based on value of _pred_ at _middle_
      if (pred(middle)) {
        first = middle + 1;
        len -= half + 1;
      }
      else
        len = half;
    }
    return LiteMath::clamp(first - 1, 0, size - 2);
  }
}

#endif //HYDRAAPI_EX_HREXTENSIONS_SPECTRAL_H
