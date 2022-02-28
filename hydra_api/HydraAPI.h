#pragma once

#include <cstdint>
#include <limits>
#include <vector>
#include "pugixml.hpp"


/**
\file
\brief Hydra Renderer API.

A short overview of Hydra API ideology:
  - To change any object you need first to open it. 
  - After you finish changes, you need to close object. It's like working with files in operating system. 
  - All structure data are incapsulated. You don't know their internal format. 
  - The API works with tiny set of abstract types. They are: (HRMesh, HRLight, HRMaterial, HRCamera, HRTextureNode, HRSceneData, HRSceneInst, HRRender).
    Objects of types HRMaterial and HRTextureNode hovewer, can have subtypes - a different actual implementation of Material and TextureNode.
 
    It is forced necessity to have subtypes for them because there could be extrimely large amount of materials, textures, shaders (don't have them for current) and e.t.c. atrist tools.
    So, unfortunately, in several cases you do have to know which subtype is used. Usually is when you create such object and/or change it's parameters. 
    Subtypes have their own names without HR prefix. For example HydraMaterial and BlendMaterial are a subtypes of HRMaterial.

    Here is the full list of subtypes: 
    - HRMaterial:    HydraMaterial, BlendMaterial, BRDFLeafLambert, BRDFLeafPhong, ... any other BRDFLeafXxx.
    - HRTextureNode: Array1D, Texture2DHDR, Texture2DLDR, Texture2DProcHDR, Texture2DProcLDR, TextureAdvanced.
    .

  - The function of API usually have subtype name as a part of their own name. For example hrTextureCreateAdvanced create HRTextureNode of subtype TextureAdvanced.
*/

#if defined(WIN32)
  
  #define HAPI

  //#ifdef HAPI_DLL
  //#define HAPI __declspec(dllexport) ///< mark all functions as 'extern "C"'; This is needed if you want to load DLL in dynamic;
  //#else
  //#define HAPI __declspec(dllimport) ///< mark all functions as 'extern "C"'; This is needed if you want to load DLL in dynamic;
  //#endif

#else

  //#if defined(__GNUC__)

  #define HAPI                       ///< mark all functions as 'extern "C"'; This is needed if you want to load DLL dynamically;

#endif


/**
\brief Information about total current numbers of all objects inside API in it's current state. 
*/
struct HRSceneLibraryInfo
{
  HRSceneLibraryInfo() : texturesNum(0), materialsNum(0), meshesNum(0), 
                         camerasNum(0), scenesNum(0), renderDriversNum(0){}

  int32_t texturesNum;
  int32_t materialsNum;
  int32_t meshesNum;
  
  int32_t camerasNum;
  int32_t scenesNum;
  int32_t renderDriversNum;

};


struct HRMeshRef     { int32_t id; HRMeshRef()     : id(-1) {} }; ///< Mesh  reference.
struct HRLightRef    { int32_t id; HRLightRef()    : id(-1) {} }; ///< Light reference.
struct HRMaterialRef { int32_t id; HRMaterialRef() : id(-1) {} }; ///< Material reference.
struct HRCameraRef   { int32_t id; HRCameraRef()   : id(-1) {} }; ///< Camera reference.


/**
\brief Node reference. Nodes are render textures, pretty much like any other nodes used in 3D modeling software.

A common 3D modeling software, like 3ds Max, Blender and espetially Houdiny uses concept called Nodes.

Nodes are fixed function or programmible units, like a functions - take some from input and put some other to output.
Nodes usually can be bind to some material slots (or light slots). Users will build a trees from nodes to do some complex stuff.

The simplest node is a Bitmap, created with hrTexture2D. Bitmaps are simple 2D images. Just a 2D array of values.
Their input is a texture coordinates, output - texture color. Texture can be bound to material slot.

The more complex example is 'Falloff' node - a blend for 2 textures based of angle between view vector and surface normal.
Falloff is a fixed function node with predefined parameters.

Any parameters (for fixed function or programmible nodes) can be set via sequence of hrTextureNodeOpen(node), ... , hrTextureNodeClose(node);
*/

struct HRTextureNodeRef { int32_t id; HRTextureNodeRef () : id(-1) {} }; ///< TextureNode reference.
struct HRSceneInstRef   { int32_t id; HRSceneInstRef()    : id(-1) {} }; ///< SceneInst reference.
struct HRRenderRef      { int32_t id; HRRenderRef()       : id(-1) {} }; ///< RenderSettings reference.


/// Settings for model importer

struct HRModelLoadInfo
{
    HRModelLoadInfo() : mtlRelativePath(nullptr),
                        useMaterial(false),
                        useCentering(true),
                        transform{1.0f, 0.0f, 0.0f, 0.0f,
                                  0.0f, 1.0f, 0.0f, 0.0f,
                                  0.0f, 0.0f, 1.0f, 0.0f,
                                  0.0f, 0.0f, 0.0f, 1.0f} {};
    wchar_t* mtlRelativePath;   ///< Relative path to .mtl files. Default setting = nullptr => importer will look for them in the same folder as .obj file.
    bool     useMaterial;       ///< Flag, indicating whether to apply materials from .obj file or not. Default setting = true.
    bool     useCentering;      ///< Flag, indicating whether to perform centering around [0.0, 0.0, 0.0] or not. Default setting = true.
    float    transform[16];     ///< Transform matrix. Default setting = identity matrix. This flag overwrites 'useCentering' flag.
};


/// When open any HRObject you must specify one of these 2 flags.

enum HR_OPEN_MODE {
  HR_OPEN_EXISTING  = 0,  ///< This flag should be used when open object if you want to change the existing object with non empty data.
  HR_WRITE_DISCARD  = 1,  ///< This flag should be used when open object if you want to fill object properties first time. Or clear object and fill it from "clear page".
  HR_OPEN_READ_ONLY = 2,  ///< This flag should be used when open object if you want to read current object state. When close object, your changes will not be applyed.
};


/// Severity levels of information that main application gets from API.

enum HR_SEVERITY_LEVEL {
  HR_SEVERITY_DEBUG          = 0,  ///< The info which your app should display only in debug mode; else - just ignore it.
  HR_SEVERITY_INFO           = 1,  ///< Normal info that you app would be great to display.
  HR_SEVERITY_WARNING        = 2,  ///< Some-thing went wrong, but the work with API can be continued. It is recommended to display such infromation. 
  HR_SEVERITY_ERROR          = 3,  ///< Some-thing went damn wrong. The work with API is still can be continued, but the result is undefined. 
  HR_SEVERITY_CRITICAL_ERROR = 4,  ///< Absolutely bad thing had happened. Please display error message and immediately call hrDestroy().
};


