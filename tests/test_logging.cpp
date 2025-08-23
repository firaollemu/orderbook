#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <string>
#include <string>
#include <vector>
#include <iostream>


#include "order.hpp"
#include "test_utils.hpp"
#include "csv_logger.hpp"


using testutil::mk_order;

using std::filesystem::path;
using std::filesystem::temp_directory_path;
using std::filesystem::exists;
using std::filesystem::remove;


static std::vector<std::string> read_lines(const path& p) {
    std::vector<std::string> lines;
    std::ifstream in(p);
    std::string s;
    while (std::getline(in, s)) lines.push_back(s);
    return lines;
}

TEST_CASE("csv logger writes header + rows") {
    path p = temp_directory_path() / "test_fills.csv";
  
    if (exists(p)) remove(p);

    // Print the path to the console so we can find the file
    std::cout << "CSV file created at: " << p << std::endl;
   
    Order taker = testutil::mk_order(100, Side::Buy, 101, 50, 1);
    Order maker1 = testutil::mk_order(7, Side::Sell, 101, 50, 1);
    Order maker2 = testutil::mk_order(8, Side::Sell, 102, 20, 1);


    std::vector<Fill> fills{
        Fill{taker.id, maker1.id, maker1.px, 50, taker.ts},
        Fill{taker.id, maker2.id, maker2.px, 20, taker.ts},
    };

    csvlog::append_fills(fills, p.string());

    auto lines = read_lines(p);
    REQUIRE(lines.size() == 1 + fills.size());
    REQUIRE(lines[0] == "taker_id,maker_id,px,qty,ts");
    REQUIRE(lines[1] == "100,7,101,50,1");
    REQUIRE(lines[2] == "100,8,102,20,1");
}