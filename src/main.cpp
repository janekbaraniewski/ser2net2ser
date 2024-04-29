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

    if (argc < 2) {
        cerr << "Usage: ser2net2ser <command> [options]\n";
        return 1;
    }

    string command = argv[1];

    if (command == "serve") {
        options_description serve_desc("Server options");
        serve_desc.add_options()
            ("help,h", "Display this help message")
            ("device,d", value<string>()->default_value("/dev/ttyUSB0"), "Device name")
            ("baud,b", value<unsigned int>()->default_value(9600), "Baud rate")
            ("port,p", value<unsigned short>()->default_value(12345), "Port number");

        variables_map serve_vm;
        store(parse_command_line(argc - 1, argv + 1, serve_desc), serve_vm);

        if (serve_vm.count("help")) {
            cout << serve_desc << endl;
            return 0;
        }

        notify(serve_vm);

        io_service io;
        tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), serve_vm["port"].as<unsigned short>()));
        RealSerialPort realSerial(io);
        realSerial.open(serve_vm["device"].as<string>());
        realSerial.set_option(serial_port_base::baud_rate(serve_vm["baud"].as<unsigned int>()));

        SerialServer server(io, realSerial, acceptor);
        server.run();
    } else if (command == "connect") {
        options_description connect_desc("Client options");
        connect_desc.add_options()
            ("help,h", "Display this help message")
            ("server,s", value<string>()->default_value("127.0.0.1"), "Server IP address")
            ("port,p", value<unsigned short>()->default_value(12345), "Server port")
            ("vsp,v", value<string>()->required(), "Virtual serial port name");

        variables_map connect_vm;
        store(parse_command_line(argc - 1, argv + 1, connect_desc), connect_vm);

        if (connect_vm.count("help")) {
            cout << connect_desc << endl;
            return 0;
        }

        notify(connect_vm);

        string server_ip = connect_vm["server"].as<string>();
        unsigned short server_port = connect_vm["port"].as<unsigned short>();
        string vsp_name = connect_vm["vsp"].as<string>();

        io_service io;
        SerialClient client(io, server_ip, server_port, vsp_name);
        client.run();
    } else {
        cerr << "Invalid command. Use 'serve' or 'connect'." << endl;
        return 1;
    }

    return 0;
}
