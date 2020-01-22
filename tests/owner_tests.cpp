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

#include <gtest/gtest.h>

#include <gsl/pointers> // for owner

using namespace gsl;

GSL_SUPPRESS(f.23) // NO-FORMAT: attribute
void f(int* i) { *i += 1; }

TEST(owner_tests, basic_test)
{
    owner<int*> p = new int(120);
    EXPECT_TRUE(*p == 120);
    f(p);
    EXPECT_TRUE(*p == 121);
    delete p;
}

TEST(owner_tests, check_pointer_constraint)
{
    #ifdef CONFIRM_COMPILATION_ERRORS
    {
        owner<int> integerTest = 10;
        owner<std::shared_ptr<int>> sharedPtrTest(new int(10));
    }
    #endif
}
