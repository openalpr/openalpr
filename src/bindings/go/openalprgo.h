#ifdef __cplusplus
extern "C" {
#endif
    typedef void* Alpr;

    Alpr AlprInit(char* country, char* configFile, char* runtimeDir);
    void SetDetectRegion(Alpr alpr, int detectRegion);
    void SetTopN(Alpr alpr, int topN);
    void SetDefaultRegion(Alpr alpr, char* region);
    int IsLoaded(Alpr alpr);
    void Unload(Alpr alpr);
    char* RecognizeByFilePath(Alpr alpr, char* filePath);
    char* RecognizeByBlob(Alpr alpr, char* imageBytes, int len);
    char* GetVersion();
#ifdef __cplusplus
}
#endif
