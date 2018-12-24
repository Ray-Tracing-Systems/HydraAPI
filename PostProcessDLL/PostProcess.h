#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdint>
#include <stdlib.h>
#include <ctime>
#include <iomanip>
#include <omp.h>
#include <cmath>
#include <math.h>

#include "hydra_api/LiteMath.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

using HydraLiteMath::float3;
using HydraLiteMath::float4;

int FloatToInt(const float inData)
{
  return int(inData + 0.5f);
}

float Luminance(const float3* data)
{
  return (0.2126f * data->x + 0.7152f * data->y + 0.0722f * data->z);
}
float Luminance(const float4* data)
{
  return (0.2126f * data->x + 0.7152f * data->y + 0.0722f * data->z);
}

void SpectralToRGB(float &r, float &g, float &b, const float l) // RGB <0,1> <- lambda l <400,700> [nm]
{
  float t;
  r = 0.0;
  g = 0.0;
  b = 0.0;

  if ((l >= 400.0f) && (l < 410.0f)) { t = (l - 400.0f) / (410.0f - 400.0f); r = +(0.33f * t) - (0.20f * t * t); }
  else if ((l >= 410.0f) && (l < 475.0f)) { t = (l - 410.0f) / (475.0f - 410.0f); r = 0.14f - (0.13f * t * t); }
  else if ((l >= 545.0f) && (l < 595.0f)) { t = (l - 545.0f) / (595.0f - 545.0f); r = +(1.98f * t) - (t * t); }
  else if ((l >= 595.0f) && (l < 650.0f)) { t = (l - 595.0f) / (650.0f - 595.0f); r = 0.98f + (0.06f * t) - (0.40f * t * t); }
  else if ((l >= 650.0f) && (l < 700.0f)) { t = (l - 650.0f) / (700.0f - 650.0f); r = 0.65f - (0.84f * t) + (0.20f * t * t); }
  if ((l >= 415.0f) && (l < 475.0f)) { t = (l - 415.0f) / (475.0f - 415.0f); g = +(0.80f * t * t); }
  else if ((l >= 475.0f) && (l < 590.0f)) { t = (l - 475.0f) / (590.0f - 475.0f); g = 0.8f + (0.76f * t) - (0.80f * t * t); }
  else if ((l >= 585.0f) && (l < 639.0f)) { t = (l - 585.0f) / (639.0f - 585.0f); g = 0.84f - (0.84f * t); }
  if ((l >= 400.0f) && (l < 475.0f)) { t = (l - 400.0f) / (475.0f - 400.0f); b = +(2.20f * t) - (1.50f * t * t); }
  else if ((l >= 475.0f) && (l < 560.0f)) { t = (l - 475.0f) / (560.0f - 475.0f); b = 0.7f - (t)+(0.30f * t * t); }
}

void ConvertRgbToHsv(float4* data)
{
  #define MIN3(x,y,z)  ((y) <= (z) ? ((x) <= (y) ? (x) : (y)) : ((x) <= (z) ? (x) : (z)))
  #define MAX3(x,y,z)  ((y) >= (z) ? ((x) >= (y) ? (x) : (y)) : ((x) >= (z) ? (x) : (z)))

  float r = data->x;
  float g = data->y;
  float b = data->z;

  r *= 255.0f;
  g *= 255.0f;
  b *= 255.0f;

  float rgb_min = MIN3(r, g, b);
  float rgb_max = MAX3(r, g, b);

  float H = 0.0f;
  float S = 0.0f;
  float V = rgb_max;

  // Норм. значение 1
  if (V != 0.0f)
  {
    r /= V;
    g /= V;
    b /= V;
  }

  rgb_min = MIN3(r, g, b);
  rgb_max = MAX3(r, g, b);

  S = rgb_max - rgb_min;

  if (S != 0.0f)
  {
    // Насыщение 
    r = (r - rgb_min) / S;
    g = (g - rgb_min) / S;
    b = (b - rgb_min) / S;
  }

  rgb_min = MIN3(r, g, b);
  rgb_max = MAX3(r, g, b);

  if (rgb_max == r)
  {
    H = 0.0f + 60.0f*(g - b);
  }
  else if (rgb_max == g)
  {
    H = 120.0f + 60.0f*(b - r);
  }
  else /* rgb_max == *b */
  {
    H = 240.0f + 60.0f*(r - g);
  }

  if (H < 0.0f)   H += 360.0f;
  if (H > 360.0f) H -= 360.0f;

  V /= 255.0f;

  data->x = H;
  data->y = S;
  data->z = V;
}
void ConvertHsvToRgb(float4* data)
{
  float h = data->x;
  float s = data->y;
  float v = data->z;

  float hh, p, q, t, ff, r, g, b;
  int i;

  if (s <= 0.0f) // < is bogus, just shuts up warnings
  {
    r = v;
    g = v;
    b = v;
  }

  hh = h;

  if (hh >= 360.0f) hh = 0.0f;
  if (hh < 0.0f)    hh = 359.9999f;

  hh /= 60.0f;
  i = (int)hh;
  ff = hh - i;
  p = v * (1.0f - s);
  q = v * (1.0f - (s * ff));
  t = v * (1.0f - (s * (1.0f - ff)));

  switch (i)
  {
  case 0:
    r = v;
    g = t;
    b = p;
    break;
  case 1:
    r = q;
    g = v;
    b = p;
    break;
  case 2:
    r = p;
    g = v;
    b = t;
    break;
  case 3:
    r = p;
    g = q;
    b = v;
    break;
  case 4:
    r = t;
    g = p;
    b = v;
    break;
  case 5:
  default:
    r = v;
    g = p;
    b = q;
    break;
  }

  data->x = r;
  data->y = g;
  data->z = b;
}