/**
\brief General callback for printing info and error messages.
\param a_message     - actual error message that will be passed to callback when error happened.
\param a_callerPlace - the last string rememberd by hrErrorCallerPlace before error have rised.
\param a_level       - severity leve of message.
Called each time when any error rises.
*/
typedef void(*HR_INFO_CALLBACK)(const wchar_t* a_message, const wchar_t* a_callerPlace, HR_SEVERITY_LEVEL a_level);


/**
\brief 2D procedural texture rendering callback for HDR images.
\param a_buffer a float buffer of size w*h*4. Write your pixels here.
\param w            - desired width of texture. Always power of 2.
\param h            - desired height of texture. Always power of 2.
\param a_customData - a pointer to your custom data structure (needed for rendering texture to bitmap) that callback knows.
 
Because render API is abstracted from concrete HW (and basically, targeted for a GPU), the tradition approach to procedural textures, generated on the fly is nearly impossible.
However, if procedural texture depends only on texture coords (i.e. classical, 2D texture), render can precompute bitmap with the desired resolution.
The resolution is automatically determined by the renderer and the way that happens is not specified.
Due to that your call back must be able to render bitmap from procedural representation in any power of 2 resolutions.
*/
typedef void(*HR_TEXTURE2D_PROC_HDR_CALLBACK)(float* a_buffer, int w, int h, void* a_customData);


/**
\brief 2D procedural texture rendering callback for LDR images.
\param a_buffer     - unsigned char buffer of size w*h*4. Write your pixels here.
\param w            - desired width of texture. Always power of 2.
\param h            - desired height of texture. Always power of 2.
\param a_customData - a pointer to your custom data structure (needed for rendering texture to bitmap) that callback knows.

Because render API is abstracted from concrete HW (and basically, targeted for a GPU), the tradition approach to procedural textures, generated on the fly is nearly impossible.
However, if procedural texture depends only on texture coords (i.e. classical, 2D texture), render can precompute bitmap with the desired resolution.
The resolution is automatically determined by the renderer and the way that happens is not specified.
Due to that your call back must be able to render bitmap from procedural representation in any power of 2 resolutions.
*/
typedef void(*HR_TEXTURE2D_PROC_LDR_CALLBACK)(unsigned char* a_buffer, int w, int h, void* a_customData);


struct HRInitInfo
{
  HRInitInfo() : copyTexturesToLocalFolder(false), localDataPath(true), sortMaterialIndices(true), computeMeshBBoxes(true), saveChanges(true),
                 vbSize(int64_t(2048)*int64_t(1024*1024)) {}

  bool    copyTexturesToLocalFolder; ///<!
  bool    localDataPath            ; ///<!
  bool    sortMaterialIndices      ; ///<!
  bool    computeMeshBBoxes        ; ///<!
  bool    saveChanges              ; ///<!    
  int64_t vbSize                   ; ///<! virtual buffer size in bytes.
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
\brief Return last error message. If no error, return nullptr.
*/
HAPI const wchar_t* hrGetLastError();


/**
\brief Remember inside API input string to further prepend it to error message if error happened.
\param a_placeName - input string.
\param a_line      - input line number. If 0, just ignored.

This function needed to mark a place in use program, where last error is happened. For example, it can be function name and line number.
Usually, you call this function inside some block to find out where error had happened.
For debug purposes you can call hrErrorCallerPlace(L"myFunction", __LINE__); several times inside one function or code block.

Note that this function only sets internal string state. If you set in in one function hrErrorCallerPlace(L"MyFirstFunction"), but error happend in the other function 
(than don't have hrErrorCallerPlace(L"MyOtherFunction"), you will get an old place name in error message - MyFirstFunction; )

It also don't have any stack or e.t.c., just a state, just one string! So please, track you call tree by yourself!
*/
HAPI void hrErrorCallerPlace(const wchar_t* a_placeName, int a_line = 0);


/**
\brief Set your custome printing error callback.
*/
HAPI void hrInfoCallback(HR_INFO_CALLBACK pCallback);


/**
\brief This structure represents quick info about scene library.
*/
struct HRSceneLibraryFileInfo
{
  HRSceneLibraryFileInfo() : exists(false), valid(false), empty(false), lastStateId(0){}

  const bool isOk() const { return exists && valid && !empty && (lastStateId > 0); }

