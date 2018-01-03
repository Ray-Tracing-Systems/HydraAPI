#pragma once

#include <memory>

#include "HydraInternal.h"
#include "HydraObjectManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct HRSceneDataCommon : public IHRSceneData
{
  HRSceneDataCommon(){}
  virtual ~HRSceneDataCommon(){}
};

struct HRSceneInstCommon : public IHRSceneInst
{
  HRSceneInstCommon(){}
  virtual ~HRSceneInstCommon(){}
};

struct HRRenderCommon : public IHRRender
{
  HRRenderCommon(){}
  virtual ~HRRenderCommon(){}
};


struct HRLight;

struct HRMeshCommon;
struct HRLightContainerGeneral;
struct HRMatCommon;
struct HRCamCommon;

struct HRTextureNodeCommon;

struct HRSceneDataCommon;
struct HRSceneInstCommon;
struct HRRenderCommon;

struct HydraFactoryCommon : public IHydraFactory
{
  HydraFactoryCommon() {}
  virtual ~HydraFactoryCommon() {}

  std::shared_ptr<IHRTextureNode> CreateTexture2DFromFile(HRTextureNode* pSysObj, const std::wstring& a_fileName);
  std::shared_ptr<IHRTextureNode> CreateTexture2DFromMemory(HRTextureNode* pSysObj, int w, int h, int bpp, const void* a_data);
  std::shared_ptr<IHRTextureNode> CreateTextureInfoFromChunkFile(HRTextureNode* pSysObj, const wchar_t* a_chunkFileName);

  std::shared_ptr<IHRMesh>        CreateVSGFFromSimpleInputMesh(HRMesh* pSysObj);
  std::shared_ptr<IHRMesh>        CreateVSGFFromFile(HRMesh* pSysObj, const std::wstring& a_fileName);
};


