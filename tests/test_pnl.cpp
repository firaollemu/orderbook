#include <catch2/catch_test_macros.hpp>
#include "pnl.hpp"



TEST_CASE("PnL buy then sell updates cash/invenotry and mark-to-market") {
    PnL pnl;

    pnl.on_trade(Side::Buy, 100, 10);
    REQUIRE(pnl.inventory == 10);
    long long cash_in_cents = static_cast<long long>(pnl.cash * 100);
    REQUIRE(cash_in_cents == -1000*100);


    pnl.on_trade(Side::Sell, 102, 4);
    REQUIRE(pnl.inventory == 6); // bought 10; sold 4; left with 6
    cash_in_cents = static_cast<long long>(pnl.cash * 100);
    REQUIRE(cash_in_cents == (-1000 + 408) * 100);

    double mtm = pnl.mark_to_market(101);
    long long mtm_in_cents = static_cast<long long>(mtm * 100);
    REQUIRE(mtm_in_cents == 14 * 100);
}