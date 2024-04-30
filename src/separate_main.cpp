#include <iostream>
#include "VirtualSerialPort.h"
#include <boost/asio.hpp>

int main() {
    try {
        boost::asio::io_context io_context;
        std::string device = "ttyUSB9"; // Adjust the device name as needed

        // Instantiate your VirtualSerialPort
        VirtualSerialPort vsp(io_context, device);

        // Prepare a message to send
        std::string test_message = "Hello, Virtual Serial Port!";
        boost::asio::const_buffer write_buffer = boost::asio::buffer(test_message);

        // Function to handle write completion
        auto write_handler = [](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            std::cout << "Write completed. Bytes transferred: " << bytes_transferred << std::endl;
            if (ec) {
                std::cerr << "Error on write: " << ec.message() << std::endl;
            }
        };

        // Write to the virtual serial port
        vsp.async_write(write_buffer, write_handler);

        // Prepare a buffer for reading
        std::array<char, 1024> read_buffer;
        boost::asio::mutable_buffer mutable_buffer = boost::asio::buffer(read_buffer);

        // Function to handle read completion
        auto read_handler = [&read_buffer](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            std::cout << "Read completed. Bytes transferred: " << bytes_transferred << std::endl;
            if (!ec) {
                std::cout << "Received: " << std::string(read_buffer.begin(), read_buffer.begin() + bytes_transferred) << std::endl;
            } else {
                std::cerr << "Error on read: " << ec.message() << std::endl;
            }
        };

        // Read from the virtual serial port
        vsp.async_read(mutable_buffer, read_handler);

        // Run the io_context to perform asynchronous operations
        io_context.run();

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
