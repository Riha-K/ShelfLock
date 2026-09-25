#include "ReservationService.h"

#include "OrderState.h"

#include <sstream>
#include <stdexcept>
#include <utility>

ReservationService::ReservationService(InventoryRepository& repo, Logger& logger, Metrics& metrics,
                                       std::unique_ptr<FulfillmentStrategy> strategy, int ttl_seconds)
    : repo_(repo),
      logger_(logger),
      metrics_(metrics),
      strategy_(std::move(strategy)),
      ttl_seconds_(ttl_seconds),
      heap_([this](std::int64_t hold_id) { expireHold(hold_id); }) {}

HoldRecord ReservationService::reserve(const std::string& sku, int qty) {
    const auto options = repo_.locationsWithStock(sku, qty);
    if (options.empty()) {
        throw std::runtime_error("no location has enough stock for " + sku);
    }
    const LocationPick loc = strategy_->pick(options);
    HoldRecord rec = repo_.reserve(sku, loc.location_id, qty, ttl_seconds_);
    heap_.push(rec.hold_id, HoldHeap::Clock::now() + std::chrono::seconds(ttl_seconds_));
    metrics_.incReserved();
    auto reserved = OrderStateFactory::create("reserved");
    std::ostringstream detail;
    detail << "hold=" << rec.hold_id << " sku=" << sku << " qty=" << qty
           << " loc=" << loc.location_id << " via=" << strategy_->name()
           << " state=" << reserved->name();
    logger_.info("reserve", detail.str());
    return rec;
}

bool ReservationService::pay(std::int64_t hold_id) {
    if (!repo_.commit(hold_id)) {
        logger_.info("pay-skip", "hold=" + std::to_string(hold_id) + " not reserved");
        return false;
    }
    metrics_.incCommitted();
    auto paid = OrderStateFactory::create("paid");
    logger_.info("pay", "hold=" + std::to_string(hold_id) + " state=" + paid->name());
    return true;
}

void ReservationService::stop() { heap_.stop(); }

std::vector<RestockHint> ReservationService::restockSuggestions(const std::vector<std::string>& skus) {
    DemandForecast forecast(repo_);
    return forecast.suggest(skus, 4);
}

void ReservationService::expireHold(std::int64_t hold_id) {
    if (!repo_.release(hold_id, "expired")) {
        return;
    }
    metrics_.incExpired();
    auto expired = OrderStateFactory::create("expired");
    logger_.info("expire", "hold=" + std::to_string(hold_id) + " state=" + expired->name());
}
