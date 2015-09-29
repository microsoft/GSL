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
#include <vector>

using namespace std;
using namespace gsl;

SUITE(at_tests)
{
    TEST(static_array)
    {
        int a[] = { 1, 2, 3, 4 };

        for (int i = 0; i < 4; ++i)
            CHECK(at(a, i) == i+1);

        CHECK_THROW(at(a, 4), fail_fast);
    }

    TEST(std_array)
    {
        std::array<int,4> a = { 1, 2, 3, 4 };

        for (int i = 0; i < 4; ++i)
            CHECK(at(a, i) == i+1);

        CHECK_THROW(at(a, 4), fail_fast);
    }

    TEST(StdVector)
    {
        std::vector<int> a = { 1, 2, 3, 4 };

        for (int i = 0; i < 4; ++i)
            CHECK(at(a, i) == i+1);

        CHECK_THROW(at(a, 4), fail_fast);
    }
}

int main(int, const char *[])
{
    return UnitTest::RunAllTests();
}
