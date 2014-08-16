
target_link_libraries(openalpr)



ADD_EXECUTABLE( sortstate EXCLUDE_FROM_ALL sortstate.cpp  )
TARGET_LINK_LIBRARIES(sortstate 
    openalpr-static
    support
    ${OpenCV_LIBS} 
	${Tesseract_LIBS}
  )
  
ADD_EXECUTABLE( classifychars EXCLUDE_FROM_ALL classifychars.cpp )
TARGET_LINK_LIBRARIES(classifychars 
    openalpr-static
    support
    ${OpenCV_LIBS} 
	${Tesseract_LIBS}
  )
  
  
ADD_EXECUTABLE(benchmark  EXCLUDE_FROM_ALL
		benchmark/benchmark.cpp 
		benchmark/benchmark_utils.cpp 
		benchmark/endtoendtest.cpp 
)
TARGET_LINK_LIBRARIES(benchmark 
    openalpr-static
    support
    pthread
    ${OpenCV_LIBS} 
	${Tesseract_LIBS}
  )
  
  
ADD_EXECUTABLE( prepcharsfortraining EXCLUDE_FROM_ALL prepcharsfortraining.cpp )
TARGET_LINK_LIBRARIES(prepcharsfortraining 
    support
    ${OpenCV_LIBS} 
  )
  
  
ADD_EXECUTABLE( tagplates EXCLUDE_FROM_ALL tagplates.cpp )
TARGET_LINK_LIBRARIES(tagplates 
    support
    ${OpenCV_LIBS} 
  )
  
