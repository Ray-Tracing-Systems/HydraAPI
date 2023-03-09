#include "HydraObjectManager.h"

HRObjectManager g_objManager;


void _Default_InfoCallBack(const wchar_t* message, const wchar_t* callerPlace, HR_SEVERITY_LEVEL a_level)
{
  if (a_level >= HR_SEVERITY_WARNING)
  {
    if (a_level == HR_SEVERITY_WARNING)
      std::wcerr << L"WRN: " << callerPlace << L": " << message; // << std::endl;
    else
      std::wcerr << L"ERR: " << callerPlace << L": " << message; // << std::endl;
  }
}


std::wstring      g_lastErrorCallerPlace = L"";
std::wstring      g_lastError      = L"";
HR_INFO_CALLBACK  g_pInfoCallback  = &_Default_InfoCallBack;


void HrError(std::wstring a_str) 
{ 
  if (g_pInfoCallback != nullptr)
    g_pInfoCallback(a_str.c_str(), g_lastErrorCallerPlace.c_str(), HR_SEVERITY_ERROR);
  
  g_lastError = a_str;
}

void _HrPrint(HR_SEVERITY_LEVEL a_level, const wchar_t* a_str)
{
  if (g_pInfoCallback != nullptr)
    g_pInfoCallback(a_str, g_lastErrorCallerPlace.c_str(), a_level);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


namespace HydraRender
{
  std::unique_ptr<IHRImageTool> CreateImageTool();
};

void HRObjectManager::init(HRInitInfo a_initInfo)
{
  m_lastInitInfo               = a_initInfo;
  m_useLocalPath               = a_initInfo.localDataPath;
  m_copyTexFilesToLocalStorage = a_initInfo.copyTexturesToLocalFolder;
  m_sortTriIndices             = a_initInfo.sortMaterialIndices;
  m_attachMode                 = (a_initInfo.vbSize <= 1024*1024);
  m_computeBBoxes              = a_initInfo.computeMeshBBoxes;
  
  m_pFactory = new HydraFactoryCommon;
  if(SharedVirtualBufferIsEnabled())
    m_pVBSysMutex = hr_create_system_mutex("hydra_virtual_buffer_lock");
  else
    m_pVBSysMutex = nullptr;
  scnData.init(m_attachMode, m_pVBSysMutex, a_initInfo.vbSize);

  m_pImgTool = HydraRender::CreateImageTool();

  m_FreeImageDll.Initialize();
}


void HRObjectManager::destroy()
{
  g_objManager.m_pDriver = nullptr; // delete curr render driver pointer to prevent global reference;
  for (auto& r : renderSettings)
    r.clear();
  renderSettings.clear();

  scnData.clear(); // for all scnData --> .clear()
  scnInst.clear();

	scnData.m_xmlDoc.reset();

	scnData.m_texturesLib  = pugi::xml_node();
	scnData.m_materialsLib = pugi::xml_node();
	scnData.m_lightsLib    = pugi::xml_node();
	scnData.m_cameraLib    = pugi::xml_node();
	scnData.m_geometryLib  = pugi::xml_node();
	scnData.m_settingsNode = pugi::xml_node();
	scnData.m_sceneNode    = pugi::xml_node();

	scnData.m_vbCache.Destroy();

  if(m_pVBSysMutex != nullptr)
    hr_free_system_mutex(m_pVBSysMutex);
  delete m_pFactory; m_pFactory = nullptr;
}

const std::wstring HRObjectManager::GetLoc(pugi::xml_node a_node) const
{
  const std::wstring locAttr = a_node.attribute(L"loc").as_string();
  if (locAttr.empty())
    return locAttr;
  else
    return scnData.m_path + std::wstring(L"/") + locAttr;
}

void HRObjectManager::SetLoc(pugi::xml_node a_node, const std::wstring& a_loc)
{
  if(a_loc == L"unknown")
  {
    a_node.attribute(L"loc") = a_loc.c_str();
    return;
  }

  const std::wstring& libPath = scnData.m_path;
  const size_t charsNum       = libPath.size();
  const std::wstring loc      = a_loc.substr(charsNum+1, a_loc.size());

  if (a_node.attribute(L"loc") != nullptr)
    a_node.attribute(L"loc").set_value(loc.c_str());
  else
    a_node.append_attribute(L"loc").set_value(loc.c_str());
}


HRMesh* HRObjectManager::PtrById(HRMeshRef a_ref)
{
  if (scnData.meshes.empty())
    return nullptr;
  else if (a_ref.id < 0 || a_ref.id >= (int)(scnData.meshes.size()))
  {
    HrError(L"Invalid HRMeshRef, id = ", a_ref.id);
    return nullptr;
  }
  else
    return &scnData.meshes[a_ref.id];
}

HRLight* HRObjectManager::PtrById(HRLightRef a_ref)
{
  if (scnData.lights.empty())
    return nullptr;
  else if (a_ref.id < 0 || a_ref.id >= (int)scnData.lights.size())
  {
    HrError(L"Invalid HRLightRef, id = ", a_ref.id);
    return nullptr;
  }
  else
    return &scnData.lights[a_ref.id];
}

HRMaterial* HRObjectManager::PtrById(HRMaterialRef a_ref)
{
  if (scnData.materials.empty())
    return nullptr;
  else if (a_ref.id < 0 || a_ref.id >= (int)scnData.materials.size())
  {
    HrError(L"Invalid HRMaterialRef, id = ", a_ref.id);
    return nullptr;
  }
  else
    return &scnData.materials[a_ref.id];
}

HRCamera* HRObjectManager::PtrById(HRCameraRef a_ref)
{
  if (scnData.cameras.empty())
    return nullptr;
  else if (a_ref.id < 0 || a_ref.id >= (int)scnData.cameras.size())
  {
    //Error(L"Invalid HRCameraRef, id = ", a_ref.id);
    return nullptr;
  }
  else
    return &scnData.cameras[a_ref.id];
}

HRTextureNode* HRObjectManager::PtrById(HRTextureNodeRef a_ref)
{
  if (scnData.textures.empty())
    return nullptr;
  else if (a_ref.id < 0 || a_ref.id >= (int)scnData.textures.size())
  {
    //Error(L"Invalid HRTextureNodeRef, id = ", a_ref.id);
    return nullptr;
  }
  else
    return &scnData.textures[a_ref.id];
}

HRSceneInst* HRObjectManager::PtrById(HRSceneInstRef a_ref)
{
  if (scnInst.empty())
    return nullptr;
  else if (a_ref.id < 0 || a_ref.id >= (int)scnInst.size())
  {
    //Error(L"Invalid HRSceneInstRef, id = ", a_ref.id);
    return nullptr;
  }
  else
    return &scnInst[a_ref.id];
}

HRRender* HRObjectManager::PtrById(HRRenderRef a_ref)
{
  if (renderSettings.empty())
    return nullptr;
  else if (a_ref.id < 0 || a_ref.id >= (int)renderSettings.size())
  {
    //Error(L"Invalid HRRenderRef, id = ", a_ref.id);
    return nullptr;
  }
  else
    return &renderSettings[a_ref.id];
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void clear_node(pugi::xml_node a_xmlNode)
{
  // clear all attribures
  //
  for (pugi::xml_attribute attr = a_xmlNode.first_attribute(); attr; )
  {
    pugi::xml_attribute next = attr.next_attribute();
    a_xmlNode.remove_attribute(attr);
    attr = next;
  }

  clear_node_childs(a_xmlNode);
}


void clear_node_childs(pugi::xml_node a_xmlNode)
{
  for (pugi::xml_node child = a_xmlNode.first_child(); child;)
  {
    pugi::xml_node next = child.next_sibling();
    child.parent().remove_child(child);
    child = next;
  }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void HRSceneData::init(bool a_attachMode, HRSystemMutex* a_pVBSysMutexLock, size_t a_vbSize)
{
  m_texturesLib  = m_xmlDoc.append_child(L"textures_lib");
  m_materialsLib = m_xmlDoc.append_child(L"materials_lib");
  m_geometryLib  = m_xmlDoc.append_child(L"geometry_lib");
  m_lightsLib    = m_xmlDoc.append_child(L"lights_lib");
  m_cameraLib    = m_xmlDoc.append_child(L"cam_lib");
  m_settingsNode = m_xmlDoc.append_child(L"render_lib");
  m_sceneNode    = m_xmlDoc.append_child(L"scenes");
  
  if(!a_attachMode)                                       // will do this init later inside HRSceneData::init_existing when open scene
    init_virtual_buffer(false, a_pVBSysMutexLock, a_vbSize);

  m_changeList.clear();
  m_changeList.reserve(2048);
}

void HRSceneData::init_existing(bool a_attachMode, HRSystemMutex* a_pVBSysMutexLock, const size_t a_size)
{
  m_texturesLib  = m_xmlDoc.child(L"textures_lib");
  m_materialsLib = m_xmlDoc.child(L"materials_lib");
  m_lightsLib    = m_xmlDoc.child(L"lights_lib");
  m_cameraLib    = m_xmlDoc.child(L"cam_lib");
  m_geometryLib  = m_xmlDoc.child(L"geometry_lib");
  m_settingsNode = m_xmlDoc.child(L"render_lib");
  m_sceneNode    = m_xmlDoc.child(L"scenes");


  init_virtual_buffer(a_attachMode, a_pVBSysMutexLock, a_size);

  m_changeList.clear();
  m_changeList.reserve(1024);
}

void HRSceneData::init_virtual_buffer(bool a_attachMode, HRSystemMutex* a_pVBSysMutexLock, size_t a_vbSize)
{
  if (a_attachMode)
  {
    if(SharedVirtualBufferIsEnabled())
    {
      bool attached = m_vbCache.Attach(a_vbSize, "HYDRAAPISHMEM2", &g_objManager.m_tempBuffer);
      if (attached)
        m_vbCache.RestoreChunks();
      else
        m_vbCache.Init(4096, "NOSUCHSHMEM", &g_objManager.m_tempBuffer, a_pVBSysMutexLock); // if fail, init single page only, dummy virtual buffer
    }
    else
      m_vbCache.Init(4096, "NOSUCHSHMEM", &g_objManager.m_tempBuffer, a_pVBSysMutexLock);
  }
  else
    m_vbCache.Init(a_vbSize, "HYDRAAPISHMEM2", &g_objManager.m_tempBuffer, a_pVBSysMutexLock);
}

void HRSceneData::clear()
{
  meshes.clear();
  lights.clear();
  materials.clear();
  cameras.clear();
  textures.clear();

  clear_node(m_texturesLib);
  clear_node(m_materialsLib);
  clear_node(m_lightsLib);
  clear_node(m_cameraLib);
  clear_node(m_geometryLib);
  clear_node(m_settingsNode);
  clear_node(m_sceneNode);

  m_commitId = 0;
  m_vbCache.Clear();
  m_textureCache.clear();
  m_iesCache.clear();
  
  m_shadowCatchers.clear();

  m_changeList.clear();
}

void HRFreeImageDLL::Initialize()
{ 
  #ifdef WIN32
  const wchar_t* dllname = L"FreeImage.dll";
  #else
  const wchar_t* dllname = L"libfreeimage.so";
  #endif

  m_dllFreeImageHangle = HydraLoadLibrary(dllname);
  if (m_dllFreeImageHangle == NULL)
  {
    HrError(L"Not found FreeImage.dll/FreeImage.so");
    return;
  }

  bool hasError        = false;
  std::wstring message = L"In FreeImage.dll not found: ";
  
  // Init / Error routines ----------------------------------------------------

  m_pFreeImage_Initialise = (FREEIMAGE_INITIALISE)HydraGetProcAddress(m_dllFreeImageHangle, "FreeImage_Initialise");
  if (m_pFreeImage_Initialise == nullptr)
  {
    message += L"FreeImage_Initialise, ";
    hasError = true;
  }
  else
    m_pFreeImage_Initialise(TRUE);

  // Message output functions -------------------------------------------------

  m_pFreeImage_SetOutputMessage = (FREEIMAGE_SETOUTPUTMESSAGE)HydraGetProcAddress(m_dllFreeImageHangle, "FreeImage_SetOutputMessage");
  if (m_pFreeImage_SetOutputMessage == nullptr)
  {
    message += L"FreeImage_SetOutputMessage, ";
    hasError = true;
  }

  // Allocate / Clone / Unload routines ---------------------------------------

  m_pFreeImage_Allocate = (FREEIMAGE_ALLOCATE)HydraGetProcAddress(m_dllFreeImageHangle, "FreeImage_Allocate");
  if (m_pFreeImage_Allocate == nullptr)
  {
    message += L"FreeImage_Allocate, ";
    hasError = true;
  }

  m_pFreeImage_AllocateT = (FREEIMAGE_ALLOCATET)HydraGetProcAddress(m_dllFreeImageHangle, "FreeImage_AllocateT");
  if (m_pFreeImage_AllocateT == nullptr)
  {
    message += L"FreeImage_AllocateT, ";
    hasError = true;
  }

  m_pFreeImage_Unload = (FREEIMAGE_UNLOAD)HydraGetProcAddress(m_dllFreeImageHangle, "FreeImage_Unload");
  if (m_pFreeImage_Unload == nullptr)
  {
    message += L"FreeImage_Unload, ";
    hasError = true;
  }

  // Load / Save routines -----------------------------------------------------

  m_pFreeImage_Load = (FREEIMAGE_LOAD)HydraGetProcAddress(m_dllFreeImageHangle, "FreeImage_Load");
  if (m_pFreeImage_Load == nullptr)
  {
    message += L"FreeImage_Load, ";
    hasError = true;
  }

  m_pFreeImage_LoadU = (FREEIMAGE_LOADU)HydraGetProcAddress(m_dllFreeImageHangle, "FreeImage_LoadU");
  if (m_pFreeImage_LoadU == nullptr)
  {
    message += L"FreeImage_LoadU, ";
    hasError = true;
  }

  m_pFreeImage_SaveU = (FREEIMAGE_SAVEU)HydraGetProcAddress(m_dllFreeImageHangle, "FreeImage_SaveU");
  if (m_pFreeImage_SaveU == nullptr)
  {
    message += L"FreeImage_SaveU, ";
    hasError = true;
  }

  // Plugin Interface ---------------------------------------------------------

  m_pFreeImage_GetFIFFromFilename = (FREEIMAGE_GETFIFFROMFILENAME)HydraGetProcAddress(m_dllFreeImageHangle, "FreeImage_GetFIFFromFilename");
  if (m_pFreeImage_GetFIFFromFilename == nullptr)
  {
    message += L"FreeImage_GetFIFFromFilename, ";
    hasError = true;
  }

  m_pFreeImage_GetFIFFromFilenameU = (FREEIMAGE_GETFIFFROMFILENAMEU)HydraGetProcAddress(m_dllFreeImageHangle, "FreeImage_GetFIFFromFilenameU");
  if (m_pFreeImage_GetFIFFromFilenameU == nullptr)
  {
    message += L"FreeImage_GetFIFFromFilenameU, ";
    hasError = true;
  }

  m_pFreeImage_FIFSupportsReading = (FREEIMAGE_FIFSUPPORTSREADING)HydraGetProcAddress(m_dllFreeImageHangle, "FreeImage_FIFSupportsReading");
  if (m_pFreeImage_FIFSupportsReading == nullptr)
  {
    message += L"FreeImage_FIFSupportsReading, ";
    hasError = true;
  }

  // File type request routines ------------------------------------------------

  m_pFreeImage_GetFileType = (FREEIMAGE_GETFILETYPE)HydraGetProcAddress(m_dllFreeImageHangle, "FreeImage_GetFileType");
  if (m_pFreeImage_GetFileType == nullptr)
  {
    message += L"FreeImage_GetFileType, ";
    hasError = true;
  }

  m_pFreeImage_GetFileTypeU = (FREEIMAGE_GETFILETYPEU)HydraGetProcAddress(m_dllFreeImageHangle, "FreeImage_GetFileTypeU");
  if (m_pFreeImage_GetFileTypeU == nullptr)
  {
    message += L"FreeImage_GetFileTypeU, ";
    hasError = true;
  }

  // DIB info routines --------------------------------------------------------

  m_pFreeImage_GetImageType = (FREEIMAGE_GETIMAGETYPE)HydraGetProcAddress(m_dllFreeImageHangle, "FreeImage_GetImageType");
  if (m_pFreeImage_GetImageType == nullptr)
  {
    message += L"FreeImage_GetImageType, ";
    hasError = true;
  }

  m_pFreeImage_GetBits = (FREEIMAGE_GETBITS)HydraGetProcAddress(m_dllFreeImageHangle, "FreeImage_GetBits");
  if (m_pFreeImage_GetBits == nullptr)
  {
    message += L"FreeImage_GetBits, ";
    hasError = true;
  }

  m_pFreeImage_GetBPP = (FREEIMAGE_GETBPP)HydraGetProcAddress(m_dllFreeImageHangle, "FreeImage_GetBPP");
  if (m_pFreeImage_GetBPP == nullptr)
  {
    message += L"FreeImage_GetBPP, ";
    hasError = true;
  }

  m_pFreeImage_GetWidth = (FREEIMAGE_GETWIDTH)HydraGetProcAddress(m_dllFreeImageHangle, "FreeImage_GetWidth");
  if (m_pFreeImage_GetWidth == nullptr)
  {
    message += L"FreeImage_GetWidth, ";
    hasError = true;
  }

  m_pFreeImage_GetHeight = (FREEIMAGE_GETHEIGHT)HydraGetProcAddress(m_dllFreeImageHangle, "FreeImage_GetHeight");
  if (m_pFreeImage_GetHeight == nullptr)
  {
    message += L"FreeImage_GetHeight, ";
    hasError = true;
  }

  // Smart conversion routines ------------------------------------------------

  m_pFreeImage_ConvertFromRawBits = (FREEIMAGE_CONVERTFROMRAWBITS)HydraGetProcAddress(m_dllFreeImageHangle, "FreeImage_ConvertFromRawBits");
  if (m_pFreeImage_ConvertFromRawBits == nullptr)
  {
    message += L"FreeImage_ConvertFromRawBits, ";
    hasError = true;
  }

  m_pFreeImage_ConvertTo8Bits = (FREEIMAGE_CONVERTTO8BITS)HydraGetProcAddress(m_dllFreeImageHangle, "FreeImage_ConvertTo8Bits");
  if (m_pFreeImage_ConvertTo8Bits == nullptr)
  {
    message += L"FreeImage_ConvertTo8Bits, ";
    hasError = true;
  }

  m_pFreeImage_ConvertTo32Bits = (FREEIMAGE_CONVERTTO32BITS)HydraGetProcAddress(m_dllFreeImageHangle, "FreeImage_ConvertTo32Bits");
  if (m_pFreeImage_ConvertTo32Bits == nullptr)
  {
    message += L"FreeImage_ConvertTo32Bits, ";
    hasError = true;
  }

  m_pFreeImage_ConvertToFloat = (FREEIMAGE_CONVERTTOFLOAT)HydraGetProcAddress(m_dllFreeImageHangle, "FreeImage_ConvertToFloat");
  if (m_pFreeImage_ConvertToFloat == nullptr)
  {
    message += L"FreeImage_ConvertToFloat, ";
    hasError = true;
  }

  m_pFreeImage_ConvertToRGBF = (FREEIMAGE_CONVERTTORGBF)HydraGetProcAddress(m_dllFreeImageHangle, "FreeImage_ConvertToRGBF");
  if (m_pFreeImage_ConvertToRGBF == nullptr)
  {
    message += L"FreeImage_ConvertToRGBF, ";
    hasError = true;
  }

  m_pFreeImage_CloneMetadata = (FREEIMAGE_CLONEMETADATA)HydraGetProcAddress(m_dllFreeImageHangle, "FreeImage_CloneMetadata");
  if (m_pFreeImage_CloneMetadata == nullptr)
  {
    message += L"FreeImage_CloneMetadata, ";
    hasError = true;
  }

  m_pFreeImage_GetPitch = (FREEIMAGE_GETHEIGHT)HydraGetProcAddress(m_dllFreeImageHangle, "FreeImage_GetPitch");
  if (m_pFreeImage_GetPitch == nullptr)
  {
    message += L"FreeImage_GetPitch, ";
    hasError = true;
  }

  if (hasError)
    HrError(message); // In FreeImage.dll not found: FreeImage_ConvertFromRawBitsEx, FreeImage_ConvertToRGBAF

}
