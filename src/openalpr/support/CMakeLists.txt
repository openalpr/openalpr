

set(support_source_files
 filesystem.cpp
 timing.cpp
 tinythread.cpp
 platform.cpp
 utf8.cpp
)

set(regex_source_files
regex/regsyntax.c
regex/regposerr.c
regex/regcomp.c
regex/reggnu.c
regex/regerror.c
regex/regext.c
regex/regversion.c
regex/regparse.c
regex/regenc.c
regex/st.c
regex/regposix.c
regex/regexec.c
regex/regtrav.c
regex/ascii.c
regex/unicode.c
regex/utf8.c
)


add_library(support STATIC 
  ${support_source_files} 
  ${regex_source_files}
)

SET_TARGET_PROPERTIES( support PROPERTIES COMPILE_FLAGS -fPIC)