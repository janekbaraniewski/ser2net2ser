#ifndef SOCKETCLIENT_H
#define SOCKETCLIENT_H

#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>

class SocketClient {
    struct sockaddr_in server_addr;

public:
    int sock_fd;
    SocketClient() : sock_fd(-1) {
        memset(&server_addr, 0, sizeof(server_addr));
    }

    ~SocketClient() {
        close(sock_fd);
    }

    bool connectToServer(const char* server_ip, int server_port) {
        Logger(Logger::Level::Info) << "Start connecting to socet server:" << server_ip << ":" << server_port;
        sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_fd == -1) {
            Logger(Logger::Level::Error) << "Error creating socket: " << strerror(errno);
            return false;
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(server_port);
        if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
            Logger(Logger::Level::Error) << "Invalid address/ Address not supported";
            return false;
        }

        if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            Logger(Logger::Level::Error) << "Connection Failed: " << strerror(errno);
            exit(1);
            return false;
        }

        Logger(Logger::Level::Info) << "Connected to server at " << server_ip << ":" << server_port;
        return true;
    }

    ssize_t sendToServer(const char* buffer, size_t bufferSize) {
        return send(sock_fd, buffer, bufferSize, 0);
    }

    ssize_t receiveFromServer(char* buffer, size_t bufferSize) {
        return recv(sock_fd, buffer, bufferSize, 0);
    }
};

#endif // CLIENT_H
