#include "RenderDriverDXRExperimental.h"
#include "LiteMath.h"
using namespace HydraLiteMath;

#include <iostream>

extern HWND mainWindowHWND;

///////////////////////////////////////////////////////////////////////////////////////////////////
//                            Tutorial stuff
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <sstream>

static dxc::DxcDllSupport gDxcDllHelper;
MAKE_SMART_COM_PTR(IDxcCompiler);
MAKE_SMART_COM_PTR(IDxcLibrary);
MAKE_SMART_COM_PTR(IDxcBlobEncoding);
MAKE_SMART_COM_PTR(IDxcOperationResult);


//////////////////////////////////////////////////////////////////////////
// Tutorial 02 code
//////////////////////////////////////////////////////////////////////////

IDXGISwapChain3Ptr createDxgiSwapChain(IDXGIFactory4Ptr pFactory, HWND hwnd, uint32_t width, uint32_t height, DXGI_FORMAT format, ID3D12CommandQueuePtr pCommandQueue)
{
  DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
  swapChainDesc.BufferCount = kDefaultSwapChainBuffers;
  swapChainDesc.Width = width;
  swapChainDesc.Height = height;
  swapChainDesc.Format = format;
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swapChainDesc.SampleDesc.Count = 1;

  // CreateSwapChainForHwnd() doesn't accept IDXGISwapChain3 (Why MS? Why?)
  MAKE_SMART_COM_PTR(IDXGISwapChain1);
  IDXGISwapChain1Ptr pSwapChain;

  HRESULT hr = pFactory->CreateSwapChainForHwnd(pCommandQueue, hwnd, &swapChainDesc, nullptr, nullptr, &pSwapChain);
  if (FAILED(hr))
  {
    d3dTraceHR("Failed to create the swap-chain", hr);
    return false;
  }

  IDXGISwapChain3Ptr pSwapChain3;
  d3d_call(pSwapChain->QueryInterface(IID_PPV_ARGS(&pSwapChain3)));
  return pSwapChain3;
}

ID3D12Device5Ptr createDevice(IDXGIFactory4Ptr pDxgiFactory)
{
  // Find the HW adapter
  IDXGIAdapter1Ptr pAdapter;

  for (uint32_t i = 0; DXGI_ERROR_NOT_FOUND != pDxgiFactory->EnumAdapters1(i, &pAdapter); i++)
  {
    DXGI_ADAPTER_DESC1 desc;
    pAdapter->GetDesc1(&desc);

    // Skip SW adapters
    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
#ifdef _DEBUG
    ID3D12DebugPtr pDx12Debug;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDx12Debug))))
    {
      pDx12Debug->EnableDebugLayer();
    }
#endif
    // Create the device
    ID3D12Device5Ptr pDevice;
    d3d_call(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&pDevice)));

    D3D12_FEATURE_DATA_D3D12_OPTIONS5 features5;
    HRESULT hr = pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &features5, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS5));
    if (FAILED(hr) || features5.RaytracingTier == D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
    {
      msgBox("Raytracing is not supported on this device. Make sure your GPU supports DXR (such as Nvidia's Volta or Turing RTX) and you're on the latest drivers. The DXR fallback layer is not supported.");
      exit(1);
    }
    return pDevice;
  }
  return nullptr;
}

ID3D12CommandQueuePtr createCommandQueue(ID3D12Device5Ptr pDevice)
{
  ID3D12CommandQueuePtr pQueue;
  D3D12_COMMAND_QUEUE_DESC cqDesc = {};
  cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  d3d_call(pDevice->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&pQueue)));
  return pQueue;
}

ID3D12DescriptorHeapPtr createDescriptorHeap(ID3D12Device5Ptr pDevice, uint32_t count, D3D12_DESCRIPTOR_HEAP_TYPE type, bool shaderVisible)
{
  D3D12_DESCRIPTOR_HEAP_DESC desc = {};
  desc.NumDescriptors = count;
  desc.Type = type;
  desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

  ID3D12DescriptorHeapPtr pHeap;
  d3d_call(pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&pHeap)));
  return pHeap;
}

D3D12_CPU_DESCRIPTOR_HANDLE createRTV(ID3D12Device5Ptr pDevice, ID3D12ResourcePtr pResource, ID3D12DescriptorHeapPtr pHeap, uint32_t& usedHeapEntries, DXGI_FORMAT format)
{
  D3D12_RENDER_TARGET_VIEW_DESC desc = {};
  desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
  desc.Format = format;
  desc.Texture2D.MipSlice = 0;
  D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = pHeap->GetCPUDescriptorHandleForHeapStart();
  rtvHandle.ptr += usedHeapEntries * pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  usedHeapEntries++;
  pDevice->CreateRenderTargetView(pResource, &desc, rtvHandle);
  return rtvHandle;
}

