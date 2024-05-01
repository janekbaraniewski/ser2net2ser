#include <iostream>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include "Logger.h"
#include "SerialClient.h"
#include "SerialServer.h"
#include "RealSerialPort.h"

using namespace boost::asio;
using namespace boost::program_options;
using std::string;
using std::cout;
using std::cerr;
using std::endl;

void setup_and_run_server(io_service& io, const variables_map& vm) {
    unsigned int baud_rate = vm["baud"].as<unsigned int>();  // Get baud rate as unsigned int
    SerialServer server(io, vm["device"].as<string>(), baud_rate);
    server.run();
}

void setup_and_run_client(io_service& io, const variables_map& vm) {
    SerialClient client(vm["server"].as<string>(), vm["port"].as<unsigned short>(), vm["vsp"].as<string>());
    client.run();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: ser2net2ser <command> [options]\n";
        return 1;
    }

    string command = argv[1];
    io_service io;

    try {
        if (command == "serve") {
            options_description serve_desc("Server options");
            serve_desc.add_options()
                ("help,h", "produce help message")
                ("device,d", value<string>()->default_value("/dev/ttyUSB0"), "Device name")
                ("baud,b", value<unsigned int>()->default_value(9600), "Baud rate")
                ("port,p", value<unsigned short>()->default_value(12345), "Port number");
            variables_map vm;
            store(parse_command_line(argc, argv, serve_desc), vm);

            if (vm.count("help")) {
                cout << serve_desc << endl;
                return 0;
            }

            notify(vm);
            setup_and_run_server(io, vm);
        } else if (command == "connect") {
            options_description connect_desc("Client options");
            connect_desc.add_options()
                ("help,h", "produce help message")
                ("server,s", value<string>()->default_value("127.0.0.1"), "Server IP address")
                ("port,p", value<unsigned short>()->default_value(12345), "Server port")
                ("vsp,v", value<string>()->required(), "Virtual serial port name");
            variables_map vm;
            store(parse_command_line(argc, argv, connect_desc), vm);

            if (vm.count("help")) {
                cout << connect_desc << endl;
                return 0;
            }


            notify(vm);
            setup_and_run_client(io, vm);
        } else {
            throw std::invalid_argument("Invalid command provided. Use 'serve' or 'connect'.");
        }
    } catch (const std::exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
