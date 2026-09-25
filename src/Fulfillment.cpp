#include "Fulfillment.h"

#include <stdexcept>

LocationPick StoreFirstStrategy::pick(const std::vector<LocationPick>& options) const {
    for (const auto& option : options) {
        if (option.location_type == "store") {
            return option;
        }
    }
    if (!options.empty()) {
        return options.front();
    }
    throw std::runtime_error("no stock location available");
}

const char* StoreFirstStrategy::name() const { return "store-first"; }

LocationPick WarehouseStrategy::pick(const std::vector<LocationPick>& options) const {
    for (const auto& option : options) {
        if (option.location_type == "warehouse") {
            return option;
        }
    }
    if (!options.empty()) {
        return options.front();
    }
    throw std::runtime_error("no stock location available");
}

const char* WarehouseStrategy::name() const { return "warehouse"; }
