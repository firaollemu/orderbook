#include <catch2/catch_test_macros.hpp>
#include "order_book.hpp"
#include "test_utils.hpp"

using testutil::mk_order;

TEST_CASE("Snapshot returns top n levels with summed qty") {
    OrderBook ob;

    auto fills = ob.add_limit(mk_order(1, Side::Buy, 100, 30));
    REQUIRE(fills.empty());
    fills = ob.add_limit(mk_order(2, Side::Buy, 100, 20));
    REQUIRE(fills.empty());
    fills = ob.add_limit(mk_order(3, Side::Buy, 99, 10));
    REQUIRE(fills.empty());
    fills = ob.add_limit(mk_order(4, Side::Sell, 101, 15));
    REQUIRE(fills.empty());
    fills = ob.add_limit(mk_order(5, Side::Sell, 102, 25));
    REQUIRE(fills.empty());
    fills = ob.add_limit(mk_order(6, Side::Sell, 102, 5));
    REQUIRE(fills.empty());

    SECTION("Get top 1 level on both ask and bid") {
        auto top_bid = ob.top_bids(1);
        auto top_ask = ob.top_asks(1);

        REQUIRE(top_bid.size() == 1);
        REQUIRE(top_bid[0].first == 100);
        REQUIRE(top_bid[0].second == 50); // two items at 100 20+30

        REQUIRE(top_ask.size() == 1);
        REQUIRE(top_ask[0].first == 101); // top ask is the smallest price
        REQUIRE(top_ask[0].second == 15);
    }

    SECTION("Get top 2 level on both ask and bid") {
        auto top_bid = ob.top_bids(2);
        auto top_ask = ob.top_asks(2);

        REQUIRE(top_bid.size() == 2);
        REQUIRE(top_bid[0].first == 100);
        REQUIRE(top_bid[0].second == 50); // two items at 100 20+30
        REQUIRE(top_bid[1].first == 99);
        REQUIRE(top_bid[1].second == 10);

        REQUIRE(top_ask.size() == 2);
        REQUIRE(top_ask[0].first == 101); // top ask is the smallest price
        REQUIRE(top_ask[0].second == 15);
        REQUIRE(top_ask[1].first == 102);
        REQUIRE(top_ask[1].second == 30);
    }

    SECTION("Get more levels than avaialble") {
        auto top_bid = ob.top_bids(10);
        auto top_ask = ob.top_asks(10);

        REQUIRE(top_bid.size() == 2); // the two converge to one i.e. 2 102 sells at different prices merge
        REQUIRE(top_ask.size() == 2);
    }
}
