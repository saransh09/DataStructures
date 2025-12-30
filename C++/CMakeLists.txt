cmake_minimum_required(VERSION 3.20)
project(DataStructures LANGUAGES CXX)

# -----------------------------------------------------------------------------
# C++ Standard
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------------------------------------------
# Build Settings
# -----------------------------------------------------------------------------
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# -----------------------------------------------------------------------------
# Compiler Warnings (important for concurrent code!)
# -----------------------------------------------------------------------------
add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE
    $<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>:
        -Wall -Wextra -Wpedantic -Werror
        -Wconversion -Wsign-conversion
        -Wnon-virtual-dtor -Wold-style-cast
        -Wcast-align -Wunused -Woverloaded-virtual
        -Wshadow -Wformat=2
    >
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4 /WX
    >
)

# -----------------------------------------------------------------------------
# Sanitizers (crucial for concurrent data structures)
# Usage: cmake -DSANITIZER=thread ..
# -----------------------------------------------------------------------------
set(SANITIZER "" CACHE STRING "Enable sanitizer (thread, address, undefined)")
if(SANITIZER)
    message(STATUS "Enabling ${SANITIZER} sanitizer")
    add_compile_options(-fsanitize=${SANITIZER} -fno-omit-frame-pointer)
    add_link_options(-fsanitize=${SANITIZER})
endif()

# -----------------------------------------------------------------------------
# Platform-specific linker flags (for Homebrew LLVM on macOS)
# -----------------------------------------------------------------------------
if(APPLE AND EXISTS /opt/homebrew/opt/llvm)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/opt/homebrew/opt/llvm/lib/c++ -Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++")
endif()

# -----------------------------------------------------------------------------
# Options
# -----------------------------------------------------------------------------
option(BUILD_TESTS "Build unit tests" ON)

# -----------------------------------------------------------------------------
# External Dependencies
# -----------------------------------------------------------------------------
add_subdirectory(external/Catch2)
add_subdirectory(external/quill)

# -----------------------------------------------------------------------------
# Tests Setup (must come before data structure subdirectories)
# -----------------------------------------------------------------------------
if(BUILD_TESTS)
    include(Catch)
    enable_testing()
endif()

# -----------------------------------------------------------------------------
# Data Structure Libraries
# -----------------------------------------------------------------------------
add_subdirectory(ThreadSafeQueue)
