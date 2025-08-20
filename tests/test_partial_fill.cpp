#include <catch2/catch_test_macros.hpp>
#include "order_book.hpp"
#include "test_utils.hpp"

using testutil::mk_order;


TEST_CASE("Partial Fill: taker < maker; maker stays in book with reduced size") {
    OrderBook ob;

    ob.add_limit(mk_order(100, Side::Sell, 1008, 30));
    REQUIRE(ob.best_ask().has_value());
    REQUIRE(ob.best_ask().value() == 1008);

    auto fill1 = ob.add_limit(mk_order(200, Side::Buy, 1008, 10));
    REQUIRE(fill1.size() == 1);
    REQUIRE(fill1[0].maker == 100);
    REQUIRE(fill1[0].px == 1008);
    REQUIRE(fill1[0].qty == 10);

    auto fill2 = ob.add_limit(mk_order(201, Side::Buy, 1008, 25));
    REQUIRE(fill2.size() == 1);
    REQUIRE(fill2[0].maker == 100);
    REQUIRE(fill2[0].px == 1008);
    REQUIRE(fill2[0].qty == 20);

    REQUIRE(ob.best_ask().has_value() == false);
}

TEST_CASE("Partial fill: taker spans multiple makers; remaining stays in orderbook") {
    OrderBook ob;

    ob.add_limit(mk_order(300, Side::Sell, 1008, 10));
    ob.add_limit(mk_order(301, Side::Sell, 1008, 12));
    REQUIRE(ob.best_ask().has_value());
    REQUIRE(ob.best_ask().value() == 1008);

    auto fill1 = ob.add_limit(mk_order(400, Side::Buy, 1008, 18));
    REQUIRE(fill1.size() == 2);
    REQUIRE(fill1[0].maker == 300);
    REQUIRE(fill1[0].px == 1008);
    REQUIRE(fill1[0].qty == 10);
    REQUIRE(fill1[1].maker == 301);
    REQUIRE(fill1[1].px == 1008);
    REQUIRE(fill1[1].qty == 8);

    auto fill2 = ob.add_limit(mk_order(401, Side::Buy, 1008, 4));
    REQUIRE(fill2.size() == 1);
    REQUIRE(fill2[0].maker == 301);
    REQUIRE(fill2[0].px == 1008);
    REQUIRE(fill2[0].qty == 4);

    REQUIRE(ob.best_ask().has_value() == false);
}

