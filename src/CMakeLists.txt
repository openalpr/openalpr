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


SET(OpenCV_DIR "${CMAKE_SOURCE_DIR}/../libraries/opencv/")
#SET(Tesseract_DIR "${CMAKE_SOURCE_DIR}/../libraries/tesseract-ocr")

include_directories(
	${Tesseract_DIR}/api
	${Tesseract_DIR}/ccutil/
	${Tesseract_DIR}/ccstruct/
	${Tesseract_DIR}/ccmain/
)

IF (WIN32)
	add_definitions( -DWINDOWS)
	add_definitions( -DNOMINMAX)

	link_directories( ${Tesseract_DIR}/lib/ )
	include_directories(${Tesseract_DIR}/include/ )
	
	# Extra linker dependencies for Windows
	SET(Tesseract_LIBS 		
		libtesseract303-static
		liblept170
		ws2_32.lib)
ELSE()

	link_directories( ${Tesseract_DIR}/api/.libs/ )
	
	SET(Tesseract_LIBS tesseract)
ENDIF()


# Opencv Package
FIND_PACKAGE( OpenCV REQUIRED )
IF (${OpenCV_VERSION} VERSION_LESS 2.4.8)
	MESSAGE(FATAL_ERROR "OpenCV version is not compatible : ${OpenCV_VERSION}")
ENDIF()


include_directories(./openalpr ) 


set(CMAKE_CSS_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall ")
ADD_EXECUTABLE( alpr  main.cpp )

TARGET_LINK_LIBRARIES(alpr
	openalpr-static
	support
        video
	${OpenCV_LIBS}
	${Tesseract_LIBS}
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
	  ${Tesseract_LIBS}
    )
ENDIF()
  

  

add_subdirectory(openalpr)
add_subdirectory(video)

IF (NOT WIN32)
	# Don't include misc utilities for Windows
	add_subdirectory(misc_utilities)
ENDIF()



install (TARGETS alpr DESTINATION bin)
install (FILES ${CMAKE_SOURCE_DIR}/../doc/man/alpr.1 DESTINATION share/man/man1 COMPONENT doc)
install (DIRECTORY ${CMAKE_SOURCE_DIR}/../runtime_data DESTINATION share/openalpr/)
install (FILES ${CMAKE_SOURCE_DIR}/../config/openalpr.conf DESTINATION /etc/openalpr/ COMPONENT config)



INCLUDE(CPack)
