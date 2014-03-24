
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#ifdef WINDOWS
#include "windows/dirent.h"
#include "windows/utils.h"
#include "windows/unistd_partial.h"
#else
#include <dirent.h>
#include <unistd.h>
#endif

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <vector>

bool hasEnding (std::string const &fullString, std::string const &ending);
bool DirectoryExists( const char* pzPath );
bool fileExists( const char* pzPath );
std::vector<std::string> getFilesInDir(const char* dirPath);

bool stringCompare( const std::string &left, const std::string &right );

#endif // FILESYSTEM_H
