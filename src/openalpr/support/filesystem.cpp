#include "filesystem.h"

#include <algorithm>
#include <errno.h>
#include <fstream>
#include <string.h>



namespace alpr
{

  bool startsWith(std::string const &fullString, std::string const &prefix)
  {
    if(fullString.substr(0, prefix.size()).compare(prefix) == 0) {
        return true;
    }

    return false;
  }

  bool hasEnding (std::string const &fullString, std::string const &ending)
  {
    if (fullString.length() >= ending.length())
    {
      return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    }
    else
    {
      return false;
    }
  }





  bool hasEndingInsensitive(const std::string& fullString, const std::string& ending)
  {
    if (fullString.length() < ending.length())
      return false;

    int startidx = fullString.length() - ending.length();

    for (unsigned int i = startidx; i < fullString.length(); ++i)
        if (tolower(fullString[i]) != tolower(ending[i - startidx]))
            return false;
    return true;
  }

  bool DirectoryExists( const char* pzPath )
  {
    if ( pzPath == NULL) return false;

    DIR *pDir;
    bool bExists = false;

    pDir = opendir (pzPath);

    if (pDir != NULL)
    {
      bExists = true;
      (void) closedir (pDir);
    }

    return bExists;
  }

  bool fileExists( const char* pzPath )
  {
    if (pzPath == NULL) return false;

    bool fExists = false;
    std::ifstream f(pzPath);
    fExists = f.is_open();
    f.close();
    return fExists;
  }

  std::vector<std::string> getFilesInDir(const char* dirPath)
  {
    DIR *dir;

    std::vector<std::string> files;

    struct dirent *ent;
    if ((dir = opendir (dirPath)) != NULL)
    {
      /* print all the files and directories within directory */
      while ((ent = readdir (dir)) != NULL)
      {
        if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
          files.push_back(ent->d_name);
      }
      closedir (dir);
    }
    else
    {
      /* could not open directory */
      perror ("");
      return files;
    }

    return files;
  }

  bool stringCompare( const std::string &left, const std::string &right )
  {
    for( std::string::const_iterator lit = left.begin(), rit = right.begin(); lit != left.end() && rit != right.end(); ++lit, ++rit )
      if( tolower( *lit ) < tolower( *rit ) )
        return true;
      else if( tolower( *lit ) > tolower( *rit ) )
        return false;
    if( left.size() < right.size() )
      return true;
    return false;
  }

  std::string filenameWithoutExtension(std::string filename)
  {
    int lastslash = filename.find_last_of("/");
    if (lastslash >= filename.size())
      lastslash = 0;
    else
      lastslash += 1;

    int lastindex = filename.find_last_of(".");

    return filename.substr(lastslash, lastindex - lastslash);
  }

  std::string get_filename_from_path(std::string file_path)
  {

    size_t found;
    found=file_path.find_last_of("/\\");

    if (found != std::string::npos)
      return file_path.substr(found+1);

    return "";

  }

  std::string get_directory_from_path(std::string file_path)
  {
    if (DirectoryExists(file_path.c_str()))
      return file_path;

    size_t found;

    found=file_path.find_last_of("/\\");

    if (found != std::string::npos)
      return file_path.substr(0,found);

    return "";
  }

  #ifdef WINDOWS
  // Stub out these functions on Windows.  They're used for the daemon anyway, which isn't supported on Windows.

  static int makeDir(const char *path, mode_t mode) { return 0; }
  bool makePath(const char* path, mode_t mode) {
	  std::stringstream pathstream;
	  pathstream << "mkdir " << path;
	  std::string candidate_path = pathstream.str();
	  std::replace(candidate_path.begin(), candidate_path.end(), '/', '\\');

	  system(candidate_path.c_str());
	  return true;
  }
  FileInfo getFileInfo(std::string filename) {
    FileInfo response;
    response.creation_time = 0;
    response.size = 0;
    return response;
  }

  #else

  FileInfo getFileInfo(std::string filename)
  {
    FileInfo response;

    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    //return rc == 0 ? stat_buf.st_size : -1;

    // 512 bytes is the standard block size
    if (rc == 0)
    {
      #if defined(__APPLE__)
            double milliseconds = (stat_buf.st_ctimespec.tv_sec * 1000) +  (((double) stat_buf.st_ctimespec.tv_nsec) / 1000000.0);
      #else
            double milliseconds = (stat_buf.st_ctim.tv_sec * 1000) +  (((double) stat_buf.st_ctim.tv_nsec) / 1000000.0);
      #endif
      response.size = 512 * stat_buf.st_blocks;
      response.creation_time = (int64_t) milliseconds;
    }
    else
    {
      response.creation_time = 0;
      response.size = 0;
    }

    return response;
  }

  static int makeDir(const char *path, mode_t mode)
  {
    struct stat            st;
    int             status = 0;

    if (stat(path, &st) != 0)
    {
        /* Directory does not exist. EEXIST for race condition */
        if (mkdir(path, mode) != 0 && errno != EEXIST)
            status = -1;
    }
    else if (!S_ISDIR(st.st_mode))
    {
        errno = ENOTDIR;
        status = -1;
    }

    return(status);
  }

  /**
  ** makePath - ensure all directories in path exist
  ** Algorithm takes the pessimistic view and works top-down to ensure
  ** each directory in path exists, rather than optimistically creating
  ** the last element and working backwards.
  */
  bool makePath(const char* path, mode_t mode)
  {

    char           *pp;
    char           *sp;
    int             status;
    char           *copypath = strdup(path);

    status = 0;
    pp = copypath;
    while (status == 0 && (sp = strchr(pp, '/')) != 0)
    {
        if (sp != pp)
        {
            /* Neither root nor double slash in path */
            *sp = '\0';
            status = makeDir(copypath, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }
    if (status == 0)
        status = makeDir(path, mode);
    free(copypath);
    return (status == 0);

  }

  #endif
}
