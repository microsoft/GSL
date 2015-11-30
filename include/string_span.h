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

#ifndef GSL_STRING_SPAN_H
#define GSL_STRING_SPAN_H

#include "gsl_assert.h"
#include "span.h"
#include <cstring>

#ifdef _MSC_VER

// No MSVC does constexpr fully yet
#pragma push_macro("constexpr")
#define constexpr /* nothing */

// VS 2013 workarounds
#if _MSC_VER <= 1800

#define GSL_MSVC_HAS_TYPE_DEDUCTION_BUG 

// noexcept is not understood 
#ifndef GSL_THROW_ON_CONTRACT_VIOLATION
#pragma push_macro("noexcept")
#define noexcept /* nothing */ 
#endif

#endif // _MSC_VER <= 1800
#endif // _MSC_VER

// In order to test the library, we need it to throw exceptions that we can catch
#ifdef GSL_THROW_ON_CONTRACT_VIOLATION

#ifdef _MSC_VER
#pragma push_macro("noexcept")
#endif

#define noexcept /* nothing */ 

#endif // GSL_THROW_ON_CONTRACT_VIOLATION 

namespace gsl
{
//
// czstring and wzstring
//
// These are "tag" typedef's for C-style strings (i.e. null-terminated character arrays) 
// that allow static analysis to help find bugs.
//
// There are no additional features/semantics that we can find a way to add inside the
// type system for these types that will not either incur significant runtime costs or
// (sometimes needlessly) break existing programs when introduced.
//
template<std::ptrdiff_t Extent = dynamic_range>
using czstring = const char*;

template<std::ptrdiff_t Extent = dynamic_range>
using cwzstring = const wchar_t*;

template<std::ptrdiff_t Extent = dynamic_range>
using zstring = char*;

template<std::ptrdiff_t Extent = dynamic_range>
using wzstring = wchar_t*;

//
// ensure_sentinel() 
//
// Provides a way to obtain an span from a contiguous sequence
// that ends with a (non-inclusive) sentinel value.
//
// Will fail-fast if sentinel cannot be found before max elements are examined.
//
template<class T, const T Sentinel>
span<T, dynamic_range> ensure_sentinel(T* seq, std::ptrdiff_t max = PTRDIFF_MAX)
{
    auto cur = seq;
    while ((cur - seq) < max && *cur != Sentinel) ++cur;
    Ensures(*cur == Sentinel);
    return{ seq, cur - seq };
}


//
// ensure_z - creates a string_span for a czstring or cwzstring.
// Will fail fast if a null-terminator cannot be found before
// the limit of size_type.
//
template<class T>
inline span<T, dynamic_range> ensure_z(T* const & sz, std::ptrdiff_t max = PTRDIFF_MAX)
{
    return ensure_sentinel<T, 0>(sz, max);
}

// TODO (neilmac) there is probably a better template-magic way to get the const and non-const overloads to share an implementation
inline span<char, dynamic_range> ensure_z(char* const& sz, std::ptrdiff_t max)
{
    auto len = strnlen(sz, max);
    Ensures(sz[len] == 0);
    return{ sz, static_cast<std::ptrdiff_t>(len) };
}

inline span<const char, dynamic_range> ensure_z(const char* const& sz, std::ptrdiff_t max)
{
    auto len = strnlen(sz, max);
    Ensures(sz[len] == 0); return{ sz, static_cast<std::ptrdiff_t>(len) };
}

inline span<wchar_t, dynamic_range> ensure_z(wchar_t* const& sz, std::ptrdiff_t max)
{
    auto len = wcsnlen(sz, max);
    Ensures(sz[len] == 0); return{ sz, static_cast<std::ptrdiff_t>(len) };
}

inline span<const wchar_t, dynamic_range> ensure_z(const wchar_t* const& sz, std::ptrdiff_t max)
{
    auto len = wcsnlen(sz, max);
    Ensures(sz[len] == 0); return{ sz, static_cast<std::ptrdiff_t>(len) };
}

template<class T, size_t N>
span<T, dynamic_range> ensure_z(T(&sz)[N]) { return ensure_z(&sz[0], static_cast<std::ptrdiff_t>(N)); }

template<class Cont>
span<typename std::remove_pointer<typename Cont::pointer>::type, dynamic_range> ensure_z(Cont& cont)
{
    return ensure_z(cont.data(), static_cast<std::ptrdiff_t>(cont.length()));
}


// TODO (neilmac) there is probably a better template-magic way to get the const and non-const overloads to share an implementation
inline span<char, dynamic_range> remove_z(char* const& sz, std::ptrdiff_t max)
{
    auto len = strnlen(sz, max);
    return{ sz, static_cast<std::ptrdiff_t>(len) };
}

inline span<const char, dynamic_range> remove_z(const char* const& sz, std::ptrdiff_t max)
{
    auto len = strnlen(sz, max);
    return{ sz, static_cast<std::ptrdiff_t>(len) };
}

inline span<wchar_t, dynamic_range> remove_z(wchar_t* const& sz, std::ptrdiff_t max)
{
    auto len = wcsnlen(sz, max);
    return{ sz, static_cast<std::ptrdiff_t>(len) };
}

inline span<const wchar_t, dynamic_range> remove_z(const wchar_t* const& sz, std::ptrdiff_t max)
{
    auto len = wcsnlen(sz, max);
    return{ sz, static_cast<std::ptrdiff_t>(len) };
}

template<class T, size_t N>
span<T, dynamic_range> remove_z(T(&sz)[N])
{
    return remove_z(&sz[0], static_cast<std::ptrdiff_t>(N));
}

template<class Cont>
span<typename std::remove_pointer<typename Cont::pointer>::type, dynamic_range> remove_z(Cont& cont)
{
    return remove_z(cont.data(), static_cast<std::ptrdiff_t>(cont.length()));
}

template<typename ValueType, std::ptrdiff_t>
class basic_string_span;

namespace details
{
    template <typename T>
    struct is_basic_string_span_oracle : std::false_type
    {};