  bool    exists;
  bool    valid;
  bool    empty;
  int32_t lastStateId;
};


/**
\brief Quick check if scene library exists in the file system at target path.
\param a_libPath       - a path to some folder that we want to check.
\param a_quickResponce - immediate responce that some thing is wrong with the scene library. For example bad references or sms like that. Must be at least 256 wchars or nullptr.
\return info about scene library at path a_libPath.
*/
HAPI HRSceneLibraryFileInfo hrSceneLibraryExists(const wchar_t* a_libPath, wchar_t a_quickResponce[256]);


/**
\brief Set current scene data path (scene library).  
\param a_libPath  - a path to the scene library - some folder that will hold all exported objects.
\param a_openMode - create new scene or open existing?

 Destroy All SceneData/SceneLibrary if (a_openMode == HR_WRITE_DISCARD).
 Passing nullptr or L"" to a_libPath will cause just to clear everything.

 \return 1 if succeded, 0 otherwise.
*/
HAPI int32_t hrSceneLibraryOpen(const wchar_t* a_libPath, HR_OPEN_MODE a_openMode, HRInitInfo a_initInfo = HRInitInfo());


/**
  \brief Destroy everything.
*/
HAPI void hrSceneLibraryClose();


/**
\brief Get information about current "scene library" / "API state".
*/
HAPI HRSceneLibraryInfo hrSceneLibraryInfo();


/**
\brief Creates 2D texture from file.
\param a_fileName - file name.
\param w          - texture width;   Optional parameter, may be used by renderer as a hint for effitiency consideretions. In case if it is not set, render read it from file.
\param h          - texture height;  Optional parameter, may be used by renderer as a hint for effitiency consideretions. In case if it is not set, render read it from file.
\param bpp        - bytes per pixel; Optional parameter, may be used by renderer as a hint for effitiency consideretions. In case if it is not set, render read it from file.

 Creates 2D texture from file.
 Passing w,h or bpp not equal to -1 ask render to resize texture; however render does not have to resize texture. It is just a hint.
 Note that all 'hrTexture2DCreateFromFile***' functions have their internal cache by 'a_fileName'. if you pass same file name twice, this function return existing texture id second time.
 If you really need to update texture with new file data, use hrTextureUpdateFromFile please.
*/
HAPI HRTextureNodeRef  hrTexture2DCreateFromFile(const wchar_t* a_fileName, int w = -1, int h = -1, int bpp = -1);


/**
\brief Create 2D texture from file with Delayed Load (DL means "Delayed Load").
\param a_fileName            - file name.
\param w                     - texture width;   Optional parameter, may be used by renderer as a hint for effitiency consideretions. In case if it is not set, render read it from file.
\param h                     - texture height;  Optional parameter, may be used by renderer as a hint for effitiency consideretions. In case if it is not set, render read it from file.
\param bpp                   - bytes per pixel; Optional parameter, may be used by renderer as a hint for effitiency consideretions. In case if it is not set, render read it from file.
\param a_copyFileToLocalData - indicates that texture should be copied to "data" folder (for further transmittion, for example).

 The "Delayed Load" means that texture will be load to memory only when passing it to render driver (or by render driver itself, if it can load images from external format).
 The advantage of using textures with delayed load is that if some texture is not really needed for rendering current frame it won't be touched at all.

 The must-to-use case is when you know you are going to do network rendering on several workstations.
 Because of well compression rate in traditional image files it is 100% desireable to pass initial compresed data to render driver to it can deliver textures through the network in their initial well-compressed format. 

 The normal case also - if you do know that your texture don't have full size copy in memory.
 Nevertheless, it is absolutely ok to use this function if you want to save memory or prevent unnecessary disk flush of internal HydraAPI cache in other cases.

 Note that all 'hrTexture2DCreateFromFile***' functions have their internal cache by 'a_fileName'. if you pass same file name twice, this function return existing texture id second time.
 If you really need to update texture with new file data, use hrTextureUpdateFromFile please.
*/
HAPI HRTextureNodeRef  hrTexture2DCreateFromFileDL(const wchar_t* a_fileName, int w = -1, int h = -1, int bpp = -1, bool a_copyFileToLocalData = false);


/**
\brief Update 2D texture from file.
\param a_texRef   - old texture reference.
\param a_fileName - new file name.
\param w          - new texture width.
\param h          - new texture height.
\param bpp        - new bytes per pixel.

 Use this function if you really need to update texture with new file data.
*/
HAPI HRTextureNodeRef hrTexture2DUpdateFromFile(HRTextureNodeRef a_texRef, const wchar_t* a_fileName, int w = -1, int h = -1, int bpp = -1);


/**
\brief Create 2D texture from memory.
\param w      - texture width.
\param h      - texture height. 
\param bpp    - bytes per pixel. 
\param a_data - pointer to image data.
*/
HAPI HRTextureNodeRef  hrTexture2DCreateFromMemory(int w, int h, int bpp, const void* a_data);


/**
\brief Update 2D texture from memory.
\param a_texRef - old texture reference.
\param w        - new texture width.
\param h        - new texture height.
\param bpp      - new bytes per pixel.
\param a_data   - pointer to new image data.
*/
HAPI HRTextureNodeRef hrTexture2DUpdateFromMemory(HRTextureNodeRef a_texRef, int w, int h, int bpp, const void* a_data);


/**
\brief Create procedural 2D texture with callback.
\param a_proc           - a callback that must be abble to render procedural texture in any power of 2 resolution.
\param a_customData     - a pointer to custom data that will be passed to callback.
\param a_customDataSize - data size in bytes.
\param w                - texture width; optional.  Should be set as a hint for renderer if you know desired texture resolution.
\param h                - texture height; optional. Should be set as a hint for renderer if you know desired texture resolution.

 See description of HR_TEXTURE2D_PROC_HDR_CALLBACK for more details.
 If you don't know desired resolution please set "-1" for both width and height!
*/
HAPI HRTextureNodeRef  hrTexture2DCreateBakedHDR(HR_TEXTURE2D_PROC_HDR_CALLBACK a_proc,
                                                 void *a_customData, int a_customDataSize, int w = -1, int h = -1);

/**
\brief Create procedural 2D texture with callback.
\param a_proc           - a callback that must be abble to render procedural texture in any power of 2 resolution.
\param a_customData     - a pointer to custom data that will be passed to callback.
\param a_customDataSize - data size in bytes.
\param w                - texture width; optional.  Should be set as a hint for renderer if you know desired texture resolution.
\param h                - texture height; optional. Should be set as a hint for renderer if you know desired texture resolution.

See description of HR_TEXTURE2D_PROC_LDR_CALLBACK for more details.
If you don't know desired resolution please set "-1" for both width and height!
*/
HAPI HRTextureNodeRef  hrTexture2DCreateBakedLDR(HR_TEXTURE2D_PROC_LDR_CALLBACK a_proc,
                                                 void *a_customData, int a_customDataSize, int w = -1, int h = -1);

/**
\brief Update 2D texture from callback.
\param a_texRef         - old texture reference.
\param a_proc           - a callback that must be abble to render procedural texture in any power of 2 resolution.
\param a_customData     - a pointer to custom data that will be passed to callback.
\param a_customDataSize - data size in bytes.
\param w                - texture width; optional. Should be set as a hint for renderer if you know desired texture resolution.
\param h                - texture height; optional. Should be set as a hint for renderer if you know desired texture resolution.
*/
HAPI HRTextureNodeRef  hrTexture2DUpdateBakedHDR(HRTextureNodeRef a_texRef, HR_TEXTURE2D_PROC_HDR_CALLBACK a_proc,
                                                 void *a_customData, int a_customDataSize, int w, int h);

/**
\brief Update 2D texture from callback.
\param a_texRef         - old texture reference.
\param a_proc           - a callback that must be abble to render procedural texture in any power of 2 resolution.
\param a_customData     - a pointer to custom data that will be passed to callback.
\param a_customDataSize - data size in bytes.
\param w                - texture width; optional. Should be set as a hint for renderer if you know desired texture resolution.
\param h                - texture height; optional. Should be set as a hint for renderer if you know desired texture resolution.
*/
HAPI HRTextureNodeRef  hrTexture2DUpdateBakedLDR(HRTextureNodeRef a_texRef, HR_TEXTURE2D_PROC_LDR_CALLBACK a_proc,
                                                 void *a_customData, int a_customDataSize, int w, int h);


/**
\brief Get raw data for custom texture object.
\param a_texRef - input  texture reference.
\param pW       - output texture width.  It will be written with 0 if hrTexture2DGetDataLDR operation is invalid.
\param pH       - output texture height. It will be written with 0 if hrTexture2DGetDataLDR operation is invalid.
\param a_pBPP   - output bytes per pixel (4 for LDR and 16 for HDR).
*/
HAPI void hrTexture2DGetSize(HRTextureNodeRef a_texRef, int* pW, int* pH, int* a_pBPP);


/**
\brief Get raw data for custom texture object.
\param a_texRef - input  texture reference.
\param pW       - output texture width. It will be written with 0 if hrTexture2DGetDataLDR operation is invalid. 
\param pH       - output texture height. It will be written with 0 if hrTexture2DGetDataLDR operation is invalid.
\param a_pData  - output data pointer that points to array of size (width*height). 
*/
HAPI void hrTexture2DGetDataLDR(HRTextureNodeRef a_texRef, int* pW, int* pH, int* a_pData);


/**
\brief Get raw data for custom texture object.
\param a_texRef - input  texture reference.
\param pW       - output texture width. It will be written with 0 if hrTexture2DGetDataHDR operation is invalid.
\param pH       - output texture height. It will be written with 0 if hrTexture2DGetDataHDR operation is invalid.
\param a_pData  - output data pointer that points to array of size (width*height).
*/
HAPI void hrTexture2DGetDataHDR(HRTextureNodeRef a_texRef, int* pW, int* pH, float* a_pData);


/**
\brief Create special object, called "Advanced Texture".
\param a_texType - Advanced texture type. Can be "falloff" or "dirt".
\param a_objName - object name. can be set to "" or nullptr.

The "Advanced Textures" in fact are not textures at all. They are special objects with very special behavior.
For the considerations of common artist functionality (especially in 3ds max) we have to expose this functionality, incapsulated in separate object type called "Advanced Texture".
The reason for we still call them "textures" is that almost any artist GUI does this. And because we have to "put" them in material slots, like a common textures.
The first example is "falloff" - a blend of 2 textures depends on dot(viewVector,normal).
Next, a "dirt" which use ambient occlusion approximation but can further can go to any material slot.
The internal implementation of "falloff", "dirt" and other is very different from traditional approaches and can be changed in future.
For example, "falloff" can be transformed to BRDF Blend when constructing BRDF tree from HydraMaterial.
*/
HAPI HRTextureNodeRef  hrTextureCreateAdvanced(const wchar_t* a_texType, const wchar_t* a_objName);


/**
\brief Open node to change it's parameters. Usually this is needed for "Advanced Texture" HRTextureNode or any other complex node type can be opened and changed.
\param a_texRef   - advanced texture reference.
\param a_openMode - open mode.
*/
HAPI void              hrTextureNodeOpen(HRTextureNodeRef a_texRef, HR_OPEN_MODE a_openMode);


/**
\brief Close node.
\param a_texRef - advanced texture reference.
*/
HAPI void              hrTextureNodeClose(HRTextureNodeRef a_texRef);


/**
\brief  Bind texture to some xml node that is attached to a material o light. In fact just create proxy reference in xml.
\param  a_texRef    - texture reference.
\param  a_node      - node reference where proxy must be inserted.
\param  a_nameChild - node in xml where to bind. To rotate the anisotropy in material, use "texture_rot".
\return Resulting texture node.
*/
HAPI pugi::xml_node hrTextureBind(HRTextureNodeRef a_texRef, pugi::xml_node a_node, const wchar_t* a_nameChild = L"texture");


/**
\brief  Allow to change custom texture parameters.
\param  a_texRef - texture reference.
\return Cpp reference to xml node that user can directly work with.

If texture was not open, this function return reference nullptr xml_node.
All changes of that dummy will be discarded.
*/
HAPI pugi::xml_node    hrTextureParamNode(HRTextureNodeRef a_texRef);


/**
\brief Create HydraMaterial which is "compound" object, contain several BRDF and some geometric properties.
\param a_objectName - object name. Can be set to "" or nullptr.
 
 For a user HydraMaterial behaves like a traditional "uber material" with huge set of predefined parameters.
 It contain everything - emission, reflection, transparency, opacity, displacement and e.t.c.

 However, inside renderer, it will be translated to BRDF tree (by well specified algorithm) with actual complexity depends on what parameters you have set.
 For example if you set only diffuse properties, there will be no tree, only one leaf node.
*/
HAPI HRMaterialRef     hrMaterialCreate(const wchar_t* a_objectName);


/**
\brief Create HydraMaterialBlend (a node in brdf tree that blends two materials)
\param a_objectName - object name. Can be set to "" or nullptr.
\param a_matRef1    - first  child material.
\param a_matRef2    - second child material. 
*/
HAPI HRMaterialRef     hrMaterialCreateBlend(const wchar_t* a_objectName, HRMaterialRef a_matRef1, HRMaterialRef a_matRef2);


/**
\brief Open material for changing parameters and binding textures.
\param a_matRef - material reference.
\param a_mode   - open mode.
*/
HAPI void              hrMaterialOpen(HRMaterialRef a_matRef, HR_OPEN_MODE a_mode);


/**
\brief Close material object. Changes can not be made after object was closed, you have to reopen it.
\param a_matRef - material reference.
*/
HAPI void              hrMaterialClose(HRMaterialRef a_matRef);


/**
\brief Allow to change custom material parameters.
\param a_matRef - material reference.
\return Cpp reference to xml node that user can directly work with.

If material was not open, this function return reference to empty dummy node. All changes of that dummy will be discarded.
*/
HAPI pugi::xml_node    hrMaterialParamNode(HRMaterialRef a_matRef);


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
\brief Create light object.
\param a_objectName - light name. Can be "" or nullptr.
*/
HAPI HRLightRef        hrLightCreate(const wchar_t* a_objectName);


/**
\brief Open light object.
\param a_lightRef - light reference.
\param a_mode - open mode.
*/
HAPI void              hrLightOpen(HRLightRef a_lightRef, HR_OPEN_MODE a_mode);


/**
\brief Close light object.
\param a_lightRef - light reference.
*/
HAPI void              hrLightClose(HRLightRef a_lightRef);


/**
\brief Allow to change custom light parameters. 
\param a_lightRef - light reference.
\return Cpp reference to xml node that user can directly work with.

If light was not open, this function return reference to empty dummy node. All changes of that dummy will be discarded.
*/
HAPI pugi::xml_node    hrLightParamNode(HRLightRef a_lightRef);


/**
\brief Create camera object.
\param a_objectName - camera name. Can be "" or nullptr.
*/
HAPI HRCameraRef       hrCameraCreate(const wchar_t* a_objectName);


/**
\brief Open camera object.
\param a_camRef - camera reference.
\param a_mode   - open mode.
*/
HAPI void              hrCameraOpen(HRCameraRef a_camRef, HR_OPEN_MODE a_mode);


/**
\brief Close canera object.
\param a_camRef - camera reference.
*/
HAPI void              hrCameraClose(HRCameraRef a_camRef);


/**
\brief  Allow to change custom camera parameters.
\param  a_camRef - camera reference.
\return Cpp reference to xml node that user can directly work with.

If camera was not open, this function return reference nullptr xml_node. 
All changes of that dummy will be discarded.
*/
HAPI pugi::xml_node hrCameraParamNode(HRCameraRef a_camRef);


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/// Mesh index type.
enum HR_PRIM_TYPE { HR_TRIANGLE_LIST  = 0,    ///< simple triangle list.
                    HR_TRIANGLE_IND3  = 1,    ///< common layout, 3 indices of vertices per triangle.
                    HR_TRIANGLE_IND9  = 2,    ///< separate indices for pos, norm, texCoord (not supported in the current version).
                    HR_TRIANGLE_IND12 = 3     ///< separate indices for pos, norm, texCoord, tangent (not supported in the current version).
};  


/**
\brief Create mesh.
\param a_objectName - object name. Can be "" or nullptr.
*/
HAPI HRMeshRef hrMeshCreate(const wchar_t* a_objectName);


/**
\brief Create mesh from internal vsgf format with delayed load.
\param a_fileName - file name of the mesh.
\param a_copyToLocalFolder - indicates if we need to copy input '.vsgf' file to local folder.
*/
HAPI HRMeshRef hrMeshCreateFromFileDL(const wchar_t* a_fileName, bool a_copyToLocalFolder = false);


/**
\brief Create mesh from obj, obj+mtl or internal vsgf format.
\param a_fileName - file name of the mesh.
\param a_modelInfo - structure, describing how to import the model.
*/
HAPI HRMeshRef hrMeshCreateFromFile(const wchar_t* a_fileName, HRModelLoadInfo a_modelInfo = HRModelLoadInfo());


/**
\brief Open mesh.
\param a_meshRef - mesh reference.
\param a_type    - primitive type that will be used during current open/close session.
\param a_mode    - open mode.
*/
HAPI void              hrMeshOpen(HRMeshRef a_meshRef, HR_PRIM_TYPE a_type, HR_OPEN_MODE a_mode);


/**
\brief Close mesh.
\param a_meshRef       - mesh reference.
\param a_compress      - compress mesh when save it to file.
\param a_placeToOrigin - force place mesh bbox center at the (0,0,0); works ONLY if 'a_compress' is enabled!
*/
HAPI void              hrMeshClose(HRMeshRef a_meshRef, bool a_compress = false, bool a_placeToOrigin = false);


/**
\brief Set input vertex attribute pointer of type float1.
\param a_meshRef - mesh reference.
\param a_name    - attribute name.
\param a_pointer - input data pointer.
\param a_stride  - input stride in bytes; 0 means 4, like in OpenGL whan attributes are placed tightly, just an float array.
*/
HAPI void              hrMeshVertexAttribPointer1f(HRMeshRef a_meshRef, const wchar_t* a_name, const float* a_pointer, int a_stride = 0);


/**
\brief Set input attribute pointer of type float2.
\param a_meshRef - mesh reference.
\param a_name    - attribute name.
\param a_pointer - input data pointer.
\param a_stride  - input stride in bytes; 0 means 8, like in OpenGL whan attributes are placed tightly, just an float2 array.
*/
HAPI void              hrMeshVertexAttribPointer2f(HRMeshRef a_meshRef, const wchar_t* a_name, const float* a_pointer, int a_stride = 0);


/**
\brief Set input vertex attribute pointer of type float3.
\param a_meshRef - mesh reference.
\param a_name    - attribute name.
\param a_pointer - input data pointer.
\param a_stride  - input stride in bytes; 0 means 12, like in OpenGL whan attributes are placed tightly, just an float3 array.
*/
HAPI void              hrMeshVertexAttribPointer3f(HRMeshRef a_meshRef, const wchar_t* a_name, const float* a_pointer, int a_stride = 0);


/**
\brief Set input vertex attribute pointer of type float4.
\param a_meshRef - mesh reference.
\param a_name    - attribute name.
\param a_pointer - input data pointer.
\param a_stride  - input stride in bytes; 0 means 16, like in OpenGL whan attributes are placed tightly, just an float4 array.
*/
HAPI void              hrMeshVertexAttribPointer4f(HRMeshRef a_meshRef, const wchar_t* a_name, const float* a_pointer, int a_stride = 0);
         

/**
\brief Set input _primitive_ attribute pointer of type int1.
\param a_meshRef - mesh reference.
\param a_name    - attribute name.
\param a_pointer - input data pointer.
\param a_stride  - input stride in bytes; 0 means 4, like in OpenGL whan attributes are placed tightly, just an float array.
*/
HAPI void              hrMeshPrimitiveAttribPointer1i(HRMeshRef a_meshRef, const wchar_t* a_name, const int* a_pointer,   int a_stride = 0);


/**
\brief Assign material to all triangles of mesh.
\param a_meshRef - mesh reference.
\param a_matId   - material id.
*/
HAPI void              hrMeshMaterialId(HRMeshRef a_meshRef, int a_matId);


/**
\brief Like glDrawArrays.
\param a_meshRef     - mesh reference.
\param indNum        - input triangle indices num.
\param indices       - input triangle indices.
\param weld_vertices - flag for enable vertex cache i.e. collapse "same" vertices together.
*/
HAPI void              hrMeshAppendTriangles3(HRMeshRef a_meshRef, int indNum, const int* indices, bool weld_vertices = true);


/**
\brief Get vertex attribute pointer. The mesh must be opened! Else the function will return nullptr.
\param a_meshRef       - mesh reference.
\param a_attributeName - attribute name (currently checks only for "pos", "norm" and "uv").
*/
HAPI void*             hrMeshGetAttribPointer(HRMeshRef a_meshRef, const wchar_t* a_attributeName);


/**
\brief Get direct vertex attribute pointer to the data inside virtual buffer. The mesh must be closed! Else the function will return nullptr.
\param a_meshRef     - mesh reference.
\param a_attributeName - attribute name (currently checks only for "pos", "norm" and "uv").
*/
HAPI const void*       hrMeshGetAttribConstPointer(HRMeshRef a_meshRef, const wchar_t* a_attributeName);


/**
\brief Get primitive attribute pointer. The mesh must be opened! Else the function will return nullptr.
\param a_meshRef     - mesh reference.
\param a_attributeName - attribute name (currently checks only for "mind", only).
*/
HAPI void*             hrMeshGetPrimitiveAttribPointer(HRMeshRef a_meshRef, const wchar_t* a_attributeName);


/**
\brief Batch is a sequence of triangles with the same material id.
*/
struct HRBatchInfo
{
  int32_t matId;    ///< material id.
  int32_t triBegin; ///< begin of triangle sequence that have same material id "matId".
  int32_t triEnd;   ///< end of triangle sequence that have same material id "matId".
};


/**
\brief Information about opened mesh. You can't call this function if mesh is not open.
*/
struct HRMeshInfo
{
  HRMeshInfo() : vertNum(0), indicesNum(0),
                 batchesList(nullptr), batchesListSize(0),
                 matNamesList(nullptr), matNamesListSize(0){}

