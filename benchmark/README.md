# `gsl::span` vs `std::span` Benchmark

Performance parity tracking between `gsl::span` and `std::span` across all supported compilers, platforms, and C++ standards — as part of [microsoft/GSL#1167](https://github.com/microsoft/GSL/issues/1167) and [microsoft/GSL#1165](https://github.com/microsoft/GSL/issues/1165).

---

## Overview

`gsl::span` should be a zero-overhead abstraction over `std::span`. This benchmark suite verifies that claim continuously — on every PR — so performance regressions are caught before they land in main.

The comparison strategy is **in-run ratio** (`gsl_ns / std_ns`): both spans are measured in the same process on the same machine at the same moment, so runner noise cancels out. A ratio close to `1.0` means parity. If `gsl::span` is more than **15% slower** than `std::span` on any benchmark, CI flags it and posts a detailed table in the PR comment.

---

## Benchmarks

All benchmarks run on a sorted vector of 1000 integers.

| Benchmark | What it tests |
|---|---|
| `IsSorted` | `std::is_sorted` via span iterators |
| `IsSortedRanges` | `std::ranges::is_sorted` via the span range interface |
| `IsSortedCustom` | Custom hand-rolled `is_sorted` loop via span iterators |
| `MinElementAlgorithm` | `std::min_element` via span iterators |
| `MinElementRangeFor` | Range-for loop with a custom min accumulator |

Each benchmark has a `StdSpan` and `GslSpan` variant. The Python script pairs them by name and computes the ratio.

---

## CI Matrix

The benchmark runs on every pull request across 13 configurations:

| OS | Compiler | C++ Standard |
|---|---|---|
| ubuntu-latest | GCC 13 | C++20 |
| ubuntu-latest | GCC 14 | C++20, C++23 |
| ubuntu-latest | Clang 17 | C++20 |
| ubuntu-latest | Clang 18 | C++20, C++23 |
| windows-latest | MSVC 2022 | C++20, C++23 |
| windows-latest | clang-cl (VS 2022 bundled) | C++20, C++23 |
| macos-14 (Apple Silicon) | Apple Clang (Xcode latest) | C++20, C++23 |

Results from all 13 jobs are collected and posted as a **single PR comment**, updated on every push.

---

## Repository Layout

```
benchmark/
├── CMakeLists.txt        # self-contained build — fetches benchmark + googletest
├── span_bench.cpp        # the benchmark source
├── check_regression.py   # parses JSON results, writes the PR comment markdown
└── README.md             # this file

.github/workflows/
└── span_benchmark.yml    # CI workflow
```

The benchmark folder is self-contained. `CMakeLists.txt` fetches `google/benchmark` (v1.9.0) and `google/googletest` via `FetchContent`. GSL itself is sourced from the **local repo checkout** — not a pinned tag — so the benchmark always tests the code in the PR, not a release.

---

## Running Locally

**Prerequisites:** CMake ≥ 3.20, a C++20-capable compiler, internet access for `FetchContent`.

```bash
# From the benchmark/ directory:
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target span_bench

# Run with JSON output (matches what CI does):
./build/span_bench \
  --benchmark_format=json \
  --benchmark_repetitions=10 \
  --benchmark_report_aggregates_only=true \
  --benchmark_out=results.json

# Generate the regression report locally:
python3 check_regression.py --threshold 0.15 results.json
```

To test a specific compiler or standard:

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER=clang++-18 \
  -DCMAKE_CXX_STANDARD=23
```

---

## Regression Detection

The `check_regression.py` script:

1. Reads one or more Google Benchmark JSON files (one per CI matrix config)
2. Pairs `*StdSpan` benchmarks with their `*GslSpan` counterparts by name
3. Computes `ratio = gsl_mean / std_mean` using the 10-repetition mean
4. Flags any ratio above `1 + threshold` (default **15%**) as a regression
5. Writes a Markdown table per config, collected into a single PR comment
6. Exits with code `1` if any regression is found — failing the CI check

```
python3 check_regression.py [--threshold 0.15] [--output report.md] results_*.json
```

### Example PR comment output

```
## 📊 gsl::span vs std::span benchmark results

### `GCC-14-cpp20`
| Benchmark            | std mean | std σ  | gsl mean | gsl σ  | ratio | status   |
|----------------------|----------|--------|----------|--------|-------|----------|
| IsSorted             | 124.1 ns | ±1.2%  | 125.3 ns | ±1.4%  | 1.01× | ✅ 1.01× |
| IsSortedRanges       |  88.4 ns | ±0.9%  |  89.1 ns | ±1.1%  | 1.01× | ✅ 1.01× |
| IsSortedCustom       | 112.6 ns | ±1.5%  | 113.2 ns | ±1.3%  | 1.01× | ✅ 1.01× |
| MinElementAlgorithm  |  95.2 ns | ±1.0%  |  96.0 ns | ±1.2%  | 1.01× | ✅ 1.01× |
| MinElementRangeFor   |  98.7 ns | ±1.1%  |  99.4 ns | ±0.8%  | 1.01× | ✅ 1.01× |
```

Status icons:
- ✅ — within the threshold (parity)
- 🔴 — `gsl::span` is more than 15% slower (regression, CI fails)
- 🟢 — `gsl::span` is more than 15% faster (improvement, noted but not a failure)

---

## Noise Considerations

GitHub-hosted runners are shared VMs with a typical noise floor of **±10–15%** on absolute timing. The ratio strategy mitigates this because both span variants experience the same CPU conditions simultaneously. The 15% threshold is chosen to sit just above the noise floor — tight enough to catch real regressions, loose enough to avoid false positives on every PR.

To further reduce variance, each benchmark runs **10 repetitions** and the script uses the **mean** (not a single sample) for the ratio calculation. The stddev column in the comment table lets reviewers eyeball how stable each measurement was.

---

## Background

This benchmark was created in response to [microsoft/GSL#1167](https://github.com/microsoft/GSL/issues/1167), which highlighted the need to maintain performance parity with `std::span`, especially when [Safe Buffers / `-fbounds-safety`](https://clang.llvm.org/docs/SafeBuffers.html) are enabled. The initial benchmark scaffolding was provided by @galenelias.