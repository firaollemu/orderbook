#include <catch2/catch_test_macros.hpp>
#include "order_id_gen.hpp"

TEST_CASE("OrderIdGen is monotonic and unique") {
    OrderIdGen gen(100);

    REQUIRE(gen.peek() == 100);
    auto a = gen.next();
    auto b = gen.next();
    auto c = gen.next();

    REQUIRE(a == 100);
    REQUIRE(b == 101);
    REQUIRE(c == 102);
    REQUIRE(gen.peek() == 103);

    gen.reset(1);
    REQUIRE(gen.next() == 1);
    REQUIRE(gen.next() == 2);
}