#include "OrderState.h"

#include <cctype>
#include <stdexcept>

const char* CreatedState::name() const { return "created"; }
const char* ReservedState::name() const { return "reserved"; }
const char* PaidState::name() const { return "paid"; }
const char* ReleasedState::name() const { return "released"; }
const char* ExpiredState::name() const { return "expired"; }

std::unique_ptr<OrderState> OrderStateFactory::create(const std::string& state) {
    std::string key;
    key.reserve(state.size());
    for (unsigned char c : state) {
        key.push_back(static_cast<char>(std::tolower(c)));
    }
    if (key == "created") return std::make_unique<CreatedState>();
    if (key == "reserved") return std::make_unique<ReservedState>();
    if (key == "paid") return std::make_unique<PaidState>();
    if (key == "released") return std::make_unique<ReleasedState>();
    if (key == "expired") return std::make_unique<ExpiredState>();
    throw std::invalid_argument("unknown order state: " + state);
}
