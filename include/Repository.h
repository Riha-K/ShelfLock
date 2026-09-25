#pragma once

#include "Types.h"

#include <libpq-fe.h>

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

class InventoryRepository {
public:
    explicit InventoryRepository(std::string conninfo);
    ~InventoryRepository();

    InventoryRepository(const InventoryRepository&) = delete;
    InventoryRepository& operator=(const InventoryRepository&) = delete;

    void applySqlFile(const std::string& path);
    std::vector<LocationPick> locationsWithStock(const std::string& sku, int qty);
    HoldRecord reserve(const std::string& sku, const std::string& location_id, int qty, int ttl_seconds);
    bool commit(std::int64_t hold_id);
    bool release(std::int64_t hold_id, const std::string& new_status);
    int onHand(const std::string& sku);
    int committedQty(const std::string& sku);

private:
    void exec(const std::string& sql);
    PGresult* execParams(const char* sql, int n, const char* const* values);
    static std::string readFile(const std::string& path);
    static std::int64_t asInt64(const char* text);

    std::string conninfo_;
    PGconn* conn_ = nullptr;
    std::mutex mu_;
};
