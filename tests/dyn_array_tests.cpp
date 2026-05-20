#include <gtest/gtest.h>

#include "deathTestCommon.h"
#include <gsl/dyn_array>
#include <gsl/util>
#include "gsl/dyn_array"
#include <cstdlib>
#include <exception>
#include <iostream>
#include <sstream>
#include <type_traits>

// Despite using <algorithm> and <ranges> utilities in this test, they
// are not being included directly by this file as a test to ensure
// transitive inclusion via <gsl/dyn_array>.

static_assert(sizeof(gsl::dyn_array<int>) == 2 * sizeof(void*),
              "gsl::dyn_array (with the default allocator) should be 16 bytes");

#ifdef GSL_HAS_CONCEPTS
static_assert(std::input_iterator<gsl::dyn_array<int>::iterator>,
              "gsl::dyn_array should expose a valid input_iterator");
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
constexpr auto default_constructed_count_dyn_array_is_constexpr()
{
    gsl::dyn_array<int> values(3);
    return values.size() == 3 && values[0] == 0 && values[1] == 0 && values[2] == 0;
}

constexpr auto copy_assigned_dyn_array_is_constexpr()
{
    gsl::dyn_array<int> source(3, 7);
    gsl::dyn_array<int> target(2, 4);

    target = source;

    return target.size() == 3 && target[0] == 7 && target[1] == 7 && target[2] == 7;
}

TEST(dyn_array_tests, constexprness)
{
    constexpr gsl::dyn_array<char> marlins;
    static_assert(marlins == marlins);
    static_assert(marlins.empty());
    static_assert(marlins.size() == 0);
    static_assert(marlins.data() == nullptr);
    static_assert(marlins.begin() == marlins.end());
    static_assert(std::distance(marlins.begin(), marlins.end()) == 0);
    static_assert(default_constructed_count_dyn_array_is_constexpr());
    static_assert(copy_assigned_dyn_array_is_constexpr());
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
    gsl::dyn_array<char> mets(std::from_range, twins);
    EXPECT_EQ(twins.size(), mets.size());
    EXPECT_TRUE(std::ranges::all_of(mets, [](char c) { return c == 'c'; }));
#endif /* GSL_HAS_CONTAINER_RANGES */
}
#endif /* GSL_HAS_RANGES */

#ifdef GSL_HAS_CONSTEXPR_ALLOCATOR
template <typename T, unsigned N>
struct ConstexprAllocator
{
    using value_type = T;

    T buf[N]{};
    std::size_t sz{};

    constexpr ConstexprAllocator() = default;

    template <typename U>
    constexpr ConstexprAllocator(const ConstexprAllocator<U, N>&) noexcept
        : buf{}, sz{}
    {}

    template <typename U>
    struct rebind
    {
        using other = ConstexprAllocator<U, N>;
    };

    constexpr auto allocate(std::size_t n) -> value_type*
    {
        auto addr = &buf[sz];
        sz += n;
        return addr;
    }

    constexpr void deallocate(value_type*, std::size_t) noexcept {}
};

template <typename T1, unsigned N1, typename T2, unsigned N2>
constexpr auto operator==(const ConstexprAllocator<T1, N1>& lhs,
                          const ConstexprAllocator<T2, N2>& rhs) noexcept
{
    return std::addressof(lhs) == std::addressof(rhs);
}

template <typename T1, unsigned N1, typename T2, unsigned N2>
constexpr auto operator!=(const ConstexprAllocator<T1, N1>& lhs,
                          const ConstexprAllocator<T2, N2>& rhs) noexcept
{
    return !(lhs == rhs);
}
#endif /* GSL_HAS_CONSTEXPR_ALLOCATOR */

template <typename T>
static int AllocCounter = 0;

template <typename T>
static int DeallocCounter = 0;

struct LifetimeCounter
{
    static int alive_count;

    int value{};

    explicit LifetimeCounter(int v = 0) : value(v) { ++alive_count; }

    LifetimeCounter(const LifetimeCounter& other) : value(other.value) { ++alive_count; }

    ~LifetimeCounter() { --alive_count; }
};

int LifetimeCounter::alive_count = 0;

struct DefaultConstructionCounter
{
    static int default_constructor_count;
    static int copy_constructor_count;

    int value{};

    DefaultConstructionCounter() { ++default_constructor_count; }

    DefaultConstructionCounter(const DefaultConstructionCounter& other) : value(other.value)
    {
        ++copy_constructor_count;
    }

