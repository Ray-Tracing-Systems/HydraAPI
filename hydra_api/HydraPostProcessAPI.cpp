#include "HydraPostProcessAPI.h"
#include "HydraPostProcessCommon.h"
#include "HydraPostProcessSpecial.h"
#include "HydraObjectManager.h"
#include "HR_HDRImageTool.h"

#include <fstream>
#include <sstream>

struct FrameBufferImage
{
  FrameBufferImage() { }
  FrameBufferImage(const wchar_t* a_name, HDRImage4f* a_hdrImg) { name = a_name; pHDRImage = std::shared_ptr<HDRImage4f>(a_hdrImg); pLDRImage = nullptr; }
  FrameBufferImage(const wchar_t* a_name, LDRImage1i* a_ldrImg) { name = a_name; pLDRImage = std::shared_ptr<LDRImage1i>(a_ldrImg); pHDRImage = nullptr; }

  std::shared_ptr<HDRImage4f> pHDRImage;  ///< only one of (pHDRImage, pLDRImage) may be not null in the same time !
  std::shared_ptr<LDRImage1i> pLDRImage;  ///< only one of (pHDRImage, pLDRImage) may be not null in the same time !
  std::wstring                name;
};

static std::unordered_map<std::wstring, std::shared_ptr<IFilter2D> >        g_commonFilters;
static std::unordered_map<std::wstring, std::shared_ptr<IFilter2DSpecial> > g_spetialFilters;
static std::vector<FrameBufferImage> g_fbImages;

#ifdef WIN32
static HMODULE g_dllFilterHangle = NULL;
PCREATEFUN_T  g_createFunc = nullptr;

static HMODULE g_dllFilterHangle2 = NULL;
PCREATEFUN_T  g_createFunc2 = nullptr;

#endif

extern HRObjectManager g_objManager;

void _hrDestroyPostProcess()
{
  g_fbImages.clear();
  g_spetialFilters.clear();
  g_commonFilters.clear();
}

void _hrInitPostProcess()
{
  // clwar previous resources
  //
  _hrDestroyPostProcess();

  // do init here
  //
  std::wstring filtersSpecial[] = { L"resample", L"median" };

  for(auto name : filtersSpecial)
    g_spetialFilters[name.c_str()] = CreateSpecialFilter(name.c_str());

  // load dll's
  //

#ifdef WIN32

  // add filters from example DLL 
  //
  {
    g_dllFilterHangle = LoadLibraryW(L"D:/PROG/HydraAPI/hydra_api/x64/Release/PostProcessDLLExample.dll");

    if (g_dllFilterHangle != NULL)
      g_createFunc = (PCREATEFUN_T)GetProcAddress(g_dllFilterHangle, "CreateFilter");

    if (g_createFunc != nullptr)
    {
      std::wstring filtersCommon[] = { L"tonemapping_obsolette" };

      for (auto name : filtersCommon)
        g_commonFilters[name] = std::shared_ptr<IFilter2D>(g_createFunc(name.c_str()));
    }
  }

  // add filters from Hydra private post process dll
  //
  {
    g_dllFilterHangle2 = LoadLibraryW(L"C:/[Hydra]/bin2/PostProcess.dll");

    if (g_dllFilterHangle2 != NULL)
      g_createFunc2 = (PCREATEFUN_T)GetProcAddress(g_dllFilterHangle2, "CreateFilter");

    if (g_createFunc2 != nullptr)
    {
      std::wstring filtersCommon[] = { L"post_process_hydra1" };

      for (auto name : filtersCommon)
        g_commonFilters[name] = std::shared_ptr<IFilter2D>(g_createFunc2(name.c_str()));
    }
  }

#endif // _WIN32


}


HRFBIRef hrFBICreate(const wchar_t* name, int w, int h, int bpp, const void* a_data)
{
  HRFBIRef res;

  if (bpp == 16)
  {
    const float* fdata = (const float*)a_data;
    HDRImage4f* pNewImage = new HDRImage4f(w, h, fdata);
    g_fbImages.push_back(FrameBufferImage(name, pNewImage));
    res.id = int32_t(g_fbImages.size()-1);
  }
  else if (bpp == 4)
  {
    const int* idata = (const int*)a_data;
    LDRImage1i* pNewImage = new LDRImage1i(w, h, idata);
    g_fbImages.push_back(FrameBufferImage(name, pNewImage));
    res.id = int32_t(g_fbImages.size() - 1);
  }
  else
  {
    HrPrint(HR_SEVERITY_ERROR, L"[hrFBICreate]: bad bpp = ", bpp);
  }

  return res;
}

