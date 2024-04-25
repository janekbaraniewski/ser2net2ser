cmake_minimum_required(VERSION 3.15)
project(SerialNetworkBridge LANGUAGES CXX)

# Set the C++ standard for the project
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find Boost libraries
find_package(Boost 1.65 REQUIRED COMPONENTS system)

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.10.0  # Prefer using tagged releases where possible
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)  # Use the MSVC runtime library settings
FetchContent_MakeAvailable(googletest)

# Define the executable for the server
add_executable(serial_server src/server.cpp)
target_include_directories(serial_server PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(serial_server PRIVATE
  Boost::system
  pthread
)

# Define the executable for the client
add_executable(serial_client src/client.cpp)
target_include_directories(serial_client PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(serial_client PRIVATE
  Boost::system
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
