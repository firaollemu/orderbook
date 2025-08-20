#include <iostream>
#include "order_book.hpp"

int main() {
    OrderBook ob;

    ob.add_limit({1, Side::Buy, 1000, 10});
    std::cout << "Best bid: "
        << (ob.best_bid().has_value() ? std::to_string(*ob.best_bid()) : "none")
        << std::endl;
    
    ob.add_limit({2, Side::Sell, 1010, 20});
    std::cout << "Best ask: "
        << (ob.best_ask().has_value() ? std::to_string(*ob.best_ask()) : "none")
        << std::endl;

    
    ob.cancel(1);
    std::cout << "After cancel, best bid : "
    << (ob.best_bid().has_value() ? std::to_string(*ob.best_bid()) : "none") 
    << std::endl;

    return 0;
}
