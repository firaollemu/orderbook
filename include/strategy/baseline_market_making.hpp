#pragma once
#include <optional>
#include <cstdint>
#include "strategy/strategy.hpp"
#include "order_utils.hpp"


namespace aStrategy {
    // params needed for quoter
    struct QuoteParams {
        int spread = 2;
        int skew_per_unit = 1;
        int qty = 5;
        Px fallback_mid = 100; // i.e. when book is empty
    };


class BaseLineMarketMaking : public StrategyInterface {
public:
    explicit BaseLineMarketMaking(QuoteParams p = {}) : quote_param_(p) {}

    std::vector<ModAction> step(EngineAPI& api) override;

private:
    QuoteParams quote_param_;

    std::optional<OrderId> bid_id_;
    std::optional<OrderId> ask_id_;
    std::optional<Px> bid_px_;
    std::optional<Px> ask_px_;
    int last_qty_{0};

    inline Px compute_mid(const EngineAPI& api) const {
        auto best_bid = api.ob.best_bid();
        auto best_ask = api.ob.best_ask();
        if (best_bid && best_ask) return (*best_bid + *best_ask) / 2;
        if (best_bid) return *best_bid;
        if (best_ask) return *best_ask;
        return quote_param_.fallback_mid;
    }
};
}