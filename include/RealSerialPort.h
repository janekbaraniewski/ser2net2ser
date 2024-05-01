#ifndef REALSERIALPORT_H
#define REALSERIALPORT_H

#include "ISerialPort.h"
#include "Logger.h"
#include <boost/asio.hpp>

using boost::asio::serial_port_base;

class RealSerialPort : public ISerialPort {
public:
    explicit RealSerialPort(boost::asio::io_service& io) : port(io) {}

    void open(const std::string& device, serial_port_base::baud_rate baudRate) override {
        Logger(Logger::Level::Info) << "Setting up connection to serial port " << device;
        try {
            port.open(device);
            setSerialOptions(baudRate);
        } catch (const boost::system::system_error& e) {
            Logger(Logger::Level::Error) << "Failed to open serial port: " << e.what();
        }
    }

    void setSerialOptions(serial_port_base::baud_rate baudRate) {
        Logger(Logger::Level::Info) << "Setting connection options.";
        port.set_option(baudRate);
        port.set_option(serial_port_base::character_size(8));
        port.set_option(serial_port_base::parity(serial_port_base::parity::none));
        port.set_option(serial_port_base::stop_bits(serial_port_base::stop_bits::one));
        port.set_option(serial_port_base::flow_control(serial_port_base::flow_control::none));
    }

    void async_read_some(const boost::asio::mutable_buffer& buffer, std::function<void(const boost::system::error_code&, std::size_t)> handler) override {
        if (!port.is_open()) {
            Logger(Logger::Level::Error) << "Attempt to read from a closed serial port.";
            return;
        }
        port.async_read_some(buffer, handler);
    }

    void async_write(const boost::asio::const_buffer& buffer, std::function<void(const boost::system::error_code&, std::size_t)> handler) override {
        Logger(Logger::Level::Info) << "write some some.";
        if (!port.is_open()) {
            Logger(Logger::Level::Error) << "Attempt to write to a closed serial port.";
            return;
        }
        boost::asio::async_write(port, buffer, handler);
    }

private:
    boost::asio::serial_port port;
};

#endif // REALSERIALPORT_H
