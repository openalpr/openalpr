project(src)

#set(CMAKE_BUILD_TYPE Debug)
cmake_minimum_required (VERSION 2.6)

# Set the OpenALPR version in cmake, and also add it as a DEFINE for the code to access
SET(OPENALPR_MAJOR_VERSION "2")
SET(OPENALPR_MINOR_VERSION "2")
SET(OPENALPR_PATCH_VERSION "0")
SET(OPENALPR_VERSION ${OPENALPR_MAJOR_VERSION}.${OPENALPR_MINOR_VERSION}.${OPENALPR_PATCH_VERSION})

add_definitions( -DOPENALPR_MAJOR_VERSION=${OPENALPR_MAJOR_VERSION})
add_definitions( -DOPENALPR_MINOR_VERSION=${OPENALPR_MINOR_VERSION})
add_definitions( -DOPENALPR_PATCH_VERSION=${OPENALPR_PATCH_VERSION})

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")

# TODO: switch to http://www.cmake.org/cmake/help/v2.8.5/cmake.html#module:GNUInstallDirs ?
IF (NOT CMAKE_INSTALL_SYSCONFDIR) 
  SET(CMAKE_INSTALL_SYSCONFDIR "/etc")
ENDIF()
IF (NOT CMAKE_INSTALL_VARDIR) 
  SET(CMAKE_INSTALL_VARDIR "${CMAKE_INSTALL_PREFIX}/var")
ENDIF()

IF ( NOT DEFINED WITH_DAEMON )
  SET(WITH_DAEMON ON)
ENDIF()

if ( NOT DEFINED WITH_GPU_DETECTOR )
  SET(WITH_GPU_DETECTOR OFF)
ENDIF()

if ( NOT DEFINED WITH_TESTS )
  SET(WITH_TESTS ON)
ENDIF()

if ( NOT DEFINED WITH_BINDING_JAVA )
  SET(WITH_BINDING_JAVA ON)
ENDIF()

if ( NOT DEFINED WITH_BINDING_PYTHON )
  SET(WITH_BINDING_PYTHON ON)
ENDIF()

if ( NOT DEFINED WITH_UTILITIES )
  SET(WITH_UTILITIES ON)
ENDIF()

IF (WIN32 AND WITH_DAEMON) 
  MESSAGE(WARNING "Skipping alprd daemon installation, as it is not supported in Windows.")
  SET(WITH_DAEMON OFF)
ENDIF()

FIND_PACKAGE( Tesseract REQUIRED )

include_directories(${Tesseract_INCLUDE_DIRS})

# Discover OpenCV directory automatically
find_path(OpenCV_DIR
  NAMES OpenCVConfig.cmake
  HINTS ${CMAKE_SOURCE_DIR}/../libraries/opencv/
        /storage/projects/alpr/libraries/opencv/
)
# Opencv Package
FIND_PACKAGE( OpenCV REQUIRED )
IF (${OpenCV_VERSION} VERSION_LESS 2.4.7)
	MESSAGE(FATAL_ERROR "OpenCV version is not compatible : ${OpenCV_VERSION}")
ENDIF()

include_directories(${OpenCV_INCLUDE_DIRS})

add_definitions( -DOPENCV_MAJOR_VERSION=${OpenCV_VERSION_MAJOR})


include_directories(./openalpr )

IF (WIN32)
	add_definitions( -DWINDOWS)
	add_definitions( -DNOMINMAX)

	# Extra linker dependencies for Windows
	SET (Tesseract_LIBRARIES
		${Tesseract_LIBRARIES}
		ws2_32.lib
	)
ELSE()

	SET (Extra_LIBS
		pthread
	)

ENDIF()


