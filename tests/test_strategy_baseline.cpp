#include <catch2/catch_test_macros.hpp>

#include "order_book.hpp"
#include "pnl.hpp"
#include "order_id_gen.hpp"
#include "order_utils.hpp"
#include "strategy/strategy.hpp"
#include "strategy/runner.hpp"
#include "strategy/baseline_market_making.hpp"


TEST_CASE("baseline_market_making: empty orderbook") {
    OrderBook ob;
    PnL pnl;
    OrderIdGen ids;
    aStrategy::BaseLineMarketMaking mm;
    
    EngineAPI api{ob, pnl, ids, 1};
    auto actions = mm.step(api);


    REQUIRE(actions.size() == 2);
    auto bid = actions[0];
    auto ask = actions[1];
    REQUIRE(bid.side == Side::Buy);
    REQUIRE(ask.side == Side::Sell);
    REQUIRE(bid.px == 99);
    REQUIRE(ask.px == 101);
    REQUIRE(bid.qty == 5);
    REQUIRE(ask.qty == 5);
}


TEST_CASE("baseline_market_making: best bid and ask") {
    OrderBook ob;
    PnL pnl;
    pnl.inventory = 4;
    OrderIdGen ids;
    aStrategy::BaseLineMarketMaking mm;
    
    EngineAPI api{ob, pnl, ids, 1};
    auto actions = mm.step(api);
    // here we are using 100 as the default value
    // so bid = 100-1-4
    // so ask = 100+1-4 
    REQUIRE(actions.size() == 2);
    REQUIRE(actions[0].px == 95);
    REQUIRE(actions[1].px == 97);
}


TEST_CASE("no buy no ask") {
    OrderBook ob;
    PnL pnl;
    OrderIdGen ids;
    aStrategy::BaseLineMarketMaking mm;

    EngineAPI api{ob, pnl, ids, 1};
    auto actions = mm.step(api);
    aStrat::apply_modifications(actions, api);


    auto best_bid = ob.best_bid();
    auto best_ask = ob.best_ask();
    REQUIRE(best_bid.has_value());
    REQUIRE(best_ask.has_value());
    // here default value for mid price = 100 skew is 2
    REQUIRE(*best_bid == 99);
    REQUIRE(*best_ask == 101);
}