#include <iostream>
#include <fstream>
#include <sstream>
#include <tuple>

#include <ctime>
#include <functional>

#include "LiteMath.h"
#include <omp.h>

#include "HydraPostProcessSpecial.h"

using HydraLiteMath::float4;
using HydraLiteMath::float3;
using HydraRender::HDRImage4f;
using HydraLiteMath::clamp;

static inline int clampi(int x, int a, int b)
{
  if (x < a) return a;
  else if (x > b) return b;
  else            return x;
}

static inline float SQRF(float x) { return x*x; }

static inline float NLMWeight(const float4* in_buff, int w, int h, int x, int y, int x1, int y1, int a_blockRadius)
{
  float w1 = 0.0f; // this is what NLM differs from KNN (bilateral)
  {
    const int minX1 = clampi(x1 - a_blockRadius, 0, w - 1);
    const int maxX1 = clampi(x1 + a_blockRadius, 0, w - 1);

    const int minY1 = clampi(y1 - a_blockRadius, 0, h - 1);
    const int maxY1 = clampi(y1 + a_blockRadius, 0, h - 1);

    for (int y2 = minY1; y2 <= maxY1; y2++)
    {
      for (int x2 = minX1; x2 <= maxX1; x2++)
      {
        const int offsX = x2 - x1;
        const int offsY = y2 - y1;

        const int x3 = clampi(x + offsX, 0, w - 1);
        const int y3 = clampi(y + offsY, 0, h - 1);

        const float4 c2 = in_buff[y2*w + x2];
        const float4 c3 = in_buff[y3*w + x3];

        const float4 dist = c2 - c3;
        w1 += dot3(dist, dist);
      }
    }

  }

  return w1 / SQRF(2.0f * float(a_blockRadius) + 1.0f);
}


static inline float surfaceSimilarity(float4 data1, float4 data2, float MADXDIFF)
{
  const float MANXDIFF = 0.1;

  float3 n1 = to_float3(data1);
  float3 n2 = to_float3(data2);

  float dist = length(n1 - n2);
  if (dist >= MANXDIFF)
    return 0.0f;

  float d1 = data1.w;
  float d2 = data2.w;

  if (abs(d1 - d2) >= MADXDIFF)
    return 0.0f;

  float normalDiff = sqrtf(1.0f - (dist / MANXDIFF));
  float depthDiff = sqrtf(1.0f - abs(d1 - d2) / MADXDIFF);

  return normalDiff*depthDiff;
}

static inline float projectedPixelSize(float dist, float FOV, float w, float h)
{
  float ppx = (FOV / w)*dist;
  float ppy = (FOV / h)*dist;

  if (dist > 0.0f)
    return 2.0f*fmax(ppx, ppy);
  else
    return 1000.0f;
}

// FILTER2D_PROGRESSBAR_CALLBACK g_progressBar = nullptr;

void NonLocalMeansGuidedTexNormDepthFilter(const HDRImage4f& inImage, const HDRImage4f& inTexColor, const HDRImage4f& inNormDepth,
                                           HDRImage4f& outImage, int a_windowRadius, int a_blockRadius, float a_noiseLevel)
{
  ////////////////////////////////////////////////////////////////////
  const float g_NoiseLevel       = 1.0f / (a_noiseLevel*a_noiseLevel);
  const float g_GaussianSigma    = 1.0f / 50.0f;
  const float g_WeightThreshold  = 0.03f;
  const float g_LerpCoefficeint  = 0.80f;
  const float g_CounterThreshold = 0.05f;

  const float DEG_TO_RAD = 0.017453292519943295769236907684886f;
  const float m_fov = DEG_TO_RAD*90.0f;
  ////////////////////////////////////////////////////////////////////

  const int w = inImage.width();
  const int h = inImage.height();

  outImage.resize(w, h);

  const float4* in_buff  = (const float4*)inImage.data();
  const float4* in_texc  = (const float4*)inTexColor.data();
  const float4* nd_buff  = (const float4*)inNormDepth.data();
  float4*       out_buff = (float4*)outImage.data();

  float windowArea = SQRF(2.0f * float(a_windowRadius) + 1.0f);

  int linesDone     = 0;
  //int linesDonePrev = 0;

  #pragma omp parallel for
  for (int y = 0; y < h; y++)
  {
    for (int x = 0; x < w; x++)
    {
      const int minX = clampi(x - a_windowRadius, 0, w - 1);
      const int maxX = clampi(x + a_windowRadius, 0, w - 1);

      const int minY = clampi(y - a_windowRadius, 0, h - 1);
      const int maxY = clampi(y + a_windowRadius, 0, h - 1);

      const float4 c0 = in_buff[y*w + x];
      const float4 n0 = nd_buff[y*w + x];
      //const float4 t0 = in_texc[y*w + x];

      float ppSize = 1.0f*float(a_windowRadius)*projectedPixelSize(n0.w, m_fov, float(w), float(h));

      int counterPass = 0;

      float fSum = 0.0f;
      float4 result(0, 0, 0, 0);

      // do window
      //
      for (int y1 = minY; y1 <= maxY; y1++)
      {
        for (int x1 = minX; x1 <= maxX; x1++)
        {
          const float4 c1 = in_buff[y1*w + x1];
          const float4 n1 = nd_buff[y1*w + x1];
          //const float4 t1 = in_texc[y1*w + x1];

          const int i = x1 - x;
          const int j = y1 - y;

          const float match = surfaceSimilarity(n0, n1, ppSize);

          const float w1 = NLMWeight(in_buff, w, h, x, y, x1, y1, a_blockRadius);
          const float wt = NLMWeight(in_texc, w, h, x, y, x1, y1, a_blockRadius);
          //const float w1 = dot3(c1-c0, c1-c0);
          //const float wt = dot3(t1-t0, t1-t0);

          const float w2 = exp(-(w1*g_NoiseLevel + (i * i + j * j) * g_GaussianSigma));
          const float w3 = exp(-(wt*g_NoiseLevel + (i * i + j * j) * g_GaussianSigma));

          const float wx = w2*w3*clamp(match, 0.25f, 1.0f);

          if (wx > g_WeightThreshold)
            counterPass++;

          fSum += wx;
          result += c1 * wx;
        }
      }

      result = result * (1.0f / fSum);

      //  Now the restored pixel is ready
      //  But maybe the area is actually edgy and so it's better to take the pixel from the original image?	
      //  This test shows if the area is smooth or not
      //
      float lerpQ = (float(counterPass) > (g_CounterThreshold * windowArea)) ? 1.0f - g_LerpCoefficeint : g_LerpCoefficeint;

      //  This is the last lerp
      //  Most common values for g_LerpCoefficient = [0.85, 1];
      //  So if the area is smooth the result will be
      //  RestoredPixel*0.85 + NoisyImage*0.15
      //  If the area is noisy
      //  RestoredPixel*0.15 + NoisyImage*0.85
      //  That allows to preserve edges more thoroughly
      //
      result = lerp(result, c0, lerpQ);

      out_buff[y*w + x] = result;
    }

    #pragma omp atomic
    linesDone++;

    // int tid = omp_get_thread_num();
    // if (tid == 0 && g_progressBar != nullptr)
    // {
    //   if (linesDone > linesDonePrev + 50)
    //   {
    //     std::stringstream strOut;
    //     strOut << "NLM Denoiser: " << int(100.0f*float(linesDone) / float(h)) << " %";
    //     auto myStr = strOut.str();
    //     g_progressBar(myStr.c_str(), float(linesDone) / float(h));
    //     linesDonePrev = linesDone;
    //   }
    // }
  }

}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*

