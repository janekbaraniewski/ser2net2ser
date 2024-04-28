#ifndef REALSERIALPORT_H
#define REALSERIALPORT_H

#include "ISerialPort.h"
#include <boost/asio.hpp>

class RealSerialPort : public ISerialPort {
public:
    explicit RealSerialPort(io_service& io) : port(io) {}

    void open(const string& device) override {
        port.open(device);
    }

    void set_option(const serial_port_base::baud_rate& option) override {
        port.set_option(option);
    }

    void async_read_some(const boost::asio::mutable_buffer& buffer, std::function<void(const boost::system::error_code&, std::size_t)> handler) override {
        port.async_read_some(buffer, handler);
    }

    void async_write(const boost::asio::const_buffer& buffer, std::function<void(const boost::system::error_code&, std::size_t)> handler) override {
        boost::asio::async_write(port, buffer, handler);
    }

private:
    boost::asio::serial_port port;
};

#endif // REALSERIALPORT_H
