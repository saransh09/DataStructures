# OrderBook/CMakeLists.txt
add_library(DINERPHILOSOPHER INTERFACE)
target_include_directories(DINERPHILOSOPHER INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(DINERPHILOSOPHER INTERFACE
    project_warnings
)

# Tests
if(BUILD_TESTS)
    add_executable(DINERPHILOSOPHER_tests tests/diner_philosopher_test.cpp)
    target_link_libraries(DINERPHILOSOPHER_tests PRIVATE
        Catch2::Catch2WithMain
        DINERPHILOSOPHER
    )
    catch_discover_tests(DINERPHILOSOPHER_tests)
endif()
