


set(lpr_source_files
 alpr.cpp
 alpr_impl.cpp
 alpr_c.cpp
 config.cpp
 config_helper.cpp
 detection/detector.cpp
 detection/detectorcpu.cpp
 detection/detectorcuda.cpp
 detection/detectorocl.cpp
 detection/detectorfactory.cpp
 detection/detectormorph.cpp
 detection/detectormask.cpp
 licenseplatecandidate.cpp
 utility.cpp
 ocr/tesseract_ocr.cpp
 ocr/ocr.cpp
 ocr/ocrfactory.cpp
 postprocess/postprocess.cpp
 postprocess/regexrule.cpp
 binarize_wolf.cpp
 ocr/segmentation/charactersegmenter.cpp
 ocr/segmentation/histogram.cpp
 ocr/segmentation/histogramvertical.cpp
 ocr/segmentation/histogramhorizontal.cpp
 edges/edgefinder.cpp
 edges/platecorners.cpp
 edges/platelines.cpp
 edges/textlinecollection.cpp
 edges/scorekeeper.cpp
 colorfilter.cpp
 prewarp.cpp
 transformation.cpp
 textdetection/characteranalysis.cpp
 textdetection/platemask.cpp
 textdetection/textcontours.cpp
 textdetection/textline.cpp
 textdetection/linefinder.cpp
 pipeline_data.cpp
 cjson.c
 motiondetector.cpp
 result_aggregator.cpp
)

 

add_subdirectory(simpleini)
add_subdirectory(support)


add_library(openalpr-static 	STATIC ${lpr_source_files} )
add_library(openalpr 		SHARED ${lpr_source_files} )

set_target_properties(openalpr PROPERTIES SOVERSION ${OPENALPR_MAJOR_VERSION})

TARGET_LINK_LIBRARIES(openalpr
  support
  ${STATE_DETECTION_LIB}
  ${OpenCV_LIBS}
  ${Tesseract_LIBRARIES}
)


install (FILES   alpr.h     DESTINATION    ${CMAKE_INSTALL_PREFIX}/include)
install (FILES   alpr_c.h     DESTINATION    ${CMAKE_INSTALL_PREFIX}/include)
install (TARGETS openalpr-static DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install (TARGETS openalpr   DESTINATION    ${CMAKE_INSTALL_PREFIX}/lib)

# Compile GPU detector
IF ( WITH_GPU_DETECTOR )
add_definitions(-DCOMPILE_GPU=1)
ENDIF()

# Add definition for default config file
add_definitions(-DDEFAULT_CONFIG_FILE="${CMAKE_INSTALL_SYSCONFDIR}/openalpr/openalpr.conf")
