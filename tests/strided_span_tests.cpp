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
#include <gsl/multi_span> // for strided_span, index, multi_span, strided_...

#include <iostream>    // for size_t
#include <iterator>    // for begin, end
#include <numeric>     // for iota
#include <type_traits> // for integral_constant<>::value, is_convertible
#include <vector>      // for vector

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

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.1) // NO-FORMAT: attribute
void iterate_every_other_element(multi_span<int, dynamic_range> av)
{
    // pick every other element

    auto length = av.size() / 2;
#if defined(_MSC_VER) && _MSC_VER > 1800
    auto bounds = strided_bounds<1>({length}, {2});
#else
    auto bounds = strided_bounds<1>(multi_span_index<1>{length}, multi_span_index<1>{2});
#endif
    strided_span<int, 1> strided(&av.data()[1], av.size() - 1, bounds);

    EXPECT_TRUE(strided.size() == length);
    EXPECT_TRUE(strided.bounds().index_bounds()[0] == length);
    for (auto i = 0; i < strided.size(); ++i) {
        EXPECT_TRUE(strided[i] == av[2 * i + 1]);
    }

    int idx = 0;
    for (auto num : strided) {
        EXPECT_TRUE(num == av[2 * idx + 1]);
        idx++;
    }
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.4) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.2) // NO-FORMAT: attribute // TODO: does not work
void iterate_second_slice(multi_span<int, dynamic_range, dynamic_range, dynamic_range> av)
{
    const int expected[6] = {2, 3, 10, 11, 18, 19};
    auto section = av.section({0, 1, 0}, {3, 1, 2});

    for (auto i = 0; i < section.extent<0>(); ++i) {
        for (auto j = 0; j < section.extent<1>(); ++j)
            for (auto k = 0; k < section.extent<2>(); ++k) {
                auto idx = multi_span_index<3>{i, j, k}; // avoid braces in the EXPECT_TRUE macro
                EXPECT_TRUE(section[idx] == expected[2 * i + 2 * j + k]);
            }
    }

    for (auto i = 0; i < section.extent<0>(); ++i) {
        for (auto j = 0; j < section.extent<1>(); ++j)
            for (auto k = 0; k < section.extent<2>(); ++k)
                EXPECT_TRUE(section[i][j][k] == expected[2 * i + 2 * j + k]);
    }

    int i = 0;
    for (const auto num : section) {
        EXPECT_TRUE(num == expected[i]);
        i++;
    }
}

}

TEST(strided_span_tests, span_section_test)
{
    int a[30][4][5];

    const auto av = as_multi_span(a);
    const auto sub = av.section({15, 0, 0}, gsl::multi_span_index<3>{2, 2, 2});
    const auto subsub = sub.section({1, 0, 0}, gsl::multi_span_index<3>{1, 1, 1});
    (void) subsub;
}

TEST(strided_span_tests, span_section)
{
    std::vector<int> data(5 * 10);
    std::iota(begin(data), end(data), 0);
    const multi_span<int, 5, 10> av = as_multi_span(multi_span<int>{data}, dim<5>(), dim<10>());

    const strided_span<int, 2> av_section_1 = av.section({1, 2}, {3, 4});
    EXPECT_TRUE(!av_section_1.empty());
    EXPECT_TRUE((av_section_1[{0, 0}] == 12));
    EXPECT_TRUE((av_section_1[{0, 1}] == 13));
    EXPECT_TRUE((av_section_1[{1, 0}] == 22));
    EXPECT_TRUE((av_section_1[{2, 3}] == 35));

    const strided_span<int, 2> av_section_2 = av_section_1.section({1, 2}, {2, 2});
    EXPECT_TRUE(!av_section_2.empty());
    EXPECT_TRUE((av_section_2[{0, 0}] == 24));
    EXPECT_TRUE((av_section_2[{0, 1}] == 25));
    EXPECT_TRUE((av_section_2[{1, 0}] == 34));
}

