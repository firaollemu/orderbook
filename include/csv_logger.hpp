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