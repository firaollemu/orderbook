#include <catch2/catch_test_macros.hpp>
#include "order_book.hpp"
#include "order.hpp"


static Order mk_order(OrderId id, Side s, Px px, Qty qty) {
    Order o;
    o.id = id;
    o.side = s;
    o.px = px;
    o.qty = qty;
    o.ts = 1;
    return o;
}


TEST_CASE("Add: resting order goes to correct side and best price updates") {
    OrderBook ob;

    auto fills1 = ob.add_limit(mk_order(1, Side::Buy, 1000, 50));
    REQUIRE(fills1.empty());
    REQUIRE(ob.best_bid().has_value());
    REQUIRE(ob.best_bid().value() == 1000);
    REQUIRE(ob.best_ask().has_value() == false); // No sell orders yet

    ob.add_limit(mk_order(2, Side::Buy, 1005, 10));
    REQUIRE(ob.best_bid().value() == 1005);

    ob.add_limit(mk_order(3, Side::Sell, 1010, 20));
    REQUIRE(ob.best_ask().has_value()); // Now we should have a best ask
    REQUIRE(ob.best_ask().value() == 1010);
}