TEST(strided_span_tests, strided_span_constructors)
{
    // EXPECT_TRUE stride constructor
    {
        int arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
        const int carr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

        strided_span<int, 1> sav1{arr, {{9}, {1}}}; // T -> T
        EXPECT_TRUE(sav1.bounds().index_bounds() == multi_span_index<1>{9});
        EXPECT_TRUE(sav1.bounds().stride() == 1);
        EXPECT_TRUE(sav1[0] == 1);
        EXPECT_TRUE(sav1[8] == 9);

        strided_span<const int, 1> sav2{carr, {{4}, {2}}}; // const T -> const T
        EXPECT_TRUE(sav2.bounds().index_bounds() == multi_span_index<1>{4});
        EXPECT_TRUE(sav2.bounds().strides() == multi_span_index<1>{2});
        EXPECT_TRUE(sav2[0] == 1);
        EXPECT_TRUE(sav2[3] == 7);

        strided_span<int, 2> sav3{arr, {{2, 2}, {6, 2}}}; // T -> const T
        EXPECT_TRUE((sav3.bounds().index_bounds() == multi_span_index<2>{2, 2}));
        EXPECT_TRUE((sav3.bounds().strides() == multi_span_index<2>{6, 2}));
        EXPECT_TRUE((sav3[{0, 0}]) == 1);
        EXPECT_TRUE((sav3[{0, 1}]) ==  3);
        EXPECT_TRUE((sav3[{1, 0}]) == 7);
    }

    // EXPECT_TRUE multi_span constructor
    {
        int arr[] = {1, 2};

        // From non-cv-qualified source
        {
            const multi_span<int> src = arr;

            strided_span<int, 1> sav{src, {2, 1}};
            EXPECT_TRUE(sav.bounds().index_bounds() == multi_span_index<1>{2});
            EXPECT_TRUE(sav.bounds().strides() == multi_span_index<1>{1});
            EXPECT_TRUE(sav[1] == 2);

#if defined(_MSC_VER) && _MSC_VER > 1800
            // strided_span<const int, 1> sav_c{ {src}, {2, 1} };
            strided_span<const int, 1> sav_c{multi_span<const int>{src},
                                             strided_bounds<1>{2, 1}};
#else
            strided_span<const int, 1> sav_c{multi_span<const int>{src},
                                             strided_bounds<1>{2, 1}};
#endif
            EXPECT_TRUE(sav_c.bounds().index_bounds() == multi_span_index<1>{2});
            EXPECT_TRUE(sav_c.bounds().strides() == multi_span_index<1>{1});
            EXPECT_TRUE(sav_c[1] == 2);

#if defined(_MSC_VER) && _MSC_VER > 1800
            strided_span<volatile int, 1> sav_v{src, {2, 1}};
#else
            strided_span<volatile int, 1> sav_v{multi_span<volatile int>{src},
                                                strided_bounds<1>{2, 1}};
#endif
            EXPECT_TRUE(sav_v.bounds().index_bounds() == multi_span_index<1>{2});
            EXPECT_TRUE(sav_v.bounds().strides() == multi_span_index<1>{1});
            EXPECT_TRUE(sav_v[1] == 2);

#if defined(_MSC_VER) && _MSC_VER > 1800
            strided_span<const volatile int, 1> sav_cv{src, {2, 1}};
#else
            strided_span<const volatile int, 1> sav_cv{multi_span<const volatile int>{src},
                                                       strided_bounds<1>{2, 1}};
#endif
            EXPECT_TRUE(sav_cv.bounds().index_bounds() == multi_span_index<1>{2});
            EXPECT_TRUE(sav_cv.bounds().strides() == multi_span_index<1>{1});
            EXPECT_TRUE(sav_cv[1] == 2);
        }

        // From const-qualified source
        {
            const multi_span<const int> src{arr};

            strided_span<const int, 1> sav_c{src, {2, 1}};
            EXPECT_TRUE(sav_c.bounds().index_bounds() == multi_span_index<1>{2});
            EXPECT_TRUE(sav_c.bounds().strides() == multi_span_index<1>{1});
            EXPECT_TRUE(sav_c[1] == 2);

#if defined(_MSC_VER) && _MSC_VER > 1800
            strided_span<const volatile int, 1> sav_cv{src, {2, 1}};
#else
            strided_span<const volatile int, 1> sav_cv{multi_span<const volatile int>{src},
                                                       strided_bounds<1>{2, 1}};
#endif

            EXPECT_TRUE(sav_cv.bounds().index_bounds() == multi_span_index<1>{2});
            EXPECT_TRUE(sav_cv.bounds().strides() == multi_span_index<1>{1});
            EXPECT_TRUE(sav_cv[1] == 2);
        }

        // From volatile-qualified source
        {
            const multi_span<volatile int> src{arr};

            strided_span<volatile int, 1> sav_v{src, {2, 1}};
            EXPECT_TRUE(sav_v.bounds().index_bounds() == multi_span_index<1>{2});
            EXPECT_TRUE(sav_v.bounds().strides() == multi_span_index<1>{1});
            EXPECT_TRUE(sav_v[1] == 2);

#if defined(_MSC_VER) && _MSC_VER > 1800
            strided_span<const volatile int, 1> sav_cv{src, {2, 1}};
#else
            strided_span<const volatile int, 1> sav_cv{multi_span<const volatile int>{src},
                                                       strided_bounds<1>{2, 1}};
#endif
            EXPECT_TRUE(sav_cv.bounds().index_bounds() == multi_span_index<1>{2});
            EXPECT_TRUE(sav_cv.bounds().strides() == multi_span_index<1>{1});
            EXPECT_TRUE(sav_cv[1] == 2);
        }

        // From cv-qualified source
        {
            const multi_span<const volatile int> src{arr};

            strided_span<const volatile int, 1> sav_cv{src, {2, 1}};
            EXPECT_TRUE(sav_cv.bounds().index_bounds() == multi_span_index<1>{2});
            EXPECT_TRUE(sav_cv.bounds().strides() == multi_span_index<1>{1});
            EXPECT_TRUE(sav_cv[1] == 2);
        }
    }

    // EXPECT_TRUE const-casting constructor
    {
        int arr[2] = {4, 5};

        const multi_span<int, 2> av(arr, 2);
        multi_span<const int, 2> av2{av};
        EXPECT_TRUE(av2[1] == 5);

        static_assert(
            std::is_convertible<const multi_span<int, 2>, multi_span<const int, 2>>::value,
            "ctor is not implicit!");

        const strided_span<int, 1> src{arr, {2, 1}};
        strided_span<const int, 1> sav{src};
        EXPECT_TRUE(sav.bounds().index_bounds() == multi_span_index<1>{2});
        EXPECT_TRUE(sav.bounds().stride() == 1);
        EXPECT_TRUE(sav[1] == 5);

        static_assert(
            std::is_convertible<const strided_span<int, 1>, strided_span<const int, 1>>::value,
            "ctor is not implicit!");
    }

    // EXPECT_TRUE copy constructor
    {
        int arr1[2] = {3, 4};
        const strided_span<int, 1> src1{arr1, {2, 1}};
        strided_span<int, 1> sav1{src1};

        EXPECT_TRUE(sav1.bounds().index_bounds() == multi_span_index<1>{2});
        EXPECT_TRUE(sav1.bounds().stride() == 1);
        EXPECT_TRUE(sav1[0] == 3);

        int arr2[6] = {1, 2, 3, 4, 5, 6};
        const strided_span<const int, 2> src2{arr2, {{3, 2}, {2, 1}}};
        strided_span<const int, 2> sav2{src2};
        EXPECT_TRUE((sav2.bounds().index_bounds() == multi_span_index<2>{3, 2}));
        EXPECT_TRUE((sav2.bounds().strides() == multi_span_index<2>{2, 1}));
        EXPECT_TRUE((sav2[{0, 0}]) ==  1);
        EXPECT_TRUE((sav2[{2, 0}]) == 5);
    }

    // EXPECT_TRUE const-casting assignment operator
    {
        int arr1[2] = {1, 2};
        int arr2[6] = {3, 4, 5, 6, 7, 8};

        const strided_span<int, 1> src{arr1, {{2}, {1}}};
        strided_span<const int, 1> sav{arr2, {{3}, {2}}};
        strided_span<const int, 1>& sav_ref = (sav = src);
        EXPECT_TRUE(sav.bounds().index_bounds() == multi_span_index<1>{2});
        EXPECT_TRUE(sav.bounds().strides() == multi_span_index<1>{1});
        EXPECT_TRUE(sav[0] == 1);
        EXPECT_TRUE(&sav_ref == &sav);
    }

    // EXPECT_TRUE copy assignment operator
    {
        int arr1[2] = {3, 4};
        int arr1b[1] = {0};
        const strided_span<int, 1> src1{arr1, {2, 1}};
        strided_span<int, 1> sav1{arr1b, {1, 1}};
        strided_span<int, 1>& sav1_ref = (sav1 = src1);
        EXPECT_TRUE(sav1.bounds().index_bounds() == multi_span_index<1>{2});
        EXPECT_TRUE(sav1.bounds().strides() == multi_span_index<1>{1});
        EXPECT_TRUE(sav1[0] == 3);
        EXPECT_TRUE(&sav1_ref == &sav1);

        const int arr2[6] = {1, 2, 3, 4, 5, 6};
        const int arr2b[1] = {0};
        const strided_span<const int, 2> src2{arr2, {{3, 2}, {2, 1}}};
        strided_span<const int, 2> sav2{arr2b, {{1, 1}, {1, 1}}};
        strided_span<const int, 2>& sav2_ref = (sav2 = src2);
        EXPECT_TRUE((sav2.bounds().index_bounds() == multi_span_index<2>{3, 2}));
        EXPECT_TRUE((sav2.bounds().strides() == multi_span_index<2>{2, 1}));
        EXPECT_TRUE((sav2[{0, 0}] == 1));
        EXPECT_TRUE((sav2[{2, 0}] == 5));
        EXPECT_TRUE(&sav2_ref == &sav2);
    }
}

