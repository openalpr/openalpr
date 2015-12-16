cmake_minimum_required (VERSION 2.6)

set(go_source_files
  openalprgo.cpp
)

add_library(openalprgo SHARED ${go_source_files})

set_target_properties(openalprgo PROPERTIES SOVERSION ${OPENALPR_MAJOR_VERSION})

TARGET_LINK_LIBRARIES(openalprgo openalpr)

install (TARGETS    openalprgo       DESTINATION     ${CMAKE_INSTALL_PREFIX}/lib)
install (FILES    openalprgo.h     DESTINATION     ${CMAKE_INSTALL_PREFIX}/include)
