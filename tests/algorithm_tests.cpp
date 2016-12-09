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

#include <UnitTest++/UnitTest++.h>
#include <gsl/gsl_algorithm>
#include <iostream>

#include <array>

using namespace std;
using namespace gsl;

SUITE(copy_tests)
{

    TEST(same_type)
    {
        // dynamic source and destination span
        {
            std::array<int, 5> src{1, 2, 3, 4, 5};
            std::array<int, 10> dst{};

            span<int> src_span(src);
            span<int> dst_span(dst);

            copy(src_span, dst_span);
            copy(src_span, dst_span.subspan(src_span.size()));

            for (std::size_t i = 0; i < src.size(); ++i) {
                CHECK(dst[i] == src[i]);
                CHECK(dst[i + src.size()] == src[i]);
            }
        }

        // static source and dynamic destination span
        {
            std::array<int, 5> src{1, 2, 3, 4, 5};
            std::array<int, 10> dst{};

            span<int, 5> src_span(src);
            span<int> dst_span(dst);

            copy(src_span, dst_span);
            copy(src_span, dst_span.subspan(src_span.size()));

            for (std::size_t i = 0; i < src.size(); ++i) {
                CHECK(dst[i] == src[i]);
                CHECK(dst[i + src.size()] == src[i]);
            }
        }

        // dynamic source and static destination span
        {
            std::array<int, 5> src{1, 2, 3, 4, 5};
            std::array<int, 10> dst{};

            span<int> src_span(src);
            span<int, 10> dst_span(dst);

            copy(src_span, dst_span);
            copy(src_span, dst_span.subspan(src_span.size()));

            for (std::size_t i = 0; i < src.size(); ++i) {
                CHECK(dst[i] == src[i]);
                CHECK(dst[i + src.size()] == src[i]);
            }
        }

        // static source and destination span
        {
            std::array<int, 5> src{1, 2, 3, 4, 5};
            std::array<int, 10> dst{};

            span<int, 5> src_span(src);
            span<int, 10> dst_span(dst);

            copy(src_span, dst_span);
            copy(src_span, dst_span.subspan(src_span.size()));

            for (std::size_t i = 0; i < src.size(); ++i) {
                CHECK(dst[i] == src[i]);
                CHECK(dst[i + src.size()] == src[i]);
            }
        }
    }

    TEST(compatible_type)
    {
        // dynamic source and destination span
        {
            std::array<int, 5> src{1, 2, 3, 4, 5};
            std::array<char, 10> dst{};

            span<int> src_span(src);
            span<char> dst_span(dst);

            copy(src_span, dst_span);
            copy(src_span, dst_span.subspan(src_span.size()));

            for (std::size_t i = 0; i < src.size(); ++i) {
                CHECK(dst[i] == src[i]);
                CHECK(dst[i + src.size()] == src[i]);
            }
        }

        // static source and dynamic destination span
        {
            std::array<int, 5> src{1, 2, 3, 4, 5};
            std::array<char, 10> dst{};

            span<int, 5> src_span(src);
            span<char> dst_span(dst);

            copy(src_span, dst_span);
            copy(src_span, dst_span.subspan(src_span.size()));

            for (std::size_t i = 0; i < src.size(); ++i) {
                CHECK(dst[i] == src[i]);
                CHECK(dst[i + src.size()] == src[i]);
            }
        }

        // dynamic source and static destination span
        {
            std::array<int, 5> src{1, 2, 3, 4, 5};
            std::array<char, 10> dst{};

            span<int> src_span(src);
            span<char, 10> dst_span(dst);

            copy(src_span, dst_span);
            copy(src_span, dst_span.subspan(src_span.size()));

            for (std::size_t i = 0; i < src.size(); ++i) {
                CHECK(dst[i] == src[i]);
                CHECK(dst[i + src.size()] == src[i]);
            }
        }

        // static source and destination span
        {
            std::array<int, 5> src{1, 2, 3, 4, 5};
            std::array<char, 10> dst{};

            span<int, 5> src_span(src);
            span<char, 10> dst_span(dst);

            copy(src_span, dst_span);
            copy(src_span, dst_span.subspan(src_span.size()));

            for (std::size_t i = 0; i < src.size(); ++i) {
                CHECK(dst[i] == src[i]);
                CHECK(dst[i + src.size()] == src[i]);
            }
        }
    }

#ifdef CONFIRM_COMPILATION_ERRORS
    TEST(incompatible_type)
    {
        std::array<int, 4> src{1, 2, 3, 4};
        std::array<int*, 12> dst{};

        span<int> src_span_dyn(src);
        span<int, 4> src_span_static(src);
        span<int*> dst_span_dyn(dst);
        span<int*, 4> dst_span_static(dst);

        // every line should produce a compilation error
        copy(src_span_dyn,    dst_span_dyn);
        copy(src_span_dyn,    dst_span_static);
        copy(src_span_static, dst_span_dyn);
        copy(src_span_static, dst_span_static);
    }
#endif

    TEST(small_destination_span)
    {
        std::array<int, 12> src{1, 2, 3, 4};
        std::array<int, 4> dst{};

        span<int> src_span_dyn(src);
        span<int, 12> src_span_static(src);
        span<int> dst_span_dyn(dst);
        span<int, 4> dst_span_static(dst);

        CHECK_THROW(copy(src_span_dyn,    dst_span_dyn),    fail_fast);
        CHECK_THROW(copy(src_span_dyn,    dst_span_static), fail_fast);
        CHECK_THROW(copy(src_span_static, dst_span_dyn),    fail_fast);

#ifdef CONFIRM_COMPILATION_ERRORS
        copy(src_span_static, dst_span_static);
#endif
    }

	TEST(algos)
	{
		std::array<int, 7> data1{ 3,7,1,1,5,2,3 };
		auto data2 = data1;
		gsl::span<int> span1(data1);
		gsl::span<int> span2(data2);

		// find
		auto pred = [](int i) -> bool { return i == 1; };
		auto cmp = [](int l, int r) -> bool { return l > r; };
		auto val = 5;
		auto pos = 3;
		CHECK(find			(span1, val) == std::find(span1.begin(), span1.end(), val));
		CHECK(find_if		(span1, pred) == std::find_if	 (span1.begin(), span1.end(), pred));
		CHECK(find_if_not	(span1, pred) == std::find_if_not(span1.begin(), span1.end(), pred));

		sort(span1);
		std::sort(span2.begin(), span2.end());
		CHECK(span1 == span2);
		CHECK(binary_search(span1, val) == std::binary_search(span1.begin(), span1.end(), val));

		CHECK(lower_bound(span1, val) == std::lower_bound(span1.begin(), span1.end(), val));
		CHECK(lower_bound(span1, val, cmp) == std::lower_bound(span1.begin(), span1.end(), val, cmp));

		CHECK(upper_bound(span1, val) == std::upper_bound(span1.begin(), span1.end(), val));
		CHECK(upper_bound(span1, val, cmp) == std::upper_bound(span1.begin(), span1.end(), val, cmp));

		sort(span1, cmp);
		std::sort(span2.begin(), span2.end(), cmp);
		CHECK(span1 == span2);
		CHECK(binary_search(span1, val, cmp) == std::binary_search(span1.begin(), span1.end(), val, cmp));

		stable_sort(span1);
		std::stable_sort(span2.begin(), span2.end());
		CHECK(span1 == span2);

		stable_sort(span1,cmp);
		std::stable_sort(span2.begin(), span2.end(), cmp);
		CHECK(span1 == span2);

		nth_element(span1, pos);
		std::nth_element(span2.begin(), span2.begin() + pos, span2.end());
		CHECK(span1[pos] == span2[pos]);

		nth_element(span1, pos, cmp);
		std::nth_element(span2.begin(), span2.begin() + pos, span2.end(), cmp);
		CHECK(span1[pos] == span2[pos]);
	}
}

int main(int, const char* []) { return UnitTest::RunAllTests(); }