void resourceBarrier(ID3D12GraphicsCommandList4Ptr pCmdList, ID3D12ResourcePtr pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
{
  D3D12_RESOURCE_BARRIER barrier = {};
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Transition.pResource = pResource;
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  barrier.Transition.StateBefore = stateBefore;
  barrier.Transition.StateAfter = stateAfter;
  pCmdList->ResourceBarrier(1, &barrier);
}

uint64_t submitCommandList(ID3D12GraphicsCommandList4Ptr pCmdList, ID3D12CommandQueuePtr pCmdQueue, ID3D12FencePtr pFence, uint64_t fenceValue)
{
  pCmdList->Close();
  ID3D12CommandList* pGraphicsList = pCmdList.GetInterfacePtr();
  pCmdQueue->ExecuteCommandLists(1, &pGraphicsList);
  fenceValue++;
  pCmdQueue->Signal(pFence, fenceValue);
  return fenceValue;
}

void RD_DXR_Experimental::initDXR(HWND winHandle, uint32_t winWidth, uint32_t winHeight)
{
  mHwnd = winHandle;
  mSwapChainSize = glm::uvec2(winWidth, winHeight);

  // Initialize the debug layer for debug builds
#ifdef _DEBUG
  ID3D12DebugPtr pDebug;
  if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebug))))
  {
    pDebug->EnableDebugLayer();
  }
#endif
  // Create the DXGI factory
  IDXGIFactory4Ptr pDxgiFactory;
  d3d_call(CreateDXGIFactory1(IID_PPV_ARGS(&pDxgiFactory)));
  mpDevice = createDevice(pDxgiFactory);
  mpCmdQueue = createCommandQueue(mpDevice);
  mpSwapChain = createDxgiSwapChain(pDxgiFactory, mHwnd, winWidth, winHeight, DXGI_FORMAT_R8G8B8A8_UNORM, mpCmdQueue);

  // Create a RTV descriptor heap
  mRtvHeap.pHeap = createDescriptorHeap(mpDevice, kRtvHeapSize, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false);

  // Create the per-frame objects
  for (uint32_t i = 0; i < arraysize(mFrameObjects); i++)
  {
    d3d_call(mpDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mFrameObjects[i].pCmdAllocator)));
    d3d_call(mpSwapChain->GetBuffer(i, IID_PPV_ARGS(&mFrameObjects[i].pSwapChainBuffer)));
    mFrameObjects[i].rtvHandle = createRTV(mpDevice, mFrameObjects[i].pSwapChainBuffer, mRtvHeap.pHeap, mRtvHeap.usedEntries, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
  }

  // Create the command-list
  d3d_call(mpDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mFrameObjects[0].pCmdAllocator, nullptr, IID_PPV_ARGS(&mpCmdList)));

  // Create a fence and the event
  d3d_call(mpDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mpFence)));
  mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

uint32_t RD_DXR_Experimental::beginFrame()
{
  // Bind the descriptor heaps
  ID3D12DescriptorHeap* heaps[] = { mpSrvUavHeap };
  mpCmdList->SetDescriptorHeaps(arraysize(heaps), heaps);
  return mpSwapChain->GetCurrentBackBufferIndex();
}

void RD_DXR_Experimental::endFrame(uint32_t rtvIndex)
{
  resourceBarrier(mpCmdList, mFrameObjects[rtvIndex].pSwapChainBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);
  mFenceValue = submitCommandList(mpCmdList, mpCmdQueue, mpFence, mFenceValue);
  mpSwapChain->Present(0, 0);

  // Prepare the command list for the next frame
  uint32_t bufferIndex = mpSwapChain->GetCurrentBackBufferIndex();

  // Make sure we have the new back-buffer is ready
  if (mFenceValue > kDefaultSwapChainBuffers)
  {
    mpFence->SetEventOnCompletion(mFenceValue - kDefaultSwapChainBuffers + 1, mFenceEvent);
    WaitForSingleObject(mFenceEvent, INFINITE);
  }

  mFrameObjects[bufferIndex].pCmdAllocator->Reset();
  mpCmdList->Reset(mFrameObjects[bufferIndex].pCmdAllocator, nullptr);
}

//////////////////////////////////////////////////////////////////////////
// Tutorial 03 code
//////////////////////////////////////////////////////////////////////////
static const D3D12_HEAP_PROPERTIES kUploadHeapProps =
{
    D3D12_HEAP_TYPE_UPLOAD,
    D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
    D3D12_MEMORY_POOL_UNKNOWN,
    0,
    0,
};

static const D3D12_HEAP_PROPERTIES kDefaultHeapProps =
{
    D3D12_HEAP_TYPE_DEFAULT,
    D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
    D3D12_MEMORY_POOL_UNKNOWN,
    0,
    0
};

