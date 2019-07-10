#pragma once

/**
\file
\brief Render Driver abstract interface

*/

#include <cstdint>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <iostream>

#include "pugixml.hpp"
#include "HydraAPI.h"

#include "HR_HDRImage.h"
#include "HydraLegacyUtils.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HrError(std::wstring a_str);
void _HrPrint(HR_SEVERITY_LEVEL a_level, const wchar_t* a_str);

template <typename HEAD>
void _HrPrint(std::wstringstream& out, HEAD head)
{
  out << head << std::endl;
}

template <typename HEAD, typename... TAIL>
void _HrPrint(std::wstringstream& out, HEAD head, TAIL... tail)
{
  out << head;
  _HrPrint(out, tail...);
}

template <typename ... Args>
static void HrPrint(HR_SEVERITY_LEVEL a_level, Args ... a_args)
{
  std::wstringstream out;
  _HrPrint(out, a_args...);
  
  std::wstring strOut = out.str();
  _HrPrint(a_level, strOut.c_str());
}

HR_ERROR_CALLBACK getErrorCallback();
HR_INFO_CALLBACK  getPrintCallback();
std::wstring&     getErrWstrObject();
std::wstring&     getErrCallerWstrObject();

template<typename X>
static void HrError(std::wstring a_str, X value)
{
  HrPrint(HR_SEVERITY_ERROR, a_str, value);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/** \brief Internal structure to help driver allocate less memory.
*
*/
struct HRTexResInfo
{
  HRTexResInfo() : id(0), w(0), h(0), rw(0), rh(0), aw(0), ah(0), bpp(4), usedAsBump(false) {}
  int id;           ///< texture id
  int w;            ///< texture width
  int h;            ///< texture height
  int rw;           ///< recommended texture width
  int rh;           ///< recommended texture height
  int aw;           ///< actual texture width
  int ah;           ///< actual texture height
  int bpp;          ///< texture bytes per pixel
  bool usedAsBump;  ///< if texture used as bump it actually may consume double memory due to it's copy in separate 'normalmap' memory storage 
};


/** \brief This structure tell render driver very particular information how much memory and objects it have to allocate
*
* fff
*/
struct HRDriverAllocInfo
{
  HRDriverAllocInfo() : libraryPath(nullptr), resourcesPath(nullptr), stateFileName(nullptr) {}
  
  int32_t imgNum;
  int32_t matNum;
  int32_t geomNum;
  int32_t lightNum;

  int32_t lightsWithIESNum;
  int32_t envLightTexSize;
  bool    envIsHDR;

  int64_t imgMem;
  int64_t imgMemAux;
  int64_t geomMem;

  const wchar_t* libraryPath;
  const wchar_t* resourcesPath;
  const wchar_t* stateFileName;

  const HRTexResInfo* imgResInfoArray;
};


/** \brief This structure tell render HydraAPI what features are supported by driver.
*
*/
struct IHRRenderDriver;
struct HRDriverInfo
{
  HRDriverInfo() : supportMultiMaterialInstance(false), supportHDRFrameBuffer(false), supportHDRTextures(false),
                   supportImageLoadFromInternalFormat(false), supportImageLoadFromExternalFormat(false),
                   supportMeshLoadFromInternalFormat(false), supportLighting(false), createsLightGeometryItself(false),
                   memTotal(0), supportGetFrameBufferLine(false), supportUtilityPrepass(false), supportDisplacement(false) {}


  bool supportMultiMaterialInstance;
  bool supportHDRFrameBuffer;
  bool supportHDRTextures;
  bool supportImageLoadFromInternalFormat;
  bool supportImageLoadFromExternalFormat;
  bool supportMeshLoadFromInternalFormat;
  bool supportLighting;
  bool createsLightGeometryItself;
  bool supportGetFrameBufferLine;
  bool supportUtilityPrepass;
  bool supportDisplacement;
  int64_t memTotal = 0l;

  std::wstring driverName = L"";

  using TCreateFunction = IHRRenderDriver*(*)();
  TCreateFunction createFunction = nullptr;
};

/** \brief This structure show which objects we have to Update if we change some of other objects
*
* It's like back propagation. If you have, for example meshDependsOfMaterial equal true, than if you change material,
* API will have to Update not material only, but all associated with this material meshes also.
*/
struct HRDriverDependencyInfo
{
  HRDriverDependencyInfo() : materialDependsOfTexture(false), meshDependsOfMaterial(true), lightDependsOfMesh(true), 
                             sceneDependsOfMesh(true), sceneDependsOfLight(true),
                             needRedrawWhenCameraChanges(true), allowInstanceReorder(true) {}

  bool materialDependsOfTexture;
  bool meshDependsOfMaterial;
  bool lightDependsOfMesh;
  bool sceneDependsOfMesh;
  bool sceneDependsOfLight;
  
  bool needRedrawWhenCameraChanges; ///< OpenGL renderers 'redraw' scene each frame, tracers don't need to 'redraw' scene, just change camera
  bool allowInstanceReorder;

};

/** \brief Input Mesh pointers
*
*  This driver will _always_ recieve mesh in this format. Never mind how it was set from application.
*
*/
struct HRMeshDriverInput
{
  HRMeshDriverInput() : vertNum(0), triNum(0), pos4f(nullptr), norm4f(nullptr), texcoord2f(nullptr), tan4f(nullptr), indices(nullptr), triMatIndices(nullptr) {}

  int    vertNum;
  int    triNum;
  const float* pos4f;
  const float* norm4f;
  const float* texcoord2f;
  const float* tan4f;
  const int*   indices;
  const int*   triMatIndices;

  const char*  allData;
};


/** \brief Basic Render Driver.
*
*  This driver should not alloc or realloc anything in dynamic. It supposed to alloc all needed memory by call 'AllocAll'  
*
*/
struct IHRRenderDriver
{
  IHRRenderDriver()          = default;
  virtual ~IHRRenderDriver() = default;
  IHRRenderDriver(const IHRRenderDriver& a_val)            = delete;
  IHRRenderDriver& operator=(const IHRRenderDriver& a_val) = delete;

  virtual void              ClearAll()                         = 0; ///< clear everethyng insidide render engine (render driver)
  virtual HRDriverAllocInfo AllocAll(HRDriverAllocInfo a_info) = 0; ///< actual destructor, COM-like (!!!)

  /**
  \brief update image inside render engine (render driver)  
  \param a_texId    - image id
  \param w          - image width
  \param h          - image height
  \param bpp        - bytes per pixel
  \param a_data     - pointer to image pixels data. The actual format depends on the bpp value. if (bpp == 4) => each pixel is int32_t RGBA,  if (bpp == 16) each pixel is float4
  \param a_texNode  - XML node of texture object to pass custom parameters to render engine
  \return true if update was sucess, false otherwise

  */
  virtual bool    UpdateImage(int32_t a_texId, int32_t w, int32_t h, int32_t bpp, const void* a_data, pugi::xml_node a_texNode) = 0; 

  /**
  \brief update material inside render engine (render driver)
  \param a_matId         - material id
  \param a_materialNode  - XML node of material object to pass custom parameters to render engine
  \return true if update was sucess, false otherwise

  */
  virtual bool    UpdateMaterial(int32_t a_matId, pugi::xml_node a_materialNode) = 0; 

  /**
  \brief update light inside render engine (render driver)
  \param a_lightId   - light id
  \param a_lightNode - XML node of light object to pass custom parameters to render engine
  \return true if update was sucess, false otherwise

  */
  virtual bool    UpdateLight(int32_t a_lightId, pugi::xml_node a_lightNode)   = 0;

  /**
  \brief update light inside render engine (render driver)
  \param a_meshId    - mesh id
  \param a_meshNode  - XML node of mesh object to pass custom parameters to render engine
  \param a_input     - actual geom data
  \param a_batchList - an auxilarry array of "batches". Each batch is a sequence of triangles with the same material.
  \param listSize    - a_batchList array size
  \return true if update was sucess, false otherwise

  If you pass L"-sort_indices 1" to hrInit() call, the API will sort all mesh triangles by their materials and guarantee that each material will be met only once in a_batchList.
  Otherwise same material id could met several times in a_batchList.

  */
  virtual bool    UpdateMesh(int32_t a_meshId, pugi::xml_node a_meshNode, const HRMeshDriverInput& a_input, const HRBatchInfo* a_batchList, int32_t listSize) = 0;

  /**
  \brief update image from file 
  \param a_texId     - image id
  \param a_fileName  - file name from where image should be loaded 
  \param a_texNode   - XML node of image object to pass custom parameters to render engine
  \return true if update was sucess, false otherwise

    You can just return false if your render-driver don't support that

  */
  virtual bool    UpdateImageFromFile(int32_t a_texId, const wchar_t* a_fileName, pugi::xml_node a_texNode) = 0;


  /**
  \brief update mesh from ".vsgf" file
  \param a_meshId   - mesh object id
  \param a_meshNode - XML node of mesh object to pass custom parameters to render engine
  \param a_fileName - ".vsgf" file name from where mesh should be loaded
  \return true if update was sucess, false otherwise

  You can just return false if your render-driver don't support that

  */
  virtual bool    UpdateMeshFromFile (int32_t a_meshId, pugi::xml_node a_meshNode, const wchar_t* a_fileName) = 0;

  /**
  \brief Camera settings are separeted from others, because they are not the part of "scene library".
         They set every time from scratch before begin rendering. I.e. Each frame.

  */
  virtual bool    UpdateCamera(pugi::xml_node a_camNode)        = 0;

  /**
  \brief Render settings are separeted from others, because they are not the part of "scene library".
         They set every time from scratch before begin rendering. I.e. Each frame.

  */
  virtual bool    UpdateSettings(pugi::xml_node a_settingsNode) = 0;

  //// Scene Instance and render methods (i.e. actually render some thing ... )
  //
  virtual void    BeginScene(pugi::xml_node a_sceneNode) = 0;  ///< this method is called before instences wiil be created (i.e. beforer all calls of InstanceMeshes/InstanceLights)
  virtual void    EndScene()   = 0;                            ///< this method is called after instences wiil be created (i.e. beforer all calls of InstanceMeshes/InstanceLights)

  /**
  \brief draw all instances of target mesh
  \param a_mesh_id     - mesh id
  \param a_matrix      - transformation matrices array. one matrix for one instance. each matrix is float[16]. So, the total a_matrix array size is a_instNum*16 
  \param a_instNum     - instances number for mesh with id == a_mesh_id.
  \param a_lightInstId - points to light instance if if this is a light mesh or -1 otherwise.
  /param a_remapId     - points array of (remapId || -1)

  */
  virtual void    InstanceMeshes(int32_t a_mesh_id, const float* a_matrix, int32_t a_instNum, const int* a_lightInstId, const int* a_remapId, const int* a_realInstId)  = 0;

  /**
  \brief add (and draw) all light instances of target light
  \param a_light_id      - light id
  \param a_matrix        - transformation matrices array. one matrix for one light instance. each matrix is float[16]. So, the total a_matrix array size is a_instNum*16
  \param a_custAttrArray - array of xml_nodes that stores custom attributes for light instance.
  \param a_instNum       - instances number for light with id == a_light_id.
  \param a_lightGroupId  - light group id if this light is bounded to some light group.

  */
  virtual void    InstanceLights(int32_t a_light_id, const float* a_matrix, pugi::xml_node* a_custAttrArray, int32_t a_instNum, int32_t a_lightGroupId) = 0;

  virtual void    Draw() = 0; ///< perform draw pass

  // frame buffer and gbuffer functions
  //
  virtual HRRenderUpdateInfo HaveUpdateNow(int a_maxRaysPerPixel) = 0; ///< signal if off-line render have an update. Could be called during async rendering of the frame. real-time renders should just return true with 100.0f progress

  /**
  \brief get frame buffer content to HDR (float4) a_out. 
  \param w     - target output width (supposed to be equal to internal framebuffer width. You may return false if this condition was broken).
  \param h     - target output height (supposed to be equal to internal framebuffer height. You may return false if this condition was broken).
  \param a_out - output in float4 format. i.e - w*h*(sizeof(float)*4)
  \param a_layerName - image name. - "color", "gbuffer1", "gbuffer2", "gbuffer3". 

    Note that you don't have to implement "gbuffer1", "gbuffer2", "gbuffer3". It's optional. Just return false if you don't support that. 
    These gbuffer layes are not some-thing general. They are compressed layers of gbuffer that is stricly related to Hydra realistic renderer driver and their format is a bit 'proprietary'.

  */
  virtual void    GetFrameBufferHDR(int32_t w, int32_t h, float*   a_out, const wchar_t* a_layerName) = 0; 
  virtual void    GetFrameBufferLDR(int32_t w, int32_t h, int32_t* a_out)                             = 0; ///< get frame buffer content to LDR (int32_t) a_out. 

  /**
  \brief more compact way of getting frame buffer. 
         
    Note that your custom render driver don't have to support them. 
    The API should call GetFrameBufferHDR/GetFrameBufferLDR to cache data in some internal location if you don't support these functions.
    But if you support them, the copying of frame buffer data can be optimised (i.e. you don't have to allocate full screen image). 
    #TODO: implement what i wrote here :)

    If you sopport these functions set 'supportGetFrameBufferLine' flag when rerurn from Info()
  */
  virtual void    GetFrameBufferLineHDR(int32_t a_xBegin, int32_t a_xEnd, int32_t y, float* a_out, const wchar_t* a_layerName) {}
  virtual void    GetFrameBufferLineLDR(int32_t a_xBegin, int32_t a_xEnd, int32_t y, int32_t* a_out)                           {}
  
  /**
  \brief These functions is needed to offline multithreaded frame buffer update implementation.
         I.e. you after Commit operation, computation are just start and they are detached.  #TODO: describe this
         ...
  */
  virtual void    LockFrameBufferUpdate() {}
  virtual void    UnlockFrameBufferUpdate() {}
  
  
  
  virtual void    EvalGBuffer() { } ///< run gbuffer evaluation (which can be async in general).

  virtual void    GetGBufferLine(int32_t a_lineNumber, HRGBufferPixel* a_lineData, int32_t a_startX, int32_t a_endX, const std::unordered_set<int32_t>& a_shadowCatchers) = 0; ///< get single gbuffer line (because the whole gbuffer is quite big!)

  // info and devices
  //
  virtual void GetRenderDriverName(std::wstring &name) {}; ///< get name by which the render driver is registered
  //HRDriverInfo Info() { return driverInfo; };                                            ///< return render driver info
  virtual HRDriverDependencyInfo DependencyInfo() { return HRDriverDependencyInfo(); }  ///< return render dependency info (look at OpenGL1 implementation with display lists to understand this. You have to update mesh when you update material because old mesh display list contains old material color).

  virtual const HRRenderDeviceInfoListElem* DeviceList() const = 0; ///< this method is for multi-GPU render. return device list or nullptr
  virtual bool  EnableDevice(int32_t id, bool a_enable)        = 0; ///<  this method is for multi-GPU render; enable target device or disable it; if fail, return false;

  virtual void  SetInfoCallBack(HR_INFO_CALLBACK a_cllBack) { m_pInfoCallBack = a_cllBack; }

  // specific signals for create correct relations between blend leafs and e.t.c
  // for most of renderers these functions do nothing
  //
  virtual void BeginMaterialUpdate() {} ///< called at the beginning of materials update phase
  virtual void EndMaterialUpdate() {}   ///< called at the end of materials update phase

  virtual void BeginTexturesUpdate() {} ///< called at the beginning of textures update phase
  virtual void EndTexturesUpdate() {}   ///< called at the end of textures update phase

  virtual void BeginLightsUpdate() {}   ///< called at the beginning of lights update phase
  virtual void EndLightsUpdate() {}     ///< called at the end       of lights update phase

  virtual void BeginGeomUpdate() {}     ///< called at the beginning of geom update phase
  virtual void EndGeomUpdate() {}       ///< called at the end       of geom update phase

  virtual void BeginFlush() {}          ///< called at the beginning of flush phase
  virtual void EndFlush() {}            ///< called at the end of flush phase

  virtual void ExecuteCommand(const wchar_t* a_cmd, wchar_t* a_out) {} ///< exec custom command
  virtual void SetLogDir(const wchar_t* a_logDir, bool a_hideCmd) {}   ///< hide render process if have such and redirect comand line output to the log file

  virtual std::shared_ptr<HydraRender::HDRImage4f> GetFrameBufferImage(const wchar_t* a_imageName) { return nullptr; } ///< get smart pointer to frame buffer storage. needed for post-process connection to render

protected:

  HR_INFO_CALLBACK m_pInfoCallBack = nullptr;
  HRDriverInfo driverInfo;
};


struct RenderDriverFactory
{
private:
  static std::unordered_map<std::wstring, HRDriverInfo> create_methods;

public:
  RenderDriverFactory() = delete;

  static bool Register(const std::wstring &renderdriver_name, HRDriverInfo &renderdriver_info)
  {
    auto it = create_methods.find(renderdriver_name);
    if (it == create_methods.end())
    {
      create_methods[renderdriver_name] = renderdriver_info;
      auto tmp = ws2s(renderdriver_name);
      std::cout << "Registered render driver with name : " << tmp.c_str() << std::endl;
      return true;
    }
    return false;
  }

  static IHRRenderDriver* Create(const std::wstring& renderdriver_name)
  {
    auto it = create_methods.find(renderdriver_name);
    if (it != create_methods.end())
      return it->second.createFunction();

    std::wstring core_drv_name = L"HydraCore";
    if(renderdriver_name == core_drv_name)
      return nullptr;

    auto driver_name_str =  ws2s(renderdriver_name);
    //std::cerr << "Render driver " << driver_name_str.c_str() << " is not registered!" << std::endl;
    HrPrint(HR_SEVERITY_WARNING, L"Render driver ", driver_name_str.c_str(), L" is not registered!");
    return nullptr;
  }

  static HRDriverInfo GetDriverInfo(const std::wstring& renderdriver_name)
  {
    auto it = create_methods.find(renderdriver_name);
    if (it != create_methods.end())
      return it->second;

    std::wstring core_drv_name = L"HydraCore";
    if(renderdriver_name == core_drv_name)
      return HRDriverInfo();

    auto driver_name_str =  ws2s(renderdriver_name);
    //std::cerr << "Render driver " << driver_name_str.c_str() << " is not registered!" << std::endl;
    HrPrint(HR_SEVERITY_WARNING, L"Render driver ", driver_name_str.c_str(), L" is not registered!");
    return HRDriverInfo();
  }

  static std::vector<std::wstring> GetListOfRegisteredDrivers()
  {
    std::vector<std::wstring> res(create_methods.size());
    for(const auto &it : create_methods)
    {
      res.push_back(it.second.driverName);
    }
    return res;
  }
};

IHRRenderDriver* CreateHydraConnection_RenderDriver();
IHRRenderDriver* CreateDebugPrint_RenderDriver();


static constexpr uint32_t MAX_TEXTURE_RESOLUTION = 16384;

/**
\brief use this function to create your own render drivers.
\param a_className - just a render name
\param a_pDriver   - a pointer to your specific implemetation of render driver

*/
HAPI HRRenderRef hrRenderCreateFromExistingDriver(const wchar_t* a_className, std::shared_ptr<IHRRenderDriver> a_pDriver);
