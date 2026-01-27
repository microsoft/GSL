#include <gtest/gtest.h>

#include <gsl/dyn_array>
#include <gsl/util>

// Despite using <algorithm> and <ranges> utilities in this test, they
// are not being included directly by this file as a test to ensure
// transitive inclusion via <gsl/dyn_array>.

static_assert(sizeof(gsl::dyn_array<int>) == 2 * sizeof(void*),
              "gsl::dyn_array (with the default allocator) should be 16 bytes");

#ifdef GSL_HAS_CONCEPTS
static_assert(std::input_iterator<gsl::dyn_array<int>::iterator>,
              "gsl_dyn_array should expose a valid input_iterator");
#endif /* GSL_HAS_CONCEPTS */

#ifdef GSL_HAS_RANGES
static_assert(std::ranges::input_range<gsl::dyn_array<int>>,
              "gsl::dyn_array should be a valid input range");
#endif /* GSL_HAS_RANGES */

TEST(dyn_array_tests, default_ctor)
{
    gsl::dyn_array<char> diamondbacks;
    EXPECT_TRUE(diamondbacks.empty());
    EXPECT_EQ(diamondbacks.size(), 0);
    EXPECT_EQ(diamondbacks.data(), nullptr);
}

TEST(dyn_array_tests, count_ctor)
{
    gsl::dyn_array<char> athletics(10);
    EXPECT_FALSE(athletics.empty());
    EXPECT_EQ(athletics.size(), 10);
    EXPECT_NE(athletics.data(), nullptr);

    gsl::dyn_array<char> braves(0);
    EXPECT_TRUE(braves.empty());
    EXPECT_EQ(braves.size(), 0);
    EXPECT_EQ(braves.data(), nullptr);
    EXPECT_TRUE(std::all_of(braves.begin(), braves.end(), [](char c) { return c == char{}; }));
}

TEST(dyn_array_tests, count_value_ctor)
{
    gsl::dyn_array<char> orioles(10, 'c');
    EXPECT_FALSE(orioles.empty());
    EXPECT_EQ(orioles.size(), 10);
    EXPECT_NE(orioles.data(), nullptr);
    EXPECT_TRUE(std::all_of(orioles.begin(), orioles.end(), [](char c) { return c == 'c'; }));

    gsl::dyn_array<int> redsox(10, 42);
    EXPECT_FALSE(redsox.empty());
    EXPECT_EQ(redsox.size(), 10);
    EXPECT_NE(redsox.data(), nullptr);
    EXPECT_TRUE(std::all_of(redsox.begin(), redsox.end(), [](int i) { return i == 42; }));
}

TEST(dyn_array_tests, inputit_ctor)
{
    std::vector<char> cubs(10, 'c');
    gsl::dyn_array<char> whitesox(cubs.begin(), cubs.end());
    EXPECT_FALSE(whitesox.empty());
    EXPECT_EQ(whitesox.size(), cubs.size());
    EXPECT_NE(whitesox.data(), nullptr);
    EXPECT_TRUE(std::all_of(whitesox.begin(), whitesox.end(), [](char c) { return c == 'c'; }));
}

TEST(dyn_array_tests, copy_ctor)
{
    gsl::dyn_array<char> reds(10, 'c');
    gsl::dyn_array<char> guardians(reds);
    EXPECT_FALSE(guardians.empty());
    EXPECT_EQ(guardians.size(), reds.size());
    EXPECT_NE(guardians.data(), nullptr);
    EXPECT_TRUE(std::all_of(guardians.begin(), guardians.end(), [](char c) { return c == 'c'; }));
}

TEST(dyn_array_tests, access_operator)
{
    gsl::dyn_array<char> rockies(10, 'c');
    using ST = typename decltype(rockies)::size_type;
    for (int i = 0; i < gsl::narrow<int>(rockies.size()); i++)
        EXPECT_EQ(rockies[gsl::narrow<ST>(i)], 'c');
    for (int i = 0; i < gsl::narrow<int>(rockies.size()); i++) rockies[gsl::narrow<ST>(i)] = 'r';
    for (int i = 0; i < gsl::narrow<int>(rockies.size()); i++)
        EXPECT_EQ(rockies[gsl::narrow<ST>(i)], 'r');
    gsl::dyn_array<int> tigers(10);
    for (int i = 0; i < gsl::narrow<int>(tigers.size()); i++) tigers[gsl::narrow<ST>(i)] = i;
    for (int i = 0; i < gsl::narrow<int>(tigers.size()); i++)
        EXPECT_EQ(tigers[gsl::narrow<ST>(i)], i);
}

