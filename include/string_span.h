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
#include "gsl_util.h"
#include "span.h"
#include <cstring>
#include <string>

#ifdef _MSC_VER

// No MSVC does constexpr fully yet
#pragma push_macro("constexpr")
#define constexpr /* nothing */

#pragma warning(push)

// blanket turn off warnings from CppCoreCheck for now
// so people aren't annoyed by them when running the tool.
// more targeted suppressions will be added in a future update to the GSL
#pragma warning(disable: 26481 26482 26483 26485 26490 26491 26492 26493 26495)


// VS 2013 workarounds
#if _MSC_VER <= 1800

#define GSL_MSVC_HAS_TYPE_DEDUCTION_BUG
#define GSL_MSVC_HAS_SFINAE_SUBSTITUTION_ICE
#define GSL_MSVC_NO_CPP14_STD_EQUAL
#define GSL_MSVC_NO_DEFAULT_MOVE_CTOR

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

template<typename CharT, std::ptrdiff_t Extent = dynamic_range>
using basic_zstring = CharT*;

template<std::ptrdiff_t Extent = dynamic_range>
using czstring = basic_zstring<const char, Extent>;

template<std::ptrdiff_t Extent = dynamic_range>
using cwzstring = basic_zstring<const wchar_t, Extent>;

template<std::ptrdiff_t Extent = dynamic_range>
using zstring = basic_zstring<char, Extent>;

template<std::ptrdiff_t Extent = dynamic_range>
using wzstring = basic_zstring<wchar_t, Extent>;

//
// ensure_sentinel()
//
// Provides a way to obtain an span from a contiguous sequence
// that ends with a (non-inclusive) sentinel value.
//
// Will fail-fast if sentinel cannot be found before max elements are examined.
//
template<typename T, const T Sentinel>
span<T, dynamic_range> ensure_sentinel(T* seq, std::ptrdiff_t max = PTRDIFF_MAX)
{
    auto cur = seq;
    while ((cur - seq) < max && *cur != Sentinel) ++cur;
    Ensures(*cur == Sentinel);
    return{ seq, cur - seq };
}


//
// ensure_z - creates a span for a czstring or cwzstring.
// Will fail fast if a null-terminator cannot be found before
// the limit of size_type.
//
template<typename T>
inline span<T, dynamic_range> ensure_z(T* const & sz, std::ptrdiff_t max = PTRDIFF_MAX)
{
    return ensure_sentinel<T, 0>(sz, max);
}

// TODO (neilmac) there is probably a better template-magic way to get the const and non-const overloads to share an implementation
inline span<char, dynamic_range> ensure_z(char* const& sz, std::ptrdiff_t max)
{
    auto len = strnlen(sz, narrow_cast<size_t>(max));
    Ensures(sz[len] == 0);
    return{ sz, static_cast<std::ptrdiff_t>(len) };
}

inline span<const char, dynamic_range> ensure_z(const char* const& sz, std::ptrdiff_t max)
{
    auto len = strnlen(sz, narrow_cast<size_t>(max));
    Ensures(sz[len] == 0);
    return{ sz, static_cast<std::ptrdiff_t>(len) };
}

inline span<wchar_t, dynamic_range> ensure_z(wchar_t* const& sz, std::ptrdiff_t max)
{
    auto len = wcsnlen(sz, narrow_cast<size_t>(max));
    Ensures(sz[len] == 0);
    return{ sz, static_cast<std::ptrdiff_t>(len) };
}

inline span<const wchar_t, dynamic_range> ensure_z(const wchar_t* const& sz, std::ptrdiff_t max)
{
    auto len = wcsnlen(sz, narrow_cast<size_t>(max));
    Ensures(sz[len] == 0);
    return{ sz, static_cast<std::ptrdiff_t>(len) };
}

template<typename T, size_t N>
span<T, dynamic_range> ensure_z(T(&sz)[N]) { return ensure_z(&sz[0], static_cast<std::ptrdiff_t>(N)); }

template<class Cont>
span<typename std::remove_pointer<typename Cont::pointer>::type, dynamic_range> ensure_z(Cont& cont)
{
    return ensure_z(cont.data(), static_cast<std::ptrdiff_t>(cont.length()));
}

