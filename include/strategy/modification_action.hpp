#pragma once 
#include <cstdint>
#include <optional>
#include "order.hpp"

// an order: OrderId, Side, Px, Qty

namespace aStrategy {
    // modtype specifies the ation to be taken on an order
    // as a market maker we quote both a buy (bid) and a sell (ask) price for different instruments
    // modtype helps with this quoting process
    // ModType: New -> quote a new price
    // ModType: Cancel -> used to remove an existing quote from the market
    // we want to cancel a quote due to changing market conditions
    enum class ModificationType : uint8_t { New, Cancel};

    struct ModificationAction {
        ModificationType type;
        OrderId id; // need id to cancel exisitng orders

        std::optional<Side> side;
        std::optional<Px> px;
        std::optional<Qty> qty;
    };
}