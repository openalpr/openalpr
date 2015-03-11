#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <alpr.h>

extern "C" {
  
  using namespace alpr;

  bool initialized = false;
  static Alpr* nativeAlpr;

  void initialize(char* ccountry, char* cconfigFile, char* cruntimeDir)
  {
    //printf("Initialize");

    // Convert strings from char* to string
    std::string country(ccountry);
    std::string configFile(cconfigFile);
    std::string runtimeDir(cruntimeDir);

    //std::cout << country << std::endl << configFile << std::endl << runtimeDir << std::endl;
    nativeAlpr = new alpr::Alpr(country, configFile, runtimeDir);

    initialized = true;
    return;
  }




  void dispose()
    {
      //printf("Dispose");
      initialized = false;
      delete nativeAlpr;
    }


  bool isLoaded()
    {
      //printf("IS LOADED");

      if (!initialized)
        return false;

      return nativeAlpr->isLoaded();

    }

  char* recognizeFile(char* cimageFile)
    {
      //printf("Recognize file");

      // Convert strings from java to C++ and release resources
      std::string imageFile(cimageFile);

      AlprResults results = nativeAlpr->recognize(imageFile);

      std::string json = Alpr::toJson(results);
      
      int strsize = sizeof(char) * (strlen(json.c_str()) + 1);
      char* membuffer = (char*)malloc(strsize);
      strcpy(membuffer, json.c_str());
      //printf("allocated address: %p\n", membuffer);
      
      return membuffer;
    }
  
  void freeJsonMem(char* ptr)
  {
    //printf("freeing address: %p\n", ptr);
    free( ptr );
  }


  char* recognizeArray(unsigned char* buf, int len)
    {
      //printf("Recognize byte array");
      //printf("buffer pointer: %p\n", buf);
      //printf("buffer length: %d\n", len);
    

      std::vector<char> cvec(buf, buf+len);

      AlprResults results = nativeAlpr->recognize(cvec);
      std::string json = Alpr::toJson(results);
      
      int strsize = sizeof(char) * (strlen(json.c_str()) + 1);
      char* membuffer = (char*)malloc(strsize);
      strcpy(membuffer, json.c_str());
      //printf("allocated address: %p\n", membuffer);
      
      return membuffer;
    }

  void setDefaultRegion(char* cdefault_region)
    {
      // Convert strings from java to C++ and release resources
      std::string default_region(cdefault_region);

      nativeAlpr->setDefaultRegion(default_region);
    }

  void setDetectRegion(bool detect_region)
    {
      nativeAlpr->setDetectRegion(detect_region);
    }

  void setTopN(int top_n)
    {
      nativeAlpr->setTopN(top_n);
    }

  char* getVersion()
    {
      std::string version = nativeAlpr->getVersion();

      int strsize = sizeof(char) * (strlen(version.c_str()) + 1);
      char* membuffer = (char*)malloc(strsize);
      strcpy(membuffer, version.c_str());
      //printf("allocated address: %p\n", membuffer);
      
      return membuffer;
    }


}