NonLocalMeansLDR::NonLocalMeansLDR(Bitmap* a_bmap_out) : runNow(false), m_noiseLvl(0.15f), m_bmapInLayers(a_bmap_out)
{

}

NonLocalMeansLDR::~NonLocalMeansLDR()
{

}

void NonLocalMeansLDR::SetPresets(const char* a_presetsStr)
{
  std::istringstream iss(a_presetsStr);

  do
  {
    std::string name, val;
    iss >> name >> val;

    if (name == "finalRun")
      runNow = (bool)(atoi(val.c_str()));
   
    if (name == "noiseLvl")
      m_noiseLvl = atof(val.c_str());

  } while (iss);

}

void NonLocalMeansLDR::Release()
{

}

bool NonLocalMeansLDR::Eval()
{
  if (m_bmapInLayers == nullptr)
    return false;

  if (!runNow)
    return true;

  if (m_noiseLvl <= 0.0000001f)
    return true;

  ULONG ctype;
  float* zbuffer     = (float*)  m_bmapInLayers->GetChannel(BMM_CHAN_Z, ctype);
  DWORD* nbuffer     = (DWORD*)  m_bmapInLayers->GetChannel(BMM_CHAN_NORMAL, ctype);
  Color24* texcolors = (Color24*)m_bmapInLayers->GetChannel(BMM_CHAN_COLOR, ctype);

  if (zbuffer == nullptr || nbuffer == nullptr || texcolors == nullptr)
    return false;

  // copy aux layers from max to HDRImage4f
  //
  HDRImage4f normDepth(m_width, m_height);
  HDRImage4f texColors(m_width, m_height);

  float* normd = normDepth.data();
  float* texcl = texColors.data();

  for (int y = 0; y < m_height; y++)
  {
    for (int x = 0; x < m_width; x++)
    {
      const int index  = y*m_width + x;
      const int index2 = (m_height - y - 1)*m_width + x;

      Point3 p;
      p = DeCompressNormal(nbuffer[index2]);

      normd[index * 4 + 0] = p.x;
      normd[index * 4 + 1] = p.y;
      normd[index * 4 + 2] = p.z;
      normd[index * 4 + 3] = zbuffer[index2];

      Color24 c;
      c = texcolors[index2];

      const float mult = 1.0f / 255.0f;

      texcl[index * 4 + 0] = float(c.r)*mult;
      texcl[index * 4 + 1] = float(c.g)*mult;
      texcl[index * 4 + 2] = float(c.b)*mult;
      texcl[index * 4 + 3] = 0.0f;
    }
  }

  g_progressBar = m_pProgressBar;

  HDRImage4f inImage(m_width, m_height, m_data);
  HDRImage4f outImage = inImage;

  inImage.medianFilter(10.0f*0.35f, 0.0f);

  NonLocalMeansGuidedTexNormDepthFilter(inImage, texColors, normDepth, outImage, 7, 1, m_noiseLvl);

  float* res = outImage.data();

  for (int y = 0; y < m_height; y++)
  {
    for (int x = 0; x < m_width; x++)
    {
      const int index = y*m_width + x;

      m_data[index * 4 + 0] = res[index * 4 + 0];
      m_data[index * 4 + 1] = res[index * 4 + 1];
      m_data[index * 4 + 2] = res[index * 4 + 2];
    }
  }

  return true;
}

void NonLocalMeansLDR::SetInput_f4(const float* a_input, int w, int h, const char* a_slotName)
{
  m_width  = w;
  m_height = h;
}

void NonLocalMeansLDR::SetOutput_f4(float* a_output, int w, int h)
{
  m_width  = w;
  m_height = h;
  m_data   = a_output;
}


*/

