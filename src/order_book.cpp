#include "order.hpp"
#include <iostream>

void test_order() {
    Order o;
    o.id = 1;
    o.px = 1050;
    o.qty = 100;

    std::cout << "Order id=" << o.id
            << " side=" << to_cstr(o.side)
            << " px=" << o.px
            << " qty=" << o.qty << "\n";

}