ID3D12ResourcePtr createBuffer(ID3D12Device5Ptr pDevice, uint64_t size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState, const D3D12_HEAP_PROPERTIES& heapProps)
{
  D3D12_RESOURCE_DESC bufDesc = {};
  bufDesc.Alignment = 0;
  bufDesc.DepthOrArraySize = 1;
  bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  bufDesc.Flags = flags;
  bufDesc.Format = DXGI_FORMAT_UNKNOWN;
  bufDesc.Height = 1;
  bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  bufDesc.MipLevels = 1;
  bufDesc.SampleDesc.Count = 1;
  bufDesc.SampleDesc.Quality = 0;
  bufDesc.Width = size;

  ID3D12ResourcePtr pBuffer;
  d3d_call(pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufDesc, initState, nullptr, IID_PPV_ARGS(&pBuffer)));
  return pBuffer;
}

ID3D12ResourcePtr createTriangleVB(ID3D12Device5Ptr pDevice, const std::vector<glm::vec3> vertices)
{
  // For simplicity, we create the vertex buffer on the upload heap, but that's not required
  ID3D12ResourcePtr pBuffer = createBuffer(pDevice, vertices.size() * sizeof(glm::vec3), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, kUploadHeapProps);
  uint8_t* pData;
  pBuffer->Map(0, nullptr, (void**)&pData);
  memcpy(pData, vertices.data(), vertices.size() * sizeof(glm::vec3));
  pBuffer->Unmap(0, nullptr);
  return pBuffer;
}

struct AccelerationStructureBuffers
{
  ID3D12ResourcePtr pScratch;
  ID3D12ResourcePtr pResult;
  ID3D12ResourcePtr pInstanceDesc;    // Used only for top-level AS
};

AccelerationStructureBuffers createBottomLevelAS(ID3D12Device5Ptr pDevice, ID3D12GraphicsCommandList4Ptr pCmdList, std::vector<std::pair<ID3D12ResourcePtr, size_t>> meshes)
{
  vector<D3D12_RAYTRACING_GEOMETRY_DESC> geomDescs;
  D3D12_RAYTRACING_GEOMETRY_DESC geomDesc = {};
  for (auto mesh : meshes) {
    geomDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    geomDesc.Triangles.VertexBuffer.StartAddress = mesh.first->GetGPUVirtualAddress();
    geomDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(glm::vec3);
    geomDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
    geomDesc.Triangles.VertexCount = mesh.second;
    geomDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
    geomDescs.push_back(geomDesc);
  }

  // Get the size requirements for the scratch and AS buffers
  D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
  inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
  inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
  inputs.NumDescs = geomDescs.size();
  inputs.pGeometryDescs = geomDescs.data();
  inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;

  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};
  pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

  // Create the buffers. They need to support UAV, and since we are going to immediately use them, we create them with an unordered-access state
  AccelerationStructureBuffers buffers;
  buffers.pScratch = createBuffer(pDevice, info.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, kDefaultHeapProps);
  buffers.pResult = createBuffer(pDevice, info.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, kDefaultHeapProps);

  // Create the bottom-level AS
  D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
  asDesc.Inputs = inputs;
  asDesc.DestAccelerationStructureData = buffers.pResult->GetGPUVirtualAddress();
  asDesc.ScratchAccelerationStructureData = buffers.pScratch->GetGPUVirtualAddress();

  pCmdList->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

  // We need to insert a UAV barrier before using the acceleration structures in a raytracing operation
  D3D12_RESOURCE_BARRIER uavBarrier = {};
  uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
  uavBarrier.UAV.pResource = buffers.pResult;
  pCmdList->ResourceBarrier(1, &uavBarrier);

  return buffers;
}

