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


std::vector<std::wstring> hr_listfiles(const wchar_t* a_folder, bool excludeFolders = true)
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

std::vector<std::string> hr_listfiles(const char* a_folder, bool excludeFolders = true)
{
  std::vector<std::string> result;
  std::string tempFolder = std::string(a_folder) + "/";
  std::string tempName   = tempFolder + "*";

  WIN32_FIND_DATAA fd;
  HANDLE hFind = ::FindFirstFileA(tempName.c_str(), &fd);
  if (hFind != INVALID_HANDLE_VALUE)
  {
    do
    {
      std::string tempName2 = tempFolder + fd.cFileName;
      result.push_back(tempName2);

    } while (::FindNextFileA(hFind, &fd));

    ::FindClose(hFind);
  }

  return result;
}

void hr_copy_file(const wchar_t* a_file1, const wchar_t* a_file2)
{
  CopyFileW(a_file1, a_file2, FALSE);
}

void hr_deletefile(const wchar_t* a_file)
{
  DeleteFileW(a_file);
}

void hr_deletefile(const char* a_file)
{
  DeleteFileA(a_file);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct HRSystemMutex
{
  HANDLE      mutex;
  std::string name;
  bool        owner;
};

HRSystemMutex* hr_create_system_mutex(const char* a_mutexName)
{
  HRSystemMutex* a_mutex = new HRSystemMutex;

  a_mutex->mutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, a_mutexName);
  if (a_mutex->mutex == NULL || a_mutex->mutex == INVALID_HANDLE_VALUE)
    a_mutex->mutex = CreateMutexA(NULL, FALSE, a_mutexName);

  return a_mutex;
}

void hr_free_system_mutex(HRSystemMutex*& a_mutex) // logic of this function is not strictly correct, but its ok for our usage case.
{
  if(a_mutex == nullptr)
    return;
  
  if (a_mutex->mutex != INVALID_HANDLE_VALUE && a_mutex->mutex != NULL)
  {
    CloseHandle(a_mutex->mutex);
    a_mutex->mutex = NULL;
  }

  delete a_mutex;
  a_mutex = nullptr;
}

bool hr_lock_system_mutex(HRSystemMutex* a_mutex, int a_msToWait)
{
  if (a_mutex == nullptr)
    return false;

  const DWORD res = WaitForSingleObject(a_mutex->mutex, a_msToWait);

  if (res == WAIT_TIMEOUT || res == WAIT_FAILED)
    return false;
  else
    return true;
}

void hr_unlock_system_mutex(HRSystemMutex* a_mutex)
{
  if (a_mutex == nullptr)
    return;

  ReleaseMutex(a_mutex->mutex);
}

#include <fstream>

void hr_ifstream_open(std::ifstream& a_stream, const wchar_t* a_fileName)
{
  a_stream.open(a_fileName, std::ios::binary);
}

void hr_ofstream_open(std::ofstream& a_stream, const wchar_t* a_fileName)
{
  a_stream.open(a_fileName, std::ios::binary);
}