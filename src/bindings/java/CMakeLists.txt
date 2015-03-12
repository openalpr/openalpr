
cmake_minimum_required (VERSION 2.6)


find_package(JNI)

if (${JNI_FOUND} MATCHES "TRUE")
  include_directories(
    ../../openalpr/
    ${JAVA_INCLUDE_PATH}
    ${JAVA_INCLUDE_PATH2}
  )


  set(jni_source_files
   openalprjni.cpp
  )


  add_library(openalprjni SHARED ${jni_source_files})

  set_target_properties(openalprjni PROPERTIES SOVERSION ${OPENALPR_MAJOR_VERSION})

  TARGET_LINK_LIBRARIES(openalprjni openalpr)


  install (TARGETS openalprjni   DESTINATION    ${CMAKE_INSTALL_PREFIX}/lib)
else()
  MESSAGE("JAVA JNI library not found, skipping openalprjni compilation")

endif()