    template <typename ValueType, std::ptrdiff_t Extent>
    struct is_basic_string_span_oracle<basic_string_span<ValueType, Extent>> : std::true_type
    {};

    template <typename T>
    struct is_basic_string_span : is_basic_string_span_oracle<std::remove_cv_t<T>>
    {};
}

//
// string_span and relatives
//
// Note that Extent is always single-dimension only
//
template <class CharT, std::ptrdiff_t Extent = dynamic_range>
class basic_string_span
{
    using value_type = CharT;
    using const_value_type = std::add_const_t<value_type>;
    using pointer = std::add_pointer_t<value_type>;
    using reference = std::add_lvalue_reference_t<value_type>;
    using const_reference = std::add_lvalue_reference_t<const_value_type>;
    using bounds_type = static_bounds<Extent>;
    using underlying_type = span<value_type, Extent>;

public:
    using size_type = ptrdiff_t;
    using iterator = typename underlying_type::iterator;
    using const_iterator = typename underlying_type::const_iterator;
    using reverse_iterator = typename underlying_type::reverse_iterator;
    using const_reverse_iterator = typename underlying_type::const_reverse_iterator;

    // empty
    constexpr basic_string_span() noexcept
        : real(nullptr)
    {}

    // copy
    constexpr basic_string_span(const basic_string_span& other) noexcept
        : real(other.real)
    {}

    // move
    constexpr basic_string_span(const basic_string_span&& other) noexcept
        : real(std::move(other.real))
    {}

    // from nullptr and length
    constexpr basic_string_span(nullptr_t ptr, size_type length) noexcept
        : real(ptr, length)
    {}

    // For pointers and static arrays - if 0-terminated, remove 0 from the view

    // from c string
    
    constexpr basic_string_span(pointer& ptr) noexcept
        : real(ensure_z(ptr))
    {}

