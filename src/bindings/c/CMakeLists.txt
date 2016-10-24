
cmake_minimum_required (VERSION 2.6)


include_directories(../../openalpr/)


set(alprc_source
 alpr_c.cpp
)

    
add_library(openalprc SHARED ${alprc_source})

set_target_properties(openalprc PROPERTIES SOVERSION ${OPENALPR_MAJOR_VERSION})

TARGET_LINK_LIBRARIES(openalprc openalpr)


add_executable(alprc_test alprc_test.c)
TARGET_LINK_LIBRARIES(alprc_test openalprc)

install (TARGETS    openalprc       DESTINATION     ${CMAKE_INSTALL_PREFIX}/lib)
install (FILES   alpr_c.h     DESTINATION    ${CMAKE_INSTALL_PREFIX}/include)
