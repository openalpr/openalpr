



set(lpr_source_files
 alpr.cpp
 alpr_impl.cpp
 config.cpp
 regiondetector.cpp
 licenseplatecandidate.cpp
 utility.cpp
 stateidentifier.cpp
 featurematcher.cpp
 ocr.cpp
 postprocess.cpp
 binarize_wolf.cpp
 platelines.cpp
 characterregion.cpp
 charactersegmenter.cpp
 platecorners.cpp
 colorfilter.cpp
 characteranalysis.cpp
 verticalhistogram.cpp
 trex.c
 cjson.c
)

 

add_subdirectory(simpleini)
add_subdirectory(support)
add_subdirectory(tinythread)


add_library(openalpr ${lpr_source_files})