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

using namespace gsl;

SUITE(assertion_tests)
{
    int f(int i)
    {
        Expects(i > 0 && i < 10);
        return i;
    }

    TEST(expects)
    {
        CHECK(f(2) == 2);
        CHECK_THROW(f(10), fail_fast);
        try { f(10); }
        catch (fail_fast const& e) {
            cstring_span<> prefix = ensure_z("GSL: Precondition failure");
            cstring_span<> what = ensure_z(e.what()).first(prefix.length());
            CHECK(prefix == what);
        }
        CHECK_THROW(f(10), std::logic_error);
        try { f(10); }
        catch (std::logic_error const& e) {
            cstring_span<> prefix = ensure_z("GSL: Precondition failure");
            cstring_span<> what = ensure_z(e.what()).first(prefix.length());
            CHECK(prefix == what);
        }
    }

    int g(int i)
    {        
        i++;
        Ensures(i > 0 && i < 10);
        return i;
    }

    TEST(ensures)
    {
        CHECK(g(2) == 3);
        CHECK_THROW(g(9), fail_fast);
        try { g(9); }
        catch (fail_fast const& e) {
            cstring_span<> prefix = ensure_z("GSL: Postcondition failure");
            cstring_span<> what = ensure_z(e.what()).first(prefix.length());
            CHECK(prefix == what);
        }
        CHECK_THROW(g(9), std::runtime_error);
        try { g(9); }
        catch (std::runtime_error const& e) {
            cstring_span<> prefix = ensure_z("GSL: Postcondition failure");
            cstring_span<> what = ensure_z(e.what()).first(prefix.length());
            CHECK(prefix == what);
        }
   }
}

int main(int, const char *[])
{
    return UnitTest::RunAllTests();
}
