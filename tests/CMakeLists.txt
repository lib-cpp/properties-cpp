set (OLD_CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
# Don't treat warnings as errors in 3rd_party/{gmock,cucumber-cpp}
string (REPLACE " -Werror " " " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
find_package(Gtest REQUIRED)
include_directories(${GMOCK_INCLUDE_DIR} ${GTEST_INCLUDE_DIR})
set (CMAKE_CXX_FLAGS ${OLD_CMAKE_CXX_FLAGS})

include_directories(${GTEST_INCLUDE_DIRS})

add_executable(
  properties_test
  properties_test.cpp
)

add_executable(
  signals_test
  signals_test.cpp
)

target_link_libraries(
  properties_test

  ${GTEST_BOTH_LIBRARIES}
)

target_link_libraries(
  signals_test

  ${GTEST_BOTH_LIBRARIES}
)

add_test(properties_test ${CMAKE_CURRENT_BINARY_DIR}/properties_test)
add_test(signals_test ${CMAKE_CURRENT_BINARY_DIR}/signals_test)