AccelerationStructureBuffers createTopLevelAS(ID3D12Device5Ptr pDevice, ID3D12GraphicsCommandList4Ptr pCmdList, ID3D12ResourcePtr pBottomLevelAS, uint64_t& tlasSize)
{
  // First, get the size of the TLAS buffers and create them
  D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
  inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
  inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
  inputs.NumDescs = 2;
  inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
  pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

  // Create the buffers
  AccelerationStructureBuffers buffers;
  buffers.pScratch = createBuffer(pDevice, info.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, kDefaultHeapProps);
  buffers.pResult = createBuffer(pDevice, info.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, kDefaultHeapProps);
  tlasSize = info.ResultDataMaxSizeInBytes;

  // The instance desc should be inside a buffer, create and map the buffer
  buffers.pInstanceDesc = createBuffer(pDevice, sizeof(D3D12_RAYTRACING_INSTANCE_DESC), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, kUploadHeapProps);
  D3D12_RAYTRACING_INSTANCE_DESC* pInstanceDesc;
  buffers.pInstanceDesc->Map(0, nullptr, (void**)&pInstanceDesc);

  // Initialize the instance desc. We only have a single instance
  pInstanceDesc->InstanceID = 0;                            // This value will be exposed to the shader via InstanceID()
  pInstanceDesc->InstanceContributionToHitGroupIndex = 0;   // This is the offset inside the shader-table. We only have a single geometry, so the offset 0
  pInstanceDesc->Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
  glm::mat4 m; // Identity matrix
  memcpy(pInstanceDesc->Transform, &m, sizeof(pInstanceDesc->Transform));
  pInstanceDesc->AccelerationStructure = pBottomLevelAS->GetGPUVirtualAddress();
  pInstanceDesc->InstanceMask = 0xFF;

  // Unmap
  buffers.pInstanceDesc->Unmap(0, nullptr);

  // Create the TLAS
  D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
  asDesc.Inputs = inputs;
  asDesc.Inputs.InstanceDescs = buffers.pInstanceDesc->GetGPUVirtualAddress();
  asDesc.DestAccelerationStructureData = buffers.pResult->GetGPUVirtualAddress();
  asDesc.ScratchAccelerationStructureData = buffers.pScratch->GetGPUVirtualAddress();

  pCmdList->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

  // We need to insert a UAV barrier before using the acceleration structures in a raytracing operation
  D3D12_RESOURCE_BARRIER uavBarrier = {};
  uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
  uavBarrier.UAV.pResource = buffers.pResult;
  pCmdList->ResourceBarrier(1, &uavBarrier);

  return buffers;
}

void RD_DXR_Experimental::createAccelerationStructures(vector<vector<glm::vec3>> meshes)
{
  std::vector<std::pair<ID3D12ResourcePtr, size_t>>  args;

  for (auto m : meshes) {
    ID3D12ResourcePtr mpVertexBuffer = createTriangleVB(mpDevice, m);
    args.push_back(make_pair(mpVertexBuffer, m.size()));
  }

  AccelerationStructureBuffers bottomLevelBuffers = createBottomLevelAS(mpDevice, mpCmdList, args);
  AccelerationStructureBuffers topLevelBuffers = createTopLevelAS(mpDevice, mpCmdList, bottomLevelBuffers.pResult, mTlasSize);

  // The tutorial doesn't have any resource lifetime management, so we flush and sync here. This is not required by the DXR spec - you can submit the list whenever you like as long as you take care of the resources lifetime.
  mFenceValue = submitCommandList(mpCmdList, mpCmdQueue, mpFence, mFenceValue);
  mpFence->SetEventOnCompletion(mFenceValue, mFenceEvent);
  WaitForSingleObject(mFenceEvent, INFINITE);
  uint32_t bufferIndex = mpSwapChain->GetCurrentBackBufferIndex();
  mpCmdList->Reset(mFrameObjects[0].pCmdAllocator, nullptr);

  // Store the AS buffers. The rest of the buffers will be released once we exit the function
  mpTopLevelAS = topLevelBuffers.pResult;
  mpBottomLevelAS = bottomLevelBuffers.pResult;
}

//////////////////////////////////////////////////////////////////////////
// Tutorial 04 code
//////////////////////////////////////////////////////////////////////////
ID3DBlobPtr compileLibrary(const WCHAR* filename, const WCHAR* targetString)
{
  // Initialize the helper
  d3d_call(gDxcDllHelper.Initialize());
  IDxcCompilerPtr pCompiler;
  IDxcLibraryPtr pLibrary;
  d3d_call(gDxcDllHelper.CreateInstance(CLSID_DxcCompiler, &pCompiler));
  d3d_call(gDxcDllHelper.CreateInstance(CLSID_DxcLibrary, &pLibrary));

  // Open and read the file
  std::ifstream shaderFile(filename);
  if (shaderFile.good() == false)
  {
    msgBox("Can't open file " + wstring_2_string(std::wstring(filename)));
    return nullptr;
  }
  std::stringstream strStream;
  strStream << shaderFile.rdbuf();
  std::string shader = strStream.str();

  // Create blob from the string
  IDxcBlobEncodingPtr pTextBlob;
  d3d_call(pLibrary->CreateBlobWithEncodingFromPinned((LPBYTE)shader.c_str(), (uint32_t)shader.size(), 0, &pTextBlob));

  // Compile
  IDxcOperationResultPtr pResult;
  d3d_call(pCompiler->Compile(pTextBlob, filename, L"", targetString, nullptr, 0, nullptr, 0, nullptr, &pResult));

  // Verify the result
  HRESULT resultCode;
  d3d_call(pResult->GetStatus(&resultCode));
  if (FAILED(resultCode))
  {
    IDxcBlobEncodingPtr pError;
    d3d_call(pResult->GetErrorBuffer(&pError));
    std::string log = convertBlobToString(pError.GetInterfacePtr());
    msgBox("Compiler error:\n" + log);
    return nullptr;
  }

  MAKE_SMART_COM_PTR(IDxcBlob);
  IDxcBlobPtr pBlob;
  d3d_call(pResult->GetResult(&pBlob));
  return pBlob;
}

