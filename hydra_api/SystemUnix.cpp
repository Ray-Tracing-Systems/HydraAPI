//
// Created by hikawa on 02.07.17.
//

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
#include <experimental/filesystem>
#include <iostream>

namespace std_fs = std::experimental::filesystem;

int hr_mkdir(const char* a_folder)
{
  return mkdir(a_folder, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
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


std::vector<std::string> hr_listfiles(const std::string &a_folder)
{
  std::vector<std::string> result;
  class dirent *ent = nullptr;
  class stat st;
  
  DIR* dir = opendir(a_folder.c_str());
  if(dir == nullptr)
  {
    // std::cout << "[debug]: hr_listfiles, can't open DIR " << a_folder.c_str() << std::endl;
    // std::cout << "[debug]: errno = " << strerror(errno) << std::endl;
    return result;
  }
  
  while ((ent = readdir(dir)) != NULL)
  {
    const std::string file_name      = ent->d_name;
    const std::string full_file_name = a_folder + "/" + file_name;
    
    if (file_name[0] == '.')
      continue;

    if (stat(full_file_name.c_str(), &st) == -1)
      continue;
    
    const bool is_directory = (st.st_mode & S_IFDIR) != 0;
    
    if (is_directory)
      continue;
    
    result.push_back(full_file_name);
  }
  closedir(dir);

  return result;
}

void hr_copy_file(const wchar_t* a_file1, const wchar_t* a_file2)
{
/*  int source = open(a_file1, O_RDONLY, 0);
  int dest = open(a_file2, O_WRONLY | O_CREAT | O_TRUNC, 0644);

  struct stat stat_source;
  fstat(source, &stat_source);

  sendfile(dest, source, 0, stat_source.st_size);

  close(source);
  close(dest);*/

  std_fs::copy_file(a_file1, a_file2, std_fs::copy_options::overwrite_existing);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct HRSystemMutex
{
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
  
  if (a_mutex->mutex == NULL || a_mutex->mutex == SEM_FAILED)
  {
    a_mutex->mutex = sem_open(a_mutexName, O_CREAT | O_EXCL, 0775, 1); //0775  | O_EXCL
    a_mutex->owner = true;
  }
  
  if (a_mutex->mutex == NULL)
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
  if(a_mutex->owner)
    sem_unlink(a_mutex->name.c_str());
  
  delete a_mutex;
  a_mutex = nullptr;
}

bool hr_lock_system_mutex(HRSystemMutex* a_mutex, int a_msToWait)
{
  struct timespec ts;
  ts.tv_sec  = a_msToWait / 1000;
  ts.tv_nsec = a_msToWait * 1'000'000 - ts.tv_sec * 1'000'000'000;
  
  int res = sem_timedwait(a_mutex->mutex, &ts);
  
  if(res == -1)
    return false;
  else
    return true;
}

void hr_unlock_system_mutex(HRSystemMutex* a_mutex)
{
  sem_post(a_mutex->mutex);
}