template<typename CharT, std::ptrdiff_t>
class basic_string_span;

namespace details
{
    template <typename T>
    struct is_basic_string_span_oracle : std::false_type
    {};

    template <typename CharT, std::ptrdiff_t Extent>
    struct is_basic_string_span_oracle<basic_string_span<CharT, Extent>> : std::true_type
    {};

    template <typename T>
    struct is_basic_string_span : is_basic_string_span_oracle<std::remove_cv_t<T>>
    {};

    template <typename T>
    struct length_func
    {};

    template <>
    struct length_func<char>
    {
        std::ptrdiff_t operator()(char* const ptr, std::ptrdiff_t length) noexcept
        {
            return narrow_cast<std::ptrdiff_t>(strnlen(ptr, narrow_cast<size_t>(length)));
        }
    };

    template <>
    struct length_func<wchar_t>
    {
        std::ptrdiff_t operator()(wchar_t* const ptr, std::ptrdiff_t length) noexcept
        {
            return narrow_cast<std::ptrdiff_t>(wcsnlen(ptr, narrow_cast<size_t>(length)));
        }
    };

    template <>
    struct length_func<const char>
    {
        std::ptrdiff_t operator()(const char* const ptr, std::ptrdiff_t length) noexcept
        {
            return narrow_cast<std::ptrdiff_t>(strnlen(ptr, narrow_cast<size_t>(length)));
        }
    };

    template <>
    struct length_func<const wchar_t>
    {
        std::ptrdiff_t operator()(const wchar_t* const ptr, std::ptrdiff_t length) noexcept
        {
            return narrow_cast<std::ptrdiff_t>(wcsnlen(ptr, narrow_cast<size_t>(length)));
        }
    };
}


//
// string_span and relatives
//
// Note that Extent is always single-dimension only
//
template <typename CharT, std::ptrdiff_t Extent = dynamic_range>
class basic_string_span
{
public:
    using value_type = CharT;
    using const_value_type = std::add_const_t<value_type>;
    using pointer = std::add_pointer_t<value_type>;
    using reference = std::add_lvalue_reference_t<value_type>;
    using const_reference = std::add_lvalue_reference_t<const_value_type>;
    using bounds_type = static_bounds<Extent>;
    using impl_type = span<value_type, Extent>;

    using size_type = ptrdiff_t;
    using iterator = typename impl_type::iterator;
    using const_iterator = typename impl_type::const_iterator;
    using reverse_iterator = typename impl_type::reverse_iterator;
    using const_reverse_iterator = typename impl_type::const_reverse_iterator;

    // default (empty)
    constexpr basic_string_span() = default;

    // copy
    constexpr basic_string_span(const basic_string_span& other) = default;

    // move
#ifndef GSL_MSVC_NO_DEFAULT_MOVE_CTOR
    constexpr basic_string_span(basic_string_span&& other) = default;
#else
    constexpr basic_string_span(basic_string_span&& other)
    	: span_(std::move(other.span_))
    {}
#endif

    // assign
    constexpr basic_string_span& operator=(const basic_string_span& other) = default;

    // move assign
#ifndef GSL_MSVC_NO_DEFAULT_MOVE_CTOR
    constexpr basic_string_span& operator=(basic_string_span&& other) = default;
#else
    constexpr basic_string_span& operator=(basic_string_span&& other)
    {
        span_ = std::move(other.span_);
        return *this;
    }
#endif

    // from nullptr
    constexpr basic_string_span(std::nullptr_t ptr) noexcept
        : span_(ptr)
    {}

    // from nullptr and length
    constexpr basic_string_span(std::nullptr_t ptr, size_type length) noexcept
        : span_(ptr, length)
    {}

    // From static arrays - if 0-terminated, remove 0 from the view

    // from static arrays and string literals
    template<size_t N>
    constexpr basic_string_span(value_type(&arr)[N]) noexcept
        : span_(remove_z(arr))
    {}

    // Those allow 0s within the length, so we do not remove them

