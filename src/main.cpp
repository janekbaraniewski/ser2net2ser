#include <iostream>
#include <string>
#include <stdexcept>
#include "Logger.h"
#include "SerialClient.h"
#include "SerialPort.h"
#include "TCPServer.h"

using std::string;

void setup_and_run_server(const string& device, unsigned int baud_rate, unsigned int port) {
    SerialPort serialDevice(device, baud_rate);
    TcpServer server(port, serialDevice);
    try {
        server.run();
    } catch (const std::exception& e) {
        Logger(Logger::Level::Error) << "Error: " << e.what();
    }
}

void setup_and_run_client(const string& server_ip, unsigned short port, const string& vsp) {
    SerialClient client(server_ip, port, vsp);
    client.run();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        Logger(Logger::Level::Error)  << "Usage: ser2net2ser <command> [options]\n";
        return 1;
    }

    string command = argv[1];

    try {
        if (command == "serve") {
            string device = "/dev/ttyUSB0";
            unsigned int baud = 9600;
            unsigned int port = 12345;

            for (int i = 2; i < argc; i += 2) {
                string arg = argv[i];
                if (arg == "--device" || arg == "-d") {
                    if (i + 1 < argc) device = argv[i + 1];
                } else if (arg == "--baud" || arg == "-b") {
                    if (i + 1 < argc) baud = std::stoi(argv[i + 1]);
                } else if (arg == "--port" || arg == "-p") {
                    if (i + 1 < argc) port = std::stoi(argv[i + 1]);
                } else if (arg == "--help" || arg == "-h") {
                    Logger(Logger::Level::Info)  << "Usage: ser2net2ser serve [--device <dev>] [--baud <rate>] [--port <port>]\n";
                    return 0;
                }
            }

            setup_and_run_server(device, baud, port);
        } else if (command == "connect") {
            string server_ip = "127.0.0.1";
            unsigned short port = 12345;
            string vsp;

            for (int i = 2; i < argc; i += 2) {
                string arg = argv[i];
                if (arg == "--server" || arg == "-s") {
                    if (i + 1 < argc) server_ip = argv[i + 1];
                } else if (arg == "--port" || arg == "-p") {
                    if (i + 1 < argc) port = static_cast<unsigned short>(std::stoi(argv[i + 1]));
                } else if (arg == "--vsp" || arg == "-v") {
                    if (i + 1 < argc) vsp = argv[i + 1];
                } else if (arg == "--help" || arg == "-h") {
                    Logger(Logger::Level::Info)  << "Usage: ser2net2ser connect [--server <ip>] [--port <port>] --vsp <name>\n";
                    return 0;
                }
            }

            if (vsp.empty()) {
                Logger(Logger::Level::Error)  << "Virtual serial port name must be specified with --vsp.\n";
                return 1;
            }

            setup_and_run_client(server_ip, port, vsp);
        } else {
            throw std::invalid_argument("Invalid command provided. Use 'serve' or 'connect'.");
        }
    } catch (const std::exception& e) {
        Logger(Logger::Level::Error)  << "Error: " << e.what();
        return 1;
    }

    return 0;
}
