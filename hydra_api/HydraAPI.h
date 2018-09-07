#pragma once

#include <cstdint>
#include <limits>
#include "pugixml.hpp"

/**
 \file
 \brief Hydra Renderer API

 A short overview of Hydra API ideology:

 (1) To change any object you need first to open it,.
 
 (2) After you finish changes, you need to close object. It's like working with files in operating system.
 
 (3) All structure data are incapsulated. You don't know their internal format. 

 (4) The API works with tiny set of abstract types. They are: (HRMesh, HRLight, HRMaterial, HRCamera, HRTextureNode, HRSceneData, HRSceneInst, HRRender)
 Objects of types HRMaterial and HRTextureNode hovewer, can have subtypes - a different actual implementation of Material and TextureNode.
 
 It is forced necessity to have subtypes for them because there could be extrimely large amount of materials, textures, shaders (don't have them for current) and e.t.c. atrist tools.
 
 So, unfortunately, in several cases you do have to know which subtype is used. Usually - when you create such object and/or change it's parameters.
 
 Subtypes have their own names without HR prefix. For example HydraMaterial and BlendMaterial are a subtypes of HRMaterial.

 Here is the full list of subtypes: 

 HRMaterial -> (HydraMaterial, BlendMaterial, BRDFLeafLambert, BRDFLeafPhong, ... {any other BRDFLeaf**})

 HRTextureNode -> (Array1D, Texture2DHDR, Texture2DLDR, Texture2DProcHDR, Texture2DProcLDR, TextureAdvanced)

 The function of API usually have subtype name as a part of their own name. For example hrTextureCreateAdvanced create HRTextureNode of subtype TextureAdvanced.

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

  #define HAPI                       ///< mark all functions as 'extern "C"'; This is needed if you want to load DLL in dynamic;

#endif

/**
\brief Information about total current numbers of all objects inside API in it's current state 

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

struct HRMeshRef     { int32_t id; HRMeshRef()     : id(-1) {} }; ///< Mesh  reference
struct HRLightRef    { int32_t id; HRLightRef()    : id(-1) {} }; ///< Light reference
struct HRMaterialRef { int32_t id; HRMaterialRef() : id(-1) {} }; ///< Material reference
struct HRCameraRef   { int32_t id; HRCameraRef()   : id(-1) {} }; ///< Camera reference

/**
\brief Node reference. Nodes are render textures, pretty much like any other nodes used in 3D modeling software.

A common 3D modeling software, like 3ds Max, Blender and espetially Houdiny uses concept called Nodes.

Nodes are fixed function or programmible units, like a functions - take some from input and put some other to output.
Nodes usually can be bind to some material slots (or light slots). Users will build a trees from nodes to do some complex stuff.

The simplest node is a Bitmap, created with hrTexture2D. Bitmaps are simple 2D images. Just a 2D array of values.
Their input is a texture coordinates, output - texture color. Texture can be bound to material slot.

The more complex example is 'Faloff' node - a blend for 2 textures based of angle between view vector and surface normal.
Faloff is a fixed function node with predefined parameters.

Any parameters (for fixed function or programmible nodes) can be set via sequence of hrTextureNodeOpen(node), ... , hrTextureNodeClose(node);

*/

struct HRTextureNodeRef { int32_t id; HRTextureNodeRef () : id(-1) {} }; ///< TextureNode reference
struct HRSceneInstRef   { int32_t id; HRSceneInstRef()    : id(-1) {} }; ///< SceneInst reference
struct HRRenderRef      { int32_t id; HRRenderRef()       : id(-1) {} }; ///< RenderSettings reference

/// When open any HRObject you must specify one of these 2 flags.

enum HR_OPEN_MODE {
  HR_OPEN_EXISTING  = 0,  ///< This flag should be used when open object if you want to change the existing object with non empty data
  HR_WRITE_DISCARD  = 1,  ///< This flag should be used when open object if you want to fill object properties first time. Or clear object and fill it from "clear page".
  HR_OPEN_READ_ONLY = 2,  ///< This flag should be used when open object if you want to read current object state. When close object, your changes will not be applyed.
};


/// Severity levels of information that main application gets from API

enum HR_SEVERITY_LEVEL {
  HR_SEVERITY_DEBUG          = 0,  ///< The info which your app should display only in debug mode; else - jjust ignore it;
  HR_SEVERITY_INFO           = 1,  ///< Normal info that you app would be great to display
  HR_SEVERITY_WARNING        = 2,  ///< Some-thing went wrong, but the work with API can be continued. It is recommended to display such infromation. 
  HR_SEVERITY_ERROR          = 3,  ///< Some-thing went damn wrong. The work with API is still can be continued, but the result is undefined. 
  HR_SEVERITY_CRITICAL_ERROR = 4,  ///< Absolutely bad thing had happened. Please display error message and immediately call hrDestroy();
};


/**
\brief Deprecated (!!!) callback for printing error messages. 
\param message     - actual error message that will be passed to callback when error happened
\param callerPlace - the last string rememberd by hrErrorCallerPlace before error have rised
Called each time when any error rises.
*/
typedef void(*HR_ERROR_CALLBACK)(const wchar_t* message, const wchar_t* callerPlace);