    // from raw data and length
    constexpr basic_string_span(pointer ptr, size_type length) noexcept
        : span_(ptr, length)
    {}

    // from string
    constexpr basic_string_span(std::string& s) noexcept
        : span_(const_cast<pointer>(s.data()), narrow_cast<std::ptrdiff_t>(s.length()))
    {}

    // from containers. Containers must have .size() and .data() function signatures
    template <typename Cont, typename DataType = typename Cont::value_type,
        typename Dummy = std::enable_if_t<!details::is_span<Cont>::value
        && !details::is_basic_string_span<Cont>::value
        && !(!std::is_const<value_type>::value && std::is_const<Cont>::value) // no converting const containers to non-const span
        && std::is_convertible<DataType*, value_type*>::value
        && std::is_same<std::decay_t<decltype(std::declval<Cont>().size(), *std::declval<Cont>().data())>, DataType>::value>
    >
    constexpr basic_string_span(Cont& cont)
        : span_(cont.data(), cont.size())
    {}

    // disallow creation from temporary containers and strings
    template <typename Cont, typename DataType = typename Cont::value_type,
        typename Dummy = std::enable_if_t<!details::is_span<Cont>::value
        && !details::is_basic_string_span<Cont>::value
        && std::is_convertible<DataType*, value_type*>::value
        && std::is_same<std::decay_t<decltype(std::declval<Cont>().size(), *std::declval<Cont>().data())>, DataType>::value>
    >
    basic_string_span(Cont&& cont) = delete;

#ifndef GSL_MSVC_HAS_SFINAE_SUBSTITUTION_ICE
    // from span
    template <typename OtherValueType, std::ptrdiff_t OtherExtent,
        typename Dummy = std::enable_if_t<
        std::is_convertible<OtherValueType*, value_type*>::value
        && std::is_convertible<static_bounds<OtherExtent>, bounds_type>::value>
    >
    constexpr basic_string_span(span<OtherValueType, OtherExtent> other) noexcept
        : span_(other)
    {}
#else
    // from span
    constexpr basic_string_span(span<value_type, Extent> other) noexcept
        : span_(other)
    {}
        
    template <typename Dummy = std::enable_if_t<!std::is_same<std::remove_const_t<value_type>, value_type>::value>>
    constexpr basic_string_span(span<std::remove_const_t<value_type>, Extent> other) noexcept
        : span_(other)
    {}
#endif

    // from string_span
    template <typename OtherValueType, std::ptrdiff_t OtherExtent,
        typename OtherBounds = static_bounds<OtherExtent>,
        typename Dummy = std::enable_if_t<std::is_convertible<OtherValueType*, value_type*>::value && std::is_convertible<OtherBounds, bounds_type>::value>
    >
    constexpr basic_string_span(basic_string_span<OtherValueType, OtherExtent> other) noexcept
        : span_(other.data(), other.length())
    {}

    constexpr bool empty() const noexcept
    {
        return length() == 0;
    }

    // first Count elements
    template<size_type Count>
    constexpr basic_string_span<value_type, Count> first() const noexcept
    {
        return{ span_.template first<Count>() };
    }

    constexpr basic_string_span<value_type, dynamic_range> first(size_type count) const noexcept
    {
        return{ span_.first(count) };
    }

    // last Count elements
    template<size_type Count>
    constexpr basic_string_span<value_type, Count> last() const noexcept
    {
        return{ span_.template last<Count>() };
    }

    constexpr basic_string_span<value_type, dynamic_range> last(size_type count) const noexcept
    {
        return{ span_.last(count) };
    }

    // create a subview of Count elements starting from Offset
    template<size_type Offset, size_type Count>
    constexpr basic_string_span<value_type, Count> subspan() const noexcept
    {
        return{ span_.template subspan<Offset, Count>() };
    }

    constexpr basic_string_span<value_type, dynamic_range> subspan(size_type offset, size_type count = dynamic_range) const noexcept
    {
        return{ span_.subspan(offset, count) };
    }

    constexpr reference operator[](size_type idx) const noexcept
    {
        return span_[idx];
    }

    constexpr pointer data() const noexcept
    {
        return span_.data();
    }

