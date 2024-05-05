#include <iostream>
#include <fcntl.h>
#include <cstring>
#include <errno.h>

#include "SerialServer.h"

SerialServer::SerialServer(const std::string& device, unsigned int baud_rate, unsigned int port)
    : serial_port_(device, baud_rate), server_(port, serial_port_) {}

SerialServer::~SerialServer() {
    server_.stop();
}

void SerialServer::run() {
    Logger(Logger::Level::Info) << "SerialServer::run";
    try {
        server_.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
