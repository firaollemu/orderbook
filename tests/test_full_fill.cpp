#include <catch2/catch_test_macros.hpp>
#include "order_book.hpp"
#include "test_utils.hpp"

using testutil::mk_order;

TEST_CASE("Full fill: maker and taker request same amount. Fill goes through. Both are gone.") {
    OrderBook ob;

    ob.add_limit(mk_order(100, Side::Sell, 1001, 30));
    REQUIRE(ob.best_ask().has_value());
    REQUIRE(ob.best_ask().value() == 1001);

    auto fill = ob.add_limit(mk_order(200, Side::Buy, 1001, 30));
    REQUIRE(fill.size() == 1);
    REQUIRE(fill[0].maker == 100);
    REQUIRE(fill[0].taker == 200);
    REQUIRE(fill[0].px == 1001);
    REQUIRE(fill[0].qty == 30);

    REQUIRE_FALSE(ob.best_ask().has_value());
    REQUIRE_FALSE(ob.best_bid().has_value());


    SECTION("Reverse. Buy order first and sell ordr next") {
        OrderBook ob2;
        ob2.add_limit(mk_order(300, Side::Buy, 1001, 50));
        auto fill1 = ob2.add_limit(mk_order(400, Side::Sell, 1001, 50));

        REQUIRE(fill1.size() == 1);
        REQUIRE(fill1[0].qty == 50);
        REQUIRE_FALSE(ob2.best_bid().has_value());
        REQUIRE_FALSE(ob2.best_ask().has_value());
    }
}