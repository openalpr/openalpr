



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
 tinythread/tinythread.cpp
)

 

add_subdirectory(simpleini)
add_subdirectory(support)


add_library(openalpr-static 	STATIC ${lpr_source_files} )
add_library(openalpr 		SHARED ${lpr_source_files} )

IF (WIN32)
  TARGET_LINK_LIBRARIES(openalpr
	  support
	  ${OpenCV_LIBS}
	  ${Tesseract_LIBS}
    )
ENDIF()
# Add definition for default runtime dir
add_definitions(-DDEFAULT_RUNTIME_DIR="${CMAKE_SOURCE_DIR}/../runtime_data/")