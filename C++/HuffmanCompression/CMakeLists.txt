# HuffmanCompression/CMakeLists.txt
add_library(HUFFMAN INTERFACE)
target_include_directories(HUFFMAN INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(HUFFMAN INTERFACE
    project_warnings
)

# Tests
if(BUILD_TESTS)
    add_executable(HUFFMAN_tests tests/huffman_test.cpp)
    target_link_libraries(HUFFMAN_tests PRIVATE
        Catch2::Catch2WithMain
        HUFFMAN
    )
    catch_discover_tests(HUFFMAN_tests)
endif()
