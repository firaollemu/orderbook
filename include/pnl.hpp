#pragma once
#include <cstdint>
#include "order.hpp"

struct PnL {
    double cash{0.0};
    long long inventory{0};

    inline void on_trade(Side my_side, Px px, Qty qty) noexcept {
        const double notional = static_cast<double>(px) * static_cast<double>(qty);
        long long long_long_qty = static_cast<long long>(qty);
        if (my_side == Side::Buy) {
            inventory += long_long_qty;
            cash -= notional;
        } else {
            inventory -= long_long_qty;
            cash += notional;
        }
    }

    // mark-to-market: gives us a way to value current inventory usnig market price
    inline double mark_to_market(Px mid) const noexcept {
        return cash + static_cast<double>(inventory) * static_cast<double>(mid);
    }

    inline void clear() noexcept {
        cash = 0.0;
        inventory = 0;
    }
};