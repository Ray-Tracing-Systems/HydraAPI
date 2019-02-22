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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HAPI bool hrRenderLockFrameBufferUpdate(HRRenderRef a_pRender)
{
  HRRender* pRender = g_objManager.PtrById(a_pRender);
  
  if (pRender == nullptr)
  {
    HrError(L"hrRenderLockFrameBufferUpdate, nullptr Render Driver ");
    return false;
  }
  
  if (pRender->m_pDriver == nullptr)
  {
    HrError(L"hrRenderLockFrameBufferUpdate, nullptr Render Driver impl ");
    return false;
  }
  
  pRender->m_pDriver->LockFrameBufferUpdate();

  return true;
}


HAPI void hrRenderUnlockFrameBufferUpdate(HRRenderRef a_pRender)
{
  HRRender* pRender = g_objManager.PtrById(a_pRender);
  
  if (pRender == nullptr)
  {
    HrError(L"hrRenderUnlockFrameBufferUpdate, nullptr Render Driver ");
    return;
  }
  
  if (pRender->m_pDriver == nullptr)
  {
    HrError(L"hrRenderUnlockFrameBufferUpdate, nullptr Render Driver impl ");
    return;
  }
  
  pRender->m_pDriver->UnlockFrameBufferUpdate();
}

HAPI bool hrRenderGetFrameBufferHDR4f(HRRenderRef a_pRender, int w, int h, float* imgData, const wchar_t* a_layerName) // (w,h) is strongly related to viewport size; return true if image was final
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
  
  pRender->m_pDriver->LockFrameBufferUpdate();
  pRender->m_pDriver->GetFrameBufferHDR(w, h, imgData, a_layerName);
  pRender->m_pDriver->UnlockFrameBufferUpdate();
  
  return true;
}


HAPI bool hrRenderGetFrameBufferLDR1i(HRRenderRef a_pRender, int w, int h, int32_t* imgData)
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
  
  pRender->m_pDriver->LockFrameBufferUpdate();
  pRender->m_pDriver->GetFrameBufferLDR(w, h, imgData);
  pRender->m_pDriver->UnlockFrameBufferUpdate();
  
  return true;
}

HAPI bool hrRenderGetFrameBufferLineHDR4f(HRRenderRef a_pRender, int a_begin, int a_end, int a_y, float* imgData, const wchar_t* a_layerName)
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

HAPI bool hrRenderGetFrameBufferLineLDR1i(HRRenderRef a_pRender, int a_begin, int a_end, int a_y, int32_t* imgData)
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

HAPI bool hrRenderSaveFrameBufferLDR(HRRenderRef a_pRender, const wchar_t* a_outFileName)
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

  auto pImgTool = g_objManager.m_pImgTool;
  auto& imgData = g_objManager.m_tempBuffer;
  if (imgData.size() < w*h)
    imgData.resize(w*h);

  pRender->m_pDriver->GetFrameBufferLDR(w, h, imgData.data());
  pImgTool->SaveLDRImageToFileLDR(a_outFileName, w, h, imgData.data());

  if (imgData.size() > TEMP_BUFFER_MAX_SIZE_DONT_FREE) // free temp buffer if it's too large
    imgData = g_objManager.EmptyBuffer();

  return true;
}

HAPI bool hrRenderSaveFrameBufferHDR(HRRenderRef a_pRender, const wchar_t* a_outFileName)
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

  const int w = node.child(L"width").text().as_int();
  const int h = node.child(L"height").text().as_int();

  if (w <= 0 || h <= 0)
  {
    HrError(L"hrRenderSaveFrameBufferHDR, <width> or <height> xml param of render was not set");
    return false;
  }

  auto pImgTool = g_objManager.m_pImgTool;
  auto& imgData = g_objManager.m_tempBuffer;
  if (imgData.size() < size_t(w*h)*size_t(4))
    imgData.resize(size_t(w*h)*size_t(4));

  pRender->m_pDriver->GetFrameBufferHDR(w, h, (float*)imgData.data(), L"color");
  pImgTool->SaveHDRImageToFileHDR(a_outFileName, w, h, (const float*)imgData.data());

  if (imgData.size() > TEMP_BUFFER_MAX_SIZE_DONT_FREE) // free temp buffer if it's too large
    imgData = g_objManager.EmptyBuffer();

  return true;
}

HAPI void hrRenderCommand(HRRenderRef a_pRender, const wchar_t* a_command, wchar_t* a_answer)
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

HAPI void hrRenderLogDir(HRRenderRef a_pRender, const wchar_t* a_logDir, bool a_hrRenderLogDir)
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

  // fix directory path if it don't end with '\' or '/'
  //
  std::wstring copyOfPath(a_logDir);
  if (copyOfPath != L"")
  {
    if (copyOfPath[copyOfPath.size() - 1] != L"/"[0] &&
      copyOfPath[copyOfPath.size() - 1] != L"\\"[0])
      copyOfPath += L"/";
  }
  pRender->m_pDriver->SetLogDir(copyOfPath.c_str(), a_hrRenderLogDir);

}
