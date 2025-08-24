#pragma once
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include "order.hpp"

namespace csvlog {
    inline void ensure_parent_dirs(const std::string& file_path) {
        std::filesystem::path p{file_path};
        if (p.has_parent_path()) {
            std::filesystem::create_directories(p.parent_path());
        }
    }

inline void append_fills(const std::vector<Fill>& fills, const std::string& csv_path="data/processed/fills.csv") {
    using std::filesystem::exists;
    using std::filesystem::file_size;
    
    csvlog::ensure_parent_dirs(csv_path);

    const bool need_header = !exists(csv_path) || file_size(csv_path) == 0;
    std::ofstream out(csv_path, std::ios::app); // open file in append mode
    if (!out) return; // cannot open file

    if (need_header) {
        out << "taker_id,maker_id,px,qty,ts\n";
    }

    // write to opened file
    for (const auto& f : fills) {
        out << f.taker << "," << f.maker << ","
            << f.px << "," << f.qty << ","
            << f.ts  << "\n";
    }

    out.flush(); // write contents of the stream's internal buffer to the associated file
}

inline void append_pnl(double cash, long long inventory, double mtm, std::uint64_t ts,
            const std::string& path = "data/processed/pnl.csv") {
                csvlog::ensure_parent_dirs(path);

                const bool need_header = !std::filesystem::exists(path) ||
                        std::filesystem::file_size(path) == 0;

                std::ofstream out(path, std::ios::app);

                if (!out) return;

                if (need_header) {
                    out << "ts,cash,inventory,mtm\n";
                }

                out << ts << ","
                    << cash << ","
                    << inventory << ","
                    << mtm << "\n";
                
                out.flush();
            }

inline void append_snapshot(const std::vector<std::pair<Px, Qty>>& bids,
                            const std::vector<std::pair<Px, Qty>>& asks,
                            std::uint64_t ts,
                            const std::string& path = "data/processed/ob_depth.csv") {
                                csvlog::ensure_parent_dirs(path);
                                const bool need_header = !std::filesystem::exists(path) ||
                                        std::filesystem::file_size(path) == 0;
                                std::ofstream out(path, std::ios::app);
                                
                                if (!out) return;
                                if (need_header) out << "ts,side,px,qty\n";
                                for (auto [px, q] : bids) out << ts << ",bid," << px << "," << q << "\n";
                                for (auto [px, q] : asks) out << ts << ",ask," << px << "," << q << "\n";
                                out.flush();
                            }
}