/**
\brief General callback for printing info and error messages.
\param message     - actual error message that will be passed to callback when error happened
\param callerPlace - the last string rememberd by hrErrorCallerPlace before error have rised
\param a_level     - severity leve of message
Called each time when any error rises.
*/
typedef void(*HR_INFO_CALLBACK)(const wchar_t* message, const wchar_t* callerPlace, HR_SEVERITY_LEVEL a_level);

/**
\brief 2D procedural texture rendering callback for HDR images
\param a_buffer a float buffer of size w*h*4. Write your pixels here.
\param w - desired width of texture. Always power of 2.
\param h - desired height of texture. Always power of 2.
\param a_customData - a pointer to your custom data structure (needed for rendering texture to bitmap) that callback knows.
 
 Because render API is abstracted from concrete HW (and basically, targeted for a GPU), the tradition approach to procedural textures, generated on the fly is nearly impossible.
 However, if procedural texture depends only on texture coords (i.e. classical, 2D texture), render can precompute bitmap with the desired resolution.
 The resolution is automatically determined by the renderer and the way that happens is not specified.
 Due to that your call back must be able to render bitmap from procedural representation in any power of 2 resolutions.

*/
typedef void(*HR_TEXTURE2D_PROC_HDR_CALLBACK)(float* a_buffer, int w, int h, void* a_customData);

/**
\brief 2D procedural texture rendering callback for LDR images
\param a_buffer - unsigned char buffer of size w*h*4. Write your pixels here.
\param w - desired width of texture. Always power of 2.
\param h - desired height of texture. Always power of 2.
\param a_customData - a pointer to your custom data structure (needed for rendering texture to bitmap) that callback knows.

 Because render API is abstracted from concrete HW (and basically, targeted for a GPU), the tradition approach to procedural textures, generated on the fly is nearly impossible.
 However, if procedural texture depends only on texture coords (i.e. classical, 2D texture), render can precompute bitmap with the desired resolution.
 The resolution is automatically determined by the renderer and the way that happens is not specified.
 Due to that your call back must be able to render bitmap from procedural representation in any power of 2 resolutions.

*/
typedef void(*HR_TEXTURE2D_PROC_LDR_CALLBACK)(unsigned char* a_buffer, int w, int h, void* a_customData);



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
  \brief initialize render API
  \param a_className - calss name of actual API implementation. Can be "" or nullptr.
  In this case API will select implementation automaticly.

*/

HAPI void hrInit(const wchar_t* a_className);

/**
  \brief destroy everything 

*/

HAPI void hrDestroy();

/**
\brief return last error message. If no error, return nullptr

*/

HAPI const wchar_t* hrGetLastError();

/**
\brief remember inside API input string to further prepend it to error message if error happened
\param a_placeName - input string
\param a_line      - input line number. if 0, just ignored.

This function needed to mark a place in use program, where last error is happened. For example, it can be function name and line number.
Usually, you call this function inside some block to find out where error had happened.
For debug purposes you can call hrErrorCallerPlace(L"myFunction", __LINE__); several times inside one function or code block.

Note that this function only sets internal string state. If you set in in one function hrErrorCallerPlace(L"MyFirstFunction"), but error happend in the other function 
(than don't have hrErrorCallerPlace(L"MyOtherFunction"), you will get an old place name in error message - MyFirstFunction; )

It also don't have any stack or e.t.c., just a state, just one string! So please, track you call tree by yourself!

*/
HAPI void hrErrorCallerPlace(const wchar_t* a_placeName, int a_line = 0);

/**
\brief set your custome printing error callback. @DEPRECATED !!! USE hrInfoCallback instead!!!
*/
HAPI void hrErrorCallback(HR_ERROR_CALLBACK pCallback);

/**
\brief set your custome printing error callback
*/
HAPI void hrInfoCallback(HR_INFO_CALLBACK pCallback);

/**
\brief this structure represents quick info about scene library

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
\brief quick check if scene library exists in the file system at target path
\param a_libPath       - a path to some folder that we want to check 
\param a_quickResponce - immediate responce that some thing is wrong with the scene library. For example bad references or sms like that. must be at least 256 wchars or nullptr
\return info about scene library at path a_libPath

*/
HAPI HRSceneLibraryFileInfo hrSceneLibraryExists(const wchar_t* a_libPath, wchar_t a_quickResponce[256]);

/**
\brief set current scene data path (scene library).  
\param a_libPath  - a path to the scene library - some folder that will hold all exported objects.
\param a_openMode - create new scene or open existing?

 Destroy All SceneData/SceneLibrary if (a_openMode == HR_WRITE_DISCARD).
 Passing nullptr or L"" to a_libPath will cause just to clear everything.

*/
HAPI int32_t hrSceneLibraryOpen(const wchar_t* a_libPath, HR_OPEN_MODE a_openMode);


/**
\brief get information about current  "scene library" / "API state" 

*/
HAPI HRSceneLibraryInfo hrSceneLibraryInfo();


/**
\brief create 2D texture from file
\param pScnData   - scene data object ptr.
\param a_fileName - file name
\param w   - texture width; optional parameter, may be used by renderer as a hint for effitiency consideretions. In case if it is not set, render read it from file.
\param h   - texture height; optional parameter, may be used by renderer as a hint for effitiency consideretions. In case if it is not set, render read it from file.
\param bpp - bytes per pixel; optional parameter, may be used by renderer as a hint for effitiency consideretions. In case if it is not set, render read it from file.

 passing w,h or bpp not equal to -1 ask render

*/

