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
#include <vector>
#include <iostream>

using namespace gsl;

struct MyBase { bool foo() { return true; } };
struct MyDerived : public MyBase {};
struct Unrelated {};

SUITE(MaybeNullTests)
{
    TEST(TestMaybeNull1)
    {
#ifdef CONFIRM_COMPILATION_ERRORS
        // Forbid non-nullptr assignable types
        maybe_null_ret<std::vector<int>> f_ret(std::vector<int>{1});
        maybe_null_ret<std::vector<int>> f_ret(std::vector<int>{1});
        maybe_null_ret<int> z_ret(10);
        maybe_null_dbg<std::vector<int>> y_dbg({1,2});
        maybe_null_dbg<int> z_dbg(10);
        maybe_null_dbg<std::vector<int>> y_dbg({1,2});
#endif
        int n = 5;
        maybe_null_dbg<int *> opt_n(&n);
        int result = 0;
        bool threw = false;

        CHECK_THROW(result = *opt_n, fail_fast);

        maybe_null_ret<std::shared_ptr<int>> x_ret(std::make_shared<int>(10)); // shared_ptr<int> is nullptr assignable
        maybe_null_dbg<std::shared_ptr<int>> x_dbg(std::make_shared<int>(10)); // shared_ptr<int> is nullptr assignable
    }

    TEST(TestMaybeNull2)
    {
        int n = 5;
        maybe_null<int *> opt_n(&n);
        int result = 0;
        if (opt_n.present())
	        result = *opt_n;
    }

    TEST(TestMaybeNull3)
    {
        int n = 5;
        maybe_null<int *> opt_n(&n);
        int result = 0;
        if (opt_n != nullptr) 
		    result = *opt_n;
    }

    int test4_helper(maybe_null<int *> p)
    {
        if (p != nullptr)
            return *p;
        return -1;
    }

    TEST(TestMaybeNull4)
    {
        int n = 5;
        int result = 0;
        result = test4_helper(&n);
    }

    int test5_helper(maybe_null_dbg<int *> p)
    {
        return *p;
    }

    TEST(TestMaybeNull5)
    {
        int n = 5;
        int result = 0;
        bool threw = false;

        CHECK_THROW(result = test5_helper(&n), fail_fast);
    }

#ifdef CONFIRM_COMPILATION_ERRORS
    int TestMaybeNull6()
    {
        int n;
        maybe_null<int> o(n);
    }
#endif

    int g_int;
    void test7_helper(maybe_null<maybe_null<int *> *> outptr)
    {
        g_int = 5;

        if (outptr.present())
	        *outptr = &g_int;
    }

    void test7b_helper(maybe_null_dbg<maybe_null_dbg<int *> *> outptr)
    {
        g_int = 5;

        if (outptr.present())
            *outptr = &g_int;
    }

    TEST(TestMaybeNull7a)
    {
        maybe_null<int *> outval;
        test7_helper(&outval);
        CHECK(outval.present() && *outval == 5);
    }

    TEST(TestMaybeNull7b)
    {
        maybe_null_dbg<int *> outval;
        test7b_helper(&outval);
	CHECK_THROW((void)*outval, fail_fast);
    }

    int test8_helper1(maybe_null_dbg<int *> opt)
    {
        return *opt;
    }

    int test8_helper2a(maybe_null_dbg<int *> opt)
    {
        if (!opt.present())
	        return 0;
        return test8_helper1(opt);
    }

    TEST(TestMaybeNull8a)
    {
        int n = 5;
        maybe_null_dbg<int *> opt(&n);
	    CHECK_THROW(test8_helper2a(opt), fail_fast);
    }

#ifdef CONVERT_TO_PTR_TO_CONST
    int test9_helper(maybe_null<const int *> copt)
    {
        if (copt.present())
	        return *copt;
        return 0;
    }

    void TestMaybeNull9()
    {
        int n = 5;
        maybe_null<int *> opt(&n);
	    CHECK_THROW(test9_helper(opt), fail_fast);
    }
#endif

    TEST(TestMaybeNullCasting)
    {
        MyDerived derived;
        maybe_null<MyDerived*> p = &derived;
        CHECK(p.present());

        maybe_null<MyBase*> q = p;
        CHECK(q == p);

        maybe_null_dbg<MyDerived*> pdbg = &derived;
        CHECK(pdbg.present());

        maybe_null_dbg<MyBase*> qdbg = pdbg;
        CHECK(qdbg == pdbg);

#ifdef CONFIRM_COMPILATION_ERRORS
        maybe_null<Unrelated*> r = p;
        maybe_null<Unrelated*> s = reinterpret_cast<Unrelated*>(p);
#endif
        maybe_null_dbg<Unrelated*> t = reinterpret_cast<Unrelated*>(p.get());

        CHECK_THROW((void)(void*)t.get(), fail_fast);
        maybe_null_dbg<Unrelated*> u = reinterpret_cast<Unrelated*>(p.get());
        CHECK(u.present());
        CHECK((void*)p.get() == (void*)u.get());
    }

    TEST(TestMaybeNullArrow)
    {
        MyDerived derived;
        maybe_null_dbg<MyDerived*> p = &derived;

        CHECK_THROW(p->foo(), fail_fast);
        CHECK(p.present());
        CHECK(p->foo());

        maybe_null<MyBase*> q = p;
        CHECK(q.present());
        CHECK(q->foo());
    }

    TEST(TestMaybeNullCompare)
    {
        int i1 = 1;
        int i2 = 2;

        maybe_null_dbg<int*> p1 = &i1;
        maybe_null_dbg<int*> p1_2 = &i1;
        maybe_null_dbg<int*> p2 = &i2;

        CHECK_THROW(p1.get(), fail_fast);
        CHECK_THROW(p1_2.get(), fail_fast);
        CHECK_THROW(p2.get(), fail_fast);

        CHECK(p1 != p2);
        CHECK(!(p1 == p2));
        CHECK(p1 == p1);
        CHECK(p1 == p1_2);

        // Make sure we no longer throw here
        CHECK(p1.get() != nullptr);
        CHECK(p1_2.get() != nullptr);
        CHECK(p2.get() != nullptr);
    }

    TEST(TestMaybeNullCopy)
    {
        int i1 = 1;
        int i2 = 2;

        maybe_null_dbg<int*> p1 = &i1;
        maybe_null_dbg<int*> p1_2 = &i1;
        maybe_null_dbg<int*> p2 = &i2;

        CHECK(p1 != p2);
        CHECK(p1 == p1_2);

        // Make sure we no longer throw here
        CHECK(p1.get() != nullptr);
        CHECK(p2.get() != nullptr);

        p1 = p2;

        // Make sure we now throw
        CHECK_THROW(p1.get(), fail_fast);

        CHECK(p1 == p2);
        CHECK(p1 != p1_2);

        // Make sure we no longer throw here
        CHECK(p1.get() != nullptr);
    }

    TEST(TestMaybeNullAssignmentOps)
    {
        MyBase base;
        MyDerived derived;
        Unrelated unrelated;

        not_null<MyBase*> nnBase(&base);
        not_null<MyDerived*> nnDerived(&derived);
        not_null<Unrelated*> nnUnrelated(&unrelated);

        maybe_null_ret<MyBase*> mnBase_ret1(&base), mnBase_ret2;
        mnBase_ret2 = mnBase_ret1; // maybe_null_ret<T> = maybe_null_ret<T>
        mnBase_ret2 = nnBase; // maybe_null_ret<T> = not_null<T>

        maybe_null_ret<MyDerived*> mnDerived_ret(&derived);
        mnBase_ret2 = mnDerived_ret; // maybe_null_ret<T> = maybe_null_ret<U>
        mnBase_ret1 = &derived; // maybe_null_ret<T> = U;
        mnBase_ret1 = nnDerived; // maybe_null_ret<T> = not_null<U>

        maybe_null_ret<Unrelated*> mnUnrelated_ret;
        mnUnrelated_ret = &unrelated; // maybe_null_ret<T> = T

        maybe_null_dbg<MyBase*> mnBase_dbg1(&base), mnBase_dbg2;
        mnBase_dbg2 = mnBase_dbg1; // maybe_null_dbg<T> = maybe_null_dbg<T>
        mnBase_dbg2 = nnBase; // maybe_null_dbg<T> = not_null<T>

        maybe_null_dbg<MyDerived*> mnDerived_dbg(&derived);
        mnBase_dbg2 = mnDerived_dbg; // maybe_null_dbg<T> = maybe_null_dbg<U>
        mnBase_dbg1 = &derived; // maybe_null_dbg<T> = U;
        mnBase_dbg1 = nnDerived; // maybe_null_dbg<T> = not_null<U>

        maybe_null_dbg<Unrelated*> mnUnrelated_dbg;
        mnUnrelated_dbg = &unrelated; // maybe_null_dbg<T> = T
    }
}

int main(int, const char *[])
{
    return UnitTest::RunAllTests();
}
