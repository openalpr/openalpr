


set(lpr_source_files
 alpr.cpp
 alpr_impl.cpp
 config.cpp
 detection/detector.cpp
 detection/detectorcpu.cpp
 detection/detectorfactory.cpp
 licenseplatecandidate.cpp
 utility.cpp
 stateidentifier.cpp
 featurematcher.cpp
 ocr.cpp
 postprocess.cpp
 binarize_wolf.cpp
 segmentation/charactersegmenter.cpp
 segmentation/verticalhistogram.cpp
 edges/edgefinder.cpp
 edges/platecorners.cpp
 edges/platelines.cpp
 edges/textlinecollection.cpp
 edges/scorekeeper.cpp
 colorfilter.cpp
 transformation.cpp
 textdetection/characteranalysis.cpp
 textdetection/platemask.cpp
 textdetection/textcontours.cpp
 textdetection/textline.cpp
 textdetection/linefinder.cpp
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
  ${Tesseract_LIBRARIES}
)


install (FILES   alpr.h     DESTINATION    ${CMAKE_INSTALL_PREFIX}/include)
install (TARGETS openalpr-static DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install (TARGETS openalpr   DESTINATION    ${CMAKE_INSTALL_PREFIX}/lib)

# Add definition for default config file
add_definitions(-DDEFAULT_CONFIG_FILE="${CMAKE_INSTALL_SYSCONFDIR}/openalpr/openalpr.conf")
