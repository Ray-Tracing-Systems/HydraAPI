//
// Created by frol on 05.04.19.
//

#ifndef HYDRAAPI_EX_HYDRAVSGFCOMPRESS_H
#define HYDRAAPI_EX_HYDRAVSGFCOMPRESS_H

#include <vector>
#include <string>
#include <map>
#include <cstring>
#include <cstdint>

#include "HydraVSGFExport.h"

/**
\brief save mesh in '.vsgfc' format and write custom data to the end of it
\param data             -
\param a_outfileName    -
\param a_customData     -
\param a_customDataSize
*/

size_t HR_SaveVSGFCompressed(const HydraGeomData& data, const wchar_t* a_outfileName, const char* a_customData, const int a_customDataSize);
size_t HR_SaveVSGFCompressed(const void* vsgfData, size_t a_vsgfSize, const wchar_t* a_outfileName, const char* a_customData, const int a_dataSize);


void _hrCompressMesh(const std::wstring& a_inPath, const std::wstring& a_outPath);
void _hrDecompressMesh(const std::wstring& a_path, const std::wstring& a_newPath);

#endif //HYDRAAPI_EX_HYDRAVSGFCOMPRESS_H
