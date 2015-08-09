


set(statedetector_source_files
  state_detector.cpp
  featurematcher.cpp
  state_detector_impl.cpp
)


add_library(statedetection 		SHARED ${statedetector_source_files} )

set_target_properties(statedetection PROPERTIES SOVERSION ${OPENALPR_MAJOR_VERSION})

TARGET_LINK_LIBRARIES(statedetection
   ${OpenCV_LIBS}
)


install (FILES   state_detector.h     DESTINATION    ${CMAKE_INSTALL_PREFIX}/include)
install (TARGETS statedetection   DESTINATION    ${CMAKE_INSTALL_PREFIX}/lib)


