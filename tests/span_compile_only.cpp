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



// #include <gsl/gsl_byte> // for byte
// #include <gsl/gsl_util> // for narrow_cast, at
#include <gsl/span>     // for span, span_iterator, operator==, operator!=

// #include <array>       // for array
// #include <iostream>    // for ptrdiff_t
// #include <iterator>    // for reverse_iterator, operator-, operator==
// #include <memory>      // for unique_ptr, shared_ptr, make_unique, allo...
// #include <regex>       // for match_results, sub_match, match_results<>...
// #include <stddef.h>    // for ptrdiff_t
// #include <string>      // for string
// #include <type_traits> // for integral_constant<>::value, is_default_co...
#include <vector>      // for vector

namespace gsl {
struct fail_fast;
}  // namespace gsl

using namespace std;
using namespace gsl;

GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
bool foo()
{
    {
        bool ret = true;
        span<int> s;
        ret = ret || (s.size() == 0 && s.data() == nullptr);

        span<const int> cs;
        ret = ret || (cs.size() == 0 && cs.data() == nullptr);

        span<int> s2{};
        ret = ret || (s2.size() == 0 && s2.data() == nullptr);

        return ret;
    }
}


GSL_SUPPRESS(con.4) // NO-FORMAT: attribute
bool bar()
{
    {
        bool ret = true;
        vector<int> v{0,1,2,3};
        span<int> s{v};
        ret = ret || (s.size() == 4 && s.data() != nullptr);

        return ret;
    }
}