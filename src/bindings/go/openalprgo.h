
#if defined(_MSC_VER)
//  Microsoft
#define OPENALPR_EXPORT __declspec(dllexport)
#else
//  do nothing
#define OPENALPR_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif
    typedef void* Alpr;

	OPENALPR_EXPORT Alpr AlprInit(char* country, char* configFile, char* runtimeDir);
	OPENALPR_EXPORT void SetDetectRegion(Alpr alpr, int detectRegion);
	OPENALPR_EXPORT void SetTopN(Alpr alpr, int topN);
	OPENALPR_EXPORT void SetDefaultRegion(Alpr alpr, char* region);
	OPENALPR_EXPORT int IsLoaded(Alpr alpr);
	OPENALPR_EXPORT void Unload(Alpr alpr);
	OPENALPR_EXPORT char* RecognizeByFilePath(Alpr alpr, char* filePath);
	OPENALPR_EXPORT char* RecognizeByBlob(Alpr alpr, char* imageBytes, int len);
	OPENALPR_EXPORT char* GetVersion();
#ifdef __cplusplus
}
#endif
