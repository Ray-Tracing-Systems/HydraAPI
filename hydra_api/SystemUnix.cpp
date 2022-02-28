#include "HydraAPI.h"
#include "HydraInternal.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <cstdio>
#include <ftw.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <ctime>

#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>

#include <map>
#include <filesystem>
#include <iostream>


int hr_mkdir(const char* a_folder)
{
  return mkdir(a_folder, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

int hr_mkdir(const wchar_t * a_folder)
{
  const std::string data = ws2s(std::wstring(a_folder));
  return mkdir(data.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
  int rv;

  if (ftwbuf->level == 0)
    return 0;

  rv = remove(fpath);

  if (rv)
    perror(fpath);

  return rv;
}



int hr_cleardir(const char* a_folder)
{
  return nftw(a_folder, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

int  hr_cleardir(const wchar_t* a_folder)
{
  const std::string data = ws2s(std::wstring(a_folder));
  return hr_cleardir(data.c_str());
}

void hr_deletefile(const wchar_t* a_file)
{
  const std::string file = ws2s(a_file);
  remove(file.c_str());
}

void hr_deletefile(const char* a_file)
{
  remove(a_file);
}


std::vector<std::string> hr_listfiles(const char* a_folder, bool excludeFolders)
{
  std::vector<std::string> result;
  class dirent *ent = nullptr;
  class stat st;
  
  DIR* dir = opendir(a_folder);
  if(dir == nullptr)
  {
    // std::cout << "[debug]: hr_listfiles, can't open DIR " << a_folder.c_str() << std::endl;
    // std::cout << "[debug]: errno = " << strerror(errno) << std::endl;
    return result;
  }
  
  while ((ent = readdir(dir)) != nullptr)
  {
    const std::string file_name      = ent->d_name;
    const std::string full_file_name = std::string(a_folder) + "/" + file_name;
    
    if (file_name[0] == '.')
      continue;

    if (stat(full_file_name.c_str(), &st) == -1)
      continue;
    
    const bool is_directory = (st.st_mode & S_IFDIR) != 0 && excludeFolders;
    
    if (is_directory)
      continue;
    
    result.push_back(full_file_name);
  }
  closedir(dir);

  return result;
}

std::vector<std::wstring> hr_listfiles(const wchar_t* a_folder2, bool excludeFolders)
{
  
  const std::string a_folder = ws2s(a_folder2);
  
  std::vector<std::wstring> result;
  class dirent *ent = nullptr;
  class stat st;
  
  DIR* dir = opendir(a_folder.c_str());
  if(dir == nullptr)
  {
    // std::cout << "[debug]: hr_listfiles, can't open DIR " << a_folder.c_str() << std::endl;
    // std::cout << "[debug]: errno = " << strerror(errno) << std::endl;
    return result;
  }
  
  while ((ent = readdir(dir)) != nullptr)
  {
    const std::string file_name      = ent->d_name;
    const std::string full_file_name = a_folder + "/" + file_name;
    
    if (file_name[0] == '.')
      continue;
    
    if (stat(full_file_name.c_str(), &st) == -1)
      continue;
    
    const bool is_directory = (st.st_mode & S_IFDIR) != 0 && excludeFolders;
    
    if (is_directory)
      continue;
    
    result.push_back(s2ws(full_file_name));
  }
  closedir(dir);
  
  return result;
}


void hr_copy_file(const char* a_file1, const char* a_file2)
{
  std::filesystem::copy_file(a_file1, a_file2, std::filesystem::copy_options::overwrite_existing);
}

void hr_copy_file(const wchar_t* a_file1, const wchar_t* a_file2)
{
  std::filesystem::copy_file(a_file1, a_file2, std::filesystem::copy_options::overwrite_existing);
}

void hr_delete_file(const wchar_t* a_file)
{
  std::filesystem::remove(a_file);
}

void hr_delete_file(const char* a_file)
{
  std::filesystem::remove(a_file);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct HRSystemMutex
{
  HRSystemMutex() : mutex(nullptr), name(""), owner(false) {}
  sem_t*      mutex;
  std::string name;
  bool        owner;
};

HRSystemMutex* hr_create_system_mutex(const char* a_mutexName)
{
  HRSystemMutex* a_mutex = new HRSystemMutex;
  
  a_mutex->name  = a_mutexName;
  a_mutex->mutex = sem_open(a_mutexName, 0);
  a_mutex->owner = false;
  
  if (a_mutex->mutex == nullptr || a_mutex->mutex == SEM_FAILED)
  {
    a_mutex->mutex = sem_open(a_mutexName, O_CREAT | O_EXCL, 0775, 1); //0775  | O_EXCL
    a_mutex->owner = true;
  }
  
  if (a_mutex->mutex == nullptr)
  {
    std::cerr << "hr_create_system_mutex (a_mutex): FAILED to create mutex (shared_mutex_init), name = " << a_mutexName << std::endl;
    return nullptr;
  }
  
  return a_mutex;
}

void hr_free_system_mutex(HRSystemMutex*& a_mutex) // logic of this function is not strictly correct, but its ok for our usage case.
{
  if(a_mutex == nullptr)
    return;
  
  sem_close(a_mutex->mutex);
  //if(a_mutex->owner)
  sem_unlink(a_mutex->name.c_str());
  
  delete a_mutex;
  a_mutex = nullptr;
}

bool hr_lock_system_mutex(HRSystemMutex* a_mutex, int a_msToWait)
{
  timespec ts = {0,0};
  ts.tv_sec  = a_msToWait / 1000;
  ts.tv_nsec = a_msToWait * 1'000'000 - ts.tv_sec * 1'000'000'000;
  
  int res = sem_timedwait(a_mutex->mutex, &ts);
  return (res != -1);
}

void hr_unlock_system_mutex(HRSystemMutex* a_mutex)
{
  sem_post(a_mutex->mutex);
}

#include <fstream>

void hr_ifstream_open(std::ifstream& a_stream, const wchar_t* a_fileName)
{
  std::wstring s1(a_fileName);
  std::string  s2(s1.begin(), s1.end());
  a_stream.open(s2.c_str(), std::ios::binary);
}

void hr_ofstream_open(std::ofstream& a_stream, const wchar_t* a_fileName)
{
  std::wstring s1(a_fileName);
  std::string  s2(s1.begin(), s1.end());
  a_stream.open(s2.c_str(), std::ios::binary);
}