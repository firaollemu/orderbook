# C++ limit order book simulator with ML-Driven Market Making
# Goal: understand quantitative development and market making.

# To build:
```bash
cmake -S . -B build -DCMAKE_CXX_COMPILER=clang++
cmake --build build
./build/orderbook

# To run test:

ctest --test-dir build -V  
```

Resources used:

https://www.youtube.com/watch?v=sX2nF1fW7kI
