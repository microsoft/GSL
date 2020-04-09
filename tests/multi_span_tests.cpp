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

#include <gtest/gtest.h>

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
static constexpr char deathstring[] = "Expected Death";
struct BaseClass
{
};
struct DerivedClass : BaseClass
{
};

void overloaded_func(multi_span<const int, dynamic_range, 3, 5> exp, int expected_value)
{
    for (auto val : exp) { EXPECT_TRUE(val == expected_value); }
}

void overloaded_func(multi_span<const char, dynamic_range, 3, 5> exp, char expected_value)
{
    for (auto val : exp) { EXPECT_TRUE(val == expected_value); }
}

void iterate_second_column(multi_span<int, dynamic_range, dynamic_range> av)
{
    auto length = av.size() / 2;

    // view to the second column
    auto section = av.section({0, 1}, {length, 1});

    EXPECT_TRUE(section.size() == length);
    for (auto i = 0; i < section.size(); ++i) { EXPECT_TRUE(section[i][0] == av[i][1]); }

    for (auto i = 0; i < section.size(); ++i)
    {
        auto idx = multi_span_index<2>{i, 0}; // avoid braces inside the CHECK macro
        EXPECT_TRUE(section[idx] == av[i][1]);
    }

    EXPECT_TRUE(section.bounds().index_bounds()[0] == length);
    EXPECT_TRUE(section.bounds().index_bounds()[1] == 1);
    for (auto i = 0; i < section.bounds().index_bounds()[0]; ++i)
    {
        for (auto j = 0; j < section.bounds().index_bounds()[1]; ++j)
        {
            auto idx = multi_span_index<2>{i, j}; // avoid braces inside the CHECK macro
            EXPECT_TRUE(section[idx] == av[i][1]);
        }
    }

    auto check_sum = 0;
    for (auto i = 0; i < length; ++i) { check_sum += av[i][1]; }

    {
        auto idx = 0;
        auto sum = 0;
        for (auto num : section)
        {
            EXPECT_TRUE(num == av[idx][1]);
            sum += num;
            idx++;
        }

        EXPECT_TRUE(sum == check_sum);
    }
    {
        auto idx = length - 1;
        auto sum = 0;
        for (auto iter = section.rbegin(); iter != section.rend(); ++iter)
        {
            EXPECT_TRUE(*iter == av[idx][1]);
            sum += *iter;
            idx--;
        }

        EXPECT_TRUE(sum == check_sum);
    }
}
template <class Bounds>
void fn(const Bounds&)
{
    static_assert(Bounds::static_size == 60, "static bounds is wrong size");
}

} // namespace

TEST(multi_span_test, default_constructor)
{
    {
        multi_span<int> s;
        EXPECT_TRUE(s.length() ==  0);
        EXPECT_TRUE(s.data() == nullptr);

        multi_span<const int> cs;
        EXPECT_TRUE(cs.length() ==  0);
        EXPECT_TRUE(cs.data() == nullptr);
    }

    {
        multi_span<int, 0> s;
        EXPECT_TRUE(s.length() ==  0);
        EXPECT_TRUE(s.data() == nullptr);

        multi_span<const int, 0> cs;
        EXPECT_TRUE(cs.length() ==  0);
        EXPECT_TRUE(cs.data() == nullptr);
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<int, 1> s;
        EXPECT_TRUE(s.length() ==  1);
        EXPECT_TRUE(s.data() == nullptr); // explains why it can't compile
#endif
    }

    {
        multi_span<int> s{};
        EXPECT_TRUE(s.length() ==  0);
        EXPECT_TRUE(s.data() == nullptr);

        multi_span<const int> cs{};
        EXPECT_TRUE(cs.length() ==  0);
        EXPECT_TRUE(cs.data() == nullptr);
    }
}

TEST(multi_span_test, from_nullptr_constructor)
{
    {
        multi_span<int> s = nullptr;
        EXPECT_TRUE(s.length() ==  0);
        EXPECT_TRUE(s.data() == nullptr);

        multi_span<const int> cs = nullptr;
        EXPECT_TRUE(cs.length() ==  0);
        EXPECT_TRUE(cs.data() == nullptr);
    }

    {
        multi_span<int, 0> s = nullptr;
        EXPECT_TRUE(s.length() ==  0);
        EXPECT_TRUE(s.data() == nullptr);

        multi_span<const int, 0> cs = nullptr;
        EXPECT_TRUE(cs.length() ==  0);
        EXPECT_TRUE(cs.data() == nullptr);
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<int, 1> s = nullptr;
        EXPECT_TRUE(s.length() ==  1);
        EXPECT_TRUE(s.data() == nullptr); // explains why it can't compile
#endif
    }

    {
        multi_span<int> s{nullptr};
        EXPECT_TRUE(s.length() ==  0);
        EXPECT_TRUE(s.data() == nullptr);

        multi_span<const int> cs{nullptr};
        EXPECT_TRUE(cs.length() ==  0);
        EXPECT_TRUE(cs.data() == nullptr);
    }

    {
        multi_span<int*> s{nullptr};
        EXPECT_TRUE(s.length() ==  0);
        EXPECT_TRUE(s.data() == nullptr);

        multi_span<const int*> cs{nullptr};
        EXPECT_TRUE(cs.length() ==  0);
        EXPECT_TRUE(cs.data() == nullptr);
    }
}

