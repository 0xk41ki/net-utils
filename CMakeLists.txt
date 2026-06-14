cmake_minimum_required(VERSION 3.25)
project(net_utils VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Warning flags (applied privately to library and examples) ─────────────────
add_library(net_utils_warnings INTERFACE)
target_compile_options(net_utils_warnings INTERFACE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
    -Wnull-dereference
    -Wdouble-promotion
    -Wformat=2
    -Wimplicit-fallthrough
    $<$<CXX_COMPILER_ID:GNU>:
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
        -Wuseless-cast
    >
    $<$<CXX_COMPILER_ID:Clang>:
        -Wno-gnu-zero-variadic-macro-arguments
    >
)

# ── Build options (no exceptions) ─────────────────────────────────────────────
add_library(net_utils_options INTERFACE)
target_compile_options(net_utils_options INTERFACE
    -fno-exceptions
)

# ── Dependencies ──────────────────────────────────────────────────────────────
find_package(OpenSSL REQUIRED)

# ── Subdirectories ────────────────────────────────────────────────────────────
add_subdirectory(src)

option(NET_UTILS_BUILD_EXAMPLES "Build example binaries" ON)
if(NET_UTILS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
