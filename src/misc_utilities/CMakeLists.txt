
ADD_EXECUTABLE( sortstate-openalpr-utils sortstate.cpp  )
TARGET_LINK_LIBRARIES(sortstate-openalpr-utils
    openalpr
    support
    ${OpenCV_LIBS} 
	${Tesseract_LIBRARIES}
  )
  
ADD_EXECUTABLE( classifychars-openalpr-utils classifychars.cpp )
TARGET_LINK_LIBRARIES(classifychars-openalpr-utils
    openalpr
    support
    ${OpenCV_LIBS} 
	${Tesseract_LIBRARIES}
  )
  
  
ADD_EXECUTABLE(benchmark-openalpr-utils
		benchmarks/benchmark.cpp 
		benchmarks/benchmark_utils.cpp 
		benchmarks/endtoendtest.cpp 
)
TARGET_LINK_LIBRARIES(benchmark-openalpr-utils
    openalpr
    support
    pthread
    ${OpenCV_LIBS} 
	${Tesseract_LIBRARIES}
  )
  
  
ADD_EXECUTABLE( prepcharsfortraining-openalpr-utils prepcharsfortraining.cpp )
TARGET_LINK_LIBRARIES(prepcharsfortraining-openalpr-utils
    support
    ${OpenCV_LIBS} 
  )
  
  
ADD_EXECUTABLE( tagplates-openalpr-utils tagplates.cpp )
TARGET_LINK_LIBRARIES(tagplates-openalpr-utils
    support
    ${OpenCV_LIBS} 
  )
  
install (TARGETS sortstate-openalpr-utils DESTINATION bin)
install (TARGETS classifychars-openalpr-utils DESTINATION bin)
install (TARGETS benchmark-openalpr-utils DESTINATION bin)
install (TARGETS prepcharsfortraining-openalpr-utils DESTINATION bin)
install (TARGETS tagplates-openalpr-utils DESTINATION bin)