TEST(multi_span_test, from_nullptr_length_constructor) {
    {
        multi_span<int> s{nullptr, 0};
        EXPECT_TRUE(s.length() ==  0);
        EXPECT_TRUE(s.data() == nullptr);

        multi_span<const int> cs{nullptr, 0};
        EXPECT_TRUE(cs.length() ==  0);
        EXPECT_TRUE(cs.data() == nullptr);
    }

    {
        multi_span<int, 0> s{nullptr, 0};
        EXPECT_TRUE(s.length() ==  0);
        EXPECT_TRUE(s.data() == nullptr);

        multi_span<const int, 0> cs{nullptr, 0};
        EXPECT_TRUE(cs.length() ==  0);
        EXPECT_TRUE(cs.data() == nullptr);
    }

    std::set_terminate([] {
        std::cerr << "Expected Death. from_nullptr_length_constructor";
        std::abort();
    });

    {
        auto workaround_macro = []() { const multi_span<int> s{nullptr, 1}; };
        EXPECT_DEATH(workaround_macro(), deathstring);

        auto const_workaround_macro = []() { const multi_span<const int> cs{nullptr, 1}; };
        EXPECT_DEATH(const_workaround_macro(), deathstring);
    }

    {
        auto workaround_macro = []() { const multi_span<int, 0> s{nullptr, 1}; };
        EXPECT_DEATH(workaround_macro(), deathstring);

        auto const_workaround_macro = []() { const multi_span<const int, 0> s{nullptr, 1}; };
        EXPECT_DEATH(const_workaround_macro(), deathstring);
    }

    {
        multi_span<int*> s{nullptr, 0};
        EXPECT_TRUE(s.length() ==  0);
        EXPECT_TRUE(s.data() == nullptr);

        multi_span<const int*> cs{nullptr, 0};
        EXPECT_TRUE(cs.length() ==  0);
        EXPECT_TRUE(cs.data() == nullptr);
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<int, 1> s{nullptr, 0};
        EXPECT_TRUE(s.length() ==  1);
        EXPECT_TRUE(s.data() == nullptr); // explains why it can't compile
#endif
    }
}

