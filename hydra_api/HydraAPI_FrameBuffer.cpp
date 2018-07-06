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

extern std::wstring      g_lastError;
extern std::wstring      g_lastErrorCallerPlace;
extern HR_ERROR_CALLBACK g_pErrorCallback;
extern HRObjectManager   g_objManager;


#include <FreeImage.h>
#include <cmath>

bool HR_SaveHDRImageToFileHDR(const wchar_t* a_fileName, int w, int h, const float* a_data, const float a_scale = 1.0f);
bool HR_SaveLDRImageToFile(const wchar_t* a_fileName, int w, int h, const int32_t* data);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HAPI bool hrRenderGetFrameBufferHDR4f(const HRRenderRef a_pRender, int w, int h, float* imgData, const wchar_t* a_layerName) // (w,h) is strongly related to viewport size; return true if image was final  
{
  HRRender* pRender = g_objManager.PtrById(a_pRender);

  if (pRender == nullptr)
  {
    HrError(L"hrRenderGetFrameBufferHDR4f, nullptr Render Driver ");
    return false;
  }

  if (pRender->m_pDriver == nullptr)
  {
    HrError(L"hrRenderGetFrameBufferHDR4f, nullptr Render Driver impl ");
    return false;
  }

  pRender->m_pDriver->GetFrameBufferHDR(w, h, imgData, a_layerName);

  return true;
}


HAPI bool hrRenderGetFrameBufferLDR1i(const HRRenderRef a_pRender, int w, int h, int32_t* imgData)
{
  HRRender* pRender = g_objManager.PtrById(a_pRender);

  if (pRender == nullptr)
  {
    HrError(L"hrRenderGetFrameBufferLDR1i, nullptr Render Driver ");
    return false;
  }

  if (pRender->m_pDriver == nullptr)
  {
    HrError(L"hrRenderGetFrameBufferLDR1i, nullptr Render Driver impl ");
    return false;
  }

  pRender->m_pDriver->GetFrameBufferLDR(w, h, imgData);

  return true;
}

HAPI bool hrRenderGetFrameBufferLineHDR4f(const HRRenderRef a_pRender, int a_begin, int a_end, int a_y, float* imgData, const wchar_t* a_layerName)
{
  HRRender* pRender = g_objManager.PtrById(a_pRender);

  if (pRender == nullptr)
  {
    HrError(L"hrRenderGetFrameBufferLineHDR4f, nullptr Render Driver ");
    return false;
  }

  if (pRender->m_pDriver == nullptr)
  {
    HrError(L"hrRenderGetFrameBufferLineHDR4f, nullptr Render Driver impl ");
    return false;
  }

  if (!pRender->m_pDriver->Info().supportGetFrameBufferLine)
  {
    HrError(L"hrRenderGetFrameBufferLineHDR4f is not implemented for general case yet. try different render driver");
    return false;
  }

  pRender->m_pDriver->GetFrameBufferLineHDR(a_begin, a_end, a_y, imgData, a_layerName);
  return true;
}