template <typename T>
void ConvertRgbToXyz65(T* data)
{
  float var_R = data->x; //var_R from 0 to 1
  float var_G = data->y; //var_G from 0 to 1
  float var_B = data->z; //var_B from 0 to 1

  if (var_R > 0.04045f) var_R = pow((var_R + 0.055f) / 1.055f, 2.4f);
  else                  var_R = var_R / 12.92f;

  if (var_G > 0.04045f) var_G = pow((var_G + 0.055f) / 1.055f, 2.4f);
  else                  var_G = var_G / 12.92f;

  if (var_B > 0.04045f) var_B = pow((var_B + 0.055f) / 1.055f, 2.4f);
  else                  var_B = var_B / 12.92f;

  var_R *= 100.0f;
  var_G *= 100.0f;
  var_B *= 100.0f;

  //Chromatic adaptation. Observer. = 2°, Illuminant = D65

  data->x = var_R * 0.4124564f + var_G * 0.3575761f + var_B * 0.1804375f;
  data->y = var_R * 0.2126729f + var_G * 0.7151522f + var_B * 0.0721750f;
  data->z = var_R * 0.0193339f + var_G * 0.1191920f + var_B * 0.9503041f;
}
void ConvertXyz65ToRgb(float4* data)
{
  float var_X = data->x / 100.0f;  //X from 0 to  95.047      (Observer = 2°, Illuminant = D65)
  float var_Y = data->y / 100.0f;  //Y from 0 to 100.000
  float var_Z = data->z / 100.0f;  //Z from 0 to 108.883

                                   //Chromatic adaptation.

  float R = var_X * 3.2404542f + var_Y * -1.5371385f + var_Z * -0.4985314f;
  float G = var_X * -0.9692660f + var_Y * 1.8760108f + var_Z * 0.0415560f;
  float B = var_X * 0.0556434f + var_Y * -0.2040259f + var_Z * 1.0572252f;

  if (R > 0.0031308f) R = 1.055f * (pow(R, 0.4166666666f)) - 0.055f; // 1.0f / 2.4f = 0.4166666666f
  else                R = 12.92f * R;
  if (G > 0.0031308f) G = 1.055f * (pow(G, 0.4166666666f)) - 0.055f;
  else                G = 12.92f * G;
  if (B > 0.0031308f) B = 1.055f * (pow(B, 0.4166666666f)) - 0.055f;
  else                B = 12.92f * B;

  data->x = R;
  data->y = G;
  data->z = B;
}

void ConvertSrgbToXyz(float3* data)
{
  const float R = data->x;
  const float G = data->y;
  const float B = data->z;

  data->x = R * 0.4124564f + G * 0.3575761f + B * 0.1804375f;
  data->y = R * 0.2126729f + G * 0.7151522f + B * 0.0721750f;
  data->z = R * 0.0193339f + G * 0.1191920f + B * 0.9503041f;
}
void ConvertSrgbToXyz(float4* data)
{
  const float R = data->x;
  const float G = data->y;
  const float B = data->z;

  data->x = R * 0.4124564f + G * 0.3575761f + B * 0.1804375f;
  data->y = R * 0.2126729f + G * 0.7151522f + B * 0.0721750f;
  data->z = R * 0.0193339f + G * 0.1191920f + B * 0.9503041f;
}
void ConvertXyzToSrgb(float4* data)
{
  const float X = data->x;
  const float Y = data->y;
  const float Z = data->z;

  data->x = X * 3.2404542f + Y * -1.5371385f + Z * -0.4985314f;
  data->y = X * -0.9692660f + Y * 1.8760108f + Z * 0.0415560f;
  data->z = X * 0.0556434f + Y * -0.2040259f + Z * 1.0572252f;
}

void ConvertXyzToLab(float3* data)
{
  float var_X = data->x / 95.0470f;   //Observer= 2°, Illuminant= D65 
  float var_Y = data->y / 100.000f;   //
  float var_Z = data->z / 108.883f;   //

  if (var_X > 0.008856f) var_X = pow(var_X, 0.33333333f);           //1.0f / 3.0f    = 0.33333333f
  else                   var_X = (7.787037f * var_X) + 0.13793103f; //16.0f / 116.0f = 0.13793103f
  if (var_Y > 0.008856f) var_Y = pow(var_Y, 0.33333333f);
  else                   var_Y = (7.787037f * var_Y) + 0.13793103f;
  if (var_Z > 0.008856f) var_Z = pow(var_Z, 0.33333333f);
  else                   var_Z = (7.787037f * var_Z) + 0.13793103f;

  data->x = 116.0f *  var_Y - 16.0f;  // CIE-L
  data->y = 500.0f * (var_X - var_Y); // CIE-a
  data->z = 200.0f * (var_Y - var_Z); // CIE-b
}
void ConvertXyzToLab(float4* data)
{
  float var_X = data->x / 95.0470f;   //Observer= 2°, Illuminant= D65 
  float var_Y = data->y / 100.000f;   //
  float var_Z = data->z / 108.883f;   //

  if (var_X > 0.008856f) var_X = pow(var_X, 0.33333333f);           //1.0f / 3.0f    = 0.33333333f
  else                   var_X = (7.787037f * var_X) + 0.13793103f; //16.0f / 116.0f = 0.13793103f
  if (var_Y > 0.008856f) var_Y = pow(var_Y, 0.33333333f);
  else                   var_Y = (7.787037f * var_Y) + 0.13793103f;
  if (var_Z > 0.008856f) var_Z = pow(var_Z, 0.33333333f);
  else                   var_Z = (7.787037f * var_Z) + 0.13793103f;

  data->x = 116.0f *  var_Y - 16.0f;  // CIE-L
  data->y = 500.0f * (var_X - var_Y); // CIE-a
  data->z = 200.0f * (var_Y - var_Z); // CIE-b
}
void ConvertLabToXyz(float4* data)
{
  float var_Y = (data->x + 16.0f) / 116.0f;
  float var_X = var_Y + data->y / 500.0f;
  float var_Z = var_Y - data->z / 200.0f;

  float pow3var_X = var_X * var_X * var_X;
  float pow3var_Y = var_Y * var_Y * var_Y;
  float pow3var_Z = var_Z * var_Z * var_Z;

  if (pow3var_X > 0.008856f) var_X = pow3var_X;
  else                       var_X = (var_X - 0.13793103f) / 7.787f;  //16.0f / 116.0f = 0.13793103f
  if (pow3var_Y > 0.008856f) var_Y = pow3var_Y;
  else                       var_Y = (var_Y - 0.13793103f) / 7.787f;
  if (pow3var_Z > 0.008856f) var_Z = pow3var_Z;
  else                       var_Z = (var_Z - 0.13793103f) / 7.787f;

  data->x = var_X * 95.047f;   // Observer= 2°, Illuminant= D65
  data->y = var_Y * 100.0f;    //
  data->z = var_Z * 108.883f;  //
}

