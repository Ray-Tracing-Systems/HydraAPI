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
#include "HydraRenderDriverAPI.h"

struct HydraHeaderC // this header is used in '.vsgfc' and '.vsgf2' formats.
{
  uint64_t geometrySizeInBytes;
  float    boxMin[3];
  float    boxMax[3];
  uint32_t batchListArraySize;
  uint32_t customDataSize;
  uint64_t customDataOffset;
};

/**
\brief save mesh in '.vsgfc' format and write custom data to the end of it
\param data             -
\param a_outfileName    -
\param a_customData     -
\param a_customDataSize
*/
size_t HR_SaveVSGFCompressed(const HydraGeomData& data, const wchar_t* a_outfileName, const char* a_customData,
                             const int a_customDataSize, bool a_placeToOrigin = false);
size_t HR_SaveVSGFCompressed(const void* vsgfData, size_t a_vsgfSize, const wchar_t* a_outfileName, const char* a_customData,
                             const int a_dataSize, bool a_placeToOrigin = false);

size_t HR_SaveVSGFUncompressed(const HydraGeomData& data, const wchar_t* a_outfileName, const char* a_customData,
                               const int a_customDataSize, bool a_placeToOrigin);

void HR_LoadVSGFCompressedHeaders(std::ifstream& fin, std::vector<HRBatchInfo>& a_outBatchList,
                                  HydraGeomData::Header& h1, HydraHeaderC& h2);

void HR_LoadVSGF2Headers(std::ifstream& fin, std::vector<HRBatchInfo>& a_outBatchList,
                         HydraGeomData::Header& h1, HydraHeaderC& h2);

HydraGeomData HR_LoadVSGFCompressedData(const wchar_t* a_fileName, std::vector<int>& dataBuffer, std::vector<HRBatchInfo>* pOutbatchList = nullptr);

void _hrCompressMesh(const std::wstring& a_inPath, const std::wstring& a_outPath);
void _hrDecompressMesh(const std::wstring& a_path, const std::wstring& a_newPath);


static inline std::wstring str_tail(const std::wstring& a_str, int a_tailSize)
{
  if(a_tailSize < a_str.size())
    return a_str.substr(a_str.size() - a_tailSize);
  else
    return a_str;
}


#endif //HYDRAAPI_EX_HYDRAVSGFCOMPRESS_H
