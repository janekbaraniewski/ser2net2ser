#include "SerialClient.h"

using namespace boost::asio;
using namespace boost::program_options;
using ip::tcp;
using std::string;
using std::cout;
using std::cerr;
using std::endl;

SerialClient::SerialClient(boost::asio::io_service& io_service, const std::string& server_ip, unsigned short server_port, const std::string& vsp_name)
    : io_service_(io_service), socket_(io_service), vsp_(vsp_name) {  // Initialize vsp_ here
    BOOST_LOG_TRIVIAL(info) << "Initializing client...";
    boost::asio::ip::tcp::resolver resolver(io_service);
    auto endpoint_iterator = resolver.resolve({server_ip, std::to_string(server_port)});
    BOOST_LOG_TRIVIAL(info) << "Connecting to server at " << server_ip << ":" << server_port;
    connect(socket_, endpoint_iterator);
    BOOST_LOG_TRIVIAL(info) << "Connected to server.";
    BOOST_LOG_TRIVIAL(info) << "Opening virtual serial port: " << vsp_name;
}

void SerialClient::run() {
    BOOST_LOG_TRIVIAL(info) << "Starting client I/O operations.";
    do_read_write();
    io_service_.run();
}

void SerialClient::do_read_write() {
    socket_.async_read_some(boost::asio::buffer(buffer_), [this](boost::system::error_code ec, std::size_t length) {
        if (!ec) {
            std::string data(buffer_.begin(), buffer_.begin() + length);
            BOOST_LOG_TRIVIAL(info) << "Received data: " << data;
            if (vsp_.write(data)) {  // Use vsp_ to write data
                BOOST_LOG_TRIVIAL(info) << "Data written to virtual serial port.";
            } else {
                BOOST_LOG_TRIVIAL(error) << "Failed to write to virtual serial port.";
            }
            do_read_write();  // Continue reading
        } else {
            BOOST_LOG_TRIVIAL(error) << "Read error: " << ec.message();
        }
    });
}

// int main(int argc, char* argv[]) {
//     init_logging();

//     try {
//         options_description desc{"Options"};
//         desc.add_options()
//             ("help,h", "Help screen")
//             ("server,s", value<string>()->default_value("127.0.0.1"), "Server IP address")
//             ("port,p", value<unsigned short>()->default_value(12345), "Server port")
//             ("vsp,v", value<string>()->required(), "Virtual serial port name");

//         variables_map vm;
//         store(parse_command_line(argc, argv, desc), vm);
//         notify(vm);

//         if (vm.count("help")) {
//             cout << desc << endl;
//             return 0;
//         }

//         string server_ip = vm["server"].as<string>();
//         unsigned short server_port = vm["port"].as<unsigned short>();
//         string vsp_name = vm["vsp"].as<string>();

//         SerialClient client(server_ip, server_port, vsp_name);
//         client.run();
//     } catch (const std::exception& e) {
//         BOOST_LOG_TRIVIAL(error) << "Exception: " << e.what();
//         cerr << "Exception: " << e.what() << endl;
//     }
//     return 0;
// }
