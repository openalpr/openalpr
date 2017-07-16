#include <string>
#include <string.h>
#include <vector>

#include <alpr.h>
#include "openalprgo.h"

extern "C" {


    //using namespace alpr;
    OPENALPR_EXPORT Alpr AlprInit(char* country, char* configFile, char* runtimeDir) {
        alpr::Alpr* alpr = new alpr::Alpr(country, configFile, runtimeDir);
        return (void*)alpr;
    }

    OPENALPR_EXPORT void SetDetectRegion(Alpr alpr, int detectRegion) {
        alpr::Alpr* cxxalpr = (alpr::Alpr*) alpr;
        cxxalpr->setDetectRegion(detectRegion);
    }

    OPENALPR_EXPORT void SetTopN(Alpr alpr, int topN) {
        alpr::Alpr* cxxalpr = (alpr::Alpr*) alpr;
        cxxalpr->setTopN(topN);
    }

    OPENALPR_EXPORT void SetDefaultRegion(Alpr alpr, char* region) {
        alpr::Alpr* cxxalpr = (alpr::Alpr*) alpr;
        cxxalpr->setDefaultRegion(region);
    }

    OPENALPR_EXPORT int IsLoaded(Alpr alpr) {
        alpr::Alpr* cxxalpr = (alpr::Alpr*) alpr;
        return cxxalpr->isLoaded();
    }

    OPENALPR_EXPORT void Unload(Alpr alpr) {
        alpr::Alpr* cxxalpr = (alpr::Alpr*) alpr;
        delete cxxalpr;
    }

    OPENALPR_EXPORT char* RecognizeByFilePath(Alpr alpr, char* filePath) {
        alpr::Alpr* cxxalpr = (alpr::Alpr*) alpr;
        alpr::AlprResults result = cxxalpr->recognize(filePath);

        std::string resultString = alpr::Alpr::toJson(result);
        char *cstr = new char[resultString.length() + 1];
        strcpy(cstr, resultString.c_str());
        return cstr;
    }

    OPENALPR_EXPORT char* RecognizeByBlob(Alpr alpr, char* imageBytes, int len) {
        alpr::Alpr* cxxalpr = (alpr::Alpr*) alpr;
        std::vector<char> vec(imageBytes, imageBytes + len);
        alpr::AlprResults result = cxxalpr->recognize(vec);

        std::string resultString = alpr::Alpr::toJson(result);
        char *cstr = new char[resultString.length() + 1];
        strcpy(cstr, resultString.c_str());
        return cstr;
    }

    OPENALPR_EXPORT char* GetVersion() {
        std::string version = alpr::Alpr::getVersion();
        char *cstr = new char[version.length() + 1];
        strcpy(cstr, version.c_str());
        return cstr;
    }
}