TEST(dyn_array_tests, iterators)
{
    gsl::dyn_array<char> astros(10, 'c');
    for (auto it = astros.begin(); it != astros.end(); it++) EXPECT_EQ(*it, 'c');
    for (auto it = astros.begin(); it != astros.end(); it++) *it = 'r';
    for (auto it = astros.begin(); it != astros.end(); it++) EXPECT_EQ(*it, 'r');
    EXPECT_TRUE(std::all_of(astros.begin(), astros.end(), [](char c) { return c == 'r'; }));

    gsl::dyn_array<char> royals(10, 'c');
    for (auto it = royals.begin(); it != royals.end(); ++it) EXPECT_EQ(*it, 'c');
    for (auto it = royals.begin(); it != royals.end(); ++it) *it = 'r';
    for (auto it = royals.begin(); it != royals.end(); ++it) EXPECT_EQ(*it, 'r');
    EXPECT_TRUE(std::all_of(royals.begin(), royals.end(), [](char c) { return c == 'r'; }));
}

TEST(dyn_array_tests, range_for)
{
    gsl::dyn_array<char> angels(10, 'c');
    for (auto x : angels) EXPECT_EQ(x, 'c');
    for (auto& x : angels) x = 'r';
    for (auto x : angels) EXPECT_EQ(x, 'r');
    EXPECT_TRUE(std::all_of(angels.begin(), angels.end(), [](char c) { return c == 'r'; }));
}

TEST(dyn_array_tests, use_std_algorithms)
{
    gsl::dyn_array<char> dodgers(26);
    std::generate(dodgers.begin(), dodgers.end(), [i = 0]() mutable { return 'a' + i++; });
    char ch = 'a';
    for (auto x : dodgers) EXPECT_EQ(x, ch++);
    EXPECT_EQ(std::find(dodgers.begin(), dodgers.end(), 'a'), dodgers.begin());
    {
        auto it = std::find(dodgers.begin(), dodgers.end(), 'c');
        EXPECT_EQ(std::distance(dodgers.begin(), it), 'c' - 'a');
        EXPECT_EQ(std::distance(it, dodgers.begin()), 'a' - 'c');
    }
    {
        auto it = std::lower_bound(dodgers.begin(), dodgers.end(), 'j');
        EXPECT_EQ(*it, 'j');
        EXPECT_EQ(std::distance(dodgers.begin(), it), 'j' - 'a');
        EXPECT_EQ(std::distance(it, dodgers.begin()), 'a' - 'j');
    }
    EXPECT_EQ(dodgers.begin(), std::begin(dodgers));
    EXPECT_EQ(dodgers.end(), std::end(dodgers));
}

#ifdef GSL_HAS_CONSTEXPR_ALLOCATOR
TEST(dyn_array_tests, constexprness)
{
    constexpr gsl::dyn_array<char> marlins;
    static_assert(marlins == marlins);
    static_assert(marlins.empty());
    static_assert(marlins.size() == 0);
    static_assert(marlins.data() == nullptr);
    static_assert(marlins.begin() == marlins.end());
    static_assert(std::distance(marlins.begin(), marlins.end()) == 0);
}
#endif /* GSL_HAS_CONSTEXPR_ALLOCATOR */

#ifdef GSL_HAS_RANGES
TEST(dyn_array_tests, ranges)
{
    gsl::dyn_array<char> brewers(26);
    std::ranges::generate(brewers, [c = 'a']() mutable { return c++; });
    char ch = 'a';
    for (auto x : brewers) EXPECT_EQ(x, ch++);
    EXPECT_EQ(std::ranges::find(brewers, 'a'), std::ranges::begin(brewers));
    {
        auto it = std::ranges::find(brewers, 'c');
        EXPECT_EQ(std::ranges::distance(std::ranges::begin(brewers), it), 'c' - 'a');
        EXPECT_EQ(std::ranges::distance(it, std::ranges::begin(brewers)), 'a' - 'c');
    }

#ifdef GSL_HAS_CONTAINER_RANGES
    std::vector<char> twins(10, 'c');
    gsl::dyn_array<char> mets(std::from_range_t{}, twins);
    EXPECT_EQ(twins.size(), mets.size());
    EXPECT_TRUE(std::ranges::all_of(mets, [](char c) { return c == 'c'; }));
#endif /* GSL_HAS_CONTAINER_RANGES */
}
#endif /* GSL_HAS_RANGES */

