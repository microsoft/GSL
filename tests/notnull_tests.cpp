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
#include <gsl/gsl>
#include <vector>

using namespace gsl;

struct MyBase {};
struct MyDerived : public MyBase {};
struct Unrelated {};

// stand-in for a user-defined ref-counted class
template<typename T>
struct RefCounted
{
    RefCounted(T* p) : p_(p) {}
    operator T*() { return p_; }
    T* p_;
};

SUITE(NotNullTests)
{

    bool helper(not_null<int*> p)
    {
        return *p == 12;
    }

    TEST(TestNotNullConstructors)
    {
#ifdef CONFIRM_COMPILATION_ERRORS
        not_null<int*> p = nullptr; // yay...does not compile!
        not_null<std::vector<char>*> p = 0; // yay...does not compile!
        not_null<int*> p; // yay...does not compile!
        std::unique_ptr<int> up = std::make_unique<int>(120);
        not_null<int*> p = up;

        // Forbid non-nullptr assignable types
        not_null<std::vector<int>> f(std::vector<int>{1});
        not_null<int> z(10);
        not_null<std::vector<int>> y({1,2});
#endif
      int i = 12; 
      auto rp = RefCounted<int>(&i);
      not_null<int*> p(rp);
      CHECK(p.get() == &i);

      not_null<std::shared_ptr<int>> x(std::make_shared<int>(10)); // shared_ptr<int> is nullptr assignable


      struct Some {
          Some(int i)
              : member(i)
          {}

          int member;
      };

      Some obj{123};

      // not_null(const T&)
      not_null<Some*> p1{&obj};
      CHECK(p1.get() == &obj);            // check state using 'get()'
      CHECK(p1 == &obj);                  // check state using 'operator const_pointer&()'
      CHECK(&*p1 == &obj);                // check state using 'operatr *()'
      CHECK(&p1->member == &obj.member);  // check state using 'operatr ->()"

      // not_null(T&&)
      auto tmp = std::make_unique<Some>(456);
      auto raw = tmp.get();
      not_null<std::unique_ptr<Some>> p2{std::move(tmp)};
      CHECK(p2.get().get() == raw);                                         // check state using 'get()'
      CHECK(static_cast<const std::unique_ptr<Some>&>(p2).get() == raw);    // check state using 'operator const_pointer&()'
      CHECK(&*p2 == raw);                                                   // check state using 'operatr *()'
      CHECK(&p2->member == &raw->member);                                   // check state using 'operatr ->()'
    }

    TEST(TestNotNullCasting)
    {
        MyBase base;
	MyDerived derived;
	Unrelated unrelated;
	not_null<Unrelated*> u = &unrelated;
        (void)u;
	not_null<MyDerived*> p = &derived;
        not_null<MyBase*> q = &base;
	q = p; // allowed with heterogeneous copy ctor
        CHECK(q == p);

#ifdef CONFIRM_COMPILATION_ERRORS
	q = u; // no viable conversion possible between MyBase* and Unrelated*
	p = q; // not possible to implicitly convert MyBase* to MyDerived*

        not_null<Unrelated*> r = p;
        not_null<Unrelated*> s = reinterpret_cast<Unrelated*>(p);
#endif
        not_null<Unrelated*> t = reinterpret_cast<Unrelated*>(p.get());
        CHECK((void*)p.get() == (void*)t.get());
    }

    TEST(TestNotNullAssignment)
    {
        int i = 12;
        not_null<int*> p = &i; 
        CHECK(helper(p));

        int* q = nullptr;
        CHECK_THROW(p = q, fail_fast);

        MyBase obj1;
        MyDerived obj2;

        // operator=(const T&)
        not_null<MyBase*> p1{&obj1};
        p1 = &obj2;
        CHECK(p1.get() == &obj2);

        MyDerived *n = nullptr;
        CHECK_THROW(p1 = n, fail_fast);

        // operator=(T&&)
        auto tmp = std::make_unique<MyDerived>();
        auto raw = tmp.get();
        not_null<std::unique_ptr<MyBase>> p2{std::make_unique<MyBase>()};

        p2 = std::move(tmp);
        CHECK(p2.get().get() == raw);

        tmp = nullptr;
        CHECK_THROW(p2 = std::move(tmp), fail_fast);

        // operator=(const not_null<U>&)
        not_null<MyBase*> p3{&obj1};
        not_null<MyDerived*> p4{&obj2};

        p3 = p4;
        CHECK(p3.get() == p4.get());
    }

    TEST(TestTestNotNullComparison)
    {
        MyBase obj1;
        MyDerived obj2;

        not_null<MyBase*> a{&obj1};
        not_null<MyDerived*> b{&obj2};
        CHECK(a != b);
        CHECK(!(a == b));
        CHECK(a == &obj1);
        CHECK(a != &obj2);
        CHECK(!(a == &obj2));

        CHECK(b != a);
        CHECK(!(b == a));
        CHECK(b == &obj2);
        CHECK(b != &obj1);
        CHECK(!(b == &obj1));

        a = b;
        CHECK(a == b);
        CHECK(!(a != b));
        CHECK(a == &obj2);
        CHECK(a != &obj1);
        CHECK(!(a == &obj1));

        CHECK(b == a);
        CHECK(!(b != a));
        CHECK(b == &obj2);
        CHECK(b != &obj1);
        CHECK(!(b == &obj1));
    }

    TEST(TestNotNullTypeQualifiers)
    {
        struct Some {
            bool isConst()
            {
                return false;
            }

            bool isConst() const
            {
                return true;
            }
        };

        Some obj;

        not_null<Some*> p1{&obj};
        CHECK(!(*p1).isConst());            // check the 'operator *' return type qualifiers
        CHECK(!p1->isConst());              // check the 'operator ->' return type qualifiers
        CHECK(!p1.get()->isConst());        // check the 'get()' return type qualifiers

        not_null<const Some*> p2{&obj};
        CHECK((*p2).isConst());             // check the 'operator *' return type qualifiers
        CHECK(p2->isConst());               // check the 'operator ->' return type qualifiers
        CHECK(p2.get()->isConst());         // check the 'get()' return type qualifiers
    }
}

int main(int, const char *[])
{
    return UnitTest::RunAllTests();
}
