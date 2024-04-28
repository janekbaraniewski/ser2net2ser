#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "server.h"  // Include the server header

using namespace boost::asio;
using namespace testing;

class MockSerialPort : public ISerialPort {
public:
    MOCK_METHOD(void, open, (const std::string&), (override));
    MOCK_METHOD(void, set_option, (const serial_port_base::baud_rate&), (override));
    MOCK_METHOD(void, async_read_some, (const boost::asio::mutable_buffer&, std::function<void(const boost::system::error_code&, std::size_t)>), (override));
    MOCK_METHOD(void, async_write, (const boost::asio::const_buffer&, std::function<void(const boost::system::error_code&, std::size_t)>), (override));
};

class SerialServerTest : public ::testing::Test {
protected:
    io_service io;
    MockSerialPort serial;
    tcp::endpoint endpoint{tcp::v4(), 12345};
    tcp::acceptor acceptor{io, endpoint};
    SerialServer server{io, serial, acceptor};

    void SetUp() override {
        // Setup expectations and actions
        EXPECT_CALL(serial, open("dummy_device"));
        EXPECT_CALL(serial, set_option(_)).Times(AtLeast(1)); // Use _ for unspecified parameters with proper scope
        EXPECT_CALL(serial, async_read_some(_, _)).Times(AtLeast(1)); // Using _ correctly
    }
};

MATCHER_P(BaudRateMatcher, rate, "Matches the baud rate of a serial port setting.") {
    return arg.value() == rate.value();
}

TEST_F(SerialServerTest, TestBaudRateSetting) {
    EXPECT_CALL(serial, set_option(BaudRateMatcher(serial_port_base::baud_rate(9600)))).Times(1);
    server.run();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);  // Initialize GoogleTest, removes all recognized flags
    // Now you can initialize Boost program_options if necessary
    return RUN_ALL_TESTS();
}

