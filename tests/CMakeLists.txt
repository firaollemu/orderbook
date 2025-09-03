add_executable(orderbook_tests
    test_basic_add.cpp
    test_cross_fill.cpp
    test_cancel.cpp
    test_partial_fill.cpp
    test_full_fill.cpp
    test_cancel_edge_case.cpp
    test_snapshot.cpp
    test_order_id_gen.cpp
    test_logging.cpp
    test_pnl.cpp
    test_market.cpp
    test_strategy_baseline.cpp
    test_strategy_cancel_and_replace.cpp
    test_avellaneda_stoikov.cpp
)

target_link_libraries(orderbook_tests
    PRIVATE
    orderbook_core
    Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(orderbook_tests)
