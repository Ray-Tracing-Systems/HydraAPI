#include "HydraPostProcessCommon.h"
#include "HydraPostProcessSpecial.h"

class ResampleFilter2D : public IFilter2DSpecial
{
public:
  ResampleFilter2D() {}
  ~ResampleFilter2D() {}

  bool Eval(ArgArray1& argsHDR, ArgArray2& argsLDR, pugi::xml_node setiings) override;
};


bool ResampleFilter2D::Eval(ArgArray1& argsHDR, ArgArray2& argsLDR, pugi::xml_node settings)
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
  MedianFilter2D() {}
  ~MedianFilter2D() {}

  bool Eval(ArgArray1& argsHDR, ArgArray2& argsLDR, pugi::xml_node setiings) override;
};


bool MedianFilter2D::Eval(ArgArray1& argsHDR, ArgArray2& argsLDR, pugi::xml_node settings)
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

  if (outImagePtr->width() != inImagePtr->width() || outImagePtr->height() != inImagePtr->height())
  {
    m_err = L"median: input and uputut image size not equal";
    return false;
  }

  float threshold = settings.attribute(L"threshold").as_float();

  if (inImagePtr != outImagePtr)
    (*outImagePtr) = (*inImagePtr);

  outImagePtr->medianFilterInPlace(100.0f*threshold, 0.0f); // last parameter is some for MLT. 0 tells that we don't have to change threshold depend on average brightness

  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<IFilter2DSpecial> CreateSpecialFilter(const wchar_t* a_filterName)
{
  std::wstring inName(a_filterName);

  if (inName == L"resample")
    return std::make_shared<ResampleFilter2D>();
  else if (inName == L"median")
    return std::make_shared<MedianFilter2D>();
  else
    return nullptr;
}