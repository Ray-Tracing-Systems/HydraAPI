#include "HydraAPI.h"
#include "HydraInternal.h"

#include <memory>
#include <vector>
#include <string>
#include <map>

#include <sstream>
#include <fstream>
#include <iomanip>

#include "HydraObjectManager.h"
#include "xxhash.h"

extern std::wstring      g_lastError;
extern std::wstring      g_lastErrorCallerPlace;
extern HR_ERROR_CALLBACK g_pErrorCallback;
extern HRObjectManager   g_objManager;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static pugi::xml_node my_force_child(pugi::xml_node a_parent, const wchar_t* a_name) ///< helper function
{
  pugi::xml_node child = a_parent.child(a_name);
  if (child != nullptr)
    return child;
  else
    return a_parent.append_child(a_name);
}

static pugi::xml_attribute my_force_attrib(pugi::xml_node a_parent, const wchar_t* a_name) ///< helper function
{
  pugi::xml_attribute attr = a_parent.attribute(a_name);
  if (attr != nullptr)
    return attr;
  else
    return a_parent.append_attribute(a_name);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HAPI HRTextureNodeRef hrTexture2DCreateFromFile(const wchar_t* a_fileName, int w, int h, int bpp) // no binding HRSceneData and creation of Texture2D
{
  /////////////////////////////////////////////////////////////////////////////////////////////////

  {
    auto p = g_objManager.scnData.m_textureCache.find(a_fileName);
    if (p != g_objManager.scnData.m_textureCache.end())
    {
      HRTextureNodeRef ref;
      ref.id = p->second;
      return ref;
    }
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////

  HRTextureNode texRes;
  texRes.name = std::wstring(a_fileName);
  texRes.id   = int32_t(g_objManager.scnData.textures.size());
  g_objManager.scnData.textures.push_back(texRes);

  HRTextureNodeRef ref;
  ref.id = HR_IDType(g_objManager.scnData.textures.size() - 1);

  HRTextureNode& texture   = g_objManager.scnData.textures[ref.id];
  auto pTextureImpl        = g_objManager.m_pFactory->CreateTexture2DFromFile(&texture, a_fileName);
  texture.pImpl            = pTextureImpl;
  texture.m_loadedFromFile = true;

  pugi::xml_node texNodeXml = g_objManager.textures_lib_append_child();

  ChunkPointer chunk(&g_objManager.scnData.m_vbCache);

  if (pTextureImpl != nullptr)
  {
    auto chunkId = pTextureImpl->chunkId();
    chunk = g_objManager.scnData.m_vbCache.chunk_at(chunkId);

    w   = pTextureImpl->width();
    h   = pTextureImpl->height();
    bpp = pTextureImpl->bpp();
  }
  else
  {
    HrPrint(HR_SEVERITY_WARNING, L"hrTexture2DCreateFromFile can't open file ", a_fileName);
    g_objManager.scnData.textures.pop_back();
    ref.id = 0;
    return ref;
  }

  auto byteSize = size_t(w*h)*size_t(bpp);

  // form tex name
  //
  const std::wstring id       = ToWString(ref.id);
  const std::wstring location = ChunkName(chunk);
  const std::wstring bytesize = ToWString(byteSize);

	texNodeXml.append_attribute(L"id").set_value(id.c_str());
  texNodeXml.append_attribute(L"name").set_value(a_fileName);
  texNodeXml.append_attribute(L"path").set_value(a_fileName);

  if (pTextureImpl == nullptr)
    texNodeXml.append_attribute(L"loc").set_value(L"unknown");
  else
    g_objManager.SetLoc(texNodeXml, location);

  texNodeXml.append_attribute(L"offset").set_value(L"8");
  texNodeXml.append_attribute(L"bytesize").set_value(bytesize.c_str());
  texNodeXml.append_attribute(L"width") = w;
  texNodeXml.append_attribute(L"height") = h;
  texNodeXml.append_attribute(L"dl").set_value(L"0");

  g_objManager.scnData.textures[ref.id].update_next(texNodeXml);
  g_objManager.scnData.m_textureCache[a_fileName] = ref.id; // remember texture id for given file name

  return ref;
}

void GetTextureFileInfo(const wchar_t* a_fileName, int32_t* pW, int32_t* pH, size_t* pByteSize);

HAPI HRTextureNodeRef hrTexture2DCreateFromFileDL(const wchar_t* a_fileName, int w, int h, int bpp)
{
  /////////////////////////////////////////////////////////////////////////////////////////////////
  {
    auto p = g_objManager.scnData.m_textureCache.find(a_fileName);
    if (p != g_objManager.scnData.m_textureCache.end())
    {
      HRTextureNodeRef ref;
      ref.id = p->second;
      return ref;
    }
  }
  /////////////////////////////////////////////////////////////////////////////////////////////////

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
  std::wstring s1(a_fileName);
  std::string  s2(s1.begin(), s1.end());
  std::ifstream testFile(s2.c_str());
#elif defined WIN32
  std::ifstream testFile(a_fileName);
#endif

  if (!testFile.good())
  {
    HrPrint(HR_SEVERITY_WARNING, L"hrTexture2DCreateFromFileDL, bad file ", a_fileName);
    HRTextureNodeRef ref2; // dummy white texture
    ref2.id = 0;
    return ref2;
  }
  else
    testFile.close();

  HRTextureNode texRes;
  texRes.name = std::wstring(a_fileName);
  texRes.id   = int32_t(g_objManager.scnData.textures.size());
  g_objManager.scnData.textures.push_back(texRes);

  HRTextureNodeRef ref;
  ref.id = HR_IDType(g_objManager.scnData.textures.size() - 1);

  HRTextureNode& texture   = g_objManager.scnData.textures[ref.id];
  texture.m_loadedFromFile = true;

  pugi::xml_node texNodeXml = g_objManager.textures_lib_append_child();

  // form tex name
  //
  std::wstring id = ToWString(ref.id);

	texNodeXml.append_attribute(L"id").set_value(id.c_str());
  texNodeXml.append_attribute(L"name").set_value(a_fileName);
  texNodeXml.append_attribute(L"path").set_value(a_fileName);

  int32_t w1, h1;
  size_t  bpp1;
  GetTextureFileInfo(a_fileName, &w1, &h1, &bpp1);

  texNodeXml.append_attribute(L"width")    = w1; 
  texNodeXml.append_attribute(L"height")   = h1; 
  texNodeXml.append_attribute(L"bytesize") = w1*h1*bpp1;
  texNodeXml.append_attribute(L"dl").set_value(L"1");

  if(w > 0) texNodeXml.append_attribute(L"width_rec")  = w;
  if(h > 0) texNodeXml.append_attribute(L"height_rec") = h;

  g_objManager.scnData.textures[ref.id].update_next(texNodeXml);
  g_objManager.scnData.m_textureCache[a_fileName] = ref.id; // remember texture id for given file name

  return ref;
}


HAPI HRTextureNodeRef hrTexture2DUpdateFromFile(HRTextureNodeRef currentRef, const wchar_t* a_fileName, int w, int h, int bpp)
{
  int w1, h1, bpp1;
  if (g_objManager.m_pImgTool->LoadImageFromFile(a_fileName, w1, h1, bpp1, g_objManager.m_tempBuffer))
    return hrTexture2DUpdateFromMemory(currentRef, w1, h1, bpp1, g_objManager.m_tempBuffer.data());
  else
    return currentRef;
}



HAPI HRTextureNodeRef hrTexture2DCreateFromMemory(int w, int h, int bpp, const void* a_data)
{
  if (w == 0 || h == 0 || bpp == 0 || a_data == nullptr)
  {
    HrPrint(HR_SEVERITY_WARNING, L"hrTexture2DCreateFromMemory, invalid input");
    HRTextureNodeRef ref2; // dummy white texture
    ref2.id = 0;
    return ref2;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////

  std::wstringstream outStr;
  outStr << L"texture2d_" << g_objManager.scnData.textures.size();

  HRTextureNode texRes; // int w, int h, int bpp, const void* a_data
  texRes.name = outStr.str();
  g_objManager.scnData.textures.push_back(texRes);

  HRTextureNodeRef ref;
  ref.id = HR_IDType(g_objManager.scnData.textures.size() - 1);

  HRTextureNode& texture = g_objManager.scnData.textures[ref.id];
  auto pTextureImpl = g_objManager.m_pFactory->CreateTexture2DFromMemory(&texture, w, h, bpp, a_data);
  texture.pImpl = pTextureImpl;

  pugi::xml_node texNodeXml = g_objManager.textures_lib_append_child();

  auto byteSize = size_t(w)*size_t(h)*size_t(bpp);

  if (pTextureImpl != nullptr)
  {
    ChunkPointer chunk = g_objManager.scnData.m_vbCache.chunk_at(pTextureImpl->chunkId());

    // form tex name
    //
    std::wstringstream namestr;
    namestr << L"Map#" << ref.id;
    std::wstring texName = namestr.str();
    std::wstring id = ToWString(ref.id);
    std::wstring location = ChunkName(chunk);
    std::wstring bytesize = ToWString(byteSize);

    texNodeXml.append_attribute(L"id").set_value(id.c_str());
    texNodeXml.append_attribute(L"name").set_value(texName.c_str());
    g_objManager.SetLoc(texNodeXml, location);
    texNodeXml.append_attribute(L"offset").set_value(L"8");
    texNodeXml.append_attribute(L"bytesize").set_value(bytesize.c_str());
    texNodeXml.append_attribute(L"width")  = w;
    texNodeXml.append_attribute(L"height") = h;
    texNodeXml.append_attribute(L"dl").set_value(L"0");

    g_objManager.scnData.textures[ref.id].update_next(texNodeXml);

    return ref;
  }
  else
  {
    HRTextureNodeRef res;
    res.id = -1;
    return res;
  }
}


HAPI HRTextureNodeRef hrTexture2DUpdateFromMemory(HRTextureNodeRef currentRef, int w, int h, int bpp, const void* a_data)
{
  if (w == 0 || h == 0 || bpp == 0 || a_data == nullptr)
  {
    HrPrint(HR_SEVERITY_WARNING, L"hrTexture2DUpdateFromMemory, invalid input");
    return currentRef;
  }
	
  // check for user try to update texture with exactly same data (each frame updates). 
  //   
  {
    auto* pSysObject = g_objManager.PtrById(currentRef);
    auto pImpl       = pSysObject->pImpl;

    if (pImpl != nullptr)
    {
      const void* data     = pImpl->GetData();
      const size_t texSize = pImpl->DataSizeInBytes();

      if (data != nullptr && texSize == size_t(w*h)*size_t(bpp) && pImpl->width() == w && pImpl->height() == h)
        if (memcmp(data, a_data, texSize) == 0)
          return currentRef;
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::wstringstream outStr;
	outStr << L"texture2d_" << g_objManager.scnData.textures.size();

	HRTextureNodeRef ref;
	ref.id = currentRef.id;

	HRTextureNode& texture = g_objManager.scnData.textures[ref.id];
	auto pTextureImpl      = g_objManager.m_pFactory->CreateTexture2DFromMemory(&texture, w, h, bpp, a_data);
	texture.pImpl          = pTextureImpl;

	pugi::xml_node texNodeXml = g_objManager.textures_lib_append_child();

	auto byteSize = size_t(w)*size_t(h)*size_t(bpp);

	ChunkPointer chunk = g_objManager.scnData.m_vbCache.chunk_at(pTextureImpl->chunkId());

	// form tex name
	//
	std::wstringstream namestr;
	namestr << L"Map#" << ref.id;
	std::wstring texName  = namestr.str();
	std::wstring id       = ToWString(ref.id);
	std::wstring location = ChunkName(chunk);
	std::wstring bytesize = ToWString(byteSize);

  texNodeXml.append_attribute(L"id").set_value(id.c_str());
	texNodeXml.append_attribute(L"name").set_value(texName.c_str());
  g_objManager.SetLoc(texNodeXml, location);
	texNodeXml.append_attribute(L"offset").set_value(L"8");
	texNodeXml.append_attribute(L"bytesize").set_value(bytesize.c_str());
  texNodeXml.append_attribute(L"width")  = w;
  texNodeXml.append_attribute(L"height") = h;
  texNodeXml.append_attribute(L"dl").set_value(L"0");

	g_objManager.scnData.textures[ref.id].update_next(texNodeXml);

	return ref;
}

HAPI HRTextureNodeRef hrArray1DCreateFromMemory(const float* a_data, int a_size) // #TODO: implement, add "g_objManager.scnData.textures[ref.id].Update(texNodeXml);"
{
  if (a_size == 0 || a_data == nullptr)
  {
    HrPrint(HR_SEVERITY_WARNING, L"hrArray1DCreateFromMemory, invalid input");
    HRTextureNodeRef ref2; // dummy white texture
    ref2.id = 0;
    return ref2;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////
  std::wstringstream outStr;
  outStr << L"array1d_" << g_objManager.scnData.textures.size();

  HRTextureNode texRes; // const float* a_data, int a_size
  texRes.name = outStr.str();
  g_objManager.scnData.textures.push_back(texRes);

  HRTextureNodeRef ref;
  ref.id = HR_IDType(g_objManager.scnData.textures.size() - 1);
  return ref;
}


HAPI void hrTexture2DGetSize(HRTextureNodeRef a_tex, int* pW, int* pH, int* pBPP)
{
  HRTextureNode* pTexture = g_objManager.PtrById(a_tex);

  if (pTexture == nullptr)
  {
    HrError(L"hrTexture2DGetSize: nullptr reference");
    (*pW)   = 0;
    (*pH)   = 0;
    (*pBPP) = 0;
    return;
  }

  auto xml_node = pTexture->xml_node_immediate();

  (*pW)   = xml_node.attribute(L"width").as_int();
  (*pH)   = xml_node.attribute(L"height").as_int();

  const size_t bytesize = (size_t)(xml_node.attribute(L"bytesize").as_ullong());
  (*pBPP) = int(bytesize/size_t((*pW)*(*pH)));
}

HAPI void hrTexture2DGetDataLDR(HRTextureNodeRef a_tex, int* pW, int* pH, int* pData)
{
  HRTextureNode* pTexture = g_objManager.PtrById(a_tex);

  if (pTexture == nullptr)
  {
    (*pW) = 0;
    (*pH) = 0;
    HrError(L"hrTexture2DGetDataLDR: nullptr reference");
    return;
  }

  auto xml_node = pTexture->xml_node_immediate();

  (*pW) = xml_node.attribute(L"width").as_int();
  (*pH) = xml_node.attribute(L"height").as_int();

  if (pTexture->pImpl == nullptr)
  {
    (*pW) = 0;
    (*pH) = 0;
    HrError(L"hrTexture2DGetDataLDR: nullptr texture data");
    return;
  }

  auto chunkId  = pTexture->pImpl->chunkId();
  auto chunk    = g_objManager.scnData.m_vbCache.chunk_at(chunkId);
  auto bytesize = xml_node.attribute(L"bytesize").as_int();
  auto offset   = xml_node.attribute(L"offset").as_int();

  char* data = (char*)chunk.GetMemoryNow();
  if (data == nullptr)
  {
#ifdef WIN32
    const std::wstring loc = g_objManager.GetLoc(xml_node);   // load from file from "loc" #TODO: find a way to test it in proper way.
#else
    std::wstring s1(g_objManager.GetLoc(xml_node));
    const std::string  loc(s1.begin(), s1.end());
#endif
    std::ifstream fin(loc.c_str(), std::ios::binary);
    fin.seekg(offset);
    fin.read((char*)pData, bytesize);
    fin.close();
  }
  else
    memcpy(pData, data + offset, bytesize);
}


HAPI void hrTexture2DGetDataHDR(HRTextureNodeRef a_tex, int* pW, int* pH, float* pData)
{
  HRTextureNode* pTexture = g_objManager.PtrById(a_tex);

  if (pTexture == nullptr)
  {
    (*pW) = 0;
    (*pH) = 0;
    HrError(L"hrTexture2DGetDataHDR: nullptr reference");
    return;
  }

  auto xml_node = pTexture->xml_node_immediate();

  (*pW) = xml_node.attribute(L"width").as_int();
  (*pH) = xml_node.attribute(L"height").as_int();

  if (pTexture->pImpl == nullptr)
  {
    (*pW) = 0;
    (*pH) = 0;
    HrError(L"hrTexture2DGetDataHDR: nullptr texture data");
    return;
  }

  auto chunkId  = pTexture->pImpl->chunkId();
  auto chunk    = g_objManager.scnData.m_vbCache.chunk_at(chunkId);
  auto bytesize = xml_node.attribute(L"bytesize").as_int();
  auto offset   = xml_node.attribute(L"offset").as_int();

  char* data = (char*)chunk.GetMemoryNow();
  if (data == nullptr)
  {

#ifdef WIN32
    const std::wstring loc = g_objManager.GetLoc(xml_node);   // load from file from "loc" #TODO: find a way to test it in proper way.
#else
    std::wstring s1(g_objManager.GetLoc(xml_node));
    const std::string  loc(s1.begin(), s1.end());
#endif

    std::ifstream fin(loc.c_str(), std::ios::binary);
    fin.seekg(offset);
    fin.read((char*)pData, bytesize);
    fin.close();
  }
  else
    memcpy(pData, data + offset, bytesize);
}


HAPI void hrTextureNodeOpen(HRTextureNodeRef a_pNode, HR_OPEN_MODE a_openMode)
{
  HRTextureNode* pData = g_objManager.PtrById(a_pNode);

  if (pData == nullptr)
  {
    HrError(L"hrTextureNodeOpen: nullptr reference");
    return;
  }

  pData->opened = true;

}

std::wstring LocalDataPathOfCurrentSceneLibrary();

void ProcessProcTexFile(const std::wstring& in_file, const std::wstring& out_file, const std::wstring& mainName, const std::wstring& prefix, 
                        pugi::xml_node a_node);

HAPI void hrTextureNodeClose(HRTextureNodeRef a_pNode)
{
  HRTextureNode* pData = g_objManager.PtrById(a_pNode);

  if (pData == nullptr)
  {
    HrError(L"hrTextureNodeClose: nullptr reference");
    return;
  }

  pugi::xml_node texNode = pData->xml_node_immediate();

  if (std::wstring(texNode.attribute(L"type").as_string()) == L"proc")
  {
    const wchar_t* filePath = texNode.child(L"code").attribute(L"file").as_string();
    const wchar_t* mainName = texNode.child(L"code").attribute(L"main").as_string();

    // shitty code; #TODO: refactor it
    //
    std::wstring dataFolderPath = LocalDataPathOfCurrentSceneLibrary();

    std::wstringstream namestream, namestream2;
    namestream  << std::fixed << L"data/"               << L"proctex_" << std::setfill(L"0"[0]) << std::setw(5) << texNode.attribute(L"id").as_string() << L".c";
    namestream2 << std::fixed << dataFolderPath.c_str() << L"proctex_" << std::setfill(L"0"[0]) << std::setw(5) << texNode.attribute(L"id").as_string() << L".c";
    std::wstring locName  = namestream.str();
    std::wstring locName2 = namestream2.str();

    if (texNode.child(L"code") != nullptr)
    {
      texNode.child(L"code").force_attribute(L"loc") = locName.c_str();

      pugi::xml_node generatedNode = texNode.child(L"code").force_child(L"generated");
      clear_node_childs(generatedNode);

      ProcessProcTexFile(filePath, locName2, mainName, texNode.attribute(L"id").as_string(),
                         generatedNode);
    }
  }

  pData->opened = false;
 // pData->pImpl  = nullptr;
}



HAPI pugi::xml_node hrTextureBind(HRTextureNodeRef a_pTexNode, pugi::xml_node a_node)
{
  HRTextureNode* pData = g_objManager.PtrById(a_pTexNode);
  if (pData == nullptr)
  {
    pugi::xml_node texNode = a_node.child(L"texture"); // delete texture
    texNode.parent().remove_child(texNode);
    HrPrint(HR_SEVERITY_WARNING, L"hrTextureBind: invalid texture id: ", a_pTexNode.id);
    return pugi::xml_node();
  }

  // add a_pTexNode to special list of material ... ? -> can do this later when close function works !!!
	//
  pugi::xml_node texNode = a_node.child(L"texture");
  if (texNode == nullptr)
    texNode = a_node.append_child(L"texture");

  pugi::xml_node texNodeOrigin = pData->xml_node_immediate();

  texNode.force_attribute(L"id").set_value(a_pTexNode.id);
  texNode.force_attribute(L"type").set_value(L"texref");

  const std::wstring texT = texNodeOrigin.attribute(L"type").as_string();
  if(texT == L"proc")
    texNode.force_attribute(L"type").set_value(L"texref_proc");

	return texNode;
}


HAPI HRTextureNodeRef  hrTexture2DCreateFromProcLDR(HR_TEXTURE2D_PROC_LDR_CALLBACK a_proc, void* a_customData,
                                                    int customDataSize, int w, int h)
{

  if (a_proc == nullptr || (a_customData == nullptr && customDataSize > 0))
  {
    HrPrint(HR_SEVERITY_WARNING, L"hrTexture2DCreateFromProcLDR, invalid input");
    HRTextureNodeRef ref2; // dummy white texture
    ref2.id = 0;
    return ref2;
  }

  if ((w != -1) && (h != -1)) //user specified resolution - can create texture immediately
  {
    int bpp = 4;

    auto* imageData = new unsigned char[w * h * bpp];

    a_proc(imageData, w, h, a_customData);

    HRTextureNodeRef procTex = hrTexture2DCreateFromMemory(w, h, bpp, imageData);

    delete [] imageData;

    return procTex;
  }
  else
  {
    std::wstringstream outStr;
    outStr << L"texture2d_" << g_objManager.scnData.textures.size();

    HRTextureNode texRes; // int w, int h, int bpp, const void* a_data
    texRes.name = outStr.str();
    texRes.id   = int32_t(g_objManager.scnData.textures.size());
    texRes.customData = malloc(customDataSize);
    memcpy(texRes.customData, a_customData, size_t(customDataSize));
    texRes.ldrCallback = a_proc;
    texRes.customDataSize = customDataSize;

    g_objManager.scnData.textures.push_back(texRes);

    HRTextureNodeRef ref;
    ref.id = texRes.id;

    pugi::xml_node texNodeXml = g_objManager.textures_lib_append_child();

    auto byteSize = 0;

    std::wstringstream namestr;
    namestr << L"Map#" << ref.id;
    std::wstring texName = namestr.str();
    std::wstring id = ToWString(ref.id);
    std::wstring bytesize = ToWString(byteSize);

    texNodeXml.append_attribute(L"id").set_value(id.c_str());
    texNodeXml.append_attribute(L"name").set_value(texName.c_str());
    texNodeXml.append_attribute(L"loc").set_value(L"");
    texNodeXml.append_attribute(L"offset").set_value(L"8");
    texNodeXml.append_attribute(L"bytesize").set_value(bytesize.c_str());
    texNodeXml.append_attribute(L"width") = w;
    texNodeXml.append_attribute(L"height") = h;
    texNodeXml.append_attribute(L"dl").set_value(L"0");

    g_objManager.scnData.textures[ref.id].update_next(texNodeXml);

    return ref;
  }
}

HAPI HRTextureNodeRef  hrTexture2DCreateFromProcHDR(HR_TEXTURE2D_PROC_HDR_CALLBACK a_proc, void* a_customData,
                                                    int customDataSize, int w, int h)
{

  if (a_proc == nullptr || (a_customData == nullptr && customDataSize > 0))
  {
    HrPrint(HR_SEVERITY_WARNING, L"hrTexture2DCreateFromProcLDR, invalid input");
    HRTextureNodeRef ref2; // dummy white texture
    ref2.id = 0;
    return ref2;
  }

  if ((w != -1) && (h != -1)) //user specified resolution - can create texture immediately
  {
    int bpp = sizeof(float)*4;

    auto* imageData = new float[w*h*bpp/sizeof(float)];

    a_proc(imageData, w, h, a_customData);

    HRTextureNodeRef procTex = hrTexture2DCreateFromMemory(w, h, bpp, imageData);

    delete [] imageData;

    return procTex;
  }
  else
  {
    std::wstringstream outStr;
    outStr << L"texture2d_" << g_objManager.scnData.textures.size();

    HRTextureNode texRes; // int w, int h, int bpp, const void* a_data
    texRes.name = outStr.str();
    texRes.id   = int32_t(g_objManager.scnData.textures.size());
    texRes.customData = malloc(customDataSize);
    memcpy(texRes.customData, a_customData, size_t(customDataSize));
    texRes.hdrCallback = a_proc;
    texRes.customDataSize = customDataSize;

    g_objManager.scnData.textures.push_back(texRes);

    HRTextureNodeRef ref;
    ref.id = texRes.id;

    pugi::xml_node texNodeXml = g_objManager.textures_lib_append_child();

    auto byteSize = 0;

    std::wstringstream namestr;
    namestr << L"Map#" << ref.id;
    std::wstring texName = namestr.str();
    std::wstring id = ToWString(ref.id);
    std::wstring bytesize = ToWString(byteSize);

    texNodeXml.append_attribute(L"id").set_value(id.c_str());
    texNodeXml.append_attribute(L"name").set_value(texName.c_str());
    texNodeXml.append_attribute(L"loc").set_value(L"");
    texNodeXml.append_attribute(L"offset").set_value(L"8");
    texNodeXml.append_attribute(L"bytesize").set_value(bytesize.c_str());
    texNodeXml.append_attribute(L"width") = w;
    texNodeXml.append_attribute(L"height") = h;
    texNodeXml.append_attribute(L"dl").set_value(L"0");

    g_objManager.scnData.textures[ref.id].update_next(texNodeXml);

    return ref;
  }

}

HAPI HRTextureNodeRef  hrTexture2DUpdateFromProcLDR(HRTextureNodeRef currentRef, HR_TEXTURE2D_PROC_LDR_CALLBACK a_proc,
                                                    void* a_customData, int customDataSize, int w, int h)
{
  if (currentRef.id < 0 || a_proc == nullptr || (a_customData == nullptr && customDataSize > 0))
  {
    HrPrint(HR_SEVERITY_WARNING, L"hrTexture2DUpdateFromProcLDR, invalid input");
    HRTextureNodeRef ref2; // dummy white texture
    ref2.id = 0;
    return ref2;
  }

  if ((w != -1) && (h != -1)) //user specified resolution - can create texture immediately
  {
    int bpp = 4;

    auto* imageData = new unsigned char[w * h * bpp];

    a_proc(imageData, w, h, a_customData);

    HRTextureNodeRef procTex = hrTexture2DUpdateFromMemory(currentRef, w, h, bpp, imageData);

    delete [] imageData;

    return procTex;
  }
  else
  {
    std::wstringstream outStr;
    outStr << L"texture2d_" << g_objManager.scnData.textures.size();

    HRTextureNode& texRes = g_objManager.scnData.textures.at(currentRef.id);
    texRes.name = outStr.str();
    texRes.id   = int32_t(g_objManager.scnData.textures.size());
    free(texRes.customData);
    texRes.customData = malloc(customDataSize);
    memcpy(texRes.customData, a_customData, size_t(customDataSize));
    texRes.ldrCallback = a_proc;
    texRes.customDataSize = customDataSize;

    return currentRef;
  }
}

HAPI HRTextureNodeRef  hrTexture2DUpdateFromProcHDR(HRTextureNodeRef currentRef, HR_TEXTURE2D_PROC_HDR_CALLBACK a_proc,
                                                    void* a_customData, int customDataSize, int w, int h)
{
  if (currentRef.id < 0 || a_proc == nullptr || (a_customData == nullptr && customDataSize > 0))
  {
    HrPrint(HR_SEVERITY_WARNING, L"hrTexture2DUpdateFromProcLDR, invalid input");
    HRTextureNodeRef ref2; // dummy white texture
    ref2.id = 0;
    return ref2;
  }

  if ((w != -1) && (h != -1)) //user specified resolution - can create texture immediately
  {
    int bpp = 4 * sizeof(float);

    auto* imageData = new float[w*h*bpp / sizeof(float)];

    a_proc(imageData, w, h, a_customData);

    HRTextureNodeRef procTex = hrTexture2DUpdateFromMemory(currentRef, w, h, bpp, imageData);

    delete[] imageData;

    return procTex;
  }
  else
  {
    std::wstringstream outStr;
    outStr << L"texture2d_" << g_objManager.scnData.textures.size();

    HRTextureNode& texRes = g_objManager.scnData.textures.at(currentRef.id);
    texRes.name = outStr.str();
    texRes.id   = int32_t(g_objManager.scnData.textures.size());
    free(texRes.customData);
    texRes.customData = malloc(customDataSize);
    memcpy(texRes.customData, a_customData, size_t(customDataSize));
    texRes.hdrCallback = a_proc;
    texRes.customDataSize = customDataSize;

    return currentRef;
  }

}


HAPI HRTextureNodeRef hrTextureCreateAdvanced(const wchar_t* a_texType, const wchar_t* a_objName)
{
	/////////////////////////////////////////////////////////////////////////////////////////////////
	HRTextureNode texRes;
	texRes.name = std::wstring(a_objName);
	texRes.id   = int32_t(g_objManager.scnData.textures.size());
	g_objManager.scnData.textures.push_back(texRes);

	HRTextureNodeRef ref;
	ref.id = HR_IDType(g_objManager.scnData.textures.size() - 1);

	HRTextureNode& texture = g_objManager.scnData.textures[ref.id];
	auto pTextureImpl = nullptr;
	texture.pImpl = pTextureImpl;

	pugi::xml_node texNodeXml = g_objManager.textures_lib_append_child();

	std::wstring id   = ToWString(ref.id);
	std::wstring type = a_texType;


	texNodeXml.append_attribute(L"id").set_value(id.c_str());
	texNodeXml.append_attribute(L"name").set_value(a_objName);
	texNodeXml.append_attribute(L"type").set_value(type.c_str());

	g_objManager.scnData.textures[ref.id].update_next(texNodeXml);

	return ref;
}

HAPI pugi::xml_node hrTextureParamNode(HRTextureNodeRef a_texRef)
{
	HRTextureNode* pTex = g_objManager.PtrById(a_texRef);
	if (pTex == nullptr)
	{
		HrError(L"hrTextureParamNode, nullptr input ");
		return pugi::xml_node();
	}

	if (!pTex->opened)
	{
    HrError(L"hrTextureParamNode, texture is not opened, texture id = ", pTex->id);
		return  pugi::xml_node();
	}

	return pTex->xml_node_next(pTex->openMode);
}

