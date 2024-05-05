#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <cstring>
#include <stdexcept>

#include "TCPServer.h"

TcpServer::TcpServer(int port, SerialPort& serial) : port_(port), is_running_(false), serial_(serial) {
    Logger(Logger::Level::Info) << "Init tcp server";
    if ((server_fd_ = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        throw std::runtime_error("Socket creation failed");
    }

    int opt = 1;
    if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        throw std::runtime_error("Setsockopt failed");
    }

    address_.sin_family = AF_INET;
    address_.sin_addr.s_addr = INADDR_ANY;
    address_.sin_port = htons(port);

    if (bind(server_fd_, (struct sockaddr*)&address_, sizeof(address_)) < 0) {
        throw std::runtime_error("Bind failed");
    }

    if (listen(server_fd_, 3) < 0) {
        throw std::runtime_error("Listen failed");
    }
    Logger(Logger::Level::Info) << "Init tcp server finished";
}

TcpServer::~TcpServer() {
    if (is_running_) {
        stop();
    }
}

void TcpServer::run() {
    Logger(Logger::Level::Info) << "Start tcp server";
    is_running_ = true;
    while (is_running_) {
        int addrlen = sizeof(address_);
        int new_socket = accept(server_fd_, (struct sockaddr *)&address_, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            throw std::runtime_error("Accept failed");
        }
        std::thread clientThread(handleClient, new_socket, std::ref(serial_));
        clientThread.detach();
    }
}

void TcpServer::stop() {
    close(server_fd_);
    is_running_ = false;
}

void TcpServer::handleClient(int client_socket, SerialPort& serial) {
    const int bufferSize = 1024;
    char buffer[bufferSize];

    while (true) {
        memset(buffer, 0, bufferSize);
        Logger(Logger::Level::Info) << "Reading client connection";
        int read_size = read(client_socket, buffer, bufferSize - 1);
        if (read_size <= 0) {
            Logger(Logger::Level::Info) << "Client disconnected or error";
            break; // Client disconnected or error
        }

        serial.writeData(std::string(buffer, read_size));
        std::string response = serial.readData();
        write(client_socket, response.c_str(), response.size());
    }

    close(client_socket);
}
