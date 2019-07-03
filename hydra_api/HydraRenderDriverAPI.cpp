#include "HydraRenderDriverAPI.h"
#include "HydraInternal.h"
#include "RenderDriverOpenGL3_Utility.h"

std::unordered_map<std::wstring, HRDriverInfo> RenderDriverFactory::create_methods;


void registerBuiltInRenderDrivers()
{
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

  hydraConnectionInfo.driverName = L"HydraModern";
  hydraConnectionInfo.createFunction = CreateHydraConnection_RenderDriver;
  RenderDriverFactory::Register(L"HydraModern", hydraConnectionInfo);

  //*****************************************************************************************************

  HRDriverInfo utilityDriverInfo;
  utilityDriverInfo.supportHDRFrameBuffer        = false;
  utilityDriverInfo.supportHDRTextures           = true;
  utilityDriverInfo.supportMultiMaterialInstance = false;

  utilityDriverInfo.supportImageLoadFromInternalFormat = false;
  utilityDriverInfo.supportImageLoadFromExternalFormat = false;
  utilityDriverInfo.supportMeshLoadFromInternalFormat  = false;
  utilityDriverInfo.supportLighting                    = false;

  utilityDriverInfo.memTotal = int64_t(8) * int64_t(1024 * 1024 * 1024); //TODO: ?

  utilityDriverInfo.driverName = L"opengl3Utility";
  utilityDriverInfo.createFunction = CreateOpenGL3_Utility_RenderDriver;
  RenderDriverFactory::Register(L"opengl3Utility", utilityDriverInfo);

  //*****************************************************************************************************

  HRDriverInfo debugPrintDriverInfo;
  debugPrintDriverInfo.createsLightGeometryItself = false;
  debugPrintDriverInfo.memTotal = 8* int64_t(1024*1024*1024);
  debugPrintDriverInfo.supportHDRFrameBuffer = false;
  debugPrintDriverInfo.supportHDRTextures = false;
  debugPrintDriverInfo.supportImageLoadFromExternalFormat = false;
  debugPrintDriverInfo.supportImageLoadFromInternalFormat = false;
  debugPrintDriverInfo.supportLighting = false;
  debugPrintDriverInfo.supportMeshLoadFromInternalFormat = false;
  debugPrintDriverInfo.supportMultiMaterialInstance = false;

  debugPrintDriverInfo.driverName = L"debugPrint";
  debugPrintDriverInfo.createFunction = CreateDebugPrint_RenderDriver;

  RenderDriverFactory::Register(L"debugPrint", debugPrintDriverInfo);
}
