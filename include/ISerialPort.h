#ifndef ISERIALPORT_H
#define ISERIALPORT_H

#include <string>
#include <boost/asio.hpp>

using namespace boost::asio;
using std::string;

class ISerialPort {
public:
    virtual void open(const string& device) = 0;
    virtual void set_option(const serial_port_base::baud_rate& option) = 0;
    virtual void async_read_some(const boost::asio::mutable_buffer& buffer, std::function<void(const boost::system::error_code&, std::size_t)> handler) = 0;
    virtual void async_write(const boost::asio::const_buffer& buffer, std::function<void(const boost::system::error_code&, std::size_t)> handler) = 0;
    virtual ~ISerialPort() = default;
};

#endif // ISERIALPORT_H
