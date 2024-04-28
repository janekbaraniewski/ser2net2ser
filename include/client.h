#ifndef CLIENT_H
#define CLIENT_H

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
using ip::tcp;
using std::string;

class SerialClient {
public:
    SerialClient(const string& server_ip, unsigned short server_port, const string& vsp_name);
    void run();

private:
    io_service io_service_;
    tcp::socket socket_;
    std::array<char, 256> buffer_;
    VirtualSerialPort vsp_;

    void do_read_write();
};


#endif // CLIENT_H
