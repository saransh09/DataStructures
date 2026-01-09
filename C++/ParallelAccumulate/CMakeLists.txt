# ParallelAccumulate/CMakeLists.txt
add_library(PARALLEL_ACCUMULATE INTERFACE)
target_include_directories(PARALLEL_ACCUMULATE INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(PARALLEL_ACCUMULATE INTERFACE
    project_warnings
    TBB::tbb
)

# Tests
if(BUILD_TESTS)
    add_executable(PARALLEL_ACCUMULATE_tests tests/parallel_accumulate_test.cpp)
    target_link_libraries(PARALLEL_ACCUMULATE_tests PRIVATE
        Catch2::Catch2WithMain
        PARALLEL_ACCUMULATE
    )
    catch_discover_tests(PARALLEL_ACCUMULATE_tests)
endif()
