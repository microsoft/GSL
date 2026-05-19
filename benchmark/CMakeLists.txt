cmake_minimum_required(VERSION 3.20)

project(myproject C CXX)
set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")   # Default build type

set(BENCHMARK_ENABLE_TESTING off)    # to suppress benchmark internal tests
set(CMAKE_CXX_STANDARD 20 CACHE STRING "C++ standard for benchmarks")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1)      # or "main" for latest
FetchContent_Declare(googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.9.0)              # or "main" for latest
FetchContent_Declare(GSL
    GIT_REPOSITORY https://github.com/microsoft/GSL.git
    GIT_TAG v4.1.0
    GIT_SHALLOW ON)


FetchContent_MakeAvailable(googletest googlebenchmark GSL)


add_executable(span_bench span_bench.cpp)
target_link_libraries(span_bench PRIVATE benchmark::benchmark)
target_link_libraries(span_bench PRIVATE Microsoft.GSL::GSL)

