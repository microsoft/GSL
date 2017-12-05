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

#include <catch/catch.hpp> // for AssertionHandler, StringRef, CHECK, TEST_...

#include <gsl/gsl_byte> // for byte
#include <gsl/gsl_util> // for narrow_cast, at
#include <gsl/span>     // for span, span_iterator, operator==, operator!=

#include <array>       // for array
#include <iostream>    // for ptrdiff_t
#include <iterator>    // for reverse_iterator, operator-, operator==
#include <memory>      // for unique_ptr, shared_ptr, make_unique, allo...
#include <regex>       // for match_results, sub_match, match_results<>...
#include <stddef.h>    // for ptrdiff_t
#include <string>      // for string
#include <type_traits> // for integral_constant<>::value, is_default_co...
#include <vector>      // for vector

namespace gsl {
struct fail_fast;
}  // namespace gsl

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
}

TEST_CASE("default_constructor")
{
    {
        span<int> s;
        CHECK((s.length() == 0 && s.data() == nullptr));

        span<const int> cs;
        CHECK((cs.length() == 0 && cs.data() == nullptr));
    }

    {
        span<int, 0> s;
        CHECK((s.length() == 0 && s.data() == nullptr));

        span<const int, 0> cs;
        CHECK((cs.length() == 0 && cs.data() == nullptr));
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        span<int, 1> s;
        CHECK((s.length() == 1 && s.data() == nullptr)); // explains why it can't compile
#endif
    }

    {
        span<int> s{};
        CHECK((s.length() == 0 && s.data() == nullptr));

        span<const int> cs{};
        CHECK((cs.length() == 0 && cs.data() == nullptr));
    }
}

TEST_CASE("size_optimization")
{
    {
        span<int> s;
        CHECK(sizeof(s) == sizeof(int*) + sizeof(ptrdiff_t));
    }

    {
        span<int, 0> s;
        CHECK(sizeof(s) == sizeof(int*));
    }
}

TEST_CASE("from_nullptr_constructor")
{
    {
        span<int> s = nullptr;
        CHECK((s.length() == 0 && s.data() == nullptr));

        span<const int> cs = nullptr;
        CHECK((cs.length() == 0 && cs.data() == nullptr));
    }

    {
        span<int, 0> s = nullptr;
        CHECK((s.length() == 0 && s.data() == nullptr));

        span<const int, 0> cs = nullptr;
        CHECK((cs.length() == 0 && cs.data() == nullptr));
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        span<int, 1> s = nullptr;
        CHECK((s.length() == 1 && s.data() == nullptr)); // explains why it can't compile
#endif
    }

    {
        span<int> s{nullptr};
        CHECK((s.length() == 0 && s.data() == nullptr));

        span<const int> cs{nullptr};
        CHECK((cs.length() == 0 && cs.data() == nullptr));
    }

    {
        span<int*> s{nullptr};
        CHECK((s.length() == 0 && s.data() == nullptr));

        span<const int*> cs{nullptr};
        CHECK((cs.length() == 0 && cs.data() == nullptr));
    }
}

TEST_CASE("from_nullptr_length_constructor")
{
    {
        span<int> s{nullptr, static_cast<span<int>::index_type>(0)};
        CHECK((s.length() == 0 && s.data() == nullptr));

        span<const int> cs{nullptr, static_cast<span<int>::index_type>(0)};
        CHECK((cs.length() == 0 && cs.data() == nullptr));
    }

    {
        span<int, 0> s{nullptr, static_cast<span<int>::index_type>(0)};
        CHECK((s.length() == 0 && s.data() == nullptr));

        span<const int, 0> cs{nullptr, static_cast<span<int>::index_type>(0)};
        CHECK((cs.length() == 0 && cs.data() == nullptr));
    }

    {
        auto workaround_macro = []() {
            span<int, 1> s{nullptr, static_cast<span<int>::index_type>(0)};
        };
        CHECK_THROWS_AS(workaround_macro(), fail_fast);
    }

    {
        auto workaround_macro = []() { span<int> s{nullptr, 1}; };
        CHECK_THROWS_AS(workaround_macro(), fail_fast);

        auto const_workaround_macro = []() { span<const int> cs{nullptr, 1}; };
        CHECK_THROWS_AS(const_workaround_macro(), fail_fast);
    }

    {
        auto workaround_macro = []() { span<int, 0> s{nullptr, 1}; };
        CHECK_THROWS_AS(workaround_macro(), fail_fast);

        auto const_workaround_macro = []() { span<const int, 0> s{nullptr, 1}; };
        CHECK_THROWS_AS(const_workaround_macro(), fail_fast);
    }

    {
        span<int*> s{nullptr, static_cast<span<int>::index_type>(0)};
        CHECK((s.length() == 0 && s.data() == nullptr));

        span<const int*> cs{nullptr, static_cast<span<int>::index_type>(0)};
        CHECK((cs.length() == 0 && cs.data() == nullptr));
    }
}

