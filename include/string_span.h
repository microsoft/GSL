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

#include "span.h"
#include <cstring>

// VS 2013 workarounds
#ifdef _MSC_VER
#if _MSC_VER <= 1800

#pragma push_macro("GSL_MSVC_HAS_TYPE_DEDUCTION_BUG") 
#define GSL_MSVC_HAS_TYPE_DEDUCTION_BUG 

#endif // _MSC_VER <= 1800
#endif // _MSC_VER

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
// string_span and relatives
//
// Note that Extent is always single-dimension only
//
template <class CharT, std::ptrdiff_t Extent = dynamic_range>
using basic_string_span = span<CharT, Extent>;

template<std::ptrdiff_t Extent = dynamic_range>
using string_span = basic_string_span<char, Extent>;

template<std::ptrdiff_t Extent = dynamic_range>
using cstring_span = basic_string_span<const char, Extent>;

template<std::ptrdiff_t Extent = dynamic_range>
using wstring_span = basic_string_span<wchar_t, Extent>;

template<std::ptrdiff_t Extent = dynamic_range>
using cwstring_span = basic_string_span<const wchar_t, Extent>;


//
// ensure_sentinel() 
//
// Provides a way to obtain an span from a contiguous sequence
// that ends with a (non-inclusive) sentinel value.
//
// Will fail-fast if sentinel cannot be found before max elements are examined.
//
template<class T, const T Sentinel>
span<T, dynamic_range> ensure_sentinel(const T* seq, std::ptrdiff_t max = PTRDIFF_MAX)
{
    auto cur = seq;
    while ((cur - seq) < max && *cur != Sentinel) ++cur;
    fail_fast_assert(*cur == Sentinel);
    return{ seq, cur - seq };
}


//
// ensure_z - creates a string_span for a czstring or cwzstring.
// Will fail fast if a null-terminator cannot be found before
// the limit of size_type.
//
template<class T>
inline basic_string_span<T, dynamic_range> ensure_z(T* const & sz, std::ptrdiff_t max = PTRDIFF_MAX)
{
    return ensure_sentinel<T, 0>(sz, max);
}

// TODO (neilmac) there is probably a better template-magic way to get the const and non-const overloads to share an implementation
inline basic_string_span<char, dynamic_range> ensure_z(char* const& sz, std::ptrdiff_t max)
{
    auto len = strnlen(sz, max);
    fail_fast_assert(sz[len] == 0);
    return{ sz, static_cast<std::ptrdiff_t>(len) };
}

inline basic_string_span<const char, dynamic_range> ensure_z(const char* const& sz, std::ptrdiff_t max)
{
    auto len = strnlen(sz, max);
    fail_fast_assert(sz[len] == 0); return{ sz, static_cast<std::ptrdiff_t>(len) };
}

inline basic_string_span<wchar_t, dynamic_range> ensure_z(wchar_t* const& sz, std::ptrdiff_t max)
{
    auto len = wcsnlen(sz, max);
    fail_fast_assert(sz[len] == 0); return{ sz, static_cast<std::ptrdiff_t>(len) };
}

inline basic_string_span<const wchar_t, dynamic_range> ensure_z(const wchar_t* const& sz, std::ptrdiff_t max)
{
    auto len = wcsnlen(sz, max);
    fail_fast_assert(sz[len] == 0); return{ sz, static_cast<std::ptrdiff_t>(len) };
}

template<class T, size_t N>
basic_string_span<T, dynamic_range> ensure_z(T(&sz)[N]) { return ensure_z(&sz[0], static_cast<std::ptrdiff_t>(N)); }

template<class Cont>
basic_string_span<typename std::remove_pointer<typename Cont::pointer>::type, dynamic_range> ensure_z(Cont& cont)
{
    return ensure_z(cont.data(), cont.length());
}

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

// VS 2013 workarounds
#ifdef _MSC_VER
#if _MSC_VER <= 1800

#pragma pop_macro("GSL_MSVC_HAS_TYPE_DEDUCTION_BUG") 
#undef GSL_MSVC_HAS_TYPE_DEDUCTION_BUG 

#endif // _MSC_VER <= 1800
#endif // _MSC_VER


#endif // GSL_STRING_SPAN_H
