#include "order_book.hpp"
#include <cassert>


std::optional<Px> OrderBook::best_of(const BidMap& m) const {
    if (m.empty()) {
        return std::nullopt;
    }
    return m.begin()->first;
}

std::optional<Px> OrderBook::best_of(const AskMap& m) const {
    if (m.empty()) {
        return std::nullopt;
    }
    return m.begin()->first;
}

std::optional<Px> OrderBook::best_bid() const {
    return best_of(bids_);
}

std::optional<Px> OrderBook::best_ask() const {
    return best_of(asks_);  
}

void OrderBook::clear() {
    bids_.clear();
    asks_.clear();
    id_index_.clear();
}


// add_limit: takes in a new limit order, matches against the opposite side if possible -> Fills 
// and if anything remains put the remaining in the order book
// limit order: trader specifies the max price they want to buy at and the min price they want to sell
std::vector<Fill> OrderBook::add_limit(const Order& in) {
    std::vector<Fill> out;
    if (!valid_order(in)) return out;

    Order o = in; // mutable copy of order

    auto take_from_level = [&](auto& side_map) {
        while (o.qty > 0 && !side_map.empty()) {
            auto lvl_it = side_map.begin();
            Px lvl_px = lvl_it->first;
            auto& fifo = lvl_it->second.fifo;

            bool crosses_now = (o.side == Side::Buy) ? (o.px >= lvl_px) : (o.px <= lvl_px);

            if (!crosses_now) break;

            while (o.qty > 0 && !fifo.empty()) {
                Order& maker = fifo.front(); // asks sotred in ascending order
                Qty traded = (o.qty < maker.qty) ? o.qty : maker.qty;

                out.push_back(Fill{
                        o.id,
                        maker.id,
                        lvl_px,
                        traded,
                        o.ts
                    });

                o.qty -= taded;
                maker.qty -= traded;

                if (maker.qty == 0) {
                    // can no longer sell this
                    id_index_.erase(maker.id);
                    fifo.pop_front();
                }
            }

            if (fifo.empty()) {
                side_map.erase(lvl_it);
            }
        }
    };

    if (o.side == Side::Buy) {
        take_from_level(ask_);
    } else {
        take_from_level(bids_);
    }


    // incoming order wasn't fully matched; save residual qty in book
    if (o.qty > 0) {
        auto& my_map = (o.side == Side::Buy) ? bids_ : asks_;
        auto [lvl_it, inserted] = my_map.try_emplace(o.px, Level{});
        Level& lvl = lvl_it->second;
        
        lvl.fifo.push_back(o);
        auto qit = std::prev(lvl.fifo.end()); // (qit - queue iterator) iterator pointing to this order


        Handle h;
        h.side = (o.side == Side::Buy) ? BookSideTag::Bid : BookSideTag::Ask;
        h.px = o.px;
        h.qit = qit;

        id_index_[o.id] = h;
    }
    return out;
}

bool OrderBook::cancel(OrderId id) {
    auto hit = id_index_.find(id);
    if (hit == id_index_.end()) return false;

    const Handle& h = hit->second;

    if (h.side == BookSideTag::Bid) {
        auto lvl_it = bids_.find(h.px);
        if (lvl_it == bids_.end()) {
            id_index_.erase(hit);
            return false;
        } 
        Level& lvl = lvl_it->second;
        lvl.fifo.erase(h.qit);
        id_index_.erase(hit);

        if (lvl.fifo.empty()) bids_.erase(lvl_it);
        return true;
    } else {
        auto lvl_it = asks_.find(h.px);
        if (lvl_it == asks_.end()) {
            id_index_.erase(hit);
            return false;
        }
        Level& lvl = lvl_it->second; // value of the map (px: lvl)
        lvl.fifo.erase(h.qit);
        id_index_.erase(hit);
        if (lvl.fifo.empty()) asks_.erase(lvl_it);
        return true;
    }
}
