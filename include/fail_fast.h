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

#ifndef GSL_FAIL_FAST_H
#define GSL_FAIL_FAST_H

#include <exception>

#if defined(GSL_THROWS_FOR_TESTING)
#include <stdexcept>
#endif

namespace gsl
{

//
// Having "fail fast" result in an exception makes unit testing
// the GSL classes that rely upon it much simpler. 
//
#if defined(GSL_THROWS_FOR_TESTING)

struct fail_fast : public std::runtime_error 
{
	fail_fast() : std::runtime_error("") {}
	explicit fail_fast(char const* const message) : std::runtime_error(message) {}
};

inline void fail_fast_assert(bool cond) { if (!cond) throw fail_fast(); }
inline void fail_fast_assert(bool cond, const char* const message) { if (!cond) throw fail_fast(message); }

#else

inline void fail_fast_assert(bool cond) { if (!cond) std::terminate(); }
inline void fail_fast_assert(bool cond, const char* const) { if (!cond) std::terminate(); }

#endif // GSL_THROWS_FOR_TESTING

}

#endif // GSL_FAIL_FAST_H
