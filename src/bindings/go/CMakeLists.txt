cmake_minimum_required (VERSION 2.6)

set(go_source_files
  openalprgo.cpp
)

add_library(openalprgo SHARED ${go_source_files})

set_target_properties(openalprgo PROPERTIES SOVERSION ${OPENALPR_MAJOR_VERSION})

TARGET_LINK_LIBRARIES(openalprgo openalpr)

if (NOT WIN32)
    #generate the .pc file
    SET(prefix "${CMAKE_INSTALL_PREFIX}")
    SET(exec_prefix "\${prefix}")
    SET(libdir "\${exec_prefix}/lib")
    SET(includedir "\${exec_prefix}/include")
    SET(VERSION "${OPENALPR_VERSION}")
    CONFIGURE_FILE(
      "${CMAKE_MODULE_PATH}/templates/openalpr.pc.in"
      "${CMAKE_CURRENT_BINARY_DIR}/pkgconfig/openalpr.pc"
      @ONLY)
    install (FILES "${CMAKE_CURRENT_BINARY_DIR}/pkgconfig/openalpr.pc" DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig")
ENDIF()

install (TARGETS    openalprgo       DESTINATION     ${CMAKE_INSTALL_PREFIX}/lib)
install (FILES    openalprgo.h     DESTINATION     ${CMAKE_INSTALL_PREFIX}/include)
