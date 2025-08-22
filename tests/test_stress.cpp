#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

#include <chrono>
#include <random>
#include <vector>
#include <string>

#include "order_book.hpp"
#include "test_utils.hpp"


using testutil::mk_order;

TEST_CASE("Stress test: adds/cancels/fills with invariants") {
    OrderBook ob;

    using Clock = std::chrono::high_resolution_clock;

    const std::uint64_t N_ops = 300000;
    const double add_ratio = 0.7;
    const std::uint64_t N_adds = static_cast<std::uint64_t>(N_ops*add_ratio);
    const std::uint64_t N_cancels = N_ops - N_adds;


    const Px px_mid = 1000;
    const int px_halfband = 100;
    const Qty min_qty = 1;
    const Qty max_qty = 1000;
    const std::uint64_t cancel_every = 5;


    std::mt19937_64 rng(42);
    std::uniform_int_distribution<int> side_d(0, 1);
    std::uniform_int_distribution<int> dpx(-px_halfband, px_halfband);
    std::uniform_int_distribution<int> dqty(min_qty, max_qty);
    
    std::vector<OrderId> live_ids;
    live_ids.reserve(N_adds);

    OrderId next_id = 1;
    std::uint64_t adds = 0, cancels = 0, fills = 0;

    for (; adds < N_adds; ++adds) {
        const Side s = side_d(rng) == 0 ? Side::Buy : Side::Sell;
        Px px = px_mid + dpx(rng);
        if (px < 0) px = 0;
        const Qty qty = static_cast<Qty>(dqty(rng));

        const OrderId id = next_id++;

        auto fs = ob.add_limit(mk_order(id, s, px, qty));

        fills += fs.size();
        
        live_ids.push_back(id);

        if ((adds % cancel_every) == 0 && !live_ids.empty() && cancels < N_cancels) {
            std::uniform_int_distribution<size_t> pick(0, live_ids.size()-1);
            const size_t idx = pick(rng);
            const OrderId cid = live_ids[idx];

            if (ob.cancel(cid)) {
                ++cancels;
            }
            live_ids[idx] = live_ids.back();
            live_ids.pop_back();
        }
    }

    size_t i = 0;

    while (cancels < N_cancels && i < live_ids.size()) {
        if (ob.cancel(live_ids[i])) ++cancels;
        ++i;
    }

    auto best_bid = ob.best_bid();
    auto best_ask = ob.best_ask();

    // check such that there is no book crossing
    if (best_bid.has_value() && best_ask.has_value()) {
        REQUIRE(best_bid.value() <= best_ask.value());
    }
}
