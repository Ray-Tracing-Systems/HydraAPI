//
// Created by hikawa on 02.07.17.
//

#include "HydraAPI.h"
#include "HydraInternal.h"
#include "HydraInternalCommon.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <ftw.h>
#include <unistd.h>


#include <map>
#include <dirent.h>


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
  DIR *dir;
  class dirent *ent;
  class stat st;

  dir = opendir(a_folder.c_str());
  while ((ent = readdir(dir)) != NULL) {
    const std::string file_name = ent->d_name;
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
