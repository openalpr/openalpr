#include <stdlib.h>
#include <string>
#include <string.h>

#include <alpr.h>

extern "C" {

#if defined(WIN32)
    //  Microsoft
    #define OPENALPR_EXPORT __declspec(dllexport)
#else
    //  do nothing
    #define OPENALPR_EXPORT
#endif

  using namespace alpr;


  OPENALPR_EXPORT Alpr* initialize(char* ccountry, char* cconfigFile, char* cruntimeDir)
  {
    //printf("Initialize");

    // Convert strings from char* to string
    std::string country(ccountry);
    std::string configFile(cconfigFile);
    std::string runtimeDir(cruntimeDir);

    //std::cout << country << std::endl << configFile << std::endl << runtimeDir << std::endl;
    Alpr* nativeAlpr = new alpr::Alpr(country, configFile, runtimeDir);

    return nativeAlpr;
  }




  OPENALPR_EXPORT void dispose(Alpr* nativeAlpr)
    {
      delete nativeAlpr;
    }


  OPENALPR_EXPORT bool isLoaded(Alpr* nativeAlpr)
    {
      //printf("IS LOADED");

      return nativeAlpr->isLoaded();

    }

  OPENALPR_EXPORT char* recognizeFile(Alpr* nativeAlpr, char* cimageFile)
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

  OPENALPR_EXPORT void freeJsonMem(char* ptr)
  {
    //printf("freeing address: %p\n", ptr);
    free( ptr );
  }


  OPENALPR_EXPORT char* recognizeArray(Alpr* nativeAlpr, unsigned char* buf, int len)
    {
      //printf("Recognize byte array");
      //printf("buffer pointer: %p\n", buf);
      //printf("buffer length: %d\n", len);

      //std::cout << "Using instance: " << nativeAlpr << std::endl;

      std::vector<char> cvec(buf, buf+len);

      AlprResults results = nativeAlpr->recognize(cvec);
      std::string json = Alpr::toJson(results);

      int strsize = sizeof(char) * (strlen(json.c_str()) + 1);
      char* membuffer = (char*)malloc(strsize);
      strcpy(membuffer, json.c_str());
      //printf("allocated address: %p\n", membuffer);

      return membuffer;
    }

  // AlprResults recognize(unsigned char* pixelData,
  // int bytesPerPixel, int imgWidth, int imgHeight,
  // std::vector<AlprRegionOfInterest> regionsOfInterest);
  OPENALPR_EXPORT char* recognizeRawImage(Alpr* nativeAlpr, unsigned char* buf, int bytesPerPixel, int imgWidth, int imgHeight)
    {
      //printf("Recognize raw image");
      //printf("buffer pointer: %p\n", buf);
      //printf("buffer length: %d\n", len);

      //std::cout << "Using instance: " << nativeAlpr << std::endl;

      std::vector<AlprRegionOfInterest> regionsOfInterest;
      AlprResults results = nativeAlpr->recognize(buf, bytesPerPixel, imgWidth, imgHeight, regionsOfInterest);
      std::string json = Alpr::toJson(results);

      int strsize = sizeof(char) * (strlen(json.c_str()) + 1);
      char* membuffer = (char*)malloc(strsize);
      strcpy(membuffer, json.c_str());
      //printf("allocated address: %p\n", membuffer);

      return membuffer;
    }

  OPENALPR_EXPORT void setCountry(Alpr* nativeAlpr, char* ccountry)
    {
      // Convert strings from java to C++ and release resources
      std::string country(ccountry);

      nativeAlpr->setCountry(country);
    }

  OPENALPR_EXPORT void setPrewarp(Alpr* nativeAlpr, char* cprewarp)
    {
      // Convert strings from java to C++ and release resources
      std::string prewarp(cprewarp);

      nativeAlpr->setPrewarp(prewarp);
    }

  OPENALPR_EXPORT void setDefaultRegion(Alpr* nativeAlpr, char* cdefault_region)
    {
      // Convert strings from java to C++ and release resources
      std::string default_region(cdefault_region);

      nativeAlpr->setDefaultRegion(default_region);
    }

  OPENALPR_EXPORT void setDetectRegion(Alpr* nativeAlpr, bool detect_region)
    {
      nativeAlpr->setDetectRegion(detect_region);
    }

  OPENALPR_EXPORT void setTopN(Alpr* nativeAlpr, int top_n)
    {
      nativeAlpr->setTopN(top_n);
    }

  OPENALPR_EXPORT char* getVersion(Alpr* nativeAlpr)
    {
      std::string version = nativeAlpr->getVersion();

      int strsize = sizeof(char) * (strlen(version.c_str()) + 1);
      char* membuffer = (char*)malloc(strsize);
      strcpy(membuffer, version.c_str());
      //printf("allocated address: %p\n", membuffer);

      return membuffer;
    }


}