HAPI HRTextureNodeRef  hrTexture2DCreateFromFile(const wchar_t* a_fileName, int w = -1, int h = -1, int bpp = -1);

/**
\brief Create 2D texture from file with Delayed Load (DL means "Delayed Load").
\param pScnData   - scene data object ptr.
\param a_fileName - file name
\param w   - texture width; optional parameter, may be used by renderer as a hint for effitiency consideretions. In case if it is not set, render read it from file.
\param h   - texture height; optional parameter, may be used by renderer as a hint for effitiency consideretions. In case if it is not set, render read it from file.
\param bpp - bytes per pixel; optional parameter, may be used by renderer as a hint for effitiency consideretions. In case if it is not set, render read it from file.

 The "Delayed Load" means that texture will be load to memory only when passing it to render driver (or by render driver itself, if it can load images from external format).
 The advantage of using textures with delayed load is that if some texture is not really needed for rendering current frame it won't be touched at all.

 The must-to-use case is when you know you are going to do network rendering on several workstations.
 Because of well compression rate in traditional image files it is 100% desireable to pass initial compresed data to render driver to it can deliver textures through the network in their initial well-compressed format. 

 The normal case also - if you do know that your texture don't have full size copy in memory.
 Nevertheless, it is absolutely ok to use this function if you want to save memory or prevent unnecessary disk flush of internal HydraAPI cache in other cases.

*/

HAPI HRTextureNodeRef  hrTexture2DCreateFromFileDL(const wchar_t* a_fileName, int w = -1, int h = -1, int bpp = -1);

/**
\brief Update 2D texture from file
\param currentRef - old texture reference.
\param a_fileName - new file name
\param w    - new texture width;
\param h    - new texture height;
\param bpp  - new bytes per pixel;

*/

HAPI HRTextureNodeRef hrTexture2DUpdateFromFile(HRTextureNodeRef currentRef, const wchar_t* a_fileName, int w = -1, int h = -1, int bpp = -1);

/**
\brief create 2D texture from memory
\param pScnData - scene data object ptr.
\param data - pointer to image data
\param w    - texture width; 
\param h    - texture height; 
\param bpp  - bytes per pixel; 

*/

HAPI HRTextureNodeRef  hrTexture2DCreateFromMemory(int w, int h, int bpp, const void* data);


/**
\brief Update 2D texture from memory
\param currentRef - old texture reference.
\param pScnData - scene data object ptr.
\param data - pointer to new image data
\param w    - new texture width;
\param h    - new texture height;
\param bpp  - new bytes per pixel;

*/
HAPI HRTextureNodeRef hrTexture2DUpdateFromMemory(HRTextureNodeRef currentRef, int w, int h, int bpp, const void* a_data);

/**
\brief create 1D float array
\param pScnData - scene data object ptr.
\param data     - pointer to float data
\param a_size   - array size

*/
HAPI HRTextureNodeRef  hrArray1DCreateFromMemory(const float* data, int a_size);

/**
\brief create procedural 2D texture with callback.
\param pScnData     - scene data object ptr.
\param a_proc       - a callback that must be abble to render procedural texture in any power of 2 resolution
\param a_customData - a pointer to custom data that will be passed to callback.
\param w - texture width; optional. should be set as a hint for renderer if you know desired texture resolution.
\param h - texture height; optional. should be set as a hint for renderer if you know desired texture resolution.

 See description of HR_TEXTURE2D_PROC_HDR_CALLBACK for more details.

*/
HAPI HRTextureNodeRef  hrTexture2DCreateFromProcHDR(HR_TEXTURE2D_PROC_HDR_CALLBACK a_proc, void* a_customData,
                                                    int customDataSize, int w = -1, int h = -1);

/**
\brief create procedural 2D texture with callback.
\param pScnData     - scene data object ptr.
\param a_proc       - a callback that must be abble to render procedural texture in any power of 2 resolution
\param a_customData - a pointer to custom data that will be passed to callback.
\param w - texture width; optional. should be set as a hint for renderer if you know desired texture resolution.
\param h - texture height; optional. should be set as a hint for renderer if you know desired texture resolution.

See description of HR_TEXTURE2D_PROC_LDR_CALLBACK for more details.

*/
HAPI HRTextureNodeRef  hrTexture2DCreateFromProcLDR(HR_TEXTURE2D_PROC_LDR_CALLBACK a_proc, void* a_customData,
                                                    int customDataSize, int w = -1, int h = -1);

/**
\brief Update 2D texture from callback
\param currentRef - old texture reference.
\param a_proc - a callback that must be abble to render procedural texture in any power of 2 resolution.
\param a_customData - a pointer to custom data that will be passed to callback.
\param w - texture width; optional. should be set as a hint for renderer if you know desired texture resolution.
\param h - texture height; optional. should be set as a hint for renderer if you know desired texture resolution.

*/
HAPI HRTextureNodeRef  hrTexture2DUpdateFromProcHDR(HRTextureNodeRef currentRef, HR_TEXTURE2D_PROC_HDR_CALLBACK a_proc,
                                                    void* a_customData, int customDataSize, int w, int h);