ID3D12RootSignaturePtr createRootSignature(ID3D12Device5Ptr pDevice, const D3D12_ROOT_SIGNATURE_DESC& desc)
{
  ID3DBlobPtr pSigBlob;
  ID3DBlobPtr pErrorBlob;
  HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &pSigBlob, &pErrorBlob);
  if (FAILED(hr))
  {
    std::string msg = convertBlobToString(pErrorBlob.GetInterfacePtr());
    msgBox(msg);
    return nullptr;
  }
  ID3D12RootSignaturePtr pRootSig;
  d3d_call(pDevice->CreateRootSignature(0, pSigBlob->GetBufferPointer(), pSigBlob->GetBufferSize(), IID_PPV_ARGS(&pRootSig)));
  return pRootSig;
}

struct RootSignatureDesc
{
  D3D12_ROOT_SIGNATURE_DESC desc = {};
  std::vector<D3D12_DESCRIPTOR_RANGE> range;
  std::vector<D3D12_ROOT_PARAMETER> rootParams;
};

RootSignatureDesc createRayGenRootDesc()
{
  // Create the root-signature
  RootSignatureDesc desc;
  desc.range.resize(2);
  // gOutput
  desc.range[0].BaseShaderRegister = 0;
  desc.range[0].NumDescriptors = 1;
  desc.range[0].RegisterSpace = 0;
  desc.range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
  desc.range[0].OffsetInDescriptorsFromTableStart = 0;

  // gRtScene
  desc.range[1].BaseShaderRegister = 0;
  desc.range[1].NumDescriptors = 1;
  desc.range[1].RegisterSpace = 0;
  desc.range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
  desc.range[1].OffsetInDescriptorsFromTableStart = 1;

  desc.rootParams.resize(1);
  desc.rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
  desc.rootParams[0].DescriptorTable.NumDescriptorRanges = 2;
  desc.rootParams[0].DescriptorTable.pDescriptorRanges = desc.range.data();

  // Create the desc
  desc.desc.NumParameters = 1;
  desc.desc.pParameters = desc.rootParams.data();
  desc.desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;

  return desc;
}

struct DxilLibrary
{
  DxilLibrary(ID3DBlobPtr pBlob, const WCHAR* entryPoint[], uint32_t entryPointCount) : pShaderBlob(pBlob)
  {
    stateSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    stateSubobject.pDesc = &dxilLibDesc;

    dxilLibDesc = {};
    exportDesc.resize(entryPointCount);
    exportName.resize(entryPointCount);
    if (pBlob)
    {
      dxilLibDesc.DXILLibrary.pShaderBytecode = pBlob->GetBufferPointer();
      dxilLibDesc.DXILLibrary.BytecodeLength = pBlob->GetBufferSize();
      dxilLibDesc.NumExports = entryPointCount;
      dxilLibDesc.pExports = exportDesc.data();

      for (uint32_t i = 0; i < entryPointCount; i++)
      {
        exportName[i] = entryPoint[i];
        exportDesc[i].Name = exportName[i].c_str();
        exportDesc[i].Flags = D3D12_EXPORT_FLAG_NONE;
        exportDesc[i].ExportToRename = nullptr;
      }
    }
  };

  DxilLibrary() : DxilLibrary(nullptr, nullptr, 0) {}

  D3D12_DXIL_LIBRARY_DESC dxilLibDesc = {};
  D3D12_STATE_SUBOBJECT stateSubobject{};
  ID3DBlobPtr pShaderBlob;
  std::vector<D3D12_EXPORT_DESC> exportDesc;
  std::vector<std::wstring> exportName;
};

static const WCHAR* kRayGenShader = L"rayGen";
static const WCHAR* kMissShader = L"miss";
static const WCHAR* kClosestHitShader = L"chs";
static const WCHAR* kHitGroup = L"HitGroup";

DxilLibrary createDxilLibrary()
{
  // Compile the shader
  ID3DBlobPtr pDxilLib = compileLibrary(L"shaders/shader.hlsl", L"lib_6_3");
  const WCHAR* entryPoints[] = { kRayGenShader, kMissShader, kClosestHitShader };
  return DxilLibrary(pDxilLib, entryPoints, arraysize(entryPoints));
}

struct HitProgram
{
  HitProgram(LPCWSTR ahsExport, LPCWSTR chsExport, const std::wstring& name) : exportName(name)
  {
    desc = {};
    desc.AnyHitShaderImport = ahsExport;
    desc.ClosestHitShaderImport = chsExport;
    desc.HitGroupExport = exportName.c_str();

    subObject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
    subObject.pDesc = &desc;
  }

