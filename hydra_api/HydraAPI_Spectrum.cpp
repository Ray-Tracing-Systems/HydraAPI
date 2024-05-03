#include "HydraAPI.h"
#include "HydraInternal.h"

#include <memory>
#include <vector>
#include <string>
#include <map>

#include <sstream>
#include <iomanip>
#include <filesystem>

#include "HydraObjectManager.h"

extern std::wstring      g_lastError;
extern std::wstring      g_lastErrorCallerPlace;
extern HRObjectManager   g_objManager;


HAPI HRSpectrumRef hrSpectrumCreateFromFile(const wchar_t* a_filePath)
{

  {
    auto p = g_objManager.scnData.m_spectraCache.find(a_filePath);
    if (p != g_objManager.scnData.m_spectraCache.end())
    {
      HRSpectrumRef ref;
      ref.id = p->second;
      return ref;
    }
  }

  HRSpectrumRef ref;
  ref.id = HR_IDType(g_objManager.scnData.spectra.size());

  std::filesystem::path p {a_filePath};

  HRSpectrum spec;
  spec.name = p.filename().wstring();
  spec.id = ref.id;
  g_objManager.scnData.spectra.push_back(spec);

  pugi::xml_node specNodeXml = g_objManager.spectra_lib_append_child();
	specNodeXml.append_attribute(L"id").set_value(ref.id);
  specNodeXml.append_attribute(L"name").set_value(spec.name.c_str());
  specNodeXml.append_attribute(L"type").set_value(L"spd");
  specNodeXml.append_attribute(L"path").set_value(p.wstring().c_str());

  std::filesystem::path loc {g_objManager.scnData.m_path};
  std::filesystem::path xmlLoc {"data"};
  xmlLoc.append(spec.name);

  loc.append(xmlLoc.string());

  hr_copy_file(a_filePath, loc.wstring().c_str());

  specNodeXml.force_attribute(L"loc").set_value(xmlLoc.wstring().c_str());

  g_objManager.scnData.spectra[ref.id].update(specNodeXml);

  return ref;
}

HAPI HRSpectrumRef hrSpectrumCreateFromTextures(const wchar_t* a_name, const HRTextureNodeRef* a_textures, const float* a_wavelengths, int a_size)
{
  HRSpectrumRef ref;
  ref.id = HR_IDType(g_objManager.scnData.spectra.size());

  std::wstring nameGenerated;
  if (a_name == nullptr) // create internal name for material
  {
    std::wstringstream strOut;
    strOut << L"mat#" << ref.id;
    nameGenerated = strOut.str();
    a_name = nameGenerated.c_str();
  }

  HRSpectrum spec;
  spec.name = a_name;
  spec.id = ref.id;
  g_objManager.scnData.spectra.push_back(spec);

  pugi::xml_node specNodeXml = g_objManager.spectra_lib_append_child();
	specNodeXml.append_attribute(L"id").set_value(ref.id);
  specNodeXml.append_attribute(L"name").set_value(spec.name.c_str());
  specNodeXml.append_attribute(L"type").set_value(L"ref");
  auto attr = specNodeXml.append_attribute(L"lambda_ref_ids");

  std::wstringstream ws;
  for(int i = 0; i < a_size; ++i)
  {
    HRTextureNode* pTex = g_objManager.PtrById(a_textures[i]);
    if (pTex == nullptr)
    {
      HrError(L"hrSpectrumCreateFromTextures: nullptr input texture ref");
      HRSpectrumRef ref;
      ref.id = -1;
      return ref;
    }
    ws << a_wavelengths[i] << L" " << a_textures[i].id << L" ";
  }

  g_objManager.scnData.spectra[ref.id].update(specNodeXml);

  return ref;
}