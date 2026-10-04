#include "Metrics.h"

#include <sstream>

void Metrics::incReserved() { reserved_.fetch_add(1, std::memory_order_relaxed); }
void Metrics::incCommitted() { committed_.fetch_add(1, std::memory_order_relaxed); }
void Metrics::incReleased() { released_.fetch_add(1, std::memory_order_relaxed); }
void Metrics::incExpired() { expired_.fetch_add(1, std::memory_order_relaxed); }

std::string Metrics::snapshot() const {
    const auto reserved = reserved_.load(std::memory_order_relaxed);
    const auto committed = committed_.load(std::memory_order_relaxed);
    const auto released = released_.load(std::memory_order_relaxed);
    const auto expired = expired_.load(std::memory_order_relaxed);
    std::ostringstream os;
    os << "reserved=" << reserved
       << " committed=" << committed
       << " released=" << released
       << " expired=" << expired;
    return os.str();
}
