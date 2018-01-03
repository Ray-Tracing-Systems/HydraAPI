#include "HydraPostProcessCommon.h"

#pragma warning(disable:4996)

float* IFilter2D::GetInputByName(const wchar_t* a_name, int* pW, int* pH, int* pBPP)
{
  const std::wstring inName(a_name);

  if (inName.find(L"in_") == std::wstring::npos)     // check for "in_" prefix in arg name
  {
    wcsncpy(m_msg, L"IFilter2D: in arg bad name ", ERR_MSG_SIZE);
    wcsncat(m_msg, a_name, ERR_MSG_SIZE);
    return nullptr;
  }

  for (int i = 0; i < m_input.argsNum; i++)
  {
    if (inName == m_input.names[i])
    {
      (*pW)   = m_input.width [i];
      (*pH)   = m_input.height[i];
      (*pBPP) = m_input.bpp   [i];
      return m_input.datas[i];
    }
  }

  wcsncpy(m_msg, L"IFilter2D: in arg not found: ", ERR_MSG_SIZE);
  wcsncat(m_msg, a_name, ERR_MSG_SIZE);
  return nullptr;
}

float* IFilter2D::GetOutputByName(const wchar_t* a_name, int* pW, int* pH, int* pBPP)
{
  const std::wstring inName(a_name);

  if (inName.find(L"out_") == std::wstring::npos)      // check for "out_" prefix in arg name
  {
    wcsncpy(m_msg, L"IFilter2D: out arg bad name: ",  ERR_MSG_SIZE);
    wcsncat(m_msg, a_name, ERR_MSG_SIZE);
    return nullptr;
  }
  for (int i = 0; i < m_input.argsNum; i++)
  {
    if (inName == m_input.names[i])
    {
      (*pW)   = m_input.width[i];
      (*pH)   = m_input.height[i];
      (*pBPP) = m_input.bpp[i];
      return m_input.datas[i];
    }
  }

  wcsncpy(m_msg, L"IFilter2D: out arg not found: ", ERR_MSG_SIZE);
  wcsncat(m_msg, a_name, ERR_MSG_SIZE);

  return nullptr;
}



