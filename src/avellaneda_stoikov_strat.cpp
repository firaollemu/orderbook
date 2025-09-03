#include "avellaneda_and_stoikov.hpp"
#include <cmath>
#include <algorithm>

using aStrategy::ModificationAction;
using aStrategy::ModificationType;


std::vector<ModificationAction> AvellanedaStoikovStrat::step(EngineAPI& api) {
    std::vector<ModificationAction> actions;
    actions.reserve(4);

    const double mid = safe_mid_from_book(api.ob);
    if (!std::isfinite(mid)) {
        return actions;
    }

    const double sigma_bar = update_sigma(mid);


    // q -> strategy's current inventory i.e. position in the asset (long/short)
    // used for managing inventory risk
    const double q = current_inventory(api.pnl); 

    const int H = std::max(1, p_.horizon_bars);
    const double varH = (sigma_bar * sigma_bar) * static_cast<double>(H);

    // r -> optimal reservation price is adjusting mid-price
    // optimal reservation price -> trader is indifferent to holding or selling their inventory
    // here if q is positive (i..e we hold long a position), we lower the reservation price r below 
    // the mid to motivate selling and reduce inventory
    // if q is negative i.e. a short position, raise r to motivate buying and get a neutal position
    const double r = mid - q * p_.gamma * varH;

    const double core = (1.0/p_.gamma) * std::log(1.0 + p_.gamma / p_.kappa);

    // delta -> optimal half-spread (distance) of the bid and ask prices for the reservartion price r
    double delta = core + 0.5 * p_.gamma * varH;

    delta = std::clamp(delta, p_.min_half_spread, p_.max_half_spread);

    const double bid_px = round_to_tick(r-delta, p_.tick, true);
    const double ask_px = round_to_tick(r+delta, p_.tick, false);

    if (!(std::isfinite(bid_px) && std::isfinite(ask_px)) || bid_px <= 0.0 || ask_px <= 0.0) {
        return actions;
    }


    if (live_bid_id_) {
        ModificationAction cancelBid{};
        cancelBid.type = ModificationType::Cancel;
        cancelBid.id = *live_bid_id_;
        actions.push_back(cancelBid);
        live_bid_id_.reset();
    }

    if (live_ask_id_) {
        ModificationAction cancelAsk{};
        cancelAsk.type = ModificationAction::Cancel;
        cancelAsk.id = *live_ask_id_;
        actions.push_back(cancelAsk);
        live_ask_id_.reset();
    }

    const OrderId new_bid_id = api.ids.next();
    const OrderId new_ask_id = api.ids.next();


    ModificationAction newBid{};
    newBid.type = ModificationAction::New;
    newBid.id = new_bid_id;
    newBid.side = Side::Buy;
    newBid.px = static_cast<Px>(bid_px);
    newBid.qty = static_cast<Qty>(p_.quote_size);
    actions.push_back(newBid);



    ModificationAction newAsk{};
    newAsk.type = ModificationAction::New;
    newAsk.id = new_ask_id;
    newAsk.side = Side::Sell;
    newAsk.px = static_cast<Px>(ask_px);
    newAsk.qty = static_cast<Qty>(p_.quote_size);
    actions.push_back(newAsk);

    live_ask_id_ = new_ask_id;
    live_bid_id_ = new_bid_id;

    return actions;

}