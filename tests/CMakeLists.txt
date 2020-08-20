set (OLD_CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
# Don't treat warnings as errors in 3rd_party/{gmock,cucumber-cpp}
string (REPLACE " -Werror " " " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
find_package(Gtest REQUIRED)
include(GoogleTest)

set (CMAKE_CXX_FLAGS ${OLD_CMAKE_CXX_FLAGS})

add_executable(
  properties_test
  properties_test.cpp
)
target_include_directories(
  properties_test
  PRIVATE ${GMOCK_INCLUDE_DIR} ${GTEST_INCLUDE_DIR}
)
target_link_libraries(properties_test 
    gtest gmock 
    gtest_main
)
gtest_discover_tests(properties_test)

add_executable(
  signals_test
  signals_test.cpp
)
target_include_directories(
  signals_test
  PRIVATE ${GMOCK_INCLUDE_DIR} ${GTEST_INCLUDE_DIR}
)
target_link_libraries(signals_test 
    gtest gmock 
    gtest_main
)
gtest_discover_tests(signals_test)