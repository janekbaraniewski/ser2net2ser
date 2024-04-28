cmake_minimum_required(VERSION 3.15)
project(SerialNetworkBridge LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boost libraries
find_package(Boost 1.65 REQUIRED COMPONENTS system program_options log_setup log date_time)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

if(TARGET GTest::gtest_main)
  message(STATUS "GTest::gtest_main found")
else()
  message(FATAL_ERROR "GTest::gtest_main not found")
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(logging src/logging.cpp)
target_include_directories(logging PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(logging PUBLIC
  Boost::log
  Boost::log_setup
  Boost::date_time
)

add_library(serial_server_lib src/logging.cpp src/server.cpp src/virtualserialport.cpp)
target_include_directories(serial_server_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(serial_server_lib PRIVATE
  Boost::system
  Boost::program_options
  Boost::log
  Boost::log_setup
  Boost::date_time
  logging
  pthread
)

add_executable(serial_server src/server.cpp)
target_link_libraries(serial_server PRIVATE
  serial_server_lib
  logging
  Boost::system
  Boost::program_options
  Boost::log
  Boost::log_setup
  Boost::date_time
)

add_executable(serial_client src/client.cpp)
target_link_libraries(serial_client PRIVATE
  serial_server_lib
  logging
  Boost::system
  Boost::program_options
  Boost::log
  Boost::log_setup
  Boost::date_time
)

# # TODO: REENABLE TESTS
# enable_testing()
# add_executable(test_app tests/test_server.cpp)
# target_compile_definitions(test_app PRIVATE UNIT_TEST)
# target_link_libraries(test_app PRIVATE
#   serial_server_lib  # this includes server.cpp
#   Boost::system
#   GTest::gtest_main
#   GTest::gtest
#   GTest::gmock
# )

# include(GoogleTest)
# gtest_discover_tests(test_app EXTRA_ARGS --gtest_color=yes)
