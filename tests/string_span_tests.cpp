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
#include <string_span.h>
#include <vector>
#include <cstdlib>

using namespace std;
using namespace gsl;


SUITE(string_span_tests)
{

    TEST(TestLiteralConstruction)
	{
        cwstring_span<> v = ensure_z(L"Hello");

        CHECK(5 == v.length());

#ifdef CONFIRM_COMPILATION_ERRORS
        wstring_span<> v2 = ensure0(L"Hello");
#endif
	}

    TEST(TestConstructFromStdString)
    {
        std::string s = "Hello there world";
        cstring_span<> v = s;
        CHECK(v.length() == s.length());
    }

    TEST(TestConstructFromStdVector)
    {
        std::vector<char> vec(5, 'h');
        string_span<> v = vec;
        CHECK(v.length() == vec.size());
    }

	TEST(TestStackArrayConstruction)
	{
        wchar_t stack_string[] = L"Hello";

        {
            cwstring_span<> v = ensure_z(stack_string);
            CHECK(v.length() == 5);
            CHECK(v.used_length() == v.length());
        }

        {
            cwstring_span<> v = stack_string;
            CHECK(v.length() == 6);
            CHECK(v.used_length() == v.length());
        }

        {
            wstring_span<> v = ensure_z(stack_string);
            CHECK(v.length() == 5);
            CHECK(v.used_length() == v.length());
        }

        {
            wstring_span<> v = stack_string;
            CHECK(v.length() == 6);
            CHECK(v.used_length() == v.length());
        }
	}

    TEST(TestConstructFromConstCharPointer)
    {
        const char* s = "Hello";
        cstring_span<> v = ensure_z(s);
        CHECK(v.length() == 5);
        CHECK(v.used_length() == v.length());
    }

    TEST(TestConversionToConst)
    {
        char stack_string[] = "Hello";
        string_span<> v = ensure_z(stack_string);
        cstring_span<> v2 = v; 
        CHECK(v.length() == v2.length());
    }

    TEST(TestConversionFromConst)
    {
        char stack_string[] = "Hello";
        cstring_span<> v = ensure_z(stack_string);
#ifdef CONFIRM_COMPILATION_ERRORS
        string_span<> v2 = v;
        string_span<> v3 = "Hello";
#endif
    }

    TEST(TestToString)
    {
        auto s = gsl::to_string(cstring_span<>{});
        CHECK(s.length() == 0);

        char stack_string[] = "Hello";
        cstring_span<> v = ensure_z(stack_string);
        auto s2 = gsl::to_string(v);
        CHECK(s2.length() == v.length());
        CHECK(s2.length() == 5);
    }
}

int main(int, const char *[])
{
    return UnitTest::RunAllTests();
}
