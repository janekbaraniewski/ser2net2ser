#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class PTYSimulator {
public:
    int master_fd, slave_fd;
    char* slave_name;

    PTYSimulator() : master_fd(-1), slave_fd(-1), slave_name(nullptr) {}

    ~PTYSimulator() {
        close(slave_fd);
        close(master_fd);
    }

    bool setup() {
        master_fd = posix_openpt(O_RDWR | O_NOCTTY);
        if (master_fd == -1 || grantpt(master_fd) == -1 || unlockpt(master_fd) == -1 || (slave_name = ptsname(master_fd)) == nullptr) {
            std::cerr << "Failed to initialize PTY: " << strerror(errno) << std::endl;
            return false;
        }

        std::cout << "Slave PTY name: " << slave_name << std::endl;

        // Attempt to create a symbolic link from slave_name to "/dev/ttyUSB0"
        if (symlink(slave_name, "/dev/ttyUSB0") == -1) {
            std::cerr << "Failed to create symlink for PTY slave: " << strerror(errno) << std::endl;
            return false;
        }

        std::cout << "Symlink for PTY slave created successfully" << std::endl;

        slave_fd = open(slave_name, O_RDWR);
        if (slave_fd == -1) {
            std::cerr << "Error opening slave PTY: " << strerror(errno) << std::endl;
            return false;
        }

        return true;
    }

    ssize_t readFromPTY(char* buffer, size_t bufferSize) {
        return read(master_fd, buffer, bufferSize);
    }

    void writeToPTY(const char* buffer, size_t bufferSize) {
        write(master_fd, buffer, bufferSize);
    }
};

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
        sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_fd == -1) {
            std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
            return false;
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(server_port);
        if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
            std::cerr << "Invalid address/ Address not supported" << std::endl;
            return false;
        }

        if (::connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Connection Failed: " << strerror(errno) << std::endl;
            return false;
        }

        std::cout << "Connected to server at " << server_ip << ":" << server_port << std::endl;
        return true;
    }

    ssize_t sendToServer(const char* buffer, size_t bufferSize) {
        return send(sock_fd, buffer, bufferSize, 0);
    }

    ssize_t receiveFromServer(char* buffer, size_t bufferSize) {
        return recv(sock_fd, buffer, bufferSize, 0);
    }
};

int main() {
    PTYSimulator pty;
    SocketClient socketClient;

    if (!pty.setup() || !socketClient.connectToServer("192.168.1.29", 3333)) {
        return 1;
    }

    fd_set read_fds;
    char buffer[256];
    int max_fd = std::max(pty.master_fd, socketClient.sock_fd) + 1;

    while (true) {
        FD_ZERO(&read_fds);
        FD_SET(pty.master_fd, &read_fds);
        FD_SET(socketClient.sock_fd, &read_fds);

        if (select(max_fd, &read_fds, NULL, NULL, NULL) < 0 && errno != EINTR) {
            std::cerr << "Select error: " << strerror(errno) << std::endl;
            break;
        }

        if (FD_ISSET(pty.master_fd, &read_fds)) {
            ssize_t bytes_read = pty.readFromPTY(buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                std::cout << "From PTY: " << buffer;
                socketClient.sendToServer(buffer, bytes_read);
            } else if (bytes_read == 0) {
                std::cout << "PTY closed." << std::endl;
                break;
            } else {
                std::cerr << "Error reading from PTY: " << strerror(errno) << std::endl;
            }
        }

        if (FD_ISSET(socketClient.sock_fd, &read_fds)) {
            ssize_t bytes_read = socketClient.receiveFromServer(buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                std::cout << "From Server: " << buffer;
                pty.writeToPTY(buffer, bytes_read);
            } else if (bytes_read == 0) {
                std::cout << "Server closed connection." << std::endl;
                break;
            } else {
                std::cerr << "Error reading from socket: " << strerror(errno) << std::endl;
            }
        }
    }

    return 0;
}
