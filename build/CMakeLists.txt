cmake_minimum_required(VERSION 3.15)
project(SerialNetworkBridge)

set(CMAKE_CXX_STANDARD 17)

# Find Boost libraries
find_package(Boost 1.65 REQUIRED COMPONENTS system)

# Include directories
include_directories(${Boost_INCLUDE_DIR})
include_directories("../include")

# Define the server executable
add_executable(serial_server ../src/server.cpp)
target_link_libraries(serial_server ${Boost_LIBRARIES} pthread)

# Define the client executable
add_executable(serial_client ../src/client.cpp)
target_link_libraries(serial_client ${Boost_LIBRARIES} pthread)
