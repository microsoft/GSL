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

#ifndef GSL_GSL_H
#define GSL_GSL_H

#include "span.h"           // span, strided_span...
#include "string_span.h"    // zstring, string_span, zstring_builder...
#include <memory>

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


namespace gsl
{

//
// GSL.owner: ownership pointers 
//
using std::unique_ptr;
using std::shared_ptr;

template <class T>
using owner = T;

//
// GSL.assert: assertions
//
#define Expects(x)  gsl::fail_fast_assert((x))
#define Ensures(x)  gsl::fail_fast_assert((x))

//
// GSL.util: utilities
//

// final_act allows you to ensure something gets run at the end of a scope
template <class F>
class final_act
{
public:
    explicit final_act(F f) noexcept : f_(std::move(f)), invoke_(true) {}

    final_act(final_act&& other) noexcept : f_(std::move(other.f_)), invoke_(other.invoke_) { other.invoke_ = false; }
    final_act(const final_act&) = delete;
    final_act& operator=(const final_act&) = delete;

    ~final_act() noexcept { if (invoke_) f_(); }

private:
    F f_;
    bool invoke_;
};

// finally() - convenience function to generate a final_act
template <class F>
final_act<F> finally(const F &f) noexcept { return final_act<F>(f); }

template <class F>
final_act<F> finally(F &&f) noexcept { return final_act<F>(std::forward<F>(f)); }

// narrow_cast(): a searchable way to do narrowing casts of values
template<class T, class U>
T narrow_cast(U u) noexcept { return static_cast<T>(u); }

struct narrowing_error : public std::exception {};
// narrow() : a checked version of narrow_cast() that throws if the cast changed the value
template<class T, class U>
T narrow(U u) { T t = narrow_cast<T>(u); if (static_cast<U>(t) != u) throw narrowing_error(); return t; }

//
// at() - Bounds-checked way of accessing static arrays, std::array, std::vector
//
template <class T, size_t N>
T& at(T(&arr)[N], size_t index) { fail_fast_assert(index < N); return arr[index]; }

template <class T, size_t N>
T& at(std::array<T, N>& arr, size_t index) { fail_fast_assert(index < N); return arr[index]; }

template <class Cont>
typename Cont::value_type& at(Cont& cont, size_t index) { fail_fast_assert(index < cont.size()); return cont[index]; }


//
// not_null
//
// Restricts a pointer or smart pointer to only hold non-null values.
// 
// Has zero size overhead over T.
//
// If T is a pointer (i.e. T == U*) then 
// - allow construction from U* or U& 
// - disallow construction from nullptr_t
// - disallow default construction
// - ensure construction from U* fails with nullptr
// - allow implicit conversion to U*
//
template<class T>
class not_null
{
    static_assert(std::is_assignable<T&, std::nullptr_t>::value, "T cannot be assigned nullptr.");
public:
    not_null(T t) : ptr_(t) { ensure_invariant(); }
    not_null& operator=(const T& t) { ptr_ = t; ensure_invariant(); return *this; }

    not_null(const not_null &other) = default;
    not_null& operator=(const not_null &other) = default;

    template <typename U, typename Dummy = std::enable_if_t<std::is_convertible<U, T>::value>>
    not_null(const not_null<U> &other)
    {
        *this = other;
    }

    template <typename U, typename Dummy = std::enable_if_t<std::is_convertible<U, T>::value>>
    not_null& operator=(const not_null<U> &other)
    {
        ptr_ = other.get();
        return *this;
    }

    // prevents compilation when someone attempts to assign a nullptr 
    not_null(std::nullptr_t) = delete;
    not_null(int) = delete;
    not_null<T>& operator=(std::nullptr_t) = delete;
	not_null<T>& operator=(int) = delete;
    
    T get() const {
#ifdef _MSC_VER
        __assume(ptr_ != nullptr);
#endif
        return ptr_;
    } // the assume() should help the optimizer

    operator T() const {  return get(); }
    T operator->() const { return get(); }

	bool operator==(const T& rhs) const { return ptr_ == rhs; }
	bool operator!=(const T& rhs) const { return !(*this == rhs); }
private:
    T ptr_;

    // we assume that the compiler can hoist/prove away most of the checks inlined from this function
    // if not, we could make them optional via conditional compilation
    void ensure_invariant() const { fail_fast_assert(ptr_ != nullptr); }

    // unwanted operators...pointers only point to single objects!
    // TODO ensure all arithmetic ops on this type are unavailable
    not_null<T>& operator++() = delete;
    not_null<T>& operator--() = delete;
    not_null<T> operator++(int) = delete;
    not_null<T> operator--(int) = delete;
    not_null<T>& operator+(size_t) = delete;
    not_null<T>& operator+=(size_t) = delete;
    not_null<T>& operator-(size_t) = delete;
    not_null<T>& operator-=(size_t) = delete;
};

} // namespace gsl

namespace std
{
    template<class T>
    struct hash<gsl::not_null<T>>
    {
        size_t operator()(const gsl::not_null<T> & value) const
        {
            return hash<T>{}(value);
        }
    };

} // namespace std

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

#endif // GSL_GSL_H