void MatrixCat02(float3* data)
{
  // Преобразование в колбочковые ответы. Mcat02 in CAM02. 
  const float R = 0.7328f * data->x + 0.4296f * data->y + -0.1624f * data->z;
  const float G = -0.7036f * data->x + 1.6975f * data->y + 0.0061f * data->z;
  const float B = 0.0030f * data->x + 0.0136f * data->y + 0.9834f * data->z;

  data->x = R;
  data->y = G;
  data->z = B;
}
void MatrixCat02(float4* data)
{
  // Преобразование в колбочковые ответы. Mcat02 in CAM02. 

  const float R = 0.7328f * data->x + 0.4296f * data->y + -0.1624f * data->z;
  const float G = -0.7036f * data->x + 1.6975f * data->y + 0.0061f * data->z;
  const float B = 0.0030f * data->x + 0.0136f * data->y + 0.9834f * data->z;

  data->x = R;
  data->y = G;
  data->z = B;
}
template <typename T>
void InverseMatrixCat02(T* data)
{
  // Inverse Mcat02 in CAM02. 
  const float X = 1.096124f * data->x + -0.278869f * data->y + 0.182745f * data->z;
  const float Y = 0.454369f * data->x + 0.473533f * data->y + 0.072098f * data->z;
  const float Z = -0.009628f * data->x + -0.005698f * data->y + 1.015326f * data->z;

  data->x = X;
  data->y = Y;
  data->z = Z;
}

template <typename T>
void MatrixHpe(T* data)
{
  // конверт в колбочки. Matrix HPE in CAM02. (Hunt-Pointer-Estevez)
  const float L = 0.38971f * data->x + 0.68898f * data->y + -0.07868f * data->z; //Kuo modify −0.07869
  const float M = -0.22981f * data->x + 1.18340f * data->y + 0.04641f * data->z; //            0.04642f ?
  const float S = data->z;

  data->x = L;
  data->y = M;
  data->z = S;
}
void InverseMatrixHpe(float4 * data)
{
  // Inverse matrix HPE in CAM02. 
  const float R = 1.910197f * data->x + -1.112124f * data->y + 0.201908f * data->z;
  const float G = 0.370950f * data->x + 0.629054f * data->y + -0.000008f * data->z;
  const float B = data->z;

  data->x = R;
  data->y = G;
  data->z = B;
}

void ConvertXyzToLms(float4* data)
{
  float L = 0.4002f * data->x + 0.7075f * data->y + -0.0807f * data->z;
  float M = -0.2280f * data->x + 1.1500f * data->y + 0.0612f * data->z;
  float S = 0.9184f * data->z;

  //const float a = 0.43f;
  //
  //if      (L >= 0.0f) L =  pow( L, a);
  //else if (L <  0.0f) L = -pow(-L, a);
  //if      (M >= 0.0f) M =  pow( M, a);
  //else if (M <  0.0f) M = -pow(-M, a);
  //if      (S >= 0.0f) S =  pow( S, a);
  //else if (S <  0.0f) S = -pow(-S, a);

  data->x = L;
  data->y = M;
  data->z = S;
}
void ConvertLmsToXyz(float4* data)
{
  float L = data->x;
  float M = data->y;
  float S = data->z;

  data->x = 1.8493f * L + -1.1383f * M + 0.2381f * S;
  data->y = 0.3660f * L + 0.6444f * M + -0.010f  * S;
  data->z = 1.0893f * S;
}

void ConvertXyzToLmsPower(float4 * data, const float power)
{
  float L = 0.4002f * data->x + 0.7075f * data->y + -0.0807f * data->z;
  float M = -0.2280f * data->x + 1.1500f * data->y + 0.0612f * data->z;
  float S = 0.9184f * data->z;

  const float a = power;

  if (L >= 0.0f) L = pow(L, a);
  else if (L <  0.0f) L = -pow(-L, a);
  if (M >= 0.0f) M = pow(M, a);
  else if (M <  0.0f) M = -pow(-M, a);
  if (S >= 0.0f) S = pow(S, a);
  else if (S <  0.0f) S = -pow(-S, a);

  data->x = L;
  data->y = M;
  data->z = S;
}
void ConvertLmsToXyzPower(float4* data)
{
  float L = data->x;
  float M = data->y;
  float S = data->z;

  const float a = 2.3255819f; //  = 1.0f / 0.43f;

  if (L >= 0.0f) L = pow(L, a);
  else if (L <  0.0f) L = -pow(-L, a);
  if (M >= 0.0f) M = pow(M, a);
  else if (M <  0.0f) M = -pow(-M, a);
  if (S >= 0.0f) S = pow(S, a);
  else if (S <  0.0f) S = -pow(-S, a);

  data->x = 1.8493f * L + -1.1383f * M + 0.2381f * S;
  data->y = 0.3660f * L + 0.6444f * M + -0.010f  * S;
  data->z = 1.0893f * S;
}

