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

#include "array_view.h"     // array_view, strided_array_view...
#include "string_view.h"    // zstring, string_view, zstring_builder...
#include <memory>

namespace Guide
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
#define Expects(x)  Guide::fail_fast_assert((x))
#define Ensures(x)  Guide::fail_fast_assert((x))

//
// GSL.util: utilities
//

// Final_act allows you to ensure something gets run at the end of a scope
template <class F>
class Final_act
{
public:
    explicit Final_act(F f) : f_(f) {}
    
    Final_act(const Final_act&& other) : f_(other.f_) {}
    Final_act(const Final_act&) = delete;
    Final_act& operator=(const Final_act&) = delete;
    
    ~Final_act() { f_(); }

private:
    F f_;
};

// finally() - convenience function to generate a Final_act
template <class F>
Final_act<F> finally(F f) { return Final_act<F>(f); }

// narrow_cast(): a searchable way to do narrowing casts of values
template<class T, class U>
T narrow_cast(U u) { return static_cast<T>(u); }

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
public:
    not_null(T t) : ptr_(t) { ensure_invariant(); }

    // deleting these two prevents compilation when initialized with a nullptr or literal 0
    not_null(std::nullptr_t) = delete;
    not_null(int) = delete;

    not_null(const not_null &other) = default;

    template <typename U, typename Dummy = std::enable_if_t<std::is_convertible<U, T>::value>>
    not_null(const not_null<U> &other) : ptr_(other.get())
    {
    }

    not_null<T>& operator=(const T& t) { ptr_ = t; ensure_invariant(); return *this; }

    // prevents compilation when someone attempts to assign a nullptr 
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


// 
// maybe_null
//
// Describes an optional pointer - provides symmetry with not_null
//
template<class T>
class maybe_null_dbg
{
public:
    maybe_null_dbg() : ptr_(nullptr), tested_(false) {}

    maybe_null_dbg(const T& p) : ptr_(p), tested_(false) {}
    maybe_null_dbg(const maybe_null_dbg& rhs) : ptr_(rhs.ptr_), tested_(false) {}

    template <typename U, typename Dummy = std::enable_if_t<std::is_convertible<U, T>::value>>
    maybe_null_dbg(const not_null<U> &other) : ptr_(other.get()), tested_(false)
    {
    }

    template <typename U, typename Dummy = std::enable_if_t<std::is_convertible<U, T>::value>>
    maybe_null_dbg(const maybe_null_dbg<U> &other) : ptr_(other.get()), tested_(false)
    {
    }

    maybe_null_dbg& operator=(const T& p)
    {
        if (ptr_ != p)
        {
            ptr_ = p;
            tested_ = false;
        }
        return *this;
    }

    maybe_null_dbg& operator=(const maybe_null_dbg& rhs)
    {
        if (this != &rhs)
        {
            ptr_ = rhs.ptr_;
            tested_ = false;
        }
        return *this;
    }

    bool present() const { tested_ = true; return ptr_ != nullptr; }

    bool operator==(const T& rhs) const { tested_ = true; return ptr_ == rhs; }
    bool operator!=(const T& rhs) const { return !(*this == rhs); }

    T get() const {
        fail_fast_assert(tested_);
#ifdef _MSC_VER
        __assume(ptr_ != nullptr);
#endif
        return ptr_; 
    }

    operator T() const { return get(); }
    T operator->() const { return get(); }

private:
    const size_t ptee_size_ = sizeof(*ptr_);  // T must be a pointer type

    // unwanted operators...pointers only point to single objects!
    // TODO ensure all arithmetic ops on this type are unavailable
    maybe_null_dbg<T>& operator++() = delete;
    maybe_null_dbg<T>& operator--() = delete;
    maybe_null_dbg<T> operator++(int) = delete;
    maybe_null_dbg<T> operator--(int) = delete;
    maybe_null_dbg<T>& operator+(size_t) = delete;
    maybe_null_dbg<T>& operator+=(size_t) = delete;
    maybe_null_dbg<T>& operator-(size_t) = delete;
    maybe_null_dbg<T>& operator-=(size_t) = delete;

    T ptr_;
    mutable bool tested_;
};

template<class T>
class maybe_null_ret
{
public:
    maybe_null_ret() : ptr_(nullptr) {}
    maybe_null_ret(std::nullptr_t) : ptr_(nullptr) {}
    maybe_null_ret(const T& p) : ptr_(p) {}
    maybe_null_ret(const maybe_null_ret& rhs) = default;

    template <typename U, typename Dummy = std::enable_if_t<std::is_convertible<U, T>::value>>
    maybe_null_ret(const not_null<U> &other) : ptr_(other.get())
    {
    }

    template <typename U, typename Dummy = std::enable_if_t<std::is_convertible<U, T>::value>>
    maybe_null_ret(const maybe_null_ret<U> &other) : ptr_(other.get())
    {
    }

    template <typename U, typename Dummy = std::enable_if_t<std::is_convertible<U, T>::value>>
    maybe_null_ret(const maybe_null_dbg<U> &other) : ptr_(other.get())
    {
    }

    maybe_null_ret& operator=(const T& p) { if (ptr_ != p) { ptr_ = p; } return *this; }
    maybe_null_ret& operator=(const maybe_null_ret& rhs) = default;

    bool present() const { return ptr_ != nullptr; }

    T get() const { return ptr_; }

    operator T() const { return get(); }
    T operator->() const { return get(); }

private:
    // unwanted operators...pointers only point to single objects!
    // TODO ensure all arithmetic ops on this type are unavailable
    maybe_null_ret<T>& operator++() = delete;
    maybe_null_ret<T>& operator--() = delete;
    maybe_null_ret<T> operator++(int) = delete;
    maybe_null_ret<T> operator--(int) = delete;
    maybe_null_ret<T>& operator+(size_t) = delete;
    maybe_null_ret<T>& operator+=(size_t) = delete;
    maybe_null_ret<T>& operator-(size_t) = delete;
    maybe_null_ret<T>& operator-=(size_t) = delete;

    const size_t ptee_size_ = sizeof(*ptr_);  // T must be a pointer type
    T ptr_;
};

template<class T> using maybe_null = maybe_null_ret<T>;

} // namespace Guide
