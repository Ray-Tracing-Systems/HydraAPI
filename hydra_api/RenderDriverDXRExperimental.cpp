#include "RenderDriverDXRExperimental.h"
#include "LiteMath.h"
using namespace HydraLiteMath;

#include <iostream>

RD_DXR_Experimental::RD_DXR_Experimental() {

}

void RD_DXR_Experimental::ClearAll()
{
  printf("ClearAll\n\n");
}

HRDriverAllocInfo RD_DXR_Experimental::AllocAll(HRDriverAllocInfo a_info)
{
  printf("AllocAll\n\n");

  return a_info;
}

HRDriverInfo RD_DXR_Experimental::Info()
{
  HRDriverInfo info; // very simple render driver implementation, does not support any other/advanced stuff

  info.supportHDRFrameBuffer = false;
  info.supportHDRTextures = false;
  info.supportMultiMaterialInstance = false;

  info.supportImageLoadFromInternalFormat = false;
  info.supportImageLoadFromExternalFormat = false;
  info.supportMeshLoadFromInternalFormat = false;
  info.supportLighting = false;

  info.memTotal = int64_t(8) * int64_t(1024 * 1024 * 1024);

  return info;
}

#pragma warning(disable:4996) // for wcscpy to be ok

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// typedef void (APIENTRYP PFNGLGENERATEMIPMAPPROC)(GLenum target);
// PFNGLGENERATEMIPMAPPROC glad_glGenerateMipmap;
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool RD_DXR_Experimental::UpdateImage(int32_t a_texId, int32_t w, int32_t h, int32_t bpp, const void* a_data, pugi::xml_node a_texNode)
{
  if (a_data == nullptr)
    return false;


  printf("UpdateImage\n");

  return true;
}


bool RD_DXR_Experimental::UpdateMaterial(int32_t a_matId, pugi::xml_node a_materialNode)
{

  printf("UpdateMaterial\n");

  return true;
}

bool RD_DXR_Experimental::UpdateLight(int32_t a_lightIdId, pugi::xml_node a_lightNode)
{
  printf("UpdateLight\n");
  return true;
}


bool RD_DXR_Experimental::UpdateCamera(pugi::xml_node a_camNode)
{
  if (a_camNode == nullptr)
    return true;


  printf("UpdateCamera\n");

  return true;
}

bool RD_DXR_Experimental::UpdateSettings(pugi::xml_node a_settingsNode)
{
  if (a_settingsNode.child(L"width") != nullptr)
    m_width = a_settingsNode.child(L"width").text().as_int();

  if (a_settingsNode.child(L"height") != nullptr)
    m_height = a_settingsNode.child(L"height").text().as_int();

  if (m_width < 0 || m_height < 0)
  {
    if (m_pInfoCallBack != nullptr)
      m_pInfoCallBack(L"bad input resolution", L"RD_DXR_Experimental::UpdateSettings", HR_SEVERITY_ERROR);
    return false;
  }

  return true;
}


bool RD_DXR_Experimental::UpdateMesh(int32_t a_meshId, pugi::xml_node a_meshNode, const HRMeshDriverInput& a_input, const HRBatchInfo* a_batchList, int32_t a_listSize)
{
  printf("UpdateMesh\n");

  if (a_input.triNum == 0) // don't support loading mesh from file 'a_fileName'
  {

    return true;
  }

  return true;
}


void RD_DXR_Experimental::BeginScene(pugi::xml_node a_sceneNode)
{

  printf("BeginScene\n");
}

void RD_DXR_Experimental::EndScene()
{
  printf("EndScene\n");
}

void RD_DXR_Experimental::Draw()
{
  printf("Draw\n");
}


void RD_DXR_Experimental::InstanceMeshes(int32_t a_mesh_id, const float* a_matrices, int32_t a_instNum, const int* a_lightInstId, const int* a_remapId, const int* a_realInstId)
{
  printf("InstanceMeshes\n");
}


void RD_DXR_Experimental::InstanceLights(int32_t a_light_id, const float* a_matrix, pugi::xml_node* a_custAttrArray, int32_t a_instNum, int32_t a_lightGroupId)
{
  printf("InstanceLights\n");
}

HRRenderUpdateInfo RD_DXR_Experimental::HaveUpdateNow(int a_maxRaysPerPixel)
{
  //glFlush();
  HRRenderUpdateInfo res;
  res.finalUpdate = true;
  res.haveUpdateFB = true;
  res.progress = 100.0f;
  return res;
}


void RD_DXR_Experimental::GetFrameBufferHDR(int32_t w, int32_t h, float*   a_out, const wchar_t* a_layerName)
{
  printf("GetFrameBufferHDR\n");
}

void RD_DXR_Experimental::GetFrameBufferLDR(int32_t w, int32_t h, int32_t* a_out)
{
  printf("GetFrameBufferLDR\n");
}


IHRRenderDriver* CreateDXRExperimental_RenderDriver()
{
  return new RD_DXR_Experimental;
}
