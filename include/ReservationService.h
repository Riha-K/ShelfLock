#pragma once

#include "DemandForecast.h"
#include "Fulfillment.h"
#include "HoldHeap.h"
#include "Logger.h"
#include "Metrics.h"
#include "Repository.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class ReservationService {
public:
    ReservationService(InventoryRepository& repo, Logger& logger, Metrics& metrics,
                       std::unique_ptr<FulfillmentStrategy> strategy, int ttl_seconds);

    HoldRecord reserve(const std::string& sku, int qty);
    bool pay(std::int64_t hold_id);
    void stop();
    std::vector<RestockHint> restockSuggestions(const std::vector<std::string>& skus);

private:
    void expireHold(std::int64_t hold_id);

    InventoryRepository& repo_;
    Logger& logger_;
    Metrics& metrics_;
    std::unique_ptr<FulfillmentStrategy> strategy_;
    int ttl_seconds_;
    HoldHeap heap_;
};
