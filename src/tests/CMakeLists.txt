enable_testing()

ADD_EXECUTABLE( unittests 
  test_api.cpp 
  test_utility.cpp
  test_config.cpp
  test_regex.cpp
)

TARGET_LINK_LIBRARIES(unittests

	openalpr

  )

add_test(unittests unittests)

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})