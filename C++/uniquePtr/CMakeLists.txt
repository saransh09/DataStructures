# uniquePtr/CMakeLists.txt
add_library(UNIQUEPTR INTERFACE)
target_include_directories(UNIQUEPTR INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(UNIQUEPTR INTERFACE
    UNIQUEPTR
    project_warnings
)

# Tests
if(BUILD_TESTS)
    add_executable(UNIQUEPTR_tests tests/unique_ptr_test.cpp)
    target_link_libraries(UNIQUEPTR_tests PRIVATE
        Catch2::Catch2WithMain
        UNIQUEPTR
    )
    catch_discover_tests(UNIQUEPTR_tests)
endif()
