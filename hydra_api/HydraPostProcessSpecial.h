#pragma once

#include "HydraPostProcessAPI.h"
#include "../hydra_api/HR_HDRImage.h"
#include "../hydra_api/HR_HDRImageTool.h"

#include "HydraRenderDriverAPI.h"

#include <unordered_map>
#include <memory>

/**
\brief interface for special post processing filters.

This interface CAN NOT BE USED FOR DLL PLUGINS!!!

*/
class IFilter2DSpecial
{
public:

  IFilter2DSpecial()          = default;
  virtual ~IFilter2DSpecial() = default;

  IFilter2DSpecial(IFilter2DSpecial&& a_rhs)      = delete;
  IFilter2DSpecial(const IFilter2DSpecial& a_rhs) = delete;

  IFilter2DSpecial& operator=(IFilter2DSpecial&& a_rhs)      = delete;
  IFilter2DSpecial& operator=(const IFilter2DSpecial& a_rhs) = delete;

  using ArgArray1 = std::unordered_map<std::wstring, std::shared_ptr< HydraRender::HDRImage4f> >;
  using ArgArray2 = std::unordered_map<std::wstring, std::shared_ptr< HydraRender::LDRImage1i> >;

  virtual bool Eval(ArgArray1& argsHDR, ArgArray2& argsLDR, pugi::xml_node settings, std::shared_ptr<IHRRenderDriver> a_pDriver) = 0;   // to implement in subclass;

  virtual const wchar_t* GetLastError() const { return m_err.c_str(); }

protected:

  std::wstring m_err;

};


std::shared_ptr<IFilter2DSpecial> CreateSpecialFilter(const wchar_t* a_filterName);
