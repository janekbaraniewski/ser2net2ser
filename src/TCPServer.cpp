#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <cstring>
#include <stdexcept>

#include "TCPServer.h"

TcpServer::TcpServer(int port, SerialPort& serial) : port_(port), is_running_(false), serial_(serial) {
    Logger(Logger::Level::Info) << "Init tcp server on port " << port_;
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
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(server_fd_, (struct sockaddr *) &clientAddr, &clientLen);
        if (clientSocket < 0) {
            Logger(Logger::Level::Info) << "Cannot accept connection";
            continue;
        }

        std::thread clientThread(handleClient, clientSocket);
        clientThread.detach(); // Detach the thread to handle multiple clients
    }

}

void TcpServer::stop() {
    close(server_fd_);
    is_running_ = false;
}

void TcpServer::handleClient(int client_socket, SerialPort& serial) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        Logger(Logger::Level::Info) << "Reading from client connection";
        ssize_t bytesReceived = read(client_socket, buffer, sizeof(buffer));
        if (bytesReceived <= 0) {
            Logger(Logger::Level::Info) << "Client disconnected or error";
            break;
        }

        serial.writeData(std::string(buffer, bytesReceived));
        std::string response = serial.readData();
        write(client_socket, response.c_str(), response.size());
    }

    close(client_socket);
}
