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

#include <catch/catch.hpp> // for AssertionHandler, StringRef, CHECK, CHECK...

#include <gsl/gsl_assert> // for fail_fast (ptr only), Ensures, Expects

using namespace gsl;

int f(int i)
{
    Expects(i > 0 && i < 10);
    return i;
}

TEST_CASE("expects")
{
    CHECK(f(2) == 2);
    CHECK_THROWS_AS(f(10), fail_fast);
}

int g(int i)
{
    i++;
    Ensures(i > 0 && i < 10);
    return i;
}

TEST_CASE("ensures")
{
    CHECK(g(2) == 3);
    CHECK_THROWS_AS(g(9), fail_fast);
}
