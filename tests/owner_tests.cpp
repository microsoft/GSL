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

#include <gsl/pointers> // for owner

using namespace gsl;

GSL_SUPPRESS(f.23) // NO-FORMAT: attribute
void f(int* i) { *i += 1; }

GSL_SUPPRESS(r.11) // NO-FORMAT: attribute
GSL_SUPPRESS(r.3) // NO-FORMAT: attribute // TODO: false positive
GSL_SUPPRESS(r.5) // NO-FORMAT: attribute
TEST_CASE("basic_test")
{
    owner<int*> p = new int(120);
    CHECK(*p == 120);
    f(p);
    CHECK(*p == 121);
    delete p;
}

TEST_CASE("check_pointer_constraint")
{
    #ifdef CONFIRM_COMPILATION_ERRORS
    {
        owner<int> integerTest = 10;
        owner<std::shared_ptr<int>> sharedPtrTest(new int(10));
    }
    #endif
}
