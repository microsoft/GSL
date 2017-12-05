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

#include <catch/catch.hpp> // for AssertionHandler, StringRef, CHECK, TEST_...

#include <gsl/pointers> // for not_null, operator<, operator<=, operator>

#include <algorithm> // for addressof
#include <memory>    // for shared_ptr, make_shared, operator<, opera...
#include <sstream>   // for operator<<, ostringstream, basic_ostream:...
#include <stdint.h>  // for uint16_t
#include <string>    // for basic_string, operator==, string, operator<<
#include <typeinfo>  // for type_info

namespace gsl {
struct fail_fast;
}  // namespace gsl

using namespace gsl;

struct MyBase
{
};
struct MyDerived : public MyBase
{
};
struct Unrelated
{
};

// stand-in for a user-defined ref-counted class
template <typename T>
struct RefCounted
{
    RefCounted(T* p) : p_(p) {}
    operator T*() { return p_; }
    T* p_;
};

// user defined smart pointer with comparison operators returning non bool value
template <typename T>
struct CustomPtr
{
    CustomPtr(T* p) : p_(p) {}
    operator T*() { return p_; }
    bool operator!=(std::nullptr_t) const { return p_ != nullptr; }
    T* p_ = nullptr;
};

template <typename T, typename U>
std::string operator==(CustomPtr<T> const& lhs, CustomPtr<U> const& rhs)
{
    return reinterpret_cast<const void*>(lhs.p_) == reinterpret_cast<const void*>(rhs.p_) ? "true"
                                                                                          : "false";
}

template <typename T, typename U>
std::string operator!=(CustomPtr<T> const& lhs, CustomPtr<U> const& rhs)
{
    return reinterpret_cast<const void*>(lhs.p_) != reinterpret_cast<const void*>(rhs.p_) ? "true"
                                                                                          : "false";
}

template <typename T, typename U>
std::string operator<(CustomPtr<T> const& lhs, CustomPtr<U> const& rhs)
{
    return reinterpret_cast<const void*>(lhs.p_) < reinterpret_cast<const void*>(rhs.p_) ? "true"
                                                                                         : "false";
}

template <typename T, typename U>
std::string operator>(CustomPtr<T> const& lhs, CustomPtr<U> const& rhs)
{
    return reinterpret_cast<const void*>(lhs.p_) > reinterpret_cast<const void*>(rhs.p_) ? "true"
                                                                                         : "false";
}

template <typename T, typename U>
std::string operator<=(CustomPtr<T> const& lhs, CustomPtr<U> const& rhs)
{
    return reinterpret_cast<const void*>(lhs.p_) <= reinterpret_cast<const void*>(rhs.p_) ? "true"
                                                                                          : "false";
}

template <typename T, typename U>
std::string operator>=(CustomPtr<T> const& lhs, CustomPtr<U> const& rhs)
{
    return reinterpret_cast<const void*>(lhs.p_) >= reinterpret_cast<const void*>(rhs.p_) ? "true"
                                                                                          : "false";
}

struct NonCopyableNonMovable
{
    NonCopyableNonMovable() = default;
    NonCopyableNonMovable(const NonCopyableNonMovable&) = delete;
    NonCopyableNonMovable& operator=(const NonCopyableNonMovable&) = delete;
    NonCopyableNonMovable(NonCopyableNonMovable&&) = delete;
    NonCopyableNonMovable& operator=(NonCopyableNonMovable&&) = delete;
};

bool helper(not_null<int*> p) { return *p == 12; }