  std::wstring exportName;
  D3D12_HIT_GROUP_DESC desc;
  D3D12_STATE_SUBOBJECT subObject;
};

struct ExportAssociation
{
  ExportAssociation(const WCHAR* exportNames[], uint32_t exportCount, const D3D12_STATE_SUBOBJECT* pSubobjectToAssociate)
  {
    association.NumExports = exportCount;
    association.pExports = exportNames;
    association.pSubobjectToAssociate = pSubobjectToAssociate;

    subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
    subobject.pDesc = &association;
  }

  D3D12_STATE_SUBOBJECT subobject = {};
  D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION association = {};
};

struct LocalRootSignature
{
  LocalRootSignature(ID3D12Device5Ptr pDevice, const D3D12_ROOT_SIGNATURE_DESC& desc)
  {
    pRootSig = createRootSignature(pDevice, desc);
    pInterface = pRootSig.GetInterfacePtr();
    subobject.pDesc = &pInterface;
    subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
  }
  ID3D12RootSignaturePtr pRootSig;
  ID3D12RootSignature* pInterface = nullptr;
  D3D12_STATE_SUBOBJECT subobject = {};
};

struct GlobalRootSignature
{
  GlobalRootSignature(ID3D12Device5Ptr pDevice, const D3D12_ROOT_SIGNATURE_DESC& desc)
  {
    pRootSig = createRootSignature(pDevice, desc);
    pInterface = pRootSig.GetInterfacePtr();
    subobject.pDesc = &pInterface;
    subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
  }
  ID3D12RootSignaturePtr pRootSig;
  ID3D12RootSignature* pInterface = nullptr;
  D3D12_STATE_SUBOBJECT subobject = {};
};

struct ShaderConfig
{
  ShaderConfig(uint32_t maxAttributeSizeInBytes, uint32_t maxPayloadSizeInBytes)
  {
    shaderConfig.MaxAttributeSizeInBytes = maxAttributeSizeInBytes;
    shaderConfig.MaxPayloadSizeInBytes = maxPayloadSizeInBytes;

    subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
    subobject.pDesc = &shaderConfig;
  }

  D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
  D3D12_STATE_SUBOBJECT subobject = {};
};

struct PipelineConfig
{
  PipelineConfig(uint32_t maxTraceRecursionDepth)
  {
    config.MaxTraceRecursionDepth = maxTraceRecursionDepth;

    subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
    subobject.pDesc = &config;
  }

  D3D12_RAYTRACING_PIPELINE_CONFIG config = {};
  D3D12_STATE_SUBOBJECT subobject = {};
};

void RD_DXR_Experimental::createRtPipelineState()
{
  // Need 10 subobjects:
  //  1 for the DXIL library
  //  1 for hit-group
  //  2 for RayGen root-signature (root-signature and the subobject association)
  //  2 for the root-signature shared between miss and hit shaders (signature and association)
  //  2 for shader config (shared between all programs. 1 for the config, 1 for association)
  //  1 for pipeline config
  //  1 for the global root signature
  std::array<D3D12_STATE_SUBOBJECT, 10> subobjects;
  uint32_t index = 0;

  // Create the DXIL library
  DxilLibrary dxilLib = createDxilLibrary();
  subobjects[index++] = dxilLib.stateSubobject; // 0 Library

  HitProgram hitProgram(nullptr, kClosestHitShader, kHitGroup);
  subobjects[index++] = hitProgram.subObject; // 1 Hit Group

  // Create the ray-gen root-signature and association
  LocalRootSignature rgsRootSignature(mpDevice, createRayGenRootDesc().desc);
  subobjects[index] = rgsRootSignature.subobject; // 2 RayGen Root Sig

  uint32_t rgsRootIndex = index++; // 2
  ExportAssociation rgsRootAssociation(&kRayGenShader, 1, &(subobjects[rgsRootIndex]));
  subobjects[index++] = rgsRootAssociation.subobject; // 3 Associate Root Sig to RGS

  // Create the miss- and hit-programs root-signature and association
  D3D12_ROOT_SIGNATURE_DESC emptyDesc = {};
  emptyDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
  LocalRootSignature hitMissRootSignature(mpDevice, emptyDesc);
  subobjects[index] = hitMissRootSignature.subobject; // 4 Root Sig to be shared between Miss and CHS

  uint32_t hitMissRootIndex = index++; // 4
  const WCHAR* missHitExportName[] = { kMissShader, kClosestHitShader };
  ExportAssociation missHitRootAssociation(missHitExportName, arraysize(missHitExportName), &(subobjects[hitMissRootIndex]));
  subobjects[index++] = missHitRootAssociation.subobject; // 5 Associate Root Sig to Miss and CHS

  // Bind the payload size to the programs
  ShaderConfig shaderConfig(sizeof(float) * 2, sizeof(float) * 3);
  subobjects[index] = shaderConfig.subobject; // 6 Shader Config

  uint32_t shaderConfigIndex = index++; // 6
  const WCHAR* shaderExports[] = { kMissShader, kClosestHitShader, kRayGenShader };
  ExportAssociation configAssociation(shaderExports, arraysize(shaderExports), &(subobjects[shaderConfigIndex]));
  subobjects[index++] = configAssociation.subobject; // 7 Associate Shader Config to Miss, CHS, RGS

  // Create the pipeline config
  PipelineConfig config(1);
  subobjects[index++] = config.subobject; // 8

  // Create the global root signature and store the empty signature
  GlobalRootSignature root(mpDevice, {});
  mpEmptyRootSig = root.pRootSig;
  subobjects[index++] = root.subobject; // 9

  // Create the state
  D3D12_STATE_OBJECT_DESC desc;
  desc.NumSubobjects = index; // 10
  desc.pSubobjects = subobjects.data();
  desc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;

  d3d_call(mpDevice->CreateStateObject(&desc, IID_PPV_ARGS(&mpPipelineState)));
}

