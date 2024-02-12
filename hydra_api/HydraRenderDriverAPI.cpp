#include "HydraRenderDriverAPI.h"
#include "HydraInternal.h"

#if defined(USE_GL) || !defined(HYDRA_API_CMAKE)
#include "RenderDriverOpenGL3_Utility.h"
#endif

std::unordered_map<std::wstring, HRDriverInfo> RenderDriverFactory::create_methods;


void registerBuiltInRenderDrivers()
{
  // Hydra Core 2

  HRDriverInfo hydraConnectionInfo;
  hydraConnectionInfo.supportHDRFrameBuffer              = true;
  hydraConnectionInfo.supportHDRTextures                 = true;
  hydraConnectionInfo.supportMultiMaterialInstance       = true;

  hydraConnectionInfo.supportImageLoadFromInternalFormat = true;
  hydraConnectionInfo.supportMeshLoadFromInternalFormat  = true;

  hydraConnectionInfo.supportImageLoadFromExternalFormat = true;
  hydraConnectionInfo.supportLighting                    = true;
  hydraConnectionInfo.createsLightGeometryItself         = false;
  hydraConnectionInfo.supportGetFrameBufferLine          = true;
  hydraConnectionInfo.supportUtilityPrepass              = true;
  hydraConnectionInfo.supportDisplacement                = true;

  hydraConnectionInfo.memTotal                           = int64_t(8) * int64_t(1024 * 1024 * 1024); // #TODO: wth i have to do with that ???

  hydraConnectionInfo.driverName                         = L"HydraModern";
  hydraConnectionInfo.createFunction                     = CreateHydraConnection_RenderDriver;
  RenderDriverFactory::Register(L"HydraModern", hydraConnectionInfo);

  //*****************************************************************************************************
  // Hydra Core 3

  HRDriverInfo hydraCore3Info;
  hydraCore3Info.supportHDRFrameBuffer              = true;
  hydraCore3Info.supportHDRTextures                 = true;
  hydraCore3Info.supportMultiMaterialInstance       = true;

  hydraCore3Info.supportImageLoadFromInternalFormat = true;
  hydraCore3Info.supportMeshLoadFromInternalFormat  = true;

  hydraCore3Info.supportImageLoadFromExternalFormat = true;
  hydraCore3Info.supportLighting                    = true;
  hydraCore3Info.createsLightGeometryItself         = false;
  hydraCore3Info.supportGetFrameBufferLine          = true;
  hydraCore3Info.supportUtilityPrepass              = true;
  hydraCore3Info.supportDisplacement                = false;

  hydraCore3Info.memTotal                           = int64_t(8) * int64_t(1024 * 1024 * 1024); // #TODO: wth i have to do with that ???

  hydraCore3Info.driverName                         = L"HydraCore3";
  hydraCore3Info.createFunction                     = CreateHydraCore3_RenderDriver;
  RenderDriverFactory::Register(L"HydraCore3", hydraCore3Info);

  //*****************************************************************************************************
  // OpenGL3

#if defined(USE_GL) || !defined(HYDRA_API_CMAKE)
  HRDriverInfo utilityDriverInfo;
  utilityDriverInfo.supportHDRFrameBuffer              = false;
  utilityDriverInfo.supportHDRTextures                 = true;
  utilityDriverInfo.supportMultiMaterialInstance       = false;

  utilityDriverInfo.supportImageLoadFromInternalFormat = false;
  utilityDriverInfo.supportImageLoadFromExternalFormat = false;
  utilityDriverInfo.supportMeshLoadFromInternalFormat  = false;
  utilityDriverInfo.supportLighting                    = false;

  utilityDriverInfo.memTotal                           = int64_t(8) * int64_t(1024 * 1024 * 1024); //TODO: ?

  utilityDriverInfo.driverName                         = L"opengl3Utility";
  utilityDriverInfo.createFunction                     = CreateOpenGL3_Utility_RenderDriver;
  RenderDriverFactory::Register(L"opengl3Utility", utilityDriverInfo);
#endif
  
  //*****************************************************************************************************
  //Debug print

  HRDriverInfo debugPrintDriverInfo;
  debugPrintDriverInfo.createsLightGeometryItself         = false;
  debugPrintDriverInfo.memTotal                           = 8* int64_t(1024*1024*1024);
  debugPrintDriverInfo.supportHDRFrameBuffer              = false;
  debugPrintDriverInfo.supportHDRTextures                 = false;
  debugPrintDriverInfo.supportImageLoadFromExternalFormat = false;
  debugPrintDriverInfo.supportImageLoadFromInternalFormat = false;
  debugPrintDriverInfo.supportLighting                    = false;
  debugPrintDriverInfo.supportMeshLoadFromInternalFormat  = false;
  debugPrintDriverInfo.supportMultiMaterialInstance       = false;

  debugPrintDriverInfo.driverName                         = L"DebugPrint";
  debugPrintDriverInfo.createFunction                     = CreateDebugPrint_RenderDriver;

  RenderDriverFactory::Register(L"DebugPrint", debugPrintDriverInfo);
}