/**
\brief Update 2D texture from callback
\param currentRef - old texture reference.
\param a_proc - a callback that must be abble to render procedural texture in any power of 2 resolution.
\param a_customData - a pointer to custom data that will be passed to callback.
\param w - texture width; optional. should be set as a hint for renderer if you know desired texture resolution.
\param h - texture height; optional. should be set as a hint for renderer if you know desired texture resolution.

*/
HAPI HRTextureNodeRef  hrTexture2DUpdateFromProcLDR(HRTextureNodeRef currentRef, HR_TEXTURE2D_PROC_LDR_CALLBACK a_proc,
                                                    void* a_customData, int customDataSize, int w, int h);


/**
\brief get raw data for custom texture object.
\param a_tex - input  texture reference
\param pW    - output texture width. It will be written with 0 if hrTexture2DGetDataLDR operation is invalid.
\param pH    - output texture height. It will be written with 0 if hrTexture2DGetDataLDR operation is invalid.
\param pBPP  - output bytes per pixel (4 for LDR and 16 for HDR).

*/
HAPI void hrTexture2DGetSize(HRTextureNodeRef a_tex, int* pW, int* pH, int* pBPP);

/**
\brief get raw data for custom texture object.
\param a_tex - input  texture reference
\param pW    - output texture width. It will be written with 0 if hrTexture2DGetDataLDR operation is invalid. 
\param pH    - output texture height. It will be written with 0 if hrTexture2DGetDataLDR operation is invalid.
\param pData - output data pointer that points to array of size (width*height). 

*/
HAPI void hrTexture2DGetDataLDR(HRTextureNodeRef a_tex, int* pW, int* pH, int* pData);

/**
\brief get raw data for custom texture object.
\param a_tex - input  texture reference
\param pW    - output texture width. It will be written with 0 if hrTexture2DGetDataHDR operation is invalid.
\param pH    - output texture height. It will be written with 0 if hrTexture2DGetDataHDR operation is invalid.
\param pData - output data pointer that points to array of size (width*height).

*/
HAPI void hrTexture2DGetDataHDR(HRTextureNodeRef a_tex, int* pW, int* pH, float* pData);

/**
\brief create special object, called "Advanced Texture".
\param pScnData  - scene data object ptr.
\param a_texType - Advanced texture type. Can be "falloff" or "dirt"
\param a_objName - object name. can be set to "" or nullptr

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
\brief open node to change it's parameters. Usually this is needed for "Advanced Texture" HRTextureNode or any other complex node type can be opened and changed
\param a_pResource - a pointer to advanced texture
\param a_openMode  - open mode

*/
HAPI void              hrTextureNodeOpen(HRTextureNodeRef a_pResource, HR_OPEN_MODE a_openMode);

/**
\brief close node
\param a_pResource - a pointer to advanced texture

*/
HAPI void              hrTextureNodeClose(HRTextureNodeRef a_pResource);

/**
\brief  bind texture to some xml node that is attached to a material o light. In fact just create proxy reference in xml.
\param  a_pTexNode - a pointer to a texture.
\param  a_node - node reference where proxy must be inserted
\return resulting texture node

*/
HAPI pugi::xml_node hrTextureBind(HRTextureNodeRef a_pTexNode, pugi::xml_node a_node);


/**
\brief  allow to change custom texture parameters.
\param  a_texRef - texture object reference
\return cpp reference to xml node that user can directly work with

If texture was not open, this function return reference nullptr xml_node.
All changes of that dummy will be discarded.

*/
HAPI pugi::xml_node    hrTextureParamNode(HRTextureNodeRef a_texRef);




/**
\brief create HydraMaterial which is "compound" object, contain several BRDF and some geometric properties.
\param pScnData      - a pointer to scene library
\param a_objectName  - object name. Can be set to "" or nullptr.

 
 For a user HydraMaterial behaves like a traditional "uber material" with huge set of predefined parameters.
 It contain everything - emission, reflection, transparency, opacity, displacement and e.t.c.

 However, inside renderer, it will be translated to BRDF tree (by well specified algorithm) with actual complexity depends on what parameters you have set.
 For example if you set only diffuse properties, there will be no tree, only one leaf node.


*/
HAPI HRMaterialRef     hrMaterialCreate(const wchar_t* a_objectName);

/**
\brief create BRDF leaf which is "atomic" object, contain single BRDF.
\param pScnData      - a pointer to scene library
\param a_brdfType    - a type of BRDF. This type strictly define futher properties of material that can be set inside xml representation. 
       Can be one of ("lambert", "oren_nayar", "phong", "torrance_sparrow", "perfect_mirror", "thin_glass", "glass", "translucent", "shadow_matte"); 
\param a_objectName  - object name. Can be set to "" or nullptr.

Unlike HydraMaterial this function create HydraBRDFLeaf - much more simple object, representing material with a single BRDF.

There are two main cases when you may want to use this function:

(1) If you need to strictly specify BRDF properties and you do want to know how exactly BRDF is constructed for your material.
In this case use combination of hrMaterialCreateBRDFLeaf and hrMaterialCreateBlend to create material you want by manually constructing BRDF tree.
The algorithm of constructing such tree from HydraMaterial is well specified, however, it is not convinient to use HydraMaterial if you do know how your material BRDF should behave.
You may accidently forget to set some parameter (like reflection extrusion) to correct value and it is not easy to traverse through HydraMaterial predefined BRDF tree to understang why your material work in other-than expected way.    
Due to that it is recommended to construct BRDF tree manually.

(2) For the consideration of some optimisations and code cleaness. Assume you have a checkbox in your material that should change its behavior in some way.
For example, disable a transparency and translucency. You may use HydraMaterial and set those parameters to 0. 
Normally this will work as expected and when translating HydraMaterial to BRDF tree renderer understand this and discard all 0 nodes. 
However, to be sure you don't have unnesesary BRDF nodes, you'd better manually construct brdf tree that have only those nodes, you have created. 
So, you don't have to rely on unknowing properties of HydraMaterial internal BRDF tree optimiser.


*/
HAPI HRMaterialRef     hrMaterialCreateBRDFLeaf(const wchar_t* a_brdfType, const wchar_t* a_objectName);

