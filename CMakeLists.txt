cmake_minimum_required(VERSION 3.15)
project(SerialNetworkBridge LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boost libraries
find_package(Boost 1.65 REQUIRED COMPONENTS system program_options)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

if(TARGET GTest::gtest_main)
  message(STATUS "GTest::gtest_main found")
else()
  message(FATAL_ERROR "GTest::gtest_main not found")
endif()


# Define the executable for the server
add_executable(serial_server src/server.cpp)
target_include_directories(serial_server PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(serial_server PRIVATE
  Boost::system
  Boost::program_options
  pthread
)


# Define the executable for the client
add_executable(serial_client src/client.cpp)
target_include_directories(serial_client PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(serial_client PRIVATE
  Boost::system
  Boost::program_options
  pthread
)

# Enable testing
enable_testing()

# Add a test executable
add_executable(test_app tests/test_server.cpp)
target_link_libraries(test_app PRIVATE
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_app)
