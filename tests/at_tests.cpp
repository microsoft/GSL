///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2015 Microsoft Corporation. All rights reserved.
//
// This code is licensed under the MIT License (MIT).
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
// blanket turn off warnings from CppCoreCheck from catch
// so people aren't annoyed by them when running the tool.
#pragma warning(disable : 26440 26426) // from catch
#endif

#include <catch/catch.hpp> // for AssertionHandler, StringRef, CHECK_THROW...

#include <gsl/gsl_util> // for at

#include <array>            // for array
#include <cstddef>          // for size_t
#include <initializer_list> // for initializer_list
#include <vector>           // for vector


namespace gsl {
struct fail_fast;
}  // namespace gsl

using gsl::fail_fast;

GSL_SUPPRESS(bounds.4) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.2) // NO-FORMAT: attribute
TEST_CASE("static_array")
{
    int a[4] = {1, 2, 3, 4};
    const int(&c_a)[4] = a;

    for (int i = 0; i < 4; ++i) {
        CHECK(&gsl::at(a, i) == &a[i]);
        CHECK(&gsl::at(c_a, i) == &a[i]);
    }

    CHECK_THROWS_AS(gsl::at(a, -1), fail_fast);
    CHECK_THROWS_AS(gsl::at(a, 4), fail_fast);
    CHECK_THROWS_AS(gsl::at(c_a, -1), fail_fast);
    CHECK_THROWS_AS(gsl::at(c_a, 4), fail_fast);
}

GSL_SUPPRESS(bounds.4) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.2) // NO-FORMAT: attribute
TEST_CASE("std_array")
{
    std::array<int, 4> a = {1, 2, 3, 4};
    const std::array<int, 4>& c_a = a;

    for (int i = 0; i < 4; ++i) {
        CHECK(&gsl::at(a, i) == &a[static_cast<std::size_t>(i)]);
        CHECK(&gsl::at(c_a, i) == &a[static_cast<std::size_t>(i)]);
    }

    CHECK_THROWS_AS(gsl::at(a, -1), fail_fast);
    CHECK_THROWS_AS(gsl::at(a, 4), fail_fast);
    CHECK_THROWS_AS(gsl::at(c_a, -1), fail_fast);
    CHECK_THROWS_AS(gsl::at(c_a, 4), fail_fast);
}

GSL_SUPPRESS(bounds.4) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.2) // NO-FORMAT: attribute
TEST_CASE("StdVector")
{
    std::vector<int> a = {1, 2, 3, 4};
    const std::vector<int>& c_a = a;

    for (int i = 0; i < 4; ++i) {
        CHECK(&gsl::at(a, i) == &a[static_cast<std::size_t>(i)]);
        CHECK(&gsl::at(c_a, i) == &a[static_cast<std::size_t>(i)]);
    }

    CHECK_THROWS_AS(gsl::at(a, -1), fail_fast);
    CHECK_THROWS_AS(gsl::at(a, 4), fail_fast);
    CHECK_THROWS_AS(gsl::at(c_a, -1), fail_fast);
    CHECK_THROWS_AS(gsl::at(c_a, 4), fail_fast);
}

GSL_SUPPRESS(bounds.4) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.2) // NO-FORMAT: attribute
TEST_CASE("InitializerList")
{
    const std::initializer_list<int> a = {1, 2, 3, 4};

    for (int i = 0; i < 4; ++i) {
        CHECK(gsl::at(a, i) == i + 1);
        CHECK(gsl::at({1, 2, 3, 4}, i) == i + 1);
    }

    CHECK_THROWS_AS(gsl::at(a, -1), fail_fast);
    CHECK_THROWS_AS(gsl::at(a, 4), fail_fast);
    CHECK_THROWS_AS(gsl::at({1, 2, 3, 4}, -1), fail_fast);
    CHECK_THROWS_AS(gsl::at({1, 2, 3, 4}, 4), fail_fast);
}

#if !defined(_MSC_VER) || defined(__clang__) || _MSC_VER >= 1910
GSL_SUPPRESS(bounds.4) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.2) // NO-FORMAT: attribute
GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
static constexpr bool test_constexpr()
{
    int a1[4] = {1, 2, 3, 4};
    const int(&c_a1)[4] = a1;
    std::array<int, 4> a2 = {1, 2, 3, 4};
    const std::array<int, 4>& c_a2 = a2;

    for (int i = 0; i < 4; ++i) {
        if (&gsl::at(a1, i) != &a1[i]) return false;
        if (&gsl::at(c_a1, i) != &a1[i]) return false;
        // requires C++17:
        // if (&gsl::at(a2, i) != &a2[static_cast<std::size_t>(i)]) return false;
        if (&gsl::at(c_a2, i) != &c_a2[static_cast<std::size_t>(i)]) return false;
        if (gsl::at({1, 2, 3, 4}, i) != i + 1) return false;
    }

    return true;
}

static_assert(test_constexpr(), "FAIL");
#endif

