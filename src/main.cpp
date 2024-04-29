#include <iostream>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include "logging.h"
#include "SerialClient.h"
#include "SerialServer.h"
#include "RealSerialPort.h"

using namespace boost::asio;
using namespace boost::program_options;
using std::string;
using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char* argv[]) {
    init_logging();

    // Declare a top-level description
    options_description global("Global options");
    global.add_options()
        ("help,h", "produce help message")
        ("mode,m", value<string>(), "mode to use (client or server)");

    // Command line parsed, considering first positional argument as the mode
    variables_map vm;
    store(parse_command_line(argc, argv, global), vm);
    notify(vm);

    if (vm.count("help") && !vm.count("mode")) {
        cout << "Usage: ser2net2ser [mode] [options]\n";
        cout << global;
        return 1;
    }

    string mode = vm["mode"].as<string>();

    if (mode == "client") {
        // Options for the client mode
        options_description client_desc("Client options");
        client_desc.add_options()
            ("help", "Display this help message")
            ("server,s", value<string>()->default_value("127.0.0.1"), "Server IP address")
            ("port,p", value<unsigned short>()->default_value(12345), "Server port")
            ("vsp,v", value<string>()->default_value("ttyUSB0"), "Virtual serial port name");

        variables_map client_vm;
        store(command_line_parser(argc, argv).options(client_desc).allow_unregistered().run(), client_vm);
        notify(client_vm);

        if (client_vm.count("help")) {
            cout << client_desc << endl;
            return 0;
        }

        string server_ip = client_vm["server"].as<string>();
        unsigned short server_port = client_vm["port"].as<unsigned short>();
        string vsp_name = client_vm["vsp"].as<string>();

        io_service io;
        SerialClient client(io, server_ip, server_port, vsp_name);
        client.run();
    } else if (mode == "server") {
        // Options for the server mode
        options_description server_desc("Server options");
        server_desc.add_options()
            ("help", "Display this help message")
            ("device,d", value<string>()->default_value("/dev/ttyUSB0"), "Device name")
            ("baud,b", value<unsigned int>()->default_value(9600), "Baud rate")
            ("port,p", value<unsigned short>()->default_value(12345), "Port number");

        variables_map server_vm;
        store(command_line_parser(argc, argv).options(server_desc).allow_unregistered().run(), server_vm);
        notify(server_vm);

        if (server_vm.count("help")) {
            cout << server_desc << endl;
            return 0;
        }

        io_service io;
        tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), server_vm["port"].as<unsigned short>()));
        RealSerialPort realSerial(io);
        realSerial.open(server_vm["device"].as<string>());
        realSerial.set_option(serial_port_base::baud_rate(server_vm["baud"].as<unsigned int>()));

        SerialServer server(io, realSerial, acceptor);
        server.run();
    } else {
        cerr << "Invalid mode specified. Use either 'client' or 'server'." << endl;
        return 1;
    }

    return 0;
}
