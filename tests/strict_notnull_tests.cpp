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
#include <gsl/pointers>           // for not_null, operator<, operator<=, operator>

namespace gsl
{
struct fail_fast;
} // namespace gsl

using namespace gsl;

GSL_SUPPRESS(f.4)  // NO-FORMAT: attribute
bool helper(not_null<int*> p) { return *p == 12; }

GSL_SUPPRESS(f.4) // NO-FORMAT: attribute
bool helper_const(not_null<const int*> p) { return *p == 12; }

GSL_SUPPRESS(f.4) // NO-FORMAT: attribute
bool strict_helper(strict_not_null<int*> p) { return *p == 12; }

GSL_SUPPRESS(f.4) // NO-FORMAT: attribute
bool strict_helper_const(strict_not_null<const int*> p) { return *p == 12; }

int* return_pointer() { return nullptr; }
const int* return_pointer_const() { return nullptr; }

TEST(strict_notnull_tests, TestStrictNotNull)
{
    {
        // raw ptr <-> strict_not_null
        int x = 42;

#ifdef CONFIRM_COMPILATION_ERRORS
        strict_not_null<int*> snn = &x;
        strict_helper(&x);
        strict_helper_const(&x);
        strict_helper(return_pointer());
        strict_helper_const(return_pointer_const());
#endif

        const strict_not_null<int*> snn1{&x};

        helper(snn1);
        helper_const(snn1);

        EXPECT_TRUE(*snn1 == 42);
    }

    {
        // strict_not_null -> strict_not_null
        int x = 42;

        strict_not_null<int*> snn1{&x};
        const strict_not_null<int*> snn2{&x};

        strict_helper(snn1);
        strict_helper_const(snn1);
        strict_helper_const(snn2);

        EXPECT_TRUE(snn1 == snn2);
    }

    {
        // strict_not_null -> not_null
        int x = 42;

        strict_not_null<int*> snn{&x};

        const not_null<int*> nn1 = snn;
        const not_null<int*> nn2{snn};

        helper(snn);
        helper_const(snn);

        EXPECT_TRUE(snn == nn1);
        EXPECT_TRUE(snn == nn2);
    }

    {
        // not_null -> strict_not_null
        int x = 42;

        not_null<int*> nn{&x};

        const strict_not_null<int*> snn1{nn};
        const strict_not_null<int*> snn2{nn};

        strict_helper(nn);
        strict_helper_const(nn);

        EXPECT_TRUE(snn1 == nn);
        EXPECT_TRUE(snn2 == nn);

        std::hash<strict_not_null<int*>> hash_snn;
        std::hash<not_null<int*>> hash_nn;

        EXPECT_TRUE(hash_nn(snn1) == hash_nn(nn));
        EXPECT_TRUE(hash_snn(snn1) == hash_nn(nn));
        EXPECT_TRUE(hash_nn(snn1) == hash_nn(snn2));
        EXPECT_TRUE(hash_snn(snn1) == hash_snn(nn));
    }

#ifdef CONFIRM_COMPILATION_ERRORS
    {
        strict_not_null<int*> p{nullptr};
    }
#endif
}

#if defined(__cplusplus) && (__cplusplus >= 201703L)
namespace
{
static constexpr char deathstring[] = "Expected Death";
}

TEST(strict_notnull_tests, TestStrictNotNullConstructorTypeDeduction)
{
    std::set_terminate([] {
        std::cerr << "Expected Death. TestStrictNotNullConstructorTypeDeduction";
        std::abort();
    });

    {
        int i = 42;

        strict_not_null x{&i};
        helper(strict_not_null{&i});
        helper_const(strict_not_null{&i});

        EXPECT_TRUE(*x == 42);
    }

    {
        int i = 42;
        int* p = &i;

        strict_not_null x{p};
        helper(strict_not_null{p});
        helper_const(strict_not_null{p});

        EXPECT_TRUE(*x == 42);
    }

    {
        auto workaround_macro = []() {
            int* p1 = nullptr;
            const strict_not_null x{p1};
        };
        EXPECT_DEATH(workaround_macro(), deathstring);
    }

    {
        auto workaround_macro = []() {
            const int* p1 = nullptr;
            const strict_not_null x{p1};
        };
        EXPECT_DEATH(workaround_macro(), deathstring);
    }

    {
        int* p = nullptr;

        EXPECT_DEATH(helper(strict_not_null{p}), deathstring);
        EXPECT_DEATH(helper_const(strict_not_null{p}), deathstring);
    }

#ifdef CONFIRM_COMPILATION_ERRORS
    {
        strict_not_null x{nullptr};
        helper(strict_not_null{nullptr});
        helper_const(strict_not_null{nullptr});
    }
#endif
}
#endif // #if defined(__cplusplus) && (__cplusplus >= 201703L)
