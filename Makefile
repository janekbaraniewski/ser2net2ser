CXX = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude
LDFLAGS = -L/usr/local/lib -lboost_system -pthread

# Define the source directory and build directory
SRC_DIR = src
BUILD_DIR = build

# Define targets
all: serial_server serial_client

serial_server: $(SRC_DIR)/server.cpp
    $(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/$@ $< $(LDFLAGS)

serial_client: $(SRC_DIR)/client.cpp
    $(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/$@ $< $(LDFLAGS)

clean:
    rm -f $(BUILD_DIR)/serial_server $(BUILD_DIR)/serial_client

.PHONY: all clean
