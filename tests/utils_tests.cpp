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

#include <catch/catch.hpp> // for AssertionHandler, StringRef, CHECK, TEST_...

#include <gsl/gsl_util> // for narrow, finally, narrow_cast, narrowing_e...

#include <algorithm>   // for move
#include <functional>  // for reference_wrapper, _Bind_helper<>::type
#include <limits>      // for numeric_limits
#include <stdint.h>    // for uint32_t, int32_t
#include <type_traits> // for is_same

using namespace gsl;

TEST_CASE("sanity check for gsl::index typedef")
{
    static_assert(std::is_same<gsl::index, std::ptrdiff_t>::value,
                  "gsl::index represents wrong arithmetic type");
}

void f(int& i) { i += 1; }

TEST_CASE("finally_lambda")
{
    int i = 0;
    {
        auto _ = finally([&]() { f(i); });
        CHECK(i == 0);
    }
    CHECK(i == 1);
}

TEST_CASE("finally_lambda_move")
{
    int i = 0;
    {
        auto _1 = finally([&]() { f(i); });
        {
            auto _2 = std::move(_1);
            CHECK(i == 0);
        }
        CHECK(i == 1);
        {
            auto _2 = std::move(_1);
            CHECK(i == 1);
        }
        CHECK(i == 1);
    }
    CHECK(i == 1);
}

TEST_CASE("finally_function_with_bind")
{
    int i = 0;
    {
        auto _ = finally(std::bind(&f, std::ref(i)));
        CHECK(i == 0);
    }
    CHECK(i == 1);
}

static int j = 0;
void g() { j += 1; }
TEST_CASE("finally_function_ptr")
{
    j = 0;
    {
        auto _ = finally(&g);
        CHECK(j == 0);
    }
    CHECK(j == 1);
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("narrow_cast")
{
    int n = 120;
    char c = narrow_cast<char>(n);
    CHECK(c == 120);

    n = 300;
    unsigned char uc = narrow_cast<unsigned char>(n);
    CHECK(uc == 44);
}

GSL_SUPPRESS(con.5) // NO-FORMAT: attribute
TEST_CASE("narrow")
{
    int n = 120;
    const char c = narrow<char>(n);
    CHECK(c == 120);

    n = 300;
    CHECK_THROWS_AS(narrow<char>(n), narrowing_error);

    const auto int32_max = std::numeric_limits<int32_t>::max();
    const auto int32_min = std::numeric_limits<int32_t>::min();

    CHECK(narrow<uint32_t>(int32_t(0)) == 0);
    CHECK(narrow<uint32_t>(int32_t(1)) == 1);
    CHECK(narrow<uint32_t>(int32_max) == static_cast<uint32_t>(int32_max));

    CHECK_THROWS_AS(narrow<uint32_t>(int32_t(-1)), narrowing_error);
    CHECK_THROWS_AS(narrow<uint32_t>(int32_min), narrowing_error);

    n = -42;
    CHECK_THROWS_AS(narrow<unsigned>(n), narrowing_error);

#if GSL_CONSTEXPR_NARROW
    static_assert(narrow<char>(120) == 120, "Fix GSL_CONSTEXPR_NARROW");
#endif
}
