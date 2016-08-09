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

#pragma once

#ifndef GSL_CONTRACTS_H
#define GSL_CONTRACTS_H

#include <exception>
#include <stdexcept>

#ifdef _MSC_VER

// MSVC 2013 workarounds
#if _MSC_VER <= 1800
// noexcept is not understood
#pragma push_macro("noexcept")
#define noexcept

#endif // _MSC_VER <= 1800

#endif // _MSC_VER

//
// There are three configuration options for this GSL implementation's behavior
// when pre/post conditions on the GSL types are violated:
//
// 1. GSL_TERMINATE_ON_CONTRACT_VIOLATION: std::terminate will be called (default)
// 2. GSL_THROW_ON_CONTRACT_VIOLATION: a gsl::fail_fast exception will be thrown
// 3. GSL_UNENFORCED_ON_CONTRACT_VIOLATION: nothing happens
//
#if !(defined(GSL_THROW_ON_CONTRACT_VIOLATION) ^ defined(GSL_TERMINATE_ON_CONTRACT_VIOLATION) ^    \
      defined(GSL_UNENFORCED_ON_CONTRACT_VIOLATION))
#define GSL_TERMINATE_ON_CONTRACT_VIOLATION
#endif

#define GSL_STRINGIFY_DETAIL(x) #x
#define GSL_STRINGIFY(x) GSL_STRINGIFY_DETAIL(x)

//
// GSL.assert: assertions
//

namespace gsl
{
struct fail_fast
{
    virtual char const* what() const noexcept = 0;
};

namespace details
{
    template<typename E>
    struct contract_violation : public E, public fail_fast
    {
        explicit contract_violation(char const* const message) : E(message) {}
        virtual char const* what() const noexcept override
        {
            return E::what();
        }
    };
}
}

#if defined(GSL_THROW_ON_CONTRACT_VIOLATION)

#define Expects(cond)                                                                              \
    if (!(cond))                                                                                   \
        throw gsl::details::contract_violation<std::logic_error>("GSL: Precondition failure at " __FILE__ ": " GSL_STRINGIFY(__LINE__));
#define Ensures(cond)                                                                              \
    if (!(cond))                                                                                   \
        throw gsl::details::contract_violation<std::runtime_error>("GSL: Postcondition failure at " __FILE__ ": " GSL_STRINGIFY(__LINE__));

#elif defined(GSL_TERMINATE_ON_CONTRACT_VIOLATION)

#define Expects(cond)                                                                              \
    if (!(cond)) std::terminate();
#define Ensures(cond)                                                                              \
    if (!(cond)) std::terminate();

#elif defined(GSL_UNENFORCED_ON_CONTRACT_VIOLATION)

#define Expects(cond)
#define Ensures(cond)

#endif

#ifdef _MSC_VER

#if _MSC_VER <= 1800

#undef noexcept
#pragma pop_macro("noexcept")

#endif // _MSC_VER <= 1800

#endif // _MSC_VER


#endif // GSL_CONTRACTS_H
