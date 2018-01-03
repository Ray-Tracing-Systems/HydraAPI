#include "input.h"

Input::Input()
{
  enableOpenGL1    = true;
  noWindow         = false;
  exitStatus       = false;

  inputLibraryPath = L"D:/PROG/HydraAPI/main/tests_f/test_201";
  inputRenderName  = L"opengl1";


  camMoveSpeed     = 7.5f;
  mouseSensitivity = 0.1f;

  // dynamic data
  //
  pathTracingEnabled = false;
  cameraFreeze       = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
\brief  Read bool from hash map. if no value was found (by key), don't overwrite 'pParam'
\param  a_params - hash map to read from
\param  a_name   - key
\param  pParam   - output parameter. if no value was found (by key), it does not touched.
\return found value or false if nothing was found

*/
static bool ReadBoolCmd(const std::unordered_map<std::string, std::string>& a_params, const std::string& a_name, bool* pParam = nullptr)
{
  const auto p = a_params.find(a_name);
  if (p != a_params.end())
  {
    const int val = atoi(p->second.c_str());
    const bool result = (val > 0);

    if (pParam != nullptr)
      (*pParam) = result;

    return result;
  }
  else
    return false;
}


/**
\brief  Read string from hash map. if no value was found (by key), don't overwrite 'pParam'
\param  a_params - hash map to read from
\param  a_name   - key
\param  pParam   - output parameter. if no value was found (by key), it does not touched.
\return found value or "" if nothing was found

*/
static std::string ReadStringCmd(const std::unordered_map<std::string, std::string>& a_params, const std::string& a_name, std::string* pParam = nullptr)
{
  const auto p = a_params.find(a_name);
  if (p != a_params.end())
  {
    if (pParam != nullptr)
      (*pParam) = p->second;

    return p->second;
  }
  else
    return std::string("");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Input::ParseCommandLineParams(const std::unordered_map<std::string, std::string>& a_params)
{
  ReadBoolCmd(a_params, "-nowindow", &noWindow);


}

