

set(video_source_files
 videobuffer.cpp

)

add_library(video STATIC ${video_source_files})

  TARGET_LINK_LIBRARIES(video
	  support
    )

#SET_TARGET_PROPERTIES( video PROPERTIES COMPILE_FLAGS -fPIC)