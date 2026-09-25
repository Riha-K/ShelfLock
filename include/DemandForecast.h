#pragma once

#include "Repository.h"

#include <string>
#include <vector>

struct RestockHint {
    std::string sku;
    int on_hand = 0;
    int recent_demand = 0;
    int suggested_qty = 0;
};

class DemandForecast {
public:
    explicit DemandForecast(InventoryRepository& repo);
    std::vector<RestockHint> suggest(const std::vector<std::string>& skus, int cover_units);

private:
    InventoryRepository& repo_;
};
