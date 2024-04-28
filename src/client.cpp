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
#include <boost/log/support/date_time.hpp> // Ensure this is included for date-time support


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
    boost::log::register_simple_formatter_factory<boost::log::trivial::severity_level, char>("Severity");
    boost::log::add_console_log(std::cout, boost::log::keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] %Message%");
    boost::log::add_file_log(boost::log::keywords::file_name = "serial_client_%N.log",
                             boost::log::keywords::rotation_size = 10 * 1024 * 1024,
                             boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0),
                             boost::log::keywords::format = "[%TimeStamp%] [%Severity%] %Message%");
    boost::log::add_common_attributes();
}



class SerialClient {
private:
    io_service io_service_;
    tcp::socket socket_;
    std::array<char, 256> buffer_;
    VirtualSerialPort vsp_;

public:
    SerialClient(const std::string& server_ip, unsigned short server_port, const std::string& vsp_name)
        : socket_(io_service_), vsp_(vsp_name) {
        tcp::resolver resolver(io_service_);
        auto endpoint_iterator = resolver.resolve({server_ip, std::to_string(server_port)});
        connect(socket_, endpoint_iterator);

        // vsp_.open();  // Ensure the virtual port is ready for use
    }

    void run() {
        do_read_write();
        io_service_.run();
    }

private:
    void do_read_write() {
        socket_.async_read_some(boost::asio::buffer(buffer_), [this](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                std::string data(buffer_.begin(), buffer_.begin() + length);
                if (!vsp_.write(data)) {
                    cerr << "Write to virtual serial port failed" << endl;
                    return;
                }
                do_read_write();  // Continue reading
            } else {
                cerr << "Read error: " << ec.message() << endl;
            }
        });
    }
};

int main(int argc, char* argv[]) {
    init_logging();

    // Process command line arguments before accessing any files
    if (argc > 1 && std::string(argv[1]) == "--help") {
        std::cout << "Usage: ..." << std::endl;
        return 0;
    }

    try {
        options_description desc{"Options"};
        desc.add_options()
            ("help,h", "Help screen")
            ("server,s", value<std::string>()->default_value("127.0.0.1"), "Server IP address")
            ("port,p", value<unsigned short>()->default_value(12345), "Server port")
            ("vsp,v", value<std::string>()->required(), "Virtual serial port name");

        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);
        notify(vm);

        if (vm.count("help")) {
            cout << desc << endl;
            return 0;
        }

        std::string server_ip = vm["server"].as<std::string>();
        unsigned short server_port = vm["port"].as<unsigned short>();
        std::string vsp_name = vm["vsp"].as<std::string>();

        SerialClient client(server_ip, server_port, vsp_name);
        client.run();
    } catch (const std::exception& e) {
        BOOST_LOG_TRIVIAL(error) << "Exception: " << e.what();
        cerr << "Exception: " << e.what() << endl;
    }
    return 0;
}
