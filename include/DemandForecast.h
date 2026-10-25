#pragma once

// Restock hints from paid demand + a fixed cover buffer.
// Not ML: suggested_qty = max(0, recent_demand + cover_units - on_hand).

#include "Repository.h"

#include <string>
#include <vector>

struct RestockHint {
    std::string sku;
    int on_hand = 0;
    int recent_demand = 0;  // committed / paid qty used as demand signal
    int cover_units = 0;    // extra units to keep beyond recent demand
    int target = 0;         // recent_demand + cover_units
    int suggested_qty = 0;  // target - on_hand when positive
};

class DemandForecast {
public:
    explicit DemandForecast(InventoryRepository& repo);

    // cover_units: buffer above recent paid demand (negative treated as 0).
    // max_suggested: optional cap per SKU (0 or less = no cap).
    std::vector<RestockHint> suggest(const std::vector<std::string>& skus,
                                     int cover_units,
                                     int max_suggested = 0);

private:
    InventoryRepository& repo_;
};
