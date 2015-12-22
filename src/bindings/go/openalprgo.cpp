#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <alpr.h>
#include "openalprgo.h"

extern "C" {

#if defined(_MSC_VER)
    //  Microsoft
#define OPENALPR_EXPORT __declspec(dllexport)
#else
    //  do nothing
#define OPENALPR_EXPORT
#endif

    //using namespace alpr;
    Alpr AlprInit(char* country, char* configFile, char* runtimeDir) {
        alpr::Alpr* alpr = new alpr::Alpr(country, configFile, runtimeDir);
        return (void*)alpr;
    }

    void SetDetectRegion(Alpr alpr, int detectRegion) {
        alpr::Alpr* cxxalpr = (alpr::Alpr*) alpr;
        cxxalpr->setDetectRegion(detectRegion);
    }

    void SetTopN(Alpr alpr, int topN) {
        alpr::Alpr* cxxalpr = (alpr::Alpr*) alpr;
        cxxalpr->setTopN(topN);
    }

    void SetDefaultRegion(Alpr alpr, char* region) {
        alpr::Alpr* cxxalpr = (alpr::Alpr*) alpr;
        cxxalpr->setDefaultRegion(region);
    }

    int IsLoaded(Alpr alpr) {
        alpr::Alpr* cxxalpr = (alpr::Alpr*) alpr;
        return cxxalpr->isLoaded();
    }

    void Unload(Alpr alpr) {
        alpr::Alpr* cxxalpr = (alpr::Alpr*) alpr;
        delete cxxalpr;
    }

    char* RecognizeByFilePath(Alpr alpr, char* filePath) {
        alpr::Alpr* cxxalpr = (alpr::Alpr*) alpr;
        alpr::AlprResults result = cxxalpr->recognize(filePath);

        std::string resultString = alpr::Alpr::toJson(result);
        char *cstr = new char[resultString.length() + 1];
        strcpy(cstr, resultString.c_str());
        return cstr;
    }

    char* RecognizeByBlob(Alpr alpr, char* imageBytes, int len) {
        alpr::Alpr* cxxalpr = (alpr::Alpr*) alpr;
        std::vector<char> vec(imageBytes, imageBytes + len);
        alpr::AlprResults result = cxxalpr->recognize(vec);

        std::string resultString = alpr::Alpr::toJson(result);
        char *cstr = new char[resultString.length() + 1];
        strcpy(cstr, resultString.c_str());
        return cstr;
    }

    char* GetVersion() {
        std::string version = alpr::Alpr::getVersion();
        char *cstr = new char[version.length() + 1];
        strcpy(cstr, version.c_str());
        return cstr;
    }
}
