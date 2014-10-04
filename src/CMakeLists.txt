project(src)


cmake_minimum_required (VERSION 2.6)

# Set the OpenALPR version in cmake, and also add it as a DEFINE for the code to access
SET(OPENALPR_MAJOR_VERSION "1")
SET(OPENALPR_MINOR_VERSION "2")
SET(OPENALPR_PATCH_VERSION "0")
SET(OPENALPR_VERSION ${OPENALPR_MAJOR_VERSION}.${OPENALPR_MINOR_VERSION}.${OPENALPR_PATCH_VERSION})

add_definitions( -DOPENALPR_MAJOR_VERSION=${OPENALPR_MAJOR_VERSION})
add_definitions( -DOPENALPR_MINOR_VERSION=${OPENALPR_MINOR_VERSION})
add_definitions( -DOPENALPR_PATCH_VERSION=${OPENALPR_PATCH_VERSION})

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")


FIND_PACKAGE( Tesseract REQUIRED )

include_directories(${Tesseract_INCLUDE_DIRS})

# Discover OpenCV directory automatically
find_path(OpenCV_DIR
  NAMES OpenCVConfig.cmake
  HINTS ${CMAKE_SOURCE_DIR}/../libraries/opencv/)
		
# Opencv Package
FIND_PACKAGE( OpenCV REQUIRED )
IF (${OpenCV_VERSION} VERSION_LESS 2.4.7)
	MESSAGE(FATAL_ERROR "OpenCV version is not compatible : ${OpenCV_VERSION}")
ENDIF()

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

TARGET_LINK_LIBRARIES(alpr
	openalpr-static
	support
    video
	${OpenCV_LIBS}
	${Tesseract_LIBRARIES}
        ${Extra_LIBS}
  )

# Compile the alprd library on Unix-based OS
IF (NOT WIN32)
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
	
	# Don't include misc utilities for Windows
	add_subdirectory(misc_utilities)
ENDIF()
  

  

add_subdirectory(openalpr)
add_subdirectory(video)


install (TARGETS    alpr DESTINATION   /usr/bin)
install (TARGETS    alprd DESTINATION  /usr/bin)
install (FILES      ${CMAKE_SOURCE_DIR}/../doc/man/alpr.1 DESTINATION       /usr/share/man/man1 COMPONENT doc)
install (DIRECTORY  ${CMAKE_SOURCE_DIR}/../runtime_data DESTINATION         /usr/share/openalpr/)
install (FILES      ${CMAKE_SOURCE_DIR}/../config/openalpr.conf DESTINATION /etc/openalpr/      COMPONENT config)
install (FILES      ${CMAKE_SOURCE_DIR}/../config/alprd.conf DESTINATION    /etc/openalpr/      COMPONENT config)


SET(CPACK_PACKAGE_VERSION ${OPENALPR_VERSION})
SET (CPACK_SET_DESTDIR "on")
SET (CPACK_GENERATOR "DEB;TGZ")
SET (CPACK_PACKAGE_NAME "openalpr")
SET(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_SOURCE_DIR}/build;src;ALL;/")
SET(CPACK_CMAKE_GENERATOR "Unix Makefiles")

SET(CPACK_STRIP_FILES "1")
SET (CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
SET (CPACK_DEBIAN_PACKAGE_SECTION "video")
SET (CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
SET (CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.1.3), libgcc1 (>= 4.1.1), libtesseract3 (>= 3.0.3), libopencv-core2.4 (>= 2.4.8), libopencv-objdetect2.4 (>= 2.4.8), libopencv-highgui2.4 (>= 2.4.8), libopencv-imgproc2.4 (>= 2.4.8), libopencv-flann2.4 (>= 2.4.8), libopencv-features2d2.4 (>= 2.4.8), libzmq1, liblog4cplus-1.0-4, libcurl3, beanstalkd")

SET (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/../LICENSE")
SET (CPACK_PACKAGE_DESCRIPTION "OpenALPR - Open Source Automatic License Plate Recognition")
SET (CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenALPR is an open source Automatic License Plate Recognition library written in C++.  The library analyzes images and identifies license plates. The output is the text representation of any license plate characters found in the image.  Check out a live online demo here: http://www.openalpr.com/demo.html" )
SET (CPACK_PACKAGE_CONTACT "Matt Hill <matt@ndu.com>")
SET (CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CPACK_DEBIAN_ARCHITECTURE}")

SET (CPACK_COMPONENTS_ALL Libraries ApplicationData)

INCLUDE(CPack)