    // from non-const pointer to const span
    template<typename Dummy = std::enable_if_t<std::is_const<value_type>::value>>
    constexpr basic_string_span(std::remove_const_t<value_type>*& ptr) noexcept
        : real(ensure_z(ptr))
    {}

    // from raw data and length - remove 0 if needed
    constexpr basic_string_span(pointer ptr, size_type length) noexcept
        : real(remove_z(ptr, length))
    {}

    // from static arrays and string literals
    template<size_t N>
    constexpr basic_string_span(value_type(&arr)[N]) noexcept
        : real(remove_z(arr))
    {}

    // Those allow 0s in the middle, so we keep them

    constexpr basic_string_span(std::string& s) noexcept
        : real(&(s.at(0)), static_cast<ptrdiff_t>(s.length()))
    {}

    // from containers. It must have .size() and .data() function signatures
    template <typename Cont, typename DataType = typename Cont::value_type,
        typename Dummy = std::enable_if_t<!details::is_span<Cont>::value
        && !details::is_basic_string_span<Cont>::value 
        && !(!std::is_const<value_type>::value && std::is_const<Cont>::value) // no converting const containers to non-const span
        && std::is_convertible<DataType*, value_type*>::value
        && std::is_same<std::decay_t<decltype(std::declval<Cont>().size(), *std::declval<Cont>().data())>, DataType>::value>
    >
    constexpr basic_string_span(Cont& cont)
        : real(cont.data(), cont.size())
    {}

    // from span
    template <typename OtherValueType, std::ptrdiff_t OtherExtent,
        typename OtherBounds = static_bounds<OtherExtent>,
        typename Dummy = std::enable_if_t<std::is_convertible<OtherValueType*, value_type*>::value && std::is_convertible<OtherBounds, bounds_type>::value>
    >
    constexpr basic_string_span(const span<OtherValueType, OtherExtent>& other) noexcept
        : real(other)
    {}

    // from string_span
    template <typename OtherValueType, std::ptrdiff_t OtherExtent,
        typename OtherBounds = static_bounds<OtherExtent>,
        typename Dummy = std::enable_if_t<std::is_convertible<OtherValueType*, value_type*>::value && std::is_convertible<OtherBounds, bounds_type>::value>
    >
    constexpr basic_string_span(const basic_string_span<OtherValueType, OtherExtent>& other) noexcept
        : real(other.data(), other.length())
    {}

    // section on linear space
    template<size_type Count>
    constexpr basic_string_span<value_type, Count> first() const noexcept
    {
        return{ real.first<Count>() };
    }

    constexpr basic_string_span<value_type, dynamic_range> first(size_type count) const noexcept
    {
        return{ real.first(count); }
    }

    template<size_type Count>
    constexpr basic_string_span<value_type, Count> last() const noexcept
    {
        return{ real.last<Count>() };
    }

    constexpr basic_string_span<value_type, dynamic_range> last(size_type count) const noexcept
    {
        return{ real.last(count); }
    }

    template<size_type Offset, size_type Count>
    constexpr basic_string_span<value_type, Count> sub() const noexcept
    {
        return{ real.sub<Count>() };
    }

    constexpr basic_string_span<value_type, dynamic_range> sub(size_type offset, size_type count = dynamic_range) const noexcept
    {
        return{ real.sub(offset, count) };
    }

    constexpr const_reference operator[](size_type idx) const noexcept
    {
        return real[idx];
    }

    constexpr reference operator[](size_type idx) noexcept
    {
        return real[idx];
    }

    constexpr pointer data() const noexcept
    {
        return real.data();
    }

    constexpr size_type length() const noexcept
    {
        return real.size();
    }

    constexpr size_type size() const noexcept
    {
        return real.size();
    }

    constexpr size_type used_length() const noexcept
    {
        return length();
    }

    constexpr size_type bytes() const noexcept
    {
        return real.bytes();
    }

