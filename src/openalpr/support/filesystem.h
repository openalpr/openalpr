
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#ifdef WINDOWS
#include "windows/dirent.h"
#include "windows/utils.h"
#include "windows/unistd_partial.h"
typedef int mode_t;
#else
#include <dirent.h>
#include <unistd.h>
#endif

#include <stdlib.h>
#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <errno.h>

namespace alpr
{

  bool startsWith(std::string const &fullString, std::string const &prefix);
  bool hasEnding (std::string const &fullString, std::string const &ending);
  bool hasEndingInsensitive(const std::string& fullString, const std::string& ending);

  std::string filenameWithoutExtension(std::string filename);

  long getFileSize(std::string filename);
  long getFileCreationTime(std::string filename);

  bool DirectoryExists( const char* pzPath );
  bool fileExists( const char* pzPath );
  std::vector<std::string> getFilesInDir(const char* dirPath);

  bool stringCompare( const std::string &left, const std::string &right );

  bool makePath(const char* path, mode_t mode);
}

#endif // FILESYSTEM_H