TEST_CASE("from_pointer_length_constructor")
{
    int arr[4] = {1, 2, 3, 4};

    {
        for(int i = 0; i<4 ; ++i)
        {
            {
                span<int> s = { &arr[0], i };
                CHECK(s.length() == i);
                CHECK(s.data() == &arr[0]);
                CHECK(s.empty() == (i == 0));
                for (int j = 0; j < i; ++j)
                {
                    CHECK(arr[j] == s[j]);
                    CHECK(arr[j] == s.at(j));
                    CHECK(arr[j] == s(j));
                }
            }
            {
                span<int> s = { &arr[i], 4-i };
                CHECK(s.length() == 4-i);
                CHECK(s.data() == &arr[i]);
                CHECK(s.empty() == (4-i == 0));
                for (int j = 0; j < 4-i; ++j)
                {
                    CHECK(arr[j+i] == s[j]);
                    CHECK(arr[j+i] == s.at(j));
                    CHECK(arr[j+i] == s(j));
                }
            }
        }
    }

    {
        span<int, 2> s{&arr[0], 2};
        CHECK((s.length() == 2 && s.data() == &arr[0]));
        CHECK((s[0] == 1 && s[1] == 2));
    }

    {
        int* p = nullptr;
        span<int> s{p, static_cast<span<int>::index_type>(0)};
        CHECK((s.length() == 0 && s.data() == nullptr));
    }

    {
        int* p = nullptr;
        auto workaround_macro = [=]() { span<int> s{p, 2}; };
        CHECK_THROWS_AS(workaround_macro(), fail_fast);
    }

    {
        auto s = make_span(&arr[0], 2);
        CHECK((s.length() == 2 && s.data() == &arr[0]));
        CHECK((s[0] == 1 && s[1] == 2));
    }

    {
        int* p = nullptr;
        auto s = make_span(p, static_cast<span<int>::index_type>(0));
        CHECK((s.length() == 0 && s.data() == nullptr));
    }

    {
        int* p = nullptr;
        auto workaround_macro = [=]() { make_span(p, 2); };
        CHECK_THROWS_AS(workaround_macro(), fail_fast);
    }
}

TEST_CASE("from_pointer_pointer_constructor")
{
    int arr[4] = {1, 2, 3, 4};

    {
        span<int> s{&arr[0], &arr[2]};
        CHECK((s.length() == 2 && s.data() == &arr[0]));
        CHECK((s[0] == 1 && s[1] == 2));
    }

    {
        span<int, 2> s{&arr[0], &arr[2]};
        CHECK((s.length() == 2 && s.data() == &arr[0]));
        CHECK((s[0] == 1 && s[1] == 2));
    }

    {
        span<int> s{&arr[0], &arr[0]};
        CHECK((s.length() == 0 && s.data() == &arr[0]));
    }

    {
        span<int, 0> s{&arr[0], &arr[0]};
        CHECK((s.length() == 0 && s.data() == &arr[0]));
    }

    // this will fail the std::distance() precondition, which asserts on MSVC debug builds
    //{
    //    auto workaround_macro = [&]() { span<int> s{&arr[1], &arr[0]}; };
    //    CHECK_THROWS_AS(workaround_macro(), fail_fast);
    //}

    // this will fail the std::distance() precondition, which asserts on MSVC debug builds
    //{
    //    int* p = nullptr;
    //    auto workaround_macro = [&]() { span<int> s{&arr[0], p}; };
    //    CHECK_THROWS_AS(workaround_macro(), fail_fast);
    //}

    {
        int* p = nullptr;
        span<int> s{p, p};
        CHECK((s.length() == 0 && s.data() == nullptr));
    }

    {
        int* p = nullptr;
        span<int, 0> s{p, p};
        CHECK((s.length() == 0 && s.data() == nullptr));
    }

    // this will fail the std::distance() precondition, which asserts on MSVC debug builds
    //{
    //    int* p = nullptr;
    //    auto workaround_macro = [&]() { span<int> s{&arr[0], p}; };
    //    CHECK_THROWS_AS(workaround_macro(), fail_fast);
    //}

    {
        auto s = make_span(&arr[0], &arr[2]);
        CHECK((s.length() == 2 && s.data() == &arr[0]));
        CHECK((s[0] == 1 && s[1] == 2));
    }

    {
        auto s = make_span(&arr[0], &arr[0]);
        CHECK((s.length() == 0 && s.data() == &arr[0]));
    }

    {
        int* p = nullptr;
        auto s = make_span(p, p);
        CHECK((s.length() == 0 && s.data() == nullptr));
    }
}

