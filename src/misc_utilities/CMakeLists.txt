
target_link_libraries(openalpr)



ADD_EXECUTABLE( sortstate  sortstate.cpp  )
TARGET_LINK_LIBRARIES(sortstate 
    openalpr-static
    support
    ${OpenCV_LIBS} 
	${Tesseract_LIBS}
  )
  
ADD_EXECUTABLE( classifychars  classifychars.cpp )
TARGET_LINK_LIBRARIES(classifychars 
    openalpr-static
    support
    ${OpenCV_LIBS} 
	${Tesseract_LIBS}
  )
  
  
ADD_EXECUTABLE( benchmark  benchmark.cpp )
TARGET_LINK_LIBRARIES(benchmark 
    openalpr-static
    support
    pthread
    ${OpenCV_LIBS} 
	${Tesseract_LIBS}
  )
  
  
ADD_EXECUTABLE( prepcharsfortraining  prepcharsfortraining.cpp )
TARGET_LINK_LIBRARIES(prepcharsfortraining 
    support
    ${OpenCV_LIBS} 
  )
  
  
