
ADD_EXECUTABLE( openalpr-utils-sortstate sortstate.cpp  )
TARGET_LINK_LIBRARIES(openalpr-utils-sortstate
    ${OPENALPR_LIB}
	statedetection
    support
    ${OpenCV_LIBS} 
	${Tesseract_LIBRARIES}
  )
  
ADD_EXECUTABLE( openalpr-utils-classifychars classifychars.cpp )
TARGET_LINK_LIBRARIES(openalpr-utils-classifychars
    ${OPENALPR_LIB}
    support
    ${OpenCV_LIBS} 
	${Tesseract_LIBRARIES}
  )
  
if (NOT DEFINED WIN32)
ADD_EXECUTABLE(openalpr-utils-benchmark
		benchmarks/benchmark.cpp 
		benchmarks/benchmark_utils.cpp 
		benchmarks/endtoendtest.cpp 
)
TARGET_LINK_LIBRARIES(openalpr-utils-benchmark
    ${OPENALPR_LIB}
    statedetection
    support
    pthread
    ${OpenCV_LIBS} 
	${Tesseract_LIBRARIES}
  )
ENDIF()
  
ADD_EXECUTABLE( openalpr-utils-prepcharsfortraining prepcharsfortraining.cpp )
TARGET_LINK_LIBRARIES(openalpr-utils-prepcharsfortraining
    support
    ${OpenCV_LIBS} 
  )
 
ADD_EXECUTABLE( openalpr-utils-binarizefontsheet binarizefontsheet.cpp )
TARGET_LINK_LIBRARIES(openalpr-utils-binarizefontsheet
    ${OPENALPR_LIB}
    support
    ${OpenCV_LIBS} 
  ) 
  
ADD_EXECUTABLE( openalpr-utils-tagplates tagplates.cpp )
TARGET_LINK_LIBRARIES(openalpr-utils-tagplates
    ${OPENALPR_LIB}
    support
    ${OpenCV_LIBS} 
  )
  
ADD_EXECUTABLE( openalpr-utils-calibrate calibrate.cpp  )
TARGET_LINK_LIBRARIES(openalpr-utils-calibrate
    ${OPENALPR_LIB}
    support
    ${OpenCV_LIBS} 
	${Tesseract_LIBRARIES}
  )

  
install (TARGETS openalpr-utils-calibrate DESTINATION bin)


install (TARGETS openalpr-utils-sortstate DESTINATION bin)
install (TARGETS openalpr-utils-classifychars DESTINATION bin)

if (NOT DEFINED WIN32)
install (TARGETS openalpr-utils-benchmark DESTINATION bin)
ENDIF()

install (TARGETS openalpr-utils-prepcharsfortraining DESTINATION bin)
install (TARGETS openalpr-utils-tagplates DESTINATION bin)
install (TARGETS openalpr-utils-calibrate DESTINATION bin)
