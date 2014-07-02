



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
 segmentation/charactersegmenter.cpp
 segmentation/verticalhistogram.cpp
 platecorners.cpp
 colorfilter.cpp
 characteranalysis.cpp
 pipeline_data.cpp
 trex.c
 cjson.c
)

 

add_subdirectory(simpleini)
add_subdirectory(support)


add_library(openalpr-static 	STATIC ${lpr_source_files} )
add_library(openalpr 		SHARED ${lpr_source_files} )

set_target_properties(openalpr PROPERTIES SOVERSION ${OPENALPR_MAJOR_VERSION})

  TARGET_LINK_LIBRARIES(openalpr
	  support
	  ${OpenCV_LIBS}
	  ${Tesseract_LIBS}
    )


install (FILES alpr.h DESTINATION include)
install (TARGETS openalpr DESTINATION lib)

# Add definition for default runtime dir
#add_definitions(-DDEFAULT_RUNTIME_DIR="/usr/share/openalpr/runtime_data")
add_definitions(-DDEFAULT_CONFIG_FILE="/etc/openalpr/openalpr.conf")
