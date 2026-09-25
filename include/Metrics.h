#pragma once

#include <atomic>
#include <cstdint>
#include <string>

class Metrics {
public:
    void incReserved();
    void incCommitted();
    void incReleased();
    void incExpired();
    std::string snapshot() const;

private:
    std::atomic<std::int64_t> reserved_{0};
    std::atomic<std::int64_t> committed_{0};
    std::atomic<std::int64_t> released_{0};
    std::atomic<std::int64_t> expired_{0};
};
