#pragma once
#include <cstdint>
#include "order.hpp"

struct PnL {
    double cash{0.0};
    long long inventory{0};

    // updates inventory and cash when a trade happens
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
    // it gives the true value of your position at any given moment -> aka actual PnL
    // cash -> is the realized PnL from trades that have been completed
    // inventory * mid_price -> unrealized PnL i.e. value fo the current open, position
    // since they are open positions it is not real cash but we need to understand its value to understand our exposure
    // here we are exposed to change in instrument prices
    // as a market maker we want our inventory close to zero (flat) to minimize exposure to price changes
    inline double mark_to_market(Px mid) const noexcept {
        return cash + static_cast<double>(inventory) * static_cast<double>(mid);
    }

    inline void clear() noexcept {
        cash = 0.0;
        inventory = 0;
    }
};