TEST_CASE("from_array_constructor")
{
    int arr[5] = {1, 2, 3, 4, 5};

    {
        span<int> s{arr};
        CHECK((s.length() == 5 && s.data() == &arr[0]));
    }

    {
        span<int, 5> s{arr};
        CHECK((s.length() == 5 && s.data() == &arr[0]));
    }

    int arr2d[2][3] = {1, 2, 3, 4, 5, 6};

#ifdef CONFIRM_COMPILATION_ERRORS
    {
        span<int, 6> s{arr};
    }

    {
        span<int, 0> s{arr};
        CHECK((s.length() == 0 && s.data() == &arr[0]));
    }

    {
        span<int> s{arr2d};
        CHECK((s.length() == 6 && s.data() == &arr2d[0][0]));
        CHECK((s[0] == 1 && s[5] == 6));
    }

    {
        span<int, 0> s{arr2d};
        CHECK((s.length() == 0 && s.data() == &arr2d[0][0]));
    }

    {
        span<int, 6> s{arr2d};
    }
#endif
    {
        span<int[3]> s{&(arr2d[0]), 1};
        CHECK((s.length() == 1 && s.data() == &arr2d[0]));
    }

    int arr3d[2][3][2] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

#ifdef CONFIRM_COMPILATION_ERRORS
    {
        span<int> s{arr3d};
        CHECK((s.length() == 12 && s.data() == &arr3d[0][0][0]));
        CHECK((s[0] == 1 && s[11] == 12));
    }

    {
        span<int, 0> s{arr3d};
        CHECK((s.length() == 0 && s.data() == &arr3d[0][0][0]));
    }

    {
        span<int, 11> s{arr3d};
    }

    {
        span<int, 12> s{arr3d};
        CHECK((s.length() == 12 && s.data() == &arr3d[0][0][0]));
        CHECK((s[0] == 1 && s[5] == 6));
    }
#endif
    {
        span<int[3][2]> s{&arr3d[0], 1};
        CHECK((s.length() == 1 && s.data() == &arr3d[0]));
    }

    {
        auto s = make_span(arr);
        CHECK((s.length() == 5 && s.data() == &arr[0]));
    }

    {
        auto s = make_span(&(arr2d[0]), 1);
        CHECK((s.length() == 1 && s.data() == &arr2d[0]));
    }

    {
        auto s = make_span(&arr3d[0], 1);
        CHECK((s.length() == 1 && s.data() == &arr3d[0]));
    }
}

TEST_CASE("from_dynamic_array_constructor")
{
    double(*arr)[3][4] = new double[100][3][4];

    {
        span<double> s(&arr[0][0][0], 10);
        CHECK((s.length() == 10 && s.data() == &arr[0][0][0]));
    }

    {
        auto s = make_span(&arr[0][0][0], 10);
        CHECK((s.length() == 10 && s.data() == &arr[0][0][0]));
    }

    delete[] arr;
}

TEST_CASE("from_std_array_constructor")
{
    std::array<int, 4> arr = {1, 2, 3, 4};

    {
        span<int> s{arr};
        CHECK((s.size() == narrow_cast<ptrdiff_t>(arr.size()) && s.data() == arr.data()));

        span<const int> cs{arr};
        CHECK((cs.size() == narrow_cast<ptrdiff_t>(arr.size()) && cs.data() == arr.data()));
    }

    {
        span<int, 4> s{arr};
        CHECK((s.size() == narrow_cast<ptrdiff_t>(arr.size()) && s.data() == arr.data()));

        span<const int, 4> cs{arr};
        CHECK((cs.size() == narrow_cast<ptrdiff_t>(arr.size()) && cs.data() == arr.data()));
    }

#ifdef CONFIRM_COMPILATION_ERRORS
    {
        span<int, 2> s{arr};
        CHECK((s.size() == 2 && s.data() == arr.data()));

        span<const int, 2> cs{arr};
        CHECK((cs.size() == 2 && cs.data() == arr.data()));
    }

    {
        span<int, 0> s{arr};
        CHECK((s.size() == 0 && s.data() == arr.data()));

        span<const int, 0> cs{arr};
        CHECK((cs.size() == 0 && cs.data() == arr.data()));
    }

    {
        span<int, 5> s{arr};
    }

    {
        auto get_an_array = []() -> std::array<int, 4> { return {1, 2, 3, 4}; };
        auto take_a_span = [](span<int> s) { static_cast<void>(s); };
        // try to take a temporary std::array
        take_a_span(get_an_array());
    }
#endif

    {
        auto get_an_array = []() -> std::array<int, 4> { return {1, 2, 3, 4}; };
        auto take_a_span = [](span<const int> s) { static_cast<void>(s); };
        // try to take a temporary std::array
        take_a_span(get_an_array());
    }

    {
        auto s = make_span(arr);
        CHECK((s.size() == narrow_cast<ptrdiff_t>(arr.size()) && s.data() == arr.data()));
    }
}

TEST_CASE("from_const_std_array_constructor")
{
    const std::array<int, 4> arr = {1, 2, 3, 4};

    {
        span<const int> s{arr};
        CHECK((s.size() == narrow_cast<ptrdiff_t>(arr.size()) && s.data() == arr.data()));
    }

    {
        span<const int, 4> s{arr};
        CHECK((s.size() == narrow_cast<ptrdiff_t>(arr.size()) && s.data() == arr.data()));
    }

#ifdef CONFIRM_COMPILATION_ERRORS
    {
        span<const int, 2> s{arr};
        CHECK((s.size() == 2 && s.data() == arr.data()));
    }

    {
        span<const int, 0> s{arr};
        CHECK((s.size() == 0 && s.data() == arr.data()));
    }

    {
        span<const int, 5> s{arr};
    }
#endif

    {
        auto get_an_array = []() -> const std::array<int, 4> { return {1, 2, 3, 4}; };
        auto take_a_span = [](span<const int> s) { static_cast<void>(s); };
        // try to take a temporary std::array
        take_a_span(get_an_array());
    }

    {
        auto s = make_span(arr);
        CHECK((s.size() == narrow_cast<ptrdiff_t>(arr.size()) && s.data() == arr.data()));
    }
}

