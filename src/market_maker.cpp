#include "strategy/baseline_market_making.hpp"

namespace aStrategy {

std::vector<ModAction> BaseLineMarketMaking::step(EngineAPI& api) {
    std::vector<ModAction> actions;
    const Px mid = compute_mid(api);
    const int half = quote_param_.spread / 2;

        // to maker profit off of the spread: want ask > bid
        // thats why market makers exist duh

        // skew -> helps understand inventory risk
        // so if we have a large positive inventory i.e. we are long shares, we want to sell.
        // thus the strategy should be such that we lower the bid to sell more

        // but if we have a large negative inventory i.e. we are short shazres -> we want to buy
        // the strategy must raise the bid so that we are demotivated to buy and allows us to get sellers
        
    const int skew = static_cast<int>(api.pnl.inventory) * quote_param_.skew_per_unit;

        // target_bid and target_ask are the new quote we want to adjust to
    const Px target_bid = mid - half - skew;
    const Px target_ask = mid + half - skew;
    const int target_qty = quote_param_.qty;

    const bool bid_needs_update = !bid_px_.has_value() || *bid_px_ != target_bid || last_qty_ != target_qty;
    const bool ask_needs_update = !ask_px_.has_value() || *ask_px_ != target_ask || last_qty_ != target_qty;

    if (bid_needs_update && bid_id_.has_value()) {
        actions.push_back({ModType::Cancel, *bid_id_});
        bid_id_.reset();
        bid_px_.reset();
    }

    if (ask_needs_update && ask_id_.has_value()) {
        actions.push_back({ModType::Cancel, *ask_id_});
        ask_id_.reset();
        ask_px_.reset();
    }

    if (bid_needs_update) {
        OrderId new_bid_id = api.ids.next();
        actions.push_back({ModType::New, new_bid_id, Side::Buy, target_bid, (Qty)target_qty});
        bid_id_ = new_bid_id;
        bid_px_ = target_bid;
    }

    if (ask_needs_update) {
        OrderId new_ask_id = api.ids.next();
        actions.push_back({ModType::New, new_ask_id, Side::Sell, target_ask, (Qty)target_qty});
        ask_id_ = new_ask_id;
        ask_px_ = target_ask;
    }

    last_qty_ = target_qty;
    return actions;
}

}


