#include "HydraAPI.h"
#include "HydraInternal.h"

#include <memory>
#include <vector>
#include <string>
#include <map>

#include <sstream>
#include <fstream>

#include "HydraObjectManager.h"

#include "LiteMath.h"
#include "HydraXMLHelpers.h"

#pragma warning(disable:4996) // MS shoild kill theirself with " 'wcsncpy': This function or variable may be unsafe. Consider using wcsncpy_s instead."

extern HRObjectManager g_objManager;

HAPI void hrRenderGetGBufferLine(HRRenderRef a_pRender, int32_t a_lineNumber, HRGBufferPixel* a_lineData, int32_t a_startX, int32_t a_endX)
{
  HRRender* pRender = g_objManager.PtrById(a_pRender);

  if (pRender == nullptr)
  {
    HrError(L"hrRenderGetDeviceList: nullptr input");
    return;
  }

  auto pDriver = pRender->m_pDriver;
  if (pDriver == nullptr)
    return;

  pDriver->GetGBufferLine(a_lineNumber, a_lineData, a_startX, a_endX, g_objManager.scnData.m_shadowCatchers);
}


static void ExtractDepthLine(const HRGBufferPixel* a_inLine, int32_t* a_outLine, int a_width, const float dmin, const float dmax)
{
  for (int x = 0; x < a_width; x++)
  {
    float d = (a_inLine[x].depth - dmin) / (dmax - dmin);
    if (d > 1e5f || a_inLine[x].matId < 0)
      d = 1.0f;

    a_outLine[x] = HRUtils::RealColorToUint32(1.0f - d, 1.0f - d, 1.0f - d, 1.0f);
  }
}

static void ExtractDepthLineU16(const HRGBufferPixel* a_inLine, unsigned short* a_outLine, int a_width, const float dmin, const float dmax)
{
  for (int x = 0; x < a_width; x++)
  {
    const float d = (a_inLine[x].depth - dmin) / (dmax - dmin);

    int r = (int)((1.0f-d)*65535.0f);
    if(r > 65535)
      r = 65535;
    else if (r < 1)
      r = 1;

    if (d > 1e5f || a_inLine[x].matId < 0)
      r = 0;

    a_outLine[x] = (unsigned short)(r);
  }
}

static void ExtractDepthLineSpecial(const HRGBufferPixel* a_inLine, int32_t* a_outLine, int a_width, const float dmax)
{
  for (int x = 0; x < a_width; x++)
  {
    const float d = a_inLine[x].depth / dmax;

    a_outLine[x] = HRUtils::RealColorToUint32(d, d, d, 1.0f);
  }
}


static void ExtractNormalsLine(const HRGBufferPixel* a_inLine, int32_t* a_outLine, int a_width)
{
  for (int x = 0; x < a_width; x++)
  {
    float norm[4];
    norm[0] = fabs(a_inLine[x].norm[0]);
    norm[1] = fabs(a_inLine[x].norm[1]);
    norm[2] = fabs(a_inLine[x].norm[2]);
    norm[3] = 1.0f;

    a_outLine[x] = RealColorToUint32(fabs(a_inLine[x].norm[0]),
                                     fabs(a_inLine[x].norm[1]),
                                     fabs(a_inLine[x].norm[2]),
                                     1.0f);
  }
}

static void ExtractTexCoordLine(const HRGBufferPixel* a_inLine, int32_t* a_outLine, int a_width)
{
  for (int x = 0; x < a_width; x++)
  {
    a_outLine[x] = RealColorToUint32(a_inLine[x].texc[0], a_inLine[x].texc[1], 0.0f, 1.0f);
  }
}

static inline float LinearToSRGB(float l)
{
  if(l <= 0.00313066844250063f)
    return l*12.92f;
  else
    return 1.055*std::pow(l, 1.0f/2.4f) - 0.055;
}

static void ExtractTexColorLine(const HRGBufferPixel* a_inLine, int32_t* a_outLine, int a_width)
{
  //const float invGamma = 1.0f / 2.2f;

  for (int x = 0; x < a_width; x++)
  {
    float texc[4];
    texc[0] = a_inLine[x].rgba[0];
    texc[1] = a_inLine[x].rgba[1];
    texc[2] = a_inLine[x].rgba[2];
    texc[3] = 1.0f;

    a_outLine[x] = RealColorToUint32(LinearToSRGB(texc[0]),
                                     LinearToSRGB(texc[1]),
                                     LinearToSRGB(texc[2]),
                                     texc[3]);
  }
}

static void ExtractAlphaLine(const HRGBufferPixel* a_inLine, int32_t* a_outLine, int a_width)
{
  for (int x = 0; x < a_width; x++)
  {
    a_outLine[x] = RealColorToUint32(a_inLine[x].rgba[3], a_inLine[x].rgba[3], a_inLine[x].rgba[3], 1.0f);
  }
}

static void ExtractShadowLine(const HRGBufferPixel* a_inLine, int32_t* a_outLine, int a_width)
{
  for (int x = 0; x < a_width; x++)
  {
    a_outLine[x] = RealColorToUint32(a_inLine[x].shadow, a_inLine[x].shadow, a_inLine[x].shadow, 1.0f);
  }
}

