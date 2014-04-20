project(src)


cmake_minimum_required (VERSION 2.6)

SET(OpenCV_DIR "${CMAKE_SOURCE_DIR}/../libraries/opencv/")
SET(Tesseract_DIR "${CMAKE_SOURCE_DIR}/../libraries/tesseract-ocr")

include_directories(
	${Tesseract_DIR}/api
	${Tesseract_DIR}/ccutil/
	${Tesseract_DIR}/ccstruct/
	${Tesseract_DIR}/ccmain/
)

IF (WIN32)
	add_definitions( -DWINDOWS)
	add_definitions( -DNOMINMAX)

	link_directories( ${Tesseract_DIR}/vs2008/LIB_Release/ )
ELSE()

	link_directories( ${Tesseract_DIR}/api/.libs/ )
ENDIF()


# Opencv Package
FIND_PACKAGE( OpenCV REQUIRED )
IF (${OpenCV_VERSION} VERSION_LESS 2.4.8)
	MESSAGE(FATAL_ERROR "OpenCV version is not compatible : ${OpenCV_VERSION}")
ENDIF()


include_directories(./openalpr )


set(CMAKE_CSS_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall ")
ADD_EXECUTABLE( alpr  main.cpp  )


IF (WIN32)
	# Extra linker dependencies for Windows
	TARGET_LINK_LIBRARIES(alpr
		openalpr
		support
		${OpenCV_LIBS}
		libtesseract302-static
		liblept168
		liblept168-static-mtdll
		libpng143-static-mtdll
		libjpeg8c-static-mtdll
		giflib416-static-mtdll
		libtiff394-static-mtdll
		zlib125-static-mtdll
		ws2_32.lib
	  )

ELSE()

	TARGET_LINK_LIBRARIES(alpr
		openalpr
		support
		${OpenCV_LIBS}
		tesseract
	  )
ENDIF()

add_subdirectory(openalpr)
add_subdirectory(misc_utilities)

install (TARGETS alpr DESTINATION bin)
install (FILES openalpr/alpr.h DESTINATION include)
install (FILES ${CMAKE_BINARY_DIR}/openalpr/libopenalpr.so DESTINATION lib)