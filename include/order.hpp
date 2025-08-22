#pragma once
#include <cstdint>
#include <type_traits>


using OrderId = std::uint64_t;
using Px = std::int64_t;
using Qty = std::int64_t;

constexpr OrderId kInvalidOrderId = 0;

enum class Side : std::uint8_t {
    Buy = 0,
    Sell = 1
};

inline int side_sign(Side s) noexcept {
    return s == Side::Buy ? +1 : -1;
}

inline bool crosses(Px buy_px, Px sell_px) noexcept {
    return buy_px >= sell_px;
}

struct Order {
  OrderId id{0};
  Side side{Side::Buy};
  Px px{0};
  Qty qty{0};
  std::uint64_t ts{0};
  std::uint32_t user_tag{0};
};


// Fill -  a record of an actual trade execuation because of two orders matching
// Created when two orders match
// Report of completed trades
struct Fill {
    OrderId taker{ kInvalidOrderId }; // incoming order
    OrderId maker{ kInvalidOrderId }; // resting order
    Px px{ 0 }; // price
    Qty qty{ 0 }; // quantity
    std::uint64_t ts { 0 }; // timestamp
};


inline const char* to_cstr(Side s) noexcept {
    return s == Side::Buy ? "Buy" : "Sell";
}

inline bool valid_id(OrderId id) noexcept {
    return id != kInvalidOrderId;
}

inline bool valid_order(const Order& o) noexcept {
    return valid_id(o.id) && o.qty > 0 && o.px >= 0;
}


