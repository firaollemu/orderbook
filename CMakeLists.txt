cmake_minimum_required(VERSION 3.20)
project(OrderBook CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
enable_testing()

add_library(orderbook_core
    src/order_book.cpp
    src/baseline_market_making.cpp)

target_include_directories(orderbook_core PUBLIC include)
add_executable(OrderBook
    src/demo.cpp)
target_link_libraries(OrderBook PRIVATE orderbook_core)


include(FetchContent)
FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.3
)

FetchContent_MakeAvailable(catch2)
add_subdirectory(tests)