# C++ limit order book simulator with ML-Driven Market Making (used LLM for trivial and redundant tasks)

# Goal: understand quantitative development and market making.

# To build:
```bash
cmake -S . -B build -DCMAKE_CXX_COMPILER=clang++
cmake --build build
./build/orderbook

# To run test:
```bash
ctest --test-dir build -V  

