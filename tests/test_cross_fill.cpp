#include <catch2/catch_test_macros.hpp>
#include "order_book.hpp"
#include "test_utils.hpp"

using testutil::mk_order;

TEST_CASE("Crossing generates fills at maker prices with FIFO across levells") {
    OrderBook ob;

    ob.add_limit(mk_order(10, Side::Sell, 1010, 30));
    ob.add_limit(mk_order(11, Side::Sell, 1008, 25));
    REQUIRE(ob.best_ask().has_value());
    REQUIRE(ob.best_ask().value() == 1008); // from buyer's perspective best is the smallest ask


    auto fills = ob.add_limit(mk_order(20, Side::Buy, 1009, 40));
    REQUIRE(fills.size() == 1);
    REQUIRE(fills[0].maker == 11);
    REQUIRE(fills[0].px == 1008);
    REQUIRE(fills[0].qty == 25); // only 25 available in orderid11


    REQUIRE(ob.best_bid().has_value());
    REQUIRE(ob.best_bid().value() == 1009); // all of 1008 are gone

    auto fills2 = ob.add_limit(mk_order(21, Side::Buy, 1010, 50));
    REQUIRE(fills2.size() == 1);
    REQUIRE(fills2[0].maker == 10);
    REQUIRE(fills2[0].px == 1010);
    REQUIRE(fills2[0].qty == 30);
    REQUIRE(ob.best_bid().value() == 1010);
}