/**
\brief Create HydraMaterialBlend (a node in brdf tree that blends two materials)
\param pScnData      - a pointer to scene library
\param a_objectName  - object name. Can be set to "" or nullptr.
\param a_pMat1       - first  child material.
\param a_pMat2       - second child material. 

*/
HAPI HRMaterialRef     hrMaterialCreateBlend(const wchar_t* a_objectName, HRMaterialRef a_pMat1, HRMaterialRef a_pMat2);

/**
\brief open material for changing parameters and binding textures
\param pMat   - a pointer to HRMaterial object
\param a_mode - open mode

*/
HAPI void              hrMaterialOpen(HRMaterialRef pMat, HR_OPEN_MODE a_mode);

/**
\brief close material object. Changes can not be made after object was closed, you have to reopen it.
\param pMat - a pointer to HRMaterial object

*/
HAPI void              hrMaterialClose(HRMaterialRef pMat);

/**
\brief allow to change custom material parameters.
\param a_matRef - light object reference
\return cpp reference to xml node that user can directly work with

If material was not open, this function return reference to empty dummy node. All changes of that dummy will be discarded.

*/
HAPI pugi::xml_node    hrMaterialParamNode(HRMaterialRef a_matRef);


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
\brief create light object
\param pScnData - scene data object ptr.
\param a_objectName - light name. Can be "" or nullptr

*/
HAPI HRLightRef        hrLightCreate(const wchar_t* a_objectName);

/**
\brief open light object
\param pLight - pointer to light object
\param a_mode - open mode

*/
HAPI void              hrLightOpen(HRLightRef pLight, HR_OPEN_MODE a_mode);

/**
\brief close light object
\param pLight - pointer to light object

*/
HAPI void              hrLightClose(HRLightRef pLight);

/**
\brief allow to change custom light parameters. 
\param a_lightRef - light object reference
\return cpp reference to xml node that user can directly work with

If light was not open, this function return reference to empty dummy node. All changes of that dummy will be discarded.

*/
HAPI pugi::xml_node    hrLightParamNode(HRLightRef a_lightRef);


/**
\brief create camera object
\param pScnData - scene data object ptr.
\param a_objectName - camera name. Can be "" or nullptr

*/
HAPI HRCameraRef       hrCameraCreate(const wchar_t* a_objectName);

/**
\brief open camera object
\param pCam   - pointer to camera object
\param a_mode - open mode

*/
HAPI void              hrCameraOpen(HRCameraRef pCam, HR_OPEN_MODE a_mode);

/**
\brief close canera object
\param pCam - pointer to light object

*/
HAPI void              hrCameraClose(HRCameraRef pCam);

/**
\brief  allow to change custom camera parameters.
\param  a_camRef - camera object reference
\return cpp reference to xml node that user can directly work with

If camera was not open, this function return reference nullptr xml_node. 
All changes of that dummy will be discarded.

*/
HAPI pugi::xml_node hrCameraParamNode(HRCameraRef a_camRef);


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/// Mesh index type.

enum HR_PRIM_TYPE { HR_TRIANGLE_LIST  = 0,    ///< simple triangle list
                    HR_TRIANGLE_IND3  = 1,    ///< common layout, 3 indices of vertices per ptriangle
                    HR_TRIANGLE_IND9  = 2,    ///< separate indices for pos, norm, texCoord
                    HR_TRIANGLE_IND12 = 3     ///< separate indices for pos, norm, texCoord, tangent
};  

/**
\brief create mesh
\param a_pScn       - pointer to scene library
\param a_objectName - object name. Can be "" or nullptr

*/
HAPI HRMeshRef hrMeshCreate(const wchar_t* a_objectName);

/**
\brief create mesh from internal vsgf format with delayed load.
\param a_pScn     - pointer to scene library
\param a_fileName - file name of the mesh.

*/
HAPI HRMeshRef hrMeshCreateFromFileDL(const wchar_t* a_fileName);

/**
\brief open mesh
\param a_pMesh - pointer to mesh
\param a_type - primitive type that will be used during current open/close session.
\param a_mode - open mode

*/
HAPI void              hrMeshOpen(HRMeshRef a_pMesh, HR_PRIM_TYPE a_type, HR_OPEN_MODE a_mode);

/**
\brief close mesh
\param a_pMesh      - pointer to mesh

*/
HAPI void              hrMeshClose(HRMeshRef a_pMesh);


/**
\brief set input vertex attribute pointer of type float1
\param a_pMesh   - pointer to mesh
\param a_name    - attribute name 
\param a_pointer - input data pointer
\param a_stride  - input stride in bytes; 0 means 4, like in OpenGL whan attributes are placed tightly, just an float array

*/

