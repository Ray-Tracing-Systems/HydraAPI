#pragma once

#include <string>
#include <unordered_map>

struct Input
{
  Input();

  // fixed data
  //
  bool noWindow;
  bool exitStatus;
  bool enableOpenGL1;

  std::wstring inputLibraryPath;
  std::wstring inputRenderName;

  // mouse and keyboad/oher gui input
  //
  float camMoveSpeed;
  float mouseSensitivity;

  // dynamic data
  //

  bool pathTracingEnabled;
  bool cameraFreeze;


  void ParseCommandLineParams(const std::unordered_map<std::string, std::string>& a_params);
};