TEST_CASE("from_std_array_const_constructor")
{
    std::array<const int, 4> arr = {1, 2, 3, 4};

    {
        span<const int> s{arr};
        CHECK((s.size() == narrow_cast<ptrdiff_t>(arr.size()) && s.data() == arr.data()));
    }

    {
        span<const int, 4> s{arr};
        CHECK((s.size() == narrow_cast<ptrdiff_t>(arr.size()) && s.data() == arr.data()));
    }

#ifdef CONFIRM_COMPILATION_ERRORS
    {
        span<const int, 2> s{arr};
        CHECK((s.size() == 2 && s.data() == arr.data()));
    }

    {
        span<const int, 0> s{arr};
        CHECK((s.size() == 0 && s.data() == arr.data()));
    }

    {
        span<const int, 5> s{arr};
    }

    {
        span<int, 4> s{arr};
    }
#endif

    {
        auto s = make_span(arr);
        CHECK((s.size() == narrow_cast<ptrdiff_t>(arr.size()) && s.data() == arr.data()));
    }
}

TEST_CASE("from_unique_pointer_construction")
{
    {
        auto ptr = std::make_unique<int>(4);

        {
            span<int> s{ptr};
            CHECK((s.length() == 1 && s.data() == ptr.get()));
            CHECK(s[0] == 4);
        }

        {
            auto s = make_span(ptr);
            CHECK((s.length() == 1 && s.data() == ptr.get()));
            CHECK(s[0] == 4);
        }
    }

    {
        auto ptr = std::unique_ptr<int>{nullptr};

        {
            span<int> s{ptr};
            CHECK((s.length() == 0 && s.data() == nullptr));
        }

        {
            auto s = make_span(ptr);
            CHECK((s.length() == 0 && s.data() == nullptr));
        }
    }

    {
        auto arr = std::make_unique<int[]>(4);

        for (auto i = 0U; i < 4; i++) arr[i] = gsl::narrow_cast<int>(i + 1);

        {
            span<int> s{arr, 4};
            CHECK((s.length() == 4 && s.data() == arr.get()));
            CHECK((s[0] == 1 && s[1] == 2));
        }

        {
            auto s = make_span(arr, 4);
            CHECK((s.length() == 4 && s.data() == arr.get()));
            CHECK((s[0] == 1 && s[1] == 2));
        }
    }

    {
        auto arr = std::unique_ptr<int[]>{nullptr};

        {
            span<int> s{arr, 0};
            CHECK((s.length() == 0 && s.data() == nullptr));
        }

        {
            auto s = make_span(arr, 0);
            CHECK((s.length() == 0 && s.data() == nullptr));
        }
    }
}

TEST_CASE("from_shared_pointer_construction")
{
    {
        auto ptr = std::make_shared<int>(4);

        {
            span<int> s{ptr};
            CHECK((s.length() == 1 && s.data() == ptr.get()));
            CHECK((s[0] == 4));
        }

        {
            auto s = make_span(ptr);
            CHECK((s.length() == 1 && s.data() == ptr.get()));
            CHECK((s[0] == 4));
        }
    }

    {
        auto ptr = std::shared_ptr<int>{nullptr};

        {
            span<int> s{ptr};
            CHECK((s.length() == 0 && s.data() == nullptr));
        }

        {
            auto s = make_span(ptr);
            CHECK((s.length() == 0 && s.data() == nullptr));
        }
    }
}

TEST_CASE("from_container_constructor")
{
    std::vector<int> v = {1, 2, 3};
    const std::vector<int> cv = v;

    {
        span<int> s{v};
        CHECK((s.size() == narrow_cast<std::ptrdiff_t>(v.size()) && s.data() == v.data()));

        span<const int> cs{v};
        CHECK((cs.size() == narrow_cast<std::ptrdiff_t>(v.size()) && cs.data() == v.data()));
    }

    std::string str = "hello";
    const std::string cstr = "hello";

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        span<char> s{str};
        CHECK((s.size() == narrow_cast<std::ptrdiff_t>(str.size()) && s.data() == str.data()));
#endif
        span<const char> cs{str};
        CHECK((cs.size() == narrow_cast<std::ptrdiff_t>(str.size()) && cs.data() == str.data()));
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        span<char> s{cstr};
#endif
        span<const char> cs{cstr};
        CHECK((cs.size() == narrow_cast<std::ptrdiff_t>(cstr.size()) &&
              cs.data() == cstr.data()));
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        auto get_temp_vector = []() -> std::vector<int> { return {}; };
        auto use_span = [](span<int> s) { static_cast<void>(s); };
        use_span(get_temp_vector());
#endif
    }

    {
        auto get_temp_vector = []() -> std::vector<int> { return {}; };
        auto use_span = [](span<const int> s) { static_cast<void>(s); };
        use_span(get_temp_vector());
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        auto get_temp_string = []() -> std::string { return {}; };
        auto use_span = [](span<char> s) { static_cast<void>(s); };
        use_span(get_temp_string());
#endif
    }

    {
        auto get_temp_string = []() -> std::string { return {}; };
        auto use_span = [](span<const char> s) { static_cast<void>(s); };
        use_span(get_temp_string());
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        auto get_temp_vector = []() -> const std::vector<int> { return {}; };
        auto use_span = [](span<const char> s) { static_cast<void>(s); };
        use_span(get_temp_vector());
#endif
    }

    {
        auto get_temp_string = []() -> const std::string { return {}; };
        auto use_span = [](span<const char> s) { static_cast<void>(s); };
        use_span(get_temp_string());
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        std::map<int, int> m;
        span<int> s{m};
#endif
    }

    {
        auto s = make_span(v);
        CHECK((s.size() == narrow_cast<std::ptrdiff_t>(v.size()) && s.data() == v.data()));

        auto cs = make_span(cv);
        CHECK((cs.size() == narrow_cast<std::ptrdiff_t>(cv.size()) && cs.data() == cv.data()));
    }
}

