enable_testing()

ADD_EXECUTABLE( unittests test_api.cpp test_utility.cpp  )

TARGET_LINK_LIBRARIES(unittests

	openalpr

  )

add_test(unittests unittests)