TEST(strided_span_tests, strided_span_slice)
{
    std::vector<int> data(5 * 10);
    std::iota(begin(data), end(data), 0);
    const multi_span<int, 5, 10> src =
        as_multi_span(multi_span<int>{data}, dim<5>(), dim<10>());

    const strided_span<int, 2> sav{src, {{5, 10}, {10, 1}}};
#ifdef CONFIRM_COMPILATION_ERRORS
    const strided_span<const int, 2> csav{{src}, {{5, 10}, {10, 1}}};
#endif
    const strided_span<const int, 2> csav{multi_span<const int, 5, 10>{src},
                                          {{5, 10}, {10, 1}}};

    strided_span<int, 1> sav_sl = sav[2];
    EXPECT_TRUE(sav_sl[0] == 20);
    EXPECT_TRUE(sav_sl[9] == 29);

    strided_span<const int, 1> csav_sl = sav[3];
    EXPECT_TRUE(csav_sl[0] == 30);
    EXPECT_TRUE(csav_sl[9] == 39);

    EXPECT_TRUE(sav[4][0] == 40);
    EXPECT_TRUE(sav[4][9] == 49);
}

TEST(strided_span_tests, strided_span_column_major)
{
    // strided_span may be used to accommodate more peculiar
    // use cases, such as column-major multidimensional array
    // (aka. "FORTRAN" layout).

    int cm_array[3 * 5] = {1, 4, 7, 10, 13, 2, 5, 8, 11, 14, 3, 6, 9, 12, 15};
    strided_span<int, 2> cm_sav{cm_array, {{5, 3}, {1, 5}}};

    // Accessing elements
    EXPECT_TRUE((cm_sav[{0, 0}] == 1));
    EXPECT_TRUE((cm_sav[{0, 1}] == 2));
    EXPECT_TRUE((cm_sav[{1, 0}] == 4));
    EXPECT_TRUE((cm_sav[{4, 2}] == 15));

    // Slice
    strided_span<int, 1> cm_sl = cm_sav[3];

    EXPECT_TRUE(cm_sl[0] == 10);
    EXPECT_TRUE(cm_sl[1] == 11);
    EXPECT_TRUE(cm_sl[2] == 12);

    // Section
    strided_span<int, 2> cm_sec = cm_sav.section({2, 1}, {3, 2});

    EXPECT_TRUE((cm_sec.bounds().index_bounds() == multi_span_index<2>{3, 2}));
    EXPECT_TRUE((cm_sec[{0, 0}] == 8));
    EXPECT_TRUE((cm_sec[{0, 1}] == 9));
    EXPECT_TRUE((cm_sec[{1, 0}] == 11));
    EXPECT_TRUE((cm_sec[{2, 1}] == 15));
}

