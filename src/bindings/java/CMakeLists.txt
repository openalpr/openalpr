
cmake_minimum_required (VERSION 2.6)


find_package(JNI REQUIRED)


include_directories(
  ../../openalpr/
  ${JAVA_INCLUDE_PATH}
)


set(jni_source_files
 openalprjni.cpp
)

    
add_library(openalprjni SHARED ${jni_source_files})


TARGET_LINK_LIBRARIES(openalprjni openalpr)


install (TARGETS openalprjni   DESTINATION    ${CMAKE_INSTALL_PREFIX}/lib)