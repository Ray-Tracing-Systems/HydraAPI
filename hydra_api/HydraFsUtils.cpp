//
// Created by frol on 11/6/19.
//

#include "HydraAPI.h"
#include "HydraInternal.h"

std::wstring hr_fs::s2ws(const std::string& str)   { return s2ws(str); }
std::string  hr_fs::ws2s(const std::wstring& wstr) { return ws2s(wstr);}

int hr_fs::mkdir(const char* a_folder)    { return hr_mkdir(a_folder); }
int hr_fs::mkdir(const wchar_t* a_folder) { return hr_mkdir(a_folder); }

int hr_fs::cleardir(const char* a_folder)    { return hr_cleardir(a_folder); }
int hr_fs::cleardir(const wchar_t* a_folder) { return hr_cleardir(a_folder); }

void hr_fs::deletefile(const char* a_file)    { return hr_deletefile(a_file); }
void hr_fs::deletefile(const wchar_t* a_file) { return hr_deletefile(a_file); }

void hr_fs::copyfile(const char* a_file1,    const char* a_file2)    { return hr_copy_file(a_file1, a_file2); }
void hr_fs::copyfile(const wchar_t* a_file1, const wchar_t* a_file2) { return hr_copy_file(a_file1, a_file2); }

std::vector<std::string>  hr_fs::listfiles(const char* a_folder,    bool excludeFolders) { return hr_listfiles(a_folder, excludeFolders); }
std::vector<std::wstring> hr_fs::listfiles(const wchar_t* a_folder, bool excludeFolders) { return hr_listfiles(a_folder, excludeFolders); }
