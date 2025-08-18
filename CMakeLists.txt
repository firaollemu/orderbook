cmake_minimum_required(VERSION 3.20)
project(OrderBook CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(OrderBook src/main.cpp src/order_book.cpp)
target_include_directories(OrderBook PRIVATE include)