    static void reset()
    {
        default_constructor_count = 0;
        copy_constructor_count = 0;
    }
};

int DefaultConstructionCounter::default_constructor_count = 0;
int DefaultConstructionCounter::copy_constructor_count = 0;

struct DefaultOnlyElement
{
    DefaultOnlyElement() = default;
    DefaultOnlyElement(const DefaultOnlyElement&) = delete;
    DefaultOnlyElement& operator=(const DefaultOnlyElement&) = delete;
    DefaultOnlyElement(DefaultOnlyElement&&) = delete;
    DefaultOnlyElement& operator=(DefaultOnlyElement&&) = delete;
};

struct ThrowOnCopy
{
    static int alive_count;
    static int copy_count;
    static int throw_on_copy_index;

    int value{};

    explicit ThrowOnCopy(int v = 0) : value(v) { ++alive_count; }

    ThrowOnCopy(const ThrowOnCopy& other) : value(other.value)
    {
        if (copy_count == throw_on_copy_index) {
            ++copy_count;
            throw 42;
        }
        ++copy_count;
        ++alive_count;
    }

    ~ThrowOnCopy() { --alive_count; }
};

int ThrowOnCopy::alive_count = 0;
int ThrowOnCopy::copy_count = 0;
int ThrowOnCopy::throw_on_copy_index = -1;

template <typename T>
class Newocator
{
public:
    using value_type = T;

    Newocator() = default;

    template <typename U>
    Newocator(const Newocator<U>&) noexcept
    {}

    static void init()
    {
        AllocCounter<Newocator<T>> = 0;
        DeallocCounter<Newocator<T>> = 0;
    }

    static void check() { EXPECT_EQ(AllocCounter<Newocator<T>>, DeallocCounter<Newocator<T>>); }

    auto allocate(std::size_t n) -> value_type*
    {
        AllocCounter<Newocator<T>>++;
        return static_cast<value_type*>(::operator new(n * sizeof(value_type)));
    }

    void deallocate(value_type* p, std::size_t) noexcept
    {
        DeallocCounter<Newocator<T>>++;
        ::operator delete(p);
    }

    template <typename U>
    struct rebind
    {
        using other = Newocator<U>;
    };
};

template <typename T, typename U>
constexpr auto operator==(const Newocator<T>&, const Newocator<U>&) noexcept
{
    return true;
}

template <typename T, typename U>
constexpr auto operator!=(const Newocator<T>& lhs, const Newocator<U>& rhs) noexcept
{
    return !(lhs == rhs);
}

template <typename T>
class OwnershipTrackingAllocator
{
public:
    using value_type = T;

    OwnershipTrackingAllocator() noexcept : owner_id(next_owner_id()) { ++next_owner_id(); }

    explicit OwnershipTrackingAllocator(int owner) noexcept : owner_id(owner) {}

    template <typename U>
    OwnershipTrackingAllocator(const OwnershipTrackingAllocator<U>& other) noexcept
        : owner_id(other.owner())
    {}

    auto allocate(std::size_t count) -> value_type*
    {
        static_assert(alignof(value_type) <= alignof(int),
                      "test allocator only supports types with int-or-smaller alignment");
        auto raw = static_cast<unsigned char*>(::operator new(sizeof(int) + count * sizeof(value_type)));
        *reinterpret_cast<int*>(raw) = owner_id;
        ++allocation_count();
        return reinterpret_cast<value_type*>(raw + sizeof(int));
    }

    void deallocate(value_type* pointer, std::size_t) noexcept
    {
        auto raw = reinterpret_cast<unsigned char*>(pointer) - sizeof(int);
        if (*reinterpret_cast<int*>(raw) != owner_id) {
            ++mismatched_deallocation_count();
        }
        ++deallocation_count();
        ::operator delete(raw);
    }

    auto owner() const noexcept { return owner_id; }

    static void reset()
    {
        next_owner_id() = 1;
        allocation_count() = 0;
        deallocation_count() = 0;
        mismatched_deallocation_count() = 0;
    }

    static auto mismatched_deallocations() { return mismatched_deallocation_count(); }

private:
    int owner_id;

    static auto next_owner_id() -> int&
    {
        static int value = 1;
        return value;
    }

    static auto allocation_count() -> int&
    {
        static int value = 0;
        return value;
    }

