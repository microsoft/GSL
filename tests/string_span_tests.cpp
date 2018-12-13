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

#ifdef _MSC_VER
// blanket turn off warnings from CppCoreCheck from catch
// so people aren't annoyed by them when running the tool.
#pragma warning(disable : 26440 26426) // from catch

#endif

#include <catch/catch.hpp> // for AssertionHandler, StringRef, CHECK, TEST_...

#include <gsl/gsl_assert>  // for Expects, fail_fast (ptr only)
#include <gsl/pointers>    // for owner
#include <gsl/span>        // for span, dynamic_extent
#include <gsl/string_span> // for basic_string_span, operator==, ensure_z

#include <algorithm>   // for move, find
#include <cstddef>     // for size_t
#include <map>         // for map
#include <string>      // for basic_string, string, char_traits, operat...
#include <type_traits> // for remove_reference<>::type
#include <vector>      // for vector, allocator

using namespace std;
using namespace gsl;

// Generic string functions

namespace generic
{

template <typename CharT>
GSL_SUPPRESS(bounds.1) // NO-FORMAT: attribute
GSL_SUPPRESS(f.23) // NO-FORMAT: attribute
auto strlen(const CharT* s)
{
    auto p = s;
    while (*p) ++p;
    return p - s;
}

template <typename CharT>
GSL_SUPPRESS(bounds.1) // NO-FORMAT: attribute
auto strnlen(const CharT* s, std::size_t n)
{
    return std::find(s, s + n, CharT{0}) - s;
}

} // namespace generic

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("TestLiteralConstruction")
{
    cwstring_span<> v = ensure_z(L"Hello");
    CHECK(5 == v.length());

#ifdef CONFIRM_COMPILATION_ERRORS
    wstring_span<> v2 = ensure0(L"Hello");
#endif
}
GSL_SUPPRESS(con.4) // NO-FORMAT: attribute