    // length of the span in elements
    constexpr size_type length() const noexcept
    {
        return span_.size();
    }

    // length of the span in elements
    constexpr size_type size() const noexcept
    {
        return span_.size();
    }

    // length of the span in bytes
    constexpr size_type size_bytes() const noexcept
    {
        return span_.size_bytes();
    }

    // length of the span in bytes
    constexpr size_type length_bytes() const noexcept
    {
        return span_.length_bytes();
    }

    constexpr iterator begin() const noexcept
    {
        return span_.begin();
    }

    constexpr iterator end() const noexcept
    {
        return span_.end();
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return span_.cbegin();
    }

    constexpr const_iterator cend() const noexcept
    {
        return span_.cend();
    }

    constexpr reverse_iterator rbegin() const noexcept
    {
        return span_.rbegin();
    }

    constexpr reverse_iterator rend() const noexcept
    {
        return span_.rend();
    }

    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return span_.crbegin();
    }

    constexpr const_reverse_iterator crend() const noexcept
    {
        return span_.crend();
    }

private:

    static impl_type remove_z(pointer const& sz, std::ptrdiff_t max) noexcept
    {
        return{ sz, details::length_func<value_type>()(sz, max)};
    }

    template<size_t N>
    static impl_type remove_z(value_type(&sz)[N]) noexcept
    {
        return remove_z(&sz[0], narrow_cast<std::ptrdiff_t>(N));
    }

    impl_type span_;
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

template<typename CharT, ptrdiff_t Extent>
std::basic_string<typename std::remove_const<CharT>::type> to_string(basic_string_span<CharT, Extent> view)
{
    return{ view.data(), static_cast<size_t>(view.length()) };
}

#else

inline std::string to_string(cstring_span<> view)
{
    return{ view.data(), static_cast<size_t>(view.length()) };
}

inline std::string to_string(string_span<> view)
{
    return{ view.data(), static_cast<size_t>(view.length()) };
}

inline std::wstring to_string(cwstring_span<> view)
{
    return{ view.data(), static_cast<size_t>(view.length()) };
}

inline std::wstring to_string(wstring_span<> view)
{
    return{ view.data(), static_cast<size_t>(view.length()) };
}

#endif

// zero-terminated string span, used to convert
// zero-terminated spans to legacy strings
template<typename CharT, std::ptrdiff_t Extent = dynamic_range>
class basic_zstring_span
{
public:
    using value_type = CharT;
    using const_value_type = std::add_const_t<CharT>;

    using pointer = std::add_pointer_t<value_type>;
    using const_pointer = std::add_pointer_t<const_value_type>;

    using zstring_type = basic_zstring<value_type, Extent>;
    using const_zstring_type = basic_zstring<const_value_type, Extent>;

    using impl_type = span<value_type, Extent>;
    using string_span_type = basic_string_span<value_type, Extent>;

    constexpr basic_zstring_span(impl_type span) noexcept
        : span_(span)
    {
        // expects a zero-terminated span
        Expects(span[span.size() - 1] == '\0');
    }

    // copy
    constexpr basic_zstring_span(const basic_zstring_span& other) = default;

    // move
#ifndef GSL_MSVC_NO_DEFAULT_MOVE_CTOR
    constexpr basic_zstring_span(basic_zstring_span&& other) = default;
#else
    constexpr basic_zstring_span(basic_zstring_span&& other)
        : span_(std::move(other.span_))
    {}
#endif

    // assign
    constexpr basic_zstring_span& operator=(const basic_zstring_span& other) = default;

    // move assign
#ifndef GSL_MSVC_NO_DEFAULT_MOVE_CTOR
    constexpr basic_zstring_span& operator=(basic_zstring_span&& other) = default;
#else
    constexpr basic_zstring_span& operator=(basic_zstring_span&& other)
    {
        span_ = std::move(other.span_);
        return *this;
    }
#endif

    constexpr bool empty() const noexcept { return span_.size() == 0; }

    constexpr string_span_type as_string_span() const noexcept { return span_.first(span_.size()-1); }

    constexpr string_span_type ensure_z() const noexcept { return gsl::ensure_z(span_); }

