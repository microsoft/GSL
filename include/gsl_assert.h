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

//
// There are three configuration options for this GSL implementation's behavior
// when pre/post conditions on the GSL types are violated:
//
// 1. GSL_TERMINATE_ON_CONTRACT_VIOLATION: std::terminate will be called (default)
// 2. GSL_THROW_ON_CONTRACT_VIOLATION: a gsl::fail_fast exception will be thrown
// 3. GSL_UNENFORCED_ON_CONTRACT_VIOLATION: nothing happens  
//
#if !(defined(GSL_THROW_ON_CONTRACT_VIOLATION) ^ defined(GSL_TERMINATE_ON_CONTRACT_VIOLATION) ^ defined(GSL_UNENFORCED_ON_CONTRACT_VIOLATION))
#define GSL_TERMINATE_ON_CONTRACT_VIOLATION 
#endif


#define GSL_STRINGIFY_DETAIL(x) #x
#define GSL_STRINGIFY(x) GSL_STRINGIFY_DETAIL(x)


//
// GSL.assert: assertions
//

#if defined(GSL_THROW_ON_CONTRACT_VIOLATION)

#include <stdexcept>

namespace gsl
{
struct fail_fast : public std::runtime_error 
{
	explicit fail_fast(char const* const message) : std::runtime_error(message) {}   
};
}

#define GSL_EXPECTS(cond)  if (!(cond)) \
    throw gsl::fail_fast("GSL: Precondition failure at " __FILE__ ": " GSL_STRINGIFY(__LINE__));
#define GSL_ENSURES(cond)  if (!(cond)) \
    throw gsl::fail_fast("GSL: Postcondition failure at " __FILE__ ": " GSL_STRINGIFY(__LINE__));


#elif defined(GSL_TERMINATE_ON_CONTRACT_VIOLATION)


#define GSL_EXPECTS(cond)           if (!(cond)) std::terminate(); 
#define GSL_ENSURES(cond)           if (!(cond)) std::terminate();


#elif defined(GSL_UNENFORCED_ON_CONTRACT_VIOLATION)

#define GSL_EXPECTS(cond)           
#define GSL_ENSURES(cond)           

#endif 

// Keep the global macro-namespace clean by default,
// but offer the dirty solution as well:
#if defined(GSL_USE_UNPREFIXED_CONTRACTS)
#define EXPECTS(cond) GSL_EXPECTS(cond)
#define ENSURES(cond) GSL_ENSURES(cond)
#endif

// For those who like to live dangerous,
// offer macros that are not ALL_CAPS:
#if defined(GSL_USE_UNPREFIXED_AND_LOWERCASE_CONTRACTS)
#define Expects(cond) GSL_EXPECTS(cond)
#define Ensures(cond) GSL_ENSURES(cond)
#endif

#endif // GSL_CONTRACTS_H
