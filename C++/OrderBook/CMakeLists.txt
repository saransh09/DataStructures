# OrderBook/CMakeLists.txt
add_library(ORDERBOOK INTERFACE)
target_include_directories(ORDERBOOK INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ORDERBOOK INTERFACE
    ORDERBOOK
    project_warnings
)

# Tests
if(BUILD_TESTS)
    add_executable(ORDERBOOK_tests tests/order_book_test.cpp)
    target_link_libraries(ORDERBOOK_tests PRIVATE
        Catch2::Catch2WithMain
        ORDERBOOK
    )
    catch_discover_tests(ORDERBOOK_tests)
endif()
