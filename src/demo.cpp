#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <filesystem>


#include "order_book.hpp"
#include "order.hpp"
#include "order_id_gen.hpp"
#include "pnl.hpp"
#include "csv_logger.hpp"

#include "strategy/strategy.hpp"
#include "strategy/modification_action.hpp"
#include "strategy/runner.hpp"
#include "strategy/baseline_market_making.hpp"


// char* -> standard way to represent strings in cpp
// char** -> standard way to represent an array of string in cpp
// argv -> argument value; argc -> argument count

static int get_flag(int argc, char** argv, const std::string& name, int defval) {
    for (int i = 1; i  < argc; ++i) {
        if (name == argv[i]) return std::stoi(argv[i+1]); // stoi -> string to int; 
        // get the integer value correspodning to number of chars processed
    }
    return defval;
}


int main(int argc, char**argv) {
    int steps = get_flag(argc, argv, "--steps", 100);
    int depth = get_flag(argc, argv, "--depth", 10); 
    int seed = get_flag(argc, argv, "--seed", 42);

    std::cout << "Running demo... \n";
    std::cout << "Config: step=" << steps << " depth=" << depth << " seed=" << seed << "\n";
    std::cout << "CWD=" << std::filesystem::current_path().string() << "\n";


    OrderBook ob;
    PnL pnl;
    OrderIdGen ids;
    std::uint64_t step = 0;

    aStrategy::QuoteParams qp{2, 1, 5, 101};
    aStrategy::BaseLineMarketMaking mm(qp);

    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> side01(0,1); // choose side
    std::uniform_int_distribution<int> qty_dist(1, 3); // specify taker quantity

    ob.submit_limit(Order{ids.next(), Side::Buy, qp.fallback_mid - 1, 10, ++step});
    ob.submit_limit(Order{ids.next(), Side::Sell, qp.fallback_mid + 1, 10, step});

    for (int t = 1; t <= steps; ++t) {
        EngineAPI api{
            ob,
            pnl, 
            ids,
            ++step
        };

        auto strategy_goal = mm.step(api);

        auto executed_strategy = aStrat::apply_modifications(strategy_goal, api, true, true);

        Side taker_side = side01(rng) ? Side::Buy : Side::Sell;
        Qty qty = qty_dist(rng);
        MarketOrder mark_to_market{ids.next(), taker_side, qty, step};
        auto fills = ob.add_market(mark_to_market);
        if (!fills.empty()) {
            for (const auto& fill : fills) {
                pnl.on_trade(taker_side, fill.px, fill.qty);
            }
            csvlog::append_fills(fills);
        }

        auto best_bid = ob.best_bid();
        auto best_ask = ob.best_ask();

        Px mid;
        
        if (best_bid.has_value() && best_ask.has_value()) {
            mid = (*best_bid + *best_ask)/2;
        } else if (best_bid.has_value()) {
            mid = *best_bid;

        } else if (best_ask.has_value()) {
            mid = *best_ask;
        } else {
            mid = qp.fallback_mid;
        }
        csvlog::append_pnl(pnl.cash, pnl.inventory, pnl.mark_to_market(mid), step);
        csvlog::append_snapshot(ob.top_bids(depth), ob.top_asks(depth), step);


        
        std::cout << "[t=" << t << "] placed" << strategy_goal.size()
                    << " cancels_ok=" << executed_strategy.failed_cancels
                    << " cash=" << pnl.cash
                    << " inventory=" << pnl.inventory
                    << " mark to market=" << pnl.mark_to_market(mid)
                    << " best bid=" << (best_bid ? std::to_string(*best_bid) : "none")
                    << " best_ask=" << (best_ask ? std::to_string(*best_ask) : "none")
                    << std::endl;
    
    }
    std::cout << "Done with Demo. CSV files written to ./data/processed"  << std::endl;
    return 0;
}