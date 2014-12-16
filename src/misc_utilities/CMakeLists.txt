
ADD_EXECUTABLE( openalpr-utils-sortstate sortstate.cpp  )
TARGET_LINK_LIBRARIES(openalpr-utils-sortstate
    openalpr
    support
    ${OpenCV_LIBS} 
	${Tesseract_LIBRARIES}
  )
  
ADD_EXECUTABLE( openalpr-utils-classifychars classifychars.cpp )
TARGET_LINK_LIBRARIES(openalpr-utils-classifychars
    openalpr
    support
    ${OpenCV_LIBS} 
	${Tesseract_LIBRARIES}
  )
  
  
ADD_EXECUTABLE(openalpr-utils-benchmark
		benchmarks/benchmark.cpp 
		benchmarks/benchmark_utils.cpp 
		benchmarks/endtoendtest.cpp 
)
TARGET_LINK_LIBRARIES(openalpr-utils-benchmark
    openalpr
    support
    pthread
    ${OpenCV_LIBS} 
	${Tesseract_LIBRARIES}
  )
  
  
ADD_EXECUTABLE( openalpr-utils-prepcharsfortraining prepcharsfortraining.cpp )
TARGET_LINK_LIBRARIES(openalpr-utils-prepcharsfortraining
    support
    ${OpenCV_LIBS} 
  )
  
  
ADD_EXECUTABLE( openalpr-utils-tagplates tagplates.cpp )
TARGET_LINK_LIBRARIES(openalpr-utils-tagplates
    support
    ${OpenCV_LIBS} 
  )
  
install (TARGETS openalpr-utils-sortstate DESTINATION bin)
install (TARGETS openalpr-utils-classifychars DESTINATION bin)
install (TARGETS openalpr-utils-benchmark DESTINATION bin)
install (TARGETS openalpr-utils-prepcharsfortraining DESTINATION bin)
install (TARGETS openalpr-utils-tagplates DESTINATION bin)