void FindDepthMinMax(std::shared_ptr<IHRRenderDriver> a_pDriver, int width, int height,
                     float& dmin, float& dmax)
{
  std::vector<HRGBufferPixel> gbufferLine(width);

  // #TODO: replace this with minmaxheap or simply downsample image 
  //
  dmin = 1e38f;
  dmax = 0.0f;

  for (int y = 0; y < height; y++)
  {
    a_pDriver->GetGBufferLine(y, &gbufferLine[0], 0, width, g_objManager.scnData.m_shadowCatchers);
    for (int x = 0; x < width; x++)
    {
      const float d = gbufferLine[x].depth;
      if (d < 1e5f && d >= 0.0f && std::isfinite(d))
      {
        if (d < dmin) dmin = d;
        if (d > dmax) dmax = d;
      }
    }
  }

  if (dmax - dmin < 1e-5f)
  {
    dmin = 0.0f;
    dmax = 1.0f;
  }

}

static void ExtractMaterialId(const HRGBufferPixel* a_inLine, int32_t* a_outLine, int a_width)
{
  for (int x = 0; x < a_width; x++)
  {
    if (a_inLine[x].instId < 0)
      a_outLine[x] = -1;
    else
      a_outLine[x] = a_inLine[x].matId;
  }
}

static void ExtractObjId(const HRGBufferPixel* a_inLine, int32_t* a_outLine, int a_width)
{
  for (int x = 0; x < a_width; x++)
    a_outLine[x] = a_inLine[x].objId;
}

static void ExtractInstId(const HRGBufferPixel* a_inLine, int32_t* a_outLine, int a_width)
{
  for (int x = 0; x < a_width; x++)
    a_outLine[x] = a_inLine[x].instId;
}



static void ExtractCoverage(const HRGBufferPixel* a_inLine, int32_t* a_outLine, int a_width)
{
  for (int x = 0; x < a_width; x++)
  {
    const float cov    = a_inLine[x].coverage;
    a_outLine[x]  = RealColorToUint32(cov, cov, cov, 1);
  }
}