void ConvertLmsToIpt(float4* data)
{
  const float I = 0.4000f * data->x + 0.4000f * data->y + 0.2000f * data->z;
  const float P = 4.4550f * data->x + -4.8510f * data->y + 0.3960f * data->z;
  const float T = 0.8056f * data->x + 0.3572f * data->y + -1.1628f * data->z;

  data->x = I;
  data->y = P;
  data->z = T;
}
void ConvertIptToLms(float4* data)
{
  const float L = 0.9999f * data->x + 0.0970f * data->y + 0.2053f * data->z;
  const float M = 0.9999f * data->x + -0.1138f * data->y + 0.1332f * data->z;
  const float S = 0.9999f * data->x + 0.0325f * data->y + -0.6768f * data->z;

  data->x = L;
  data->y = M;
  data->z = S;
}

void ChromAdaptIcam(float4* data, float3 dataW, float3 d65, const float D)
{
  MatrixCat02(data);

  const float Rw = dataW.x + 0.0000001f;
  const float Gw = dataW.y + 0.0000001f;
  const float Bw = dataW.z + 0.0000001f;

  const float Rc = (d65.x * D / Rw + 1.0f - D) * data->x;
  const float Gc = (d65.y * D / Gw + 1.0f - D) * data->y;
  const float Bc = (d65.z * D / Bw + 1.0f - D) * data->z;

  data->x = Rc;
  data->y = Gc;
  data->z = Bc;

  InverseMatrixCat02(data);
}



