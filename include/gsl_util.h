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

#include "gsl_assert.h"  // GSL_ENSURES/GSL_EXPECTS
#include <array>
#include <utility>
#include <exception>

#define GSL_IMPL_CONSTEXPR constexpr
#define GSL_IMPL_NOEXCEPT noexcept

#ifdef _MSC_VER

// No MSVC does constexpr fully yet
#undef GSL_IMPL_CONSTEXPR
#define GSL_IMPL_CONSTEXPR

// MSVC 2013 workarounds
#if _MSC_VER <= 1800
// noexcept is not understood 
#undef GSL_IMPL_NOEXCEPT
#define GSL_IMPL_NOEXCEPT

// turn off some misguided warnings
#pragma warning(push)
#pragma warning(disable: 4351) // warns about newly introduced aggregate initializer behavior

#endif // _MSC_VER <= 1800

#endif // _MSC_VER


namespace gsl
{
//
// GSL.util: utilities
//

// final_act allows you to ensure something gets run at the end of a scope
template <class F>
class final_act
{
public:
    explicit final_act(F f) GSL_IMPL_NOEXCEPT
    : f_(std::move(f)), invoke_(true)
    {}

    final_act(final_act&& other) GSL_IMPL_NOEXCEPT
    : f_(std::move(other.f_)), invoke_(other.invoke_)
    { other.invoke_ = false; }
    
    final_act(const final_act&) = delete;
    final_act& operator=(const final_act&) = delete;

    ~final_act() GSL_IMPL_NOEXCEPT { if (invoke_) f_(); }

private:
    F f_;
    bool invoke_;
};

// finally() - convenience function to generate a final_act
template <class F>
inline final_act<F> finally(const F &f)
GSL_IMPL_NOEXCEPT { return final_act<F>(f); }

template <class F>
inline final_act<F> finally(F &&f) GSL_IMPL_NOEXCEPT
{ return final_act<F>(std::forward<F>(f)); }

// narrow_cast(): a searchable way to do narrowing casts of values
template<class T, class U>
inline GSL_IMPL_CONSTEXPR T narrow_cast(U u) GSL_IMPL_NOEXCEPT
{ return static_cast<T>(u); }

struct narrowing_error : public std::exception {};

// narrow() : a checked version of narrow_cast() that throws if the cast changed the value
template<class T, class U>
inline T narrow(U u)
{ T t = narrow_cast<T>(u); if (static_cast<U>(t) != u) throw narrowing_error(); return t; }

//
// at() - Bounds-checked way of accessing static arrays, std::array, std::vector
//
template <class T, size_t N> 
GSL_IMPL_CONSTEXPR T& at(T(&arr)[N], size_t index)
{ GSL_EXPECTS(index < N); return arr[index]; }

template <class T, size_t N>
GSL_IMPL_CONSTEXPR T& at(std::array<T, N>& arr, size_t index)
{ GSL_EXPECTS(index < N); return arr[index]; }

template <class Cont>
GSL_IMPL_CONSTEXPR typename Cont::value_type& at(Cont& cont, size_t index)
{ GSL_EXPECTS(index < cont.size()); return cont[index]; }

} // namespace gsl


#undef GSL_IMPL_CONSTEXPR
#undef GSL_IMPL_NOEXCEPT

#if defined(_MSC_VER) and (_MSC_VER <= 1800)
#pragma warning(pop)
#endif // _MSC_VER <= 1800

#endif // GSL_UTIL_H
