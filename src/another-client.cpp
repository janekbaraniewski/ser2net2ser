#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
    int master_fd, slave_fd;
    int sock_fd;
    struct sockaddr_in server_addr;
    char *slave_name;
    char server_ip[] = "192.168.1.29"; // Example IP address
    int server_port = 3333; // Example port

    // Create a pseudoterminal
    master_fd = posix_openpt(O_RDWR | O_NOCTTY);
    if (master_fd == -1) {
        std::cerr << "Error opening PTY: " << strerror(errno) << std::endl;
        return 1;
    }

    if (grantpt(master_fd) == -1 || unlockpt(master_fd) == -1 || (slave_name = ptsname(master_fd)) == nullptr) {
        std::cerr << "Failed to initialize PTY: " << strerror(errno) << std::endl;
        close(master_fd);
        return 1;
    }

    std::cout << "Slave PTY name: " << slave_name << std::endl;

    // Attempt to create a symbolic link from slave_name to "/dev/ttyUSB0"
    if (symlink(slave_name, "/dev/ttyUSB0") == -1) {
        std::cerr << "Failed to create symlink for PTY slave: " << strerror(errno) << std::endl;
        close(master_fd);
        return 1;
    }
    std::cout << "Symlink for PTY slave created successfully" << std::endl;

    // Open the slave pseudoterminal
    slave_fd = open(slave_name, O_RDWR);
    if (slave_fd == -1) {
        std::cerr << "Error opening slave PTY: " << strerror(errno) << std::endl;
        close(master_fd);
        return 1;
    }

    // Set up the socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
        close(slave_fd);
        close(master_fd);
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        close(sock_fd);
        close(slave_fd);
        close(master_fd);
        return 1;
    }

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection Failed: " << strerror(errno) << std::endl;
        close(sock_fd);
        close(slave_fd);
        close(master_fd);
        return 1;
    }

    std::cout << "Connected to server at " << server_ip << ":" << server_port << std::endl;

    // Communication loop
    fd_set read_fds;
    char buffer[256];
    int max_fd = std::max(master_fd, sock_fd) + 1;
    while (true) {
        FD_ZERO(&read_fds);
        FD_SET(master_fd, &read_fds);
        FD_SET(sock_fd, &read_fds);

        int activity = select(max_fd, &read_fds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            std::cerr << "Select error: " << strerror(errno) << std::endl;
            break;
        }

        if (FD_ISSET(master_fd, &read_fds)) {
            ssize_t bytes_read = read(master_fd, buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                std::cout << "From PTY: " << buffer;
                send(sock_fd, buffer, bytes_read, 0); // Send to network
            } else if (bytes_read == 0) {
                std::cout << "PTY closed." << std::endl;
                break;
            } else {
                std::cerr << "Error reading from PTY: " << strerror(errno) << std::endl;
            }
        }

        if (FD_ISSET(sock_fd, &read_fds)) {
            ssize_t bytes_read = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                std::cout << "From Server: " << buffer;
                write(master_fd, buffer, bytes_read); // Send back to PTY
            } else if (bytes_read == 0) {
                std::cout << "Server closed connection." << std::endl;
                break;
            } else {
                std::cerr << "Error reading from socket: " << strerror(errno) << std::endl;
            }
        }
    }

    close(sock_fd);
    close(slave_fd);
    close(master_fd);
    return 0;
}