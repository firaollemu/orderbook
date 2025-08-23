#include <catch2/catch_test_macros.hpp>
#include "test_utils.hpp"
#include "order_book.hpp"
#include "order.hpp"


using testutil::mk_order;

TEST_CASE("Market buy consumes asks best-first") {
    OrderBook ob;
    
    ob.submit_limit(mk_order(kInvalidOrderId, Side::Sell, 101, 30));
    ob.submit_limit(mk_order(kInvalidOrderId, Side::Sell, 102, 25));

    MarketOrder mo {
        9001,
        Side::Buy,
        40,
    };

    auto fills = ob.add_market(mo);

    REQUIRE(fills.size() >= 1);
    REQUIRE(fills[0].px == 101);
    REQUIRE(fills[0].qty == 30);
    REQUIRE(fills[1].px == 102);
    REQUIRE(fills[1].qty == 10);


    auto top_ask = ob.top_asks(2);
    REQUIRE(top_ask.size() >= 1);
    REQUIRE(top_ask[0].first == 102);
    REQUIRE(top_ask[0].second == 15);

}