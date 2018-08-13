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

#include <catch/catch.hpp>    // for AssertionHandler, StringRef, CHECK, TEST_...

#include <gsl_transition>     // for sloppy_not_null
#include <gsl/pointers>       // for not_null, operator<, operator<=, operator>

namespace gsl
{
struct fail_fast;
} // namespace gsl

using namespace gsl;


bool helper(not_null<int*> p) { return *p == 12; }
bool helper_const(not_null<const int*> p) { return *p == 12; }

bool sloppy_helper(sloppy_not_null<int*> p) { return *p == 12; }
bool sloppy_helper_const(sloppy_not_null<const int*> p) { return *p == 12; }

TEST_CASE("TestSloppyNotNull")
{
    {
        // raw ptr <-> sloppy_not_null
        int x = 42;

        sloppy_not_null<int*> snn = &x;

        sloppy_helper(&x);
        sloppy_helper_const(&x);

        CHECK(*snn == 42);
    }

    {
        // sloppy_not_null -> sloppy_not_null
        int x = 42;

        sloppy_not_null<int*> snn1{&x};
        sloppy_not_null<int*> snn2{&x};

        sloppy_helper(snn1);
        sloppy_helper_const(snn1);

        CHECK(snn1 == snn2);
    }

    {
        // sloppy_not_null -> not_null
        int x = 42;

        sloppy_not_null<int*> snn{&x};

        not_null<int*> nn1 = snn;
        not_null<int*> nn2{snn};

        helper(snn);
        helper_const(snn);

        CHECK(snn == nn1);
        CHECK(snn == nn2);
    }

    {
        // not_null -> sloppy_not_null
        int x = 42;

        not_null<int*> nn{&x};

        sloppy_not_null<int*> snn1{nn};
        sloppy_not_null<int*> snn2 = nn;

        sloppy_helper(nn);
        sloppy_helper_const(nn);

        CHECK(snn1 == nn);
        CHECK(snn2 == nn);

        std::hash<sloppy_not_null<int*>> hash_snn;
        std::hash<not_null<int*>> hash_nn;

        CHECK(hash_nn(snn1) == hash_nn(nn));
        CHECK(hash_snn(snn1) == hash_nn(nn));
        CHECK(hash_nn(snn1) == hash_nn(snn2));
        CHECK(hash_snn(snn1) == hash_snn(nn));
    }

#ifdef CONFIRM_COMPILATION_ERRORS
    {
        sloppy_not_null<int*> p{nullptr};
    }
#endif
}

static_assert(std::is_nothrow_move_constructible<sloppy_not_null<void*>>::value,
              "sloppy_not_null must be no-throw move constructible");