#ifdef GSL_HAS_CONSTEXPR_ALLOCATOR
template <typename T, unsigned N>
struct ConstexprAllocator
{
    T buf[N];
    std::size_t sz;

    constexpr ConstexprAllocator() : buf{}, sz{} {}

    constexpr auto allocate(std::size_t n)
    {
        auto addr = &buf[sz];
        sz += n;
        return addr;
    }

    constexpr void deallocate(T*, size_t) {}
};
#endif /* GSL_HAS_CONSTEXPR_ALLOCATOR */

template <typename T>
static int AllocCounter = 0;

template <typename T>
static int DeallocCounter = 0;

template <typename T>
class Newocator
{
    static int allocations;
    static int deallocations;

public:
    static void init()
    {
        AllocCounter<Newocator<T>> = 0;
        DeallocCounter<Newocator<T>> = 0;
    }

    static void check() { EXPECT_EQ(AllocCounter<Newocator<T>>, DeallocCounter<Newocator<T>>); }

    T* allocate(std::size_t n)
    {
        AllocCounter<Newocator<T>> ++;
        return static_cast<T*>(new T[n]);
    }

    void deallocate(T* p, size_t)
    {
        DeallocCounter<Newocator<T>> ++;
        delete[] p;
    }
};

TEST(dyn_array_tests, custom_allocator)
{
#ifdef GSL_HAS_CONSTEXPR_ALLOCATOR
    static constexpr gsl::dyn_array<char, ConstexprAllocator<char, 10>> mets(10, 'c');
    static_assert(mets.size() == 10);
    static_assert(mets[0] == 'c');
    static_assert(std::all_of(std::begin(mets), std::end(mets), [](char c) { return c == 'c'; }));
#endif /* GSL_HAS_CONSTEXPR_ALLOCATOR */

    Newocator<char>::init();
    {
        gsl::dyn_array<char, Newocator<char>> yankees(10, 'c');
        EXPECT_EQ(yankees.size(), 10);
        EXPECT_TRUE(
            std::all_of(std::begin(yankees), std::end(yankees), [](char c) { return c == 'c'; }));
        yankees[0] = 'a';
        yankees[1] = 'b';
        EXPECT_EQ(yankees[0], 'a');
        EXPECT_EQ(yankees[1], 'b');
        EXPECT_EQ(yankees[2], 'c');
        yankees.get_allocator().deallocate(yankees.get_allocator().allocate(1), 1);
    }
    Newocator<char>::check();
}

TEST(dyn_array_tests, init_list)
{
    gsl::dyn_array<char> phillies = {'a', 'b', 'c'};
    EXPECT_EQ(phillies.size(), 3);
    EXPECT_EQ(phillies[0], 'a');
    EXPECT_EQ(phillies[1], 'b');
    EXPECT_EQ(phillies[2], 'c');
}

TEST(dyn_array_tests, const_operations)
{
    const gsl::dyn_array<char> pirates{'a', 'b', 'c', 'd'};
    EXPECT_EQ(pirates.size(), 4);
    EXPECT_EQ(pirates[0], 'a');
}

TEST(dyn_array_tests, reverse_iterator)
{
    const gsl::dyn_array<char> padres{'a', 'b', 'c'};
    auto it = std::rbegin(padres);
    EXPECT_EQ(*it++, 'c');
    EXPECT_EQ(*it++, 'b');
    EXPECT_EQ(*it++, 'a');
}

#ifdef GSL_HAS_DEDUCTION_GUIDES
TEST(dyn_array_tests, deduction_guides)
{
    std::vector<char> giants{10};
#ifdef GSL_HAS_CONTAINER_RANGES
    gsl::dyn_array mariners(std::from_range_t{}, giants);
#endif /* GSL_HAS_CONTAINER_RANGES */
    gsl::dyn_array cardinals(std::begin(giants), std::end(giants));
}
#endif /* GSL_HAS_DEDUCTION_GUIDES */