HAPI bool hrRenderSaveGBufferLayerLDR(HRRenderRef a_pRender, const wchar_t* a_outFileName, const wchar_t* a_layerName,
                                      const int* a_palette, int a_paletteSize)
{
  HRRender* pRender = g_objManager.PtrById(a_pRender);

  if (pRender == nullptr)
  {
    HrError(L"hrRenderGetDeviceList: nullptr input");
    return false;
  }

  auto pDriver = pRender->m_pDriver;
  if (pDriver == nullptr)
    return false;

  std::wstring lname = std::wstring(a_layerName);

  auto renderSettingsNode = pRender->xml_node();

  const int width     = renderSettingsNode.child(L"width").text().as_int();
  const int height    = renderSettingsNode.child(L"height").text().as_int();
  const int evalgbuff = renderSettingsNode.child(L"evalgbuffer").text().as_int();

  if (evalgbuff != 1)
  {
    HrError(L"hrRenderSaveGBufferLayerLDR: don't have gbuffer; set 'evalgbuffer' = 1 and render again; ");
    return false;
  }

  std::vector<int32_t>        imageLDR(width * height);
  std::vector<HRGBufferPixel> gbufferLine(width);
  
  float dmin = 1e38f;
  float dmax = 0.0f;
  if (lname == L"depth")
  {
    FindDepthMinMax(pDriver, width, height,
                    dmin, dmax);
  }

  // #TODO: refactor, put to separate procedure
  //
  const unsigned int defaultpalette[20] = { 0xffe6194b, 0xff3cb44b, 0xffffe119, 0xff0082c8,
                                            0xfff58231, 0xff911eb4, 0xff46f0f0, 0xfff032e6,
                                            0xffd2f53c, 0xfffabebe, 0xff008080, 0xffe6beff,
                                            0xffaa6e28, 0xfffffac8, 0xff800000, 0xffaaffc3,
                                            0xff808000, 0xffffd8b1, 0xff000080, 0xff808080 };

  const unsigned int* palette = &defaultpalette[0];
  int paletteSize = 20;
  if (a_palette != nullptr && a_paletteSize!=0)
  {
    palette     = (const unsigned int*)a_palette;
    paletteSize = a_paletteSize;
  }

  HRSceneInstRef scnRef;
  scnRef.id = g_objManager.m_currSceneId;
  HRSceneInst *pScn = g_objManager.PtrById(scnRef);
  auto scnNode = pScn->xml_node();

  std::vector <int32_t> instanceIdToScnId(pScn->drawList.size(), 0);


  if(lname == L"scnsid")
  {
    for (auto node = scnNode.first_child(); node != nullptr; node = node.next_sibling())
      if (std::wstring(node.name()) == L"instance")
        instanceIdToScnId[node.attribute(L"id").as_int()] = node.attribute(L"scn_sid").as_int();
  }
  else if(lname == L"scnid")
  {
    for (auto node = scnNode.first_child(); node != nullptr; node = node.next_sibling())
      if (std::wstring(node.name()) == L"instance")
        instanceIdToScnId[node.attribute(L"id").as_int()] = node.attribute(L"scn_id").as_int();
  }


  unsigned short* imageU16 = (unsigned short*)imageLDR.data();

  hrRenderLockFrameBufferUpdate(a_pRender);

  for (int y = 0; y < height; y++)
  {
    pDriver->GetGBufferLine(y, &gbufferLine[0], 0, width, g_objManager.scnData.m_shadowCatchers);

    if (lname == L"depth")
      //ExtractDepthLine(&gbufferLine[0], &imageLDR[y*width], width, dmin, dmax);
      ExtractDepthLineU16(&gbufferLine[0], &imageU16[y*width], width, dmin, dmax);
    else if (lname == L"normals")
      ExtractNormalsLine(&gbufferLine[0], &imageLDR[y*width], width);
    else if (lname == L"texcoord")
      ExtractTexCoordLine(&gbufferLine[0], &imageLDR[y*width], width);
    else if (lname == L"diffcolor")
      ExtractTexColorLine(&gbufferLine[0], &imageLDR[y*width], width);
    else if (lname == L"alpha")
      ExtractAlphaLine(&gbufferLine[0], &imageLDR[y*width], width);
    else if (lname == L"shadow")
      ExtractShadowLine(&gbufferLine[0], &imageLDR[y*width], width);
    else if (lname == L"matid" || lname == L"mid" || lname == L"catcher")
      ExtractMaterialId(&gbufferLine[0], &imageLDR[y*width], width);
    else if (lname == L"objid")
      ExtractObjId(&gbufferLine[0], &imageLDR[y*width], width);
    else if (lname == L"instid" || lname == L"scnsid" || lname == L"scnid")
      ExtractInstId(&gbufferLine[0], &imageLDR[y*width], width);
    else if (lname == L"coverage")
      ExtractCoverage(&gbufferLine[0], &imageLDR[y*width], width);


    if (lname == L"matid" || lname == L"mid" || lname == L"objid" || lname == L"instid")
    {
      auto* line = (unsigned int*)&imageLDR[y*width];
      for (int x = 0; x < width; x++)
      {
        const int index = line[x];
        if (index < 0)
          line[x] = 0;
        else
          line[x] = palette[index % paletteSize];
      }
    }

    else if (lname == L"scnsid" || lname == L"scnid")
    {
      auto* line = (unsigned int*)&imageLDR[y*width];
      for (int x = 0; x < width; x++)
      {
        const int index = line[x];
        if (index < 0)
          line[x] = 0;
        else
        {
          int new_index = instanceIdToScnId.at(index);
          line[x] = palette[new_index % paletteSize];
        }
      }
    }
    else if(lname == L"catcher")
    {
      auto* line = (unsigned int*)&imageLDR[y*width];
      for (int x = 0; x < width; x++)
      {
        const int index = gbufferLine[x].matId;// & 0x00FFFFFF;

        if (g_objManager.scnData.m_shadowCatchers.find(index) != g_objManager.scnData.m_shadowCatchers.end())
          line[x] = 0x00FFFFFF;
        else
          line[x] = 0;
      }
    }
  }

  hrRenderUnlockFrameBufferUpdate(a_pRender);

  if (lname == L"depth")
    g_objManager.m_pImgTool->Save16BitMonoImageTo16BitPNG(a_outFileName, width, height, imageU16);
  else
    g_objManager.m_pImgTool->SaveLDRImageToFileLDR(a_outFileName, width, height, imageLDR.data());

  return true;
}

bool HRUtils::hrRenderSaveDepthRaw(HRRenderRef a_pRender, const wchar_t* a_outFileName)
{
  HRRender* pRender = g_objManager.PtrById(a_pRender);

  if (pRender == nullptr)
  {
    HrError(L"hrRenderGetDeviceList: nullptr input");
    return false;
  }

  auto pDriver = pRender->m_pDriver;
  if (pDriver == nullptr)
    return false;

  auto renderSettingsNode = pRender->xml_node();

  const int width     = renderSettingsNode.child(L"width").text().as_int();
  const int height    = renderSettingsNode.child(L"height").text().as_int();
  const int evalgbuff = renderSettingsNode.child(L"evalgbuffer").text().as_int();

  if (evalgbuff != 1)
  {
    HrError(L"hrRenderSaveGBufferLayerLDR: don't have gbuffer; set 'evalgbuffer' = 1 and render again; ");
    return false;
  }

  std::vector<float>          rawDepth(width * height);
  std::vector<HRGBufferPixel> gbufferLine(width);

  float* data = rawDepth.data();

  for (int y = 0; y < height; y++)
  {
    pDriver->GetGBufferLine(y, &gbufferLine[0], 0, width, g_objManager.scnData.m_shadowCatchers);
    for (int x = 0; x < width; x++)
    {
      data[y * width + x] = gbufferLine[x].depth;
    }
  }

  g_objManager.m_pImgTool->SaveMonoHDRImageToFileHDR(a_outFileName, width, height, data);

  return true;
}
