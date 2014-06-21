#include "filesystem.h"

bool startsWith(std::string const &fullString, std::string const &prefix)
{
  if(fullString.substr(0, prefix.size()) == prefix) {
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
  int lastindex = filename.find_last_of("."); 
  return filename.substr(0, lastindex); 
}