
#include "support/filesystem.h"
#include "benchmark_utils.h"

using namespace std;
using namespace alpr;

vector<string> filterByExtension(vector<string> fileList, string extension)
{
  vector<string> filteredList;
  
  if (extension.size() == 0)
    return filteredList;
  
  if (extension[0] != '.')
    extension = "." + extension;
  
  for (int i = 0; i < fileList.size(); i++)
  {
    if (hasEnding(fileList[i], extension))
      filteredList.push_back(fileList[i]);
  }
  
  return filteredList;
}