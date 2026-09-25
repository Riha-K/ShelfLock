#include "OrderState.h"

#include <stdexcept>

const char* CreatedState::name() const { return "created"; }
const char* ReservedState::name() const { return "reserved"; }
const char* PaidState::name() const { return "paid"; }
const char* ReleasedState::name() const { return "released"; }
const char* ExpiredState::name() const { return "expired"; }

std::unique_ptr<OrderState> OrderStateFactory::create(const std::string& state) {
    if (state == "created") return std::make_unique<CreatedState>();
    if (state == "reserved") return std::make_unique<ReservedState>();
    if (state == "paid") return std::make_unique<PaidState>();
    if (state == "released") return std::make_unique<ReleasedState>();
    if (state == "expired") return std::make_unique<ExpiredState>();
    throw std::invalid_argument("unknown order state: " + state);
}
