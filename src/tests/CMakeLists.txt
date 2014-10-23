enable_testing()

ADD_EXECUTABLE( unittests runtests.cpp  )

TARGET_LINK_LIBRARIES(unittests

	openalpr

  )

add_test(unittests unittests)