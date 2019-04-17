
#pragma once


#include "../hydra_api/RTX/Framework.h"

#include "HydraRenderDriverAPI.h"
#include "OpenGLCoreProfileUtils.h"

using namespace std;

using namespace HydraLiteMath;
using namespace GL_RENDER_DRIVER_UTILS;

struct RD_DXR_Experimental : public IHRRenderDriver
{
  RD_DXR_Experimental();

  void ClearAll() override;
  HRDriverAllocInfo AllocAll(HRDriverAllocInfo a_info) override;

  bool UpdateImage(int32_t a_texId, int32_t w, int32_t h, int32_t bpp, const void* a_data, pugi::xml_node a_texNode) override;
  bool UpdateMaterial(int32_t a_matId, pugi::xml_node a_materialNode) override;
  bool UpdateLight(int32_t a_lightIdId, pugi::xml_node a_lightNode) override;
  bool UpdateMesh(int32_t a_meshId, pugi::xml_node a_meshNode, const HRMeshDriverInput& a_input, const HRBatchInfo* a_batchList, int32_t a_listSize) override;

  bool UpdateImageFromFile(int32_t a_texId, const wchar_t* a_fileName, pugi::xml_node a_texNode) override { return false; }
  bool UpdateMeshFromFile(int32_t a_meshId, pugi::xml_node a_meshNode, const wchar_t* a_fileName) override { return false; }


  bool UpdateCamera(pugi::xml_node a_camNode) override;
  bool UpdateSettings(pugi::xml_node a_settingsNode) override;

  /////////////////////////////////////////////////////////////////////////////////////////////

  void BeginScene(pugi::xml_node a_sceneNode) override;
  void EndScene() override;
  void InstanceMeshes(int32_t a_mesh_id, const float* a_matrices, int32_t a_instNum, const int* a_lightInstId, const int* a_remapId, const int* a_realInstId) override;
  void InstanceLights(int32_t a_light_id, const float* a_matrix, pugi::xml_node* a_custAttrArray, int32_t a_instNum, int32_t a_lightGroupId) override;

  void Draw() override;

  HRRenderUpdateInfo HaveUpdateNow(int a_maxRaysPerPixel) override;

  void GetFrameBufferHDR(int32_t w, int32_t h, float*   a_out, const wchar_t* a_layerName) override;
  void GetFrameBufferLDR(int32_t w, int32_t h, int32_t* a_out) override;

  void GetGBufferLine(int32_t a_lineNumber, HRGBufferPixel* a_lineData, int32_t a_startX, int32_t a_endX, const std::unordered_set<int32_t>& a_shadowCatchers) override {}

  HRDriverInfo Info() override;
  const HRRenderDeviceInfoListElem* DeviceList() const override { return nullptr; }
  bool EnableDevice(int32_t id, bool a_enable) override { return true; }

protected:

  std::wstring m_libPath;
  std::wstring m_msg;

  GLuint m_whiteTex;

  // camera parameters
  //
  float camPos[3];
  float camLookAt[3];
  float camUp[3];

  float camFov;
  float camNearPlane;
  float camFarPlane;
  int   m_width;
  int   m_height;

  void initDXR(HWND winHandle, uint32_t winWidth, uint32_t winHeight);
  uint32_t beginFrame();
  void endFrame(uint32_t rtvIndex);
  HWND mHwnd = nullptr;
  ID3D12Device5Ptr mpDevice;
  ID3D12CommandQueuePtr mpCmdQueue;
  IDXGISwapChain3Ptr mpSwapChain;
  glm::uvec2 mSwapChainSize;
  ID3D12GraphicsCommandList4Ptr mpCmdList;
  ID3D12FencePtr mpFence;
  HANDLE mFenceEvent;
  uint64_t mFenceValue = 0;

  struct
  {
    ID3D12CommandAllocatorPtr pCmdAllocator;
    ID3D12ResourcePtr pSwapChainBuffer;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
  } mFrameObjects[kDefaultSwapChainBuffers];

  // Heap data
  struct HeapData
  {
    ID3D12DescriptorHeapPtr pHeap;
    uint32_t usedEntries = 0;
  };
  HeapData mRtvHeap;
  static const uint32_t kRtvHeapSize = 3;

  //////////////////////////////////////////////////////////////////////////
  // Tutorial 03, Tutorial 11
  //////////////////////////////////////////////////////////////////////////
  void createAccelerationStructures();
  ID3D12ResourcePtr mpVertexBuffer[2];
  ID3D12ResourcePtr mpTopLevelAS;
  ID3D12ResourcePtr mpBottomLevelAS[2];
  uint64_t mTlasSize = 0;

  //////////////////////////////////////////////////////////////////////////
  // Tutorial 04
  //////////////////////////////////////////////////////////////////////////
  void createRtPipelineState();
  ID3D12StateObjectPtr mpPipelineState;
  ID3D12RootSignaturePtr mpEmptyRootSig;

  //////////////////////////////////////////////////////////////////////////
  // Tutorial 05
  //////////////////////////////////////////////////////////////////////////
  void createShaderTable();
  ID3D12ResourcePtr mpShaderTable;
  uint32_t mShaderTableEntrySize = 0;

  //////////////////////////////////////////////////////////////////////////
  // Tutorial 06
  //////////////////////////////////////////////////////////////////////////
  void createShaderResources();
  ID3D12ResourcePtr mpOutputResource;
  ID3D12DescriptorHeapPtr mpSrvUavHeap;
  static const uint32_t kSrvUavHeapSize = 2;

  //////////////////////////////////////////////////////////////////////////
  // Tutorial 10
  //////////////////////////////////////////////////////////////////////////
  void createConstantBuffers();
  ID3D12ResourcePtr mpConstantBuffer[3];

  //void CreateCameraBuffer();
  //void UpdateCameraBuffer();
  ID3D12ResourcePtr m_cameraBuffer;
  ID3D12DescriptorHeapPtr m_constHeap;
  uint32_t m_cameraBufferSize = 0;

  template <typename T>
  struct MeshAttrib {
    vector<T> data;
  };
  
  vector<MeshAttrib<glm::vec3>> sceneCoord;
};