TEST_CASE("from_convertible_span_constructor")
{
    {
        span<DerivedClass> avd;
        span<const DerivedClass> avcd = avd;
        static_cast<void>(avcd);
    }

    {
    #ifdef CONFIRM_COMPILATION_ERRORS
        span<DerivedClass> avd;
        span<BaseClass> avb = avd;
        static_cast<void>(avb);
    #endif
    }

    #ifdef CONFIRM_COMPILATION_ERRORS
    {
        span<int> s;
        span<unsigned int> s2 = s;
        static_cast<void>(s2);
    }

    {
        span<int> s;
        span<const unsigned int> s2 = s;
        static_cast<void>(s2);
    }

    {
        span<int> s;
        span<short> s2 = s;
        static_cast<void>(s2);
    }
    #endif
}

TEST_CASE("copy_move_and_assignment")
{
    span<int> s1;
    CHECK(s1.empty());

    int arr[] = {3, 4, 5};

    span<const int> s2 = arr;
    CHECK((s2.length() == 3 && s2.data() == &arr[0]));

    s2 = s1;
    CHECK(s2.empty());

    auto get_temp_span = [&]() -> span<int> { return {&arr[1], 2}; };
    auto use_span = [&](span<const int> s) { CHECK((s.length() == 2 && s.data() == &arr[1])); };
    use_span(get_temp_span());

    s1 = get_temp_span();
    CHECK((s1.length() == 2 && s1.data() == &arr[1]));
}

TEST_CASE("first")
{
    int arr[5] = {1, 2, 3, 4, 5};

    {
        span<int, 5> av = arr;
        CHECK(av.first<2>().length() == 2);
        CHECK(av.first(2).length() == 2);
    }

    {
        span<int, 5> av = arr;
        CHECK(av.first<0>().length() == 0);
        CHECK(av.first(0).length() == 0);
    }

    {
        span<int, 5> av = arr;
        CHECK(av.first<5>().length() == 5);
        CHECK(av.first(5).length() == 5);
    }

    {
        span<int, 5> av = arr;
#ifdef CONFIRM_COMPILATION_ERRORS
        CHECK(av.first<6>().length() == 6);
        CHECK(av.first<-1>().length() == -1);
#endif
        CHECK_THROWS_AS(av.first(6).length(), fail_fast);
    }

    {
        span<int> av;
        CHECK(av.first<0>().length() == 0);
        CHECK(av.first(0).length() == 0);
    }
}

TEST_CASE("last")
{
    int arr[5] = {1, 2, 3, 4, 5};

    {
        span<int, 5> av = arr;
        CHECK(av.last<2>().length() == 2);
        CHECK(av.last(2).length() == 2);
    }

    {
        span<int, 5> av = arr;
        CHECK(av.last<0>().length() == 0);
        CHECK(av.last(0).length() == 0);
    }

    {
        span<int, 5> av = arr;
        CHECK(av.last<5>().length() == 5);
        CHECK(av.last(5).length() == 5);
    }

    {
        span<int, 5> av = arr;
#ifdef CONFIRM_COMPILATION_ERRORS
        CHECK(av.last<6>().length() == 6);
#endif
        CHECK_THROWS_AS(av.last(6).length(), fail_fast);
    }

    {
        span<int> av;
        CHECK(av.last<0>().length() == 0);
        CHECK(av.last(0).length() == 0);
    }
}

