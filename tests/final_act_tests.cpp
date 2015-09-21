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
#include <exception>

using namespace std;
using namespace Guide;

struct Increment {
  int* i_;

  Increment(int& i) : i_(&i) {}

  void operator()() const noexcept { ++*i_; }
};

struct ThrowOnInvocation {
  class Exception : public exception {};
  void operator()() const { throw Exception(); }
};

SUITE(finallyTests) {
  TEST(SingleInvocationOnConvenienceConstruction) {
    int i = 0;
    { auto f = finally(Increment(i)); }
    CHECK(i == 1);
  }

  TEST(SingleInvocationOnMoveConstruction) {
    int i = 0;
    {
      auto f = Final_act<Increment>(Increment(i));
      auto ff(move(f));
    }
    CHECK(i == 1);
  }

  TEST(NoexceptFinalActIsCreated) {
    int i = 0;
    auto f = finally(Increment(i));
    CHECK((is_same<decltype(f), Final_act_noexcept<Increment>>::value));
  }

  TEST(FinalActDoesNotThrow) {
    bool exceptionCaught = false;
    try {
      finally(ThrowOnInvocation());
    } catch (...) {
      exceptionCaught = true;
    }
    CHECK(!exceptionCaught);
  }
}

int main(int, const char* []) { return UnitTest::RunAllTests(); }
