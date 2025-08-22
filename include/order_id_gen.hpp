#pragma once
#include <atomic>
#include <limits>
#include <stdexcept>
#include "order.hpp"

using OrderId = std::uint64_t;

// Generate a monotonic, unique OrderId
class OrderIdGen {
    public:
    explicit OrderIdGen(OrderId start = static_cast<OrderId>(1)) : counter_(start) {}

    OrderId next() {
        OrderId v = counter_.fetch_add(static_cast<OrderId>(1), std::memory_order_relaxed);
        if (v == std::numeric_limits<OrderId>::max()) {
            throw std::overflow_error("OrderIdGen overflow");
        }
        return v;
    }

    OrderId peek() const {
        return counter_.load(std::memory_order_relaxed);
    }

    void reset(OrderId start = static_cast<OrderId>(1)) {
        counter_.store(start, std::memory_order_relaxed);
    }

    private:
    std::atomic<OrderId> counter_;
};