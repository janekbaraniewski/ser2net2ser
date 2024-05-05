#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>

void handleClient(int clientSocket) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytesReceived = read(clientSocket, buffer, sizeof(buffer));
        if (bytesReceived <= 0) {
            std::cerr << "Read error or connection closed by client" << std::endl;
            break;
        }

        std::cout << "Received: " << buffer << std::endl;
        write(clientSocket, buffer, bytesReceived);
    }

    close(clientSocket);
}

int main() {
    const int port = 3333;
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        std::cerr << "Cannot open socket" << std::endl;
        return 1;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Listen on all network interfaces
    serverAddr.sin_port = htons(port);

    if (bind(serverFd, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Cannot bind to port " << port << std::endl;
        return 1;
    }

    if (listen(serverFd, 10) < 0) { // Listen for up to 10 connections
        std::cerr << "Listen failed" << std::endl;
        return 1;
    }

    std::cout << "Server is listening on port " << port << std::endl;

    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverFd, (struct sockaddr *) &clientAddr, &clientLen);
        if (clientSocket < 0) {
            std::cerr << "Cannot accept connection" << std::endl;
            continue;
        }

        std::thread clientThread(handleClient, clientSocket);
        clientThread.detach(); // Detach the thread to handle multiple clients
    }

    close(serverFd);
    return 0;
}