//////////////////////////////////////////////////////////////////////////
// Tutorial 05
//////////////////////////////////////////////////////////////////////////
void RD_DXR_Experimental::createShaderTable()
{
  /** The shader-table layout is as follows:
      Entry 0 - Ray-gen program
      Entry 1 - Miss program
      Entry 2 - Hit program
      All entries in the shader-table must have the same size, so we will choose it base on the largest required entry.
      The ray-gen program requires the largest entry - sizeof(program identifier) + 8 bytes for a descriptor-table.
      The entry size must be aligned up to D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT
  */

  // Calculate the size and create the buffer
  mShaderTableEntrySize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
  mShaderTableEntrySize += 8; // The ray-gen's descriptor table
  mShaderTableEntrySize = align_to(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT, mShaderTableEntrySize);
  uint32_t shaderTableSize = mShaderTableEntrySize * 3;

  // For simplicity, we create the shader-table on the upload heap. You can also create it on the default heap
  mpShaderTable = createBuffer(mpDevice, shaderTableSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, kUploadHeapProps);

  // Map the buffer
  uint8_t* pData;
  d3d_call(mpShaderTable->Map(0, nullptr, (void**)&pData));

  MAKE_SMART_COM_PTR(ID3D12StateObjectProperties);
  ID3D12StateObjectPropertiesPtr pRtsoProps;
  mpPipelineState->QueryInterface(IID_PPV_ARGS(&pRtsoProps));

  // Entry 0 - ray-gen program ID and descriptor data
  memcpy(pData, pRtsoProps->GetShaderIdentifier(kRayGenShader), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
  uint64_t heapStart = mpSrvUavHeap->GetGPUDescriptorHandleForHeapStart().ptr;
  *(uint64_t*)(pData + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES) = heapStart;

  // Entry 1 - miss program
  memcpy(pData + mShaderTableEntrySize, pRtsoProps->GetShaderIdentifier(kMissShader), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);

  // Entry 2 - hit program
  uint8_t* pHitEntry = pData + mShaderTableEntrySize * 2; // +2 skips the ray-gen and miss entries
  memcpy(pHitEntry, pRtsoProps->GetShaderIdentifier(kHitGroup), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);

  // Unmap
  mpShaderTable->Unmap(0, nullptr);
}

//////////////////////////////////////////////////////////////////////////
// Tutorial 06
//////////////////////////////////////////////////////////////////////////
void RD_DXR_Experimental::createShaderResources()
{
  // Create the output resource. The dimensions and format should match the swap-chain
  D3D12_RESOURCE_DESC resDesc = {};
  resDesc.DepthOrArraySize = 1;
  resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // The backbuffer is actually DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, but sRGB formats can't be used with UAVs. We will convert to sRGB ourselves in the shader
  resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
  resDesc.Height = mSwapChainSize.y;
  resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  resDesc.MipLevels = 1;
  resDesc.SampleDesc.Count = 1;
  resDesc.Width = mSwapChainSize.x;
  d3d_call(mpDevice->CreateCommittedResource(&kDefaultHeapProps, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_COPY_SOURCE, nullptr, IID_PPV_ARGS(&mpOutputResource))); // Starting as copy-source to simplify onFrameRender()

  // Create an SRV/UAV descriptor heap. Need 2 entries - 1 SRV for the scene and 1 UAV for the output
  mpSrvUavHeap = createDescriptorHeap(mpDevice, 2, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true);

  // Create the UAV. Based on the root signature we created it should be the first entry
  D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
  uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
  mpDevice->CreateUnorderedAccessView(mpOutputResource, nullptr, &uavDesc, mpSrvUavHeap->GetCPUDescriptorHandleForHeapStart());

  // Create the TLAS SRV right after the UAV. Note that we are using a different SRV desc here
  D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
  srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srvDesc.RaytracingAccelerationStructure.Location = mpTopLevelAS->GetGPUVirtualAddress();
  D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = mpSrvUavHeap->GetCPUDescriptorHandleForHeapStart();
  srvHandle.ptr += mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  mpDevice->CreateShaderResourceView(nullptr, &srvDesc, srvHandle);
}


//////////////////////////////////////////////////////////////////////////
// The render itself
//////////////////////////////////////////////////////////////////////////

RD_DXR_Experimental::RD_DXR_Experimental() {

}

void RD_DXR_Experimental::ClearAll()
{
  printf("ClearAll\n\n");

  // Wait for the command queue to finish execution
  mFenceValue++;
  mpCmdQueue->Signal(mpFence, mFenceValue);
  mpFence->SetEventOnCompletion(mFenceValue, mFenceEvent);
  WaitForSingleObject(mFenceEvent, INFINITE);
}

HRDriverAllocInfo RD_DXR_Experimental::AllocAll(HRDriverAllocInfo a_info)
{
  printf("AllocAll\n\n");
  RECT r;
  GetClientRect(mainWindowHWND, &r);
  int g_width = r.right - r.left;
  int g_height = r.bottom - r.top;

  initDXR(mainWindowHWND, g_width, g_height);        // Tutorial 02

  std::vector<glm::vec3> vertices =
  {
      glm::vec3(0.5, 0.5,  0) + glm::vec3(0,          1,  0),
      glm::vec3(0.5, 0.5,  0) + glm::vec3(0.866f,  -0.5f, 0),
      glm::vec3(0.5, 0.5,  0) + glm::vec3(-0.866f, -0.5f, 0),
      glm::vec3(0.5, 1.5,  0) + glm::vec3(-0.866f, -0.5f, 0),
  };

  std::vector<glm::vec3> vertices2 =
  {
      glm::vec3(0,          1,  0),
      glm::vec3(0.866f,  -0.5f, 0),
      glm::vec3(-0.866f, -0.5f, 0),
  };

  vector<vector<glm::vec3>> meshes;
  meshes.push_back(vertices);
  meshes.push_back(vertices2);
  createAccelerationStructures(meshes);                 // Tutorial 03
  createRtPipelineState();                        // Tutorial 04
  createShaderResources();                        // Tutorial 06. Need to do this before initializing the shader-table
  createShaderTable();                            // Tutorial 05

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
  printf("UpdateSettings\n");
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
  uint32_t rtvIndex = beginFrame();

  // Let's raytrace
  resourceBarrier(mpCmdList, mpOutputResource, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
  D3D12_DISPATCH_RAYS_DESC raytraceDesc = {};
  raytraceDesc.Width = mSwapChainSize.x;
  raytraceDesc.Height = mSwapChainSize.y;
  raytraceDesc.Depth = 1;

  // RayGen is the first entry in the shader-table
  raytraceDesc.RayGenerationShaderRecord.StartAddress = mpShaderTable->GetGPUVirtualAddress() + 0 * mShaderTableEntrySize;
  raytraceDesc.RayGenerationShaderRecord.SizeInBytes = mShaderTableEntrySize;

  // Miss is the second entry in the shader-table
  size_t missOffset = 1 * mShaderTableEntrySize;
  raytraceDesc.MissShaderTable.StartAddress = mpShaderTable->GetGPUVirtualAddress() + missOffset;
  raytraceDesc.MissShaderTable.StrideInBytes = mShaderTableEntrySize;
  raytraceDesc.MissShaderTable.SizeInBytes = mShaderTableEntrySize;   // Only a s single miss-entry

  // Hit is the third entry in the shader-table
  size_t hitOffset = 2 * mShaderTableEntrySize;
  raytraceDesc.HitGroupTable.StartAddress = mpShaderTable->GetGPUVirtualAddress() + hitOffset;
  raytraceDesc.HitGroupTable.StrideInBytes = mShaderTableEntrySize;
  raytraceDesc.HitGroupTable.SizeInBytes = mShaderTableEntrySize;

  // Bind the empty root signature
  mpCmdList->SetComputeRootSignature(mpEmptyRootSig);

  // Dispatch
  mpCmdList->SetPipelineState1(mpPipelineState.GetInterfacePtr());
  mpCmdList->DispatchRays(&raytraceDesc);

  // Copy the results to the back-buffer
  resourceBarrier(mpCmdList, mpOutputResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
  resourceBarrier(mpCmdList, mFrameObjects[rtvIndex].pSwapChainBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST);
  mpCmdList->CopyResource(mFrameObjects[rtvIndex].pSwapChainBuffer, mpOutputResource);

  endFrame(rtvIndex);

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
