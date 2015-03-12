
cmake_minimum_required (VERSION 2.6)


include_directories(../../openalpr/)


set(python_source_files
 openalprpy.cpp
)

    
add_library(openalprpy SHARED ${python_source_files})


TARGET_LINK_LIBRARIES(openalprpy openalpr)