void hrFBIResize(HRFBIRef a_image, int w, int h)
{
  if (a_image.id >= g_fbImages.size() || a_image.id < 0)
  {
    HrPrint(HR_SEVERITY_ERROR, L"[hrFBIResize]: bad image id = ", a_image.id);
    return;
  }

  auto& image = g_fbImages[a_image.id];

  if (image.pHDRImage != nullptr)
    image.pHDRImage->resize(w, h);
  else if (image.pLDRImage != nullptr)
    image.pLDRImage->resize(w, h);

}

HRFBIRef hrFBICreateFromFile(const wchar_t* a_fileName, int a_desiredBpp, float a_inputGamma)
{
  HRFBIRef resId = hrFBICreate(a_fileName, 0, 0, 16, nullptr);
  hrFBILoadFromFile(resId, a_fileName, a_desiredBpp, a_inputGamma);
  return resId;
}

void hrFBISaveToFile(HRFBIRef a_image, const wchar_t* a_fileName, const float a_gamma)
{
  if (a_image.id >= g_fbImages.size() || a_image.id < 0)
  {
    HrPrint(HR_SEVERITY_ERROR, L"[hrFBISaveToFile]: bad image id = ", a_image.id);
    return;
  }

  std::wstring inFileName(a_fileName);

  auto& image = g_fbImages[a_image.id];

  if (image.pHDRImage != nullptr)
  {
    const float* fdata = image.pHDRImage->data();
    
    if (inFileName.find(L".image4f") != std::wstring::npos) //#TODO: check normal way check of file extension
    {
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
      std::wstring s1(a_fileName);
      std::string  s2(s1.begin(), s1.end());
      std::ofstream fout(s2.c_str(), std::ios::binary);
#elif defined WIN32
      std::ofstream fout(a_fileName, std::ios::binary);
#endif
      int wh[2] = { image.pHDRImage->width(), image.pHDRImage->height() };
      fout.write((const char*)wh, sizeof(wh));
      fout.write((const char*)fdata, wh[0]*wh[1]*sizeof(float));
      fout.close();
    }
    else if (inFileName.find(L".hdr") != std::wstring::npos || inFileName.find(L".exr") != std::wstring::npos)
      HydraRender::SaveHDRImageToFileHDR(inFileName, image.pHDRImage->width(), image.pHDRImage->height(), fdata);
    else
      HydraRender::SaveImageToFile(inFileName, *image.pHDRImage, a_gamma);

  }
  else if (image.pLDRImage != nullptr)
  {
    const unsigned int* idata = (const unsigned int*)image.pLDRImage->data();
    HydraRender::SaveImageToFile(a_fileName, image.pHDRImage->width(), image.pHDRImage->height(), idata);
  }
  else
  {
    HrPrint(HR_SEVERITY_ERROR, L"[hrFBISaveToFile]: attempt to save empty ldf image, id = ", a_image.id);
  }

}