  int32_t vertNum;
  int32_t indicesNum;
  float   boxMin[3] ; ///< mesh bounding box min (x,y,z).
  float   boxMax[3] ; ///< mesh bounding box max (x,y,z).
  
  const HRBatchInfo* batchesList;
  int32_t            batchesListSize;
  
  const wchar_t** matNamesList;
  int32_t         matNamesListSize;
};


/**
\brief Get mesh info.
\param a_meshRef - mesh reference.
\return Information about mesh.
*/
HAPI HRMeshInfo hrMeshGetInfo(HRMeshRef a_mesh);


/**
\brief Get params node for mesh.
\param a_meshRef - mesh reference.
*/
HAPI pugi::xml_node    hrMeshParamNode(HRMeshRef a_meshRef);


/**
\brief Immediately save current mesh to '.vsgf' file to specified path.
\param a_meshRef  - mesh reference.
\param a_fileName - out file name; must end with '.vsgf'.

*/
HAPI void              hrMeshSaveVSGF(HRMeshRef a_meshRef, const wchar_t* a_fileName);


/**
\brief Iimmediately save current mesh to '.vsgfc' file (compressed vsgf).
\param a_meshRef  - mesh reference.
\param a_fileName - out file name; must end with '.vsgfc'.
*/
HAPI void              hrMeshSaveVSGFCompressed(HRMeshRef a_meshRef, const wchar_t* a_fileName);


HAPI void hrMeshComputeTangents(HRMeshRef a_mesh, int indexNum);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Final Scene 
//
HAPI HRSceneInstRef    hrSceneCreate(const wchar_t* a_objectName);
HAPI void              hrSceneOpen(HRSceneInstRef a_sceneRef, HR_OPEN_MODE a_mode);
HAPI void              hrSceneClose(HRSceneInstRef a_sceneRef);

/**
\brief Get params node for scene.
\param a_sceneRef - scene reference.
*/
HAPI pugi::xml_node  hrSceneParamNode(HRSceneInstRef a_sceneRef);


/**
\brief Like glDrawArraysInstanced.
\param a_sceneRef   - scene reference where mesh instances will be inserted.
\param a_meshRef    - mesh reference.
\param a_mat        - matrix.
\param a_mmList     - multimaterial remap pairs array. ALWAYS must be multiple of 2.
\param a_mmListSize - size of multimaterial remap pairs array. EXAMPLE: [0,1,3,4,7,5] means [0-->1; 3-->4; 7-->5;].
*/
HAPI int              hrMeshInstance(HRSceneInstRef a_sceneRef, HRMeshRef a_meshRef, float a_mat[16], 
                                      const int32_t* a_mmListm = nullptr, int32_t a_mmListSize = 0);


/**
\brief Like glDrawArraysInstanced, but for lights.
\param a_sceneRef      - scene reference where mesh instances will be inserted.
\param a_lightRef      - light reference.
\param a_mat           - matrix.
\param a_customAttribs - wstring that contains custom attrib list. For example: L"color_mult ="\1 0 0"\ rotationMatrix = "\1 0 0 0 1 0 0 0 1"\".
*/
HAPI void              hrLightInstance(HRSceneInstRef a_sceneRef, HRLightRef  a_lightRef, float a_mat[16], const wchar_t* a_customAttribs = nullptr);


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HAPI HRRenderRef       hrRenderCreate(const wchar_t* a_className, const wchar_t* a_flags = L"");
HAPI void              hrRenderOpen(HRRenderRef a_renderRef, HR_OPEN_MODE a_mode);
HAPI void              hrRenderClose(HRRenderRef a_renderRef);
HAPI pugi::xml_node    hrRenderParamNode(HRRenderRef a_camRef);


/**
\brief OpenCL and other (pure C/C++) devices that can be used for rendering.
*/
struct HRRenderDeviceInfoListElem
{
  wchar_t name[256];
  wchar_t driver[256];
  int32_t id;
  bool    isCPU;
  bool    isEnabled;
  const HRRenderDeviceInfoListElem* next;
};


/**
\brief Return a pointer to the head of device list.
*/
HAPI const HRRenderDeviceInfoListElem* hrRenderGetDeviceList(HRRenderRef a_renderRef);


/**
\brief Enable or disable target device by id. 
*/
HAPI void hrRenderEnableDevice(HRRenderRef a_renderRef, int32_t a_deviceId, bool a_enable);


/**
\brief These struct is returned by hrRenderHaveUpdate.
*/
struct HRRenderUpdateInfo
{
  HRRenderUpdateInfo() : haveUpdateFB(false), haveUpdateMSG(false), finalUpdate(false), progress(0.0f), msg(L"") {}

