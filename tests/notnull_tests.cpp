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

using namespace Guide;

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
        not_null<std::vector<int>> f(std::vector<int>{1}); // Not a pointer type. Must be tested with a non-integral type.
#endif
      int i = 12; 
      auto rp = RefCounted<int>(&i);
      not_null<int*> p(rp);
      CHECK(p.get() == &i);
    }

    TEST(TestNotNullCasting)
    {
        MyDerived derived;
        not_null<MyDerived*> p = &derived;
        not_null<MyBase*> q = p;
        CHECK(q == p);

#ifdef CONFIRM_COMPILATION_ERRORS
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
    }
}

int main(int, const char *[])
{
    return UnitTest::RunAllTests();
}
