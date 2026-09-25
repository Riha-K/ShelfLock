#include "Metrics.h"

#include <sstream>

void Metrics::incReserved() { reserved_.fetch_add(1); }
void Metrics::incCommitted() { committed_.fetch_add(1); }
void Metrics::incReleased() { released_.fetch_add(1); }
void Metrics::incExpired() { expired_.fetch_add(1); }

std::string Metrics::snapshot() const {
    std::ostringstream os;
    os << "reserved=" << reserved_.load()
       << " committed=" << committed_.load()
       << " released=" << released_.load()
       << " expired=" << expired_.load();
    return os.str();
}
