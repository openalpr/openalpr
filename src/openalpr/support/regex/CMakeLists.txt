
set(regex_source_files
regsyntax.c
regposerr.c
regcomp.c
reggnu.c
regerror.c
regext.c
testc.c
regversion.c
regparse.c
regenc.c
st.c
regposix.c
testu.c
regexec.c
regtrav.c
)

add_library(onig_regex STATIC ${regex_source_files})

SET_TARGET_PROPERTIES( onig_regex PROPERTIES COMPILE_FLAGS -fPIC)