TEST(strided_span_tests, strided_span_bounds)
{
    int arr[] = {0, 1, 2, 3};
    multi_span<int> av(arr);

    std::set_terminate([] {
        std::cerr << "Expected Death. strided_span_bounds";
        std::abort();
    });

    {
        // incorrect sections

        EXPECT_DEATH(av.section(0, 0)[0], deathstring);
        EXPECT_DEATH(av.section(1, 0)[0], deathstring);
        EXPECT_DEATH(av.section(1, 1)[1], deathstring);

        EXPECT_DEATH(av.section(2, 5), deathstring);
        EXPECT_DEATH(av.section(5, 2), deathstring);
        EXPECT_DEATH(av.section(5, 0), deathstring);
        EXPECT_DEATH(av.section(0, 5), deathstring);
        EXPECT_DEATH(av.section(5, 5), deathstring);
    }

    {
        // zero stride
        strided_span<int, 1> sav{av, {{4}, {}}};
        EXPECT_TRUE(sav[0] == 0);
        EXPECT_TRUE(sav[3] == 0);
        EXPECT_DEATH(sav[4], deathstring);
    }

    {
        // zero extent
        strided_span<int, 1> sav{av, {{}, {1}}};
        EXPECT_DEATH(sav[0], deathstring);
    }

    {
        // zero extent and stride
        strided_span<int, 1> sav{av, {{}, {}}};
        EXPECT_DEATH(sav[0], deathstring);
    }

    {
        // strided array ctor with matching strided bounds
        strided_span<int, 1> sav{arr, {4, 1}};
        EXPECT_TRUE(sav.bounds().index_bounds() == multi_span_index<1>{4});
        EXPECT_TRUE(sav[3] == 3);
        EXPECT_DEATH(sav[4], deathstring);
    }

    {
        // strided array ctor with smaller strided bounds
        strided_span<int, 1> sav{arr, {2, 1}};
        EXPECT_TRUE(sav.bounds().index_bounds() == multi_span_index<1>{2});
        EXPECT_TRUE(sav[1] == 1);
        EXPECT_DEATH(sav[2], deathstring);
    }

    {
        // strided array ctor with fitting irregular bounds
        strided_span<int, 1> sav{arr, {2, 3}};
        EXPECT_TRUE(sav.bounds().index_bounds() == multi_span_index<1>{2});
        EXPECT_TRUE(sav[0] == 0);
        EXPECT_TRUE(sav[1] == 3);
        EXPECT_DEATH(sav[2], deathstring);
    }

    {
        // bounds cross data boundaries - from static arrays
        EXPECT_DEATH((strided_span<int, 1>{arr, {3, 2}}), deathstring);
        EXPECT_DEATH((strided_span<int, 1>{arr, {3, 3}}), deathstring);
        EXPECT_DEATH((strided_span<int, 1>{arr, {4, 5}}), deathstring);
        EXPECT_DEATH((strided_span<int, 1>{arr, {5, 1}}), deathstring);
        EXPECT_DEATH((strided_span<int, 1>{arr, {5, 5}}), deathstring);
    }

    {
        // bounds cross data boundaries - from array view
        EXPECT_DEATH((strided_span<int, 1>{av, {3, 2}}), deathstring);
        EXPECT_DEATH((strided_span<int, 1>{av, {3, 3}}), deathstring);
        EXPECT_DEATH((strided_span<int, 1>{av, {4, 5}}), deathstring);
        EXPECT_DEATH((strided_span<int, 1>{av, {5, 1}}), deathstring);
        EXPECT_DEATH((strided_span<int, 1>{av, {5, 5}}), deathstring);
    }

    {
        // bounds cross data boundaries - from dynamic arrays
        EXPECT_DEATH((strided_span<int, 1>{av.data(), 4, {3, 2}}), deathstring);
        EXPECT_DEATH((strided_span<int, 1>{av.data(), 4, {3, 3}}), deathstring);
        EXPECT_DEATH((strided_span<int, 1>{av.data(), 4, {4, 5}}), deathstring);
        EXPECT_DEATH((strided_span<int, 1>{av.data(), 4, {5, 1}}), deathstring);
        EXPECT_DEATH((strided_span<int, 1>{av.data(), 4, {5, 5}}), deathstring);
        EXPECT_DEATH((strided_span<int, 1>{av.data(), 2, {2, 2}}), deathstring);
    }

#ifdef CONFIRM_COMPILATION_ERRORS
    {
        strided_span<int, 1> sav0{av.data(), {3, 2}};
        strided_span<int, 1> sav1{arr, {1}};
        strided_span<int, 1> sav2{arr, {1, 1, 1}};
        strided_span<int, 1> sav3{av, {1}};
        strided_span<int, 1> sav4{av, {1, 1, 1}};
        strided_span<int, 2> sav5{av.as_multi_span(dim<2>(), dim<2>()), {1}};
        strided_span<int, 2> sav6{av.as_multi_span(dim<2>(), dim<2>()), {1, 1, 1}};
        strided_span<int, 2> sav7{av.as_multi_span(dim<2>(), dim<2>()),
                                  {{1, 1}, {1, 1}, {1, 1}}};

        multi_span_index<1> index{0, 1};
        strided_span<int, 1> sav8{arr, {1, {1, 1}}};
        strided_span<int, 1> sav9{arr, {{1, 1}, {1, 1}}};
        strided_span<int, 1> sav10{av, {1, {1, 1}}};
        strided_span<int, 1> sav11{av, {{1, 1}, {1, 1}}};
        strided_span<int, 2> sav12{av.as_multi_span(dim<2>(), dim<2>()), {{1}, {1}}};
        strided_span<int, 2> sav13{av.as_multi_span(dim<2>(), dim<2>()), {{1}, {1, 1, 1}}};
        strided_span<int, 2> sav14{av.as_multi_span(dim<2>(), dim<2>()), {{1, 1, 1}, {1}}};
    }
#endif
}

