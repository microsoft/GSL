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

#include <catch/catch.hpp> // for AssertionHandler, StringRef, CHECK, CHECK...

#include <gsl/gsl_byte>   // for byte
#include <gsl/gsl_util>   // for narrow_cast
#include <gsl/multi_span> // for multi_span, contiguous_span_iterator, dim

#include <algorithm> // for fill, for_each
#include <array>     // for array
#include <iostream>  // for ptrdiff_t, size_t
#include <iterator>  // for reverse_iterator, begin, end, operator!=
#include <numeric>   // for iota
#include <stddef.h>  // for ptrdiff_t
#include <string>    // for string
#include <vector>    // for vector

namespace gsl
{
struct fail_fast;
} // namespace gsl

using namespace std;
using namespace gsl;

namespace
{
struct BaseClass
{
};
struct DerivedClass : BaseClass
{
};
} // namespace

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("default_constructor")
{
    {
        multi_span<int> s;
        CHECK((s.length() == 0 && s.data() == nullptr));

        multi_span<const int> cs;
        CHECK((cs.length() == 0 && cs.data() == nullptr));
    }

    {
        multi_span<int, 0> s;
        CHECK((s.length() == 0 && s.data() == nullptr));

        multi_span<const int, 0> cs;
        CHECK((cs.length() == 0 && cs.data() == nullptr));
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<int, 1> s;
        CHECK((s.length() == 1 && s.data() == nullptr)); // explains why it can't compile
#endif
    }

    {
        multi_span<int> s{};
        CHECK((s.length() == 0 && s.data() == nullptr));

        multi_span<const int> cs{};
        CHECK((cs.length() == 0 && cs.data() == nullptr));
    }
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("from_nullptr_constructor")
{
    {
        multi_span<int> s = nullptr;
        CHECK((s.length() == 0 && s.data() == nullptr));

        multi_span<const int> cs = nullptr;
        CHECK((cs.length() == 0 && cs.data() == nullptr));
    }

    {
        multi_span<int, 0> s = nullptr;
        CHECK((s.length() == 0 && s.data() == nullptr));

        multi_span<const int, 0> cs = nullptr;
        CHECK((cs.length() == 0 && cs.data() == nullptr));
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<int, 1> s = nullptr;
        CHECK((s.length() == 1 && s.data() == nullptr)); // explains why it can't compile
#endif
    }

    {
        multi_span<int> s{nullptr};
        CHECK((s.length() == 0 && s.data() == nullptr));

        multi_span<const int> cs{nullptr};
        CHECK((cs.length() == 0 && cs.data() == nullptr));
    }

    {
        multi_span<int*> s{nullptr};
        CHECK((s.length() == 0 && s.data() == nullptr));

        multi_span<const int*> cs{nullptr};
        CHECK((cs.length() == 0 && cs.data() == nullptr));
    }
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("from_nullptr_length_constructor") {
    {
        multi_span<int> s{nullptr, 0};
        CHECK((s.length() == 0 && s.data() == nullptr));

        multi_span<const int> cs{nullptr, 0};
        CHECK((cs.length() == 0 && cs.data() == nullptr));
    }

    {
        multi_span<int, 0> s{nullptr, 0};
        CHECK((s.length() == 0 && s.data() == nullptr));

        multi_span<const int, 0> cs{nullptr, 0};
        CHECK((cs.length() == 0 && cs.data() == nullptr));
    }

    {
        auto workaround_macro = []() { const multi_span<int> s{nullptr, 1}; };
        CHECK_THROWS_AS(workaround_macro(), fail_fast);

        auto const_workaround_macro = []() { const multi_span<const int> cs{nullptr, 1}; };
        CHECK_THROWS_AS(const_workaround_macro(), fail_fast);
    }

    {
        auto workaround_macro = []() { const multi_span<int, 0> s{nullptr, 1}; };
        CHECK_THROWS_AS(workaround_macro(), fail_fast);

        auto const_workaround_macro = []() { const multi_span<const int, 0> s{nullptr, 1}; };
        CHECK_THROWS_AS(const_workaround_macro(), fail_fast);
    }

    {
        multi_span<int*> s{nullptr, 0};
        CHECK((s.length() == 0 && s.data() == nullptr));

        multi_span<const int*> cs{nullptr, 0};
        CHECK((cs.length() == 0 && cs.data() == nullptr));
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<int, 1> s{nullptr, 0};
        CHECK((s.length() == 1 && s.data() == nullptr)); // explains why it can't compile
#endif
    }
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("from_element_constructor")
{
    int i = 5;

    {
        multi_span<int> s = i;
        CHECK((s.length() == 1 && s.data() == &i));
        CHECK(s[0] == 5);

        multi_span<const int> cs = i;
        CHECK((cs.length() == 1 && cs.data() == &i));
        CHECK(cs[0] == 5);
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        const j = 1;
        multi_span<int, 0> s = j;
#endif
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<int, 0> s = i;
        CHECK((s.length() == 0 && s.data() == &i));
#endif
    }

    {
        multi_span<int, 1> s = i;
        CHECK((s.length() == 1 && s.data() == &i));
        CHECK(s[0] == 5);
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<int, 2> s = i;
        CHECK((s.length() == 2 && s.data() == &i));
#endif
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        auto get_a_temp = []() -> int { return 4; };
        auto use_a_span = [](multi_span<int> s) { (void) s; };
        use_a_span(get_a_temp());
#endif
    }
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("from_pointer_length_constructor")
{
    int arr[4] = {1, 2, 3, 4};

    {
        multi_span<int> s{&arr[0], 2};
        CHECK((s.length() == 2 && s.data() == &arr[0]));
        CHECK((s[0] == 1 && s[1] == 2));
    }

    {
        multi_span<int, 2> s{&arr[0], 2};
        CHECK((s.length() == 2 && s.data() == &arr[0]));
        CHECK((s[0] == 1 && s[1] == 2));
    }

    {
        int* p = nullptr;
        multi_span<int> s{p, 0};
        CHECK((s.length() == 0 && s.data() == nullptr));
    }

    {
        int* p = nullptr;
        auto workaround_macro = [=]() { const multi_span<int> s{p, 2}; };
        CHECK_THROWS_AS(workaround_macro(), fail_fast);
    }
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("from_pointer_pointer_constructor")
{
    int arr[4] = {1, 2, 3, 4};

    {
        multi_span<int> s{&arr[0], &arr[2]};
        CHECK((s.length() == 2 && s.data() == &arr[0]));
        CHECK((s[0] == 1 && s[1] == 2));
    }

    {
        multi_span<int, 2> s{&arr[0], &arr[2]};
        CHECK((s.length() == 2 && s.data() == &arr[0]));
        CHECK((s[0] == 1 && s[1] == 2));
    }

    {
        multi_span<int> s{&arr[0], &arr[0]};
        CHECK((s.length() == 0 && s.data() == &arr[0]));
    }

    {
        multi_span<int, 0> s{&arr[0], &arr[0]};
        CHECK((s.length() == 0 && s.data() == &arr[0]));
    }

    {
        auto workaround_macro = [&]() { const multi_span<int> s{&arr[1], &arr[0]}; };
        CHECK_THROWS_AS(workaround_macro(), fail_fast);
    }

    {
        int* p = nullptr;
        auto workaround_macro = [&]() { const multi_span<int> s{&arr[0], p}; };
        CHECK_THROWS_AS(workaround_macro(), fail_fast);
    }

    {
        int* p = nullptr;
        auto workaround_macro = [&]() { const multi_span<int> s{p, p}; };
        CHECK_THROWS_AS(workaround_macro(), fail_fast);
    }

    {
        int* p = nullptr;
        auto workaround_macro = [&]() { const multi_span<int> s{&arr[0], p}; };
        CHECK_THROWS_AS(workaround_macro(), fail_fast);
    }
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.3) // NO-FORMAT: attribute
TEST_CASE("from_array_constructor")
{
    int arr[5] = {1, 2, 3, 4, 5};

    {
        multi_span<int> s{arr};
        CHECK((s.length() == 5 && s.data() == &arr[0]));
    }

    {
        multi_span<int, 5> s{arr};
        CHECK((s.length() == 5 && s.data() == &arr[0]));
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<int, 6> s{arr};
#endif
    }

    {
        multi_span<int, 0> s{arr};
        CHECK((s.length() == 0 && s.data() == &arr[0]));
    }

    int arr2d[2][3] = {1, 2, 3, 4, 5, 6};

    {
        multi_span<int> s{arr2d};
        CHECK((s.length() == 6 && s.data() == &arr2d[0][0]));
        CHECK((s[0] == 1 && s[5] == 6));
    }

    {
        multi_span<int, 0> s{arr2d};
        CHECK((s.length() == 0 && s.data() == &arr2d[0][0]));
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<int, 5> s{arr2d};
#endif
    }

    {
        multi_span<int, 6> s{arr2d};
        CHECK((s.length() == 6 && s.data() == &arr2d[0][0]));
        CHECK((s[0] == 1 && s[5] == 6));
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<int, 7> s{arr2d};
#endif
    }

    {
        multi_span<int[3]> s{arr2d[0]};
        CHECK((s.length() == 1 && s.data() == &arr2d[0]));
    }

    {
        multi_span<int, 2, 3> s{arr2d};
        CHECK((s.length() == 6 && s.data() == &arr2d[0][0]));
        auto workaround_macro = [&]() { return s[{1, 2}] == 6; };
        CHECK(workaround_macro());
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<int, 3, 3> s{arr2d};
#endif
    }

    int arr3d[2][3][2] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

    {
        multi_span<int> s{arr3d};
        CHECK((s.length() == 12 && s.data() == &arr3d[0][0][0]));
        CHECK((s[0] == 1 && s[11] == 12));
    }

    {
        multi_span<int, 0> s{arr3d};
        CHECK((s.length() == 0 && s.data() == &arr3d[0][0][0]));
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<int, 11> s{arr3d};
#endif
    }

    {
        multi_span<int, 12> s{arr3d};
        CHECK((s.length() == 12 && s.data() == &arr3d[0][0][0]));
        CHECK((s[0] == 1 && s[5] == 6));
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<int, 13> s{arr3d};
#endif
    }

    {
        multi_span<int[3][2]> s{arr3d[0]};
        CHECK((s.length() == 1 && s.data() == &arr3d[0]));
    }

    {
        multi_span<int, 3, 2, 2> s{arr3d};
        CHECK((s.length() == 12 && s.data() == &arr3d[0][0][0]));
        auto workaround_macro = [&]() { return s[{2, 1, 0}] == 11; };
        CHECK(workaround_macro());
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<int, 3, 3, 3> s{arr3d};
#endif
    }
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.1) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.4) // NO-FORMAT: attribute
GSL_SUPPRESS(i.11) // NO-FORMAT: attribute
GSL_SUPPRESS(r.11)  // NO-FORMAT: attribute
TEST_CASE("from_dynamic_array_constructor")
{
    double(*arr)[3][4] = new double[100][3][4];

    {
        multi_span<double, dynamic_range, 3, 4> s(arr, 10);
        CHECK((s.length() == 120 && s.data() == &arr[0][0][0]));
        CHECK_THROWS_AS(s[10][3][4], fail_fast);
    }

    {
        multi_span<double, dynamic_range, 4, 3> s(arr, 10);
        CHECK((s.length() == 120 && s.data() == &arr[0][0][0]));
    }

    {
        multi_span<double> s(arr, 10);
        CHECK((s.length() == 120 && s.data() == &arr[0][0][0]));
    }

    {
        multi_span<double, dynamic_range, 3, 4> s(arr, 0);
        CHECK((s.length() == 0 && s.data() == &arr[0][0][0]));
    }

    delete[] arr;
}

GSL_SUPPRESS(con.4) // NO-FORMAT: Attribute
TEST_CASE("from_std_array_constructor")
{
    std::array<int, 4> arr = {1, 2, 3, 4};

    {
        multi_span<int> s{arr};
        CHECK((s.size() == narrow_cast<ptrdiff_t>(arr.size()) && s.data() == arr.data()));

        multi_span<const int> cs{arr};
        CHECK((cs.size() == narrow_cast<ptrdiff_t>(arr.size()) && cs.data() == arr.data()));
    }

    {
        multi_span<int, 4> s{arr};
        CHECK((s.size() == narrow_cast<ptrdiff_t>(arr.size()) && s.data() == arr.data()));

        multi_span<const int, 4> cs{arr};
        CHECK((cs.size() == narrow_cast<ptrdiff_t>(arr.size()) && cs.data() == arr.data()));
    }

    {
        multi_span<int, 2> s{arr};
        CHECK((s.size() == 2 && s.data() == arr.data()));

        multi_span<const int, 2> cs{arr};
        CHECK((cs.size() == 2 && cs.data() == arr.data()));
    }

    {
        multi_span<int, 0> s{arr};
        CHECK((s.size() == 0 && s.data() == arr.data()));

        multi_span<const int, 0> cs{arr};
        CHECK((cs.size() == 0 && cs.data() == arr.data()));
    }

    // TODO This is currently an unsupported scenario. We will come back to it as we revise
    // the multidimensional interface and what transformations between dimensionality look like
    //{
    //    multi_span<int, 2, 2> s{arr};
    //    CHECK(s.size() == narrow_cast<ptrdiff_t>(arr.size()) && s.data() == arr.data());
    //}

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<int, 5> s{arr};
#endif
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        auto get_an_array = []() { return std::array<int, 4>{1, 2, 3, 4}; };
        auto take_a_span = [](multi_span<int> s) { (void) s; };
        // try to take a temporary std::array
        take_a_span(get_an_array());
#endif
    }
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("from_const_std_array_constructor")
{
    const std::array<int, 4> arr = {1, 2, 3, 4};

    {
        multi_span<const int> s{arr};
        CHECK((s.size() == narrow_cast<ptrdiff_t>(arr.size()) && s.data() == arr.data()));
    }

    {
        multi_span<const int, 4> s{arr};
        CHECK((s.size() == narrow_cast<ptrdiff_t>(arr.size()) && s.data() == arr.data()));
    }

    {
        multi_span<const int, 2> s{arr};
        CHECK((s.size() == 2 && s.data() == arr.data()));
    }

    {
        multi_span<const int, 0> s{arr};
        CHECK((s.size() == 0 && s.data() == arr.data()));
    }

    // TODO This is currently an unsupported scenario. We will come back to it as we revise
    // the multidimensional interface and what transformations between dimensionality look like
    //{
    //    multi_span<int, 2, 2> s{arr};
    //    CHECK(s.size() == narrow_cast<ptrdiff_t>(arr.size()) && s.data() == arr.data());
    //}

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<const int, 5> s{arr};
#endif
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        auto get_an_array = []() -> const std::array<int, 4> { return {1, 2, 3, 4}; };
        auto take_a_span = [](multi_span<const int> s) { (void) s; };
        // try to take a temporary std::array
        take_a_span(get_an_array());
#endif
    }
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("from_container_constructor")
{
    std::vector<int> v = {1, 2, 3};
    const std::vector<int> cv = v;

    {
        multi_span<int> s{v};
        CHECK((s.size() == narrow_cast<std::ptrdiff_t>(v.size()) && s.data() == v.data()));

        multi_span<const int> cs{v};
        CHECK((cs.size() == narrow_cast<std::ptrdiff_t>(v.size()) && cs.data() == v.data()));
    }

    std::string str = "hello";
    const std::string cstr = "hello";

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<char> s{str};
        CHECK((s.size() == narrow_cast<std::ptrdiff_t>(str.size()) && s.data() == str.data()));
#endif
        multi_span<const char> cs{str};
        CHECK((cs.size() == narrow_cast<std::ptrdiff_t>(str.size()) && cs.data() == str.data()));
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<char> s{cstr};
#endif
        multi_span<const char> cs{cstr};
        CHECK((cs.size() == narrow_cast<std::ptrdiff_t>(cstr.size()) && cs.data() == cstr.data()));
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        auto get_temp_vector = []() -> std::vector<int> { return {}; };
        auto use_span = [](multi_span<int> s) { (void) s; };
        use_span(get_temp_vector());
#endif
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        auto get_temp_string = []() -> std::string { return {}; };
        auto use_span = [](multi_span<char> s) { (void) s; };
        use_span(get_temp_string());
#endif
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        auto get_temp_vector = []() -> const std::vector<int> { return {}; };
        auto use_span = [](multi_span<const char> s) { (void) s; };
        use_span(get_temp_vector());
#endif
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        auto get_temp_string = []() -> const std::string { return {}; };
        auto use_span = [](multi_span<const char> s) { (void) s; };
        use_span(get_temp_string());
#endif
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        std::map<int, int> m;
        multi_span<int> s{m};
#endif
    }
}

GSL_SUPPRESS(f.4) // NO-FORMAT: attribute
GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("from_convertible_span_constructor")
{
#ifdef CONFIRM_COMPILATION_ERRORS
    multi_span<int, 7, 4, 2> av1(nullptr, b1);

    auto f = [&]() { multi_span<int, 7, 4, 2> av1(nullptr); };
    CHECK_THROWS_AS(f(), fail_fast);
#endif

#ifdef CONFIRM_COMPILATION_ERRORS
    static_bounds<std::size_t, 7, dynamic_range, 2> b12(b11);
    b12 = b11;
    b11 = b12;

    multi_span<int, dynamic_range> av1 = nullptr;
    multi_span<int, 7, dynamic_range, 2> av2(av1);
    multi_span<int, 7, 4, 2> av2(av1);
#endif

    multi_span<DerivedClass> avd;
#ifdef CONFIRM_COMPILATION_ERRORS
    multi_span<BaseClass> avb = avd;
#endif
    multi_span<const DerivedClass> avcd = avd;
    (void) avcd;
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("copy_move_and_assignment")
{
    multi_span<int> s1;
    CHECK(s1.empty());

    int arr[] = {3, 4, 5};

    multi_span<const int> s2 = arr;
    CHECK((s2.length() == 3 && s2.data() == &arr[0]));

    s2 = s1;
    CHECK(s2.empty());

    auto get_temp_span = [&]() -> multi_span<int> { return {&arr[1], 2}; };
    auto use_span = [&](multi_span<const int> s) {
        CHECK((s.length() == 2 && s.data() == &arr[1]));
    };
    use_span(get_temp_span());

    s1 = get_temp_span();
    CHECK((s1.length() == 2 && s1.data() == &arr[1]));
}

template <class Bounds>
void fn(const Bounds&)
{
    static_assert(Bounds::static_size == 60, "static bounds is wrong size");
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("as_multi_span_reshape")
{
    int a[3][4][5];
    auto av = as_multi_span(a);
    fn(av.bounds());
    auto av2 = as_multi_span(av, dim<60>());
    auto av3 = as_multi_span(av2, dim<3>(), dim<4>(), dim<5>());
    auto av4 = as_multi_span(av3, dim<4>(), dim(3), dim<5>());
    auto av5 = as_multi_span(av4, dim<3>(), dim<4>(), dim<5>());
    auto av6 = as_multi_span(av5, dim<12>(), dim(5));

    fill(av6.begin(), av6.end(), 1);

    auto av7 = as_bytes(av6);

    auto av8 = as_multi_span<int>(av7);

    CHECK(av8.size() == av6.size());
    for (auto i = 0; i < av8.size(); i++) { CHECK(av8[i] == 1); }
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("first")
{
    int arr[5] = {1, 2, 3, 4, 5};

    {
        multi_span<int, 5> av = arr;
        CHECK((av.first<2>().bounds() == static_bounds<2>()));
        CHECK(av.first<2>().length() == 2);
        CHECK(av.first(2).length() == 2);
    }

    {
        multi_span<int, 5> av = arr;
        CHECK((av.first<0>().bounds() == static_bounds<0>()));
        CHECK(av.first<0>().length() == 0);
        CHECK(av.first(0).length() == 0);
    }

    {
        multi_span<int, 5> av = arr;
        CHECK((av.first<5>().bounds() == static_bounds<5>()));
        CHECK(av.first<5>().length() == 5);
        CHECK(av.first(5).length() == 5);
    }

    {
        multi_span<int, 5> av = arr;
#ifdef CONFIRM_COMPILATION_ERRORS
        CHECK(av.first<6>().bounds() == static_bounds<6>());
        CHECK(av.first<6>().length() == 6);
        CHECK(av.first<-1>().length() == -1);
#endif
        CHECK_THROWS_AS(av.first(6).length(), fail_fast);
    }

    {
        multi_span<int, dynamic_range> av;
        CHECK((av.first<0>().bounds() == static_bounds<0>()));
        CHECK(av.first<0>().length() == 0);
        CHECK(av.first(0).length() == 0);
    }
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("last")
{
    int arr[5] = {1, 2, 3, 4, 5};

    {
        multi_span<int, 5> av = arr;
        CHECK((av.last<2>().bounds() == static_bounds<2>()));
        CHECK(av.last<2>().length() == 2);
        CHECK(av.last(2).length() == 2);
    }

    {
        multi_span<int, 5> av = arr;
        CHECK((av.last<0>().bounds() == static_bounds<0>()));
        CHECK(av.last<0>().length() == 0);
        CHECK(av.last(0).length() == 0);
    }

    {
        multi_span<int, 5> av = arr;
        CHECK((av.last<5>().bounds() == static_bounds<5>()));
        CHECK(av.last<5>().length() == 5);
        CHECK(av.last(5).length() == 5);
    }

    {
        multi_span<int, 5> av = arr;
#ifdef CONFIRM_COMPILATION_ERRORS
        CHECK((av.last<6>().bounds() == static_bounds<6>()));
        CHECK(av.last<6>().length() == 6);
#endif
        CHECK_THROWS_AS(av.last(6).length(), fail_fast);
    }

    {
        multi_span<int, dynamic_range> av;
        CHECK((av.last<0>().bounds() == static_bounds<0>()));
        CHECK(av.last<0>().length() == 0);
        CHECK(av.last(0).length() == 0);
    }
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("subspan")
{
    int arr[5] = {1, 2, 3, 4, 5};

    {
        multi_span<int, 5> av = arr;
        CHECK((av.subspan<2, 2>().bounds() == static_bounds<2>()));
        CHECK((av.subspan<2, 2>().length() == 2));
        CHECK(av.subspan(2, 2).length() == 2);
        CHECK(av.subspan(2, 3).length() == 3);
    }

    {
        multi_span<int, 5> av = arr;
        CHECK((av.subspan<0, 0>().bounds() == static_bounds<0>()));
        CHECK((av.subspan<0, 0>().length() == 0));
        CHECK(av.subspan(0, 0).length() == 0);
    }

    {
        multi_span<int, 5> av = arr;
        CHECK((av.subspan<0, 5>().bounds() == static_bounds<5>()));
        CHECK((av.subspan<0, 5>().length() == 5));
        CHECK(av.subspan(0, 5).length() == 5);
        CHECK_THROWS_AS(av.subspan(0, 6).length(), fail_fast);
        CHECK_THROWS_AS(av.subspan(1, 5).length(), fail_fast);
    }

    {
        multi_span<int, 5> av = arr;
        CHECK((av.subspan<5, 0>().bounds() == static_bounds<0>()));
        CHECK((av.subspan<5, 0>().length() == 0));
        CHECK(av.subspan(5, 0).length() == 0);
        CHECK_THROWS_AS(av.subspan(6, 0).length(), fail_fast);
    }

    {
        multi_span<int, dynamic_range> av;
        CHECK((av.subspan<0, 0>().bounds() == static_bounds<0>()));
        CHECK((av.subspan<0, 0>().length() == 0));
        CHECK(av.subspan(0, 0).length() == 0);
        CHECK_THROWS_AS((av.subspan<1, 0>().length()), fail_fast);
    }

    {
        multi_span<int> av;
        CHECK(av.subspan(0).length() == 0);
        CHECK_THROWS_AS(av.subspan(1).length(), fail_fast);
    }

    {
        multi_span<int> av = arr;
        CHECK(av.subspan(0).length() == 5);
        CHECK(av.subspan(1).length() == 4);
        CHECK(av.subspan(4).length() == 1);
        CHECK(av.subspan(5).length() == 0);
        CHECK_THROWS_AS(av.subspan(6).length(), fail_fast);
        auto av2 = av.subspan(1);
        for (int i = 0; i < 4; ++i) CHECK(av2[i] == i + 2);
    }

    {
        multi_span<int, 5> av = arr;
        CHECK(av.subspan(0).length() == 5);
        CHECK(av.subspan(1).length() == 4);
        CHECK(av.subspan(4).length() == 1);
        CHECK(av.subspan(5).length() == 0);
        CHECK_THROWS_AS(av.subspan(6).length(), fail_fast);
        auto av2 = av.subspan(1);
        for (int i = 0; i < 4; ++i) CHECK(av2[i] == i + 2);
    }
}

GSL_SUPPRESS(con.4)  // NO-FORMAT: attribute
TEST_CASE("rank")
{
    int arr[2] = {1, 2};

    {
        multi_span<int> s;
        CHECK(s.rank() == 1);
    }

    {
        multi_span<int, 2> s = arr;
        CHECK(s.rank() == 1);
    }

    int arr2d[1][1] = {};
    {
        multi_span<int, 1, 1> s = arr2d;
        CHECK(s.rank() == 2);
    }
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("extent")
{
    {
        multi_span<int> s;
        CHECK(s.extent() == 0);
        CHECK(s.extent(0) == 0);
        CHECK_THROWS_AS(s.extent(1), fail_fast);
#ifdef CONFIRM_COMPILATION_ERRORS
        CHECK(s.extent<1>() == 0);
#endif
    }

    {
        multi_span<int, 0> s;
        CHECK(s.extent() == 0);
        CHECK(s.extent(0) == 0);
        CHECK_THROWS_AS(s.extent(1), fail_fast);
    }

    {
        int arr2d[1][2] = {};

        multi_span<int, 1, 2> s = arr2d;
        CHECK(s.extent() == 1);
        CHECK(s.extent<0>() == 1);
        CHECK(s.extent<1>() == 2);
        CHECK(s.extent(0) == 1);
        CHECK(s.extent(1) == 2);
        CHECK_THROWS_AS(s.extent(3), fail_fast);
    }

    {
        int arr2d[1][2] = {};

        multi_span<int, 0, 2> s = arr2d;
        CHECK(s.extent() == 0);
        CHECK(s.extent<0>() == 0);
        CHECK(s.extent<1>() == 2);
        CHECK(s.extent(0) == 0);
        CHECK(s.extent(1) == 2);
        CHECK_THROWS_AS(s.extent(3), fail_fast);
    }
}

TEST_CASE("operator_function_call")
{
    int arr[4] = {1, 2, 3, 4};

    {
        multi_span<int> s = arr;
        CHECK(s(0) == 1);
        CHECK_THROWS_AS(s(5), fail_fast);
    }

    int arr2d[2][3] = {1, 2, 3, 4, 5, 6};

    {
        multi_span<int, 2, 3> s = arr2d;
        CHECK(s(0, 0) == 1);
        CHECK(s(0, 1) == 2);
        CHECK(s(1, 2) == 6);
    }

    int arr3d[2][2][2] = {1, 2, 3, 4, 5, 6, 7, 8};

    {
        multi_span<int, 2, 2, 2> s = arr3d;
        CHECK(s(0, 0, 0) == 1);
        CHECK(s(1, 1, 1) == 8);
    }
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("comparison_operators")
{
    {
        int arr[10][2];
        auto s1 = as_multi_span(arr);
        multi_span<const int, dynamic_range, 2> s2 = s1;

        CHECK(s1 == s2);

        multi_span<int, 20> s3 = as_multi_span(s1, dim(20));
        CHECK((s3 == s2 && s3 == s1));
    }

    {
        multi_span<int> s1 = nullptr;
        multi_span<int> s2 = nullptr;
        CHECK(s1 == s2);
        CHECK(!(s1 != s2));
        CHECK(!(s1 < s2));
        CHECK(s1 <= s2);
        CHECK(!(s1 > s2));
        CHECK(s1 >= s2);
        CHECK(s2 == s1);
        CHECK(!(s2 != s1));
        CHECK(!(s2 < s1));
        CHECK(s2 <= s1);
        CHECK(!(s2 > s1));
        CHECK(s2 >= s1);
    }

    {
        int arr[] = {2, 1}; // bigger

        multi_span<int> s1 = nullptr;
        multi_span<int> s2 = arr;

        CHECK(s1 != s2);
        CHECK(s2 != s1);
        CHECK(!(s1 == s2));
        CHECK(!(s2 == s1));
        CHECK(s1 < s2);
        CHECK(!(s2 < s1));
        CHECK(s1 <= s2);
        CHECK(!(s2 <= s1));
        CHECK(s2 > s1);
        CHECK(!(s1 > s2));
        CHECK(s2 >= s1);
        CHECK(!(s1 >= s2));
    }

    {
        int arr1[] = {1, 2};
        int arr2[] = {1, 2};
        multi_span<int> s1 = arr1;
        multi_span<int> s2 = arr2;

        CHECK(s1 == s2);
        CHECK(!(s1 != s2));
        CHECK(!(s1 < s2));
        CHECK(s1 <= s2);
        CHECK(!(s1 > s2));
        CHECK(s1 >= s2);
        CHECK(s2 == s1);
        CHECK(!(s2 != s1));
        CHECK(!(s2 < s1));
        CHECK(s2 <= s1);
        CHECK(!(s2 > s1));
        CHECK(s2 >= s1);
    }

    {
        int arr[] = {1, 2, 3};

        multi_span<int> s1 = {&arr[0], 2}; // shorter
        multi_span<int> s2 = arr;          // longer

        CHECK(s1 != s2);
        CHECK(s2 != s1);
        CHECK(!(s1 == s2));
        CHECK(!(s2 == s1));
        CHECK(s1 < s2);
        CHECK(!(s2 < s1));
        CHECK(s1 <= s2);
        CHECK(!(s2 <= s1));
        CHECK(s2 > s1);
        CHECK(!(s1 > s2));
        CHECK(s2 >= s1);
        CHECK(!(s1 >= s2));
    }

    {
        int arr1[] = {1, 2}; // smaller
        int arr2[] = {2, 1}; // bigger

        multi_span<int> s1 = arr1;
        multi_span<int> s2 = arr2;

        CHECK(s1 != s2);
        CHECK(s2 != s1);
        CHECK(!(s1 == s2));
        CHECK(!(s2 == s1));
        CHECK(s1 < s2);
        CHECK(!(s2 < s1));
        CHECK(s1 <= s2);
        CHECK(!(s2 <= s1));
        CHECK(s2 > s1);
        CHECK(!(s1 > s2));
        CHECK(s2 >= s1);
        CHECK(!(s1 >= s2));
    }
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
GSL_SUPPRESS(i.11) // NO-FORMAT: attribute
GSL_SUPPRESS(r.11) // NO-FORMAT: attribute
TEST_CASE("basics")
{
    auto ptr = as_multi_span(new int[10], 10);
    fill(ptr.begin(), ptr.end(), 99);
    for (int num : ptr) { CHECK(num == 99); }

    delete[] ptr.data();
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.4) // NO-FORMAT: attribute
TEST_CASE("bounds_checks")
{
    int arr[10][2];
    auto av = as_multi_span(arr);

    fill(begin(av), end(av), 0);

    av[2][0] = 1;
    av[1][1] = 3;

    // out of bounds
    CHECK_THROWS_AS(av[1][3] = 3, fail_fast);
    CHECK_THROWS_AS((av[{1, 3}] = 3), fail_fast);

    CHECK_THROWS_AS(av[10][2], fail_fast);
    CHECK_THROWS_AS((av[{10, 2}]), fail_fast);

    CHECK_THROWS_AS(av[-1][0], fail_fast);
    CHECK_THROWS_AS((av[{-1, 0}]), fail_fast);

    CHECK_THROWS_AS(av[0][-1], fail_fast);
    CHECK_THROWS_AS((av[{0, -1}]), fail_fast);
}

void overloaded_func(multi_span<const int, dynamic_range, 3, 5> exp, int expected_value)
{
    for (auto val : exp) { CHECK(val == expected_value); }
}

void overloaded_func(multi_span<const char, dynamic_range, 3, 5> exp, char expected_value)
{
    for (auto val : exp) { CHECK(val == expected_value); }
}

void fixed_func(multi_span<int, 3, 3, 5> exp, int expected_value)
{
    for (auto val : exp) { CHECK(val == expected_value); }
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
GSL_SUPPRESS(r.11) // NO-FORMAT: attribute
GSL_SUPPRESS(r.3) // NO-FORMAT: attribute
GSL_SUPPRESS(r.5) // NO-FORMAT: attribute
GSL_SUPPRESS(r.5) // NO-FORMAT: attribute
TEST_CASE("span_parameter_test")
{
    auto data = new int[4][3][5];

    auto av = as_multi_span(data, 4);

    CHECK(av.size() == 60);

    fill(av.begin(), av.end(), 34);

    int count = 0;
    for_each(av.rbegin(), av.rend(), [&](int val) { count += val; });
    CHECK(count == 34 * 60);
    overloaded_func(av, 34);

    overloaded_func(as_multi_span(av, dim(4), dim(3), dim(5)), 34);

    // fixed_func(av, 34);
    delete[] data;
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.4)  // NO-FORMAT: attribute // false positive, checker does not recognize multi_span yet
GSL_SUPPRESS(r.11) // NO-FORMAT: attribute
GSL_SUPPRESS(r.3) // NO-FORMAT: attribute
TEST_CASE("md_access")
{
    auto width = 5, height = 20;

    auto imgSize = width * height;
    auto image_ptr = new int[narrow_cast<std::size_t>(imgSize)][3];

    // size check will be done
    auto image_view =
        as_multi_span(as_multi_span(image_ptr, imgSize), dim(height), dim(width), dim<3>());

    iota(image_view.begin(), image_view.end(), 1);

    int expected = 0;
    for (auto i = 0; i < height; i++)
    {
        for (auto j = 0; j < width; j++)
        {
            CHECK(expected + 1 == image_view[i][j][0]);
            CHECK(expected + 2 == image_view[i][j][1]);
            CHECK(expected + 3 == image_view[i][j][2]);

            auto val = image_view[{i, j, 0}];
            CHECK(expected + 1 == val);
            val = image_view[{i, j, 1}];
            CHECK(expected + 2 == val);
            val = image_view[{i, j, 2}];
            CHECK(expected + 3 == val);

            expected += 3;
        }
    }
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.3) // NO-FORMAT: attribute
GSL_SUPPRESS(i.11) // NO-FORMAT: attribute
GSL_SUPPRESS(r.11) // NO-FORMAT: attribute
GSL_SUPPRESS(r.3) // NO-FORMAT: attribute
TEST_CASE("as_multi_span")
{
    {
        int* arr = new int[150];

        auto av = as_multi_span(arr, dim<10>(), dim(3), dim<5>());

        fill(av.begin(), av.end(), 24);
        overloaded_func(av, 24);

        delete[] arr;

        array<int, 15> stdarr{0};
        auto av2 = as_multi_span(stdarr);
        overloaded_func(as_multi_span(av2, dim(1), dim<3>(), dim<5>()), 0);

        string str = "ttttttttttttttt"; // size = 15
        auto t = str.data();
        GSL_SUPPRESS(type.4) // NO-FORMAT: attribute // TODO: false positive
        (void) t;
        auto av3 = as_multi_span(str);
        overloaded_func(as_multi_span(av3, dim(1), dim<3>(), dim<5>()), 't');
    }

    {
        string str;
        multi_span<char> strspan = as_multi_span(str);
        (void) strspan;
        const string cstr;
        multi_span<const char> cstrspan = as_multi_span(cstr);
        (void) cstrspan;
    }

    {
        int a[3][4][5];
        auto av = as_multi_span(a);
        const int(*b)[4][5];
        b = a;
        auto bv = as_multi_span(b, 3);

        CHECK(av == bv);

        const std::array<double, 3> arr = {0.0, 0.0, 0.0};
        auto cv = as_multi_span(arr);
        (void) cv;

        vector<float> vec(3);
        auto dv = as_multi_span(vec);
        (void) dv;

    #ifdef CONFIRM_COMPILATION_ERRORS
        auto dv2 = as_multi_span(std::move(vec));
    #endif
    }
}

GSL_SUPPRESS(bounds.4) // NO-FORMAT: attribute
TEST_CASE("empty_spans")
{
    {
        multi_span<int, 0> empty_av(nullptr);

        CHECK(empty_av.bounds().index_bounds() == multi_span_index<1>{0});
        CHECK_THROWS_AS(empty_av[0], fail_fast);
        CHECK_THROWS_AS(empty_av.begin()[0], fail_fast);
        CHECK_THROWS_AS(empty_av.cbegin()[0], fail_fast);
        for (auto& v : empty_av)
        {
            (void) v;
            CHECK(false);
        }
    }

    {
        multi_span<int> empty_av = {};
        CHECK(empty_av.bounds().index_bounds() == multi_span_index<1>{0});
        CHECK_THROWS_AS(empty_av[0], fail_fast);
        CHECK_THROWS_AS(empty_av.begin()[0], fail_fast);
        CHECK_THROWS_AS(empty_av.cbegin()[0], fail_fast);
        for (auto& v : empty_av)
        {
            (void) v;
            CHECK(false);
        }
    }
}

GSL_SUPPRESS(con.4)    // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.1) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.4) // NO-FORMAT: attribute
GSL_SUPPRESS(r.3)      // NO-FORMAT: attribute
GSL_SUPPRESS(r.5)      // NO-FORMAT: attribute
GSL_SUPPRESS(r.11)     // NO-FORMAT: attribute
TEST_CASE("index_constructor")
{
    auto arr = new int[8];
    for (int i = 0; i < 4; ++i)
    {
        arr[2 * i] = 4 + i;
        arr[2 * i + 1] = i;
    }

    multi_span<int, dynamic_range> av(arr, 8);

    ptrdiff_t a[1] = {0};
    multi_span_index<1> i = a;

    CHECK(av[i] == 4);

    auto av2 = as_multi_span(av, dim<4>(), dim(2));
    ptrdiff_t a2[2] = {0, 1};
    multi_span_index<2> i2 = a2;

    CHECK(av2[i2] == 0);
    CHECK(av2[0][i] == 4);

    delete[] arr;
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("index_constructors")
{
    {
        // components of the same type
        multi_span_index<3> i1(0, 1, 2);
        CHECK(i1[0] == 0);

        // components of different types
        std::size_t c0 = 0;
        std::size_t c1 = 1;
        multi_span_index<3> i2(c0, c1, 2);
        CHECK(i2[0] == 0);

        // from array
        multi_span_index<3> i3 = {0, 1, 2};
        CHECK(i3[0] == 0);

        // from other index of the same size type
        multi_span_index<3> i4 = i3;
        CHECK(i4[0] == 0);

        // default
        multi_span_index<3> i7;
        CHECK(i7[0] == 0);

        // default
        multi_span_index<3> i9 = {};
        CHECK(i9[0] == 0);
    }

    {
        // components of the same type
        multi_span_index<1> i1(0);
        CHECK(i1[0] == 0);

        // components of different types
        std::size_t c0 = 0;
        multi_span_index<1> i2(c0);
        CHECK(i2[0] == 0);

        // from array
        multi_span_index<1> i3 = {0};
        CHECK(i3[0] == 0);

        // from int
        multi_span_index<1> i4 = 0;
        CHECK(i4[0] == 0);

        // from other index of the same size type
        multi_span_index<1> i5 = i3;
        CHECK(i5[0] == 0);

        // default
        multi_span_index<1> i8;
        CHECK(i8[0] == 0);

        // default
        multi_span_index<1> i9 = {};
        CHECK(i9[0] == 0);
    }

#ifdef CONFIRM_COMPILATION_ERRORS
    {
        multi_span_index<3> i1(0, 1);
        multi_span_index<3> i2(0, 1, 2, 3);
        multi_span_index<3> i3 = {0};
        multi_span_index<3> i4 = {0, 1, 2, 3};
        multi_span_index<1> i5 = {0, 1};
    }
#endif
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("index_operations")
{
    ptrdiff_t a[3] = {0, 1, 2};
    ptrdiff_t b[3] = {3, 4, 5};
    multi_span_index<3> i = a;
    multi_span_index<3> j = b;

    CHECK(i[0] == 0);
    CHECK(i[1] == 1);
    CHECK(i[2] == 2);

    {
        multi_span_index<3> k = i + j;

        CHECK(i[0] == 0);
        CHECK(i[1] == 1);
        CHECK(i[2] == 2);
        CHECK(k[0] == 3);
        CHECK(k[1] == 5);
        CHECK(k[2] == 7);
    }

    {
        multi_span_index<3> k = i * 3;

        CHECK(i[0] == 0);
        CHECK(i[1] == 1);
        CHECK(i[2] == 2);
        CHECK(k[0] == 0);
        CHECK(k[1] == 3);
        CHECK(k[2] == 6);
    }

    {
        multi_span_index<3> k = 3 * i;

        CHECK(i[0] == 0);
        CHECK(i[1] == 1);
        CHECK(i[2] == 2);
        CHECK(k[0] == 0);
        CHECK(k[1] == 3);
        CHECK(k[2] == 6);
    }

    {
        multi_span_index<2> k = details::shift_left(i);

        CHECK(i[0] == 0);
        CHECK(i[1] == 1);
        CHECK(i[2] == 2);
        CHECK(k[0] == 1);
        CHECK(k[1] == 2);
    }
}

GSL_SUPPRESS(bounds.4) // NO-FORMAT: attribute
GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
void iterate_second_column(multi_span<int, dynamic_range, dynamic_range> av)
{
    auto length = av.size() / 2;

    // view to the second column
    auto section = av.section({0, 1}, {length, 1});

    CHECK(section.size() == length);
    for (auto i = 0; i < section.size(); ++i) { CHECK(section[i][0] == av[i][1]); }

    for (auto i = 0; i < section.size(); ++i)
    {
        auto idx = multi_span_index<2>{i, 0}; // avoid braces inside the CHECK macro
        CHECK(section[idx] == av[i][1]);
    }

    CHECK(section.bounds().index_bounds()[0] == length);
    CHECK(section.bounds().index_bounds()[1] == 1);
    for (auto i = 0; i < section.bounds().index_bounds()[0]; ++i)
    {
        for (auto j = 0; j < section.bounds().index_bounds()[1]; ++j)
        {
            auto idx = multi_span_index<2>{i, j}; // avoid braces inside the CHECK macro
            CHECK(section[idx] == av[i][1]);
        }
    }

    auto check_sum = 0;
    for (auto i = 0; i < length; ++i) { check_sum += av[i][1]; }

    {
        auto idx = 0;
        auto sum = 0;
        for (auto num : section)
        {
            CHECK(num == av[idx][1]);
            sum += num;
            idx++;
        }

        CHECK(sum == check_sum);
    }
    {
        auto idx = length - 1;
        auto sum = 0;
        for (auto iter = section.rbegin(); iter != section.rend(); ++iter)
        {
            CHECK(*iter == av[idx][1]);
            sum += *iter;
            idx--;
        }

        CHECK(sum == check_sum);
    }
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("span_section_iteration")
{
    int arr[4][2] = {{4, 0}, {5, 1}, {6, 2}, {7, 3}};

    // static bounds
    {
        multi_span<int, 4, 2> av = arr;
        iterate_second_column(av);
    }
    // first bound is dynamic
    {
        multi_span<int, dynamic_range, 2> av = arr;
        iterate_second_column(av);
    }
    // second bound is dynamic
    {
        multi_span<int, 4, dynamic_range> av = arr;
        iterate_second_column(av);
    }
    // both bounds are dynamic
    {
        multi_span<int, dynamic_range, dynamic_range> av = arr;
        iterate_second_column(av);
    }
}

GSL_SUPPRESS(con.4)     // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.1)  // NO-FORMAT: attribute
GSL_SUPPRESS(r.3)       // NO-FORMAT: attribute
GSL_SUPPRESS(r.5)       // NO-FORMAT: attribute
GSL_SUPPRESS(r.11)      // NO-FORMAT: attribute
TEST_CASE("dynamic_span_section_iteration")
{
    auto height = 4, width = 2;
    auto size = height * width;

    auto arr = new int[narrow_cast<std::size_t>(size)];
    for (auto i = 0; i < size; ++i) { arr[i] = i; }

    auto av = as_multi_span(arr, size);

    // first bound is dynamic
    {
        multi_span<int, dynamic_range, 2> av2 = as_multi_span(av, dim(height), dim(width));
        iterate_second_column(av2);
    }
    // second bound is dynamic
    {
        multi_span<int, 4, dynamic_range> av2 = as_multi_span(av, dim(height), dim(width));
        iterate_second_column(av2);
    }
    // both bounds are dynamic
    {
        multi_span<int, dynamic_range, dynamic_range> av2 =
            as_multi_span(av, dim(height), dim(width));
        iterate_second_column(av2);
    }

    delete[] arr;
}

GSL_SUPPRESS(bounds.4) // NO-FORMAT: attribute
GSL_SUPPRESS(con.4)    // NO-FORMAT: attribute
GSL_SUPPRESS(r.11)     // NO-FORMAT: attribute
GSL_SUPPRESS(i.11)     // NO-FORMAT: attribute
TEST_CASE("span_structure_size")
{
    double(*arr)[3][4] = new double[100][3][4];
    multi_span<double, dynamic_range, 3, 4> av1(arr, 10);

    struct EffectiveStructure
    {
        double* v1;
        ptrdiff_t v2;
    };
    CHECK(sizeof(av1) == sizeof(EffectiveStructure));

    CHECK_THROWS_AS(av1[10][3][4], fail_fast);

    multi_span<const double, dynamic_range, 6, 4> av2 =
        as_multi_span(av1, dim(5), dim<6>(), dim<4>());
    (void) av2;
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("fixed_size_conversions")
{
    int arr[] = {1, 2, 3, 4};

    // converting to an multi_span from an equal size array is ok
    multi_span<int, 4> av4 = arr;
    CHECK(av4.length() == 4);

    // converting to dynamic_range a_v is always ok
    {
        multi_span<int, dynamic_range> av = av4;
        (void) av;
    }
    {
        multi_span<int, dynamic_range> av = arr;
        (void) av;
    }

// initialization or assignment to static multi_span that REDUCES size is NOT ok
#ifdef CONFIRM_COMPILATION_ERRORS
    {
        multi_span<int, 2> av2 = arr;
    }
    {
        multi_span<int, 2> av2 = av4;
    }
#endif

    {
        multi_span<int, dynamic_range> av = arr;
        multi_span<int, 2> av2 = av;
        (void) av2;
    }

#ifdef CONFIRM_COMPILATION_ERRORS
    {
        multi_span<int, dynamic_range> av = arr;
        multi_span<int, 2, 1> av2 = av.as_multi_span(dim<2>(), dim<2>());
    }
#endif

    {
        multi_span<int, dynamic_range> av = arr;
        multi_span<int, 2, 1> av2 = as_multi_span(av, dim(2), dim(2));
        auto workaround_macro = [&]() { return av2[{1, 0}] == 2; };
        CHECK(workaround_macro());
    }

    // but doing so explicitly is ok

    // you can convert statically
    {
        multi_span<int, 2> av2 = {arr, 2};
        (void) av2;
    }
    {
        multi_span<int, 1> av2 = av4.first<1>();
        (void) av2;
    }

    // ...or dynamically
    {
        // NB: implicit conversion to multi_span<int,2> from multi_span<int,dynamic_range>
        multi_span<int, 1> av2 = av4.first(1);
        (void) av2;
    }

    // initialization or assignment to static multi_span that requires size INCREASE is not ok.
    int arr2[2] = {1, 2};

#ifdef CONFIRM_COMPILATION_ERRORS
    {
        multi_span<int, 4> av4 = arr2;
    }
    {
        multi_span<int, 2> av2 = arr2;
        multi_span<int, 4> av4 = av2;
    }
#endif
    {
        auto f = [&]() {
            const multi_span<int, 4> av9 = {arr2, 2};
            (void) av9;
        };
        CHECK_THROWS_AS(f(), fail_fast);
    }

    // this should fail - we are trying to assign a small dynamic a_v to a fixed_size larger one
    multi_span<int, dynamic_range> av = arr2;
    auto f = [&]() {
        const multi_span<int, 4> av2 = av;
        (void) av2;
    };
    CHECK_THROWS_AS(f(), fail_fast);
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("as_writeable_bytes")
{
    int a[] = {1, 2, 3, 4};

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        // you should not be able to get writeable bytes for const objects
        multi_span<const int, dynamic_range> av = a;
        auto wav = av.as_writeable_bytes();
#endif
    }

    {
        multi_span<int, dynamic_range> av;
        auto wav = as_writeable_bytes(av);
        CHECK(wav.length() == av.length());
        CHECK(wav.length() == 0);
        CHECK(wav.size_bytes() == 0);
    }

    {
        multi_span<int, dynamic_range> av = a;
        auto wav = as_writeable_bytes(av);
        CHECK(wav.data() == reinterpret_cast<byte*>(&a[0]));
        CHECK(static_cast<std::size_t>(wav.length()) == sizeof(a));
    }
}


GSL_SUPPRESS(bounds.2) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.4) // NO-FORMAT: attribute
GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("iterator")
{
    int a[] = {1, 2, 3, 4};

    {
        multi_span<int, dynamic_range> av = a;
        auto wav = as_writeable_bytes(av);
        for (auto& b : wav) {
#if defined(__cplusplus) && (__cplusplus >= 201703L)
            b = byte{0};
#else
            GSL_SUPPRESS(es.49)
            b = byte(0);
#endif
        }

        for (std::size_t i = 0; i < 4; ++i) {
            CHECK(a[i] == 0);
        }
    }

    {
        multi_span<int, dynamic_range> av = a;
        for (auto& n : av) {
            n = 1;
        }
        for (std::size_t i = 0; i < 4; ++i) {
            CHECK(a[i] == 1);
        }
    }
}

#ifdef CONFIRM_COMPILATION_ERRORS
copy(src_span_static, dst_span_static);
#endif
