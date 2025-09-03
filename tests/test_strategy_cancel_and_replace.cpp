#include <catch2/catch_test_macros.hpp>
#include "order_book.hpp"
#include "pnl.hpp"
#include "order_id_gen.hpp"
#include "strategy/strategy.hpp"
#include "strategy/baseline_market_making.hpp"
#include "strategy/runner.hpp"

// const OrderBook& ob -> cannot modify ob
// return the snapshot of the top 10 bids or asks
// return qty at matching price level on either the ask/bid side
static Qty qty_at_price(const OrderBook& ob, Side side, Px px) {
    auto snap = (side == Side::Buy) ? ob.top_bids(10) : ob.top_asks(10);
    for (auto [pc, qty] : snap) {
        if (pc == px) return qty;
    }
    return 0;
}

TEST_CASE("Baseline market making does not stack identical quotes") {
    // goal: we don't want to place a new order if an identical one is already active
    OrderBook ob;
    PnL pnl;
    OrderIdGen ids;
    aStrategy::BaseLineMarketMaking mm;

    // create components needed to run the market-making strategy
    EngineAPI api {
        ob,
        pnl,
        ids,
        1 // timestamp
    };

    auto actions1 = mm.step(api);
    // need to apply the actions returned by the market-making strategy to the orderbook
    aStrat::apply_modifications(actions1, api);
    // our fallback mid price = 100
    REQUIRE(qty_at_price(ob, Side::Buy, 99) == 5); // 5 is predefined in the baseline market making strategy
    REQUIRE(qty_at_price(ob, Side::Sell, 101) == 5);

    // market conditions havent changed so we expect no new orders to be placed
    // strategy needs to see that the target bid and target ask are the same as the existing orders
    // so it will not place new orders i.e. does nothing
    api.ts = 2;
    auto actions2 = mm.step(api);
    REQUIRE(actions2.empty());
    aStrat::apply_modifications(actions2, api);
    REQUIRE(qty_at_price(ob, Side::Buy, 99) == 5);
    REQUIRE(qty_at_price(ob, Side::Sell, 101) == 5);
}

TEST_CASE("Baseline market making places new orders if target prices have changed") {
    OrderBook ob;
    PnL pnl;
    OrderIdGen ids;
    aStrategy::BaseLineMarketMaking mm;

    EngineAPI api {
        ob,
        pnl,
        ids,
        1
    };

    auto actions3 = mm.step(api);
    aStrat::apply_modifications(actions3, api);
    REQUIRE(qty_at_price(ob, Side::Buy, 99) == 5); // orderbook is initially empty
    REQUIRE(qty_at_price(ob, Side::Sell, 101) == 5);

    pnl.inventory = 3;
    api.ts = 2; // ts -> time step
    auto actions4 = mm.step(api);
    aStrat::apply_modifications(actions4, api);

    // now our inventory has changed; we are long 
    // fallback mid = 100, half = 1, skew = 1*3 = 3
    REQUIRE(qty_at_price(ob, Side::Buy, 99) == 0); // our old bid has to be cancled
    REQUIRE(qty_at_price(ob, Side::Sell, 101) == 0);
    REQUIRE(qty_at_price(ob, Side::Buy, 96) == 5);
    REQUIRE(qty_at_price(ob, Side::Sell, 98) == 5);
}