TEST(strided_span_tests, strided_span_type_conversion)
{
    int arr[] = {0, 1, 2, 3};
    multi_span<int> av(arr);

    std::set_terminate([] {
        std::cerr << "Expected Death. strided_span_type_conversion";
        std::abort();
    });

    {
        strided_span<int, 1> sav{av.data(), av.size(), {av.size() / 2, 2}};
#ifdef CONFIRM_COMPILATION_ERRORS
        strided_span<long, 1> lsav1 = sav.as_strided_span<long, 1>();
#endif
    }
    {
        strided_span<int, 1> sav{av, {av.size() / 2, 2}};
#ifdef CONFIRM_COMPILATION_ERRORS
        strided_span<long, 1> lsav1 = sav.as_strided_span<long, 1>();
#endif
    }

    multi_span<const byte, dynamic_range> bytes = as_bytes(av);

    // retype strided array with regular strides - from raw data
    {
        strided_bounds<2> bounds{{2, bytes.size() / 4}, {bytes.size() / 2, 1}};
        strided_span<const byte, 2> sav2{bytes.data(), bytes.size(), bounds};
        strided_span<const int, 2> sav3 = sav2.as_strided_span<const int>();
        EXPECT_TRUE(sav3[0][0] == 0);
        EXPECT_TRUE(sav3[1][0] == 2);
        EXPECT_DEATH(sav3[1][1], deathstring);
        EXPECT_DEATH(sav3[0][1], deathstring);
    }

    // retype strided array with regular strides - from multi_span
    {
        strided_bounds<2> bounds{{2, bytes.size() / 4}, {bytes.size() / 2, 1}};
        multi_span<const byte, 2, dynamic_range> bytes2 =
            as_multi_span(bytes, dim<2>(), dim(bytes.size() / 2));
        strided_span<const byte, 2> sav2{bytes2, bounds};
        strided_span<int, 2> sav3 = sav2.as_strided_span<int>();
        EXPECT_TRUE(sav3[0][0] == 0);
        EXPECT_TRUE(sav3[1][0] == 2);
        EXPECT_DEATH(sav3[1][1], deathstring);
        EXPECT_DEATH(sav3[0][1], deathstring);
    }

    // retype strided array with not enough elements - last dimension of the array is too small
    {
        strided_bounds<2> bounds{{4, 2}, {4, 1}};
        multi_span<const byte, 2, dynamic_range> bytes2 =
            as_multi_span(bytes, dim<2>(), dim(bytes.size() / 2));
        strided_span<const byte, 2> sav2{bytes2, bounds};
        EXPECT_DEATH(sav2.as_strided_span<int>(), deathstring);
    }

    // retype strided array with not enough elements - strides are too small
    {
        strided_bounds<2> bounds{{4, 2}, {2, 1}};
        multi_span<const byte, 2, dynamic_range> bytes2 =
            as_multi_span(bytes, dim<2>(), dim(bytes.size() / 2));
        strided_span<const byte, 2> sav2{bytes2, bounds};
        EXPECT_DEATH(sav2.as_strided_span<int>(), deathstring);
    }

    // retype strided array with not enough elements - last dimension does not divide by the new
    // typesize
    {
        strided_bounds<2> bounds{{2, 6}, {4, 1}};
        multi_span<const byte, 2, dynamic_range> bytes2 =
            as_multi_span(bytes, dim<2>(), dim(bytes.size() / 2));
        strided_span<const byte, 2> sav2{bytes2, bounds};
        EXPECT_DEATH(sav2.as_strided_span<int>(), deathstring);
    }

    // retype strided array with not enough elements - strides does not divide by the new
    // typesize
    {
        strided_bounds<2> bounds{{2, 1}, {6, 1}};
        multi_span<const byte, 2, dynamic_range> bytes2 =
            as_multi_span(bytes, dim<2>(), dim(bytes.size() / 2));
        strided_span<const byte, 2> sav2{bytes2, bounds};
        EXPECT_DEATH(sav2.as_strided_span<int>(), deathstring);
    }

    // retype strided array with irregular strides - from raw data
    {
        strided_bounds<1> bounds{bytes.size() / 2, 2};
        strided_span<const byte, 1> sav2{bytes.data(), bytes.size(), bounds};
        EXPECT_DEATH(sav2.as_strided_span<int>(), deathstring);
    }

    // retype strided array with irregular strides - from multi_span
    {
        strided_bounds<1> bounds{bytes.size() / 2, 2};
        strided_span<const byte, 1> sav2{bytes, bounds};
        EXPECT_DEATH(sav2.as_strided_span<int>(), deathstring);
    }
}