HAPI void              hrMeshVertexAttribPointer1f(HRMeshRef pMesh, const wchar_t* a_name, const float* a_pointer, int a_stride = 0);

/**
\brief set input attribute pointer of type float2
\param a_pMesh   - pointer to mesh
\param a_name    - attribute name
\param a_pointer - input data pointer
\param a_stride  - input stride in bytes; 0 means 8, like in OpenGL whan attributes are placed tightly, just an float2 array

*/
HAPI void              hrMeshVertexAttribPointer2f(HRMeshRef pMesh, const wchar_t* a_name, const float* a_pointer, int a_stride = 0);

/**
\brief set input vertex attribute pointer of type float3
\param a_pMesh   - pointer to mesh
\param a_name    - attribute name
\param a_pointer - input data pointer
\param a_stride  - input stride in bytes; 0 means 12, like in OpenGL whan attributes are placed tightly, just an float3 array

*/
HAPI void              hrMeshVertexAttribPointer3f(HRMeshRef pMesh, const wchar_t* a_name, const float* a_pointer, int a_stride = 0);

/**
\brief set input vertex attribute pointer of type float4
\param a_pMesh   - pointer to mesh
\param a_name    - attribute name
\param a_pointer - input data pointer
\param a_stride  - input stride in bytes; 0 means 16, like in OpenGL whan attributes are placed tightly, just an float4 array

*/
HAPI void              hrMeshVertexAttribPointer4f(HRMeshRef pMesh, const wchar_t* a_name, const float* a_pointer, int a_stride = 0);
         

/**
\brief set input _primitive_ attribute pointer of type int1
\param a_pMesh   - pointer to mesh
\param a_name    - attribute name
\param a_pointer - input data pointer
\param a_stride  - input stride in bytes; 0 means 4, like in OpenGL whan attributes are placed tightly, just an float array

*/

HAPI void              hrMeshPrimitiveAttribPointer1i(HRMeshRef pMesh, const wchar_t* a_name, const int* a_pointer,   int a_stride = 0);


/**
\brief Assign material to all triangles of mesh
\param a_pMesh - pointer to mesh
\param matId   - material id

*/

HAPI void              hrMeshMaterialId(HRMeshRef a_pMesh, int matId);


/**
\brief like glDrawArrays
\param a_pMesh - pointer to mesh

\param indNum        - input triangle indices num
\param indices       - input triangle indices
\param weld_vertices - flag for enable vertex cache i.e. collapse "same" vertices together.

*/
HAPI void              hrMeshAppendTriangles3(HRMeshRef a_pMesh, int indNum, const int* indices, bool weld_vertices = true);



/**
\brief get vertex attribute pointer. The mesh must be opened! Else the function will return nullptr;
\param a_pMesh - pointer to mesh
\param attributeName - attribute name (currently checks only for "pos", "norm" and "uv")

*/
HAPI void*             hrMeshGetAttribPointer(HRMeshRef a_mesh, const wchar_t* attributeName);


/**
\brief get primitive attribute pointer. The mesh must be opened! Else the function will return nullptr;
\param a_pMesh - pointer to mesh
\param attributeName - attribute name (currently checks only for "mind", only)

*/
HAPI void*             hrMeshGetPrimitiveAttribPointer(HRMeshRef a_mesh, const wchar_t* attributeName);

/**
\brief Information about opened mesh. // You can't call this function if mesh is not open

*/
struct HROpenedMeshInfo
{
  HROpenedMeshInfo() : vertNum(0), indicesNum(0) {}

  int32_t vertNum;
  int32_t indicesNum;
};

/**
\brief get mesh info. You can't call this function if mesh is not open.
\param a_pMesh - pointer to mesh
\return  Information about opened mesh. 

*/
HAPI HROpenedMeshInfo  hrMeshGetInfo(HRMeshRef a_mesh);


/**
\brief get params node for mesh
\param a_pMesh - pointer to mesh

*/
HAPI pugi::xml_node    hrMeshParamNode(HRMeshRef a_meshRef);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Final Scene 
//
HAPI HRSceneInstRef    hrSceneCreate(const wchar_t* a_objectName);
HAPI void              hrSceneOpen(HRSceneInstRef pScn, HR_OPEN_MODE a_mode);
HAPI void              hrSceneClose(HRSceneInstRef pScn);


/**
\brief like glDrawArraysInstanced
\param a_pScn   - pointer to scene where mesh instances will be inserted
\param a_pMesh  - pointer to mesh
\param a_mat    - matrix
\param a_mmList - multimaterial remap pairs array;             @ALWAYS must be multiple of 2
\param a_mmListSize - size of multimaterial remap pairs array; @EXAMPLE: [0,1,3,4,7,5] means [0-->1; 3-->4; 7-->5;]
*/
HAPI int              hrMeshInstance(HRSceneInstRef a_pScn, HRMeshRef a_pMesh, float a_mat[16], 
                                      const int32_t* a_mmListm = nullptr, int32_t a_mmListSize = 0);
