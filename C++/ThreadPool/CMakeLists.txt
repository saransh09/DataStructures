# ThreadPool/CMakeLists.txt
add_library(ThreadPool INTERFACE)
target_include_directories(ThreadPool INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ThreadPool INTERFACE
    ThreadSafeQueue
    project_warnings
)

# Tests
if(BUILD_TESTS)
    add_executable(ThreadPool_tests tests/thread_pool_test.cpp)
    target_link_libraries(ThreadPool_tests PRIVATE
        Catch2::Catch2WithMain
        ThreadPool
    )
    catch_discover_tests(ThreadPool_tests)
endif()
