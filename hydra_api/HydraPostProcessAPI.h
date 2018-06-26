#pragma once

#include "HydraAPI.h"
#include <cstring>

/**
\brief "Frame Buffer Image" Ref.
*/
struct HRFBIRef 
{ 
  HRFBIRef() : id(-1) {} 
  int32_t id; 
};

/**
\brief Create internal "Frame Buffer Image" and return it's Reference. This function does alloc internal memory.

\param name   - any name for your custom image 
\param w      - image width
\param h      - image height
\param bpp    - image bytes per pixel. Only "16" and "4" values are allowed currently. 16 means float4, 4 - LDR image format.
\param a_data - optional parameter. input data in float4 (bpp == 16) or i1 (bpp == 4) formats.

\return reference to created frame buffer

*/
HRFBIRef hrFBICreate(const wchar_t* name, int w, int h, int bpp, const void* a_data = nullptr);

/**
\brief Create internal "Frame Buffer Image" from file return it's Reference. This function does alloc internal memory.

\param a_fileName   - input file name
\param a_desiredBpp - optional parameter. Put 16 or 4 if you need to have predefined format. For example put 16 if you strictly need image4f.
\return reference to created frame buffer

*/
HRFBIRef hrFBICreateFromFile(const wchar_t* a_fileName, int a_desiredBpp = -1, float a_inputGamma = 2.2f);

/**
\brief Save frame buffer image to file. Perform gamma correction also!!!
\param a_image    - input image reference
\param a_fileName - input file name
\param a_gamma    - optional parameter. gamma. used if hdr image is saved to LDR format.
*/
void hrFBISaveToFile(HRFBIRef a_image, const wchar_t* a_fileName, const float a_gamma = 2.2f);

/**
\brief Load frame buffer image from file
\param a_image    - input image reference
\param a_fileName - input file name
\param a_desiredBpp - optional parameter. Put 16 or 4 if you need to have predefined format. For example put 16 if you strictly need image4f.
\param a_inputGamma - optional parameter. You may specify gamma if you want to load ldr file to hdr image

*/
void hrFBILoadFromFile(HRFBIRef a_image, const wchar_t* a_fileName, int a_desiredBpp = -1, float a_inputGamma = 2.2f);

/**
\brief Load frame buffer image from file
\param a_image - input image reference
\param pW      - output image width;
\param pH      - output image height;
\param pBpp    - output image bytes per pixel;
\return pointer to raw data with pitch == width of image and bpp == (16 or 4). 

*/
const void* hrFBIGetData(HRFBIRef a_image, 
                         int* pW, int* pH, int* pBpp);

/**
\brief Resize internal "Frame Buffer Image". This functions does realloc internal memory.

\param a_image - input image reference
\param w       - mew image width
\param h       - mew image height

Note: this functions does not implement resample from one image size to another! 
It just discard old data. Use "Resample" filter instead if you need resampling.
    
*/
void hrFBIResize(HRFBIRef a_image, int w, int h);

/**
\brief Copy frame buffer image data to FBI object from render internal storage.

\param name      - predefined name of internal frame buffer. Currently they can be only ["color" | "gbuffer1" | "gbuffer2"]
\param a_render  - render object reference
\param a_outData - output frame buffer image (FBI)

*/
void hrRenderCopyFrameBufferToFBI(HRRenderRef a_render, const wchar_t* name, HRFBIRef a_outData);

/**
\brief Return reference to some internal render frame buffer image

\param a_filterName - predefined filter name
\param a_parameters - xml node with parameters
\param a_rendRef    - render reference (may be "HRRenderRef()" in most cases) if it is needed for filter

\param a_argName1   - predefined argument name (eqch filter have it's own predefined argument names)
\param a_arg1       - input or output frame buffer image

*/
void hrFilterApply(const wchar_t* a_filterName, pugi::xml_node a_parameters, HRRenderRef a_rendRef = HRRenderRef(),
                   const wchar_t* a_argName1 = L"", HRFBIRef a_arg1 = HRFBIRef(),
                   const wchar_t* a_argName2 = L"", HRFBIRef a_arg2 = HRFBIRef(),
                   const wchar_t* a_argName3 = L"", HRFBIRef a_arg3 = HRFBIRef(),
                   const wchar_t* a_argName4 = L"", HRFBIRef a_arg4 = HRFBIRef(),
                   const wchar_t* a_argName5 = L"", HRFBIRef a_arg5 = HRFBIRef(),
                   const wchar_t* a_argName6 = L"", HRFBIRef a_arg6 = HRFBIRef(),
                   const wchar_t* a_argName7 = L"", HRFBIRef a_arg7 = HRFBIRef(),
                   const wchar_t* a_argName8 = L"", HRFBIRef a_arg8 = HRFBIRef());
