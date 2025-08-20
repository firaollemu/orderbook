#include <catch2/catch_test_macros.hpp>
#include "order_book.hpp"
#include "test_utils.hpp"

using testutil::mk_order;

TEST_CASE("Cancel non-existent order id returns false and does nothing") {
    OrderBook ob;

    ob.add_limit(mk_order(1, Side::Buy, 1000, 10));
    ob.add_limit(mk_order(2, Side::Sell, 1010, 20));


    REQUIRE(ob.best_bid().has_value());
    REQUIRE(ob.best_bid().value() == 1000);
    REQUIRE(ob.best_ask().has_value());
    REQUIRE(ob.best_ask().value() == 1010);

    REQUIRE(ob.cancel(999) == false);

    REQUIRE(ob.best_bid().has_value());
    REQUIRE(ob.best_bid().value() == 1000);
    REQUIRE(ob.best_ask().has_value());
    REQUIRE(ob.best_ask().value() == 1010);
}

TEST_CASE("Cancel prev order removes level and updates best price") {
    OrderBook ob;
    
    ob.add_limit(mk_order(10, Side::Buy, 1000, 5));
    ob.add_limit(mk_order(11, Side::Buy, 995, 7));

    REQUIRE(ob.best_bid().has_value());
    REQUIRE(ob.best_bid().value() == 1000);


    REQUIRE(ob.cancel(10) == true);
    REQUIRE(ob.best_bid().has_value());
    REQUIRE(ob.best_bid().value() == 995);

    REQUIRE(ob.cancel(11) == true);
    REQUIRE_FALSE(ob.best_bid().has_value());

    ob.add_limit(mk_order(20, Side::Sell, 1010, 3));
    REQUIRE(ob.best_ask().has_value());
    REQUIRE(ob.best_ask().value() == 1010);

    REQUIRE(ob.cancel(20) == true);
    REQUIRE_FALSE(ob.best_ask().has_value());
}