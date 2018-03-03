#include "PostProcess.h"
#include "hydra_api/pugixml.hpp"
#include "hydra_api/HydraPostProcessCommon.h"

#include "hydra_api/HydraXMLHelpers.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class PostProcessHydra1 : public IFilter2D
{
  void Release() override;   // COM like destructor; 
  bool Eval()    override;   //
};


extern "C" __declspec(dllexport) IFilter2D* CreateFilter(const wchar_t* a_filterName)
{
  std::wstring inFilterName(a_filterName);

  if (inFilterName == L"post_process_hydra1")
    return new PostProcessHydra1;
  else
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PostProcessHydra1::Release()
{
  // nothing to destroy here, we have simple implementation
}

void ExecutePostProcessHydra1(
  const float* a_input, float* a_output, unsigned int a_numThreads,
  const float a_exposure, const float a_compress, const float a_contrast,
  const float a_saturation, const float a_whiteBalance, float3 a_whitePointColor,
  const float a_uniformContrast, const float a_normalize, const float a_vignette,
  const float a_chromAberr, const float a_sharpness, const float a_sizeStar,
  const int a_numRay, const int a_rotateRay, const float a_randomAngle,
  const float a_sprayRay, unsigned int a_width, unsigned int a_height); // forward declaraion

bool PostProcessHydra1::Eval()
{
  // read presets
  //
  pugi::xml_document docXml;
  docXml.load_string(GetSettingsStr());
  pugi::xml_node settings = docXml.child(L"settings");

  // read input and output arguments
  //
  unsigned int numThreads = settings.attribute(L"numThreads").as_int();

  float exposure = settings.attribute(L"exposure").as_float();
  float compress = settings.attribute(L"compress").as_float();
  float contrast = settings.attribute(L"contrast").as_float();
  float saturation = settings.attribute(L"saturation").as_float();
  float whiteBalance = settings.attribute(L"whiteBalance").as_float();
  float3 whitePointColor = HydraXMLHelpers::ReadFloat3(settings.attribute(L"whitePointColor"));
  float uniformContrast = settings.attribute(L"uniformContrast").as_float();
  float normalize = settings.attribute(L"normalize").as_float();
  float vignette = settings.attribute(L"vignette").as_float();
  float chromAberr = settings.attribute(L"chromAberr").as_float();
  float sharpness = settings.attribute(L"sharpness").as_float();

  // Diffraction stars
  float sizeStar    = settings.attribute(L"diffStars_sizeStar").as_float(); // 0-100
  int   numRay      = settings.attribute(L"diffStars_numRay").as_int();  // 0-16
  int   rotateRay   = settings.attribute(L"diffStars_rotateRay").as_int();  // 0-360
  float randomAngle = settings.attribute(L"diffStars_randomAngle").as_float();   // 0-1
  float sprayRay    = settings.attribute(L"diffStars_sprayRay").as_float();   // 0-1 

  int w1, h1, bpp1;
  int w2, h2, bpp2;
  const float* input = GetInputByName(L"in_color", &w1, &h1, &bpp1);
  float* output = GetOutputByName(L"out_color", &w2, &h2, &bpp2);


  // check we have correct in and out arguments
  //
  if (numThreads < 0 || exposure < 0.0f || compress < 0.0f || contrast < 0.0f ||
    saturation < 0.0f || whiteBalance < 0.0f || whitePointColor.x < 0.0f || 
    whitePointColor.y < 0.0f || whitePointColor.z < 0.0f || uniformContrast < 0.0f || 
    normalize < 0.0f || vignette < 0.0f || chromAberr < 0.0f || sharpness < 0.0f ||
    sizeStar < 0.0f || numRay < 0.0f || rotateRay < 0.0f || randomAngle < 0.0f || 
    sprayRay < 0.0f)
  {
    wcsncpy_s(m_msg, L"post_process_hydra1; Arguments must be greater than zero.", ERR_MSG_SIZE);
    return false;
  }

  if (exposure > 4.0f)
  {
    wcsncpy_s(m_msg, L"post_process_hydra1; exposure should be in the range 0 - 4. Default 1. This will be limited to 4.", ERR_MSG_SIZE);
    exposure = 4.0f;
  }
  if (compress > 1.0f)
  {
    wcsncpy_s(m_msg, L"post_process_hydra1; compress should be in the range 0 - 1. Default 0. This will be limited to 1.", ERR_MSG_SIZE);
    compress = 1.0f;
  }
  if (contrast < 1.0f)
  {
    wcsncpy_s(m_msg, L"post_process_hydra1; contrast should be in the range 1 - 2. Default 1. This will be limited to 1.", ERR_MSG_SIZE);
    contrast = 1.0f;
  }
  else if (contrast > 2.0f)
  {
    wcsncpy_s(m_msg, L"post_process_hydra1; contrast should be in the range 1 - 2. Default 1. This will be limited to 2.", ERR_MSG_SIZE);
    contrast = 2.0f;
  }
  if (saturation > 2.0f)
  {
    wcsncpy_s(m_msg, L"post_process_hydra1; saturation should be in the range 0 - 2. Default 1. This will be limited to 2.", ERR_MSG_SIZE);
    saturation = 2.0f;
  }
  if (whiteBalance > 1.0f)
  {
    wcsncpy_s(m_msg, L"post_process_hydra1; whiteBalance should be in the range 0 - 1. Default 0. This will be limited to 1.", ERR_MSG_SIZE);
    whiteBalance = 1.0f;
  }
  if (uniformContrast > 1.0f)
  {
    wcsncpy_s(m_msg, L"post_process_hydra1; uniformContrast should be in the range 0 - 1. Default 0. This will be limited to 1.", ERR_MSG_SIZE);
    uniformContrast = 1.0f;
  }
  if (normalize > 1.0f)
  {
    wcsncpy_s(m_msg, L"post_process_hydra1; normalize should be in the range 0 - 1. Default 0. This will be limited to 1.", ERR_MSG_SIZE);
    normalize = 1.0f;
  }
  if (vignette > 1.0f)
  {
    wcsncpy_s(m_msg, L"post_process_hydra1; vignette should be in the range 0 - 1. Default 0. This will be limited to 1.", ERR_MSG_SIZE);
    vignette = 1.0f;
  }
  if (chromAberr > 1.0f)
  {
    wcsncpy_s(m_msg, L"post_process_hydra1; chromAberr should be in the range 0 - 1. This will be limited to 1.", ERR_MSG_SIZE);
    chromAberr = 1.0f;
  }
  if (sharpness > 1.0f)
  {
    wcsncpy_s(m_msg, L"post_process_hydra1; sharpness should be in the range 0 - 1. This will be limited to 1.", ERR_MSG_SIZE);
    sharpness = 1.0f;
  }
  if (sizeStar > 100.0f)
  {
    wcsncpy_s(m_msg, L"post_process_hydra1; sizeStar should be in the range 0 - 100. This will be limited to 100.", ERR_MSG_SIZE);
    sizeStar = 100.0f;
  }
  if (numRay > 16)
  {
    wcsncpy_s(m_msg, L"post_process_hydra1; numRay should be in the range 0 - 16. This will be limited to 16.", ERR_MSG_SIZE);
    numRay = 16;
  }
  if (rotateRay > 360)
  {
    wcsncpy_s(m_msg, L"post_process_hydra1; rotateRay should be in the range 0 - 360. This will be limited to 360.", ERR_MSG_SIZE);
    rotateRay = 360;
  }
  if (randomAngle > 1.0f)
  {
    wcsncpy_s(m_msg, L"post_process_hydra1; randomAngle should be in the range 0 - 1. This will be limited to 1.", ERR_MSG_SIZE);
    randomAngle = 1.0f;
  }
  if (sprayRay > 1.0f)
  {
    wcsncpy_s(m_msg, L"post_process_hydra1; sprayRay should be in the range 0 - 1. This will be limited to 1.", ERR_MSG_SIZE);
    sprayRay = 1.0f;
  }

  if (w1 != w2 || h1 != h2 || w1 <= 0 || h1 <= 0)
  {
    wcsncpy_s(m_msg, L"post_process_hydra1; bad input size", ERR_MSG_SIZE);
    return false;
  }
  if (bpp1 != bpp1 || bpp1 != 16)
  {
    wcsncpy_s(m_msg, L"post_process_hydra1; ivalid image format; both images must be HDR;", ERR_MSG_SIZE);
    return false;
  }
  if (input == nullptr)
  {
    wcsncpy_s(m_msg, L"post_process_hydra1; argument not found: 'in_color' ", ERR_MSG_SIZE);
    return false;
  }
  if (output == nullptr)
  {
    wcsncpy_s(m_msg, L"post_process_hydra1; argument not found: 'out_color' ", ERR_MSG_SIZE);
    return false;
  }

  // now run our internal implementation
  //

  ExecutePostProcessHydra1(
    input, output, numThreads, exposure, compress, contrast, saturation, whiteBalance,
    whitePointColor, uniformContrast, normalize, vignette, chromAberr, sharpness,
    sizeStar, numRay, rotateRay, randomAngle, sprayRay, w1, h1);

  return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ExecutePostProcessHydra1(
  const float* a_input, float* a_output, unsigned int a_numThreads,
  const float a_exposure, const float a_compress, const float a_contrast,
  const float a_saturation, const float a_whiteBalance, float3 a_whitePointColor,
  const float a_uniformContrast, const float a_normalize, const float a_vignette,
  const float a_chromAberr, const float a_sharpness, const float a_sizeStar,
  const int a_numRay, const int a_rotateRay, const float a_randomAngle, 
  const float a_sprayRay, unsigned int a_width, unsigned int a_height)       
{ 
  // Set the desired number of threads
  const unsigned int maxThreads = omp_get_num_procs();
  if (a_numThreads > maxThreads) a_numThreads = maxThreads;
  else if (a_numThreads <= 0)    a_numThreads = maxThreads;
  omp_set_num_threads(a_numThreads);

  float4* image4in = (float4*)a_input;
  float4* image4out = (float4*)a_output;


  // variables and constants
  const int sizeImage = a_width * a_height;
  const float centerImageX = a_width / 2.0f;
  const float centerImageY = a_height / 2.0f;
  const float diagonalImage = Distance(0, 0, (float)a_width, (float)a_height);
  const float radiusImage = diagonalImage / 2.0f;

  const int histogramBin = 10000;
  const float iterrHistogramBin = 1.0f;

  bool autoWhiteBalance = true;
  if (a_whitePointColor.x > 0.0f || a_whitePointColor.y > 0.0f || a_whitePointColor.z > 0.0f)
  {
    autoWhiteBalance = false;
  }
  float3 summRgb = { 0.0f, 0.0f, 0.0f };
  float3 summRgbColorPass = { 0.0f, 0.0f, 0.0f };
  float3 whitePointColorPass = { 1.0f, 1.0f, 1.0f };
  float3 d65 = { 0.9505f , 1.0f, 1.0888f }; // in XYZ

  float  coefOffsetHue = 0.0f;
  float  minRgbSource = 10000.0f;
  float  maxRgbSource = 0.0f;
  float  minRgbFinish = 10000.0f;
  float  maxRgbFinish = 0.0f;
  float  minRgbSourceBlur = 10000.0f;
  float  maxRgbSourceBlur = 0.0f;

  float  minAllHistBin = 0.0f;
  float  maxAllHistBin = 1.0f;
  float  minHistRbin = 0.0f;
  float  minHistGbin = 0.0f;
  float  minHistBbin = 0.0f;
  float  maxHistRbin = histogramBin;
  float  maxHistGbin = histogramBin;
  float  maxHistBbin = histogramBin;

  // arrays
  float* bigBlurPass = NULL;
  float* chromAbberR = NULL;
  float* chromAbberG = NULL;
  float* lumForSharp = NULL;
  float* diffrStarsR = NULL;
  float* diffrStarsG = NULL;
  float* diffrStarsB = NULL;

  tagHistogram histogram;
  histogram.r = NULL;
  histogram.g = NULL;
  histogram.b = NULL;

  if (a_chromAberr > 0.0f)
  {
    chromAbberR = (float*)calloc(sizeImage, sizeof(float));
    chromAbberG = (float*)calloc(sizeImage, sizeof(float));
  }
  if (a_normalize > 0.0f)
  {
    histogram.r = (float*)calloc(histogramBin, sizeof(float));
    histogram.g = (float*)calloc(histogramBin, sizeof(float));
    histogram.b = (float*)calloc(histogramBin, sizeof(float));
  }
  if (a_sharpness > 0.0f)
  {
    lumForSharp = (float*)calloc(sizeImage, sizeof(float));
  }
  if (a_sizeStar > 0.0f)
  {
    diffrStarsR = (float*)calloc(sizeImage, sizeof(float));
    diffrStarsG = (float*)calloc(sizeImage, sizeof(float));
    diffrStarsB = (float*)calloc(sizeImage, sizeof(float));
  }

  //////////////////////////////////////////////////////////////////////////////////////


  // ----- Analization, precalculate and any effects. Loop 1. -----
  //#pragma omp parallel for
  for (unsigned int y = 0; y < a_height; ++y)
  {
    for (unsigned int x = 0; x < a_width; ++x)
    {
      const unsigned int i = y * a_width + x;

      image4out[i] = image4in[i];  //uncomment in 3dmax plugin

      // ----- Sharpness -----
      if (a_sharpness > 0.0f)
      {
        lumForSharp[i] = (image4out[i].x + image4out[i].y + image4out[i].z) / 3.0f;
        lumForSharp[i] /= (1.0f + lumForSharp[i]);
      }

      // ----- Exposure -----
      if (a_exposure != 1.0f)
      {
        image4out[i].x *= a_exposure;
        image4out[i].y *= a_exposure;
        image4out[i].z *= a_exposure;
      }

      // Max & Min value and clamp minus zero
      MinMaxRgb(image4out, minRgbSource, maxRgbSource, 0.0f, i);

      // Collect all the brightness values by channel.
      if (a_whiteBalance > 0.0f)
      {
        if (autoWhiteBalance && image4out[i].x < 1.0f && image4out[i].y < 1.0f && image4out[i].z < 1.0f)
        {
          SummValueOnField(image4out, &summRgb, i);
        }
      }

      // ----- Vignette -----
      if (a_vignette > 0.0f)
      {
        Vignette(image4out, a_width, a_height, a_vignette, diagonalImage, centerImageX,
          centerImageY, radiusImage, x, y, i);
      }

      // ----- Difraction stars -----
      if (a_sizeStar > 0.0f && image4out[i].x > 50.0f || image4out[i].y > 50.0f || image4out[i].z > 50.0f)
      {
        DiffractionStars(image4out, diffrStarsR, diffrStarsG, diffrStarsB, a_sizeStar,
          a_numRay, a_rotateRay, a_randomAngle, a_sprayRay, a_width, a_height, sizeImage, radiusImage,
          x, y, i);
      }

      // ----- Chromatic aberration -----
      if (a_chromAberr > 0.0f)
      {
        ChrommAberr(image4out, chromAbberR, chromAbberG, a_width, a_height, sizeImage, a_chromAberr, x, y, i);
      }
    }
  }

  // ----- Chromatic aberration -----
  if (a_chromAberr > 0.0f)
  {
    #pragma omp parallel for
    for (int i = 0; i < sizeImage; ++i)
    {
      image4out[i].x = chromAbberR[i];
      image4out[i].y = chromAbberG[i];
    }
  }

  // ----- Sharpness -----
  if (a_sharpness > 0.0f)
  {
    #pragma omp parallel for
    for (int y = 0; y < (int)a_height; ++y)
    {
      for (int x = 0; x < (int)a_width; ++x)
      {
        const unsigned int i = y * a_width + x;

        float mean = 0.0f;

        for (int i = -1; i <= 1; ++i)
        {
          for (int j = -1; j <= 1; ++j)
          {
            int Y = y + i;
            int X = x + j;

            if      (Y < 0)         Y = 1;
            else if (Y >= (int)a_height) Y = a_height - 1;
            if      (X < 0)         X = 1;
            else if (X >= (int)a_width)  X = a_width - 1;

            mean += lumForSharp[Y * a_width + X];
          }
        }
        mean /= 9.0f;

        float dispers = 0.0f;
        for (int i = -1; i <= 1; i++)
        {
          for (int j = -1; j <= 1; j++)
          {
            int Y = y + i;
            int X = x + j;

            if      (Y < 0)         Y = 1;
            else if (Y >= (int)a_height) Y = a_height - 1;
            if      (X < 0)         X = 1;
            else if (X >= (int)a_width)  X = a_width - 1;

            float a = lumForSharp[Y * a_width + X] - mean;
            dispers += abs(a);
          }
        }

        dispers /= (1.0f + dispers);
        const float lumCompr = lumForSharp[i] / (1.0f + lumForSharp[i]);
        const float meanCompr = mean / (1.0f + mean);
        float hiPass = (lumForSharp[i] - mean) * 5.0f + 1.0f;
        hiPass = pow(hiPass, 2);

        float sharp = 1.0f;
        Blend(sharp, hiPass, a_sharpness * (1.0f - dispers));

        image4out[i].x *= sharp;
        image4out[i].y *= sharp;
        image4out[i].z *= sharp;
      }
    }
  }

  // ----- White point for white balance -----
  if (a_whiteBalance > 0.0f)
  {
    if (autoWhiteBalance)
    {
      ComputeWhitePoint(summRgb, &a_whitePointColor, sizeImage);
    }

    ConvertSrgbToXyz(&a_whitePointColor);
    MatrixCat02(&a_whitePointColor);
    MatrixCat02(&d65);

    const float lum = Luminance(&a_whitePointColor);

    if (lum > 0.0f)
    {
      a_whitePointColor.x /= lum;
      a_whitePointColor.y /= lum;
      a_whitePointColor.z /= lum;
    }
  }



  // ---------- Many filters Loop 2. ----------
  #pragma omp parallel for
  for (int i = 0; i < sizeImage; ++i)
  {
    // ----- Diffraction stars -----
    if (a_sizeStar > 0.0f)
    {
      image4out[i].x += diffrStarsR[i];
      image4out[i].y += diffrStarsG[i];
      image4out[i].z += diffrStarsB[i];
    }

    // ----- White balance -----
    if (a_whiteBalance > 0.0f)
    {
      ConvertSrgbToXyz(&image4out[i]);
      ChromAdaptIcam(&image4out[i], a_whitePointColor, d65, a_whiteBalance);
      ConvertXyzToSrgb(&image4out[i]);
    }

    // ----- Compress -----
    if (a_compress > 0.0f && maxRgbSource > 1.01f)
    {
      float4 rgbDataComp = image4out[i];

      float knee = 10.0f;
      Blend(knee, 1.0f, pow(a_compress, 0.175f)); // lower = softer
      const float antiKnee = 1.0f / knee;

      // Small value "a_compress" start in RGB
      if (a_compress < 1.0f)
      {
        rgbDataComp.x /= pow((1.0f + pow(rgbDataComp.x, knee)), antiKnee);
        rgbDataComp.y /= pow((1.0f + pow(rgbDataComp.y, knee)), antiKnee);
        rgbDataComp.z /= pow((1.0f + pow(rgbDataComp.z, knee)), antiKnee);
      }

      // Compress in RGB, but result equal compress in LMS or IPT
      const float lum = image4out[i].x + image4out[i].y + image4out[i].z;
      const float lumMean = pow(lum, 2.4f) + 1.0f;
      const float mult = powf((lum / 3.0f + 1.0f) / lumMean, 0.4166f);

      image4out[i].x *= mult;
      image4out[i].y *= mult;
      image4out[i].z *= mult;

      // Return to main array
      const float mix = 1.0f - a_compress;
      Blend(image4out[i].x, rgbDataComp.x, mix);
      Blend(image4out[i].y, rgbDataComp.y, mix);
      Blend(image4out[i].z, rgbDataComp.z, mix);
    }

    // ----- Saturation  -----
    if (a_saturation != 1.0f)
    {
      const float lum = Luminance(&image4out[i]);

      // Return to main array
      image4out[i].x = lum + (image4out[i].x - lum) * a_saturation;
      image4out[i].y = lum + (image4out[i].y - lum) * a_saturation;
      image4out[i].z = lum + (image4out[i].z - lum) * a_saturation;

      ClampMinusToZero(image4out, i);
    }

    // Addition little compress
    if (a_compress > 0.0f && maxRgbSource > 1.01f)
    {
      float4 rgbData = image4out[i];

      rgbData.x = pow(rgbData.x, 4.0f);
      rgbData.y = pow(rgbData.y, 4.0f);
      rgbData.z = pow(rgbData.z, 4.0f);

      rgbData.x /= (1.0f + rgbData.x);
      rgbData.y /= (1.0f + rgbData.y);
      rgbData.z /= (1.0f + rgbData.z);

      rgbData.x = pow(rgbData.x, 0.25f);
      rgbData.y = pow(rgbData.y, 0.25f);
      rgbData.z = pow(rgbData.z, 0.25f);

      // Return to main array
      Blend(image4out[i].x, rgbData.x, a_compress);
      Blend(image4out[i].y, rgbData.y, a_compress);
      Blend(image4out[i].z, rgbData.z, a_compress);
    }

    //// ----- Contrast -----
    if (a_contrast > 1.0f)
    {
      float4 contrastData = image4out[i];

      contrastData.x = pow(contrastData.x, 0.4545f);
      contrastData.y = pow(contrastData.y, 0.4545f);
      contrastData.z = pow(contrastData.z, 0.4545f);
      ContrastField(contrastData.x);
      ContrastField(contrastData.y);
      ContrastField(contrastData.z);
      contrastData.x = pow(contrastData.x, 2.2f);
      contrastData.y = pow(contrastData.y, 2.2f);
      contrastData.z = pow(contrastData.z, 2.2f);

      // Return to main array
      const float mix = a_contrast - 1.0f;
      Blend(image4out[i].x, contrastData.x, mix);
      Blend(image4out[i].y, contrastData.y, mix);
      Blend(image4out[i].z, contrastData.z, mix);
    }
  }
  

  // ----- Uniform contrast. Loop 3 and 4. -----
  if (a_uniformContrast > 0.0f)
  {
    std::vector<float> rgbArray(sizeImage * 3);

    // Convert 3 field RGB to linear array.
    #pragma omp parallel for
    for (int i = 0; i < sizeImage; ++i)
    {
      rgbArray[i] = image4out[i].x;
      rgbArray[sizeImage + i] = image4out[i].y;
      rgbArray[sizeImage * 2 + i] = image4out[i].z;
    }

    UniformContrastRgb(&rgbArray[0], sizeImage, histogramBin);

    // Return to main array
    #pragma omp parallel for
    for (int i = 0; i < sizeImage; ++i)
    {
      Blend(image4out[i].x, rgbArray[i], a_uniformContrast);
      Blend(image4out[i].y, rgbArray[i + sizeImage], a_uniformContrast);
      Blend(image4out[i].z, rgbArray[i + sizeImage * 2], a_uniformContrast);
    }
  }

  // ----- Calculate histogram for normalize. Loop 5 and 6. -----
  if (a_normalize > 0.0f)
  {
    #pragma omp parallel for
    for (int i = 0; i < sizeImage; ++i)
    {
      CalculateHistogram(image4out[i].x, &histogram.r[0], histogramBin, iterrHistogramBin);
      CalculateHistogram(image4out[i].y, &histogram.g[0], histogramBin, iterrHistogramBin);
      CalculateHistogram(image4out[i].z, &histogram.b[0], histogramBin, iterrHistogramBin);
    }

    // Calculate min/max histogram for a_normalize
    MinMaxHistBin(&histogram.r[0], minHistRbin, maxHistRbin, sizeImage, histogramBin);
    MinMaxHistBin(&histogram.g[0], minHistGbin, maxHistGbin, sizeImage, histogramBin);
    MinMaxHistBin(&histogram.b[0], minHistBbin, maxHistBbin, sizeImage, histogramBin);

    minAllHistBin = Min3(minHistRbin, minHistGbin, minHistBbin);
    maxAllHistBin = Max3(maxHistRbin, maxHistGbin, maxHistBbin);

    // Normalize 
    #pragma omp parallel for
    for (int i = 0; i < sizeImage; ++i)
    {
      float3 dataNorm;
      dataNorm.x = image4out[i].x;
      dataNorm.y = image4out[i].y;
      dataNorm.z = image4out[i].z;

      Normalize(dataNorm.x, minAllHistBin, maxAllHistBin, 0.0f, 1.0f);
      Normalize(dataNorm.y, minAllHistBin, maxAllHistBin, 0.0f, 1.0f);
      Normalize(dataNorm.z, minAllHistBin, maxAllHistBin, 0.0f, 1.0f);

      Blend(image4out[i].x, dataNorm.x, a_normalize);
      Blend(image4out[i].y, dataNorm.y, a_normalize);
      Blend(image4out[i].z, dataNorm.z, a_normalize);
    }
  }

  //********************* End filters *********************
  if (a_chromAberr > 0.0f)
  {
    free(chromAbberR);
    free(chromAbberG);
    chromAbberR = NULL;
    chromAbberG = NULL;
  }
  if (a_sharpness > 0.0f)
  {
    free(lumForSharp);
    lumForSharp = NULL;
  }
  if (a_sizeStar > 0.0f)
  {
    free(diffrStarsR);
    free(diffrStarsG);
    free(diffrStarsB);
    diffrStarsR = NULL;
    diffrStarsG = NULL;
    diffrStarsB = NULL;
  }
  if (a_normalize > 0.0f)
  {
    free(histogram.r);
    free(histogram.g);
    free(histogram.b);

    histogram.r = NULL;
    histogram.g = NULL;
    histogram.b = NULL;
  }
}




