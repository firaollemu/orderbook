#include <catch2/catch_test_macros.hpp>
#include "order_book.hpp"
#include "test_utils.hpp"

using testutil::mk_order;

TEST_CASE("Cancel removes resting order and empty level") {
    OrderBook ob;

    ob.add_limit(mk_order(1, Side::Sell, 1050, 10));
    ob.add_limit(mk_order(2, Side::Sell, 1050, 20));

    REQUIRE(ob.best_ask().has_value());
    REQUIRE(ob.best_ask().value() == 1050);

    // cancel first id
    REQUIRE(ob.cancel(1) == true);
    REQUIRE(ob.best_ask().value() == 1050);

    // cancel second id
    REQUIRE(ob.cancel(2) == true);
    REQUIRE_FALSE(ob.best_ask().has_value());

    REQUIRE_FALSE(ob.cancel(1111)); // nonexistent order
}