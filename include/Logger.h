#pragma once

#include <mutex>
#include <string>

// Each event is written to the console and appended to the log file.
class Logger {
public:
    explicit Logger(std::string path = "shelflock.log");
    void info(const std::string& event, const std::string& detail);

private:
    std::string path_;
    std::mutex mu_;
};
