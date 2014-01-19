project(src)


cmake_minimum_required (VERSION 2.6) 

IF (WIN32)
	add_definitions( -DWINDOWS)
	add_definitions( -DNOMINMAX)
	
	SET(OpenCV_DIR "C:\\projects\\openalpr\\libraries\\opencv")
	SET(Tesseract_DIR "C:\\projects\\openalpr\\libraries\\tesseract-ocr")
	
	include_directories(
		${Tesseract_DIR}/api
		${Tesseract_DIR}/ccutil/
		${Tesseract_DIR}/ccstruct/
		${Tesseract_DIR}/ccmain/
	)
	link_directories( ${Tesseract_DIR}/vs2008/LIB_Release/ ) 
ELSE()

	SET(OpenCV_DIR "../libraries/opencv/")
	SET(Tesseract_DIR "/home/mhill/projects/alpr/libraries/tesseract-ocr")
		
	include_directories(
		${Tesseract_DIR}/api
		${Tesseract_DIR}/ccutil/
		${Tesseract_DIR}/ccstruct/
		${Tesseract_DIR}/ccmain/
		
	)
	link_directories( ${Tesseract_DIR}/api/.libs/ ) 
ENDIF()


# Opencv Package
FIND_PACKAGE( OpenCV REQUIRED )
IF (${OpenCV_VERSION} VERSION_LESS 2.3.0)
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