    constexpr const_zstring_type assume_z() const noexcept { return span_.data(); }

private:
    impl_type span_;
};

template <std::ptrdiff_t Max = dynamic_range>
using zstring_span = basic_zstring_span<char, Max>;

template <std::ptrdiff_t Max = dynamic_range>
using wzstring_span = basic_zstring_span<wchar_t, Max>;

template <std::ptrdiff_t Max = dynamic_range>
using czstring_span = basic_zstring_span<const char, Max>;

template <std::ptrdiff_t Max = dynamic_range>
using cwzstring_span = basic_zstring_span<const wchar_t, Max>;

} // namespace GSL

// operator ==
template <typename CharT, std::ptrdiff_t Extent = gsl::dynamic_range, typename T,
    typename = std::enable_if_t<
    std::is_convertible<T, gsl::basic_string_span<std::add_const_t<CharT>, Extent>>::value>
>
bool operator==(gsl::basic_string_span<CharT, Extent> one, const T& other) noexcept
{
    gsl::basic_string_span<std::add_const_t<CharT>, Extent> tmp(other);
#ifdef GSL_MSVC_NO_CPP14_STD_EQUAL
    return (one.size() == tmp.size()) && std::equal(one.begin(), one.end(), tmp.begin());
#else
    return std::equal(one.begin(), one.end(), tmp.begin(), tmp.end());
#endif
}

template <typename CharT, std::ptrdiff_t Extent = gsl::dynamic_range, typename T,
    typename Dummy = std::enable_if_t<
    std::is_convertible<T, gsl::basic_string_span<std::add_const_t<CharT>, Extent>>::value
    && !gsl::details::is_basic_string_span<T>::value>
>
bool operator==(const T& one, gsl::basic_string_span<CharT, Extent> other) noexcept
{
    gsl::basic_string_span<std::add_const_t<CharT>, Extent> tmp(one);
#ifdef GSL_MSVC_NO_CPP14_STD_EQUAL
    return (tmp.size() == other.size()) && std::equal(tmp.begin(), tmp.end(), other.begin());
#else
    return std::equal(tmp.begin(), tmp.end(), other.begin(), other.end());
#endif
}

#ifndef _MSC_VER 

// VS treats temp and const containers as convertible to basic_string_span,
// so the cases below are already covered by the previous operators

template <typename CharT, std::ptrdiff_t Extent = gsl::dynamic_range, typename T,
    typename DataType = typename T::value_type,
    typename Dummy = std::enable_if_t<
    !gsl::details::is_span<T>::value
    && !gsl::details::is_basic_string_span<T>::value
    && std::is_convertible<DataType*, CharT*>::value
    && std::is_same<std::decay_t<decltype(std::declval<T>().size(), *std::declval<T>().data())>, DataType>::value>
>
bool operator==(gsl::basic_string_span<CharT, Extent> one, const T& other) noexcept
{
    gsl::basic_string_span<std::add_const_t<CharT>, Extent> tmp(other);
    return std::equal(one.begin(), one.end(), tmp.begin(), tmp.end());
}

template <typename CharT, std::ptrdiff_t Extent = gsl::dynamic_range, typename T,
    typename DataType = typename T::value_type,
    typename Dummy = std::enable_if_t<
    !gsl::details::is_span<T>::value
    && !gsl::details::is_basic_string_span<T>::value
    && std::is_convertible<DataType*, CharT*>::value
    && std::is_same<std::decay_t<decltype(std::declval<T>().size(), *std::declval<T>().data())>, DataType>::value>
>
bool operator==(const T& one, gsl::basic_string_span<CharT, Extent> other) noexcept
{
    gsl::basic_string_span<std::add_const_t<CharT>, Extent> tmp(one);
    return std::equal(tmp.begin(), tmp.end(), other.begin(), other.end());
}
#endif

// operator !=
template <typename CharT, std::ptrdiff_t Extent = gsl::dynamic_range, typename T,
    typename = std::enable_if_t<
    std::is_convertible<T, gsl::basic_string_span<std::add_const_t<CharT>, Extent>>::value>
