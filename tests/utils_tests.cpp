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

    TEST(finally_function_object)
    {
        struct Functor
        {
            bool destructed_ = false;
            int& callCount_;
            int& moveCount_;
            int& copyCount_;
        public:
            Functor(int& callCount, int& moveCount, int& copyCount) : callCount_(callCount), moveCount_(moveCount), copyCount_(copyCount) {}
            Functor(Functor&& other) : callCount_(other.callCount_), moveCount_(++other.moveCount_), copyCount_(other.copyCount_) {}
            Functor(const Functor& other) : callCount_(other.callCount_), moveCount_(other.moveCount_), copyCount_(++other.copyCount_) {}

            ~Functor()
            {
                Expects(destructed_ == false);
                destructed_ = true;
            }
            void operator()()
            {
                Expects(destructed_ == false);
                ++callCount_;
            }
        };

        int callCount, copyCount, moveCount;
        {
            Functor functor(callCount, moveCount, copyCount);

            callCount = copyCount = moveCount = 0;
            auto _ = finally(functor);
            CHECK(callCount == 0);
            CHECK(copyCount == 1);
            CHECK(moveCount == 0);

            {
                callCount = copyCount = moveCount = 0;
                auto _2 = decltype(_)(std::move(_));
                CHECK(callCount == 0);
                CHECK(copyCount == 0);
                CHECK(moveCount == 1);
            }
            CHECK(callCount == 1);

            {
                callCount = copyCount = moveCount = 0;
                auto _2 = finally(std::move(functor));
                CHECK(callCount == 0);
                CHECK(copyCount == 0);
                CHECK(moveCount == 1);
            }
            CHECK(callCount == 1);
        }
        CHECK(callCount == 1);
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
    }
}

int main(int, const char *[])
{
    return UnitTest::RunAllTests();
}