    static auto deallocation_count() -> int&
    {
        static int value = 0;
        return value;
    }

    static auto mismatched_deallocation_count() -> int&
    {
        static int value = 0;
        return value;
    }
};

template <typename T, typename U>
constexpr auto operator==(const OwnershipTrackingAllocator<T>& lhs,
                          const OwnershipTrackingAllocator<U>& rhs) noexcept
{
    return lhs.owner() == rhs.owner();
}

template <typename T, typename U>
constexpr auto operator!=(const OwnershipTrackingAllocator<T>& lhs,
                          const OwnershipTrackingAllocator<U>& rhs) noexcept
{
    return !(lhs == rhs);
}

TEST(dyn_array_tests, custom_allocator_models_allocator)
{
    using traits = std::allocator_traits<Newocator<char>>;
    using ptr = traits::pointer;

    static_assert(std::is_same<traits::value_type, char>::value, "allocator trait type mismatch");
    static_assert(std::is_same<ptr, char*>::value, "allocator trait type mismatch");

    Newocator<char> alloc;
    auto p = traits::allocate(alloc, 1);
    traits::deallocate(alloc, p, 1);

#ifdef GSL_HAS_CONSTEXPR_ALLOCATOR
    using constexpr_traits = std::allocator_traits<ConstexprAllocator<char, 10>>;
    static_assert(std::is_same<constexpr_traits::value_type, char>::value, "allocator trait type mismatch");
#endif /* GSL_HAS_CONSTEXPR_ALLOCATOR */
}

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

TEST(dyn_array_tests, copy_assignment_deallocates_with_the_allocator_that_owns_the_storage)
{
    using allocator_type = OwnershipTrackingAllocator<char>;
    using array_type = gsl::dyn_array<char, allocator_type>;

    allocator_type::reset();

    {
        array_type source(3, 's', allocator_type{1});
        array_type target(2, 't', allocator_type{2});

        target = source;
    }

    EXPECT_EQ(allocator_type::mismatched_deallocations(), 0);
}

TEST(dyn_array_tests, non_trivial_elements_are_destroyed)
{
    LifetimeCounter::alive_count = 0;

    {
        gsl::dyn_array<LifetimeCounter> values(5, LifetimeCounter{7});
        EXPECT_EQ(values.size(), 5);
        EXPECT_EQ(LifetimeCounter::alive_count, 5);
    }

    EXPECT_EQ(LifetimeCounter::alive_count, 0);
}

TEST(dyn_array_tests, count_constructor_default_constructs_each_element)
{
    DefaultConstructionCounter::reset();

    {
        gsl::dyn_array<DefaultConstructionCounter> values(4);
        EXPECT_EQ(values.size(), 4);
    }

    EXPECT_EQ(DefaultConstructionCounter::default_constructor_count, 4);
    EXPECT_EQ(DefaultConstructionCounter::copy_constructor_count, 0);
}

#ifdef GSL_DYN_ARRAY_COMPILE_FAILURE_TESTS
TEST(dyn_array_compile_failure_tests, count_constructor_accepts_default_constructible_only_elements)
{
    gsl::dyn_array<DefaultOnlyElement> values(4);
    EXPECT_EQ(values.size(), 4);
}
#endif /* GSL_DYN_ARRAY_COMPILE_FAILURE_TESTS */

