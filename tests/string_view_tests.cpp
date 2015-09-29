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
#include <string_view.h>
#include <vector>
#include <cstdlib>

using namespace std;
using namespace gsl;

SUITE(string_view_tests)
{

    TEST(TestLiteralConstruction)
	{
        cwstring_view<> v = ensure_z(L"Hello");

        CHECK(5 == v.length());

#ifdef CONFIRM_COMPILATION_ERRORS
        wstring_view<> v2 = ensure0(L"Hello");
#endif
	}

    TEST(TestConstructFromStdString)
    {
        std::string s = "Hello there world";
        cstring_view<> v = s;
        CHECK(v.length() == s.length());
    }

    TEST(TestConstructFromStdVector)
    {
        std::vector<char> vec('h', 5);
        string_view<> v = vec;
        CHECK(v.length() == vec.size());
    }

	TEST(TestStackArrayConstruction)
	{
        wchar_t stack_string[] = L"Hello";

        {
            cwstring_view<> v = ensure_z(stack_string);
            CHECK(v.length() == 5);
            CHECK(v.used_length() == v.length());
        }

        {
            cwstring_view<> v = stack_string;
            CHECK(v.length() == 6);
            CHECK(v.used_length() == v.length());
        }

        {
            wstring_view<> v = ensure_z(stack_string);
            CHECK(v.length() == 5);
            CHECK(v.used_length() == v.length());
        }

        {
            wstring_view<> v = stack_string;
            CHECK(v.length() == 6);
            CHECK(v.used_length() == v.length());
        }
	}

    TEST(TestConversionToConst)
    {
        char stack_string[] = "Hello";
        string_view<> v = ensure_z(stack_string);
        cstring_view<> v2 = v; 
        CHECK(v.length() == v2.length());
    }

    TEST(TestConversionFromConst)
    {
        char stack_string[] = "Hello";
        cstring_view<> v = ensure_z(stack_string);
#ifdef CONFIRM_COMPILATION_ERRORS
        string_view<> v2 = v;
        string_view<> v3 = "Hello";
#endif
    }
}

int main(int, const char *[])
{
    return UnitTest::RunAllTests();
}