  bool           haveUpdateFB;
  bool           haveUpdateMSG;
  bool           finalUpdate;
  float          progress;
  const wchar_t* msg;
};


/**
\brief Indicate that render do have Update for FrameBuffer.
*/
HAPI HRRenderUpdateInfo  hrRenderHaveUpdate(HRRenderRef a_renderRef);


/**
\brief Get framebuffer content to imgData.
*/
HAPI bool hrRenderGetFrameBufferHDR4f(HRRenderRef a_renderRef, int w, int h, float* imgData, const wchar_t* a_layerName = L"color");  // w*h*4*sizeof(float)


/**
\brief Get framebuffer content to imgData, only color.
*/
HAPI bool hrRenderGetFrameBufferLDR1i(HRRenderRef a_renderRef, int w, int h, int32_t* imgData);  // w*h*sizeof(int) --> RGBA


/**
\brief You must use this function _before_ call hrRenderGetFrameBufferLineHDR4f/hrRenderGetFrameBufferLineLDR1i.
       If use hrRenderGetFrameBufferHDR4f/hrRenderGetFrameBufferLDR1i, you don't have to call this.
*/
HAPI bool hrRenderLockFrameBufferUpdate(HRRenderRef a_renderRef);


/**
\brief You must use this function _after_ call hrRenderGetFrameBufferLineHDR4f/hrRenderGetFrameBufferLineLDR1i.
       If use hrRenderGetFrameBufferHDR4f/hrRenderGetFrameBufferLDR1i, you don't have to call this.
*/
HAPI void hrRenderUnlockFrameBufferUpdate(HRRenderRef a_renderRef);


/**
\brief Get framebuffer line to imgData. 
*/
HAPI bool hrRenderGetFrameBufferLineHDR4f(HRRenderRef a_renderRef, int a_begin, int a_end, int a_y, float* imgData, const wchar_t* a_layerName = L"color");  // w*h*4*sizeof(float)


/**
\brief Get framebuffer line to imgData, only color.
*/
HAPI bool hrRenderGetFrameBufferLineLDR1i(HRRenderRef a_renderRef, int a_begin, int a_end, int a_y, int32_t* imgData);  // w*h*sizeof(int) --> RGBA


/**
\brief Save framebuffer content to imgData (convert it to LDR). Color only.
*/
HAPI bool hrRenderSaveFrameBufferLDR(HRRenderRef a_renderRef, const wchar_t* a_outFileName);  // w*h*4*sizeof(float)


/**
\brief Save framebuffer content to imgData. Color only.
*/
HAPI bool hrRenderSaveFrameBufferHDR(HRRenderRef a_renderRef, const wchar_t* a_outFileName);  // w*h*4*sizeof(float)


struct HRGBufferPixel
{
  float   depth;
  float   norm[3];
  float   texc[2];
  float   rgba[4];
  float   shadow;
  float   coverage;
  int32_t matId;
  int32_t objId;
  int32_t instId;
};


/**
\brief Unpack gbuffer data for a custom line.
\param a_renderRef  - render reference.
\param a_lineNumber - line number, i.e. y coordinate.
\param a_lineData   - output line of size at least (a_endX - a_startX).
\param a_startX     - x coordinate, start of the line.
\param a_endX       - x coordinate, end of the line.
*/
HAPI void hrRenderGetGBufferLine(HRRenderRef a_renderRef, int32_t a_lineNumber, HRGBufferPixel* a_lineData, int32_t a_startX, int32_t a_endX);  // w*4*sizeof(float)


/**
\brief Save custom gbuffer layer.
\param a_renderRef   - render reference.
\param a_outFileName - output file name.
\param a_layerName   - layer name we are going to save [depth,normals,texcoord,diffcolor,alpha,shadow,matid,objid,instid].
\param a_palette     - color palette; applied to indices only!
\param a_paletteSize - color palette size.

If palette is not set for "id" layers, this function will use it's default palette. 
if GBuffer was not evaluated by render driver (you have to set 'evalgbuffer' = 1 in render settings), this function do nothing.
*/
HAPI bool hrRenderSaveGBufferLayerLDR(HRRenderRef a_renderRef, const wchar_t* a_outFileName, const wchar_t* a_layerName,
                                      const int* a_palette = nullptr, int a_paletteSize = 0);

/**
\brief Execute custom command for render driver.
\param a_renderRef - render reference.
\param a_command   - command string.
\param a_answer    - optional string answer. Max 256 wchars.

In the case of "HydraModern" render driver, the command will be sended to all render processes. 
All these commands (except 'exitnow') are used in default 'offline' render mode. The 'interactive' render mode is controlled by hrCommit() only.
command list:

start    [-statefile statex_00009.xml] 
                                -- signal to start rendering for hydra processes that are waiting. "-statefile" argument is optional.
                                   Automaticly sends after hrFlush() is performed (or hrCommit() + shared VB is enabled).

continue [-statefile statex_00009.xml] 
                                -- launch hydra processes first, then and send them "start".  "-statefile" argument is optional.

runhydra -cl_device_id 0 [-statefile statex_00009.xml] [-contribsamples 256] [-maxsamples 512]   

                                -- launch single hydra process and pass everything via command line. I.e. running process will not react to any commands (except "exitnow") in this mode. 
                                   This is like running hydra process in the 'box mode'. You can only stop it.    
        
         args: -statefile       -- specify hydra process to render target state.
         args: -contribsamples  -- specify hydra process to count exact contribution to shared image this process should do and then exit when reach contribsamples value.
         args: -maxsamples      -- specify hydra process to evaluate no more then maxsamples values sample per pixel. Note that this value should be greater then contribsamples.
         args: -cl_device_id    -- specify device id for hydra process.

exitnow [-cl_device_id 0]       -- all hydra processes should immediately exit. Can be used also in 'interactive mode' and 'box mode'.
                                   #NOT_IMPLEMENTED: optionally you can stop only single process by specifying target device id.                                 

pause  z_image.bin              -- save accumulated shared image to "z_image.bin", then send "exitnow". No spaces, no quotes allowed, single string file name.
                               
resume z_image.bin              -- load accumulated shared image from "z_image.bin", then send "start" to waiting processes. 
                                   Note that this command does not launch hydra processes (!!!), you have to manually call "hrCommit(scnRef, renderRef)".
                                   This is due to pause may occur within main program exit. When main program will be launched again it must open scene.
                                   We want to continue render and Commit the new state via hrCommit(scnRef, renderRef). Thus it launch processes.
*/
HAPI void hrRenderCommand(HRRenderRef a_renderRef, const wchar_t* a_command, wchar_t* a_answer = nullptr);


/**
\brief Set log directory for renderer. If a_logDir == L"" or nullptr, logging is disabled (default).
* \param a_renderRef   - render reference.
* \param a_logDir      - directory where renderer save logs.
* \param a_hideConsole - if you want to hide console.
*/
HAPI void hrRenderLogDir(HRRenderRef a_renderRef, const wchar_t* a_logDir, bool a_hideConsole = false);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
\brief Non blocking commit, send commands to renderer and return immediately.

For the "HydraModern" render driver this command will launch new process or transfer changes to existing (if interactive mode is implemented and enabled).
*/
HAPI void hrCommit(HRSceneInstRef a_sceneRef = HRSceneInstRef(), 
                   HRRenderRef a_renderRef = HRRenderRef(),
                   HRCameraRef a_camRef    = HRCameraRef()); // non blocking commit, send commands to renderer and return immediately 

/**
\brief Blocking commit, waiting for all current commands to be executed.
*/
HAPI void hrFlush(HRSceneInstRef a_sceneRef = HRSceneInstRef(), 
                  HRRenderRef a_renderRef = HRRenderRef(),
                  HRCameraRef a_camRef    = HRCameraRef());

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef int32_t HR_IDType;


/**
\brief An extension helper for groups of lights.
*/
struct HRLightGroupExt
{
  typedef float float16[16];

