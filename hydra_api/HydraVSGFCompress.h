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

HydraGeomData HR_LoadVSGFCompressedData(const wchar_t* a_fileName, std::vector<int>& dataBuffer);

void _hrCompressMesh(const std::wstring& a_inPath, const std::wstring& a_outPath);
void _hrDecompressMesh(const std::wstring& a_path, const std::wstring& a_newPath);

struct HydraHeaderC // this header i used only fpr '.vsgfc', compressed format.
{
  uint64_t compressedSizeInBytes;
  float    boxMin[3];
  float    boxMax[3];
  uint32_t batchListArraySize;
  uint32_t customDataSize;
  uint64_t customDataOffset;
};


#endif //HYDRAAPI_EX_HYDRAVSGFCOMPRESS_H
