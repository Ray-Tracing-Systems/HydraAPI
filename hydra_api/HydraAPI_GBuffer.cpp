#include "HydraAPI.h"
#include "HydraInternal.h"
#include "HydraInternalCommon.h"

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

HAPI void hrRenderGetGBufferLine(const HRRenderRef a_pRender, int32_t a_lineNumber, HRGBufferPixel* a_lineData, int32_t a_startX, int32_t a_endX)
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

  pDriver->GetGBufferLine(a_lineNumber, a_lineData, a_startX, a_endX);
}

static inline int RealColorToUint32(const float real_color[4])
{
  float  r = real_color[0] * 255.0f;
  float  g = real_color[1] * 255.0f;
  float  b = real_color[2] * 255.0f;
  float  a = real_color[3] * 255.0f;

  unsigned char red   = (unsigned char)r;
  unsigned char green = (unsigned char)g;
  unsigned char blue  = (unsigned char)b;
  unsigned char alpha = (unsigned char)a;

  return red | (green << 8) | (blue << 16) | (alpha << 24);
}

static void ExtractDepthLine(const HRGBufferPixel* a_inLine, int32_t* a_outLine, int a_width, const float dmin, const float dmax)
{
  for (int x = 0; x < a_width; x++)
  {
    float d = (a_inLine[x].depth - dmin) / (dmax - dmin);
    if (d > 1e5f || a_inLine[x].matId < 0)
      d = 0.0f;

    float depth[4];
    depth[0] = d;
    depth[1] = d;
    depth[2] = d;
    depth[3] = 1.0f;

    a_outLine[x] = RealColorToUint32(depth);
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

    a_outLine[x] = RealColorToUint32(norm);
  }
}

static void ExtractTexCoordLine(const HRGBufferPixel* a_inLine, int32_t* a_outLine, int a_width)
{
  for (int x = 0; x < a_width; x++)
  {
    float texc[4];
    texc[0] = a_inLine[x].texc[0];
    texc[1] = a_inLine[x].texc[1];
    texc[2] = 0.0f;
    texc[3] = 1.0f;

    a_outLine[x] = RealColorToUint32(texc);
  }
}

static void ExtractTexColorLine(const HRGBufferPixel* a_inLine, int32_t* a_outLine, int a_width)
{
  const float invGamma = 1.0f / 2.2f;

  for (int x = 0; x < a_width; x++)
  {
    float texc[4];
    texc[0] = a_inLine[x].rgba[0];
    texc[1] = a_inLine[x].rgba[1];
    texc[2] = a_inLine[x].rgba[2];
    texc[3] = 1.0f;

    const float color[4] = { powf(texc[0], invGamma),
                             powf(texc[1], invGamma),
                             powf(texc[2], invGamma),
                             texc[3]
    };

    a_outLine[x] = RealColorToUint32(color);
  }
}

static void ExtractAlphaLine(const HRGBufferPixel* a_inLine, int32_t* a_outLine, int a_width)
{
  for (int x = 0; x < a_width; x++)
  {
    float texc[4];
    texc[0] = a_inLine[x].rgba[3];
    texc[1] = a_inLine[x].rgba[3];
    texc[2] = a_inLine[x].rgba[3];
    texc[3] = 1.0f;

    a_outLine[x] = RealColorToUint32(texc);
  }
}

static void ExtractShadowLine(const HRGBufferPixel* a_inLine, int32_t* a_outLine, int a_width)
{
  for (int x = 0; x < a_width; x++)
  {
    float texc[4];
    texc[0] = a_inLine[x].shadow;
    texc[1] = a_inLine[x].shadow;
    texc[2] = a_inLine[x].shadow;
    texc[3] = 1.0f;

    a_outLine[x] = RealColorToUint32(texc);
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
    a_pDriver->GetGBufferLine(y, &gbufferLine[0], 0, width);
    for (int x = 0; x < width; x++)
    {
      const float d = gbufferLine[x].depth;
      if (d < 1e5f && d >= 0.0f && isfinite(d))
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



bool HR_SaveLDRImageToFile(const wchar_t* a_fileName, int w, int h, int32_t* data);

HAPI bool hrRenderSaveGBufferLayerLDR(const HRRenderRef a_pRender, const wchar_t* a_outFileName, const wchar_t* a_layerName,
                                      const int* a_palette, const int a_paletteSize)
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

  auto renderSettingsNode = pRender->xml_node_immediate();

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


  for (int y = 0; y < height; y++)
  {
    pDriver->GetGBufferLine(y, &gbufferLine[0], 0, width);

    if (lname == L"depth")
      ExtractDepthLine(&gbufferLine[0], &imageLDR[y*width], width, dmin, dmax);
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
    else if (lname == L"matid" || lname == L"mid")
      ExtractMaterialId(&gbufferLine[0], &imageLDR[y*width], width);
    else if (lname == L"objid")
      ExtractObjId(&gbufferLine[0], &imageLDR[y*width], width);
    else if (lname == L"instid")
      ExtractInstId(&gbufferLine[0], &imageLDR[y*width], width);

    if (lname == L"matid" || lname == L"mid" || lname == L"objid" || lname == L"instid")
    {
      unsigned int* line = (unsigned int*)&imageLDR[y*width];
      for (int x = 0; x < width; x++)
      {
        const int index = line[x];
        if (index < 0)
          line[x] = 0;
        else
          line[x] = palette[index % paletteSize];
      }
    }
  }

  HR_SaveLDRImageToFile(a_outFileName, width, height, &imageLDR[0]);

  return true;
}