TEST(strided_span_tests, empty_strided_spans)
{
    std::set_terminate([] {
        std::cerr << "Expected Death. empty_strided_spans";
        std::abort();
    });

    {
        multi_span<int, 0> empty_av(nullptr);
        strided_span<int, 1> empty_sav{empty_av, {0, 1}};

        EXPECT_TRUE(empty_sav.bounds().index_bounds() == multi_span_index<1>{0});
        EXPECT_TRUE(empty_sav.empty());
        EXPECT_DEATH(empty_sav[0], deathstring);
        EXPECT_DEATH(empty_sav.begin()[0], deathstring);
        EXPECT_DEATH(empty_sav.cbegin()[0], deathstring);

        for (const auto& v : empty_sav) {
            (void) v;
            EXPECT_TRUE(false);
        }
    }

    {
        strided_span<int, 1> empty_sav{nullptr, 0, {0, 1}};

        EXPECT_TRUE(empty_sav.bounds().index_bounds() == multi_span_index<1>{0});
        EXPECT_DEATH(empty_sav[0], deathstring);
        EXPECT_DEATH(empty_sav.begin()[0], deathstring);
        EXPECT_DEATH(empty_sav.cbegin()[0], deathstring);

        for (const auto& v : empty_sav) {
            (void) v;
            EXPECT_TRUE(false);
        }
    }
}