>
bool operator!=(gsl::basic_string_span<CharT, Extent> one, const T& other) noexcept
{
    return !(one == other);
}

template <typename CharT, std::ptrdiff_t Extent = gsl::dynamic_range, typename T,
    typename Dummy = std::enable_if_t<
    std::is_convertible<T, gsl::basic_string_span<std::add_const_t<CharT>, Extent>>::value
    && !gsl::details::is_basic_string_span<T>::value>
>
bool operator!=(const T& one, gsl::basic_string_span<CharT, Extent> other) noexcept
{
    return !(one == other);
}

#ifndef _MSC_VER 

// VS treats temp and const containers as convertible to basic_string_span,
// so the cases below are already covered by the previous operators

template <typename CharT, std::ptrdiff_t Extent = gsl::dynamic_range, typename T,
    typename DataType = typename T::value_type,
    typename Dummy = std::enable_if_t<
    !gsl::details::is_span<T>::value
    && !gsl::details::is_basic_string_span<T>::value
    && std::is_convertible<DataType*, CharT*>::value
    && std::is_same<std::decay_t<decltype(std::declval<T>().size(), *std::declval<T>().data())>, DataType>::value>
>
bool operator!=(gsl::basic_string_span<CharT, Extent> one, const T& other) noexcept
{
    return !(one == other);
}

template <typename CharT, std::ptrdiff_t Extent = gsl::dynamic_range, typename T,
    typename DataType = typename T::value_type,
    typename Dummy = std::enable_if_t<
    !gsl::details::is_span<T>::value
    && !gsl::details::is_basic_string_span<T>::value
    && std::is_convertible<DataType*, CharT*>::value
    && std::is_same<std::decay_t<decltype(std::declval<T>().size(), *std::declval<T>().data())>, DataType>::value>
>
bool operator!=(const T& one, gsl::basic_string_span<CharT, Extent> other) noexcept
{
    return !(one == other);
}
#endif

// operator<
template <typename CharT, std::ptrdiff_t Extent = gsl::dynamic_range, typename T,
    typename = std::enable_if_t<
    std::is_convertible<T, gsl::basic_string_span<std::add_const_t<CharT>, Extent>>::value>
>
bool operator<(gsl::basic_string_span<CharT, Extent> one, const T& other) noexcept
{
    gsl::basic_string_span<std::add_const_t<CharT>, Extent> tmp(other);
    return std::lexicographical_compare(one.begin(), one.end(), tmp.begin(), tmp.end());
}

template <typename CharT, std::ptrdiff_t Extent = gsl::dynamic_range, typename T,
    typename Dummy = std::enable_if_t<
    std::is_convertible<T, gsl::basic_string_span<std::add_const_t<CharT>, Extent>>::value
    && !gsl::details::is_basic_string_span<T>::value>
>
bool operator<(const T& one, gsl::basic_string_span<CharT, Extent> other) noexcept
{
    gsl::basic_string_span<std::add_const_t<CharT>, Extent> tmp(one);
    return std::lexicographical_compare(tmp.begin(), tmp.end(), other.begin(), other.end());
}

#ifndef _MSC_VER 

// VS treats temp and const containers as convertible to basic_string_span,
// so the cases below are already covered by the previous operators

template <typename CharT, std::ptrdiff_t Extent = gsl::dynamic_range, typename T,
    typename DataType = typename T::value_type,
    typename Dummy = std::enable_if_t<
    !gsl::details::is_span<T>::value
    && !gsl::details::is_basic_string_span<T>::value
    && std::is_convertible<DataType*, CharT*>::value
    && std::is_same<std::decay_t<decltype(std::declval<T>().size(), *std::declval<T>().data())>, DataType>::value>
>
bool operator<(gsl::basic_string_span<CharT, Extent> one, const T& other) noexcept
{
    gsl::basic_string_span<std::add_const_t<CharT>, Extent> tmp(other);
    return std::lexicographical_compare(one.begin(), one.end(), tmp.begin(), tmp.end());
}