    constexpr size_type used_bytes() const noexcept
    {
        return bytes();
    }

    constexpr explicit operator bool() const noexcept
    {
        return real;
    }

    constexpr iterator begin() const noexcept
    {
        return real.begin();
    }

    constexpr iterator end() const noexcept
    {
        return real.end();
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return real.cbegin();
    }

    constexpr const_iterator cend() const noexcept
    {
        real.cend();
    }

    constexpr reverse_iterator rbegin() const noexcept
    {
        return real.rbegin();
    }

    constexpr reverse_iterator rend() const noexcept
    {
        return real.rend();
    }

    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return real.crbegin();
    }

    constexpr const_reverse_iterator crend() const noexcept
    {
        return real.crend();
    }

private:
    span<CharT, Extent> real;
};

template<std::ptrdiff_t Extent = dynamic_range>
using string_span = basic_string_span<char, Extent>;

template<std::ptrdiff_t Extent = dynamic_range>
using cstring_span = basic_string_span<const char, Extent>;

template<std::ptrdiff_t Extent = dynamic_range>
using wstring_span = basic_string_span<wchar_t, Extent>;

template<std::ptrdiff_t Extent = dynamic_range>
using cwstring_span = basic_string_span<const wchar_t, Extent>;

//
// to_string() allow (explicit) conversions from string_span to string
//
#ifndef GSL_MSVC_HAS_TYPE_DEDUCTION_BUG 

template<class CharT, ptrdiff_t Extent>
std::basic_string<typename std::remove_const<CharT>::type> to_string(basic_string_span<CharT, Extent> view)
{
    return{ view.data(), static_cast<size_t>(view.length()) };
}

#else

inline std::string to_string(cstring_span<> view)
{
    return{ view.data(), view.length() };
}

inline std::string to_string(string_span<> view)
{
    return{ view.data(), view.length() };
}

inline std::wstring to_string(cwstring_span<> view)
{
    return{ view.data(), view.length() };
}

inline std::wstring to_string(wstring_span<> view)
{
    return{ view.data(), view.length() };
}

#endif 

template<class CharT, size_t Extent = dynamic_range>
class basic_zstring_builder
{
public:
    using string_span_type = basic_string_span<CharT, Extent>;
    using value_type = CharT;
    using pointer = CharT*;
    using size_type = typename string_span_type::size_type;
    using iterator = typename string_span_type::iterator;

    basic_zstring_builder(CharT* data, size_type length) : sv_(data, length) {}

    template<size_t Size>
    basic_zstring_builder(CharT(&arr)[Size]) : sv_(arr) {}

    pointer data() const { return sv_.data(); }
    string_span_type view() const { return sv_; }

    size_type length() const { return sv_.length(); }

    pointer assume0() const { return data(); }
    string_span_type ensure_z() const { return gsl::ensure_z(sv_); }

    iterator begin() const { return sv_.begin(); }
    iterator end() const { return sv_.end(); }

private:
    string_span_type sv_;
};

template <size_t Max = dynamic_range>
using zstring_builder = basic_zstring_builder<char, Max>;

template <size_t Max = dynamic_range>
using wzstring_builder = basic_zstring_builder<wchar_t, Max>;
}


constexpr bool operator==(const gsl::cstring_span<>& one, const gsl::cstring_span<>& other) noexcept
{
    return std::equal(one.begin(), one.end(), other.begin(), other.end());
}

constexpr bool operator==(const gsl::string_span<>& one, const gsl::string_span<>& other) noexcept
{
    return std::equal(one.begin(), one.end(), other.begin(), other.end());
}