set(CMAKE_CSS_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall ")
ADD_EXECUTABLE( alpr  main.cpp )

if (WIN32) 
   SET(OPENALPR_LIB openalpr-static)
ELSE()
   SET(OPENALPR_LIB openalpr)
ENDIF()

TARGET_LINK_LIBRARIES(alpr
	${OPENALPR_LIB}
	statedetection
	support
    video
	${OpenCV_LIBS}
	${Tesseract_LIBRARIES}
    ${Extra_LIBS}
  )

# Compile the alprd library on Unix-based OS
IF (WITH_DAEMON)
  ADD_EXECUTABLE( alprd  daemon.cpp daemon/beanstalk.c daemon/beanstalk.cc )

  TARGET_LINK_LIBRARIES(alprd
	  openalpr
	  support
      video
	  curl
	  log4cplus
	  ${OpenCV_LIBS}
	  ${Tesseract_LIBRARIES}
      ${Extra_LIBS}
    )
	
ENDIF()

if(WITH_UTILITIES)
add_subdirectory(misc_utilities)
ENDIF()

if (WITH_TESTS)  
add_subdirectory(tests)
ENDIF()

add_subdirectory(openalpr)
add_subdirectory(statedetection)
add_subdirectory(video)

if (WITH_BINDING_JAVA) 
add_subdirectory(bindings/java)
ENDIF()

if (WITH_BINDING_PYTHON)
add_subdirectory(bindings/python)
ENDIF()

install (TARGETS    alpr DESTINATION   ${CMAKE_INSTALL_PREFIX}/bin)
install (FILES      ${CMAKE_SOURCE_DIR}/../doc/man/alpr.1 DESTINATION       ${CMAKE_INSTALL_PREFIX}/share/man/man1 COMPONENT doc)
install (DIRECTORY  ${CMAKE_SOURCE_DIR}/../runtime_data DESTINATION         ${CMAKE_INSTALL_PREFIX}/share/openalpr)

# set runtime_data to reflect the current CMAKE_INSTALL_PREFIX
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/../config/openalpr.conf.in ${CMAKE_CURRENT_BINARY_DIR}/config/openalpr.conf)
install (FILES      ${CMAKE_CURRENT_BINARY_DIR}/config/openalpr.conf DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/openalpr/      COMPONENT config)

IF (WITH_DAEMON)
  install (TARGETS    alprd DESTINATION  ${CMAKE_INSTALL_PREFIX}/bin)
  install (FILES      ${CMAKE_SOURCE_DIR}/../config/alprd.conf DESTINATION    ${CMAKE_INSTALL_SYSCONFDIR}/openalpr      COMPONENT config)
ENDIF()


SET (CPACK_PACKAGE_VERSION ${OPENALPR_VERSION})
SET (CPACK_SET_DESTDIR "on")
SET (CPACK_GENERATOR "DEB;TGZ")
SET (CPACK_PACKAGE_NAME "openalpr")
SET (CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_SOURCE_DIR}/build;src;ALL;/")
SET (CPACK_CMAKE_GENERATOR "Unix Makefiles")

SET (CPACK_STRIP_FILES "1")
SET (CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
SET (CPACK_DEBIAN_PACKAGE_SECTION "video")
SET (CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
SET (CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.1.3), libgcc1 (>= 4.1.1), libtesseract3 (>= 3.0.3), libopencv-core2.4 (>= 2.4.8), libopencv-objdetect2.4 (>= 2.4.8), libopencv-highgui2.4 (>= 2.4.8), libopencv-imgproc2.4 (>= 2.4.8), libopencv-flann2.4 (>= 2.4.8), libopencv-features2d2.4 (>= 2.4.8), libopencv-video2.4 (>= 2.4.8), libopencv-gpu2.4 (>=2.4.8), liblog4cplus-1.0-4, libcurl3, beanstalkd")

SET (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/../LICENSE")
SET (CPACK_PACKAGE_DESCRIPTION "OpenALPR - Open Source Automatic License Plate Recognition")
SET (CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenALPR is an open source Automatic License Plate Recognition library written in C++.  The library analyzes images and identifies license plates. The output is the text representation of any license plate characters found in the image.  Check out a live online demo here: http://www.openalpr.com/demo.html" )
SET (CPACK_PACKAGE_CONTACT "Matt Hill <matt@ndu.com>")
SET (CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CPACK_DEBIAN_ARCHITECTURE}")

SET (CPACK_COMPONENTS_ALL Libraries ApplicationData)

INCLUDE(CPack)

# ----------------------------------------------------------------------------
# Uninstall target, for "make uninstall"
# http://www.cmake.org/Wiki/CMake_FAQ#Can_I_do_.22make_uninstall.22_with_CMake.3F
# ----------------------------------------------------------------------------
CONFIGURE_FILE(
  "${CMAKE_MODULE_PATH}/templates/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  @ONLY)

ADD_CUSTOM_TARGET(uninstall COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