void hrFBILoadFromFile(HRFBIRef a_image, const wchar_t* a_fileName, int a_desiredBpp, float a_inputGamma) //#TODO: alot of work should be done here
{
  if (a_image.id >= g_fbImages.size() || a_image.id < 0)
  {
    HrPrint(HR_SEVERITY_ERROR, L"[hrFBILoadFromFile(1)]: bad image id = ", a_image.id);
    return;
  }

  std::wstring inFileName(a_fileName);

  auto& image = g_fbImages[a_image.id];

  if (inFileName.find(L".image4f") != std::wstring::npos)
  {
    if (a_desiredBpp != -1 && a_desiredBpp != 16) // #TODO: implement conversion from 16 to 4 bpp (HDR to LDR)
    {

    }
    else
    {
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
      std::wstring s1(a_fileName);
      std::string  s2(s1.begin(), s1.end());
      std::ifstream fout(s2.c_str(), std::ios::binary);
#elif defined WIN32
      std::ifstream fout(a_fileName, std::ios::binary);
#endif

      if (!fout.is_open())
      {
        HrPrint(HR_SEVERITY_WARNING, L"[hrFBILoadFromFile(2)]: can't load image file = ", inFileName);
        return;
      }

      int wh[2];
      fout.read((char*)wh, sizeof(wh));

      image.pLDRImage = nullptr;
      image.pHDRImage = std::make_shared<HDRImage4f>(wh[0], wh[1]);
      image.name      = a_fileName;

      float* data = image.pHDRImage->data();
      fout.read((char*)data, wh[0] * wh[1] * sizeof(float));
      fout.close();
    }
  }
  else if (inFileName.find(L".hdr") != std::wstring::npos || inFileName.find(L".exr") != std::wstring::npos)
  {
    if (a_desiredBpp != -1 && a_desiredBpp != 16) // #TODO: implement conversion from 16 to 4 bpp (HDR to LDR)
    {

    }
    else
    {
      image.pLDRImage = nullptr;
      image.pHDRImage = std::make_shared<HDRImage4f>(0, 0);
      image.name      = a_fileName;

      HydraRender::LoadImageFromFile(inFileName, (*image.pHDRImage));

      if (image.pHDRImage->width() == 0 || image.pHDRImage->height() == 0)
      {
        HrPrint(HR_SEVERITY_WARNING, L"[hrFBILoadFromFile(3)]: can't load image file = ", inFileName);
        return;
      }
    }
  }
  else
  {
    if (a_desiredBpp == 16 || a_desiredBpp == -1) // default is to load LDR to HDR
    {
      image.pLDRImage = nullptr;
      image.pHDRImage = std::make_shared<HDRImage4f>(0, 0);
      image.name      = a_fileName;

      HydraRender::LoadImageFromFile(inFileName, (*image.pHDRImage));

      int w = image.pHDRImage->width();
      int h = image.pHDRImage->height();

      if (w == 0 || h == 0)
      {
        HrPrint(HR_SEVERITY_WARNING, L"[hrFBILoadFromFile(4)]: can't load image file = ", inFileName);
        return;
      }

      if (fabs(a_inputGamma - 1.0f) > 1e-5f) // don't perform gamma transform if gamma == 1.0f
      {
        float* data = image.pHDRImage->data();

        #pragma omp parallel for
        for (int i = 0; i < w*h; i++) // perform gamma transform
        {
          data[i * 4 + 0] = powf(data[i * 4 + 0], a_inputGamma);
          data[i * 4 + 1] = powf(data[i * 4 + 1], a_inputGamma);
          data[i * 4 + 2] = powf(data[i * 4 + 2], a_inputGamma);
        }
      }
    }
    else // #TODO: implement loading of LDR images to LDR image
    {
      image.pLDRImage = std::make_shared<LDRImage1i>(0, 0);
      image.pHDRImage = nullptr;
      image.name      = a_fileName;

      // int w, h;
      // auto& vec = image.pLDRImage->dataVector();
      // HydraRender::LoadImageFromFile(a_fileName, vec, &w, &h);

    }
  }

}

const void* hrFBIGetData(HRFBIRef a_image, int* pW, int* pH, int* pBpp)
{
  if (a_image.id >= g_fbImages.size() || a_image.id < 0)
  {
    HrPrint(HR_SEVERITY_ERROR, L"[hrFBIGetData]: bad image id = ", a_image.id);
    return nullptr;
  }

  auto& image = g_fbImages[a_image.id];

  const void* result = nullptr;

  if (image.pHDRImage != nullptr)
  {
    if (pW != nullptr) 
      (*pW) = image.pHDRImage->width();

    if (pH != nullptr)
      (*pH) = image.pHDRImage->height();

    if (pBpp != nullptr)
      (*pBpp) = 16;

    result = image.pHDRImage->data();
  }
  else if (image.pLDRImage != nullptr)
  {
    if (pW != nullptr)
      (*pW) = image.pLDRImage->width();

    if (pH != nullptr)
      (*pH) = image.pLDRImage->height();

    if (pBpp != nullptr)
      (*pBpp) = 4;

    result = image.pLDRImage->data();
  }

  return result;
}

void hrRenderCopyFrameBufferToFBI(HRRenderRef a_render, const wchar_t* a_name, HRFBIRef a_outData)
{
  HRRender* pRenderObj = g_objManager.PtrById(a_render);

  if (pRenderObj == nullptr)
  {
    HrError(L"hrFBIGetFromRender: nullptr input");
    return;
  }

  if (pRenderObj->m_pDriver == nullptr)
  {
    HrError(L"hrFBIGetFromRender: nullptr render driver");
    return;
  }

  if (a_outData.id >= g_fbImages.size() || a_outData.id < 0)
  {
    HrPrint(HR_SEVERITY_ERROR, L"[hrFBIGetDataFromRender]: bad image id = ", a_outData.id);
    return;
  }

  auto& image = g_fbImages[a_outData.id].pHDRImage;

  pRenderObj->m_pDriver->GetFrameBufferHDR(image->width(), image->height(), image->data(), a_name);
}