  HRLightGroupExt() : lights(nullptr), matrix(nullptr), customAttr(nullptr), lightsNum(0) { }
  explicit HRLightGroupExt(int a_lightNum);
  ~HRLightGroupExt();

  HRLightGroupExt(HRLightGroupExt&& a_in);
  HRLightGroupExt(const HRLightGroupExt& a_in);
  HRLightGroupExt& operator=(HRLightGroupExt&& a_in);
  HRLightGroupExt& operator=(const HRLightGroupExt& a_in);

  HRLightRef* lights;
  float16*    matrix;
  wchar_t**   customAttr;
  int         lightsNum;
};


/**
\brief Instance group of lights.
*/
HAPI void  hrLightGroupInstanceExt(HRSceneInstRef a_sceneRef, HRLightGroupExt a_pLight, float a_mat[16], const wchar_t** a_customAttribsArray = nullptr);

#ifdef WIN32
#undef min
#undef max
#endif

namespace HRUtils
{
  struct MergeInfo
  {
    int32_t meshRange    [2]; ///<! stores [first, last)
    int32_t texturesRange[2];
    int32_t materialRange[2];
    int32_t lightsRange  [2];
  };

  HRSceneInstRef MergeLibraryIntoLibrary(const wchar_t* a_libPath, bool mergeLights = false, bool copyScene = false,
                                         const wchar_t* a_stateFileName = L"", MergeInfo* pInfo = nullptr);

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  HRMaterialRef    MergeOneMaterialIntoLibrary(const wchar_t* a_libPath, const wchar_t* a_matName, int a_matId = -1);
  HRMeshRef        MergeOneMeshIntoLibrary    (const wchar_t* a_libPath, const wchar_t* a_meshName);
  HRLightRef       MergeOneLightIntoLibrary   (const wchar_t* a_libPath, const wchar_t* a_lightName);
  HRTextureNodeRef MergeOneTextureIntoLibrary (const wchar_t* a_libPath, const wchar_t* a_texName, int a_texId = -1);