TEST_CASE("subspan")
{
    int arr[5] = {1, 2, 3, 4, 5};

    {
        span<int, 5> av = arr;
        CHECK((av.subspan<2, 2>().length() == 2));
        CHECK(av.subspan(2, 2).length() == 2);
        CHECK(av.subspan(2, 3).length() == 3);
    }

    {
        span<int, 5> av = arr;
        CHECK((av.subspan<0, 0>().length() == 0));
        CHECK(av.subspan(0, 0).length() == 0);
    }

    {
        span<int, 5> av = arr;
        CHECK((av.subspan<0, 5>().length() == 5));
        CHECK(av.subspan(0, 5).length() == 5);
        CHECK_THROWS_AS(av.subspan(0, 6).length(), fail_fast);
        CHECK_THROWS_AS(av.subspan(1, 5).length(), fail_fast);
    }

    {
        span<int, 5> av = arr;
        CHECK((av.subspan<4, 0>().length() == 0));
        CHECK(av.subspan(4, 0).length() == 0);
        CHECK(av.subspan(5, 0).length() == 0);
        CHECK_THROWS_AS(av.subspan(6, 0).length(), fail_fast);
    }

    {
        span<int> av;
        CHECK((av.subspan<0, 0>().length() == 0));
        CHECK(av.subspan(0, 0).length() == 0);
        CHECK_THROWS_AS((av.subspan<1, 0>().length()), fail_fast);
    }

    {
        span<int> av;
        CHECK(av.subspan(0).length() == 0);
        CHECK_THROWS_AS(av.subspan(1).length(), fail_fast);
    }

    {
        span<int> av = arr;
        CHECK(av.subspan(0).length() == 5);
        CHECK(av.subspan(1).length() == 4);
        CHECK(av.subspan(4).length() == 1);
        CHECK(av.subspan(5).length() == 0);
        CHECK_THROWS_AS(av.subspan(6).length(), fail_fast);
        const auto av2 = av.subspan(1);
        for (int i = 0; i < 4; ++i) CHECK(av2[i] == i + 2);
    }

    {
        span<int, 5> av = arr;
        CHECK(av.subspan(0).length() == 5);
        CHECK(av.subspan(1).length() == 4);
        CHECK(av.subspan(4).length() == 1);
        CHECK(av.subspan(5).length() == 0);
        CHECK_THROWS_AS(av.subspan(6).length(), fail_fast);
        const auto av2 = av.subspan(1);
        for (int i = 0; i < 4; ++i) CHECK(av2[i] == i + 2);
    }
}

TEST_CASE("at_call")
{
    int arr[4] = {1, 2, 3, 4};

    {
        span<int> s = arr;
        CHECK(s.at(0) == 1);
        CHECK_THROWS_AS(s.at(5), fail_fast);
    }

    {
        int arr2d[2] = {1, 6};
        span<int, 2> s = arr2d;
        CHECK(s.at(0) == 1);
        CHECK(s.at(1) == 6);
        CHECK_THROWS_AS(s.at(2), fail_fast);
    }
}

TEST_CASE("operator_function_call")
{
    int arr[4] = {1, 2, 3, 4};

    {
        span<int> s = arr;
        CHECK(s(0) == 1);
        CHECK_THROWS_AS(s(5), fail_fast);
    }

    {
        int arr2d[2] = {1, 6};
        span<int, 2> s = arr2d;
        CHECK(s(0) == 1);
        CHECK(s(1) == 6);
        CHECK_THROWS_AS(s(2), fail_fast);
    }
}

TEST_CASE("iterator_default_init")
{
    span<int>::iterator it1;
    span<int>::iterator it2;
    CHECK(it1 == it2);
}

TEST_CASE("const_iterator_default_init")
{
    span<int>::const_iterator it1;
    span<int>::const_iterator it2;
    CHECK(it1 == it2);
}

TEST_CASE("iterator_conversions")
{
    span<int>::iterator badIt;
    span<int>::const_iterator badConstIt;
    CHECK(badIt == badConstIt);

    int a[] = {1, 2, 3, 4};
    span<int> s = a;

    auto it = s.begin();
    auto cit = s.cbegin();

    CHECK(it == cit);
    CHECK(cit == it);

    span<int>::const_iterator cit2 = it;
    CHECK(cit2 == cit);

    span<int>::const_iterator cit3 = it + 4;
    CHECK(cit3 == s.cend());
}

TEST_CASE("iterator_comparisons")
{
    int a[] = {1, 2, 3, 4};
    {
        span<int> s = a;
        span<int>::iterator it = s.begin();
        auto it2 = it + 1;
        span<int>::const_iterator cit = s.cbegin();

        CHECK(it == cit);
        CHECK(cit == it);
        CHECK(it == it);
        CHECK(cit == cit);
        CHECK(cit == s.begin());
        CHECK(s.begin() == cit);
        CHECK(s.cbegin() == cit);
        CHECK(it == s.begin());
        CHECK(s.begin() == it);

        CHECK(it != it2);
        CHECK(it2 != it);
        CHECK(it != s.end());
        CHECK(it2 != s.end());
        CHECK(s.end() != it);
        CHECK(it2 != cit);
        CHECK(cit != it2);

        CHECK(it < it2);
        CHECK(it <= it2);
        CHECK(it2 <= s.end());
        CHECK(it < s.end());
        CHECK(it <= cit);
        CHECK(cit <= it);
        CHECK(cit < it2);
        CHECK(cit <= it2);
        CHECK(cit < s.end());
        CHECK(cit <= s.end());

        CHECK(it2 > it);
        CHECK(it2 >= it);
        CHECK(s.end() > it2);
        CHECK(s.end() >= it2);
        CHECK(it2 > cit);
        CHECK(it2 >= cit);
    }
}

