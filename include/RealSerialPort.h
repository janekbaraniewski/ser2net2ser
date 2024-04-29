#ifndef REALSERIALPORT_H
#define REALSERIALPORT_H

#include "ISerialPort.h"
#include <boost/asio.hpp>

using boost::asio::serial_port_base;

class RealSerialPort : public ISerialPort {
public:
    explicit RealSerialPort(boost::asio::io_service& io) : port(io) {}

    void open(const std::string& device, serial_port_base::baud_rate baudRate) override {
        port.open(device);
        port.set_option(baudRate);
        port.set_option(serial_port_base::character_size(8));
        port.set_option(serial_port_base::parity(serial_port_base::parity::none));
        port.set_option(serial_port_base::stop_bits(serial_port_base::stop_bits::one));
        port.set_option(serial_port_base::flow_control(serial_port_base::flow_control::none));
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
