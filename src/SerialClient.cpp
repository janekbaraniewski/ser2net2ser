#include "SerialClient.h"

using namespace boost::asio;
using namespace boost::program_options;
using ip::tcp;
using std::string;
using std::cout;
using std::cerr;
using std::endl;

SerialClient::SerialClient(boost::asio::io_service& io_service, const std::string& server_ip, unsigned short server_port, const std::string& vsp_name)
    : vsp_(io_service, vsp_name) {
    std::cout << "Initializing client...";
    std::cout << "Connecting to server at " << server_ip << ":" << server_port;
    socketClient_.connectToServer(server_ip.c_str(), server_port);
    std::cout << "Connected to server.";
    std::cout << "Opening virtual serial port: " << vsp_name;
}

void SerialClient::run() {
    fd_set read_fds;
    char buffer[256];
    int max_fd = std::max(vsp_.master_fd_.native_handle(), socketClient_.sock_fd) + 1;

    while (true) {
        FD_ZERO(&read_fds);
        FD_SET(vsp_.master_fd_.native_handle(), &read_fds);
        FD_SET(socketClient_.sock_fd, &read_fds);

        if (select(max_fd, &read_fds, NULL, NULL, NULL) < 0 && errno != EINTR) {
            std::cerr << "Select error: " << strerror(errno) << std::endl;
            break;
        }

        if (FD_ISSET(vsp_.master_fd_.native_handle(), &read_fds)) {
            ssize_t bytes_read = vsp_.async_read(buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                std::cout << "From PTY: " << buffer;
                socketClient_.sendToServer(buffer, bytes_read);
            } else if (bytes_read == 0) {
                std::cout << "PTY closed." << std::endl;
                break;
            } else {
                std::cerr << "Error reading from PTY: " << strerror(errno) << std::endl;
            }
        }

        if (FD_ISSET(socketClient_.sock_fd, &read_fds)) {
            ssize_t bytes_read = socketClient_.receiveFromServer(buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                std::cout << "From Server: " << buffer;
                vsp_.async_write(buffer, bytes_read);
            } else if (bytes_read == 0) {
                std::cout << "Server closed connection." << std::endl;
                break;
            } else {
                std::cerr << "Error reading from socket: " << strerror(errno) << std::endl;
            }
        }
    }
}
