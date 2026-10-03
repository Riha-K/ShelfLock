#include "Logger.h"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

Logger::Logger(std::string path) : path_(std::move(path)) {}

void Logger::info(const std::string& event, const std::string& detail) {
    const auto now = std::chrono::system_clock::now();
    const std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream line;
    line << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << " [" << event << "] " << detail;

    std::lock_guard<std::mutex> lock(mu_);
    std::cout << line.str() << '\n';
    std::ofstream out(path_, std::ios::app);
    if (out) {
        out << line.str() << '\n';
    } else {
        std::cerr << "log file unavailable: " << path_ << '\n';
    }
}
