# benchmark/CMakeLists.txt
#
# Self-contained build for the gsl::span vs std::span benchmark.
# All dependencies are fetched automatically via FetchContent:
#   - google/benchmark  v1.9.0
#   - google/googletest release-1.12.1  (benchmark's internal dep)
#   - microsoft/GSL     v4.1.0
#
# Usage (from this directory):
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
#   cmake --build build --target span_bench
#   ./build/span_bench --benchmark_format=json --benchmark_repetitions=10 \
#                      --benchmark_report_aggregates_only=true \
#                      --benchmark_out=results.json
#
# To override the compiler or standard (as CI does):
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
#         -DCMAKE_CXX_COMPILER=clang++-18 \
#         -DCMAKE_CXX_STANDARD=23

cmake_minimum_required(VERSION 3.20)

project(gsl_span_benchmark CXX)

# ── C++ standard ──────────────────────────────────────────────────────────────
# Default C++20 (minimum for std::span). CI overrides via -DCMAKE_CXX_STANDARD.
set(CMAKE_CXX_STANDARD 20 CACHE STRING "C++ standard")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS NO)

set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")

# ── FetchContent dependencies ──────────────────────────────────────────────────
include(FetchContent)

# Suppress benchmark's own test suite — we don't need it.
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        release-1.12.1
    GIT_SHALLOW    ON
)

FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        v1.9.0
    GIT_SHALLOW    ON
)

FetchContent_MakeAvailable(googletest googlebenchmark)

# Use the GSL from the local checkout — NOT a pinned release tag.
# The benchmark/ folder sits one level below the repo root, so
# CMAKE_CURRENT_SOURCE_DIR/.. resolves to the repo root which contains
# the real include/gsl/span header being modified by the PR.
# This is what makes the CI actually test the PR's changes.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. gsl_root)

# ── Benchmark executable ───────────────────────────────────────────────────────
add_executable(span_bench span_bench.cpp)

# Link benchmark::benchmark only — NOT benchmark::benchmark_main.
# span_bench.cpp uses the BENCHMARK_MAIN() macro which expands to its own
# main(), so linking benchmark_main would cause a duplicate-symbol linker error.
target_link_libraries(span_bench PRIVATE
    benchmark::benchmark   # provides the benchmark framework + runner
    Microsoft.GSL::GSL     # provides gsl::span and friends
)

# ── Optimisation flags ─────────────────────────────────────────────────────────
# -O3 and -march=native let the compiler apply the same optimisations to both
# gsl::span and std::span, keeping the comparison fair.
# -fno-omit-frame-pointer keeps perf/profiling usable if you need it locally.
target_compile_options(span_bench PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
        -O3
        -march=native
        -fno-omit-frame-pointer
    >
    $<$<CXX_COMPILER_ID:MSVC>:
        /O2
        /GL        # whole-program optimisation
    >
)