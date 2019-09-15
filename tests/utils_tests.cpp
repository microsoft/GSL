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
#include <type_traits> // for is_same, is_copy_constructible, is_nothrow_default_constructible,
                       // is_nothrow_move_constructible, is_copy_assignable, is_nothrow_move_assignable,
                       // is_nothrow_swappable_v, is_swappable_with_v

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

TEST_CASE("stateful joinable_thread joins upon destruction")
{
    // this test only checks if std::terminate() is called;
    // unlike std::thread, this will not call terminate unless join()
    // throws an exception in which case both terminate
    joining_thread([]{});
}

TEST_CASE("joinable_thread constructors")
{
    // due to notion of immediate context, std::is_constructible
    // cannot be used here as it returns wrong results

    // deleted copy constructor
    static_assert(std::is_copy_constructible<joining_thread>::value == false, "");

    // non-spawning constructors
    static_assert(std::is_nothrow_default_constructible<joining_thread>::value, "");
    static_assert(std::is_nothrow_move_constructible<joining_thread>::value, "");
    joining_thread(std::thread{}); // additional move c-tor from xvalue of std::thread

    // spawning constructors
    auto fun_no_parameters = []{};
    auto fun_with_parameters = [](int){};

    joining_thread(std::move(fun_no_parameters));
    joining_thread(std::move(fun_with_parameters), 0);
}

TEST_CASE("joinable_thread assignment operators")
{
    // deleted copy assignment operator
    static_assert(std::is_copy_assignable<joining_thread>::value == false, "");

    // move assignment operator
    static_assert(std::is_nothrow_move_assignable<joining_thread>::value, "");
    std::thread t1;
    joining_thread t2;
    t2 = std::move(t1); // additional move assignment op from xvalue of std::thread
}

TEST_CASE("joinable_thread swap")
{
    // TODO: std::is_swappable from C++17 is a better replacement
#if __cplusplus >= 201703L
    // deleted swapping with std::thread
    static_assert(std::is_swappable_with_v<joining_thread, std::thread> == false);

    // swap with another joining_thread
    static_assert(std::is_nothrow_swappable_v<joining_thread>);
#else
    std::thread t1;
    joining_thread t2;

#ifdef CONFIRM_COMPILATION_ERRORS
    // deleted swapping with std::thread
    t1.swap(t2);
    t2.swap(t1);
#endif // CONFIRM_COMPILATION_ERRORS

    // swap with another joining_thread
    static_assert(noexcept(t2.swap(t2)), "swapping should be a nothrow operation");
#endif // __cplusplus >= 201703L
}

#ifdef CONFIRM_COMPILATION_ERRORS
TEST_CAST("joinable_thread is undetachable")
{
    joining_thread().detach();
    joining_thread([]{}).detach();
}
#endif // CONFIRM_COMPILATION_ERRORS