/**
\brief like glDrawArraysInstanced, but for lights
\param a_pScn          - pointer to scene where mesh instances will be inserted
\param a_pMesh         - pointer to mesh
\param a_mat           - matrix
\param a_customAttribs - wstring that contains custom attrib list. For example: L"color_mult ="\1 0 0"\ rotationMatrix = "\1 0 0 0 1 0 0 0 1"\"

*/
HAPI void              hrLightInstance(HRSceneInstRef pScn, HRLightRef  pLight, float m[16], const wchar_t* a_customAttribs = nullptr);


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HAPI HRRenderRef       hrRenderCreate(const wchar_t* a_className, const wchar_t* a_flags = L"");
HAPI void              hrRenderOpen(HRRenderRef a_pRender, HR_OPEN_MODE a_mode);
HAPI void              hrRenderClose(HRRenderRef a_pRender);
HAPI pugi::xml_node    hrRenderParamNode(HRRenderRef a_camRef);

/**
\brief OpenCL and other (pure C/C++) devices that can be used for rendering
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
\brief return a pointer to the head of device list
*/
HAPI const HRRenderDeviceInfoListElem* hrRenderGetDeviceList(HRRenderRef a_pRender);

/**
\brief enable or disable target device by id. 
*/
HAPI void hrRenderEnableDevice(HRRenderRef a_pRender, int32_t a_deviceId, bool a_enable);

/**
\brief these struct is returned by hrRenderHaveUpdate
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
\brief indicate that render do have Update for FrameBuffer
*/
HAPI HRRenderUpdateInfo  hrRenderHaveUpdate(HRRenderRef pRender);


/**
\brief get framebuffer content to imgData
*/
HAPI bool hrRenderGetFrameBufferHDR4f(HRRenderRef pRender, int w, int h, float* imgData, const wchar_t* a_layerName = L"color");  // w*h*4*sizeof(float)

/**
\brief get framebuffer content to imgData, only color
*/
HAPI bool hrRenderGetFrameBufferLDR1i(HRRenderRef pRender, int w, int h, int32_t* imgData);  // w*h*sizeof(int) --> RGBA

/**
\brief get framebuffer line to imgData. 
*/
HAPI bool hrRenderGetFrameBufferLineHDR4f(HRRenderRef pRender, int a_begin, int a_end, int a_y, float* imgData, const wchar_t* a_layerName = L"color");  // w*h*4*sizeof(float)

/**
\brief get framebuffer line to imgData, only color
*/
HAPI bool hrRenderGetFrameBufferLineLDR1i(HRRenderRef pRender, int a_begin, int a_end, int a_y, int32_t* imgData);  // w*h*sizeof(int) --> RGBA

/**
\brief save framebuffer content to imgData (convert it to LDR). Color only.
*/
HAPI bool hrRenderSaveFrameBufferLDR(HRRenderRef pRender, const wchar_t* a_outFileName);  // w*h*4*sizeof(float)

/**
\brief save framebuffer content to imgData. Color only.
*/
HAPI bool hrRenderSaveFrameBufferHDR(HRRenderRef pRender, const wchar_t* a_outFileName);  // w*h*4*sizeof(float)

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
\brief unpack gbuffer data for a custom line.
* \param a_pRender - render reference
* \param a_lineNumber - line number, i.e. y coordinate
* \param a_lineData - output line of size at least (a_endX - a_startX)
* \param a_startX   - x coordinate, start of the line
* \param a_endX     - x coordinate, end of the line
*
*/

HAPI void hrRenderGetGBufferLine(HRRenderRef a_pRender, int32_t a_lineNumber, HRGBufferPixel* a_lineData, int32_t a_startX, int32_t a_endX);  // w*4*sizeof(float)

/**
\brief save custom gbuffer layer
* \param a_pRender     - render reference
* \param a_outFileName - output file name
* \param a_layerName   - layer name we are going to save [depth,normals,texcoord,diffcolor,alpha,shadow,matid,objid,instid]
* \param a_palette     - color palette; aplied to indices only!
* \param a_paletteSize - color palette size;

If palette is not set for "id" layers, this function will use it's default palette. 
if GBuffer was not evaluated by render driver (you have to set 'evalgbuffer' = 1 in render settings), this function do nothing.

*/
HAPI bool hrRenderSaveGBufferLayerLDR(HRRenderRef a_pRender, const wchar_t* a_outFileName, const wchar_t* a_layerName,
                                      const int* a_palette = nullptr, int a_paletteSize = 0);

