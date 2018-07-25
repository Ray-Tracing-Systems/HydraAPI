#pragma once

#include "HydraPostProcessAPI.h"
constexpr int FILTER_MAX_ARGS = 8;

/**
\brief Input for 2D image filters. All inputs must be read by their names.  

*/
struct Filter2DInput
{
  const wchar_t*   names [FILTER_MAX_ARGS]; ///< arg name;
  float*           datas [FILTER_MAX_ARGS]; ///< arg data pointer; if bpp[i] != 16 you should cast datas[i] to int* (int1 instead of float4 !)
  int              width [FILTER_MAX_ARGS]; ///< arg image width
  int              height[FILTER_MAX_ARGS]; ///< arg image height
  int              bpp   [FILTER_MAX_ARGS]; ///< arg image bpp;    if bpp[i] != 16 you should cast datas[i] to int* (int1 instead of float4 !)
  int              argsNum;                 ///< arguments number
  const wchar_t*   settingsXmlStr;          ///< wstring that contain xml with filter settings
};

/**
\brief General interface for implementing post processing filters. 
       
This interface is the only way to implement custom filter via DLL plugin.

*/
class IFilter2D
{
public:

  IFilter2D() {  memset(m_msg, 0, sizeof(m_msg)); }
  virtual ~IFilter2D() = default;

  IFilter2D(IFilter2D&& a_rhs)      = delete;
  IFilter2D(const IFilter2D& a_rhs) = delete;

  IFilter2D& operator=(IFilter2D&& a_rhs)      = delete;
  IFilter2D& operator=(const IFilter2D& a_rhs) = delete;

  virtual void Release() = 0;   // COM like destructor; to implement is subclass;
  virtual bool Eval()    = 0;   // to implement is subclass;

  virtual void SetInput(Filter2DInput a_input) { m_input = a_input; }
  virtual const wchar_t* GetLastError() const  { return m_msg; }
  virtual const bool HasWarning() const { return m_hasWarning; }

  enum { ERR_MSG_SIZE = 1024};

protected:

  virtual float* GetInputByName (const wchar_t* a_name, int* pW, int* pH, int* pBPP);
  virtual float* GetOutputByName(const wchar_t* a_name, int* pW, int* pH, int* pBPP);

  const wchar_t* GetSettingsStr() const { return m_input.settingsXmlStr; }

  bool m_hasWarning = false;
  wchar_t m_msg[ERR_MSG_SIZE];

private:

  Filter2DInput m_input;
};

typedef IFilter2D* (*PCREATEFUN_T)(const wchar_t* a_name);
