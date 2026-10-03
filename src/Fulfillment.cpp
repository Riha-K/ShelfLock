#include "Fulfillment.h"

#include <stdexcept>

LocationPick StoreFirstStrategy::pick(const std::vector<LocationPick>& options) const {
    LocationPick fallback;
    bool have_fallback = false;
    for (const auto& option : options) {
        if (option.location_id.empty()) {
            continue;
        }
        if (option.location_type == "store") {
            return option;
        }
        if (!have_fallback) {
            fallback = option;
            have_fallback = true;
        }
    }
    if (have_fallback) {
        return fallback;
    }
    throw std::runtime_error("no stock location available");
}

const char* StoreFirstStrategy::name() const { return "store-first"; }

LocationPick WarehouseStrategy::pick(const std::vector<LocationPick>& options) const {
    LocationPick fallback;
    bool have_fallback = false;
    for (const auto& option : options) {
        if (option.location_id.empty()) {
            continue;
        }
        if (option.location_type == "warehouse") {
            return option;
        }
        if (!have_fallback) {
            fallback = option;
            have_fallback = true;
        }
    }
    if (have_fallback) {
        return fallback;
    }
    throw std::runtime_error("no stock location available");
}

const char* WarehouseStrategy::name() const { return "warehouse"; }
