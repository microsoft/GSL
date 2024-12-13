#include <gtest/gtest.h>

#include <gsl/pointers>

#include <memory>

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

