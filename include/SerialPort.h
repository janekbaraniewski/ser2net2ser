#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "Logger.h"

class SerialPort {
private:
    int serial_fd;
    std::thread read_thread;
    std::atomic<bool> keep_reading;
    std::queue<std::string> read_buffer;
    std::mutex mtx;
    std::condition_variable cv;

    void readLoop();
    void configurePort(int baud_rate);

public:
    SerialPort(const std::string& device, int baud_rate);
    ~SerialPort();
    void startReading();
    void stopReading();
    void writeData(const std::string& data);
    std::string readData();
};

#endif // SERIALPORT_H
