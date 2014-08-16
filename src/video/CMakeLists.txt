

set(video_source_files
 videobuffer.cpp

)

add_library(video STATIC ${video_source_files})

#SET_TARGET_PROPERTIES( video PROPERTIES COMPILE_FLAGS -fPIC)