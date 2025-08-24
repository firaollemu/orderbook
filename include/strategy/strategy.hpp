#pragma once
#include <cstdint>
#include <vector>
#include "order_book.hpp"
#include "pnl.hpp"
#include "order_id_gen.hpp"


struct EngineAPI {
    OrderBook& ob;
    PnL& pnl;
    OrderIdGen& ids;
    std::uint64_t ts;
};


enum class ModType : uint8_t {
    New, 
    Cancel,
    Modify
};


struct ModAction {
    ModType type;
    OrderId id = kInvalidOrderId;
    Side side;
    Px px;
    Qty qty;
};


// use struct instead of class since we want all memebers to be public
struct StrategyInterface {
    // if a virtual function is redefined in the derived class, the function in the derived class is executed
    // virtual function: allow for polymorphisms in cpp
    virtual  ~StrategyInterface() = default;

    virtual std::vector<ModAction> step(EngineAPI& api) = 0;
    // = 0 forces the derived class to provide an implementation
    // each different strategy just implements step() and returns a set of actions (QuoteAction)
};