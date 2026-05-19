# GSL Performance Benchmarks

This directory contains performance benchmarks comparing `gsl::span` with `std::span` across various workloads and compiler implementations.

## Overview

The benchmarks test critical span operations to ensure `gsl::span` maintains **performance parity** with `std::span`, particularly when [Safe Buffers](https://clang.llvm.org/docs/SafeBuffers.html) are enabled.

### Tested Operations

- **`is_sorted`**: Custom iterator-based check, `std::is_sorted`, and `std::ranges::is_sorted`
- **`min_element`**: Algorithm-based and range-for iteration approaches

Each test compares both `std::span` and `gsl::span` implementations.

## Building Locally

### Prerequisites

- CMake 3.20+
- C++20 compiler or newer (GCC, Clang, or MSVC)
- Google Benchmark (fetched automatically via CMake)
- Google Test (fetched automatically via CMake)

### Build Steps

```bash
# From the repository root
cd benchmark
mkdir -p build
cd build

# Configure with Release build type (important for accurate benchmarks)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release
```

## Running Benchmarks

```bash
# Run all benchmarks with default output
./span_bench

# Run with JSON output (useful for analysis)
./span_bench --benchmark_format=json > results.json

# Run specific benchmark (regex matching)
./span_bench --benchmark_filter="IsSorted"

# Show detailed statistics
./span_bench --benchmark_report_aggregates_only=true
```

For more options, see [Google Benchmark documentation](https://github.com/google/benchmark#usage).

## CI Integration

Benchmarks automatically run on every pull request across:
- **Compilers**: GCC 14 (latest), Clang 18 (latest), MSVC (latest)
- **Standards**: C++20
- **Platforms**: Linux (GCC/Clang), Windows (MSVC on windows-2025)

Results are uploaded as GitHub Actions artifacts for easy download and analysis.

Results are uploaded as GitHub Actions artifacts and can be analyzed for performance regressions.

## Performance Analysis

When comparing results:
1. Always compare the same compiler, C++ standard, and platform
2. Release builds should be used for accurate measurements
3. Significant differences (>5%) between `std::span` and `gsl::span` warrant investigation
4. Account for noise in CI environments when analyzing small variations

## Contributing

When adding new benchmarks:
1. Follow the existing pattern in `span_bench.cpp`
2. Compare both `std::span` and `gsl::span` implementations
3. Use `benchmark::DoNotOptimize` to prevent compiler optimizations from skewing results
4. Document what the benchmark tests and why it matters
