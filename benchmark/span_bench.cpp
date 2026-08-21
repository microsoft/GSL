#include "gsl/span"
#include <algorithm>
#include <benchmark/benchmark.h>
#include <limits>
#include <ranges>
#include <span>
#include <vector>

static std::vector<int> make_vector()
{
    std::vector<int> v;
    constexpr size_t vec_size = 1000;
    v.reserve(vec_size);
    for (int i = 0; i < vec_size; ++i)
        v.push_back(i);

    return v;
}

template<class ForwardIt>
bool custom_is_sorted(ForwardIt first, ForwardIt last)
{
    if (first != last)
    {
        ForwardIt next = first;
        while (++next != last)
        {
            if (*next < *first)
                return false;
            first = next;
        }
    }
    return true;
}

static void BM_IsSortedCustomStdSpan(benchmark::State& state)
{
    auto vec = make_vector();

    for (auto _ : state)
    {
        std::span<const int> sp = vec;
        benchmark::DoNotOptimize(custom_is_sorted(sp.begin(), sp.end()));
    }
}

static void BM_IsSortedCustomGslSpan(benchmark::State& state)
{
    auto vec = make_vector();

    for (auto _ : state)
    {
        gsl::span<const int> sp = vec;
        benchmark::DoNotOptimize(custom_is_sorted(sp.begin(), sp.end()));
    }
}

static void BM_IsSortedStdSpan(benchmark::State& state)
{
    auto vec = make_vector();

    for (auto _ : state)
    {
        std::span<const int> sp = vec;
        benchmark::DoNotOptimize(std::is_sorted(sp.begin(), sp.end()));
    }
}

static void BM_IsSortedGslSpan(benchmark::State& state)
{
    auto vec = make_vector();

    for (auto _ : state)
    {
        gsl::span<const int> sp = vec;
        benchmark::DoNotOptimize(std::is_sorted(sp.begin(), sp.end()));
    }
}

static void BM_IsSortedRangesStdSpan(benchmark::State& state)
{
    auto vec = make_vector();

    for (auto _ : state)
    {
        std::span<const int> sp = vec;
        benchmark::DoNotOptimize(std::ranges::is_sorted(sp));
    }
}


static void BM_IsSortedRangesGslSpan(benchmark::State& state)
{
    auto vec = make_vector();

    for (auto _ : state)
    {
        gsl::span<const int> sp = vec;
        benchmark::DoNotOptimize(std::ranges::is_sorted(sp));
    }
}

BENCHMARK(BM_IsSortedStdSpan);
BENCHMARK(BM_IsSortedGslSpan);
BENCHMARK(BM_IsSortedRangesStdSpan);
BENCHMARK(BM_IsSortedRangesGslSpan);
BENCHMARK(BM_IsSortedCustomStdSpan);
BENCHMARK(BM_IsSortedCustomGslSpan);

template <typename TSpan>
static int CustomMinElement(TSpan span)
{
    auto min = std::numeric_limits<int>::max();
    for (int e : span)
    {
        if (e < min)
            min = e;
    }
    return min;
}

static void BM_MinElementAlgorithmStdSpan(benchmark::State& state)
{
    auto vec = make_vector();

    for (auto _ : state)
    {
        std::span<const int> sp = vec;
        benchmark::DoNotOptimize(std::min_element(sp.begin(), sp.end()));
    }
}

static void BM_MinElementAlgorithmGslSpan(benchmark::State& state)
{
    auto vec = make_vector();

    for (auto _ : state)
    {
        gsl::span<const int> sp = vec;
        benchmark::DoNotOptimize(std::min_element(sp.begin(), sp.end()));
    }
}

static void BM_MinElementRangeForStdSpan(benchmark::State& state)
{
    auto vec = make_vector();

    for (auto _ : state)
    {
        std::span<const int> sp = vec;
        benchmark::DoNotOptimize(CustomMinElement(sp));
    }
}


static void BM_MinElementRangeForGslSpan(benchmark::State& state)
{
    auto vec = make_vector();

    for (auto _ : state)
    {
        gsl::span<const int> sp = vec;
        benchmark::DoNotOptimize(CustomMinElement(sp));
    }
}


BENCHMARK(BM_MinElementAlgorithmStdSpan);
BENCHMARK(BM_MinElementAlgorithmGslSpan);
BENCHMARK(BM_MinElementRangeForStdSpan);
BENCHMARK(BM_MinElementRangeForGslSpan);

BENCHMARK_MAIN();