TEST_CASE("TestNotNullConstructors")
{
#ifdef CONFIRM_COMPILATION_ERRORS
    not_null<int*> p = nullptr;         // yay...does not compile!
    not_null<std::vector<char>*> p = 0; // yay...does not compile!
    not_null<int*> p;                   // yay...does not compile!
    std::unique_ptr<int> up = std::make_unique<int>(120);
    not_null<int*> p = up;

    // Forbid non-nullptr assignable types
    not_null<std::vector<int>> f(std::vector<int>{1});
    not_null<int> z(10);
    not_null<std::vector<int>> y({1, 2});
#endif
    int i = 12;
    auto rp = RefCounted<int>(&i);
    not_null<int*> p(rp);
    CHECK(p.get() == &i);

    not_null<std::shared_ptr<int>> x(
        std::make_shared<int>(10)); // shared_ptr<int> is nullptr assignable
}

template<typename T>
void ostream_helper(T v)
{
    not_null<T*> p(&v);
    {
        std::ostringstream os;
        std::ostringstream ref;
        os << p;
        ref << &v;
        CHECK(os.str() == ref.str());
    }
    {
        std::ostringstream os;
        std::ostringstream ref;
        os << *p;
        ref << v;
        CHECK(os.str() == ref.str());
    }
}

TEST_CASE("TestNotNullostream")
{
    ostream_helper<int>(17);
    ostream_helper<float>(21.5f);
    ostream_helper<double>(3.4566e-7f);
    ostream_helper<char>('c');
    ostream_helper<uint16_t>(0x0123u);
    ostream_helper<const char*>("cstring");
    ostream_helper<std::string>("string");
}


TEST_CASE("TestNotNullCasting")
{
    MyBase base;
    MyDerived derived;
    Unrelated unrelated;
    not_null<Unrelated*> u = &unrelated;
    (void) u;
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
    CHECK(reinterpret_cast<void*>(p.get()) == reinterpret_cast<void*>(t.get()));
}

TEST_CASE("TestNotNullAssignment")
{
    int i = 12;
    not_null<int*> p = &i;
    CHECK(helper(p));

    int* q = nullptr;
    CHECK_THROWS_AS(p = q, fail_fast);
}

TEST_CASE("TestNotNullRawPointerComparison")
{
    int ints[2] = {42, 43};
    int* p1 = &ints[0];
    const int* p2 = &ints[1];

    using NotNull1 = not_null<decltype(p1)>;
    using NotNull2 = not_null<decltype(p2)>;

    CHECK((NotNull1(p1) == NotNull1(p1)) == true);
    CHECK((NotNull1(p1) == NotNull2(p2)) == false);

    CHECK((NotNull1(p1) != NotNull1(p1)) == false);
    CHECK((NotNull1(p1) != NotNull2(p2)) == true);

    CHECK((NotNull1(p1) < NotNull1(p1)) == false);
    CHECK((NotNull1(p1) < NotNull2(p2)) == (p1 < p2));
    CHECK((NotNull2(p2) < NotNull1(p1)) == (p2 < p1));

    CHECK((NotNull1(p1) > NotNull1(p1)) == false);
    CHECK((NotNull1(p1) > NotNull2(p2)) == (p1 > p2));
    CHECK((NotNull2(p2) > NotNull1(p1)) == (p2 > p1));

    CHECK((NotNull1(p1) <= NotNull1(p1)) == true);
    CHECK((NotNull1(p1) <= NotNull2(p2)) == (p1 <= p2));
    CHECK((NotNull2(p2) <= NotNull1(p1)) == (p2 <= p1));

}

TEST_CASE("TestNotNullDereferenceOperator")
{
    {
        auto sp1 = std::make_shared<NonCopyableNonMovable>();

        using NotNullSp1 = not_null<decltype(sp1)>;
        CHECK(typeid(*sp1) == typeid(*NotNullSp1(sp1))); 
        CHECK(std::addressof(*NotNullSp1(sp1)) == std::addressof(*sp1));
    }

    {
        int ints[1] = { 42 };
        CustomPtr<int> p1(&ints[0]);

        using NotNull1 = not_null<decltype(p1)>;
        CHECK(typeid(*NotNull1(p1)) == typeid(*p1));
        CHECK(*NotNull1(p1) == 42);
        *NotNull1(p1) = 43;
        CHECK(ints[0] == 43);
    }

    {
        int v = 42;
        gsl::not_null<int*> p(&v);
        CHECK(typeid(*p) == typeid(*(&v)));
        *p = 43;
        CHECK(v == 43);
    }
}

