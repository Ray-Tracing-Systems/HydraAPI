#include "HydraAPI.h"
#include "HydraInternal.h"

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


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct HRSystemMutex
{
  std::string name;
  bool        owner;
};

HRSystemMutex* hr_create_system_mutex(const char* a_mutexName)
{
  return nullptr;
}

void hr_free_system_mutex(HRSystemMutex*& a_mutex) // logic of this function is not strictly correct, but its ok for our usage case.
{
  if(a_mutex == nullptr)
    return;
  
  //todo: implement this
  
  delete a_mutex;
  a_mutex = nullptr;
}

bool hr_lock_system_mutex(HRSystemMutex* a_mutex, int a_msToWait)
{
  //todo: implement this
  return false;
}

void hr_unlock_system_mutex(HRSystemMutex* a_mutex)
{
  //todo: implement this
}
