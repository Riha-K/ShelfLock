#include "DemandForecast.h"

DemandForecast::DemandForecast(InventoryRepository& repo) : repo_(repo) {}

std::vector<RestockHint> DemandForecast::suggest(const std::vector<std::string>& skus,
                                                 int cover_units,
                                                 int max_suggested) {
    std::vector<RestockHint> hints;
    const int cover = cover_units < 0 ? 0 : cover_units;

    for (const auto& sku : skus) {
        RestockHint hint;
        hint.sku = sku;
        hint.on_hand = repo_.onHand(sku);
        hint.recent_demand = repo_.committedQty(sku);
        hint.cover_units = cover;
        hint.target = hint.recent_demand + cover;

        if (hint.on_hand >= hint.target) {
            continue;
        }

        hint.suggested_qty = hint.target - hint.on_hand;
        if (max_suggested > 0 && hint.suggested_qty > max_suggested) {
            hint.suggested_qty = max_suggested;
        }
        if (hint.suggested_qty > 0) {
            hints.push_back(hint);
        }
    }
    return hints;
}