TEST_CASE("TestNotNullSharedPtrComparison")
{
    auto sp1 = std::make_shared<int>(42);
    auto sp2 = std::make_shared<const int>(43);

    using NotNullSp1 = not_null<decltype(sp1)>;
    using NotNullSp2 = not_null<decltype(sp2)>;

    CHECK((NotNullSp1(sp1) == NotNullSp1(sp1)) == true);
    CHECK((NotNullSp1(sp1) == NotNullSp2(sp2)) == false);

    CHECK((NotNullSp1(sp1) != NotNullSp1(sp1)) == false);
    CHECK((NotNullSp1(sp1) != NotNullSp2(sp2)) == true);

    CHECK((NotNullSp1(sp1) < NotNullSp1(sp1)) == false);
    CHECK((NotNullSp1(sp1) < NotNullSp2(sp2)) == (sp1 < sp2));
    CHECK((NotNullSp2(sp2) < NotNullSp1(sp1)) == (sp2 < sp1));

    CHECK((NotNullSp1(sp1) > NotNullSp1(sp1)) == false);
    CHECK((NotNullSp1(sp1) > NotNullSp2(sp2)) == (sp1 > sp2));
    CHECK((NotNullSp2(sp2) > NotNullSp1(sp1)) == (sp2 > sp1));

    CHECK((NotNullSp1(sp1) <= NotNullSp1(sp1)) == true);
    CHECK((NotNullSp1(sp1) <= NotNullSp2(sp2)) == (sp1 <= sp2));
    CHECK((NotNullSp2(sp2) <= NotNullSp1(sp1)) == (sp2 <= sp1));

    CHECK((NotNullSp1(sp1) >= NotNullSp1(sp1)) == true);
    CHECK((NotNullSp1(sp1) >= NotNullSp2(sp2)) == (sp1 >= sp2));
    CHECK((NotNullSp2(sp2) >= NotNullSp1(sp1)) == (sp2 >= sp1));
}

TEST_CASE("TestNotNullCustomPtrComparison")
{
    int ints[2] = {42, 43};
    CustomPtr<int> p1(&ints[0]);
    CustomPtr<const int> p2(&ints[1]);

    using NotNull1 = not_null<decltype(p1)>;
    using NotNull2 = not_null<decltype(p2)>;

    CHECK((NotNull1(p1) == NotNull1(p1)) == "true");
    CHECK((NotNull1(p1) == NotNull2(p2)) == "false");

    CHECK((NotNull1(p1) != NotNull1(p1)) == "false");
    CHECK((NotNull1(p1) != NotNull2(p2)) == "true");

    CHECK((NotNull1(p1) < NotNull1(p1)) == "false");
    CHECK((NotNull1(p1) < NotNull2(p2)) == (p1 < p2));
    CHECK((NotNull2(p2) < NotNull1(p1)) == (p2 < p1));

    CHECK((NotNull1(p1) > NotNull1(p1)) == "false");
    CHECK((NotNull1(p1) > NotNull2(p2)) == (p1 > p2));
    CHECK((NotNull2(p2) > NotNull1(p1)) == (p2 > p1));

    CHECK((NotNull1(p1) <= NotNull1(p1)) == "true");
    CHECK((NotNull1(p1) <= NotNull2(p2)) == (p1 <= p2));
    CHECK((NotNull2(p2) <= NotNull1(p1)) == (p2 <= p1));

    CHECK((NotNull1(p1) >= NotNull1(p1)) == "true");
    CHECK((NotNull1(p1) >= NotNull2(p2)) == (p1 >= p2));
    CHECK((NotNull2(p2) >= NotNull1(p1)) == (p2 >= p1));
}
