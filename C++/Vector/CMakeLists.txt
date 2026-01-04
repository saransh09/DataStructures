# Vector/CMakeLists.txt
add_library(VECTOR INTERFACE)
target_include_directories(VECTOR INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(VECTOR INTERFACE
    VECTOR
    project_warnings
)

# Tests
if(BUILD_TESTS)
    add_executable(VECTOR_tests tests/vector_test.cpp)
    target_link_libraries(VECTOR_tests PRIVATE
        Catch2::Catch2WithMain
        VECTOR
    )
    catch_discover_tests(VECTOR_tests)
endif()