template <typename CharT, std::ptrdiff_t Extent = gsl::dynamic_range, typename T,
    typename DataType = typename T::value_type,
    typename Dummy = std::enable_if_t<
    !gsl::details::is_span<T>::value
    && !gsl::details::is_basic_string_span<T>::value
    && std::is_convertible<DataType*, CharT*>::value
    && std::is_same<std::decay_t<decltype(std::declval<T>().size(), *std::declval<T>().data())>, DataType>::value>
>
bool operator<(const T& one, gsl::basic_string_span<CharT, Extent> other) noexcept
{
    gsl::basic_string_span<std::add_const_t<CharT>, Extent> tmp(one);
    return std::lexicographical_compare(tmp.begin(), tmp.end(), other.begin(), other.end());
}
#endif

// operator <=
template <typename CharT, std::ptrdiff_t Extent = gsl::dynamic_range, typename T,
    typename = std::enable_if_t<
    std::is_convertible<T, gsl::basic_string_span<std::add_const_t<CharT>, Extent>>::value>
>
bool operator<=(gsl::basic_string_span<CharT, Extent> one, const T& other) noexcept
{
    return !(other < one);
}

template <typename CharT, std::ptrdiff_t Extent = gsl::dynamic_range, typename T,
    typename Dummy = std::enable_if_t<
    std::is_convertible<T, gsl::basic_string_span<std::add_const_t<CharT>, Extent>>::value
    && !gsl::details::is_basic_string_span<T>::value>
>
bool operator<=(const T& one, gsl::basic_string_span<CharT, Extent> other) noexcept
{
    return !(other < one);
}

#ifndef _MSC_VER 

// VS treats temp and const containers as convertible to basic_string_span,
// so the cases below are already covered by the previous operators

template <typename CharT, std::ptrdiff_t Extent = gsl::dynamic_range, typename T,
    typename DataType = typename T::value_type,
    typename Dummy = std::enable_if_t<
    !gsl::details::is_span<T>::value
    && !gsl::details::is_basic_string_span<T>::value
    && std::is_convertible<DataType*, CharT*>::value
    && std::is_same<std::decay_t<decltype(std::declval<T>().size(), *std::declval<T>().data())>, DataType>::value>
>
bool operator<=(gsl::basic_string_span<CharT, Extent> one, const T& other) noexcept
{
    return !(other < one);
}

template <typename CharT, std::ptrdiff_t Extent = gsl::dynamic_range, typename T,
    typename DataType = typename T::value_type,
    typename Dummy = std::enable_if_t<
    !gsl::details::is_span<T>::value
    && !gsl::details::is_basic_string_span<T>::value
    && std::is_convertible<DataType*, CharT*>::value
    && std::is_same<std::decay_t<decltype(std::declval<T>().size(), *std::declval<T>().data())>, DataType>::value>
>
bool operator<=(const T& one, gsl::basic_string_span<CharT, Extent> other) noexcept
{
    return !(other < one);
}
#endif

// operator>
template <typename CharT, std::ptrdiff_t Extent = gsl::dynamic_range, typename T,
    typename = std::enable_if_t<
    std::is_convertible<T, gsl::basic_string_span<std::add_const_t<CharT>, Extent>>::value>
>
bool operator>(gsl::basic_string_span<CharT, Extent> one, const T& other) noexcept
{
    return other < one;
}

template <typename CharT, std::ptrdiff_t Extent = gsl::dynamic_range, typename T,
    typename Dummy = std::enable_if_t<
    std::is_convertible<T, gsl::basic_string_span<std::add_const_t<CharT>, Extent>>::value
    && !gsl::details::is_basic_string_span<T>::value>
>
bool operator>(const T& one, gsl::basic_string_span<CharT, Extent> other) noexcept
{
    return other < one;
}

#ifndef _MSC_VER 

// VS treats temp and const containers as convertible to basic_string_span,
// so the cases below are already covered by the previous operators

template <typename CharT, std::ptrdiff_t Extent = gsl::dynamic_range, typename T,
    typename DataType = typename T::value_type,
    typename Dummy = std::enable_if_t<
    !gsl::details::is_span<T>::value
    && !gsl::details::is_basic_string_span<T>::value
    && std::is_convertible<DataType*, CharT*>::value
    && std::is_same<std::decay_t<decltype(std::declval<T>().size(), *std::declval<T>().data())>, DataType>::value>