TEST_CASE("begin_end")
{
    {
        int a[] = {1, 2, 3, 4};
        span<int> s = a;

        span<int>::iterator it = s.begin();
        span<int>::iterator it2 = std::begin(s);
        CHECK(it == it2);

        it = s.end();
        it2 = std::end(s);
        CHECK(it == it2);
    }

    {
        int a[] = {1, 2, 3, 4};
        span<int> s = a;

        auto it = s.begin();
        auto first = it;
        CHECK(it == first);
        CHECK(*it == 1);

        auto beyond = s.end();
        CHECK(it != beyond);
        CHECK_THROWS_AS(*beyond, fail_fast);

        CHECK(beyond - first == 4);
        CHECK(first - first == 0);
        CHECK(beyond - beyond == 0);

        ++it;
        CHECK(it - first == 1);
        CHECK(*it == 2);
        *it = 22;
        CHECK(*it == 22);
        CHECK(beyond - it == 3);

        it = first;
        CHECK(it == first);
        while (it != s.end()) {
            *it = 5;
            ++it;
        }

        CHECK(it == beyond);
        CHECK(it - beyond == 0);

        for (const auto& n : s) {
            CHECK(n == 5);
        }
    }
}

TEST_CASE("cbegin_cend")
{
    {
        int a[] = {1, 2, 3, 4};
        span<int> s = a;

        span<int>::const_iterator cit = s.cbegin();
        span<int>::const_iterator cit2 = std::cbegin(s);
        CHECK(cit == cit2);

        cit = s.cend();
        cit2 = std::cend(s);
        CHECK(cit == cit2);
    }

    {
        int a[] = {1, 2, 3, 4};
        span<int> s = a;

        auto it = s.cbegin();
        auto first = it;
        CHECK(it == first);
        CHECK(*it == 1);

        auto beyond = s.cend();
        CHECK(it != beyond);
        CHECK_THROWS_AS(*beyond, fail_fast);

        CHECK(beyond - first == 4);
        CHECK(first - first == 0);
        CHECK(beyond - beyond == 0);

        ++it;
        CHECK(it - first == 1);
        CHECK(*it == 2);
        CHECK(beyond - it == 3);

        int last = 0;
        it = first;
        CHECK(it == first);
        while (it != s.cend()) {
            CHECK(*it == last + 1);

            last = *it;
            ++it;
        }

        CHECK(it == beyond);
        CHECK(it - beyond == 0);
    }
}

TEST_CASE("rbegin_rend")
{
    {
        int a[] = {1, 2, 3, 4};
        span<int> s = a;

        auto it = s.rbegin();
        auto first = it;
        CHECK(it == first);
        CHECK(*it == 4);

        auto beyond = s.rend();
        CHECK(it != beyond);
        CHECK_THROWS_AS(*beyond, fail_fast);

        CHECK(beyond - first == 4);
        CHECK(first - first == 0);
        CHECK(beyond - beyond == 0);

        ++it;
        CHECK(it - first == 1);
        CHECK(*it == 3);
        *it = 22;
        CHECK(*it == 22);
        CHECK(beyond - it == 3);

        it = first;
        CHECK(it == first);
        while (it != s.rend()) {
            *it = 5;
            ++it;
        }

        CHECK(it == beyond);
        CHECK(it - beyond == 0);

        for (const auto& n : s) {
            CHECK(n == 5);
        }
    }
}

TEST_CASE("crbegin_crend")
{
    {
        int a[] = {1, 2, 3, 4};
        span<int> s = a;

        auto it = s.crbegin();
        auto first = it;
        CHECK(it == first);
        CHECK(*it == 4);

        auto beyond = s.crend();
        CHECK(it != beyond);
        CHECK_THROWS_AS(*beyond, fail_fast);

        CHECK(beyond - first == 4);
        CHECK(first - first == 0);
        CHECK(beyond - beyond == 0);

        ++it;
        CHECK(it - first == 1);
        CHECK(*it == 3);
        CHECK(beyond - it == 3);

        it = first;
        CHECK(it == first);
        int last = 5;
        while (it != s.crend()) {
            CHECK(*it == last - 1);
            last = *it;

            ++it;
        }

        CHECK(it == beyond);
        CHECK(it - beyond == 0);
    }
}

