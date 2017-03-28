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

#include <gsl/gsl_util> // for narrow_cast, at
#include <gsl/span>     // for span, span_iterator, operator==, operator!=

#include "test_noexcept.h"

using namespace gsl;

TEST_CASE("narrow_no_throw")
{
    {
        long long arr[10];
        span<long long> sp { arr };
        arr[9] = 0x0fffffffffffffff;
        return  narrow<int>(sp[9]); // narrow should terminate
    }
}

TEST_CASE("operator_subscript_no_throw")
{
    {
        long long arr[10];
        span<long long> sp { arr };
        arr[9] = 0x0fffffffffffffff;
        return narrow<int>(sp[11]); // span::operator[] should terminate
    }
}