HAPI bool hrRenderGetFrameBufferLineLDR1i(const HRRenderRef a_pRender, int a_begin, int a_end, int a_y, int32_t* imgData)
{
  HRRender* pRender = g_objManager.PtrById(a_pRender);
  if (pRender == nullptr)
  {
    HrError(L"hrRenderGetFrameBufferLineLDR1i, nullptr Render Driver ");
    return false;
  }

  if (pRender->m_pDriver == nullptr)
  {
    HrError(L"hrRenderGetFrameBufferLineLDR1i, nullptr Render Driver impl ");
    return false;
  }

  if (!pRender->m_pDriver->Info().supportGetFrameBufferLine)
  {
    HrError(L"GetFrameBufferLineLDR1i is not implemented for general case yet. try different render driver");
    return false;
  }

  pRender->m_pDriver->GetFrameBufferLineLDR(a_begin, a_end, a_y, imgData);
  return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HAPI bool hrRenderSaveFrameBufferLDR(const HRRenderRef a_pRender, const wchar_t* a_outFileName)
{
  HRRender* pRender = g_objManager.PtrById(a_pRender);

  if (pRender == nullptr)
  {
    HrError(L"hrRenderSaveFrameBufferLDR, nullptr Render Driver ");
    return false;
  }

  if (pRender->m_pDriver == nullptr)
  {
    HrError(L"hrRenderSaveFrameBufferLDR, nullptr Render Driver impl ");
    return false;
  }

  pugi::xml_node node = pRender->xml_node_immediate();

  int w = node.child(L"width").text().as_int();
  int h = node.child(L"height").text().as_int();

  if (w <= 0 || h <= 0)
  {
    HrError(L"hrRenderSaveFrameBufferLDR, <width> or <height> xml param of render was not set");
    return false;
  }

  std::vector<int32_t> imgData(w*h);
  pRender->m_pDriver->GetFrameBufferLDR(w, h, &imgData[0]);

  //HR_MyDebugSaveBMP(a_outFileName, &imgData[0], w, h);
  //return true;

  return HR_SaveLDRImageToFile(a_outFileName, w, h, &imgData[0]);
}

HAPI bool hrRenderSaveFrameBufferHDR(const HRRenderRef a_pRender, const wchar_t* a_outFileName)
{
  HRRender* pRender = g_objManager.PtrById(a_pRender);

  if (pRender == nullptr)
  {
    HrError(L"hrRenderSaveFrameBufferHDR, nullptr Render Driver ");
    return false;
  }

  if (pRender->m_pDriver == nullptr)
  {
    HrError(L"hrRenderSaveFrameBufferHDR, nullptr Render Driver impl ");
    return false;
  }

  pugi::xml_node node = pRender->xml_node_immediate();

  int w = node.child(L"width").text().as_int();
  int h = node.child(L"height").text().as_int();

  if (w <= 0 || h <= 0)
  {
    HrError(L"hrRenderSaveFrameBufferHDR, <width> or <height> xml param of render was not set");
    return false;
  }

  std::vector<float> imgData(w*h*4);
  pRender->m_pDriver->GetFrameBufferHDR(w, h, &imgData[0], L"color");

  return HR_SaveHDRImageToFileHDR(a_outFileName, w, h, &imgData[0]);
}

HAPI void hrRenderCommand(const HRRenderRef a_pRender, const wchar_t* a_command, wchar_t* a_answer)
{
  HRRender* pRender = g_objManager.PtrById(a_pRender);
  if (pRender == nullptr)
  {
    HrError(L"[hrRenderCommand]: nullptr Render Driver ");
    return;
  }

  if (pRender->m_pDriver == nullptr)
  {
    HrError(L"[hrRenderCommand]: nullptr Render Driver impl ");
    return;
  }
  
  std::wstring command(a_command);
  if(command == L"start" && g_objManager.scnData.m_fileState!= L"") // just 'start', without additional arguments
  {
    command += std::wstring(L" -statefile ") + g_objManager.scnData.m_fileState;
    a_command = command.c_str();
  }
  
  pRender->m_pDriver->ExecuteCommand(a_command, a_answer);

}

HAPI void hrRenderLogDir(const HRRenderRef a_pRender, const wchar_t* a_logDir, bool a_hrRenderLogDir)
{
  HRRender* pRender = g_objManager.PtrById(a_pRender);
  if (pRender == nullptr)
  {
    HrError(L"[hrRenderLogDir]: nullptr Render Driver ");
    return;
  }

  if (pRender->m_pDriver == nullptr)
  {
    HrError(L"[hrRenderLogDir]: nullptr Render Driver impl ");
    return;
  }

  pRender->m_pDriver->SetLogDir(a_logDir, a_hrRenderLogDir);

}
