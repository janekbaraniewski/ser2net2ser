#include "SerialClient.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;

SerialClient::SerialClient(const std::string& server_ip, unsigned short server_port, const std::string& vsp_name)
    : vsp_(vsp_name) {
    Logger(Logger::Level::Info)  << "Initializing client...";
    Logger(Logger::Level::Info)  << "Connecting to server at " << server_ip << ":" << server_port;
    socketClient_.connectToServer(server_ip.c_str(), server_port);
    Logger(Logger::Level::Info)  << "Connected to server.";
    Logger(Logger::Level::Info)  << "Opening virtual serial port: " << vsp_name;
}

void SerialClient::run() {
    fd_set read_fds;
    char buffer[256];
    int max_fd = std::max(vsp_.master_fd_raw_, socketClient_.sock_fd) + 1;

    while (true) {
        FD_ZERO(&read_fds);
        FD_SET(vsp_.master_fd_raw_, &read_fds);
        FD_SET(socketClient_.sock_fd, &read_fds);

        if (select(max_fd, &read_fds, NULL, NULL, NULL) < 0 && errno != EINTR) {
            Logger(Logger::Level::Error) << "Select error: " << strerror(errno) << std::endl;
            break;
        }

        if (FD_ISSET(vsp_.master_fd_raw_, &read_fds)) {
            ssize_t bytes_read = vsp_.async_read(buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                Logger(Logger::Level::Info)  << "From PTY: " << buffer;
                socketClient_.sendToServer(buffer, bytes_read);
            } else if (bytes_read == 0) {
                Logger(Logger::Level::Info) << "PTY closed." << std::endl;
                break;
            } else {
                Logger(Logger::Level::Error) << "Error reading from PTY: " << strerror(errno) << std::endl;
            }
        }

        if (FD_ISSET(socketClient_.sock_fd, &read_fds)) {
            ssize_t bytes_read = socketClient_.receiveFromServer(buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                Logger(Logger::Level::Info) << "From Server: " << buffer;
                vsp_.async_write(buffer, bytes_read);
            } else if (bytes_read == 0) {
                Logger(Logger::Level::Info) << "Server closed connection." << std::endl;
                break;
            } else {
                Logger(Logger::Level::Error) << "Error reading from socket: " << strerror(errno) << std::endl;
            }
        }
    }
}
