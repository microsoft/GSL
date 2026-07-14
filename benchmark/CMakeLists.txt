cmake_minimum_required(VERSION 3.14...3.16)

project(GSLBenchmarks LANGUAGES CXX)

# ── C++ standard ───────────────────────────────────────────────────────────────
# Minimum is 20 because std::span (required for this benchmark) is C++20 only.
# If a consumer passes GSL_CXX_STANDARD < 20 we clamp and warn.
set(GSL_CXX_STANDARD "20" CACHE STRING "Use c++ standard")

if(GSL_CXX_STANDARD LESS 20)
    message(WARNING
        "GSL_CXX_STANDARD=${GSL_CXX_STANDARD} is too old for the span benchmark "
        "(std::span requires C++20). Overriding to 20.")
    set(GSL_CXX_STANDARD "20" CACHE STRING "Use c++ standard" FORCE)
endif()

set(CMAKE_CXX_STANDARD ${GSL_CXX_STANDARD})
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Makes Visual Studio organise targets into folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(FetchContent)

# Suppress benchmark's own test suite
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        v1.9.5
    GIT_SHALLOW    ON
)
FetchContent_MakeAvailable(googlebenchmark)

add_executable(span_bench span_bench.cpp)

target_link_libraries(span_bench PRIVATE
    benchmark::benchmark
    Microsoft.GSL::GSL
)

set_target_properties(span_bench PROPERTIES FOLDER "benchmarks")

# ── Optimisation flags ─────────────────────────────────────────────────────────
target_compile_options(span_bench PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
        -O3
        -march=native
    >
    $<$<CXX_COMPILER_ID:MSVC>:
        /O2
        /GL
    >
)