void hrFilterApply(const wchar_t* a_filterName, pugi::xml_node a_parameters,
                   const wchar_t* a_argName1,   HRFBIRef a_arg1,
                   const wchar_t* a_argName2,   HRFBIRef a_arg2,
                   const wchar_t* a_argName3,   HRFBIRef a_arg3,
                   const wchar_t* a_argName4,   HRFBIRef a_arg4,
                   const wchar_t* a_argName5,   HRFBIRef a_arg5,
                   const wchar_t* a_argName6,   HRFBIRef a_arg6,
                   const wchar_t* a_argName7,   HRFBIRef a_arg7,
                   const wchar_t* a_argName8,   HRFBIRef a_arg8)
{

  const wchar_t* args  [FILTER_MAX_ARGS];
  HRFBIRef       images[FILTER_MAX_ARGS];

  args[0] = a_argName1;
  args[1] = a_argName2;
  args[2] = a_argName3;
  args[3] = a_argName4;
  args[4] = a_argName5;
  args[5] = a_argName6;
  args[6] = a_argName7;
  args[7] = a_argName8;

  images[0] = a_arg1;
  images[1] = a_arg2;
  images[2] = a_arg3;
  images[3] = a_arg4;
  images[4] = a_arg5;
  images[5] = a_arg6;
  images[6] = a_arg7;
  images[7] = a_arg8;

  // first we need to figure pout how many args are valid
  //
  int imagesNumber = 0;

  for (int i = 0; i < FILTER_MAX_ARGS; i++)
  {
    if (images[i].id != -1 && args[i] != nullptr)
    {
      if (std::wstring(args[i]) != L"")
        imagesNumber++;
      else
        break;
    }
    else
      break;
  }

  // now we must figure out what API should be used for filter with name a_filterName
  //
  auto p = g_spetialFilters.find(a_filterName);
  if (p != g_spetialFilters.end())              // some special implementation like Resample or MedianInPlace
  {
    std::shared_ptr<IFilter2DSpecial> pFilterSpecial = p->second;

    IFilter2DSpecial::ArgArray1 argArrayHDR;
    IFilter2DSpecial::ArgArray2 argArrayLDR;

    for (int i = 0; i < imagesNumber; i++)
    {
      if (images[i].id >= g_fbImages.size() || images[i].id < 0)
      {
        HrPrint(HR_SEVERITY_ERROR, L"[hrFilterApply]: bad image id = ", images[i].id);
        continue;
      }

      auto& image = g_fbImages[images[i].id];
      argArrayHDR[args[i]] = image.pHDRImage;      
      argArrayLDR[args[i]] = image.pLDRImage;      
    }

    bool isOk = pFilterSpecial->Eval(argArrayHDR, argArrayLDR, a_parameters);
    if (!isOk)
    {
      HrPrint(HR_SEVERITY_ERROR, L"[hrFilterApply]: filter error = ", pFilterSpecial->GetLastError());
      return;
    }
  }
  else                                          // common filter impl.
  {
    // first, find filter by name
    //
    auto p2 = g_commonFilters.find(a_filterName);
    if (p2 == g_commonFilters.end())
    {
      HrPrint(HR_SEVERITY_ERROR, L"[hrFilterApply]: unknown filter, name  = ", a_filterName);
      return;
    }

    auto pFilter = p2->second;

    // next, put xml node to string to pass it to DLL plugin
    //
    pugi::xml_document doc;
    doc.append_copy(a_parameters);

    std::wstringstream strOut;
    doc.save(strOut);

    const std::wstring xmlStr = strOut.str();
    
    // then, fill C-style input to pass it to DLL plugin
    //
    Filter2DInput input;
    
    input.argsNum        = imagesNumber;
    input.settingsXmlStr = xmlStr.c_str();
    
    for (int i = 0; i < imagesNumber; i++)
    {
      input.names[i] = args[i];
    
      if (images[i].id >= g_fbImages.size() || images[i].id < 0)
      {
        HrPrint(HR_SEVERITY_ERROR, L"[hrFilterApply]: bad image id = ", images[i].id);
        continue;
      }
    
      auto& image = g_fbImages[images[i].id];
    
      if (image.pHDRImage != nullptr)
      {
        input.width [i] = image.pHDRImage->width();
        input.height[i] = image.pHDRImage->height();
        input.datas [i] = image.pHDRImage->data();
        input.bpp   [i] = 16;
      }
      else
      {
        input.width [i] = image.pLDRImage->width();
        input.height[i] = image.pLDRImage->height();
        input.datas [i] = (float*)image.pLDRImage->data();
        input.bpp   [i] = 4;
      }
    }
    
    // finally set input and run filter
    //
    pFilter->SetInput(input);
    if (!pFilter->Eval())
    {
      HrPrint(HR_SEVERITY_ERROR, L"[hrFilterApply]: filter error = ", pFilter->GetLastError());
    }
    else if (pFilter->HasWarning())
    {
      HrPrint(HR_SEVERITY_WARNING, L"[hrFilterApply]: filter warning = ", pFilter->GetLastError());
    }

  }

}
