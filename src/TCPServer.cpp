#include "TCPServer.h"

TcpServer::TcpServer(int port, SerialPort& serial) : port_(port), serial_(serial), is_running_(false) {
    Logger(Logger::Level::Info) << "TCPServer init start, creating server on port " << port_;
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
    Logger(Logger::Level::Info) << "TCPServer init finish";
}

TcpServer::~TcpServer() {
    if (is_running_) {
        stop();
    }
}

void TcpServer::run() {
    is_running_ = true;
    Logger(Logger::Level::Info) << "TCPServer start listening";
    while (is_running_) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(server_fd_, (struct sockaddr *)&clientAddr, &clientLen);
        if (clientSocket < 0) {
            continue;
        }

        std::thread clientThread(&TcpServer::handleClient, this, clientSocket);
        clientThread.detach();
    }
}

void TcpServer::stop() {
    close(server_fd_);
    is_running_ = false;
}

void TcpServer::handleClient(int client_socket) {
    char buffer[1024];
    Logger(Logger::Level::Info) << "HandleClient - start serial reading thread.";
    serial_.startReading();
    std::thread readThread([&]() {
        while (true) {
            std::string response = serial_.readData();
            Logger(Logger::Level::Info) << "HandleClient - write: " << response;
            if (!response.empty()) {
                write(client_socket, response.c_str(), response.size());
            }
        }
    });

    Logger(Logger::Level::Info) << "HandleClient - start read data from client";
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytesReceived = read(client_socket, buffer, sizeof(buffer));
        if (bytesReceived <= 0) {
            break;
        }
        Logger(Logger::Level::Info) << "HandleClient - read: " << buffer;
        serial_.writeData(std::string(buffer, bytesReceived));
    }

    serial_.stopReading();
    readThread.join();
    close(client_socket);
}
