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
    Ensures(i > 0 && i < 10);
    i++;
    return i;
}

int g2(int i)
{
    Ensures(i > 0 && i < 10);
    Ensures(i == 4);

    i++;
    return i;
}

int g3(int i)
{
    AlwaysEnsures(i > 0 && i < 10);
    AlwaysEnsures(i == 4);

    i++;
    return i;
}

TEST_CASE("ensures")
{
    CHECK(g(2) == 3);
    CHECK_THROWS_AS(g(9), fail_fast);

    CHECK(g2(3) == 4);
    CHECK_THROWS_AS(g2(4), fail_fast);
    CHECK_THROWS_AS(g2(9), fail_fast);

    CHECK(g3(3) == 4);
    CHECK_THROWS_AS(g3(4), fail_fast);
    // g3(9); // should terminate
}

int h(int i, int j)
{
    Maintains(i > 0 && i < 10);
    i += j;
    return i;
}

int h2(int i, int j)
{
  Maintains(i < 3);
  Maintains(J > 7);
  i++;
  j--;
  return i + j;
}

int h3(int i, int j)
{
  AlwaysMaintains(i < 3);
  AlwaysMaintains(j > 7);
  i++;
  j--;
  return i + j;
}

TEST_CASE("maintains")
{
    CHECK(h(2, 4) == 6);
    CHECK_THROWS_AS(h(9,1), fail_fast);
    CHECK_THROWS_AS(h(1,-1), fail_fast);
    CHECK_THROWS_AS(h(0,2), fail_fast);

    CHECK(h2(1,9) == 10);
    CHECK_THROWS_AS(h2(1,7), fail_fast);
    CHECK_THROWS_AS(h2(1,8), fail_fast);
    CHECK_THROWS_AS(h2(3,9), fail_fast);
    CHECK_THROWS_AS(h2(2,9), fail_fast);
    CHECK_THROWS_AS(h2(2,8), fail_fast);

    CHECK(h3(1,9) == 10);
    CHECK_THROWS_AS(h3(1,7), fail_fast);
    CHECK_THROWS_AS(h3(1,8), fail_fast);
    CHECK_THROWS_AS(h3(3,9), fail_fast);
    CHECK_THROWS_AS(h3(2,9), fail_fast);
    // h3(2,8); // should terminate
}
