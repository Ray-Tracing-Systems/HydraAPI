#include "HydraAPI.h"
#include "HydraInternal.h"
#include "HydraInternalCommon.h"

#include <memory>
#include <vector>
#include <string>
#include <sstream>

#include <map>

#ifdef WIN32
  #include <direct.h>
#else

#endif


int hr_mkdir(const wchar_t* a_folder)
{
  return _wmkdir(a_folder);
}

void hr_cleardir(const wchar_t* a_folder)
{
  std::wstring tempFolder = std::wstring(a_folder) + L"/";
  std::wstring tempName = tempFolder + L"*";
  WIN32_FIND_DATAW fd;
  HANDLE hFind = ::FindFirstFileW(tempName.c_str(), &fd);
  if (hFind != INVALID_HANDLE_VALUE)
  {
    do
    {
      std::wstring tempName2 = tempFolder + fd.cFileName;
      DeleteFileW(tempName2.c_str());

    } while (::FindNextFileW(hFind, &fd));

    ::FindClose(hFind);
  }
}


std::vector<std::wstring> hr_listfiles(const wchar_t* a_folder)
{
	std::vector<std::wstring> result;
	std::wstring tempFolder = std::wstring(a_folder) + L"/";
	std::wstring tempName   = tempFolder + L"*";
	
	WIN32_FIND_DATAW fd;
	HANDLE hFind = ::FindFirstFileW(tempName.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			std::wstring tempName2 = tempFolder + fd.cFileName;
			result.push_back(tempName2);

		} while (::FindNextFileW(hFind, &fd));

		::FindClose(hFind);
	}

	return result;
}

void hr_copy_file(const wchar_t* a_file1, const wchar_t* a_file2)
{
  CopyFileW(a_file1, a_file2, FALSE);
}
