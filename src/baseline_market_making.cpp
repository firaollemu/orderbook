#include "strategy/baseline_market_making.hpp"


namespace aStrategy {
    std::vector<ModificationAction> BaseLineMarketMaking::step(EngineAPI& api) {
    std::vector<ModificationAction> actions;

    const Px mid = compute_mid(api);
    const int half = qp_.spread / 2;
    const int skew = static_cast<int>(api.pnl.inventory) * qp_.skew_per_unit;
    const Px target_bid = mid - half - skew;
    const Px target_ask = mid + half - skew;
    const int target_qty = qp_.qty;

    const bool bid_needs_update = !bid_px_.has_value() || *bid_px_ != target_bid || last_qty_ != target_qty;
    const bool ask_needs_update = !ask_px_.has_value() || *ask_px_ != target_ask || last_qty_ != target_qty;

    // cancel only needs modification type and id
    if (bid_needs_update && bid_id_) {
        actions.push_back(ModificationAction{ModificationType::Cancel, *bid_id_, std::nullopt, std::nullopt, std::nullopt});
        bid_id_.reset();
        bid_px_.reset();
    }

    if (ask_needs_update && ask_id_) {
        actions.push_back(ModificationAction{ModificationType::Cancel, *ask_id_, std::nullopt, std::nullopt, std::nullopt});
        ask_id_.reset();
        ask_px_.reset();
    }

    if (bid_needs_update) {
        OrderId new_id = api.ids.next(); // get next id
        actions.push_back(ModificationAction{
            ModificationType::New,
            new_id,
            Side::Buy,
            target_bid,
            static_cast<Qty>(target_qty)
        });
        bid_id_ = new_id;
        bid_px_ = target_bid;
    }

    if (ask_needs_update) {
        OrderId new_id = api.ids.next();
        actions.push_back(ModificationAction{
            ModificationType::New,
            new_id,
            Side::Sell,
            target_ask,
            static_cast<Qty>(target_qty)
        });
        ask_id_ = new_id;
        ask_px_ = target_ask;
    }

    last_qty_ = target_qty;
    return actions;
    }

}