  bool             hrRenderSaveDepthRaw(HRRenderRef a_renderRef, const wchar_t* a_outFileName);

  // Parses the .obj file, consisting of 1+ shapes
  MergeInfo LoadMultipleShapesFromObj(const wchar_t* a_fileName, bool a_copyToLocalFolder = false);


  /**
  \brief
  */
  struct BBox
  {
      float x_min;
      float x_max;

      float y_min;
      float y_max;

      float z_min;
      float z_max;

      BBox(): x_min(std::numeric_limits<float>::max()), x_max(std::numeric_limits<float>::lowest()),
              y_min(std::numeric_limits<float>::max()), y_max(std::numeric_limits<float>::lowest()),
              z_min(std::numeric_limits<float>::max()), z_max(std::numeric_limits<float>::lowest()) {}
  };

  BBox GetMeshBBox(HRMeshRef);

  BBox transformBBox(const BBox &a_bbox, const float a_mat[16]);

  void getRandomPointsOnMesh(HRMeshRef mesh_ref, float *points, uint32_t n_points, bool tri_area_weighted, uint32_t seed = 0u);

  HRTextureNodeRef Cube2SphereLDR(HRTextureNodeRef a_cube[6]);

  BBox InstanceSceneIntoScene(HRSceneInstRef a_scnFrom, HRSceneInstRef a_scnTo, float a_mat[16], bool origin = true,
                              const int32_t* remapListOverride = nullptr, int32_t remapListSize = 0);

};


namespace HRExtensions
{
    typedef void(*HR_TEXTURE_DISPLACEMENT_CALLBACK)(const float *pos, const float *normal, const HRUtils::BBox &bbox,
                                                    float displace_vec[3],
                                                    void* a_customData, uint32_t a_customDataSize);

