#include "DemandForecast.h"

DemandForecast::DemandForecast(InventoryRepository& repo) : repo_(repo) {}

std::vector<RestockHint> DemandForecast::suggest(const std::vector<std::string>& skus, int cover_units) {
    std::vector<RestockHint> hints;
    for (const auto& sku : skus) {
        RestockHint hint;
        hint.sku = sku;
        hint.on_hand = repo_.onHand(sku);
        hint.recent_demand = repo_.committedQty(sku);
        const int target = hint.recent_demand + cover_units;
        if (hint.on_hand < target) {
            hint.suggested_qty = target - hint.on_hand;
            hints.push_back(hint);
        }
    }
    return hints;
}
