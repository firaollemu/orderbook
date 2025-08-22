#pragma once
#include "order.hpp"

namespace orderutil {
    inline Order mk_order(OrderId id, Side s, Px px, Qty qty, uint64_t ts=1, std::uint32_t user_tag = 0) {
        return Order{ id, s, px, qty, ts, user_tag };
    }
}