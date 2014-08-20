project(src)


cmake_minimum_required (VERSION 2.6)

# Set the OpenALPR version in cmake, and also add it as a DEFINE for the code to access
SET(OPENALPR_MAJOR_VERSION "1")
SET(OPENALPR_MINOR_VERSION "1")
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
IF (${OpenCV_VERSION} VERSION_LESS 2.4.8)
	MESSAGE(FATAL_ERROR "OpenCV version is not compatible : ${OpenCV_VERSION}")
ENDIF()

include_directories(./openalpr )

IF (WIN32)
	add_definitions( -DWINDOWS)
	add_definitions( -DNOMINMAX)

	# Extra linker dependencies for Windows
	SET (Extra_LIBS
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
  ADD_EXECUTABLE( alprd  daemon.cpp daemon/beanstalk.c daemon/beanstalk.cc daemon/uuid.cpp )

  TARGET_LINK_LIBRARIES(alprd
	  openalpr
	  support
      video
	  uuid
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


install (TARGETS alpr DESTINATION bin)
install (FILES ${CMAKE_SOURCE_DIR}/../doc/man/alpr.1 DESTINATION share/man/man1 COMPONENT doc)
install (DIRECTORY ${CMAKE_SOURCE_DIR}/../runtime_data DESTINATION share/openalpr/)
install (FILES ${CMAKE_SOURCE_DIR}/../config/openalpr.conf DESTINATION /etc/openalpr/ COMPONENT config)



INCLUDE(CPack)
