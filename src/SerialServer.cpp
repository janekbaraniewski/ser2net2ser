#include "SerialServer.h"
#include <iostream>
#include <fcntl.h>
#include <cstring>
#include <errno.h>

SerialServer::SerialServer(const std::string& device, unsigned int baud_rate, unsigned int port)
    : serial_port_(device, baud_rate) {
    struct sockaddr_in serv_addr;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        std::cerr << "ERROR opening socket";
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "ERROR on binding: " << strerror(errno);
        exit(1);
    }

    listen(server_sock, 5);
}

SerialServer::~SerialServer() {
    close(server_sock);
}

void SerialServer::run() {
    std::cout << "SerialServer::run";
    start_accept();
}

void SerialServer::start_accept() {
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    client_sock = accept(server_sock, (struct sockaddr *)&cli_addr, &clilen);
    if (client_sock < 0) {
        std::cerr << "ERROR on accept: " << strerror(errno);
        return;
    }
    handle_session(client_sock);
}

void SerialServer::handle_session(int client_sock) {
    std::cout << "SerialServer::handle_session";
    async_read_socket(client_sock);
    async_read_serial(client_sock);
}

void SerialServer::async_read_socket(int client_sock) {
    std::cout << "SerialServer::async_read_socket";
    ssize_t length = read(client_sock, buffer_.data(), buffer_.size());

    if (length > 0) {
        serial_port_.async_write(buffer_.data(), length);
    } else if (length < 0) {
        std::cerr << "Read error on socket: " << strerror(errno);
        close(client_sock);
    }
}

void SerialServer::async_read_serial(int client_sock) {
    std::cout << "SerialServer::async_read_serial";
    ssize_t length = serial_port_.async_read_some(buffer_.data(), buffer_.size());

    if (length > 0) {
        write(client_sock, buffer_.data(), length);
    } else if (length < 0) {
        std::cerr << "Read error on serial port: " << strerror(errno);
        close(client_sock);
    }
}
