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

#include <cstdlib>  // for std::exit
#include <gsl/span> // for span

int operator_subscript_no_throw() noexcept
{
    int arr[10];
    const gsl::span<int> sp{arr};
    return sp[11];
}

[[noreturn]] void test_terminate() { std::exit(0); }

void setup_termination_handler() noexcept
{
#if defined(_MSC_VER)

    auto& handler = gsl::details::get_terminate_handler();
    handler = &test_terminate;

#else

    std::set_terminate(test_terminate);

#endif
}

int main() noexcept
{
    setup_termination_handler();
    operator_subscript_no_throw();
    return -1;
}
