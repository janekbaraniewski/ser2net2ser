cmake_minimum_required(VERSION 3.15)
project(SerialNetworkBridge LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# Create core library without Logger.cpp, which is now in the logging library
add_library(core_lib
  src/Logger.cpp
  src/RealSerialPort.cpp
  src/SerialServer.cpp
  src/SerialClient.cpp
  src/VirtualSerialPort.cpp
  src/SerialPort.cpp
  src/TCPServer.cpp
)
target_include_directories(core_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(core_lib PUBLIC
  pthread
)

# Create the executable
add_executable(ser2net2ser src/main.cpp)
target_link_libraries(ser2net2ser PRIVATE
  core_lib
)
