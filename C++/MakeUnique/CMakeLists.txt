# MakeUnique/CMakeLists.txt
add_library(MAKEUNIQUE INTERFACE)
target_include_directories(MAKEUNIQUE INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(MAKEUNIQUE INTERFACE
    MAKEUNIQUE
    project_warnings
)

# Tests
if(BUILD_TESTS)
    add_executable(MAKEUNIQUE_tests tests/make_unique_test.cpp)
    target_link_libraries(MAKEUNIQUE_tests PRIVATE
        Catch2::Catch2WithMain
        MAKEUNIQUE
    )
    catch_discover_tests(MAKEUNIQUE_tests)
endif()
