#pragma once
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <mutex>

class Logger {
private:
    std::ostringstream buffer;
    std::string log_filename;
    std::mutex mutex;

    static std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::tm tm_now;
        localtime_s(&tm_now, &time_t_now);

        std::ostringstream oss;
        oss << "["
            << std::setfill('0')
            << std::setw(4) << (tm_now.tm_year + 1900) << "-"
            << std::setw(2) << (tm_now.tm_mon + 1) << "-"
            << std::setw(2) << tm_now.tm_mday << " "
            << std::setw(2) << tm_now.tm_hour << ":"
            << std::setw(2) << tm_now.tm_min << ":"
            << std::setw(2) << tm_now.tm_sec
            << "]";
        return oss.str();
    }

    static std::string generateLogFilename() {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::tm tm_now;
        localtime_s(&tm_now, &time_t_now);

        std::ostringstream oss;
        oss << "output/log_"
            << std::setfill('0')
            << std::setw(4) << (tm_now.tm_year + 1900)
            << std::setw(2) << (tm_now.tm_mon + 1)
            << std::setw(2) << tm_now.tm_mday
            << "_"
            << std::setw(2) << tm_now.tm_hour
            << std::setw(2) << tm_now.tm_min
            << std::setw(2) << tm_now.tm_sec
            << ".txt";
        return oss.str();
    }

public:
    Logger() {
        log_filename = generateLogFilename();
    }

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex);
        std::string timestamp = getCurrentTimestamp();
        std::string line = timestamp + " " + message;

        std::cout << line << std::endl;
        buffer << line << "\n";
    }

    void saveToFile() {
        std::lock_guard<std::mutex> lock(mutex);
        std::ofstream file(log_filename);
        if (file.is_open()) {
            file << buffer.str();
            file.close();
            std::string timestamp = getCurrentTimestamp();
            std::string msg = timestamp + " 日志已保存到: " + log_filename;
            std::cout << msg << std::endl;
            buffer << msg << "\n";
        } else {
            std::cerr << getCurrentTimestamp() << " [错误] 无法保存日志文件: " << log_filename << std::endl;
        }
    }

    std::string getLogFilename() const {
        return log_filename;
    }
};