/**
\brief execute custom command for render driver
* \param a_pRender - render reference
* \param a_command - command string
* \param a_answer  - optional string answer; max 256 wchars;
* 
*  In the case of "HydraModern" render driver, the command will be sended to all render processes. 
*  All these commands (except 'exitnow') are used in default 'offline' render mode. The 'interactive' render mode is controlled by hrCommit only.
*  command list:
* 
*  start    [-statefile statex_00009.xml] 
*                                  -- signal to start rendering for hydra processes that are waiting. "-statefile" argument is optional.
*                                  -- Automaticly sends after hrFlush() is performed (or hrCommit() + shared VB is enabled).
*
*  continue [-statefile statex_00009.xml] 
*                                  -- launch hydra processes first, then and send them "start".  "-statefile" argument is optional.
*
*  runhydra -cl_device_id 0 [-statefile statex_00009.xml] [-contribsamples 256] [-maxsamples 512]   
*
*                                  -- launch single hydra process and pass everything via command line; i.e. running process will not react to any commands (except "exitnow") in this mode; 
*                                  -- this is like running hydra process in the 'box mode'; you can only stop it.    
*          
*           args: -statefile       -- specify hydra process to render target state
*           args: -contribsamples  -- specify hydra process to count exact contribution to shared image this process should do and then exit when reach contribsamples value
*           args: -maxsamples      -- specify hydra process to evaluate no more then maxsamples values sample per pixel; note that this value should be greater then contribsamples.
*           args: -cl_device_id    -- specify device id for hydra process
* 
*  exitnow [-cl_device_id 0]       -- all hydra processes should immediately exit; can be used also in 'interactive mode' and 'box mode'
*                                  -- #NOT_IMPLEMENTED: optionally you can stop only single process by specifying target device id.                                 
*
*  pause  z_image.bin              -- save accumulated shared image to "z_image.bin", then send "exitnow"; no spaces, no quotes allowed, single string file name
*                                 
*  resume z_image.bin              -- load accumulated shared image from "z_image.bin", then send "start" to waiting processes. 
*                                  -- note that this command does not launch hydra processes (!!!), you have to manually call "hrCommit(scnRef, renderRef)".
*                                  -- This is due to pause may occur within main program exit. When main program will be launched again it must open scene
*                                  -- we want to continue render and Commit the new state via hrCommit(scnRef, renderRef). Thus it launch processes.
*/
HAPI void hrRenderCommand(HRRenderRef a_pRender, const wchar_t* a_command, wchar_t* a_answer = nullptr);


/**
\brief set log directory for renderer. If a_logDir == L"" or nullptr, logging is disabled (default).
* \param a_pRender     - render reference
* \param a_logDir      - directory where renderer save logs.
* \param a_hideConsole - if you want to hide console
*/
HAPI void hrRenderLogDir(HRRenderRef a_pRender, const wchar_t* a_logDir, bool a_hideConsole = false);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
\brief non blocking commit, send commands to renderer and return immediately.
* 
* For the "HydraModern" render driver this command will launch new process or transfer changes to existing (if interactive mode is implemented and enabled).
*/
HAPI void hrCommit(HRSceneInstRef a_pScn = HRSceneInstRef(), 
                   HRRenderRef a_pRender = HRRenderRef(),
                   HRCameraRef a_pCam    = HRCameraRef()); // non blocking commit, send commands to renderer and return immediately 

/**
\brief blocking commit, waiting for all current commands to be executed
*/
HAPI void hrFlush(HRSceneInstRef a_pScn = HRSceneInstRef(), 
                  HRRenderRef a_pRender = HRRenderRef(),
                  HRCameraRef a_pCam    = HRCameraRef());

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef int32_t HR_IDType;

constexpr int64_t VIRTUAL_BUFFER_SIZE = int64_t(2048)*int64_t(1024 * 1024);



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
\brief An extension helper for groups of lights
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
\brief instance group of lights

*/
HAPI void  hrLightGroupInstanceExt(HRSceneInstRef pScn, HRLightGroupExt pLight, float m[16], const wchar_t** a_customAttribsArray = nullptr);

#ifdef WIN32
#undef min
#undef max
#endif

namespace HRUtils
{
  /**
  \brief Convert LDR cube map to LDR spheremap

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


  HRTextureNodeRef Cube2SphereLDR(HRTextureNodeRef a_cube[6]);

  BBox InstanceSceneIntoScene(HRSceneInstRef a_scnFrom, HRSceneInstRef a_scnTo, float a_mat[16], bool origin = true,
                              const int32_t* remapListOverride = nullptr, int32_t remapListSize = 0);

  HRSceneInstRef MergeLibraryIntoLibrary(const wchar_t* a_libPath, bool mergeLights = false, bool copyScene = false);

  HRMaterialRef MergeOneMaterialIntoLibrary(const wchar_t* a_libPath, const wchar_t* a_matName, int a_matId = -1);

  HRMeshRef MergeOneMeshIntoLibrary(const wchar_t* a_libPath, const wchar_t* a_meshName);

  HRLightRef MergeOneLightIntoLibrary(const wchar_t* a_libPath, const wchar_t* a_lightName);

  HRTextureNodeRef MergeOneTextureIntoLibrary(const wchar_t* a_libPath, const wchar_t* a_texName, int a_texId = -1);
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
\brief get HRMaterialRef by name from the library.
\param a_matName  - material name


 If material with name = a_matName does not exist, HRMaterialRef with id = -1 is returned

*/
HAPI HRMaterialRef hrFindMaterialByName(const wchar_t *a_matName);


/**
\brief get HRLightRef by name from the library.
\param a_lightName  - light name


 If light with name = a_lightName does not exist, HRLightRef with id = -1 is returned

*/
HAPI HRLightRef hrFindLightByName(const wchar_t *a_lightName);

/**
\brief get HRCameraRef by name from the library.
\param a_cameraName  - camera name


 If camera with name = a_cameraName does not exist, HRCameraRef with id = -1 is returned

*/
HAPI HRCameraRef hrFindCameraByName(const wchar_t *a_cameraName);

/**
\brief get HRRenderRef by its type name from the library.
\param a_renderTypeName - render type name


If camera with name = a_cameraName does not exist, HRCameraRef with id = -1 is returned

*/
HAPI HRRenderRef hrFindRenderByTypeName(const wchar_t *a_renderTypeName);