    HAPI HRTextureNodeRef hrTextureDisplacementCustom(HR_TEXTURE_DISPLACEMENT_CALLBACK a_proc, void* a_customData,
                                                      uint32_t a_customDataSize);
}


/**
\brief Get HRMaterialRef by name from the library.
\param a_matName  - material name.

 If material with name = a_matName does not exist, HRMaterialRef with id = -1 is returned.
*/
HAPI HRMaterialRef hrFindMaterialByName(const wchar_t *a_matName);


/**
\brief Get HRLightRef by name from the library.
\param a_lightName  - light name.

 If light with name = a_lightName does not exist, HRLightRef with id = -1 is returned.
*/
HAPI HRLightRef hrFindLightByName(const wchar_t *a_lightName);


/**
\brief Get HRCameraRef by name from the library.
\param a_cameraName  - camera name.

 If camera with name = a_cameraName does not exist, HRCameraRef with id = -1 is returned.
*/
HAPI HRCameraRef hrFindCameraByName(const wchar_t *a_cameraName);


/**
\brief get HRRenderRef by its type name from the library.
\param a_renderTypeName - render type name.

If camera with name = a_cameraName does not exist, HRCameraRef with id = -1 is returned.
*/
HAPI HRRenderRef hrFindRenderByTypeName(const wchar_t *a_renderTypeName);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////// UTILITY FUNCTIONS ///////////////////////////////////////////////////////


namespace hr_prng
{
  struct uint2
  {
    unsigned int x;
    unsigned int y;
  };

  typedef struct RandomGenT
  {
    uint2 state;

  } RandomGen;

  RandomGen RandomGenInit(const int a_seed);

  /**
   \brief Get next pseudo random float value in range [0,1].
   \param gen - pointer to current generator state.
   */
  float rndFloat(RandomGen *gen);


  /**
   \brief Get next pseudo random float value in range [s,e].
   \param gen - reference to current generator state.
   \param s   - low  boundary of generated random number.
   \param e   - high boundary of generated random number.
   */
  float rndFloatUniform(RandomGen& gen, float s, float e);


  /**
   \brief Get next pseudo random integer value in range [s,e].
   \param gen - reference to current generator state.
   \param s   - low  boundary of generated random number.
   \param e   - high boundary of generated random number.
   */
  int rndIntUniform(RandomGen& gen, int a, int b);
};


namespace hr_qmc
{
  constexpr static int   QRNG_DIMENSIONS = 11;
  constexpr static int   QRNG_RESOLUTION = 31;
  constexpr static float INT_SCALE       = (1.0f / (float)0x80000001U);

  void init(unsigned int table[hr_qmc::QRNG_DIMENSIONS][hr_qmc::QRNG_RESOLUTION]);


  /**
  \brief Get arbitrary 'Sobol/Niederreiter' quasi random float value in range [0,1].
  \param pos     - id of value in sequence (i.e. first, second, ... ).
  \param dim     - dimention/coordinate id (i.e. x,y,z,w, ... ).
  \param c_Table - some table previously initialised with hr_qmc::init function.
  */
  float rndFloat(unsigned int pos, int dim, unsigned int *c_Table); 


  /**
  \brief Get arbitrary 'Sobol/Niederreiter' quasi random float value in range [s,e].
  \param gen - pointer to current generator state.
  \param s   - low  boundary of generated random number.
  \param e   - high boundary of generated random number.
  */
  float rndFloatUniform(unsigned int pos, int dim, unsigned int *c_Table, float s, float e);


  /**
  \brief Get arbitrary 'Sobol/Niederreiter' quasi random integer value in range [s,e].
  \param gen - pointer to current generator state.
  \param s   - low  boundary of generated random number.
  \param e   - high boundary of generated random number.
  */
  int rndIntUniform(unsigned int pos, int dim, unsigned int *c_Table, int a, int b);
};


namespace hr_fs
{
  std::wstring s2ws(const std::string& str);
  std::string  ws2s(const std::wstring& wstr);
  
  int mkdir(const char* a_folder);
  int mkdir(const wchar_t* a_folder);
  
  int cleardir(const char* a_folder);
  int cleardir(const wchar_t* a_folder);
  
  void deletefile(const char* a_file);
  void deletefile(const wchar_t* a_file);
  
  void copyfile(const char* a_file1,    const char* a_file2);
  void copyfile(const wchar_t* a_file1, const wchar_t* a_file2);
  
  std::vector<std::string>  listfiles(const char* a_folder,    bool excludeFolders = true);
  std::vector<std::wstring> listfiles(const wchar_t* a_folder, bool excludeFolders = true);
  
};


/**
  \brief These parameters are not related to hr_qmc namespace, they are for 'HydraModern' presets.
 */
#define HYDRA_QMC_DOF_FLAG 1
#define HYDRA_QMC_MTL_FLAG 2
#define HYDRA_QMC_LGT_FLAG 4
