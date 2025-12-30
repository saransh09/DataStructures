# ThreadSafeQueue - Header-only library
add_library(ThreadSafeQueue INTERFACE)
target_include_directories(ThreadSafeQueue INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(ThreadSafeQueue INTERFACE project_warnings)

# Tests
if(BUILD_TESTS)
    add_executable(ThreadSafeQueue_tests tests/thread_safe_queue_test.cpp)
    target_link_libraries(ThreadSafeQueue_tests PRIVATE
        Catch2::Catch2WithMain
        ThreadSafeQueue
    )
    catch_discover_tests(ThreadSafeQueue_tests)
endif()
