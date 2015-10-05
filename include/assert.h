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

#ifndef GSL_UTIL_H
#define GSL_UTIL_H

#include "fail_fast.h"

#ifdef _MSC_VER

// No MSVC does constexpr fully yet
#pragma push_macro("constexpr")
#define constexpr /* nothing */

// MSVC 2013 workarounds
#if _MSC_VER <= 1800

// noexcept is not understood 
#ifndef GSL_THROWS_FOR_TESTING
#define noexcept /* nothing */ 
#endif

// turn off some misguided warnings
#pragma warning(push)
#pragma warning(disable: 4351) // warns about newly introduced aggregate initializer behavior

#endif // _MSC_VER <= 1800

#endif // _MSC_VER

// In order to test the library, we need it to throw exceptions that we can catch
#ifdef GSL_THROWS_FOR_TESTING
#define noexcept /* nothing */ 
#endif // GSL_THROWS_FOR_TESTING 

namespace gsl {

//
// GSL.assert: assertions
//
#define Expects(x)  gsl::fail_fast_assert((x))
#define Ensures(x)  gsl::fail_fast_assert((x))

} // namespace gsl

#ifdef _MSC_VER

#undef constexpr
#pragma pop_macro("constexpr")

#if _MSC_VER <= 1800
#pragma warning(pop)

#ifndef GSL_THROWS_FOR_TESTING
#pragma undef noexcept
#endif // GSL_THROWS_FOR_TESTING

#endif // _MSC_VER <= 1800

#endif // _MSC_VER

#if defined(GSL_THROWS_FOR_TESTING) 
#undef noexcept 
#endif // GSL_THROWS_FOR_TESTING 

#endif // GSL_UTIL_H
