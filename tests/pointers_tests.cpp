#include <gtest/gtest.h>

#include <gsl/pointers>

#include <memory>
#include <type_traits>
#include <utility>

namespace
{

TEST(pointers_test, swap)
{
    // taken from gh-1129:
    gsl::not_null<std::unique_ptr<int>> a(std::make_unique<int>(0));
    gsl::not_null<std::unique_ptr<int>> b(std::make_unique<int>(1));

    EXPECT_TRUE(*a == 0);
    EXPECT_TRUE(*b == 1);

    gsl::swap(a, b);

    EXPECT_TRUE(*a == 1);
    EXPECT_TRUE(*b == 0);
}

//  These are regressions, should be fixed.
struct NotMovable
{
    NotMovable(NotMovable&&) = delete;
    NotMovable& operator=(NotMovable&&) = delete;
};
template <typename U, typename = void>
static constexpr bool SwapCompilesFor = false;
template <typename U>
static constexpr bool SwapCompilesFor<
    U, std::void_t<decltype(gsl::swap<U, void>(std::declval<gsl::not_null<U>&>(),
                                               std::declval<gsl::not_null<U>&>()))>> =
    true;
static_assert(SwapCompilesFor<NotMovable>, "SwapCompilesFor<NotMovable>");

} // namespace
