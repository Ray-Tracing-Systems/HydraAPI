#include "HydraPostProcessCommon.h"
#include "HydraPostProcessSpecial.h"

class ResampleFilter2D : public IFilter2DSpecial
{
public:
  ResampleFilter2D()  = default;
  ~ResampleFilter2D() = default;

  bool Eval(ArgArray1& argsHDR, ArgArray2& argsLDR, pugi::xml_node settings, std::shared_ptr<IHRRenderDriver> a_pDriver) override;
};


bool ResampleFilter2D::Eval(ArgArray1& argsHDR, ArgArray2& argsLDR, pugi::xml_node settings, std::shared_ptr<IHRRenderDriver> a_pDriver)
{
  auto inImagePtr  = argsHDR[L"in_color"];
  auto outImagePtr = argsHDR[L"out_color"];

  if (inImagePtr == nullptr)
  {
    m_err = L"resample: arg 'in_color' not found";
    return false;
  }

  if (outImagePtr == nullptr)
  {
    m_err = L"resample: arg 'out_color' not found";
    return false;
  }

  if (inImagePtr == outImagePtr)
  {
    m_err = L"resample: input must not be equal to output";
    return false;
  }

  if (outImagePtr->width() <= 0 || outImagePtr->height() <= 0)
  {
    m_err = L"resample: output image have invalid size";
    return false;
  }

  inImagePtr->resampleTo(*outImagePtr);

  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MedianFilter2D : public IFilter2DSpecial
{
public:
  MedianFilter2D()  = default;
  ~MedianFilter2D() = default;

  bool Eval(ArgArray1& argsHDR, ArgArray2& argsLDR, pugi::xml_node settings, std::shared_ptr<IHRRenderDriver> a_pDriver) override;
};


bool MedianFilter2D::Eval(ArgArray1& argsHDR, ArgArray2& argsLDR, pugi::xml_node settings, std::shared_ptr<IHRRenderDriver> a_pDriver)
{
  auto inImagePtr  = argsHDR[L"in_color"];
  auto outImagePtr = argsHDR[L"out_color"];

  if (inImagePtr == nullptr)
  {
    m_err = L"median: arg 'in_color' not found";
    return false;
  }

  if (outImagePtr == nullptr)
  {
    m_err = L"median: arg 'out_color' not found";
    return false;
  }

  if (inImagePtr != outImagePtr)
    (*outImagePtr) = (*inImagePtr);

  float threshold = settings.attribute(L"threshold").as_float();

  outImagePtr->medianFilterInPlace(threshold);

  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class MedianFilter2DN : public IFilter2DSpecial
{
public:
  MedianFilter2DN() = default;
  ~MedianFilter2DN() = default;

  bool Eval(ArgArray1& argsHDR, ArgArray2& argsLDR, pugi::xml_node settings, std::shared_ptr<IHRRenderDriver> a_pDriver) override;
};


bool MedianFilter2DN::Eval(ArgArray1& argsHDR, ArgArray2& argsLDR, pugi::xml_node settings, std::shared_ptr<IHRRenderDriver> a_pDriver)
{
  auto inImagePtr  = argsHDR[L"in_color"];
  auto outImagePtr = argsHDR[L"out_color"];

  if (inImagePtr == nullptr)
  {
    m_err = L"median: arg 'in_color' not found";
    return false;
  }

  if (outImagePtr == nullptr)
  {
    m_err = L"median: arg 'out_color' not found";
    return false;
  }

  if (inImagePtr != outImagePtr)
    (*outImagePtr) = (*inImagePtr);

  float threshold = settings.attribute(L"threshold").as_float();
  int pixelsNum   = settings.attribute(L"pixels_num").as_float();

  outImagePtr->medianFilterInPlace(threshold, 1, pixelsNum);

  return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class GaussBlur2D : public IFilter2DSpecial
{
public:
  GaussBlur2D()  = default;
  ~GaussBlur2D() = default;

  bool Eval(ArgArray1& argsHDR, ArgArray2& argsLDR, pugi::xml_node settings, std::shared_ptr<IHRRenderDriver> a_pDriver) override;
};

bool GaussBlur2D::Eval(ArgArray1& argsHDR, ArgArray2& argsLDR, pugi::xml_node settings, std::shared_ptr<IHRRenderDriver> a_pDriver)
{
  auto inImagePtr  = argsHDR[L"in_color"];
  auto outImagePtr = argsHDR[L"out_color"];

  if (inImagePtr == nullptr)
  {
    m_err = L"GaussBlur2D: arg 'in_color' not found";
    return false;
  }

  if (outImagePtr == nullptr)
  {
    m_err = L"GaussBlur2D: arg 'out_color' not found";
    return false;
  }

  if (outImagePtr->width() != inImagePtr->width() || outImagePtr->height() != inImagePtr->height())
    outImagePtr->resize(inImagePtr->width(), inImagePtr->height());

  const float sigma = settings.attribute(L"sigma").as_float();
  const int radius  = settings.attribute(L"radius").as_int();

  if (inImagePtr != outImagePtr)
    (*outImagePtr) = (*inImagePtr);        // #TODO: implement this more effitiently! Don't make a copy!

  outImagePtr->gaussBlur(radius, sigma);   //

  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class NLMDenoiserPut : public IFilter2DSpecial
{
public:
  NLMDenoiserPut()  = default;
  ~NLMDenoiserPut() = default;
  
  bool Eval(ArgArray1& argsHDR, ArgArray2& argsLDR, pugi::xml_node setiings, std::shared_ptr<IHRRenderDriver> a_pDriver) override;
};

using HydraRender::HDRImage4f;

void NonLocalMeansGuidedTexNormDepthFilter(const HDRImage4f& inImage, const HDRImage4f& inTexColor, const HDRImage4f& inNormDepth,
                                           HDRImage4f& outImage, int a_windowRadius, int a_blockRadius, float a_noiseLevel);

#include <iostream>

bool NLMDenoiserPut::Eval(ArgArray1& argsHDR, ArgArray2& argsLDR, pugi::xml_node setiings, std::shared_ptr<IHRRenderDriver> a_pDriver)
{
  auto outImagePtr = argsHDR[L"out_color"];
  
  if (outImagePtr == nullptr)
  {
    m_err = L"median: arg 'out_color' not found";
    return false;
  }
  
  // (1) get color and gbuffer from render driver
  //
  const int w = outImagePtr->width();
  const int h = outImagePtr->height();
  
  HDRImage4f colorImage(w,h);
  HDRImage4f texColor(w,h);
  HDRImage4f normdImage(w,h);
  
  float* colorIn  = colorImage.data();
  float* normdIn  = normdImage.data();
  float* texcolIn = texColor.data();
  
  std::vector<float> colorLine(w*4);
  std::vector<HRGBufferPixel> gbuffLine(w);
  
  const float gammaInv = 1.0f/2.2f;
  const float gamma    = 2.2f;

  #pragma omp parallel for
  for(int j=0;j<h;j++)
  {
    const int offset = j*w*4;
    
    a_pDriver->GetFrameBufferLineHDR(0, w, j, colorLine.data(), L"color");
    a_pDriver->GetGBufferLine(j, gbuffLine.data(), 0, w, std::unordered_set<int32_t>());
  
    for(int i=0;i<w;i++)
    {
      normdIn[offset + i*4 + 0] = gbuffLine[i].norm[0];
      normdIn[offset + i*4 + 1] = gbuffLine[i].norm[1];
      normdIn[offset + i*4 + 2] = gbuffLine[i].norm[2];
      normdIn[offset + i*4 + 3] = gbuffLine[i].depth;
  
      texcolIn[offset + i*4 + 0] = gbuffLine[i].rgba[0];
      texcolIn[offset + i*4 + 1] = gbuffLine[i].rgba[1];
      texcolIn[offset + i*4 + 2] = gbuffLine[i].rgba[2];
      texcolIn[offset + i*4 + 3] = gbuffLine[i].rgba[3];
      
      colorIn[offset + i*4 + 0] = powf(colorLine[i*4 + 0], gammaInv);
      colorIn[offset + i*4 + 1] = powf(colorLine[i*4 + 1], gammaInv);
      colorIn[offset + i*4 + 2] = powf(colorLine[i*4 + 2], gammaInv);
      colorIn[offset + i*4 + 3] = colorLine[i*4 + 3];
    }
  }
  
  // (2) run NLM filter
  {
    HDRImage4f colorImage2(w,h);
    std::cout << "begin NLM" << std::endl;
    NonLocalMeansGuidedTexNormDepthFilter(colorImage, texColor, normdImage,
                                          colorImage2, 5, 1, 0.10f);
    
    colorIn = colorImage2.data();
    std::cout << "end   NLM" << std::endl;
  
    float* outColor = outImagePtr->data();
    float* inColor2 = colorImage2.data();
    #pragma omp parallel for
    for(int j=0;j<h;j++)
    {
      const int offset = j * w * 4;
      for (int i = 0; i < w; i++)
      {
        outColor[offset + i*4 + 0] = powf(inColor2[offset + i*4 + 0], gamma);
        outColor[offset + i*4 + 1] = powf(inColor2[offset + i*4 + 1], gamma);
        outColor[offset + i*4 + 2] = powf(inColor2[offset + i*4 + 2], gamma);
        outColor[offset + i*4 + 3] = inColor2[offset + i*4 + 3];
      }
    }
    
  }
  
  // float* outColor = outImagePtr->data();
  // #pragma omp parallel for
  // for(int j=0;j<h;j++)
  // {
  //   const int offset = j * w * 4;
  //   for (int i = 0; i < w; i++)
  //   {
  //     outColor[offset + i*4 + 0] = normdIn[offset + i*4 + 0];
  //     outColor[offset + i*4 + 1] = normdIn[offset + i*4 + 1];
  //     outColor[offset + i*4 + 2] = normdIn[offset + i*4 + 2];
  //     outColor[offset + i*4 + 3] = normdIn[offset + i*4 + 3];
  //   }
  // }
  
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<IFilter2DSpecial> CreateSpecialFilter(const wchar_t* a_filterName)
{
  std::wstring inName(a_filterName);

  if (inName == L"resample")
    return std::make_shared<ResampleFilter2D>();
  else if (inName == L"median")
    return std::make_shared<MedianFilter2D>();
  else if (inName == L"median_n")
    return std::make_shared<MedianFilter2DN>();
  else if (inName == L"blur" || inName == L"gauss_blur")
    return std::make_shared<GaussBlur2D>();
  else if (inName == L"NLMPut")
    return std::make_shared<NLMDenoiserPut>();
  else
    return nullptr;
}