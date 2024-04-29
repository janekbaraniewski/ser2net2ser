#include <iostream>
#include <boost/program_options.hpp>
#include "SerialServer.h"
#include "SerialClient.h"

using namespace boost::program_options;

int main(int argc, char* argv[]) {
    init_logging();
    try {
        options_description desc{"Options"};
        desc.add_options()
            ("help,h", "Display this help message")
            ("mode,m", value<std::string>(), "Mode: server or client")
            ("server,s", value<std::string>()->default_value("127.0.0.1"), "Server IP address (client mode)")
            ("port,p", value<unsigned short>()->default_value(12345), "Port number")
            ("device,d", value<std::string>()->default_value("/dev/ttyUSB0"), "Device name (server mode)")
            ("baud,b", value<unsigned int>()->default_value(9600), "Baud rate (server mode)")
            ("vsp,v", value<std::string>(), "Virtual serial port name (client mode)");

        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);

        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return 0;
        }

        notify(vm); // This will throw if 'mode' is not provided

        if (!vm.count("mode")) {
            throw std::invalid_argument("Mode not specified. Use --mode=server or --mode=client.");
        }

        std::string mode = vm["mode"].as<std::string>();
        if (mode == "server") {
            // Initialize and run server
        } else if (mode == "client") {
            // Initialize and run client
        } else {
            throw std::invalid_argument("Invalid mode. Use 'server' or 'client'.");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