TEST(strided_span_tests, strided_span_section_iteration)
{
    int arr[8] = {4, 0, 5, 1, 6, 2, 7, 3};

    // static bounds
    {
        multi_span<int, 8> av(arr, 8);
        iterate_every_other_element(av);
    }

    // dynamic bounds
    {
        multi_span<int, dynamic_range> av(arr, 8);
        iterate_every_other_element(av);
    }
}

TEST(strided_span_tests, dynamic_strided_span_section_iteration)
{
    auto arr = new int[8];
    for (int i = 0; i < 4; ++i) {
        arr[2 * i] = 4 + i;
        arr[2 * i + 1] = i;
    }

    auto av = as_multi_span(arr, 8);
    iterate_every_other_element(av);

    delete[] arr;
}

TEST(strided_span_tests, strided_span_section_iteration_3d)
{
    int arr[3][4][2]{};
    for (auto i = 0; i < 3; ++i) {
        for (auto j = 0; j < 4; ++j)
            for (auto k = 0; k < 2; ++k) arr[i][j][k] = 8 * i + 2 * j + k;
    }

    {
        multi_span<int, 3, 4, 2> av = arr;
        iterate_second_slice(av);
    }
}

TEST(strided_span_tests, dynamic_strided_span_section_iteration_3d)
{
    const auto height = 12, width = 2;
    const auto size = height * width;

    auto arr = new int[static_cast<std::size_t>(size)];
    for (auto i = 0; i < size; ++i) {
        arr[i] = i;
    }

    {
        auto av = as_multi_span(as_multi_span(arr, 24), dim<3>(), dim<4>(), dim<2>());
        iterate_second_slice(av);
    }

    {
        auto av = as_multi_span(as_multi_span(arr, 24), dim(3), dim<4>(), dim<2>());
        iterate_second_slice(av);
    }

    {
        auto av = as_multi_span(as_multi_span(arr, 24), dim<3>(), dim(4), dim<2>());
        iterate_second_slice(av);
    }

    {
        auto av = as_multi_span(as_multi_span(arr, 24), dim<3>(), dim<4>(), dim(2));
        iterate_second_slice(av);
    }
    delete[] arr;
}

