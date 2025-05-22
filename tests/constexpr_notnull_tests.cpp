///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2025 Microsoft Corporation. All rights reserved.
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

#include <gsl/pointers> // for not_null
#include <gtest/gtest.h>

#include <type_traits> // for declval

using namespace gsl;

namespace
{
constexpr bool comparison_test(const int* ptr1, const int* ptr2)
{
    const not_null<const int*> p1(ptr1);
    const not_null<const int*> p1_same(ptr1);
    const not_null<const int*> p2(ptr2);
    
    // Testing operator==
    const bool eq_result = (p1 == p1_same); // Should be true
    const bool neq_result = (p1 != p2);     // Should be true
    
    // Testing operator<= and operator>=
    const bool le_result = (p1 <= p1_same); // Should be true
    const bool ge_result = (p1 >= p1_same); // Should be true
    
    // The exact comparison results will depend on pointer ordering,
    // but we can verify that the basic equality checks work as expected
    return eq_result && neq_result && le_result && ge_result;
}

constexpr bool workaround_test(const int* ptr1, const int* ptr2)
{
    const not_null<const int*> p1(ptr1);
    const not_null<const int*> p1_same(ptr1);
    const not_null<const int*> p2(ptr2);
    
    // Using .get() to compare
    const bool eq_result = (p1.get() == p1_same.get()); // Should be true
    const bool neq_result = (p1.get() != p2.get());     // Should be true
    
    return eq_result && neq_result;
}
} // namespace

constexpr int test_value1 = 1;
constexpr int test_value2 = 2;

static_assert(comparison_test(&test_value1, &test_value2), "not_null comparison operators should be constexpr");
static_assert(workaround_test(&test_value1, &test_value2), "not_null .get() comparison workaround should work");

TEST(notnull_constexpr_tests, TestNotNullConstexprComparison)
{
    // This test simply verifies that the constexpr functions compile and run
    // If we got here, it means the constexpr comparison operators are working
    static const int value1 = 1;
    static const int value2 = 2;
    EXPECT_TRUE(comparison_test(&value1, &value2));
    EXPECT_TRUE(workaround_test(&value1, &value2));
}

