
target_link_libraries(openalpr)



ADD_EXECUTABLE( sortstate  sortstate.cpp  )
TARGET_LINK_LIBRARIES(sortstate
    openalpr
    support
    ${OpenCV_LIBS}
    tesseract
  )

ADD_EXECUTABLE( classifychars  classifychars.cpp )
TARGET_LINK_LIBRARIES(classifychars
    openalpr
    support
    ${OpenCV_LIBS}
    tesseract
  )


ADD_EXECUTABLE( benchmark  benchmark.cpp )
TARGET_LINK_LIBRARIES(benchmark
    openalpr
    support
    ${OpenCV_LIBS}
    tesseract
  )


ADD_EXECUTABLE( prepcharsfortraining  prepcharsfortraining.cpp )
TARGET_LINK_LIBRARIES(prepcharsfortraining
    support
    ${OpenCV_LIBS}
  )
