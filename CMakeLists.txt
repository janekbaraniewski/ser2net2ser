cmake_minimum_required(VERSION 3.15)
project(SerialNetworkBridge)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boost libraries
find_package(Boost 1.65 REQUIRED COMPONENTS system)

# Include directories
include_directories(${Boost_INCLUDE_DIR})
include_directories("include")

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Define the server executable
add_executable(serial_server src/server.cpp)
target_link_libraries(serial_server ${Boost_LIBRARIES} pthread)

# Define the client executable
add_executable(serial_client src/client.cpp)
target_link_libraries(serial_client ${Boost_LIBRARIES} pthread)

enable_testing()

add_executable(
  test_app
  tests/test_server.cpp
)
target_link_libraries(
  test_app
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_app)