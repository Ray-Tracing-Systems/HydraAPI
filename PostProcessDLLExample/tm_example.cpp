#include "tm_example.h"
#include "../hydra_api/pugixml.hpp"
#include "../hydra_api/HydraPostProcessCommon.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class ToneMappingExample : public IFilter2D
{
  void Release() override;   // COM like destructor; 
  bool Eval()    override;   //
};


extern "C" __declspec(dllexport) IFilter2D* CreateFilter(const wchar_t* a_filterName)
{
  std::wstring inFilterName(a_filterName);

  if (inFilterName == L"tonemapping_obsolette")
    return new ToneMappingExample;
  else
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ToneMappingExample::Release()
{
  // nothing to destroy here, we have simple implementation
}

void ExecuteToneMappingExample(const float* p_input, float* p_output, CHDRData *pData, unsigned int width, unsigned int height); // forward declaraion

bool ToneMappingExample::Eval()
{
  // read presets
  //
  pugi::xml_document docXml;
  docXml.load_string(GetSettingsStr());
  pugi::xml_node settings = docXml.child(L"settings");

  float kLow     = settings.attribute(L"low").as_float();
  float kHigh    = settings.attribute(L"high").as_float();
  float exposure = settings.attribute(L"exposure").as_float();
  float defog    = settings.attribute(L"defog").as_float();

  // read input and output arguments
  //
  int w1, h1, bpp1;
  int w2, h2, bpp2;
  const float* input = GetInputByName (L"in_color",  &w1, &h1, &bpp1);
  float* output      = GetOutputByName(L"out_color", &w2, &h2, &bpp2);

  // check we have correct in and out arguments
  //
  if (w1 != w2 || h1 != h2 || w1 <= 0 || h1 <= 0)
  {
    wcsncpy_s(m_msg, L"ToneMappingExample; bad input size", ERR_MSG_SIZE);
    return false;
  }

  if (bpp1 != bpp1 || bpp1 != 16)
  {
    wcsncpy_s(m_msg, L"ToneMappingExample; ivalid image format; both images must be HDR;", ERR_MSG_SIZE);
    return false;
  }

  if(input == nullptr)
  { 
    wcsncpy_s(m_msg, L"ToneMappingExample; argument not found: 'in_color' ", ERR_MSG_SIZE);
    return false;
  }

  if (output == nullptr)
  {
    wcsncpy_s(m_msg, L"ToneMappingExample; argument not found: 'out_color' ", ERR_MSG_SIZE);
    return false;
  }

  // now run our internal implementation
  //
  CHDRData presetsData = calcPresets(kLow, kHigh, exposure, defog);
  ExecuteToneMappingExample(input, output, &presetsData, w1, h1);

  return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ExecuteToneMappingExample(const float* p_input, float* p_output, CHDRData *pData, unsigned int width, unsigned int height)
{
  #pragma omp parallel for
  for (unsigned int j = 0; j < height; j++)
    EvaluateRaw(p_input, p_output, pData, width, j);
}


void EvaluateRaw(const float* inputArray, float* outputArray, CHDRData *pData, int arrayWidth, int iRow)
{
  const __m128 fPowKLow     = _mm_set_ps1(pData->fPowKLow);
  const __m128 fFStops      = _mm_set_ps1(pData->fFStops);
  const __m128 fFStopsInv   = _mm_set_ps1(pData->fFStopsInv);
  const __m128 fPowExposure = _mm_set_ps1(pData->fPowExposure);
  const __m128 fDefog       = _mm_set_ps1(pData->fDefog);
  const __m128 fGamma       = _mm_set_ps1(pData->fGamma);
  const __m128 fPowGamma    = _mm_set_ps1(pData->fPowGamma);

  // and define method constants.
  const __m128 fOne      = _mm_set_ps1(1.0f);
  const __m128 fZerro    = _mm_set_ps1(0.0f);
  const __m128 fSaturate = _mm_set_ps1(1.0f);

  __m128 fAlphaMask, fColorMask;

  fAlphaMask.m128_u32[0] = 0x00000000;
  fAlphaMask.m128_u32[1] = 0x00000000;
  fAlphaMask.m128_u32[2] = 0x00000000;
  fAlphaMask.m128_u32[3] = 0xFFFFFFFF;

  fColorMask.m128_u32[0] = 0xFFFFFFFF;
  fColorMask.m128_u32[1] = 0xFFFFFFFF;
  fColorMask.m128_u32[2] = 0xFFFFFFFF;
  fColorMask.m128_u32[3] = 0x00000000;

  for (int iCol = 0; iCol < arrayWidth; iCol++)
  {
    __m128 fColor = _mm_setzero_ps();
    fColor = _mm_load_ps((float*)((char*)inputArray + iRow*arrayWidth * 4 * sizeof(float) + iCol * 4 * sizeof(float)));

    __m128 fAlpha = _mm_and_ps(fColor, fAlphaMask);

    // Defog
    fColor = _mm_sub_ps(fColor, fDefog);
    fColor = _mm_max_ps(fZerro, fColor);

    // Multiply color by pow( 2.0f, exposure +  2.47393f )
    fColor = _mm_mul_ps(fColor, fPowExposure);

    // Apply a knee function (Please, refer to the OpenEXR algorithm).
    __m128 fCmpFlag = _mm_cmpge_ps(fColor, fPowKLow);

    if ((_mm_movemask_ps(fCmpFlag) & 7) != 0)
    {
      __m128 fTmpPixel = _mm_sub_ps(fColor, fPowKLow);
      fTmpPixel = _mm_mul_ps(fTmpPixel, fFStops);
      fTmpPixel = _mm_add_ps(fTmpPixel, fOne);

      // fTmpPixel = logf( fTmpPixel);
      ((float*)&fTmpPixel)[0] = logf(((float*)&fTmpPixel)[0]);
      ((float*)&fTmpPixel)[1] = logf(((float*)&fTmpPixel)[1]);
      ((float*)&fTmpPixel)[2] = logf(((float*)&fTmpPixel)[2]);
      ((float*)&fTmpPixel)[3] = logf(((float*)&fTmpPixel)[3]);

      fTmpPixel = _mm_mul_ps(fTmpPixel, fFStopsInv);
      fColor = _mm_add_ps(fTmpPixel, fPowKLow);
    }

    // Scale the values
    fColor = _mm_mul_ps(fColor, fSaturate);
    fColor = _mm_mul_ps(fColor, fPowGamma);

    // Saturate
    fColor = _mm_max_ps(fColor, _mm_setzero_ps());
    fColor = _mm_min_ps(fColor, fSaturate);

    fColor = _mm_or_ps(_mm_and_ps(fColor, fColorMask), fAlpha); // restore original alpha

    // Store result pixel
    _mm_store_ps((float*)((char*)outputArray + iRow*arrayWidth * 4 * sizeof(float) + iCol * 4 * sizeof(float)), fColor);
  }

}

// calculate FStops value parameter from the arguments
float resetFStopsParameter(float powKLow, float kHigh)
{
  float curveBoxWidth  = powf(2.0f, kHigh) - powKLow;
  float curveBoxHeight = powf(2.0f, 3.5f) - powKLow;

  // Initial boundary values
  float fFStopsLow = 0.0f;
  float fFStopsHigh = 100.0f;
  int iterations = 23; //interval bisection iterations

                       // Interval bisection to find the final knee function fStops parameter
  for (int i = 0; i < iterations; i++)
  {
    float fFStopsMiddle = (fFStopsLow + fFStopsHigh) * 0.5f;
    if ((curveBoxWidth * fFStopsMiddle + 1.0f) < exp(curveBoxHeight * fFStopsMiddle))
    {
      fFStopsHigh = fFStopsMiddle;
    }
    else
    {
      fFStopsLow = fFStopsMiddle;
    }
  }

  return (fFStopsLow + fFStopsHigh) * 0.5f;
}

CHDRData calcPresets(float kLow, float kHigh, float exposure, float defog)
{
  float gamma = 1.0f;

  // fill HDR parameters structure
  CHDRData HDRData;
  HDRData.fGamma    = gamma;
  HDRData.fPowGamma = powf(2.0f, -3.5f*gamma);
  HDRData.fDefog    = defog;

  HDRData.fPowKLow     = powf(2.0f, kLow);
  HDRData.fPowKHigh    = powf(2.0f, kHigh);
  HDRData.fPow35       = powf(2.0f, 3.5f);
  HDRData.fPowExposure = powf(2.0f, exposure + 2.47393f);

  // calculate FStops
  HDRData.fFStops = resetFStopsParameter(HDRData.fPowKLow, kHigh);
  //printf("resetFStopsParameter result = %f\n", HDRData.fFStops);

  HDRData.fFStopsInv = 1.0f / HDRData.fFStops;

  return HDRData;
}

