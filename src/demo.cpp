#include <iostream>
#include <vector>
#include <string>
#include "order_book.hpp"
#include "order.hpp"
#include "order_utils.hpp"
#include "order_id_gen.hpp"
#include "pnl.hpp"
#include "csv_logger.hpp"

using orderutil::mk_order;

static void print_fills(const std::vector<Fill>& fills, const std::string& label) {
    std::cout << "\n=== " << label << " ===\n";
    for (const auto& f : fills) {
        std::cout << "trade: px=" << f.px
                  << " qty=" << f.qty
                  << " taker=" << f.taker
                  << " maker=" << f.maker
                  << " ts=" << f.ts
                  << std::endl;
    }
}


static void print_top(const OrderBook& ob, int depth = 5) {
    auto top_bid = ob.top_bids(depth);
    auto top_ask = ob.top_asks(depth);

    std::cout << "\n-- Top of Book (depth " << depth << ") ---\n";
    std::cout << "BIDS (px x qty):\n";
    for (auto& [px, q] : top_bid) std::cout << " " << px << " x " << q << "\n";

    std::cout << "ASKS (px x qty): \n";
    for (auto& [px, q] : top_ask) std::cout << " " << px << " x" << q << "\n";

    auto best_bid = ob.best_bid();
    auto best_ask = ob.best_ask();
    std::cout << "best_bid=" << (best_bid ? std::to_string(*best_bid) : "none")
              << " best_ask=" << (best_ask ? std::to_string(*best_ask) : "none")
              << std::endl;
}


int main() {
    OrderBook ob;
    PnL pnl;
    
    static uint64_t step = 0; // static -> lasts for the whole duration

    auto b1 = ob.submit_limit(mk_order(kInvalidOrderId, Side::Buy, 100, 10));
    auto b2 = ob.submit_limit(mk_order(kInvalidOrderId, Side::Buy, 99, 20));
    auto a1 = ob.submit_limit(orderutil::mk_order(kInvalidOrderId, Side::Sell, 102, 15));

    print_top(ob, 5); 
   
    auto res = ob.submit_limit(orderutil::mk_order(kInvalidOrderId, Side::Buy, 103, 10));

    csvlog::append_fills(res.fills);
    for (const auto& f : res.fills) {
        std::cout << "Fill: " << f.qty << " @ " << f.px << "\n";
        pnl.on_trade(Side::Buy, f.px, f.qty);
    }


    auto best_bid = ob.best_bid();
    auto best_ask = ob.best_ask();

    Px mid = (best_bid && best_ask) ? ((*best_bid + *best_ask) / 2) : (best_bid ? *best_bid : (best_ask ? *best_ask : 0));

    csvlog::append_pnl(pnl.cash, pnl.inventory, pnl.mark_to_market(mid), ++step);
    csvlog::append_snapshot(ob.top_bids(5), ob.top_asks(5), step);
    std::cout << "PnL cash = " << pnl.cash << "\n";
    std::cout << "PnL inventory = " << pnl.inventory << "\n";
    std::cout << "PnL mark-to-market = " << pnl.mark_to_market(mid) << "\n";
    
    bool ok = ob.cancel(2); 
    std::cout << "cancel result: " << ok << "\n";

    auto bids = ob.top_bids(5);
    auto asks = ob.top_asks(5);

    std::cout << "Bids:\n";
    for (auto [px, qty] : bids) std::cout << px << " x " << qty << "\n";

    std::cout << "Asks:\n";
    for (auto [px, qty] : asks) std::cout << px << " x " << qty << "\n";

    return 0;

}
