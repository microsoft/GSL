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
constexpr bool comparison_test()
{
    int value1 = 1;
    int value2 = 2;
    
    const not_null<int*> p1(&value1);
    const not_null<int*> p1_same(&value1);
    const not_null<int*> p2(&value2);
    
    // Testing operator==
    const bool eq_result = (p1 == p1_same); // Should be true
    const bool neq_result = (p1 != p2);     // Should be true
    
    // Testing operator< and operator>
    const bool lt_result = (p1 < p2);      // Implementation dependent
    const bool gt_result = (p2 > p1);      // Should be the same as lt_result
    
    // Testing operator<= and operator>=
    const bool le_result = (p1 <= p1_same); // Should be true
    const bool ge_result = (p1 >= p1_same); // Should be true
    
    // The exact comparison results will depend on pointer ordering,
    // but we can verify that the equality checks work as expected
    return eq_result && neq_result && le_result && ge_result && (lt_result == gt_result);
}

constexpr bool workaround_test()
{
    int value1 = 1;
    int value2 = 2;
    
    const not_null<int*> p1(&value1);
    const not_null<int*> p1_same(&value1);
    const not_null<int*> p2(&value2);
    
    // Using .get() to compare
    const bool eq_result = (p1.get() == p1_same.get()); // Should be true
    const bool neq_result = (p1.get() != p2.get());     // Should be true
    
    return eq_result && neq_result;
}
} // namespace

// Only enable these static_assert tests for C++14 and above with compilers that
// fully support relaxed constexpr requirements
#if defined(__cpp_constexpr) && (__cpp_constexpr >= 201304) && \
    !(defined(_MSC_VER) && _MSC_VER < 1910) && \
    !(defined(__GNUC__) && !defined(__clang__) && __GNUC__ < 5)
static_assert(comparison_test(), "not_null comparison operators should be constexpr");
static_assert(workaround_test(), "not_null .get() comparison workaround should work");
#endif

TEST(notnull_constexpr_tests, TestNotNullConstexprComparison)
{
    // This test simply verifies that the static_assert above compiles
    // If we got here, it means the constexpr comparison operators are working
    EXPECT_TRUE(comparison_test());
    EXPECT_TRUE(workaround_test());
}