TEST_CASE("TestConstructFromStdString")
{
    std::string s = "Hello there world";
    cstring_span<> v = s;
    CHECK(v.length() == static_cast<cstring_span<>::index_type>(s.length()));
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("TestConstructFromStdVector")
{
    std::vector<char> vec(5, 'h');
    string_span<> v{vec};
    CHECK(v.length() == static_cast<string_span<>::index_type>(vec.size()));
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("TestStackArrayConstruction")
{
    wchar_t stack_string[] = L"Hello";

    {
        cwstring_span<> v = ensure_z(stack_string);
        CHECK(v.length() == 5);
    }

    {
        cwstring_span<> v = stack_string;
        CHECK(v.length() == 5);
    }

    {
        wstring_span<> v = ensure_z(stack_string);
        CHECK(v.length() == 5);
    }

    {
        wstring_span<> v = stack_string;
        CHECK(v.length() == 5);
    }
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("TestConstructFromConstCharPointer")
{
    const char* s = "Hello";
    cstring_span<> v = ensure_z(s);
    CHECK(v.length() == 5);
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("TestConversionToConst")
{
    char stack_string[] = "Hello";
    string_span<> v = ensure_z(stack_string);
    cstring_span<> v2 = v;
    CHECK(v.length() == v2.length());
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("TestConversionFromConst")
{
    char stack_string[] = "Hello";
    cstring_span<> v = ensure_z(stack_string);
    (void) v;
#ifdef CONFIRM_COMPILATION_ERRORS
    string_span<> v2 = v;
    string_span<> v3 = "Hello";
#endif
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("TestToString")
{
    auto s = gsl::to_string(cstring_span<>{});
    CHECK(s.length() == 0);

    char stack_string[] = "Hello";
    cstring_span<> v = ensure_z(stack_string);
    auto s2 = gsl::to_string(v);
    CHECK(static_cast<cstring_span<>::index_type>(s2.length()) == v.length());
    CHECK(s2.length() == 5);
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("TestToBasicString")
{
    auto s = gsl::to_basic_string<char, std::char_traits<char>, ::std::allocator<char>>(
        cstring_span<>{});
    CHECK(s.length() == 0);

    char stack_string[] = "Hello";
    cstring_span<> v = ensure_z(stack_string);
    auto s2 = gsl::to_basic_string<char, std::char_traits<char>, ::std::allocator<char>>(v);
    CHECK(static_cast<cstring_span<>::index_type>(s2.length()) == v.length());
    CHECK(s2.length() == 5);
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.3) // NO-FORMAT: attribute
TEST_CASE("EqualityAndImplicitConstructors")
{
    {
        cstring_span<> span = "Hello";
        cstring_span<> span1;

        // comparison to empty span
        CHECK(span1 != span);
        CHECK(span != span1);
    }

    {
        cstring_span<> span = "Hello";
        cstring_span<> span1 = "Hello1";

        // comparison to different span
        CHECK(span1 != span);
        CHECK(span != span1);
    }

    {
        cstring_span<> span = "Hello";

        const char ar[] = {'H', 'e', 'l', 'l', 'o'};
        const char ar1[] = "Hello";
        const char ar2[10] = "Hello";
        const char* ptr = "Hello";
        const std::string str = "Hello";
        const std::vector<char> vec = {'H', 'e', 'l', 'l', 'o'};
        gsl::span<const char> sp = ensure_z("Hello");

        // comparison to  literal
        CHECK(span == cstring_span<>("Hello"));

        // comparison to static array with no null termination
        CHECK(span == cstring_span<>(ar));

        // comparison to static array with null at the end
        CHECK(span == cstring_span<>(ar1));

        // comparison to static array with null in the middle
        CHECK(span == cstring_span<>(ar2));

        // comparison to null-terminated c string
        CHECK(span == cstring_span<>(ptr, 5));

        // comparison to string
        CHECK(span == cstring_span<>(str));

        // comparison to vector of charaters with no null termination
        CHECK(span == cstring_span<>(vec));

        // comparison to span
        CHECK(span == cstring_span<>(sp));

        // comparison to string_span
        CHECK(span == span);
    }

    {
        char ar[] = {'H', 'e', 'l', 'l', 'o'};

        string_span<> span = ar;

        char ar1[] = "Hello";
        char ar2[10] = "Hello";
        char* ptr = ar;
        std::string str = "Hello";
        std::vector<char> vec = {'H', 'e', 'l', 'l', 'o'};
        gsl::span<char> sp = ensure_z(ar1);

        // comparison to static array with no null termination
        CHECK(span == string_span<>(ar));

        // comparison to static array with null at the end
        CHECK(span == string_span<>(ar1));

        // comparison to static array with null in the middle
        CHECK(span == string_span<>(ar2));

        // comparison to null-terminated c string
        CHECK(span == string_span<>(ptr, 5));

        // comparison to string
        CHECK(span == string_span<>(str));

        // comparison to vector of charaters with no null termination
        CHECK(span == string_span<>(vec));

        // comparison to span
        CHECK(span == string_span<>(sp));

        // comparison to string_span
        CHECK(span == span);
    }

    {
        const char ar[] = {'H', 'e', 'l', 'l', 'o'};
        const char ar1[] = "Hello";
        const char ar2[10] = "Hello";
        const std::string str = "Hello";
        const std::vector<char> vec = {'H', 'e', 'l', 'l', 'o'};
        const gsl::span<const char> sp = ensure_z("Hello");

        cstring_span<> span = "Hello";

        // const span, const other type

        CHECK(span == "Hello");
        CHECK(span == ar);
        CHECK(span == ar1);
        CHECK(span == ar2);
#ifdef CONFIRM_COMPILATION_ERRORS
        const char* ptr = "Hello";
        CHECK(span == ptr);
#endif
        CHECK(span == str);
        CHECK(span == vec);
        CHECK(span == sp);

        CHECK("Hello" == span);
        CHECK(ar == span);
        CHECK(ar1 == span);
        CHECK(ar2 == span);
#ifdef CONFIRM_COMPILATION_ERRORS
        CHECK(ptr == span);
#endif
        CHECK(str == span);
        CHECK(vec == span);
        CHECK(sp == span);

        // const span, non-const other type

        char _ar[] = {'H', 'e', 'l', 'l', 'o'};
        char _ar1[] = "Hello";
        char _ar2[10] = "Hello";
        char* _ptr = _ar;
        std::string _str = "Hello";
        std::vector<char> _vec = {'H', 'e', 'l', 'l', 'o'};
        gsl::span<char> _sp{_ar, 5};

        CHECK(span == _ar);
        CHECK(span == _ar1);
        CHECK(span == _ar2);
#ifdef CONFIRM_COMPILATION_ERRORS
        CHECK(span == _ptr);
#endif
        CHECK(span == _str);
        CHECK(span == _vec);
        CHECK(span == _sp);

        CHECK(_ar == span);
        CHECK(_ar1 == span);
        CHECK(_ar2 == span);
#ifdef CONFIRM_COMPILATION_ERRORS
        CHECK(_ptr == span);
#endif
        CHECK(_str == span);
        CHECK(_vec == span);
        CHECK(_sp == span);

        string_span<> _span{_ptr, 5};

        // non-const span, non-const other type

        CHECK(_span == _ar);
        CHECK(_span == _ar1);
        CHECK(_span == _ar2);
#ifdef CONFIRM_COMPILATION_ERRORS
        CHECK(_span == _ptr);
#endif
        CHECK(_span == _str);
        CHECK(_span == _vec);
        CHECK(_span == _sp);

        CHECK(_ar == _span);
        CHECK(_ar1 == _span);
        CHECK(_ar2 == _span);
#ifdef CONFIRM_COMPILATION_ERRORS
        CHECK(_ptr == _span);
#endif
        CHECK(_str == _span);
        CHECK(_vec == _span);
        CHECK(_sp == _span);

        // non-const span, const other type

        CHECK(_span == "Hello");
        CHECK(_span == ar);
        CHECK(_span == ar1);
        CHECK(_span == ar2);
#ifdef CONFIRM_COMPILATION_ERRORS
        CHECK(_span == ptr);
#endif
        CHECK(_span == str);
        CHECK(_span == vec);
        CHECK(_span == sp);

        CHECK("Hello" == _span);
        CHECK(ar == _span);
        CHECK(ar1 == _span);
        CHECK(ar2 == _span);
#ifdef CONFIRM_COMPILATION_ERRORS
        CHECK(ptr == _span);
#endif
        CHECK(str == _span);
        CHECK(vec == _span);
        CHECK(sp == _span);

        // two spans

        CHECK(_span == span);
        CHECK(span == _span);
    }

    {
        std::vector<char> str1 = {'H', 'e', 'l', 'l', 'o'};
        cstring_span<> span1 = str1;
        std::vector<char> str2 = std::move(str1);
        cstring_span<> span2 = str2;

        // comparison of spans from the same vector before and after move (ok)
        CHECK(span1 == span2);
    }
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.3) // NO-FORMAT: attribute
TEST_CASE("ComparisonAndImplicitConstructors")
{
    {
        cstring_span<> span = "Hello";

        const char ar[] = {'H', 'e', 'l', 'l', 'o'};
        const char ar1[] = "Hello";
        const char ar2[10] = "Hello";
        const char* ptr = "Hello";
        const std::string str = "Hello";
        const std::vector<char> vec = {'H', 'e', 'l', 'l', 'o'};

        // comparison to  literal
        CHECK(span < cstring_span<>("Helloo"));
        CHECK(span > cstring_span<>("Hell"));

        // comparison to static array with no null termination
        CHECK(span >= cstring_span<>(ar));

        // comparison to static array with null at the end
        CHECK(span <= cstring_span<>(ar1));

        // comparison to static array with null in the middle
        CHECK(span >= cstring_span<>(ar2));

        // comparison to null-terminated c string
        CHECK(span <= cstring_span<>(ptr, 5));

        // comparison to string
        CHECK(span >= cstring_span<>(str));

        // comparison to vector of charaters with no null termination
        CHECK(span <= cstring_span<>(vec));
    }

    {
        char ar[] = {'H', 'e', 'l', 'l', 'o'};

        string_span<> span = ar;

        char larr[] = "Hell";
        char rarr[] = "Helloo";

        char ar1[] = "Hello";
        char ar2[10] = "Hello";
        char* ptr = ar;
        std::string str = "Hello";
        std::vector<char> vec = {'H', 'e', 'l', 'l', 'o'};

        // comparison to static array with no null termination
        CHECK(span <= string_span<>(ar));
        CHECK(span < string_span<>(rarr));
        CHECK(span > string_span<>(larr));

        // comparison to static array with null at the end
        CHECK(span >= string_span<>(ar1));

        // comparison to static array with null in the middle
        CHECK(span <= string_span<>(ar2));

        // comparison to null-terminated c string
        CHECK(span >= string_span<>(ptr, 5));

        // comparison to string
        CHECK(span <= string_span<>(str));

        // comparison to vector of charaters with no null termination
        CHECK(span >= string_span<>(vec));
    }
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
GSL_SUPPRESS(r.11) // NO-FORMAT: attribute
GSL_SUPPRESS(r.3) // NO-FORMAT: attribute
GSL_SUPPRESS(r.5) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.1) // NO-FORMAT: attribute
TEST_CASE("ConstrutorsEnsureZ")
{
    // remove z from literals
    {
        cstring_span<> sp = "hello";
        CHECK((sp.length() == 5));
    }

    // take the string as is
    {
        auto str = std::string("hello");
        cstring_span<> sp = str;
        CHECK((sp.length() == 5));
    }

    // ensure z on c strings
    {
        gsl::owner<char*> ptr = new char[3];

        ptr[0] = 'a';
        ptr[1] = 'b';
        ptr[2] = '\0';

        string_span<> span = ensure_z(ptr);
        CHECK(span.length() == 2);

        delete[] ptr;
    }
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.3) // NO-FORMAT: attribute
TEST_CASE("Constructors")
{
    // creating cstring_span

    // from span of a final extent
    {
        span<const char, 6> sp = "Hello";
        cstring_span<> span = sp;
        CHECK(span.length() == 6);
    }

// from const span of a final extent to non-const string_span
#ifdef CONFIRM_COMPILATION_ERRORS
    {
        span<const char, 6> sp = "Hello";
        string_span<> span = sp;
        CHECK(span.length() == 6);
    }
#endif

// from string temporary
#ifdef CONFIRM_COMPILATION_ERRORS
    {
        cstring_span<> span = std::string("Hello");
    }
#endif

    // default
    {
        cstring_span<> span;
        CHECK(span.length() == 0);
    }

    // from string literal
    {
        cstring_span<> span = "Hello";
        CHECK(span.length() == 5);
    }

    // from const static array
    {
        const char ar[] = {'H', 'e', 'l', 'l', 'o'};
        cstring_span<> span = ar;
        CHECK(span.length() == 5);
    }

    // from non-const static array
    {
        char ar[] = {'H', 'e', 'l', 'l', 'o'};
        cstring_span<> span = ar;
        CHECK(span.length() == 5);
    }

    // from const ptr and length
    {
        const char* ptr = "Hello";
        cstring_span<> span{ptr, 5};
        CHECK(span.length() == 5);
    }

    // from const ptr and length, include 0
    {
        const char* ptr = "Hello";
        cstring_span<> span{ptr, 6};
        CHECK(span.length() == 6);
    }

    // from const ptr and length, 0 inside
    {
        const char* ptr = "He\0lo";
        cstring_span<> span{ptr, 5};
        CHECK(span.length() == 5);
    }

    // from non-const ptr and length
    {
        char ar[] = {'H', 'e', 'l', 'l', 'o'};
        char* ptr = ar;
        cstring_span<> span{ptr, 5};
        CHECK(span.length() == 5);
    }

    // from non-const ptr and length, 0 inside
    {
        char ar[] = {'H', 'e', '\0', 'l', 'o'};
        char* ptr = ar;
        cstring_span<> span{ptr, 5};
        CHECK(span.length() == 5);
    }

    // from const string
    {
        const std::string str = "Hello";
        const cstring_span<> span = str;
        CHECK(span.length() == 5);
    }

    // from non-const string
    {
        std::string str = "Hello";
        const cstring_span<> span = str;
        CHECK(span.length() == 5);
    }

    // from const vector
    {
        const std::vector<char> vec = {'H', 'e', 'l', 'l', 'o'};
        const cstring_span<> span = vec;
        CHECK(span.length() == 5);
    }

    // from non-const vector
    {
        std::vector<char> vec = {'H', 'e', 'l', 'l', 'o'};
        const cstring_span<> span = vec;
        CHECK(span.length() == 5);
    }

    // from const span
    {
        const std::vector<char> vec = {'H', 'e', 'l', 'l', 'o'};
        const span<const char> inner = vec;
        const cstring_span<> span = inner;
        CHECK(span.length() == 5);
    }

    // from non-const span
    {
        std::vector<char> vec = {'H', 'e', 'l', 'l', 'o'};
        const span<char> inner = vec;
        const cstring_span<> span = inner;
        CHECK(span.length() == 5);
    }

    // from const string_span
    {
        const std::vector<char> vec = {'H', 'e', 'l', 'l', 'o'};
        const cstring_span<> tmp = vec;
        const cstring_span<> span = tmp;
        CHECK(span.length() == 5);
    }

    // from non-const string_span
    {
        std::vector<char> vec = {'H', 'e', 'l', 'l', 'o'};
        string_span<> tmp = vec;
        cstring_span<> span = tmp;
        CHECK(span.length() == 5);
    }

    // creating string_span

    // from string literal
    {
#ifdef CONFIRM_COMPILATION_ERRORS
        string_span<> span = "Hello";
#endif
    }

    // from const static array
    {
#ifdef CONFIRM_COMPILATION_ERRORS
        const char ar[] = {'H', 'e', 'l', 'l', 'o'};
        string_span<> span = ar;
        CHECK(span.length() == 5);
#endif
    }

    // from non-const static array
    {
        char ar[] = {'H', 'e', 'l', 'l', 'o'};
        string_span<> span = ar;
        CHECK(span.length() == 5);
    }

    // from const ptr and length
    {
#ifdef CONFIRM_COMPILATION_ERRORS
        const char* ptr = "Hello";
        string_span<> span{ptr, 5};
        CHECK(span.length() == 5);
#endif
    }

    // from non-const ptr and length
    {
        char ar[] = {'H', 'e', 'l', 'l', 'o'};
        char* ptr = ar;
        string_span<> span{ptr, 5};
        CHECK(span.length() == 5);
    }

    // from const string
    {
#ifdef CONFIRM_COMPILATION_ERRORS
        const std::string str = "Hello";
        string_span<> span = str;
        CHECK(span.length() == 5);
#endif
    }

    // from non-const string
    {
        std::string str = "Hello";
        string_span<> span = str;
        CHECK(span.length() == 5);
    }

    // from const vector
    {
#ifdef CONFIRM_COMPILATION_ERRORS
        const std::vector<char> vec = {'H', 'e', 'l', 'l', 'o'};
        string_span<> span = vec;
        CHECK(span.length() == 5);
#endif
    }

    // from non-const vector
    {
        std::vector<char> vec = {'H', 'e', 'l', 'l', 'o'};
        string_span<> span = vec;
        CHECK(span.length() == 5);
    }

    // from const span
    {
#ifdef CONFIRM_COMPILATION_ERRORS
        std::vector<char> vec = {'H', 'e', 'l', 'l', 'o'};
        const span<const char> inner = vec;
        string_span<> span = inner;
        CHECK(span.length() == 5);
#endif
    }

    // from non-const span
    {
        std::vector<char> vec = {'H', 'e', 'l', 'l', 'o'};
        span<char> inner = vec;
        string_span<> span = inner;
        CHECK(span.length() == 5);
    }

    // from non-const span of non-const data from const vector
    {
#ifdef CONFIRM_COMPILATION_ERRORS
        const std::vector<char> vec = {'H', 'e', 'l', 'l', 'o'};
        const span<char> inner = vec;
        string_span<> span = inner;
        CHECK(span.length() == 5);
#endif
    }

    // from const string_span
    {
#ifdef CONFIRM_COMPILATION_ERRORS
        std::vector<char> vec = {'H', 'e', 'l', 'l', 'o'};
        cstring_span<> tmp = vec;
        string_span<> span = tmp;
        CHECK(span.length() == 5);
#endif
    }

    // from non-const string_span
    {
        std::vector<char> vec = {'H', 'e', 'l', 'l', 'o'};
        const string_span<> tmp = vec;
        const string_span<> span = tmp;
        CHECK(span.length() == 5);
    }

    // from non-const string_span from const vector
    {
#ifdef CONFIRM_COMPILATION_ERRORS
        const std::vector<char> vec = {'H', 'e', 'l', 'l', 'o'};
        string_span<> tmp = vec;
        string_span<> span = tmp;
        CHECK(span.length() == 5);
#endif
    }

    // from const string_span of non-const data
    {
        std::vector<char> vec = {'H', 'e', 'l', 'l', 'o'};
        const string_span<> tmp = vec;
        const string_span<> span = tmp;
        CHECK(span.length() == 5);
    }
}

template <typename T>
T move_wrapper(T&& t)
{
    return std::move(t);
}

template <class T>
T create()
{
    return T{};
}

template <class T>
void use(basic_string_span<T, gsl::dynamic_extent>)
{
}

TEST_CASE("MoveConstructors")
{
    // move string_span
    {
        cstring_span<> span = "Hello";
        const auto span1 = std::move(span);
        CHECK(span1.length() == 5);
    }
    {
        cstring_span<> span = "Hello";
        const auto span1 = move_wrapper(std::move(span));
        CHECK(span1.length() == 5);
    }
    {
        cstring_span<> span = "Hello";
        const auto span1 = move_wrapper(std::move(span));
        CHECK(span1.length() == 5);
    }

    // move span
    {
        span<const char> span = ensure_z("Hello");
        const cstring_span<> span1 = std::move(span);
        CHECK(span1.length() == 5);
    }
    {
        span<const char> span = ensure_z("Hello");
        const cstring_span<> span2 = move_wrapper(std::move(span));
        CHECK(span2.length() == 5);
    }

    // move string
    {
#ifdef CONFIRM_COMPILATION_ERRORS
        std::string str = "Hello";
        string_span<> span = std::move(str);
        CHECK(span.length() == 5);
#endif
    }
    {
#ifdef CONFIRM_COMPILATION_ERRORS
        std::string str = "Hello";
        string_span<> span = move_wrapper<std::string>(std::move(str));
        CHECK(span.length() == 5);
#endif
    }
    {
#ifdef CONFIRM_COMPILATION_ERRORS
        use<char>(create<string>());
#endif
    }

    // move container
    {
#ifdef CONFIRM_COMPILATION_ERRORS
        std::vector<char> vec = {'H', 'e', 'l', 'l', 'o'};
        string_span<> span = std::move(vec);
        CHECK(span.length() == 5);
#endif
    }
    {
#ifdef CONFIRM_COMPILATION_ERRORS
        std::vector<char> vec = {'H', 'e', 'l', 'l', 'o'};
        string_span<> span = move_wrapper<std::vector<char>>(std::move(vec));
        CHECK(span.length() == 5);
#endif
    }
    {
#ifdef CONFIRM_COMPILATION_ERRORS
        use<char>(create<std::vector<char>>());
#endif
    }
}

TEST_CASE("Conversion")
{
#ifdef CONFIRM_COMPILATION_ERRORS
    cstring_span<> span = "Hello";
    cwstring_span<> wspan{span};
    CHECK(wspan.length() == 5);
#endif
}

czstring_span<> CreateTempName(string_span<> span)
{
    Expects(span.size() > 1);

    int last = 0;
    if (span.size() > 4) {
        span[0] = 't';
        span[1] = 'm';
        span[2] = 'p';
        last = 3;
    }
    span[last] = '\0';

    auto ret = span.subspan(0, 4);
    return {ret};
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.1) // NO-FORMAT: attribute
TEST_CASE("zstring")
{

    // create zspan from zero terminated string
    {
        char buf[1];
        buf[0] = '\0';

        zstring_span<> zspan({buf, 1});

        CHECK(generic::strlen(zspan.assume_z()) == 0);
        CHECK(zspan.as_string_span().size() == 0);
        CHECK(zspan.ensure_z().size() == 0);
    }

    // create zspan from non-zero terminated string
    {
        char buf[1];
        buf[0] = 'a';

        auto workaround_macro = [&]() { const zstring_span<> zspan({buf, 1}); };
        CHECK_THROWS_AS(workaround_macro(), fail_fast);
    }

    // usage scenario: create zero-terminated temp file name and pass to a legacy API
    {
        char buf[10];

        auto name = CreateTempName({buf, 10});
        if (!name.empty()) {
            czstring<> str = name.assume_z();
            CHECK(generic::strlen(str) == 3);
            CHECK(*(str + 3) == '\0');
        }
    }
}

cwzstring_span<> CreateTempNameW(wstring_span<> span)
{
    Expects(span.size() > 1);

    int last = 0;
    if (span.size() > 4) {
        span[0] = L't';
        span[1] = L'm';
        span[2] = L'p';
        last = 3;
    }
    span[last] = L'\0';

    auto ret = span.subspan(0, 4);
    return {ret};
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.1) // NO-FORMAT: attribute
TEST_CASE("wzstring")
{

    // create zspan from zero terminated string
    {
        wchar_t buf[1];
        buf[0] = L'\0';

        wzstring_span<> zspan({buf, 1});

        CHECK(generic::strnlen(zspan.assume_z(), 1) == 0);
        CHECK(zspan.as_string_span().size() == 0);
        CHECK(zspan.ensure_z().size() == 0);
    }

    // create zspan from non-zero terminated string
    {
        wchar_t buf[1];
        buf[0] = L'a';

        const auto workaround_macro = [&]() { const wzstring_span<> zspan({buf, 1}); };
        CHECK_THROWS_AS(workaround_macro(), fail_fast);
    }

    // usage scenario: create zero-terminated temp file name and pass to a legacy API
    {
        wchar_t buf[10];

        const auto name = CreateTempNameW({buf, 10});
        if (!name.empty()) {
            cwzstring<> str = name.assume_z();
            CHECK(generic::strnlen(str, 10) == 3);
            CHECK(*(str + 3) == L'\0');
        }
    }
}

cu16zstring_span<> CreateTempNameU16(u16string_span<> span)
{
    Expects(span.size() > 1);

    int last = 0;
    if (span.size() > 4) {
        span[0] = u't';
        span[1] = u'm';
        span[2] = u'p';
        last = 3;
    }
    span[last] = u'\0';

    auto ret = span.subspan(0, 4);
    return {ret};
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.1) // NO-FORMAT: attribute
TEST_CASE("u16zstring")
{

    // create zspan from zero terminated string
    {
        char16_t buf[1];
        buf[0] = L'\0';

        u16zstring_span<> zspan({buf, 1});

        CHECK(generic::strnlen(zspan.assume_z(), 1) == 0);
        CHECK(zspan.as_string_span().size() == 0);
        CHECK(zspan.ensure_z().size() == 0);
    }

    // create zspan from non-zero terminated string
    {
        char16_t buf[1];
        buf[0] = u'a';

        const auto workaround_macro = [&]() { const u16zstring_span<> zspan({buf, 1}); };
        CHECK_THROWS_AS(workaround_macro(), fail_fast);
    }

    // usage scenario: create zero-terminated temp file name and pass to a legacy API
    {
        char16_t buf[10];

        const auto name = CreateTempNameU16({buf, 10});
        if (!name.empty()) {
            cu16zstring<> str = name.assume_z();
            CHECK(generic::strnlen(str, 10) == 3);
            CHECK(*(str + 3) == L'\0');
        }
    }
}

cu32zstring_span<> CreateTempNameU32(u32string_span<> span)
{
    Expects(span.size() > 1);

    int last = 0;
    if (span.size() > 4) {
        span[0] = U't';
        span[1] = U'm';
        span[2] = U'p';
        last = 3;
    }
    span[last] = U'\0';

    auto ret = span.subspan(0, 4);
    return {ret};
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.1) // NO-FORMAT: attribute
TEST_CASE("u32zstring")
{

    // create zspan from zero terminated string
    {
        char32_t buf[1];
        buf[0] = L'\0';

        u32zstring_span<> zspan({buf, 1});

        CHECK(generic::strnlen(zspan.assume_z(), 1) == 0);
        CHECK(zspan.as_string_span().size() == 0);
        CHECK(zspan.ensure_z().size() == 0);
    }

    // create zspan from non-zero terminated string
    {
        char32_t buf[1];
        buf[0] = u'a';

        const auto workaround_macro = [&]() { const u32zstring_span<> zspan({buf, 1}); };
        CHECK_THROWS_AS(workaround_macro(), fail_fast);
    }

    // usage scenario: create zero-terminated temp file name and pass to a legacy API
    {
        char32_t buf[10];

        const auto name = CreateTempNameU32({buf, 10});
        if (!name.empty()) {
            cu32zstring<> str = name.assume_z();
            CHECK(generic::strnlen(str, 10) == 3);
            CHECK(*(str + 3) == L'\0');
        }
    }
}

TEST_CASE("Issue305")
{
    std::map<gsl::cstring_span<>, int> foo = {{"foo", 0}, {"bar", 1}};
    CHECK(foo["foo"] == 0);
    CHECK(foo["bar"] == 1);
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.3) // NO-FORMAT: attribute
TEST_CASE("char16_t type")
{
    gsl::cu16string_span<> ss1 = gsl::ensure_z(u"abc");
    CHECK(ss1.size() == 3);
    CHECK(ss1.size_bytes() == 6);

    std::u16string s1 = gsl::to_string(ss1);
    CHECK(s1 == u"abc");

    std::u16string s2 = u"abc";
    gsl::u16string_span<> ss2 = s2;
    CHECK(ss2.size() == 3);

    gsl::u16string_span<> ss3 = ss2.subspan(1, 1);
    CHECK(ss3.size() == 1);
    CHECK(ss3[0] == u'b');

    char16_t buf[4]{u'a', u'b', u'c', u'\0'};
    gsl::u16string_span<> ss4{buf, 4};
    CHECK(ss4[3] == u'\0');

    gsl::cu16zstring_span<> ss5(u"abc");
    CHECK(ss5.as_string_span().size() == 3);

    gsl::cu16string_span<> ss6 = ss5.as_string_span();
    CHECK(ss6 == ss1);

    std::vector<char16_t> v7 = {u'a', u'b', u'c'};
    gsl::cu16string_span<> ss7{v7};
    CHECK(ss7 == ss1);

    gsl::cu16string_span<> ss8 = gsl::ensure_z(u"abc");
    gsl::cu16string_span<> ss9 = gsl::ensure_z(u"abc");
    CHECK(ss8 == ss9);

    ss9 = gsl::ensure_z(u"abd");
    CHECK(ss8 < ss9);
    CHECK(ss8 <= ss9);
    CHECK(ss8 != ss9);
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
GSL_SUPPRESS(bounds.3) // NO-FORMAT: attribute
TEST_CASE("char32_t type")
{
    gsl::cu32string_span<> ss1 = gsl::ensure_z(U"abc");
    CHECK(ss1.size() == 3);
    CHECK(ss1.size_bytes() == 12);

    std::u32string s1 = gsl::to_string(ss1);
    CHECK(s1 == U"abc");

    std::u32string s2 = U"abc";
    gsl::u32string_span<> ss2 = s2;
    CHECK(ss2.size() == 3);

    gsl::u32string_span<> ss3 = ss2.subspan(1, 1);
    CHECK(ss3.size() == 1);
    CHECK(ss3[0] == U'b');

    char32_t buf[4]{U'a', U'b', U'c', U'\0'};
    gsl::u32string_span<> ss4{buf, 4};
    CHECK(ss4[3] == u'\0');

    gsl::cu32zstring_span<> ss5(U"abc");
    CHECK(ss5.as_string_span().size() == 3);

    gsl::cu32string_span<> ss6 = ss5.as_string_span();
    CHECK(ss6 == ss1);

    gsl::cu32string_span<> ss8 = gsl::ensure_z(U"abc");
    gsl::cu32string_span<> ss9 = gsl::ensure_z(U"abc");
    CHECK(ss8 == ss9);

    ss9 = gsl::ensure_z(U"abd");
    CHECK(ss8 < ss9);
    CHECK(ss8 <= ss9);
    CHECK(ss8 != ss9);
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("as_bytes")
{
    cwzstring_span<> v(L"qwerty");
    const auto s = v.as_string_span();
    const auto bs = as_bytes(s);
    CHECK(static_cast<const void*>(bs.data()) == static_cast<const void*>(s.data()));
    CHECK(bs.size() == s.size_bytes());
}

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
TEST_CASE("as_writeable_bytes")
{
    wchar_t buf[]{L"qwerty"};
    wzstring_span<> v(buf);
    const auto s = v.as_string_span();
    const auto bs = as_writeable_bytes(s);
    CHECK(static_cast<const void*>(bs.data()) == static_cast<const void*>(s.data()));
    CHECK(bs.size() == s.size_bytes());
}
