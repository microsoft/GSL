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
#include <gsl.h>
#include <functional>

using namespace gsl;

SUITE(utils_tests)
{
    void f(int& i)
    {
        i += 1;
    }

    TEST(finally_lambda)
    {
        int i = 0;
        {
            auto _ = finally([&]() {f(i);});
            CHECK(i == 0);
        }
        CHECK(i == 1);
    }

    TEST(finally_lambda_move)
    {
        int i = 0;
        {
            auto _1 = finally([&]() {f(i);});
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

    TEST(finally_function_with_bind)
    {
        int i = 0;
        {
            auto _ = finally(std::bind(&f, std::ref(i)));
            CHECK(i == 0);
        }
        CHECK(i == 1);
    }

    int j = 0;
    void g() { j += 1; };
    TEST(finally_function_ptr)
    {
        j = 0;
        {
            auto _ = finally(&g);
            CHECK(j == 0);
        }
        CHECK(j == 1);
    }

    TEST(narrow_cast)
    {
        int n = 120;
        char c = narrow_cast<char>(n);
        CHECK(c == 120);

        n = 300;
        unsigned char uc = narrow_cast<unsigned char>(n);
        CHECK(uc == 44);
    }

    TEST(narrow)
    {
        int n = 120;
        char c = narrow<char>(n);
        CHECK(c == 120);

        n = 300;
        CHECK_THROW(narrow<char>(n), narrowing_error);

        const auto int32_max = std::numeric_limits<int32_t>::max();
        const auto int32_min = std::numeric_limits<int32_t>::min();

        CHECK(narrow<uint32_t>(int32_t(0)) == 0);
        CHECK(narrow<uint32_t>(int32_t(1)) == 1);
        CHECK(narrow<uint32_t>(int32_max) == int32_max);

        CHECK_THROW(narrow<uint32_t>(int32_t(-1)), narrowing_error);
        CHECK_THROW(narrow<uint32_t>(int32_min), narrowing_error);

        n = -42;
        CHECK_THROW(narrow<unsigned>(n), narrowing_error);
    }
}

int main(int, const char *[])
{
    return UnitTest::RunAllTests();
}
