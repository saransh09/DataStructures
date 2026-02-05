# OrderBook/CMakeLists.txt
add_library(LATCH INTERFACE)
target_include_directories(LATCH INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(LATCH INTERFACE
    LATCH
    project_warnings
)

# Tests
if(BUILD_TESTS)
    add_executable(LATCH_tests tests/latch_test.cpp)
    target_link_libraries(LATCH_tests PRIVATE
        Catch2::Catch2WithMain
        LATCH
    )
    catch_discover_tests(LATCH_tests)
endif()
