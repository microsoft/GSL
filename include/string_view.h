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

#ifndef GSL_STRING_VIEW_H
#define GSL_STRING_VIEW_H

#include "array_view.h"
#include <cstring>

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
template<size_t Max = dynamic_range>
using czstring = const char*;

template<size_t Max = dynamic_range>
using cwzstring = const wchar_t*;

template<size_t Max = dynamic_range>
using zstring = char*;

template<size_t Max = dynamic_range>
using wzstring = wchar_t*;

//
// string_view and relatives
//
// Note that Extent is always single-dimension only
// Note that SizeType is defaulted to be smaller than size_t which is the array_view default
//
// TODO (neilmac) once array_view regains configurable size_type, update these typedef's
//
template <class CharT, size_t Extent = dynamic_range>
using basic_string_view = array_view<CharT, Extent>;

template<size_t Extent = dynamic_range>
using string_view = basic_string_view<char, Extent>;

template<size_t Extent = dynamic_range>
using cstring_view = basic_string_view<const char, Extent>;

template<size_t Extent = dynamic_range>
using wstring_view = basic_string_view<wchar_t, Extent>;

template<size_t Extent = dynamic_range>
using cwstring_view = basic_string_view<const wchar_t, Extent>;


//
// ensure_sentinel() 
//
// Provides a way to obtain an array_view from a contiguous sequence
// that ends with a (non-inclusive) sentinel value.
//
// Will fail-fast if sentinel cannot be found before max elements are examined.
//
template<class T, class SizeType, const T Sentinel>
array_view<T, dynamic_range> ensure_sentinel(const T* seq, SizeType max = std::numeric_limits<SizeType>::max())
{
    auto cur = seq;
    while ((cur - seq) < max && *cur != Sentinel) ++cur;
    fail_fast_assert(*cur == Sentinel);
    return{ seq, cur - seq };
}


//
// ensure_z - creates a string_view for a czstring or cwzstring.
// Will fail fast if a null-terminator cannot be found before
// the limit of size_type.
//
template<class T>
inline basic_string_view<T, dynamic_range> ensure_z(T* const & sz, size_t max = std::numeric_limits<size_t>::max())
{
    return ensure_sentinel<0>(sz, max);
}

// TODO (neilmac) there is probably a better template-magic way to get the const and non-const overloads to share an implementation
inline basic_string_view<char, dynamic_range> ensure_z(char* const & sz, size_t max)
{
    auto len = strnlen(sz, max);
    fail_fast_assert(sz[len] == 0); return{ sz, len };
}

inline basic_string_view<const char, dynamic_range> ensure_z(const char* const& sz, size_t max)
{
    auto len = strnlen(sz, max);
    fail_fast_assert(sz[len] == 0); return{ sz, len };
}

inline basic_string_view<wchar_t, dynamic_range> ensure_z(wchar_t* const & sz, size_t max)
{
    auto len = wcsnlen(sz, max);
    fail_fast_assert(sz[len] == 0); return{ sz, len };
}

inline basic_string_view<const wchar_t, dynamic_range> ensure_z(const wchar_t* const & sz, size_t max)
{
    auto len = wcsnlen(sz, max);
    fail_fast_assert(sz[len] == 0); return{ sz, len };
}

template<class T, size_t N>
basic_string_view<T, dynamic_range> ensure_z(T(&sz)[N]) { return ensure_z(&sz[0], N); }

template<class Cont>
basic_string_view<typename std::remove_pointer<typename Cont::pointer>::type, dynamic_range> ensure_z(Cont& cont)
{
    return ensure_z(cont.data(), cont.length());
}

//
// to_string() allow (explicit) conversions from string_view to string
//
template<class CharT, size_t Extent>
std::basic_string<typename std::remove_const<CharT>::type> to_string(basic_string_view<CharT, Extent> view)
{
    return{ view.data(), view.length() };
}


template<class CharT, size_t Extent = dynamic_range>
class basic_zstring_builder
{
public:
    using string_view_type = basic_string_view<CharT, Extent>;
    using value_type = CharT;
    using pointer = CharT*;
    using size_type = typename string_view_type::size_type;
    using iterator = typename string_view_type::iterator;

    basic_zstring_builder(CharT* data, size_type length) : sv_(data, length) {}

    template<size_t Size>
    basic_zstring_builder(CharT(&arr)[Size]) : sv_(arr) {}

    pointer data() const { return sv_.data(); }
    string_view_type view() const { return sv_; }

    size_type length() const { return sv_.length(); }

    pointer assume0() const { return data(); }
    string_view_type ensure_z() const { return gsl::ensure_z(sv_); }

    iterator begin() const { return sv_.begin(); }
    iterator end() const { return sv_.end(); }

private:
    string_view_type sv_;
};

template <size_t Max = dynamic_range>
using zstring_builder = basic_zstring_builder<char, Max>;

template <size_t Max = dynamic_range>
using wzstring_builder = basic_zstring_builder<wchar_t, Max>;
}

#endif // GSL_STRING_VIEW_H
