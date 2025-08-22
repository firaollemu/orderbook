#pragma once
#include <deque>
#include <map>
#include <optional>
#include <unordered_map>
#include <vector>

#include "order.hpp"
#include "order_id_gen.hpp"

struct Level {
    Px px{0};
    std::deque<Order> fifo;
};

struct GreaterPx {
    bool operator()(const Px&a, const Px&b) const noexcept {
        return a > b;
    }
};

using BidMap = std::map<Px, Level, GreaterPx>;
using AskMap = std::map<Px, Level>;
using QueueIt = std::deque<Order>::iterator;

enum class BookSideTag : uint8_t {
    Bid,
    Ask
};

// A handle - a pointer into the live order book
// Exists while order is live in book
// Used for fast management of active orders
struct Handle {
    BookSideTag side;
    Px px;
    QueueIt qit; // iterator 
};

class OrderBook {
    public:
        std::vector<Fill> add_limit(const Order& o);
        bool cancel(OrderId id);
        std::optional<Px> best_bid() const;
        std::optional<Px> best_ask() const;
        void clear();
        std::vector<std::pair<Px, Qty>> top_bids(int depth) const;
        std::vector<std::pair<Px, Qty>> top_asks(int depth) const;

        struct SubmitResult {
            OrderId id;
            std::vector<Fill> fills;
        };
        
        SubmitResult submit_limit(Order o);

    private:
        BidMap bids_;
        AskMap asks_;

        std::unordered_map<OrderId, Handle> id_index_;

        std::optional<Px> best_of(const BidMap& m) const;
        std::optional<Px> best_of(const AskMap& m) const;
        OrderIdGen order_id_gen;
};