TEST(strided_span_tests, strided_span_conversion)
{
    std::set_terminate([] {
        std::cerr << "Expected Death. strided_span_conversion";
        std::abort();
    });

    // get an multi_span of 'c' values from the list of X's

    struct X
    {
        int a;
        int b;
        int c;
    };

    X arr[4] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {9, 10, 11}};

    int s = sizeof(int) / sizeof(byte);
    auto d2 = 3 * s;
    auto d1 = narrow_cast<int>(sizeof(int)) * 12 / d2;

    // convert to 4x12 array of bytes
    auto av = as_multi_span(as_bytes(as_multi_span(&arr[0], 4)), dim(d1), dim(d2));

    EXPECT_TRUE(av.bounds().index_bounds()[0] == 4);
    EXPECT_TRUE(av.bounds().index_bounds()[1] == 12);

    // get the last 4 columns
    auto section = av.section({0, 2 * s}, {4, s}); // { { arr[0].c[0], arr[0].c[1], arr[0].c[2],
                                                   // arr[0].c[3] } , { arr[1].c[0], ... } , ...
                                                   // }

    // convert to array 4x1 array of integers
    auto cs = section.as_strided_span<int>(); // { { arr[0].c }, {arr[1].c } , ... }

    EXPECT_TRUE(cs.bounds().index_bounds()[0] == 4);
    EXPECT_TRUE(cs.bounds().index_bounds()[1] == 1);

    // transpose to 1x4 array
    strided_bounds<2> reverse_bounds{
        {cs.bounds().index_bounds()[1], cs.bounds().index_bounds()[0]},
        {cs.bounds().strides()[1], cs.bounds().strides()[0]}};

    strided_span<int, 2> transposed{cs.data(), cs.bounds().total_size(), reverse_bounds};

    // slice to get a one-dimensional array of c's
    strided_span<int, 1> result = transposed[0];

    EXPECT_TRUE(result.bounds().index_bounds()[0] == 4);
    EXPECT_DEATH(result.bounds().index_bounds()[1], deathstring);

    int i = 0;
    for (auto& num : result) {
        EXPECT_TRUE(num == arr[i].c);
        i++;
    }
}