void Blend(float& inData1, const float& inData2, const float coeff) // 0 - data1, 1 - data2
{
  inData1 = inData1 + (inData2 - inData1) * coeff;
}
template <typename T>
void Normalize(T& data, const float inMin, const float inMax, const float outMin, const float outMax)
{
  // Формула нормализации y = (inCurrent - inMin) * (outMax - outMin) / (inMax - inMin) + outMin
  // inCurrent - значение, подлежащее нормализации
  // [Xmin, Xmax] - интервал значений х
  // [outMin, outMax] - Интервал, к которому будет приведено значение x
  data = (data - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}
void OffsetCenter(float& data, const float inMin, const float inMax, const float coef)
{
  // Ф-ция смещения центра диапазона на coef
  Normalize(data, inMin, inMax, 0, 1);  // Нормализуем в 0 - 1

  data = pow(data, coef);

  Normalize(data, 0, 1, inMin, inMax);  // Нормализуем обратно
}
void ContrastField(float& data)
{
  if (data > 0.0f && data < 1.0f)
  {
    const float a = data * 1.5707f;
    data = sin(a) * sin(a);
  }
}
void Compress(float& data, const float maxRgb)
{
  data = (data * (1.0f + data / (maxRgb * maxRgb))) / (1.0f + data);
}
void CompressHsv(float4* hsvData)
{
  // Сжимаем яркость 
  hsvData->z = hsvData->z / (1.0f + hsvData->z);


  // Смещение тона по аддитивному закону в зависимости от яркости.
  float coefOffsetHue = 1.0f - pow(hsvData->z, 10.0f);
  //coefOffsetHue = 1.0f; // turned off

  // От красного (360) до пурпурного (300) сжимаются к пурпурному.
  if (hsvData->x < 360 && hsvData->x >= 300)
    OffsetCenter(hsvData->x, 360, 300, coefOffsetHue);

  // От пурпурного (300) до синего (240) сжимаются к пурпурному.
  else if (hsvData->x < 300 && hsvData->x >= 240)
    OffsetCenter(hsvData->x, 240, 300, coefOffsetHue);

  // От синего (240) до бирюзового (180) сжимаются к бирюзовому
  else if (hsvData->x < 240 && hsvData->x >= 180)
    OffsetCenter(hsvData->x, 240, 180, coefOffsetHue);

  // От бирюзового (180) до зелёного (120) сжимаются к бирюзовому
  else if (hsvData->x < 180 && hsvData->x >= 120)
    OffsetCenter(hsvData->x, 120, 180, coefOffsetHue);

  // От зелёного (120) до жёлтого (60) сжимаются к жёлтому.
  else if (hsvData->x < 120 && hsvData->x >= 60)
    OffsetCenter(hsvData->x, 120, 60, coefOffsetHue);

  // От красного (0) до жёлтого (60) сжимаются к жёлтому
  else if (hsvData->x < 60 && hsvData->x >= 0)
    OffsetCenter(hsvData->x, 0, 60, coefOffsetHue);
}
void CalculateHistogram(const float& data, float histogram[], const int histogramBin, const float iterrHistogramBin)
{
  if (data <= 1.0f)
  {
    // Current bin.      
    const int currentHistogramBin = (int)((data * float(histogramBin - 1)) + 0.5f);

    // Increase count currents bin.
    if (currentHistogramBin >= 0 && currentHistogramBin < histogramBin)
      histogram[currentHistogramBin] += iterrHistogramBin;
  }
}
void UniformHistogram(float histogram[], const int histogramBin)
{
  // Recalculate histogramm
  for (int i = 1; i < histogramBin; i++)
  {
    histogram[i] = histogram[i - 1] + histogram[i]; // cummulative uniform
  }
}
void NormalizeHistogram(float histogram[], const int histogramBin)
{
  float histMin = 999999.9f;
  float histMax = 0.0f;

  // Calculate min and max value
  //#pragma omp parallel for
  for (int i = 0; i < histogramBin; ++i)
  {
    if (histogram[i] < histMin && histogram[i] != 0)
      histMin = histogram[i];

    if (histogram[i] > histMax)
      histMax = histogram[i];
  }

  // Normalize to 0 - 1
  #pragma omp parallel for
  for (int i = 0; i < histogramBin; ++i)
  {
    Normalize(histogram[i], histMin, histMax, 0.0f, 1.0f);
  }
}
void UniformContrastRgb(float* data, const int sizeImage, const int histogramBin)
{
  float histMin = 9999999.9f;
  float histMax = 0.0f;

  const float iterrHistogramBin = 1.0f / (float)(sizeImage * 3);

  std::vector<float> histogram(histogramBin);

  // --- Cчитаем гистограмму ---
  //#pragma omp parallel for
  for (int i = 0; i < sizeImage * 3; i++)
  {
    if (data[i] <= 1.0f)
    {
      // Текущая корзина.      
      const int currentHistogramBin = (int)round(data[i] * float(histogramBin - 1));

      // Увеличиваем счётчик текущей корзины.
      if (currentHistogramBin >= 0 && currentHistogramBin < histogramBin)
        histogram[currentHistogramBin] += iterrHistogramBin;
    }
  }

  // Пересчитываем гистограмму (перераспределение)
  for (int i = 1; i < histogramBin; i++)
    histogram[i] = histogram[i - 1] + histogram[i];

  // // Находим мин и макс значение гистограммы
  //#pragma omp parallel for
  for (int i = 0; i < histogramBin; i++)
  {
    if (histogram[i] < histMin && histogram[i] != 0)
      histMin = histogram[i];

    if (histogram[i] > histMax)
      histMax = histogram[i];
  }

  // Нормализуем гистограмму к 0 - 1 и выводим
  #pragma omp parallel for
  for (int i = 0; i < histogramBin; i++)
    Normalize(histogram[i], histMin, histMax, 0.0f, 1.0f);

  // Присваиваем гистограмму яркости.
  #pragma omp parallel for
  for (int i = 0; i < sizeImage * 3; i++)
  {
    if (data[i] <= 1.0f)
    {
      const int currentHistogramBin = (int)round(data[i] * float(histogramBin - 1));

      if (currentHistogramBin >= 0 && currentHistogramBin < histogramBin)
        data[i] = pow(histogram[currentHistogramBin], 2.2f);
      else
        data[i] = 0.0f;
    }
  }
}
void AssignHistogramToData(float& data, const float* histogram, const int histogramBin)
{
  if (data <= 1.0f)
  {
    // Assign the histogram to the data->
    const int currentHistogramBin = (int)round(data * float(histogramBin - 1));

    if (currentHistogramBin >= 0 && currentHistogramBin < histogramBin)
      data = pow(histogram[currentHistogramBin], 2.2f);
    else
      data = 0.0f;
  }
}
void UniformContrastField0(float4* data, const int sizeImage, const int histogramBin)
{
  #define MAX3(x,y,z)  ((y) >= (z) ? ((x) >= (y) ? (x) : (y)) : ((x) >= (z) ? (x) : (z)))

  float histMin = 10000000.0f;
  float histMax = 0.0f;

  const float iterrHistogramBin = 1.0f / (float)(sizeImage);

  std::vector<float> histogram(histogramBin);

  // --- Cчитаем гистограмму ---
  //#pragma omp parallel for
  for (int i = 0; i < sizeImage; i++)
  {
    data[i].x = data[i].x / (1.0f + data[i].x);

    // Текущая корзина.      
    const int currentHistogramBin = (int)round(data[i].x * float(histogramBin - 1));

    // Увеличиваем счётчик текущей корзины.
    if (currentHistogramBin >= 0 && currentHistogramBin < histogram.size())
      histogram[currentHistogramBin] += iterrHistogramBin;
  }

  // Пересчитываем гистограмму (перераспределение)
  for (int i = 1; i < histogramBin; i++)
    histogram[i] = histogram[i - 1] + histogram[i];

  // Находим мин и макс значение гистограммы
  #pragma omp parallel for
  for (int i = 0; i < histogramBin; i++)
  {
    if (histogram[i] < histMin && histogram[i] != 0)
      histMin = histogram[i];

    if (histogram[i] > histMax)
      histMax = histogram[i];
  }

  // Нормализуем гистограмму к 0 - 1 и выводим
  #pragma omp parallel for
  for (int i = 0; i < histogramBin; i++)
    Normalize(histogram[i], histMin, histMax, 0.0f, 1.0f);

  // Присваиваем гистограмму яркости.
  #pragma omp parallel for
  for (int i = 0; i < sizeImage; i++)
  {
    const int currentHistogramBin = (int)round(data[i].x * float(histogramBin - 1));

    if (currentHistogramBin >= 0 && currentHistogramBin < histogram.size())
      data[i].x = pow(histogram[currentHistogramBin], 2.2f);
    else
      data[i].x = 0.0f;
  }
}
float Distance(const float x1, const float y1, const float x2, const float y2)
{
  float differenceX = x1 - x2;
  float differenceY = y1 - y2;

  float distance = sqrt(differenceX * differenceX + differenceY * differenceY);
  return distance;
}
void Bilinear(const float inData, float outData[], float newPosX, float newPosY, const int m_width, const int m_height, const int sizeImage)
{
  // |------|------|
  // | dxy3 | dxy4 |
  // |------|------|
  // | dxy1 | dxy2 |
  // |------|------|

  const int floorY = (int)floor(newPosY);
  const int floorX = (int)floor(newPosX);

  const int     dxy1 = floorY * m_width + floorX;
  int           dxy2 = dxy1 + 1;
  if (dxy1 < 0) dxy2 = dxy1 - 1;
  const int     dxy3 = (floorY + 1) * m_width + floorX;
  int           dxy4 = dxy3 + 1;
  if (dxy3 < 0) dxy4 = dxy3 - 1;

  float dx = newPosX - (int)newPosX;
  float dy = newPosY - (int)newPosY;

  if (dx < 0.0f) dx = 1.0f - abs(dx);
  if (dy < 0.0f) dy = 1.0f - abs(dy);

  float multBrightness1 = (1.0f - dx) * (1.0f - dy);
  float multBrightness2 = dx * (1.0f - dy);
  float multBrightness3 = dy * (1.0f - dx);
  float multBrightness4 = dx * dy;

  if (floorY >= 0.0f && floorX >= 0.0f && floorY < m_height - 1 && floorX < m_width - 1)
  {
    outData[dxy1] += inData * multBrightness1;
    outData[dxy2] += inData * multBrightness2;
    outData[dxy3] += inData * multBrightness3;
    outData[dxy4] += inData * multBrightness4;
  }
}
void ClampMinusToZero(float4* data4, const int i)
{
  if (data4[i].x < 0) data4[i].x = 0;
  if (data4[i].y < 0) data4[i].y = 0;
  if (data4[i].z < 0) data4[i].z = 0;
}
void ClampMinusToZero(float4* data4)
{
  if (data4->x < 0) data4->x = 0;
  if (data4->y < 0) data4->y = 0;
  if (data4->z < 0) data4->z = 0;
}
void Vignette(float4* data4, const int m_width, const int m_height, const float a_vignette,
  const float diagonalImage, const float centerImageX, const float centerImageY,
  const float radiusImage, const int x, const int y, const int i)
{
  const float distanceFromCenter = Distance((float)x, (float)y, centerImageX, centerImageY) / radiusImage;

  float vignetteIntensity = sqrt(1 - distanceFromCenter * a_vignette);
  if (vignetteIntensity < 0.0f) vignetteIntensity = 0;

  data4[i].x *= vignetteIntensity;
  data4[i].y *= vignetteIntensity;
  data4[i].z *= vignetteIntensity;
}
void SummValueOnField(const float4 data[], float3* summRgb, const int i)
{
  #pragma omp atomic
  summRgb->x += data[i].x;
  #pragma omp atomic
  summRgb->y += data[i].y;
  #pragma omp atomic
  summRgb->z += data[i].z;
}
void ChrommAberr(const float4 inData[], float outData1[], float outData2[], const int m_width, const int m_height, const int sizeImage, const float a_chromAberr, const int x, const int y, const int i)
{
  // Генерируем карту велосити.
  // (-1, -1) - влево вниз, (0, 0) - нет движения. (1, 1) - вправо вверх.  Красный > зелёный > синий.
  const float stepVelocityX = (float)x / m_width;
  const float stepVelocityY = (float)y / m_height;
  const float velocityX = stepVelocityX * 2.0f - 1.0f; // -1.0f to 1.0f
  const float velocityY = stepVelocityY * 2.0f - 1.0f; // -1.0f to 1.0f

  const float newPosXr = x + velocityX * a_chromAberr;
  const float newPosYr = y + velocityY * a_chromAberr;
  const float newPosXg = x + velocityX * 0.5f * a_chromAberr;
  const float newPosYg = y + velocityY * 0.5f * a_chromAberr;

  // Anti-aliasing
  Bilinear(inData[i].x, outData1, newPosXr, newPosYr, m_width, m_height, sizeImage);
  Bilinear(inData[i].y, outData2, newPosXg, newPosYg, m_width, m_height, sizeImage);
}
void ComputeWhitePoint(const float3 summRgb, float3* whitePoint, const int sizeImage)
{
  whitePoint->x = summRgb.x / (float)sizeImage;
  whitePoint->y = summRgb.y / (float)sizeImage;
  whitePoint->z = summRgb.z / (float)sizeImage;
}
void MinMaxRgb(float4* data, float& minRgb, float& maxRgb, const float thresholdFloor, const int i)
{
  if (data[i].x < minRgb && data[i].x > 0.0f) minRgb = data[i].x;
  if (data[i].x > maxRgb)                     maxRgb = data[i].x;

  if (data[i].y < minRgb && data[i].y > 0.0f) minRgb = data[i].y;
  if (data[i].y > maxRgb)                     maxRgb = data[i].y;

  if (data[i].z < minRgb && data[i].z > 0.0f) minRgb = data[i].z;
  if (data[i].z > maxRgb)                     maxRgb = data[i].z;

  if (minRgb < thresholdFloor) minRgb = thresholdFloor;

  if (data[i].x < thresholdFloor) data[i].x = thresholdFloor;
  if (data[i].y < thresholdFloor) data[i].y = thresholdFloor;
  if (data[i].z < thresholdFloor) data[i].z = thresholdFloor;
}
void MinMaxArray(float& data, float& min, float& max, const float thresholdFloor, const float thresholdTop)
{
  if (data < min && data > 0.0f)  min = data;
  else if (data > max)                 max = data;

  if (min < thresholdFloor) min = thresholdFloor;
  if (max > thresholdTop)   max = thresholdTop;

  if (data < thresholdFloor) data = thresholdFloor;
  else if (data > thresholdTop)   data = thresholdTop;
}
void DrawColumn(float4 data[], const int width, const int height, const int offsetX, const int ImgWidth, const float3 color, const bool transparent)
{
  const int endA = height * ImgWidth + width + offsetX;
  for (int y = 0; y < height; ++y)
  {
    for (int x = 0; x < width; ++x)
    {
      const int a = y * ImgWidth + x + offsetX;

      if (transparent)
      {
        data[a].x = data[a].x / 2.0f + color.x * (pow((float)a / endA, 4) + 0.2f);
        data[a].y = data[a].y / 2.0f + color.y * (pow((float)a / endA, 4) + 0.2f);
        data[a].z = data[a].z / 2.0f + color.z * (pow((float)a / endA, 4) + 0.2f);
      }
      else
      {
        data[a].x = color.x;
        data[a].y = color.y;
        data[a].z = color.z;
      }
    }
  }
}
void ViewHistorgam(float4 data[], float3* histogram, const int m_width, const int m_height, const int histogramBin)
{
  const int countCol = m_width;
  const int step = FloatToInt((float)histogramBin / countCol);
  int width = FloatToInt((float)m_width / countCol);
  if (width < 1) width = 1;

  float height;
  float gamma = 0.455f;

  float3 color;
  color.x = 0;
  color.y = 0;
  color.z = 0;

  // Draw black histogram
  for (int i = 0; i < countCol; ++i)
  {
    const int j = i * step;

    histogram[j].x = pow(histogram[j].x, gamma);
    histogram[j].y = pow(histogram[j].y, gamma);
    histogram[j].z = pow(histogram[j].z, gamma);

    height = histogram[j].x * m_height;
    DrawColumn(data, width, (int)height, i * width, m_width, color, 0);

    height = histogram[j].y * m_height;
    DrawColumn(data, width, (int)height, i * width, m_width, color, 0);

    height = histogram[j].z * m_height;
    DrawColumn(data, width, (int)height, i * width, m_width, color, 0);
  }

  // Draw color histogram
  for (int i = 0; i < countCol; ++i)
  {
    const int j = i * step;

    color.x = 0.0f;
    color.y = 0.0f;
    color.z = 0.5f;
    height = histogram[j].z * m_height;
    DrawColumn(data, width, (int)height, i * width, m_width, color, 1);

    color.x = 0.5f;
    color.y = 0.0;
    color.z = 0.0;
    height = histogram[j].x * m_height;
    DrawColumn(data, width, (int)height, i * width, m_width, color, 1);

    color.x = 0.0f;
    color.y = 0.5f;
    color.z = 0.0f;
    height = histogram[j].y * m_height;
    DrawColumn(data, width, (int)height, i * width, m_width, color, 1);

  }

}
void MinMaxHistBin(const float* histogram, float& minHistBin, float& maxHistBin, const int sizeImage, const int histogramBin)
{
  int i = 0;
  const float floor = (float)sizeImage * 0.00001f; // 0.001% from image resolution

  while (histogram[i] <= floor && i < histogramBin)
  {
    minHistBin = (float)i;
    i++;
  }

  i = histogramBin - 1;

  while (histogram[i] <= floor && i >= 0)
  {
    maxHistBin = (float)i;
    i--;
  }

  minHistBin /= histogramBin;
  maxHistBin /= histogramBin;
}
float Min3(const float value1, const float value2, const float value3)
{
  if (value1 <= value2 && value1 <= value3) return value1;
  else if (value2 <= value1 && value1 <= value3) return value2;
  else                                           return value3;
}
float Max3(const float value1, const float value2, const float value3)
{
  if (value1 >= value2 && value1 >= value3) return value1;
  else if (value2 >= value1 && value1 >= value3) return value2;
  else                                           return value3;

}
void Resize(float inData[], const float sourceWidth, const float sourceHeight, const float sourceSizeImage, const float resize)
{
  const int newSizeImage = FloatToInt(sourceSizeImage * resize);
  const int newHeight = FloatToInt(sourceHeight * resize);
  const int newWidth = FloatToInt(sourceWidth * resize);

  if (resize < 1.0f)
  {
    for (int y = 0; y < newHeight; ++y)
    {
      for (int x = 0; x < newWidth; ++x)
      {
        const int i = FloatToInt(y * sourceWidth + x);
        int j = FloatToInt(y / resize * sourceWidth + x / resize);
        if (j >= sourceSizeImage) j = FloatToInt(sourceSizeImage - 1);
        inData[i] = inData[j];
      }
    }
  }
  else
  {
    float* resizeArray = (float*)calloc(newSizeImage, sizeof(float));

    for (int y = 0; y < newHeight; ++y)
    {
      for (int x = 0; x < newWidth; ++x)
      {
        const int i = (y * newWidth) + x;
        const int j = FloatToInt((y / resize * 0.99f) * newWidth + x / resize);
        resizeArray[i] = inData[j];
      }
    }


    for (int i = 0; i < newSizeImage; ++i)
    {
      inData[i] = resizeArray[i];
    }

    free(resizeArray);
    resizeArray = NULL;
  }
}
std::vector<float> createGaussKernelWeights1D_HDRImage2(const int size, const float a_sigma)
{
  std::vector<float> gKernel(size);

  // set standard deviation to 1.0
  const float sigma = a_sigma;
  const float s = 2.0f * sigma * sigma;

  // sum is for normalization
  float sum = 0.0;
  const int halfSize = size / 2;

  for (int x = -halfSize; x <= halfSize; ++x)
  {
    int index = x + halfSize;
    gKernel[index] = (exp(-abs(x) / s)) / (3.141592654f * s);
    sum += gKernel[index];
  }

  // normalize the Kernel
  for (int i = 0; i < size; ++i)
    gKernel[i] /= sum;

  return gKernel;
}
void Sharp(float4 image4out[], const float lumForSharp[], const float a_sharpness, const int a_width, const int a_height)
{
#pragma omp parallel for
  for (int y = 0; y < a_height; ++y)
  {
    for (int x = 0; x < a_width; ++x)
    {
      const int i = y * a_width + x;

      if (image4out[i].x < 0.0f) image4out[i].x = 0.0f;
      if (image4out[i].y < 0.0f) image4out[i].y = 0.0f;
      if (image4out[i].z < 0.0f) image4out[i].z = 0.0f;

      float mean = 0.0f;

      for (int i = -1; i <= 1; ++i)
      {
        for (int j = -1; j <= 1; ++j)
        {
          int Y = y + i;
          int X = x + j;

          if (Y < 0)         Y = 1;
          else if (Y >= a_height) Y = a_height - 1;
          if (X < 0)         X = 1;
          else if (X >= a_width)  X = a_width - 1;

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

          if (Y < 0)         Y = 1;
          else if (Y >= a_height) Y = a_height - 1;
          if (X < 0)         X = 1;
          else if (X >= a_width)  X = a_width - 1;

          float a = lumForSharp[Y * a_width + X] - mean;
          dispers += abs(a);
        }
      }

      dispers /= (1.0f + dispers);
      const float lumCompr = lumForSharp[i] / (1.0f + lumForSharp[i]);
      const float meanCompr = mean / (1.0f + mean);
      float hiPass = (lumForSharp[i] - mean) * 5 + 1.0f;
      hiPass = pow(hiPass, 2);

      float sharp = 1.0f;
      Blend(sharp, hiPass, a_sharpness * (1.0f - dispers));

      image4out[i].x *= sharp;
      image4out[i].y *= sharp;
      image4out[i].z *= sharp;
    }
  }
}
void Blur(float inData[], int blurRadius, const int m_width, const int m_height, const int sizeImage)
{
  const float diagonalImage = Distance(0, 0, (float)m_width, (float)m_height);
  float radiusImage = diagonalImage / 2.0f;

  float resize = 1.0f;
  if (blurRadius >= (radiusImage / 2.0f)) resize = 0.50f;
  else if (blurRadius >= radiusImage)         resize = 0.25f;

  blurRadius = FloatToInt(blurRadius * resize);
  const int colInRadius = FloatToInt((blurRadius + blurRadius + 1) / 2.0f);

  const int smallSizeImage = FloatToInt(sizeImage * resize);
  const int smallHeight = FloatToInt(m_height * resize);
  const int smallWidth = FloatToInt(m_width * resize);
  float* blurRow = (float*)calloc(FloatToInt(sizeImage * resize), sizeof(float));
  float* blurRowCol = (float*)calloc(FloatToInt(sizeImage * resize), sizeof(float));
  std::vector<float> weights = createGaussKernelWeights1D_HDRImage2(blurRadius * 2 + 1, 1.0f);

  // Downsize for up speed blur
  if (resize < 1.0f) Resize(inData, (float)m_width, (float)m_height, (float)sizeImage, resize);

  // ----- Blur -----

  // All row.  
  #pragma omp parallel for
  for (int y = 0; y < smallHeight; ++y)
  {
    for (int x = 0; x < smallWidth; ++x)
    {
      float summ = 0;

      for (int row = -blurRadius; row <= blurRadius; ++row)
      {
        int currX = x + row;
        if (currX < 0)            currX = 0;
        else if (currX >= smallWidth)  currX = smallWidth - 1;
        summ += inData[y * m_width + currX] * weights[row + blurRadius];
      }
      blurRow[y * m_width + x] = summ;
    }
  }

  // All col.
  #pragma omp parallel for
  for (int x = 0; x < smallWidth; ++x)
  {
    for (int y = 0; y < smallHeight; ++y)
    {
      float summ = 0;

      for (int col = -blurRadius; col <= blurRadius; ++col)
      {
        int currY = y + col;
        if (currY < 0)            currY = 0;
        else if (currY >= smallHeight) currY = smallHeight - 1;
        summ += blurRow[currY * m_width + x] * weights[col + blurRadius];
      }
      inData[y * m_width + x] = summ;
    }
  }

  // Upsize
  resize = 1.0f / resize;
  if (resize > 1.0f) Resize(inData, m_width / resize, m_height / resize, sizeImage / resize, resize);

  free(blurRow);
  free(blurRowCol);
  blurRow = NULL;
  blurRowCol = NULL;
}
float ConvertAngleToRad(int angle)
{
  return angle * 0.01745329252f;
}

void DiffractionStars(float4* inData, float3 diffrStars[], const float a_sizeStar, const int a_numRay, const int a_RotateRay, const float a_randomAngle,
  const float a_sprayRay, const int m_width, const int m_height, const int sizeImage, const float radiusImage, const int x, const int y, const int i)
{
  const float PI = 3.141592f;
  const float twoPI = 6.283185f;
  const float angle = twoPI / a_numRay;
  const float waveLenghtR = 0.000700f;
  const float waveLenghtY = 0.000575f;
  const float waveLenghtG = 0.000530f;
  const float waveLenghtC = 0.000485f;
  const float waveLenghtB = 0.000450f;
  const float waveLenghtV = 0.000400f;

  float lum = Luminance(&inData[i]);
  lum /= (1.0f + lum);

  // Ray
  for (int numRay = 0; numRay < a_numRay; ++numRay)
  {
    float jitter = (rand() % 100) / 100.0f + 0.5f;
    float nextAngle = angle * numRay - ConvertAngleToRad(a_RotateRay);
    if ((rand() % 1000) / 1000.0f <= a_randomAngle)
    {
      nextAngle = (rand() % 628) / 100.0f;
    }
    const float sprayAngle = ((float)(rand() % (int)(angle * 100)) / 100.0f) * a_sprayRay;
    nextAngle += (sprayAngle - angle / 2.0f);

    const float sizeStar = a_sizeStar * radiusImage / 100.0f;
    const float sizeStarFromLum = sizeStar * lum * jitter / 5.0f;

    // Aperture big   - dist ray small, wave small
    // Aperture small - dist ray big, wave big
    const float diafragma = 1.0f / (sizeStarFromLum * 5.0f * jitter);

    for (float sample = 1.0f; sample < sizeStarFromLum; sample += 0.1f)
    {
      const float angleFi = sample;
      const float temp = diafragma * angleFi;
      const float multDist = 1.0f - (float)sample / sizeStarFromLum;
      const float u = temp / waveLenghtG + 0.000001f;

      const float newX = x + cos(nextAngle) * sample;
      const float newY = y + sin(nextAngle) * sample;

      if (newX > 0 && newX < m_width && newY > 0 && newY < m_height)
      {
        const float I = pow((sinf(u) / u), 2) * multDist;
        Bilinear(inData[i].x * I, &diffrStars[0].x, newX * 3.0f, newY, m_width * 3.0f, m_height, sizeImage);
        Bilinear(inData[i].y * I, &diffrStars[0].y, newX * 3.0f, newY, m_width * 3.0f, m_height, sizeImage);
        Bilinear(inData[i].z * I, &diffrStars[0].z, newX * 3.0f, newY, m_width * 3.0f, m_height, sizeImage);
      }
    }
  }
}
