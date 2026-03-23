#include <gtest/gtest.h>

#include <gsl/pointers>

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

#if __cplusplus >= 201703l
using std::void_t;
#else  // __cplusplus >= 201703l
template <class...>
using void_t = void;
#endif // __cplusplus < 201703l

namespace
{
// Custom pointer type that can be used for gsl::not_null, but for which these cannot be swapped.
struct NotMoveAssignableCustomPtr
{
    NotMoveAssignableCustomPtr() = default;
    NotMoveAssignableCustomPtr(const NotMoveAssignableCustomPtr&) = default;
    NotMoveAssignableCustomPtr& operator=(const NotMoveAssignableCustomPtr&) = default;
    NotMoveAssignableCustomPtr(NotMoveAssignableCustomPtr&&) = default;
    NotMoveAssignableCustomPtr& operator=(NotMoveAssignableCustomPtr&&) = delete;

    bool operator!=(std::nullptr_t) const { return true; }

    int dummy{}; // Without this clang warns, that NotMoveAssignableCustomPtr() is unneeded
};

template <typename U, typename = void>
static constexpr bool SwapCompilesFor = false;
template <typename U>
static constexpr bool
    SwapCompilesFor<U, void_t<decltype(gsl::swap<U>(std::declval<gsl::not_null<U>&>(),
                                                    std::declval<gsl::not_null<U>&>()))>> = true;

TEST(pointers_test, swap)
{
    // taken from gh-1129:
    {
        gsl::not_null<std::unique_ptr<int>> a(std::make_unique<int>(0));
        gsl::not_null<std::unique_ptr<int>> b(std::make_unique<int>(1));

        static_assert(noexcept(gsl::swap(a, b)), "not null unique_ptr should be noexcept-swappable");

        EXPECT_TRUE(*a == 0);
        EXPECT_TRUE(*b == 1);

        gsl::swap(a, b);

        EXPECT_TRUE(*a == 1);
        EXPECT_TRUE(*b == 0);

        // Make sure our custom ptr can be used with not_null. The shared_pr is to prevent "unused"
        // compiler warnings.
        const auto shared_custom_ptr{std::make_shared<NotMoveAssignableCustomPtr>()};
        gsl::not_null<NotMoveAssignableCustomPtr> c{*shared_custom_ptr};
        EXPECT_TRUE(c.get() != nullptr);
    }

    {
        gsl::strict_not_null<std::unique_ptr<int>> a{std::make_unique<int>(0)};
        gsl::strict_not_null<std::unique_ptr<int>> b{std::make_unique<int>(1)};

        static_assert(noexcept(gsl::swap(a, b)), "strict not null unique_ptr should be noexcept-swappable");

        EXPECT_TRUE(*a == 0);
        EXPECT_TRUE(*b == 1);

        gsl::swap(a, b);

        EXPECT_TRUE(*a == 1);
        EXPECT_TRUE(*b == 0);
    }

    {
        gsl::not_null<std::unique_ptr<int>> a{std::make_unique<int>(0)};
        gsl::strict_not_null<std::unique_ptr<int>> b{std::make_unique<int>(1)};

        EXPECT_TRUE(*a == 0);
        EXPECT_TRUE(*b == 1);

        gsl::swap(a, b);

        EXPECT_TRUE(*a == 1);
        EXPECT_TRUE(*b == 0);
    }

    static_assert(!SwapCompilesFor<NotMoveAssignableCustomPtr>,
                  "!SwapCompilesFor<NotMoveAssignableCustomPtr>");
}

TEST(pointers_test, not_null_conversion_move_copy)
{

    struct MyBase
    {
        bool base{true};
    };

    struct MyDerived : MyBase
    {
        MyDerived() { base = false; }
    };

    {
        gsl::not_null<std::shared_ptr<MyDerived>> nn_derived{std::make_shared<MyDerived>()};
        gsl::not_null<std::shared_ptr<MyBase>> nn_base{std::make_shared<MyBase>()};

        nn_base = nn_derived; // copy conversion
        EXPECT_EQ(nn_base->base, false);

        // get() const& returns a reference to the shared pointer
        EXPECT_EQ(nn_base.get().use_count(), 2);
        EXPECT_EQ(nn_derived.get().use_count(), 2);

        // get()&& moves the shared pointer
        {
            const auto tmp{std::move(nn_base).get()};
            EXPECT_EQ(tmp.use_count(), 2);
        }
        EXPECT_EQ(nn_derived.get().use_count(), 1);
        EXPECT_EQ(nn_base.get(),
                  nullptr); // Warning: In real code, a use-after-move is not allowed!

        // Start fresh. Using nn_base is now allowed again.
        nn_base = nn_derived; // copy conversion
        EXPECT_EQ(nn_base->base, false);

        // T() const& returns a copy of the shared pointer.
        {
            const std::shared_ptr<MyBase> tmp{nn_base};
            EXPECT_EQ(tmp.use_count(), 3);
        }
        EXPECT_EQ(nn_derived.get().use_count(), 2);

        // T()&& moves the shared pointer
        {
            const std::shared_ptr<MyBase> tmp{std::move(nn_base)};
            EXPECT_EQ(tmp.use_count(), 2);
        }
        EXPECT_EQ(nn_derived.get().use_count(), 1);
        EXPECT_EQ(nn_base.get(),
                  nullptr); // Warning: In real code, a use-after-move is not allowed!
    }
    {
        // Test the same with unique pointers and strict_not_null
        gsl::strict_not_null<std::unique_ptr<MyDerived>> nn_derived{std::make_unique<MyDerived>()};
        gsl::strict_not_null<std::unique_ptr<MyBase>> nn_base{std::make_unique<MyBase>()};

        // nn_base = nn_derived; // does not compile:
        static_assert(!std::is_copy_assignable<decltype(nn_base)>::value,
                      "nn_base is not copy-assignable");
        static_assert(!std::is_copy_constructible<decltype(nn_base)>::value,
                      "nn_base is not copy-constructible");

        nn_base = std::move(nn_derived);
        EXPECT_EQ(nn_base->base, false);

        // get() const& returns a reference
        EXPECT_EQ(nn_base.get()->base, false);

        // get()&& moves the unique pointer
        {
            const auto tmp{std::move(nn_base).get()};
            EXPECT_EQ(tmp->base, false);
        }
        EXPECT_EQ(nn_base.get(),
                  nullptr); // Warning: In real code, a use-after-move is not allowed!

        // Start fresh. Using nn_base is now allowed again.
        nn_base = gsl::strict_not_null<std::unique_ptr<MyBase>>{std::make_unique<MyDerived>()};
        EXPECT_EQ(nn_base->base, false);

        // T() const& returns a copy of the unique pointer, which does not compile.
        {
            static_assert(!std::is_copy_constructible<decltype(nn_base)::element_type>::value,
                          "compilation of `const std::unique_ptr<MyBase>& tmp{nn_base};` is not "
                          "possible because the element type is not copy constructible");
        }

        // T()&& moves the unique pointer
        {
            const std::unique_ptr<MyBase> tmp{std::move(nn_base)};
            EXPECT_EQ(tmp->base, false);
        }
        EXPECT_EQ(nn_base.get(),
                  nullptr); // Warning: In real code, a use-after-move is not allowed!
    }
}

TEST(pointers_test, member_types)
{
    static_assert(std::is_same<gsl::not_null<int*>::element_type, int*>::value,
                  "check member type: element_type");
}

TEST(pointers_test, hash_noexcept_compiles)
{
    {
        using Key = gsl::not_null<std::shared_ptr<int>>;
        static_assert(noexcept(std::hash<Key>{}(std::declval<Key>())),
                      "gsl::not_null hash operator must be noexcept");
    }

    {
        using Key = gsl::strict_not_null<std::shared_ptr<int>>;
        static_assert(noexcept(std::hash<Key>{}(std::declval<Key>())),
                      "gsl::strict_not_null hash operator must be noexcept");
    }
}

} // namespace
