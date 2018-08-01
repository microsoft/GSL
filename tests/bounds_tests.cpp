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

#include <catch/catch.hpp> // for AssertionHandler, StringRef, TEST_CASE

#include <gsl/multi_span> // for static_bounds, static_bounds_dynamic_range_t

#include <cstddef> // for ptrdiff_t, size_t

namespace gsl {
struct fail_fast;
}  // namespace gsl

using namespace std;
using namespace gsl;

namespace
{
void use(std::ptrdiff_t&) {}
}

GSL_SUPPRESS(type.1) // NO-FORMAT: attribute
TEST_CASE("basic_bounds")
{
    for (auto point : static_bounds<dynamic_range, 3, 4>{2}) {
        for (decltype(point)::size_type j = 0;
             j < static_cast<decltype(point)::size_type>(decltype(point)::rank); j++)
        {
            use(j);
            use(point[static_cast<std::size_t>(j)]);
        }
    }
}

GSL_SUPPRESS(f.4) // NO-FORMAT: attribute
GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("bounds_basic")
{
    static_bounds<3, 4, 5> b;
    const auto a = b.slice();
    (void) a;
    static_bounds<4, dynamic_range, 2> x{4};
    x.slice().slice();
}

GSL_SUPPRESS(f.4) // NO-FORMAT: attribute
GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("arrayview_iterator")
{
    static_bounds<4, dynamic_range, 2> bounds{3};

    const auto itr = bounds.begin();
    (void) itr;
#ifdef CONFIRM_COMPILATION_ERRORS
    multi_span<int, 4, dynamic_range, 2> av(nullptr, bounds);

    auto itr2 = av.cbegin();

    for (auto& v : av) {
        v = 4;
    }
    fill(av.begin(), av.end(), 0);
#endif
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("bounds_convertible")
{
    static_bounds<7, 4, 2> b1;
    static_bounds<7, dynamic_range, 2> b2 = b1;
    (void) b2;
#ifdef CONFIRM_COMPILATION_ERRORS
    static_bounds<7, dynamic_range, 1> b4 = b2;
#endif

    static_bounds<dynamic_range, dynamic_range, dynamic_range> b3 = b1;
    static_bounds<7, 4, 2> b4 = b3;
    (void) b4;

    static_bounds<dynamic_range> b11;

    static_bounds<dynamic_range> b5;
    static_bounds<34> b6;

    b5 = static_bounds<20>();
    CHECK_THROWS_AS(b6 = b5, fail_fast);
    b5 = static_bounds<34>();
    b6 = b5;

    CHECK(b5 == b6);
    CHECK(b5.size() == b6.size());
}

#ifdef CONFIRM_COMPILATION_ERRORS
copy(src_span_static, dst_span_static);
#endif