>
bool operator>(gsl::basic_string_span<CharT, Extent> one, const T& other) noexcept
{
    return other < one;
}

template <typename CharT, std::ptrdiff_t Extent = gsl::dynamic_range, typename T,
    typename DataType = typename T::value_type,
    typename Dummy = std::enable_if_t<
    !gsl::details::is_span<T>::value
    && !gsl::details::is_basic_string_span<T>::value
    && std::is_convertible<DataType*, CharT*>::value
    && std::is_same<std::decay_t<decltype(std::declval<T>().size(), *std::declval<T>().data())>, DataType>::value>
>
bool operator>(const T& one, gsl::basic_string_span<CharT, Extent> other) noexcept
{
    return other < one;
}
#endif

// operator >=
template <typename CharT, std::ptrdiff_t Extent = gsl::dynamic_range, typename T,
    typename = std::enable_if_t<
    std::is_convertible<T, gsl::basic_string_span<std::add_const_t<CharT>, Extent>>::value>
>
bool operator>=(gsl::basic_string_span<CharT, Extent> one, const T& other) noexcept
{
    return !(one < other);
}

template <typename CharT, std::ptrdiff_t Extent = gsl::dynamic_range, typename T,
    typename Dummy = std::enable_if_t<
    std::is_convertible<T, gsl::basic_string_span<std::add_const_t<CharT>, Extent>>::value
    && !gsl::details::is_basic_string_span<T>::value>
>
bool operator>=(const T& one, gsl::basic_string_span<CharT, Extent> other) noexcept
{
    return !(one < other);
}

#ifndef _MSC_VER 

// VS treats temp and const containers as convertible to basic_string_span,
// so the cases below are already covered by the previous operators

template <typename CharT, std::ptrdiff_t Extent = gsl::dynamic_range, typename T,
    typename DataType = typename T::value_type,
    typename Dummy = std::enable_if_t<
    !gsl::details::is_span<T>::value
    && !gsl::details::is_basic_string_span<T>::value
    && std::is_convertible<DataType*, CharT*>::value
    && std::is_same<std::decay_t<decltype(std::declval<T>().size(), *std::declval<T>().data())>, DataType>::value>
>
bool operator>=(gsl::basic_string_span<CharT, Extent> one, const T& other) noexcept
{
    return !(one < other);
}

template <typename CharT, std::ptrdiff_t Extent = gsl::dynamic_range, typename T,
    typename DataType = typename T::value_type,
    typename Dummy = std::enable_if_t<
    !gsl::details::is_span<T>::value
    && !gsl::details::is_basic_string_span<T>::value
    && std::is_convertible<DataType*, CharT*>::value
    && std::is_same<std::decay_t<decltype(std::declval<T>().size(), *std::declval<T>().data())>, DataType>::value>
>
bool operator>=(const T& one, gsl::basic_string_span<CharT, Extent> other) noexcept
{
    return !(one < other);
}
#endif

#ifdef _MSC_VER

#pragma warning(pop)

#undef constexpr
#pragma pop_macro("constexpr")

// VS 2013 workarounds
#if _MSC_VER <= 1800

#ifndef GSL_THROW_ON_CONTRACT_VIOLATION
#undef noexcept
#pragma pop_macro("noexcept")
#endif // GSL_THROW_ON_CONTRACT_VIOLATION

#undef GSL_MSVC_HAS_TYPE_DEDUCTION_BUG
#undef GSL_MSVC_HAS_SFINAE_SUBSTITUTION_ICE
#undef GSL_MSVC_NO_CPP14_STD_EQUAL
#undef GSL_MSVC_NO_DEFAULT_MOVE_CTOR

#endif // _MSC_VER <= 1800
#endif // _MSC_VER

#if defined(GSL_THROW_ON_CONTRACT_VIOLATION)

#undef noexcept

#ifdef _MSC_VER
#pragma pop_macro("noexcept")
#endif

#endif // GSL_THROW_ON_CONTRACT_VIOLATION
#endif // GSL_STRING_SPAN_H
