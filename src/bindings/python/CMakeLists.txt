
cmake_minimum_required (VERSION 2.6)


include_directories(../../openalpr/)


set(python_source_files
 openalprpy.cpp
)

    
add_library(openalprpy SHARED ${python_source_files})

set_target_properties(openalprpy PROPERTIES SOVERSION ${OPENALPR_MAJOR_VERSION})

TARGET_LINK_LIBRARIES(openalprpy openalpr)


install (TARGETS    openalprpy       DESTINATION     ${CMAKE_INSTALL_PREFIX}/lib)
install (DIRECTORY  openalpr  DESTINATION     ${CMAKE_INSTALL_PREFIX}/lib/python2.7/dist-packages
    USE_SOURCE_PERMISSIONS
)
