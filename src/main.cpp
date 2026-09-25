#include "Fulfillment.h"
#include "Logger.h"
#include "Metrics.h"
#include "ReservationService.h"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {

std::string conninfoFromEnv() {
    if (const char* dsn = std::getenv("SHELFLOCK_DSN")) {
        return dsn;
    }
    return "dbname=shelflock";
}

}  // namespace

int main() {
    try {
        Logger logger;
        Metrics metrics;
        InventoryRepository repo(conninfoFromEnv());
        repo.applySqlFile("sql/schema.sql");
        repo.applySqlFile("sql/seed.sql");

        ReservationService service(repo, logger, metrics, std::make_unique<StoreFirstStrategy>(), 3);

        std::vector<std::thread> workers;
        workers.emplace_back([&] {
            const HoldRecord a = service.reserve("MILK-1L", 1);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            service.pay(a.hold_id);
        });
        workers.emplace_back([&] {
            const HoldRecord b = service.reserve("MILK-1L", 1);
            service.pay(b.hold_id);
        });
        workers.emplace_back([&] {
            service.reserve("MILK-1L", 1);
            std::this_thread::sleep_for(std::chrono::seconds(4));
        });

        for (auto& worker : workers) {
            worker.join();
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        service.stop();

        std::cout << "metrics " << metrics.snapshot() << '\n';
        for (const auto& hint : service.restockSuggestions({"MILK-1L", "BREAD-W", "EGGS-12"})) {
            std::cout << "restock " << hint.sku << " on_hand=" << hint.on_hand
                      << " demand=" << hint.recent_demand << " suggest=" << hint.suggested_qty << '\n';
        }
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "error: " << ex.what() << '\n';
        return 1;
    }
}