TEST_CASE("comparison_operators")
{
    {
        span<int> s1 = nullptr;
        span<int> s2 = nullptr;
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
        int arr[] = {2, 1};
        span<int> s1 = arr;
        span<int> s2 = arr;

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

        span<int> s1 = nullptr;
        span<int> s2 = arr;

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
        span<int> s1 = arr1;
        span<int> s2 = arr2;

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

        span<int> s1 = {&arr[0], 2}; // shorter
        span<int> s2 = arr;          // longer

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

        span<int> s1 = arr1;
        span<int> s2 = arr2;

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

TEST_CASE("as_bytes")
{
    int a[] = {1, 2, 3, 4};

    {
        const span<const int> s = a;
        CHECK(s.length() == 4);
        const span<const byte> bs = as_bytes(s);
        CHECK(static_cast<const void*>(bs.data()) == static_cast<const void*>(s.data()));
        CHECK(bs.length() == s.length_bytes());
    }

    {
        span<int> s;
        const auto bs = as_bytes(s);
        CHECK(bs.length() == s.length());
        CHECK(bs.length() == 0);
        CHECK(bs.size_bytes() == 0);
        CHECK(static_cast<const void*>(bs.data()) == static_cast<const void*>(s.data()));
        CHECK(bs.data() == nullptr);
    }

    {
        span<int> s = a;
        const auto bs = as_bytes(s);
        CHECK(static_cast<const void*>(bs.data()) == static_cast<const void*>(s.data()));
        CHECK(bs.length() == s.length_bytes());
    }
}

TEST_CASE("as_writeable_bytes")
{
    int a[] = {1, 2, 3, 4};

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        // you should not be able to get writeable bytes for const objects
        span<const int> s = a;
        CHECK(s.length() == 4);
        span<const byte> bs = as_writeable_bytes(s);
        CHECK(static_cast<void*>(bs.data()) == static_cast<void*>(s.data()));
        CHECK(bs.length() == s.length_bytes());
#endif
    }

    {
        span<int> s;
        const auto bs = as_writeable_bytes(s);
        CHECK(bs.length() == s.length());
        CHECK(bs.length() == 0);
        CHECK(bs.size_bytes() == 0);
        CHECK(static_cast<void*>(bs.data()) == static_cast<void*>(s.data()));
        CHECK(bs.data() == nullptr);
    }

    {
        span<int> s = a;
        const auto bs = as_writeable_bytes(s);
        CHECK(static_cast<void*>(bs.data()) == static_cast<void*>(s.data()));
        CHECK(bs.length() == s.length_bytes());
    }
}

TEST_CASE("fixed_size_conversions")
{
    int arr[] = {1, 2, 3, 4};

    // converting to an span from an equal size array is ok
    span<int, 4> s4 = arr;
    CHECK(s4.length() == 4);

    // converting to dynamic_range is always ok
    {
        span<int> s = s4;
        CHECK(s.length() == s4.length());
        static_cast<void>(s);
    }

// initialization or assignment to static span that REDUCES size is NOT ok
#ifdef CONFIRM_COMPILATION_ERRORS
    {
        span<int, 2> s = arr;
    }
    {
        span<int, 2> s2 = s4;
        static_cast<void>(s2);
    }
#endif

    // even when done dynamically
    {
        span<int> s = arr;
        auto f = [&]() {
            span<int, 2> s2 = s;
            static_cast<void>(s2);
        };
        CHECK_THROWS_AS(f(), fail_fast);
    }

    // but doing so explicitly is ok

    // you can convert statically
    {
        const span<int, 2> s2 = {arr, 2};
        static_cast<void>(s2);
    }
    {
        const span<int, 1> s1 = s4.first<1>();
        static_cast<void>(s1);
    }

    // ...or dynamically
    {
        // NB: implicit conversion to span<int,1> from span<int>
        span<int, 1> s1 = s4.first(1);
        static_cast<void>(s1);
    }

    // initialization or assignment to static span that requires size INCREASE is not ok.
    int arr2[2] = {1, 2};

#ifdef CONFIRM_COMPILATION_ERRORS
    {
        span<int, 4> s3 = arr2;
    }
    {
        span<int, 2> s2 = arr2;
        span<int, 4> s4a = s2;
    }
#endif
    {
        auto f = [&]() {
            span<int, 4> _s4 = {arr2, 2};
            static_cast<void>(_s4);
        };
        CHECK_THROWS_AS(f(), fail_fast);
    }

    // this should fail - we are trying to assign a small dynamic span to a fixed_size larger one
    span<int> av = arr2;
    auto f = [&]() {
        span<int, 4> _s4 = av;
        static_cast<void>(_s4);
    };
    CHECK_THROWS_AS(f(), fail_fast);
}

TEST_CASE("interop_with_std_regex")
{
    char lat[] = {'1', '2', '3', '4', '5', '6', 'E', 'F', 'G'};
    span<char> s = lat;
    const auto f_it = s.begin() + 7;

    std::match_results<span<char>::iterator> match;

    std::regex_match(s.begin(), s.end(), match, std::regex(".*"));
    CHECK(match.ready());
    CHECK(!match.empty());
    CHECK(match[0].matched);
    CHECK(match[0].first == s.begin());
    CHECK(match[0].second == s.end());

    std::regex_search(s.begin(), s.end(), match, std::regex("F"));
    CHECK(match.ready());
    CHECK(!match.empty());
    CHECK(match[0].matched);
    CHECK(match[0].first == f_it);
    CHECK(match[0].second == (f_it + 1));
}

TEST_CASE("interop_with_gsl_at")
{
    int arr[5] = {1, 2, 3, 4, 5};
    span<int> s{arr};
    CHECK((at(s, 0) == 1 && at(s, 1) == 2));
}

TEST_CASE("default_constructible")
{
    CHECK((std::is_default_constructible<span<int>>::value));
    CHECK((std::is_default_constructible<span<int, 0>>::value));
    CHECK((!std::is_default_constructible<span<int, 42>>::value));
}
