#pragma once

#include <math.h>
#include <xmmintrin.h>

typedef struct
{
  float fPowKLow;     // fPowKLow = pow( 2.0f, kLow)
  float fPowKHigh;    // fPowKHigh = pow( 2.0f, kHigh)
  float fPow35;       // fPow35 = pow( 2.0f, 3.5f)
  float fFStops;      // F stops
  float fFStopsInv;   // Invesrse fFStops value
  float fPowExposure; // fPowExposure = pow( 2.0f, exposure +  2.47393f )
  float fGamma;       // Gamma correction parameter
  float fPowGamma;    // Scale factor
  float fDefog;       // Defog value

} CHDRData;

void EvaluateRaw(const float* inputArray, float* outputArray, CHDRData *pData, int arrayWidth, int iRow);
void ExecuteToneMappingExample(const float* p_input, float* p_output, CHDRData *pData, unsigned int width, unsigned int height);
float resetFStopsParameter(float powKLow, float kHigh);

CHDRData calcPresets(float kLow = -3.0f, float kHigh = 7.5f, float exposure = 3.0f, float defog = 0.0f);

