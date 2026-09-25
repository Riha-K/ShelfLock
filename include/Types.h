#pragma once

#include <cstdint>
#include <string>

struct HoldRecord {
    std::int64_t hold_id = 0;
    std::string sku;
    std::string location_id;
    int qty = 0;
};

struct LocationPick {
    std::string location_id;
    std::string location_type;
};