TEST(dyn_array_tests, failed_element_construction_rolls_back)
{
    ThrowOnCopy::alive_count = 0;
    ThrowOnCopy::copy_count = 0;
    ThrowOnCopy::throw_on_copy_index = 2;

    EXPECT_THROW((gsl::dyn_array<ThrowOnCopy>(5, ThrowOnCopy{1})), int);
    EXPECT_EQ(ThrowOnCopy::alive_count, 0);

    ThrowOnCopy::throw_on_copy_index = -1;
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

TEST(dyn_array_tests, copy_assignment)
{
    gsl::dyn_array<char> rays(3, 'r');
    const gsl::dyn_array<char> rangers{'a', 'b', 'c'};

    auto& assigned = (rays = rangers);

    EXPECT_EQ(&assigned, &rays);
    EXPECT_EQ(rays.size(), rangers.size());
    EXPECT_EQ(rays[0], 'a');
    EXPECT_EQ(rays[1], 'b');
    EXPECT_EQ(rays[2], 'c');
}

TEST(dyn_array_tests, random_access_iterator_arithmetic)
{
    gsl::dyn_array<char> bluejays{'a', 'b', 'c', 'd'};

    auto first = bluejays.begin();
    auto third = first + 2;

    EXPECT_EQ(*third, 'c');
    EXPECT_EQ(third - first, 2);
    EXPECT_EQ(*(third - 1), 'b');
    EXPECT_EQ(*std::prev(third), 'b');
}

TEST(dyn_array_tests, random_access_iterator_arithmetic_accepts_negative_offsets)
{
    gsl::dyn_array<char> bluejays{'a', 'b', 'c', 'd'};

    auto third = bluejays.begin() + 2;
    char previous{};
    char next{};

    EXPECT_NO_THROW(previous = *(third + -1));
    EXPECT_EQ(previous, 'b');

    EXPECT_NO_THROW(next = *(third - -1));
    EXPECT_EQ(next, 'd');
}

TEST(dyn_array_tests, input_iterator_constructor)
{
    std::istringstream stream{"n a t s"};
    std::istream_iterator<char> first{stream};
    const std::istream_iterator<char> last{};

    gsl::dyn_array<char> nationals(first, last);

    ASSERT_EQ(nationals.size(), 4);
    EXPECT_EQ(nationals[0], 'n');
    EXPECT_EQ(nationals[1], 'a');
    EXPECT_EQ(nationals[2], 't');
    EXPECT_EQ(nationals[3], 's');
}

TEST(dyn_array_tests, contract_violations)
{
    const auto terminateHandler = std::set_terminate([] {
        std::cerr << "Expected Death. dyn_array_contract_violations";
        std::abort();
    });
    const auto expected = GetExpectedDeathString(terminateHandler);

    gsl::dyn_array<char> values(3, 'v');
    gsl::dyn_array<char> other(3, 'o');

    EXPECT_DEATH(values[values.size()], expected);
    EXPECT_DEATH((void) *values.end(), expected);
    EXPECT_DEATH(++values.end(), expected);
    EXPECT_DEATH(--values.begin(), expected);
    EXPECT_DEATH((void) (values.begin() == other.begin()), expected);
}

#ifdef _MSC_VER
TEST(dyn_array_tests, unchecked_iterators)
{
    gsl::dyn_array<char> values;
    const gsl::dyn_array<char> const_values(3, 'v');

    EXPECT_TRUE((std::is_same<decltype(const_values._Unchecked_begin()), const char*>::value));
    EXPECT_TRUE((std::is_same<decltype(const_values._Unchecked_end()), const char*>::value));

    std::size_t count = 0;
    for (const auto value : const_values) {
        EXPECT_EQ(value, 'v');
        ++count;
    }
    EXPECT_EQ(count, const_values.size());

    EXPECT_EQ(values._Unchecked_begin(), nullptr);
    EXPECT_EQ(values._Unchecked_end(), nullptr);
}
#endif /* _MSC_VER */

TEST(DynArrayTests, TypeConsistency)
{
    static_assert(std::is_same<gsl::dyn_array<int>::value_type, int>::value, "Value type mismatch");
    static_assert(std::is_same<gsl::dyn_array<int>::reference, int&>::value, "Reference type mismatch");
    static_assert(std::is_same<gsl::dyn_array<int>::const_reference, const int&>::value, "Const reference type mismatch");
    static_assert(std::is_same<gsl::dyn_array<int>::iterator::value_type, int>::value, "Iterator value type mismatch");
    static_assert(std::is_same<gsl::dyn_array<int>::iterator::reference, int&>::value, "Iterator reference type mismatch");
    static_assert(std::is_same<gsl::dyn_array<int>::iterator::const_reference, const int&>::value, "Iterator const reference type mismatch");
    static_assert(std::is_same<gsl::dyn_array<int>::size_type, std::size_t>::value, "Size type mismatch");
    static_assert(std::is_same<gsl::dyn_array<int>::difference_type, std::ptrdiff_t>::value, "Difference type mismatch");
}

#ifdef GSL_HAS_DEDUCTION_GUIDES
TEST(dyn_array_tests, deduction_guides)
{
    std::vector<char> giants{10};
#ifdef GSL_HAS_CONTAINER_RANGES
    gsl::dyn_array mariners(std::from_range, giants);
#endif /* GSL_HAS_CONTAINER_RANGES */
    gsl::dyn_array cardinals(std::begin(giants), std::end(giants));
}
#endif /* GSL_HAS_DEDUCTION_GUIDES */
