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

    auto take_from_asks = [&]() {
        while (o.qty > 0 && !asks_.empty()) {
            auto lvl_it = asks_.begin();
            Px lvl_px = lvl_it->first;
            auto& fifo = lvl_it->second.fifo;

            bool crosses_now = o.side == Side::Buy && o.px >= lvl_px;
            if (!crosses_now) break;

            while (o.qty > 0 && !fifo.empty()) {
                Order& maker = fifo.front();
                Qty traded = std::min(o.qty, maker.qty);

                out.push_back(Fill{o.id, maker.id, lvl_px, traded, o.ts});
                o.qty -= traded;
                maker.qty -= traded;

                if (maker.qty == 0) {
                    id_index_.erase(maker.id);
                    fifo.pop_front();
                }
            }

            if (fifo.empty()) {
                asks_.erase(lvl_it);
            }
        }
    };

    auto take_from_bids = [&]() {
        while (o.qty > 0 && !bids_.empty()) {
            auto lvl_it = bids_.begin();
            Px lvl_px = lvl_it->first;
            auto& fifo = lvl_it->second.fifo;

            bool crosses_now = o.side == Side::Sell && o.px <= lvl_px;
            if (!crosses_now) break;

            while (o.qty > 0 && !fifo.empty()) {
                Order& maker = fifo.front();
                Qty traded = std::min(o.qty, maker.qty);

                out.push_back(Fill{o.id, maker.id, lvl_px, traded, o.ts});
                o.qty -= traded;
                maker.qty -= traded;

                if (maker.qty == 0) {
                    id_index_.erase(maker.id);
                    fifo.pop_front();
                }
            }

            if (fifo.empty()) {
                bids_.erase(lvl_it);
            }
        }
    };

    // First try to match with opposite side
    if (o.side == Side::Buy) {
        take_from_asks();
    } else {
        take_from_bids();
    }


    // incoming order wasn't fully matched; save residual qty in book
    if (o.qty > 0) {
        if (o.side == Side::Buy) {
            auto [lvl_it, inserted] = bids_.try_emplace(o.px, Level{});
            Level& lvl = lvl_it->second;
            
            lvl.fifo.push_back(o);
            auto qit = std::prev(lvl.fifo.end());

            Handle h;
            h.side = BookSideTag::Bid;
            h.px = o.px;
            h.qit = qit;

            id_index_[o.id] = h;
        } else {
            auto [lvl_it, inserted] = asks_.try_emplace(o.px, Level{});
            Level& lvl = lvl_it->second;
            
            lvl.fifo.push_back(o);
            auto qit = std::prev(lvl.fifo.end());

            Handle h;
            h.side = BookSideTag::Ask;
            h.px = o.px;
            h.qit = qit;

            id_index_[o.id] = h;
        }
    }
    return out;
}

namespace {
    inline Qty level_total_qty(const Level& lvl) noexcept {
        Qty sum = 0;
        for (const auto& o : lvl.fifo) sum += o.qty;
        return sum;
    }
}

template <typename PriceMap>
std::vector<std::pair<Px, Qty>>
top_of_side(const PriceMap& side, int depth) {
    std::vector<std::pair<Px, Qty>> out;
    if (depth <= 0 || side.empty()) return out;
    out.reserve(static_cast<size_t>(depth));

    auto it = side.begin();
    int count = 0;

    while (it != side.end() && count < depth) {
        const Px px = it->first;
        const Qty qty = level_total_qty(it->second);

        if (qty > 0) {
            out.emplace_back(px, qty);
            count++;
        }

        ++it;
    }

    return out;
}

// get the top depth bids/asks available from the order book
std::vector<std::pair<Px, Qty>> OrderBook::top_bids(int depth) const {
    return top_of_side(bids_, depth);
}

std::vector<std::pair<Px, Qty>> OrderBook::top_asks(int depth) const {
    return top_of_side(asks_, depth);
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

OrderBook::SubmitResult OrderBook::submit_limit(Order o) {
    if (o.id == kInvalidOrderId) {
        o.id = order_id_gen.next();
    }

    auto fills = add_limit(o);
    return SubmitResult{
        o.id,
        std::move(fills)
    };
}


std::vector<Fill> OrderBook::add_market(const MarketOrder& mo) {
    std::vector<Fill> out;
    Qty need = mo.qty;

    auto fill_logic = [&](auto& opposite_side) {
        while (need > 0 && !opposite_side.empty()) {
            auto lvl_it = opposite_side.begin();
            auto& fifo = lvl_it->second.fifo;

            while (need > 0 && !fifo.empty()) {
                Order& maker = fifo.front();
                Qty traded = std::min<Qty>(need, maker.qty);

                out.push_back(Fill{mo.id, maker.id, lvl_it->first, traded, mo.ts});

                need -= traded;
                maker.qty -= traded;

                if (maker.qty == 0) {
                    id_index_.erase(maker.id);
                    fifo.pop_front();
                }
            }
            if (fifo.empty()) {
                opposite_side.erase(lvl_it);
            }
        }
    };

    if (mo.side == Side::Buy) {
        fill_logic(asks_);
    } else {
        fill_logic(bids_);
    }

    return out;
}