TEST(multi_span_test, from_element_constructor)
{
    int i = 5;

    {
        multi_span<int> s = i;
        EXPECT_TRUE(s.length() ==  1);
        EXPECT_TRUE(s.data() == &i);
        EXPECT_TRUE(s[0] == 5);

        multi_span<const int> cs = i;
        EXPECT_TRUE(cs.length() ==  1);
        EXPECT_TRUE(cs.data() == &i);
        EXPECT_TRUE(cs[0] == 5);
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
        EXPECT_TRUE(s.length() ==  0);
        EXPECT_TRUE(s.data() == &i);
#endif
    }

    {
        multi_span<int, 1> s = i;
        EXPECT_TRUE(s.length() ==  1);
        EXPECT_TRUE(s.data() == &i);
        EXPECT_TRUE(s[0] == 5);
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<int, 2> s = i;
        EXPECT_TRUE(s.length() ==  2);
        EXPECT_TRUE(s.data() == &i);
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

TEST(multi_span_test, from_pointer_length_constructor)
{
    int arr[4] = {1, 2, 3, 4};

    {
        multi_span<int> s{&arr[0], 2};
        EXPECT_TRUE(s.length() ==  2);
        EXPECT_TRUE(s.data() == &arr[0]);
        EXPECT_TRUE(s[0] ==  1);
        EXPECT_TRUE(s[1] == 2);
    }

    {
        multi_span<int, 2> s{&arr[0], 2};
        EXPECT_TRUE(s.length() ==  2);
        EXPECT_TRUE(s.data() == &arr[0]);
        EXPECT_TRUE(s[0] ==  1);
        EXPECT_TRUE(s[1] == 2);
    }

    {
        int* p = nullptr;
        multi_span<int> s{p, 0};
        EXPECT_TRUE(s.length() ==  0);
        EXPECT_TRUE(s.data() == nullptr);
    }

    std::set_terminate([] {
        std::cerr << "Expected Death. from_pointer_length_constructor";
        std::abort();
    });

    {
        int* p = nullptr;
        auto workaround_macro = [=]() { const multi_span<int> s{p, 2}; };
        EXPECT_DEATH(workaround_macro(), deathstring);
    }
}

TEST(multi_span_test, from_pointer_pointer_constructor)
{
    int arr[4] = {1, 2, 3, 4};

    {
        multi_span<int> s{&arr[0], &arr[2]};
        EXPECT_TRUE(s.length() ==  2);
        EXPECT_TRUE(s.data() == &arr[0]);
        EXPECT_TRUE(s[0] ==  1);
        EXPECT_TRUE(s[1] == 2);
    }

    {
        multi_span<int, 2> s{&arr[0], &arr[2]};
        EXPECT_TRUE(s.length() ==  2);
        EXPECT_TRUE(s.data() == &arr[0]);
        EXPECT_TRUE(s[0] ==  1);
        EXPECT_TRUE(s[1] == 2);
    }

    {
        multi_span<int> s{&arr[0], &arr[0]};
        EXPECT_TRUE(s.length() ==  0);
        EXPECT_TRUE(s.data() == &arr[0]);
    }

    {
        multi_span<int, 0> s{&arr[0], &arr[0]};
        EXPECT_TRUE(s.length() ==  0);
        EXPECT_TRUE(s.data() == &arr[0]);
    }


    std::set_terminate([] {
        std::cerr << "Expected Death. from_pointer_pointer_constructor";
        std::abort();
    });

    {
        auto workaround_macro = [&]() { const multi_span<int> s{&arr[1], &arr[0]}; };
        EXPECT_DEATH(workaround_macro(), deathstring);
    }

    {
        int* p = nullptr;
        auto workaround_macro = [&]() { const multi_span<int> s{&arr[0], p}; };
        EXPECT_DEATH(workaround_macro(), deathstring);
    }

    {
        int* p = nullptr;
        auto workaround_macro = [&]() { const multi_span<int> s{p, p}; };
        EXPECT_DEATH(workaround_macro(), deathstring);
    }

    {
        int* p = nullptr;
        auto workaround_macro = [&]() { const multi_span<int> s{&arr[0], p}; };
        EXPECT_DEATH(workaround_macro(), deathstring);
    }
}

TEST(multi_span_test, from_array_constructor)
{
    int arr[5] = {1, 2, 3, 4, 5};

    {
        multi_span<int> s{arr};
        EXPECT_TRUE(s.length() ==  5);
        EXPECT_TRUE(s.data() == &arr[0]);
    }

    {
        multi_span<int, 5> s{arr};
        EXPECT_TRUE(s.length() ==  5);
        EXPECT_TRUE(s.data() == &arr[0]);
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<int, 6> s{arr};
#endif
    }

    {
        multi_span<int, 0> s{arr};
        EXPECT_TRUE(s.length() ==  0);
        EXPECT_TRUE(s.data() == &arr[0]);
    }

    int arr2d[2][3] = {1, 2, 3, 4, 5, 6};

    {
        multi_span<int> s{arr2d};
        EXPECT_TRUE(s.length() ==  6);
        EXPECT_TRUE(s.data() == &arr2d[0][0]);
        EXPECT_TRUE(s[0] ==  1);
        EXPECT_TRUE(s[5] == 6);
    }

    {
        multi_span<int, 0> s{arr2d};
        EXPECT_TRUE(s.length() ==  0);
        EXPECT_TRUE(s.data() == &arr2d[0][0]);
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<int, 5> s{arr2d};
#endif
    }

    {
        multi_span<int, 6> s{arr2d};
        EXPECT_TRUE(s.length() ==  6);
        EXPECT_TRUE(s.data() == &arr2d[0][0]);
        EXPECT_TRUE(s[0] ==  1);
        EXPECT_TRUE(s[5] == 6);
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<int, 7> s{arr2d};
#endif
    }

    {
        multi_span<int[3]> s{arr2d[0]};
        EXPECT_TRUE(s.length() ==  1);
        EXPECT_TRUE(s.data() == &arr2d[0]);
    }

    {
        multi_span<int, 2, 3> s{arr2d};
        EXPECT_TRUE(s.length() ==  6);
        EXPECT_TRUE(s.data() == &arr2d[0][0]);
        auto workaround_macro = [&]() { return s[{1, 2}] == 6; };
        EXPECT_TRUE(workaround_macro());
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<int, 3, 3> s{arr2d};
#endif
    }

    int arr3d[2][3][2] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

    {
        multi_span<int> s{arr3d};
        EXPECT_TRUE(s.length() ==  12);
        EXPECT_TRUE(s.data() == &arr3d[0][0][0]);
        EXPECT_TRUE(s[0] ==  1);
        EXPECT_TRUE(s[11] == 12);
    }

    {
        multi_span<int, 0> s{arr3d};
        EXPECT_TRUE(s.length() ==  0);
        EXPECT_TRUE(s.data() == &arr3d[0][0][0]);
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<int, 11> s{arr3d};
#endif
    }

    {
        multi_span<int, 12> s{arr3d};
        EXPECT_TRUE(s.length() ==  12);
        EXPECT_TRUE(s.data() == &arr3d[0][0][0]);
        EXPECT_TRUE(s[0] ==  1);
        EXPECT_TRUE(s[5] == 6);
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<int, 13> s{arr3d};
#endif
    }

    {
        multi_span<int[3][2]> s{arr3d[0]};
        EXPECT_TRUE(s.length() ==  1);
        EXPECT_TRUE(s.data() == &arr3d[0]);
    }

    {
        multi_span<int, 3, 2, 2> s{arr3d};
        EXPECT_TRUE(s.length() ==  12);
        EXPECT_TRUE(s.data() == &arr3d[0][0][0]);
        auto workaround_macro = [&]() { return s[{2, 1, 0}] == 11; };
        EXPECT_TRUE(workaround_macro());
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<int, 3, 3, 3> s{arr3d};
#endif
    }
}

TEST(multi_span_test, from_dynamic_array_constructor)
{
    double(*arr)[3][4] = new double[100][3][4];

    std::set_terminate([] {
        std::cerr << "Expected Death. from_nullptr_length_constructor";
        std::abort();
    });

    {
        multi_span<double, dynamic_range, 3, 4> s(arr, 10);
        EXPECT_TRUE(s.length() ==  120);
        EXPECT_TRUE(s.data() == &arr[0][0][0]);
        EXPECT_DEATH(s[10][3][4], deathstring);
    }

    {
        multi_span<double, dynamic_range, 4, 3> s(arr, 10);
        EXPECT_TRUE(s.length() ==  120);
        EXPECT_TRUE(s.data() == &arr[0][0][0]);
    }

    {
        multi_span<double> s(arr, 10);
        EXPECT_TRUE(s.length() ==  120);
        EXPECT_TRUE(s.data() == &arr[0][0][0]);
    }

    {
        multi_span<double, dynamic_range, 3, 4> s(arr, 0);
        EXPECT_TRUE(s.length() ==  0);
        EXPECT_TRUE(s.data() == &arr[0][0][0]);
    }

    delete[] arr;
}

TEST(multi_span_test, from_std_array_constructor)
{
    std::array<int, 4> arr = {1, 2, 3, 4};

    {
        multi_span<int> s{arr};
        EXPECT_TRUE(s.size() ==  narrow_cast<ptrdiff_t>(arr.size()));
        EXPECT_TRUE(s.data() == arr.data());

        multi_span<const int> cs{arr};
        EXPECT_TRUE(cs.size() ==  narrow_cast<ptrdiff_t>(arr.size()));
        EXPECT_TRUE(cs.data() == arr.data());
    }

    {
        multi_span<int, 4> s{arr};
        EXPECT_TRUE(s.size() ==  narrow_cast<ptrdiff_t>(arr.size()));
        EXPECT_TRUE(s.data() == arr.data());

        multi_span<const int, 4> cs{arr};
        EXPECT_TRUE(cs.size() ==  narrow_cast<ptrdiff_t>(arr.size()));
        EXPECT_TRUE(cs.data() == arr.data());
    }

    {
        multi_span<int, 2> s{arr};
        EXPECT_TRUE(s.size() ==  2);
        EXPECT_TRUE(s.data() == arr.data());

        multi_span<const int, 2> cs{arr};
        EXPECT_TRUE(cs.size() ==  2);
        EXPECT_TRUE(cs.data() == arr.data());
    }

    {
        multi_span<int, 0> s{arr};
        EXPECT_TRUE(s.size() ==  0);
        EXPECT_TRUE(s.data() == arr.data());

        multi_span<const int, 0> cs{arr};
        EXPECT_TRUE(cs.size() ==  0);
        EXPECT_TRUE(cs.data() == arr.data());
    }

    // TODO This is currently an unsupported scenario. We will come back to it as we revise
    // the multidimensional interface and what transformations between dimensionality look like
    //{
    //    multi_span<int, 2, 2> s{arr};
    //    EXPECT_TRUE(s.size() == narrow_cast<ptrdiff_t>(arr.size()));
    //    EXPECT_TRUE(s.data() == arr.data());
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

TEST(multi_span_test, from_const_std_array_constructor)
{
    const std::array<int, 4> arr = {1, 2, 3, 4};

    {
        multi_span<const int> s{arr};
        EXPECT_TRUE(s.size() ==  narrow_cast<ptrdiff_t>(arr.size()));
        EXPECT_TRUE(s.data() == arr.data());
    }

    {
        multi_span<const int, 4> s{arr};
        EXPECT_TRUE(s.size() ==  narrow_cast<ptrdiff_t>(arr.size()));
        EXPECT_TRUE(s.data() == arr.data());
    }

    {
        multi_span<const int, 2> s{arr};
        EXPECT_TRUE(s.size() ==  2);
        EXPECT_TRUE(s.data() == arr.data());
    }

    {
        multi_span<const int, 0> s{arr};
        EXPECT_TRUE(s.size() ==  0);
        EXPECT_TRUE(s.data() == arr.data());
    }

    // TODO This is currently an unsupported scenario. We will come back to it as we revise
    // the multidimensional interface and what transformations between dimensionality look like
    //{
    //    multi_span<int, 2, 2> s{arr};
    //    EXPECT_TRUE(s.size() == narrow_cast<ptrdiff_t>(arr.size()));
    //    EXPECT_TRUE(s.data() == arr.data());
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

TEST(multi_span_test, from_container_constructor)
{
    std::vector<int> v = {1, 2, 3};
    const std::vector<int> cv = v;

    {
        multi_span<int> s{v};
        EXPECT_TRUE(s.size() ==  narrow_cast<std::ptrdiff_t>(v.size()));
        EXPECT_TRUE(s.data() == v.data());

        multi_span<const int> cs{v};
        EXPECT_TRUE(cs.size() ==  narrow_cast<std::ptrdiff_t>(v.size()));
        EXPECT_TRUE(cs.data() == v.data());
    }

    std::string str = "hello";
    const std::string cstr = "hello";

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<char> s{str};
        EXPECT_TRUE(s.size() ==  narrow_cast<std::ptrdiff_t>(str.size()));
        EXPECT_TRUE(s.data() == str.data());
#endif
        multi_span<const char> cs{str};
        EXPECT_TRUE(cs.size() ==  narrow_cast<std::ptrdiff_t>(str.size()));
        EXPECT_TRUE(cs.data() == str.data());
    }

    {
#ifdef CONFIRM_COMPILATION_ERRORS
        multi_span<char> s{cstr};
#endif
        multi_span<const char> cs{cstr};
        EXPECT_TRUE(cs.size() ==  narrow_cast<std::ptrdiff_t>(cstr.size()));
        EXPECT_TRUE(cs.data() == cstr.data());
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

TEST(multi_span_test, from_convertible_span_constructor)
{
#ifdef CONFIRM_COMPILATION_ERRORS
    multi_span<int, 7, 4, 2> av1(nullptr, b1);

    std::set_terminate([] {
        std::cerr << "Expected Death. from_convertible_span_constructor";
        std::abort();
    });

    auto f = [&]() { multi_span<int, 7, 4, 2> av1(nullptr); };
    EXPECT_DEATH(f(), deathstring);
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

TEST(multi_span_test, copy_move_and_assignment)
{
    multi_span<int> s1;
    EXPECT_TRUE(s1.empty());

    int arr[] = {3, 4, 5};

    multi_span<const int> s2 = arr;
    EXPECT_TRUE(s2.length() ==  3);
    EXPECT_TRUE(s2.data() == &arr[0]);

    s2 = s1;
    EXPECT_TRUE(s2.empty());

    auto get_temp_span = [&]() -> multi_span<int> { return {&arr[1], 2}; };
    auto use_span = [&](multi_span<const int> s) {
        EXPECT_TRUE(s.length() ==  2);
        EXPECT_TRUE(s.data() == &arr[1]);
    };
    use_span(get_temp_span());

    s1 = get_temp_span();
    EXPECT_TRUE(s1.length() ==  2);
    EXPECT_TRUE(s1.data() == &arr[1]);
}

TEST(multi_span_test, as_multi_span_reshape)
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

    EXPECT_TRUE(av8.size() == av6.size());
    for (auto i = 0; i < av8.size(); i++) { EXPECT_TRUE(av8[i] == 1); }
}

TEST(multi_span_test, first)
{
    int arr[5] = {1, 2, 3, 4, 5};

    {
        multi_span<int, 5> av = arr;
        EXPECT_TRUE(av.first<2>().bounds() == static_bounds<2>());
        EXPECT_TRUE(av.first<2>().length() == 2);
        EXPECT_TRUE(av.first(2).length() == 2);
    }

    {
        multi_span<int, 5> av = arr;
        EXPECT_TRUE(av.first<0>().bounds() == static_bounds<0>());
        EXPECT_TRUE(av.first<0>().length() == 0);
        EXPECT_TRUE(av.first(0).length() == 0);
    }

    {
        multi_span<int, 5> av = arr;
        EXPECT_TRUE(av.first<5>().bounds() == static_bounds<5>());
        EXPECT_TRUE(av.first<5>().length() == 5);
        EXPECT_TRUE(av.first(5).length() == 5);
    }

    {
        multi_span<int, 5> av = arr;
#ifdef CONFIRM_COMPILATION_ERRORS
        EXPECT_TRUE(av.first<6>().bounds() == static_bounds<6>());
        EXPECT_TRUE(av.first<6>().length() == 6);
        EXPECT_TRUE(av.first<-1>().length() == -1);
#endif

        std::set_terminate([] {
            std::cerr << "Expected Death. first";
            std::abort();
        });

        EXPECT_DEATH(av.first(6).length(), deathstring);
    }

    {
        multi_span<int, dynamic_range> av;
        EXPECT_TRUE(av.first<0>().bounds() == static_bounds<0>());
        EXPECT_TRUE(av.first<0>().length() == 0);
        EXPECT_TRUE(av.first(0).length() == 0);
    }
}

TEST(multi_span_test, last)
{
    int arr[5] = {1, 2, 3, 4, 5};

    {
        multi_span<int, 5> av = arr;
        EXPECT_TRUE(av.last<2>().bounds() == static_bounds<2>());
        EXPECT_TRUE(av.last<2>().length() == 2);
        EXPECT_TRUE(av.last(2).length() == 2);
    }

    {
        multi_span<int, 5> av = arr;
        EXPECT_TRUE(av.last<0>().bounds() == static_bounds<0>());
        EXPECT_TRUE(av.last<0>().length() == 0);
        EXPECT_TRUE(av.last(0).length() == 0);
    }

    {
        multi_span<int, 5> av = arr;
        EXPECT_TRUE(av.last<5>().bounds() == static_bounds<5>());
        EXPECT_TRUE(av.last<5>().length() == 5);
        EXPECT_TRUE(av.last(5).length() == 5);
    }

    {
        multi_span<int, 5> av = arr;
#ifdef CONFIRM_COMPILATION_ERRORS
        EXPECT_TRUE(av.last<6>().bounds() == static_bounds<6>());
        EXPECT_TRUE(av.last<6>().length() == 6);
#endif

        std::set_terminate([] {
            std::cerr << "Expected Death. last";
            std::abort();
        });

        EXPECT_DEATH(av.last(6).length(), deathstring);
    }

    {
        multi_span<int, dynamic_range> av;
        EXPECT_TRUE(av.last<0>().bounds() == static_bounds<0>());
        EXPECT_TRUE(av.last<0>().length() == 0);
        EXPECT_TRUE(av.last(0).length() == 0);
    }
}

TEST(multi_span_test, subspan)
{
    int arr[5] = {1, 2, 3, 4, 5};

    {
        multi_span<int, 5> av = arr;
        EXPECT_TRUE((av.subspan<2, 2>().bounds()) == static_bounds<2>());
        EXPECT_TRUE((av.subspan<2, 2>().length()) == 2);
        EXPECT_TRUE(av.subspan(2, 2).length() == 2);
        EXPECT_TRUE(av.subspan(2, 3).length() == 3);
    }

    {
        multi_span<int, 5> av = arr;
        EXPECT_TRUE((av.subspan<0, 0>().bounds()) == static_bounds<0>());
        EXPECT_TRUE((av.subspan<0, 0>().length()) == 0);
        EXPECT_TRUE(av.subspan(0, 0).length() == 0);
    }


    std::set_terminate([] {
        std::cerr << "Expected Death. subspan";
        std::abort();
    });
    {
        multi_span<int, 5> av = arr;
        EXPECT_TRUE((av.subspan<0, 5>().bounds()) == static_bounds<5>());
        EXPECT_TRUE((av.subspan<0, 5>().length()) == 5);
        EXPECT_TRUE(av.subspan(0, 5).length() == 5);
        EXPECT_DEATH(av.subspan(0, 6).length(), deathstring);
        EXPECT_DEATH(av.subspan(1, 5).length(), deathstring);
    }

    {
        multi_span<int, 5> av = arr;
        EXPECT_TRUE((av.subspan<5, 0>().bounds()) == static_bounds<0>());
        EXPECT_TRUE((av.subspan<5, 0>().length()) == 0);
        EXPECT_TRUE(av.subspan(5, 0).length() == 0);
        EXPECT_DEATH(av.subspan(6, 0).length(), deathstring);
    }

    {
        multi_span<int, dynamic_range> av;
        EXPECT_TRUE((av.subspan<0, 0>().bounds()) == static_bounds<0>());
        EXPECT_TRUE((av.subspan<0, 0>().length()) == 0);
        EXPECT_TRUE(av.subspan(0, 0).length() == 0);
        EXPECT_DEATH((av.subspan<1, 0>().length()), deathstring);
    }

    {
        multi_span<int> av;
        EXPECT_TRUE(av.subspan(0).length() == 0);
        EXPECT_DEATH(av.subspan(1).length(), deathstring);
    }

    {
        multi_span<int> av = arr;
        EXPECT_TRUE(av.subspan(0).length() == 5);
        EXPECT_TRUE(av.subspan(1).length() == 4);
        EXPECT_TRUE(av.subspan(4).length() == 1);
        EXPECT_TRUE(av.subspan(5).length() == 0);
        // Disabled test instead of fixing since multi_span is deprecated. (PR#835)
#if !(defined(__GNUC__) && __GNUC__ == 8)
        EXPECT_DEATH(av.subspan(6).length(), deathstring);
#endif
        auto av2 = av.subspan(1);
        for (int i = 0; i < 4; ++i) EXPECT_TRUE(av2[i] == i + 2);
    }

    {
        multi_span<int, 5> av = arr;
        EXPECT_TRUE(av.subspan(0).length() == 5);
        EXPECT_TRUE(av.subspan(1).length() == 4);
        EXPECT_TRUE(av.subspan(4).length() == 1);
        EXPECT_TRUE(av.subspan(5).length() == 0);
        EXPECT_DEATH(av.subspan(6).length(), deathstring);
        auto av2 = av.subspan(1);
        for (int i = 0; i < 4; ++i) EXPECT_TRUE(av2[i] == i + 2);
    }
}

TEST(multi_span_test, rank)
{
    int arr[2] = {1, 2};

    {
        multi_span<int> s;
        EXPECT_TRUE(s.rank() == static_cast<size_t>(1));
    }

    {
        multi_span<int, 2> s = arr;
        EXPECT_TRUE(s.rank() == static_cast<size_t>(1));
    }

    int arr2d[1][1] = {};
    {
        multi_span<int, 1, 1> s = arr2d;
        EXPECT_TRUE(s.rank() == static_cast<size_t>(2));
    }
}

TEST(multi_span_test, extent)
{
    std::set_terminate([] {
        std::cerr << "Expected Death. extent";
        std::abort();
    });

    {
        multi_span<int> s;
        EXPECT_TRUE(s.extent() == 0);
        EXPECT_TRUE(s.extent(0) == 0);
        EXPECT_DEATH(s.extent(1), deathstring);
#ifdef CONFIRM_COMPILATION_ERRORS
        EXPECT_TRUE(s.extent<1>() == 0);
#endif
    }

    {
        multi_span<int, 0> s;
        EXPECT_TRUE(s.extent() == 0);
        EXPECT_TRUE(s.extent(0) == 0);
        EXPECT_DEATH(s.extent(1), deathstring);
    }

    {
        int arr2d[1][2] = {};

        multi_span<int, 1, 2> s = arr2d;
        EXPECT_TRUE(s.extent() == 1);
        EXPECT_TRUE(s.extent<0>() == 1);
        EXPECT_TRUE(s.extent<1>() == 2);
        EXPECT_TRUE(s.extent(0) == 1);
        EXPECT_TRUE(s.extent(1) == 2);
        EXPECT_DEATH(s.extent(3), deathstring);
    }

    {
        int arr2d[1][2] = {};

        multi_span<int, 0, 2> s = arr2d;
        EXPECT_TRUE(s.extent() == 0);
        EXPECT_TRUE(s.extent<0>() == 0);
        EXPECT_TRUE(s.extent<1>() == 2);
        EXPECT_TRUE(s.extent(0) == 0);
        EXPECT_TRUE(s.extent(1) == 2);
        EXPECT_DEATH(s.extent(3), deathstring);
    }
}

TEST(multi_span_test, operator_function_call)
{
    int arr[4] = {1, 2, 3, 4};

    std::set_terminate([] {
        std::cerr << "Expected Death. operator_function_call";
        std::abort();
    });

    {
        multi_span<int> s = arr;
        EXPECT_TRUE(s(0) == 1);
        EXPECT_DEATH(s(5), deathstring);
    }

    int arr2d[2][3] = {1, 2, 3, 4, 5, 6};

    {
        multi_span<int, 2, 3> s = arr2d;
        EXPECT_TRUE(s(0, 0) == 1);
        EXPECT_TRUE(s(0, 1) == 2);
        EXPECT_TRUE(s(1, 2) == 6);
    }

    int arr3d[2][2][2] = {1, 2, 3, 4, 5, 6, 7, 8};

    {
        multi_span<int, 2, 2, 2> s = arr3d;
        EXPECT_TRUE(s(0, 0, 0) == 1);
        EXPECT_TRUE(s(1, 1, 1) == 8);
    }
}

TEST(multi_span_test, comparison_operators)
{
    {
        int arr[10][2];
        auto s1 = as_multi_span(arr);
        multi_span<const int, dynamic_range, 2> s2 = s1;

        EXPECT_TRUE(s1 == s2);

        multi_span<int, 20> s3 = as_multi_span(s1, dim(20));
        EXPECT_TRUE(s3 == s2);
        EXPECT_TRUE(s3 == s1);
    }
    {
        multi_span<int> s1 = nullptr;
        multi_span<int> s2 = nullptr;
        EXPECT_TRUE(s1 == s2);
        EXPECT_FALSE(s1 != s2);
        EXPECT_FALSE(s1 < s2);
        EXPECT_TRUE(s1 <= s2);
        EXPECT_FALSE(s1 > s2);
        EXPECT_TRUE(s1 >= s2);
        EXPECT_TRUE(s2 == s1);
        EXPECT_FALSE(s2 != s1);
        EXPECT_FALSE(s2 < s1);
        EXPECT_TRUE(s2 <= s1);
        EXPECT_FALSE(s2 > s1);
        EXPECT_TRUE(s2 >= s1);
    }


    {
        int arr[] = {2, 1}; // bigger

        multi_span<int> s1 = nullptr;
        multi_span<int> s2 = arr;

        EXPECT_TRUE(s1 != s2);
        EXPECT_TRUE(s2 != s1);
        EXPECT_FALSE(s1 == s2);
        EXPECT_FALSE(s2 == s1);
        EXPECT_TRUE(s1 < s2);
        EXPECT_FALSE(s2 < s1);
        EXPECT_TRUE(s1 <= s2);
        EXPECT_FALSE(s2 <= s1);
        EXPECT_TRUE(s2 > s1);
        EXPECT_FALSE(s1 > s2);
        EXPECT_TRUE(s2 >= s1);
        EXPECT_FALSE(s1 >= s2);
    }

    {
        int arr1[] = {1, 2};
        int arr2[] = {1, 2};
        multi_span<int> s1 = arr1;
        multi_span<int> s2 = arr2;

        EXPECT_TRUE(s1 == s2);
        EXPECT_FALSE(s1 != s2);
        EXPECT_FALSE(s1 < s2);
        EXPECT_TRUE(s1 <= s2);
        EXPECT_FALSE(s1 > s2);
        EXPECT_TRUE(s1 >= s2);
        EXPECT_TRUE(s2 == s1);
        EXPECT_FALSE(s2 != s1);
        EXPECT_FALSE(s2 < s1);
        EXPECT_TRUE(s2 <= s1);
        EXPECT_FALSE(s2 > s1);
        EXPECT_TRUE(s2 >= s1);
    }

    {
        int arr[] = {1, 2, 3};

        multi_span<int> s1 = {&arr[0], 2}; // shorter
        multi_span<int> s2 = arr;          // longer

        EXPECT_TRUE(s1 != s2);
        EXPECT_TRUE(s2 != s1);
        EXPECT_FALSE(s1 == s2);
        EXPECT_FALSE(s2 == s1);
        EXPECT_TRUE(s1 < s2);
        EXPECT_FALSE(s2 < s1);
        EXPECT_TRUE(s1 <= s2);
        EXPECT_FALSE(s2 <= s1);
        EXPECT_TRUE(s2 > s1);
        EXPECT_FALSE(s1 > s2);
        EXPECT_TRUE(s2 >= s1);
        EXPECT_FALSE(s1 >= s2);
    }

    {
        int arr1[] = {1, 2}; // smaller
        int arr2[] = {2, 1}; // bigger

        multi_span<int> s1 = arr1;
        multi_span<int> s2 = arr2;

        EXPECT_TRUE(s1 != s2);
        EXPECT_TRUE(s2 != s1);
        EXPECT_FALSE(s1 == s2);
        EXPECT_FALSE(s2 == s1);
        EXPECT_TRUE(s1 < s2);
        EXPECT_FALSE(s2 < s1);
        EXPECT_TRUE(s1 <= s2);
        EXPECT_FALSE(s2 <= s1);
        EXPECT_TRUE(s2 > s1);
        EXPECT_FALSE(s1 > s2);
        EXPECT_TRUE(s2 >= s1);
        EXPECT_FALSE(s1 >= s2);
    }
}

TEST(multi_span_test, basics)
{
    auto ptr = as_multi_span(new int[10], 10);
    fill(ptr.begin(), ptr.end(), 99);
    for (int num : ptr) { EXPECT_TRUE(num == 99); }

    delete[] ptr.data();
}

TEST(multi_span_test, bounds_checks)
{
    int arr[10][2];
    auto av = as_multi_span(arr);

    fill(begin(av), end(av), 0);

    av[2][0] = 1;
    av[1][1] = 3;

    std::set_terminate([] {
        std::cerr << "Expected Death. bounds_check";
        std::abort();
    });

    // out of bounds
    EXPECT_DEATH(av[1][3] = 3, deathstring);
    EXPECT_DEATH((av[{1, 3}] = 3), deathstring);

    EXPECT_DEATH(av[10][2], deathstring);
    EXPECT_DEATH((av[{10, 2}]), deathstring);

    EXPECT_DEATH(av[-1][0], deathstring);
    EXPECT_DEATH((av[{-1, 0}]), deathstring);

    EXPECT_DEATH(av[0][-1], deathstring);
    EXPECT_DEATH((av[{0, -1}]), deathstring);
}

TEST(multi_span_test, span_parameter_test)
{
    auto data = new int[4][3][5];

    auto av = as_multi_span(data, 4);

    EXPECT_TRUE(av.size() == 60);

    fill(av.begin(), av.end(), 34);

    int count = 0;
    for_each(av.rbegin(), av.rend(), [&](int val) { count += val; });
    EXPECT_TRUE(count == 34 * 60);
    overloaded_func(av, 34);

    overloaded_func(as_multi_span(av, dim(4), dim(3), dim(5)), 34);

    delete[] data;
}

TEST(multi_span_test, md_access)
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
            EXPECT_TRUE(expected + 1 == image_view[i][j][0]);
            EXPECT_TRUE(expected + 2 == image_view[i][j][1]);
            EXPECT_TRUE(expected + 3 == image_view[i][j][2]);

            auto val = image_view[{i, j, 0}];
            EXPECT_TRUE(expected + 1 == val);
            val = image_view[{i, j, 1}];
            EXPECT_TRUE(expected + 2 == val);
            val = image_view[{i, j, 2}];
            EXPECT_TRUE(expected + 3 == val);

            expected += 3;
        }
    }

    delete[] image_ptr;
}

TEST(multi_span_test, as_multi_span)
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

        EXPECT_TRUE(av == bv);

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

TEST(multi_span_test, empty_spans)
{
    std::set_terminate([] {
        std::cerr << "Expected Death. empty_spans";
        std::abort();
    });

    {
        multi_span<int, 0> empty_av(nullptr);

        EXPECT_TRUE(empty_av.bounds().index_bounds() == multi_span_index<1>{0});
        EXPECT_DEATH(empty_av[0], deathstring);
        EXPECT_DEATH(empty_av.begin()[0], deathstring);
        EXPECT_DEATH(empty_av.cbegin()[0], deathstring);
        for (auto& v : empty_av)
        {
            (void) v;
            EXPECT_TRUE(false);
        }
    }

    {
        multi_span<int> empty_av = {};
        EXPECT_TRUE(empty_av.bounds().index_bounds() == multi_span_index<1>{0});
        EXPECT_DEATH(empty_av[0], deathstring);
        EXPECT_DEATH(empty_av.begin()[0], deathstring);
        EXPECT_DEATH(empty_av.cbegin()[0], deathstring);
        for (auto& v : empty_av)
        {
            (void) v;
            EXPECT_TRUE(false);
        }
    }
}

TEST(multi_span_test, index_constructor)
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

    EXPECT_TRUE(av[i] == 4);

    auto av2 = as_multi_span(av, dim<4>(), dim(2));
    ptrdiff_t a2[2] = {0, 1};
    multi_span_index<2> i2 = a2;

    EXPECT_TRUE(av2[i2] == 0);
    EXPECT_TRUE(av2[0][i] == 4);

    delete[] arr;
}

TEST(multi_span_test, index_constructors)
{
    {
        // components of the same type
        multi_span_index<3> i1(0, 1, 2);
        EXPECT_TRUE(i1[0] == 0);

        // components of different types
        std::size_t c0 = 0;
        std::size_t c1 = 1;
        multi_span_index<3> i2(c0, c1, 2);
        EXPECT_TRUE(i2[0] == 0);

        // from array
        multi_span_index<3> i3 = {0, 1, 2};
        EXPECT_TRUE(i3[0] == 0);

        // from other index of the same size type
        multi_span_index<3> i4 = i3;
        EXPECT_TRUE(i4[0] == 0);

        // default
        multi_span_index<3> i7;
        EXPECT_TRUE(i7[0] == 0);

        // default
        multi_span_index<3> i9 = {};
        EXPECT_TRUE(i9[0] == 0);
    }

    {
        // components of the same type
        multi_span_index<1> i1(0);
        EXPECT_TRUE(i1[0] == 0);

        // components of different types
        std::size_t c0 = 0;
        multi_span_index<1> i2(c0);
        EXPECT_TRUE(i2[0] == 0);

        // from array
        multi_span_index<1> i3 = {0};
        EXPECT_TRUE(i3[0] == 0);

        // from int
        multi_span_index<1> i4 = 0;
        EXPECT_TRUE(i4[0] == 0);

        // from other index of the same size type
        multi_span_index<1> i5 = i3;
        EXPECT_TRUE(i5[0] == 0);

        // default
        multi_span_index<1> i8;
        EXPECT_TRUE(i8[0] == 0);

        // default
        multi_span_index<1> i9 = {};
        EXPECT_TRUE(i9[0] == 0);
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

TEST(multi_span_test, index_operations)
{
    ptrdiff_t a[3] = {0, 1, 2};
    ptrdiff_t b[3] = {3, 4, 5};
    multi_span_index<3> i = a;
    multi_span_index<3> j = b;

    EXPECT_TRUE(i[0] == 0);
    EXPECT_TRUE(i[1] == 1);
    EXPECT_TRUE(i[2] == 2);

    {
        multi_span_index<3> k = i + j;

        EXPECT_TRUE(i[0] == 0);
        EXPECT_TRUE(i[1] == 1);
        EXPECT_TRUE(i[2] == 2);
        EXPECT_TRUE(k[0] == 3);
        EXPECT_TRUE(k[1] == 5);
        EXPECT_TRUE(k[2] == 7);
    }

    {
        multi_span_index<3> k = i * 3;

        EXPECT_TRUE(i[0] == 0);
        EXPECT_TRUE(i[1] == 1);
        EXPECT_TRUE(i[2] == 2);
        EXPECT_TRUE(k[0] == 0);
        EXPECT_TRUE(k[1] == 3);
        EXPECT_TRUE(k[2] == 6);
    }

    {
        multi_span_index<3> k = 3 * i;

        EXPECT_TRUE(i[0] == 0);
        EXPECT_TRUE(i[1] == 1);
        EXPECT_TRUE(i[2] == 2);
        EXPECT_TRUE(k[0] == 0);
        EXPECT_TRUE(k[1] == 3);
        EXPECT_TRUE(k[2] == 6);
    }

    {
        multi_span_index<2> k = details::shift_left(i);

        EXPECT_TRUE(i[0] == 0);
        EXPECT_TRUE(i[1] == 1);
        EXPECT_TRUE(i[2] == 2);
        EXPECT_TRUE(k[0] == 1);
        EXPECT_TRUE(k[1] == 2);
    }
}

TEST(multi_span_test, span_section_iteration)
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

TEST(multi_span_test, dynamic_span_section_iteration)
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

TEST(multi_span_test, span_structure_size)
{
    double(*arr)[3][4] = new double[100][3][4];
    multi_span<double, dynamic_range, 3, 4> av1(arr, 10);

    struct EffectiveStructure
    {
        double* v1;
        ptrdiff_t v2;
    };
    EXPECT_TRUE(sizeof(av1) == sizeof(EffectiveStructure));

    std::set_terminate([] {
        std::cerr << "Expected Death. span_structure_size";
        std::abort();
    });

    EXPECT_DEATH(av1[10][3][4], deathstring);

    multi_span<const double, dynamic_range, 6, 4> av2 =
        as_multi_span(av1, dim(5), dim<6>(), dim<4>());
    (void) av2;

    delete[] arr;
}

TEST(multi_span_test, fixed_size_conversions)
{
    int arr[] = {1, 2, 3, 4};

    // converting to an multi_span from an equal size array is ok
    multi_span<int, 4> av4 = arr;
    EXPECT_TRUE(av4.length() == 4);

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
        EXPECT_TRUE(workaround_macro());
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

    std::set_terminate([] {
        std::cerr << "Expected Death. fixed_size_conversions";
        std::abort();
    });

    {
        auto f = [&]() {
            const multi_span<int, 4> av9 = {arr2, 2};
            (void) av9;
        };
        EXPECT_DEATH(f(), deathstring);
    }

    // this should fail - we are trying to assign a small dynamic a_v to a fixed_size larger one
    multi_span<int, dynamic_range> av = arr2;
    auto f = [&]() {
        const multi_span<int, 4> av2 = av;
        (void) av2;
    };
    EXPECT_DEATH(f(), deathstring);
}

TEST(multi_span_test, as_writeable_bytes)
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
        EXPECT_TRUE(wav.length() == av.length());
        EXPECT_TRUE(wav.length() == 0);
        EXPECT_TRUE(wav.size_bytes() == 0);
    }

    {
        multi_span<int, dynamic_range> av = a;
        auto wav = as_writeable_bytes(av);
        EXPECT_TRUE(wav.data() == reinterpret_cast<byte*>(&a[0]));
        EXPECT_TRUE(static_cast<std::size_t>(wav.length()) == sizeof(a));
    }
}

TEST(multi_span_test, iterator)
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
            EXPECT_TRUE(a[i] == 0);
        }
    }

    {
        multi_span<int, dynamic_range> av = a;
        for (auto& n : av) {
            n = 1;
        }
        for (std::size_t i = 0; i < 4; ++i) {
            EXPECT_TRUE(a[i] == 1);
        }
    }
}

#ifdef CONFIRM_COMPILATION_ERRORS
copy(src_span_static, dst_span_static);
#endif
