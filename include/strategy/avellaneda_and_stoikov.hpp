#pragma once
#include <deque>
#include <vector>
#include <cmath>
#include <cstdint>
#include "order_book.hpp"
#include "pnl.hpp"
#include "order_id_gen.hpp"
#include "order.hpp"
#include "modification_action.hpp"
#include "strategy.hpp"


struct AvellanedaStoikovParams {
    double gamma = 0.10;
    double kappa = 1.00;
    int horizon_bars = 5;
    int vol_window = 60;
    double tick = 0.01;
    double beta_mom = 0.0;
    double min_half_spread = 0.00;
    double max_half_spread = 1.00;
    double quote_size = 100.0;
};



class AvellanedaStoikovStrat final : public StrategyInterface {
    public:
      explicit AvellanedaStoikovStrat(const AvellanedaStoikovParams p = {}) {
        p_ = p;
      }
      std::vector<ModificationAction> step(EngineAPI& api) override;

    private:
      AvellanedaStoikovParams p_;


      std::deque<double> returns_;
      double sum_ = 0.0;
      double sumsq_ = 0.0;
      bool have_last_mid_ = false;
      double last_mid_ = NAN;

      std::optional<OrderId> live_bid_id_;
      std::optional<OrderId> live_ask_id_;

      static inline double round_to_tick(double px, double tick, bool is_bid) {
        if (tick <= 0.0 || std::isnan(px)) return px;
        const double n = px/tick;
        return (is_bid ? std::floor(n) : std::ceil(n)) * tick;
      }


      static inline double safe_mid_from_book(const OrderBook& ob) {
        const double best_bid = static_cast<double>(ob.best_bid());
        const double best_ask = static_cast<double>(ob.best_ask());

        return (best_bid > 0.0 &&  best_ask > 0.0) ? 0.5 * (best_bid + best_ask) : NAN;
      }

      static inline double current_inventory(const PnL& pnl) {
        return static_cast<double>(pnl.inventory);
      }

      double update_sigma(const double mid) {
        if (!std::isfinite(mid) || mid <= 0.0) return 0.0;
        if (have_last_mid_) {
            const double log_return = std::log(mid) - std::log(last_mid_);
            returns_.push_back(log_return);
            sum_ += log_return;
            sumsq_ += log_return*log_return;
            if (static_cast<int>(returns_.size()) > p_.vol_window) {
                const double old = returns_.front();
                returns_.pop_front();
                sum_ -= old;
                sumsq_ -= old*old;
            }

        }
        last_mid_ = mid;
        have_last_mid_ = true;

        const int n = static_cast<int>(returns_.size());
        if (n <= 1) return 0.0;
        const double mean = sum_ / n;
        const double var = std::max(0.0, (sumsq_/n) - mean*mean);
        return std::sqrt(var);
      }
}