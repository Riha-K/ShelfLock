#pragma once

#include "Types.h"

#include <memory>
#include <string>
#include <vector>

class FulfillmentStrategy {
public:
    virtual ~FulfillmentStrategy() = default;
    virtual LocationPick pick(const std::vector<LocationPick>& options) const = 0;
    virtual const char* name() const = 0;
};

class StoreFirstStrategy : public FulfillmentStrategy {
public:
    LocationPick pick(const std::vector<LocationPick>& options) const override;
    const char* name() const override;
};

class WarehouseStrategy : public FulfillmentStrategy {
public:
    LocationPick pick(const std::vector<LocationPick>& options) const override;
    const char* name() const override;
};