// TODO: ca we make twmplate ops work?
//template <typename ValueType, std::ptrdiff_t Extent>
//constexpr bool operator==(const gsl::basic_string_span<ValueType, Extent>& one, const gsl::basic_string_span<ValueType, Extent>& other) noexcept
//{
//    return std::equal(one.begin(), one.end(), other.begin(), other.end());
//}
/*
template <typename ValueType, std::ptrdiff_t Extent, typename OtherValueType, std::ptrdiff_t OtherExtent, typename Dummy = std::enable_if_t<std::is_same<std::remove_cv_t<ValueType>, std::remove_cv_t<OtherValueType>>::value>>
constexpr bool operator==(const gsl::basic_string_span<ValueType, Extent>& one, const gsl::basic_string_span<OtherValueType, OtherExtent>& other) noexcept
{
    return std::equal(one.begin(), one.end(), other.begin(), other.end());
}

template <typename ValueType, std::ptrdiff_t Extent, typename OtherValueType, std::ptrdiff_t OtherExtent, typename Dummy = std::enable_if_t<std::is_same<std::remove_cv_t<ValueType>, std::remove_cv_t<OtherValueType>>::value>>
constexpr bool operator!=(const gsl::basic_string_span<ValueType, Extent>& one, const gsl::basic_string_span<OtherValueType, OtherExtent>& other) noexcept
{
    return !(one == other);
}

template <typename ValueType, std::ptrdiff_t Extent, typename OtherValueType, std::ptrdiff_t OtherExtent, typename Dummy = std::enable_if_t<std::is_same<std::remove_cv_t<ValueType>, std::remove_cv_t<OtherValueType>>::value>>
constexpr bool operator<(const gsl::basic_string_span<ValueType, Extent>& one, const gsl::basic_string_span<OtherValueType, OtherExtent>& other) noexcept
{
    return std::lexicographical_compare(one.begin(), one.end(), other.begin(), other.end());
}

template <typename ValueType, std::ptrdiff_t Extent, typename OtherValueType, std::ptrdiff_t OtherExtent, typename Dummy = std::enable_if_t<std::is_same<std::remove_cv_t<ValueType>, std::remove_cv_t<OtherValueType>>::value>>
constexpr bool operator<=(const gsl::basic_string_span<ValueType, Extent>& one, const gsl::basic_string_span<OtherValueType, OtherExtent>& other) noexcept
{
    return !(other < one);
}

template <typename ValueType, std::ptrdiff_t Extent, typename OtherValueType, std::ptrdiff_t OtherExtent, typename Dummy = std::enable_if_t<std::is_same<std::remove_cv_t<ValueType>, std::remove_cv_t<OtherValueType>>::value>>
constexpr bool operator>(const gsl::basic_string_span<ValueType, Extent>& one, const gsl::basic_string_span<OtherValueType, OtherExtent>& other) noexcept
{
    return other < one;
}

template <typename ValueType, std::ptrdiff_t Extent, typename OtherValueType, std::ptrdiff_t OtherExtent, typename Dummy = std::enable_if_t<std::is_same<std::remove_cv_t<ValueType>, std::remove_cv_t<OtherValueType>>::value>>
constexpr bool operator>=(const gsl::basic_string_span<ValueType, Extent>& one, const gsl::basic_string_span<OtherValueType, OtherExtent>& other) noexcept
{
    return !(one < other);
}
*/
// VS 2013 workarounds
#ifdef _MSC_VER

#undef constexpr
#pragma pop_macro("constexpr")

#if _MSC_VER <= 1800

#pragma warning(pop)

#ifndef GSL_THROW_ON_CONTRACT_VIOLATION
#undef noexcept
#pragma pop_macro("noexcept")
#endif // GSL_THROW_ON_CONTRACT_VIOLATION

#undef GSL_MSVC_HAS_TYPE_DEDUCTION_BUG

#endif // _MSC_VER <= 1800
#endif // _MSC_VER

#if defined(GSL_THROW_ON_CONTRACT_VIOLATION) 

#undef noexcept

#ifdef _MSC_VER
#pragma pop_macro("noexcept")
#endif

#endif // GSL_THROW_ON_CONTRACT_VIOLATION

#endif // GSL_STRING_SPAN_H
