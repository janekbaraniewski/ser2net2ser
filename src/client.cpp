#include <iostream>
#include <array>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/support/date_time.hpp>

#include "logging.h"
#include "VirtualSerialPort.h"

using namespace boost::asio;
using namespace boost::program_options;
using ip::tcp;
using std::string;
using std::cout;
using std::cerr;
using std::endl;

void init_logging() {
    boost::log::add_console_log(
        std::cout,
        boost::log::keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] %Message%",
        boost::log::keywords::auto_flush = true
    );
    boost::log::add_file_log(
        boost::log::keywords::file_name = "serial_client_%N.log",
        boost::log::keywords::rotation_size = 10 * 1024 * 1024,
        boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0),
        boost::log::keywords::format = "[%TimeStamp%] [%Severity%] %Message%"
    );
    boost::log::add_common_attributes();
    boost::log::register_simple_formatter_factory<boost::log::trivial::severity_level, char>("Severity");
}

class SerialClient {
private:
    io_service io_service_;
    tcp::socket socket_;
    std::array<char, 256> buffer_;
    VirtualSerialPort vsp_;

public:
    SerialClient(const string& server_ip, unsigned short server_port, const string& vsp_name)
        : socket_(io_service_), vsp_(vsp_name) {
        BOOST_LOG_TRIVIAL(info) << "Initializing client...";
        tcp::resolver resolver(io_service_);
        auto endpoint_iterator = resolver.resolve({server_ip, std::to_string(server_port)});
        BOOST_LOG_TRIVIAL(info) << "Connecting to server at " << server_ip << ":" << server_port;
        connect(socket_, endpoint_iterator);
        BOOST_LOG_TRIVIAL(info) << "Connected to server.";
        BOOST_LOG_TRIVIAL(info) << "Opening virtual serial port: " << vsp_name;
        vsp_.open();
    }

    void run() {
        BOOST_LOG_TRIVIAL(info) << "Starting client I/O operations.";
        do_read_write();
        io_service_.run();
    }

private:
    void do_read_write() {
        socket_.async_read_some(boost::asio::buffer(buffer_), [this](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                string data(buffer_.data(), length);
                BOOST_LOG_TRIVIAL(info) << "Received data: " << data;
                if (vsp_.write(data)) {
                    BOOST_LOG_TRIVIAL(info) << "Data written to virtual serial port.";
                } else {
                    BOOST_LOG_TRIVIAL(error) << "Failed to write to virtual serial port.";
                }
                do_read_write();
            } else {
                BOOST_LOG_TRIVIAL(error) << "Read error: " << ec.message();
            }
        });
    }
};

int main(int argc, char* argv[]) {
    init_logging();

    try {
        options_description desc{"Options"};
        desc.add_options()
            ("help,h", "Help screen")
            ("server,s", value<string>()->default_value("127.0.0.1"), "Server IP address")
            ("port,p", value<unsigned short>()->default_value(12345), "Server port")
            ("vsp,v", value<string>()->required(), "Virtual serial port name");

        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);
        notify(vm);

        if (vm.count("help")) {
            cout << desc << endl;
            return 0;
        }

        string server_ip = vm["server"].as<string>();
        unsigned short server_port = vm["port"].as<unsigned short>();
        string vsp_name = vm["vsp"].as<string>();

        SerialClient client(server_ip, server_port, vsp_name);
        client.run();
    } catch (const std::exception& e) {
        BOOST_LOG_TRIVIAL(error) << "Exception: " << e.what();
        cerr << "Exception: " << e.what() << endl;
    }
    return 0;
}
