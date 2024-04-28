#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <string>
#include "ISerialPort.h"

#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include <boost/asio.hpp>
#include <boost/array.hpp>
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
#include "ISerialPort.h"
#include "RealSerialPort.h"

using namespace boost::asio;
using ip::tcp;
using std::string;

class SerialServer {
public:
    SerialServer(io_service& io, ISerialPort& serial, tcp::acceptor& acceptor);
    void run();

private:
    io_service& io_service_;
    ISerialPort& serial_;
    tcp::acceptor& acceptor_;
    tcp::socket socket_;

    void start_accept();
    void do_read_write();
};

#endif // SERVER_H
