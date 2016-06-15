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

#ifndef GSL_SPAN_H
#define GSL_SPAN_H

#include "gsl_assert.h"
#include "gsl_util.h"
#include "byte.h"
#include <array>
#include <limits>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>

#ifdef _MSC_VER

// turn off some warnings that are noisy about our Expects statements
#pragma warning(push)
#pragma warning(disable : 4127) // conditional expression is constant

// No MSVC does constexpr fully yet
#pragma push_macro("constexpr")
#define constexpr

// VS 2013 workarounds
#if _MSC_VER <= 1800

#define GSL_MSVC_HAS_VARIADIC_CTOR_BUG
#define GSL_MSVC_NO_SUPPORT_FOR_MOVE_CTOR_DEFAULT

// noexcept is not understood 
#ifndef GSL_THROW_ON_CONTRACT_VIOLATION
#pragma push_macro("noexcept")
#define noexcept /* nothing */
#endif

// turn off some misguided warnings
#pragma warning(push)
#pragma warning(disable : 4351) // warns about newly introduced aggregate initializer behavior
#pragma warning(disable : 4512) // warns that assignment op could not be generated

#endif // _MSC_VER <= 1800

#endif // _MSC_VER

#ifdef GSL_THROW_ON_CONTRACT_VIOLATION

#ifdef _MSC_VER
#pragma push_macro("noexcept")
#endif

#define noexcept /* nothing */

#endif // GSL_THROW_ON_CONTRACT_VIOLATION

namespace gsl
{

template <class ElementType, std::ptrdiff_t Extent = dynamic_extent>
class span;


// [views.constants], constants  
constexpr const std::ptrdiff_t dynamic_extent = -1;


// implementation details
namespace details
{
template <class T>
struct is_span_oracle : std::false_type
{
};

template <class ElementType, std::ptrdiff_t Extent>
struct is_span_oracle<gsl::span<ElementType, Extent>> : std::true_type
{
};

template <class T>
struct is_span : is_span_oracle<std::remove_cv_t<T>>
{
};

template <class From, class To>
struct is_allowed_pointer_conversion
    : std::bool_constant<
    std::is_pointer<From>::value &&
    std::is_pointer<To>::value &&
    std::is_convertible<From, To>::value
    >
{
};

template <class From, class To>
struct is_allowed_integral_conversion
    : std::bool_constant<
    std::is_integral<From>::value &&
    std::is_integral<To>::value &&
    sizeof(From) == sizeof(To) &&
    alignof(From) == alignof(To) &&
    std::is_convertible<From, To>::value
    >
{
};

template <std::ptrdiff_t From, std::ptrdiff_t To>
struct is_allowed_extent_conversion
    : std::bool_constant<
    From == To ||
    From == gsl::dynamic_extent ||
    To == gsl::dynamic_extent
    >
{
};

template <class From, class To>
struct is_allowed_element_type_conversion
    : std::bool_constant<
    std::is_same<From, std::remove_cv_t<To>>::value ||
    is_allowed_pointer_conversion<From, To>::value ||
    is_allowed_integral_conversion<From, To>::value
    >
{
};

template <class From>
struct is_allowed_element_type_conversion<From, byte>
    : std::bool_constant<!std::is_const<From>::value>
{
};

template <class From>
struct is_allowed_element_type_conversion<From, const byte>
    : std::true_type
{
};

template <class Span>
class span_iterator
    : public std::iterator<std::random_access_iterator_tag, typename Span::element_type>
{
    using Base = std::iterator<std::random_access_iterator_tag, typename Span::element_type>;

public:
    using typename Base::reference;
    using typename Base::pointer;
    using typename Base::difference_type;

    span_iterator() : span_iterator(nullptr, 0) {}
    span_iterator(const Span* span, typename Span::index_type index) : span_(span), index_(index)
    {
        Expects(span == nullptr || (index_ >= 0 && index <= span_->length()));
    }

    reference operator*() const { Expects(span_); return (*span_)[index_]; }
    pointer operator->() const { Expects(span_); return &((*span_)[index_]); }

    span_iterator& operator++() noexcept
    {
        Expects(span_ && index_ >= 0 && index_ < span_->length());
        ++index_;
        return *this;
    }

    span_iterator operator++(int) noexcept
    {
        auto ret = *this;
        ++(*this);
        return ret;
    }

    span_iterator& operator--() noexcept
    {
        Expects(span_ && index > 0 && index_ <= span_->length());
        --index_;
        return *this;
    }

    span_iterator operator--(int) noexcept
    {
        auto ret = *this;
        --(*this);
        return ret;
    }

    span_iterator operator+(difference_type n) const noexcept
    {
        auto ret{*this};
        return ret += n;
    }

    span_iterator& operator+=(difference_type n) noexcept
    {
        index_ += n;
        Expects(span_ && index_ >= 0 && index_ <= span_->length());
        return *this;
    }

    span_iterator operator-(difference_type n) const noexcept
    {
        auto ret{*this};
        return ret -= n;
    }

    span_iterator& operator-=(difference_type n) noexcept
    {
        return *this += -n;
    }

    difference_type operator-(const span_iterator& rhs) const noexcept
    {
        Expects(span_ == rhs.span_);
        return index_ - rhs.index_;
    }

    reference operator[](difference_type n) const noexcept
    {
        return *(*this + n);
    }

    bool operator==(const span_iterator& rhs) const noexcept
    {
        return span_ == rhs.span_ && index_ == rhs.index_;
    }

    bool operator!=(const span_iterator& rhs) const noexcept { return !(*this == rhs); }

    bool operator<(const span_iterator& rhs) const noexcept
    {
        Expects(span_ == rhs.span_);
        return index_ < rhs.index_;
    }

    bool operator<=(const span_iterator& rhs) const noexcept { return !(rhs < *this); }

    bool operator>(const span_iterator& rhs) const noexcept { return rhs < *this; }

    bool operator>=(const span_iterator& rhs) const noexcept { return !(rhs > *this); }

    void swap(span_iterator& rhs) noexcept
    {
        std::swap(index_, rhs.index_);
        std::swap(m_span, rhs.m_span);
    }

private:
    const Span* span_;
    ptrdiff_t index_;
};

template <typename Span>
span_iterator<Span> operator+(typename span_iterator<Span>::difference_type n,
    const span_iterator<Span>& rhs) noexcept
{
    return rhs + n;
}

template <typename Span>
span_iterator<Span> operator-(typename span_iterator<Span>::difference_type n,
    const span_iterator<Span>& rhs) noexcept
{
    return rhs - n;
}

} // namespace details


// [span], class template span 
template <class ElementType, std::ptrdiff_t Extent>
class span {
public:
    // constants and types 
    using element_type = ElementType;
    using index_type = std::ptrdiff_t;
    using pointer = element_type*;
    using reference = element_type&;

    using iterator = details::span_iterator<span<ElementType, Extent>>;
    using reverse_iterator = std::reverse_iterator<iterator>;

    constexpr static const index_type extent = Extent;

    // [span.cons], span constructors, copy, assignment, and destructor 
    constexpr span() noexcept : storage_(nullptr, extent_type<0>())
    {}

    constexpr span(nullptr_t) noexcept : span()
    {}

    constexpr span(pointer ptr, index_type count) : storage_(ptr, count)
    { Expects((!ptr && count == 0) || (ptr && count >= 0)); }

    constexpr span(pointer firstElem, pointer lastElem)
        : storage_(firstElem, std::distance(firstElem, lastElem))
    {}
    
    template <size_t N>
    constexpr span(element_type(&arr)[N])
        : storage_(&arr[0], extent_type<N>())
    {}

    template <size_t N>
    constexpr span(std::array<element_type, N>& arr)
        : storage_(&arr[0], extent_type<N>())
    {}

    template <size_t N, class = std::enable_if_t<is_const<element_type>::value>>
    constexpr span(std::array<std::remove_const_t<element_type>, N>& arr)
        : storage_(&arr[0], extent_type<N>())
    {}

    template <size_t N, class = std::enable_if_t<is_const<element_type>::value>>
    constexpr span(const std::array<std::remove_const_t<element_type>, N>& arr)
        : storage_(&arr[0], extent_type<N>())
    {}

    // NB: the SFINAE here uses .data() as a incomplete/imperfect proxy for the requirement
    // on Container to be a contiguous sequence container.
    template <class Container,
        class = std::enable_if_t<!details::is_span<Container>::value &&
        std::is_convertible<Container::pointer, pointer>::value &&
        std::is_convertible<Container::pointer, decltype(std::declval<Container>().data())>::value>
    >
    constexpr span(Container& cont) : span(cont.data(), cont.size()) {}

    template <class Container,
    class = std::enable_if_t<std::is_const<element_type>::value &&
        !details::is_span<Container>::value &&
        std::is_convertible<Container::pointer, pointer>::value &&
        std::is_convertible<Container::pointer, decltype(std::declval<Container>().data())>::value>
    >
    constexpr span(const Container& cont) : span(cont.data(), cont.size()) {}

    constexpr span(const span& other) noexcept = default;
    constexpr span(span&& other) noexcept = default;

    template <class OtherElementType, std::ptrdiff_t OtherExtent,
        class = std::enable_if_t<
            details::is_allowed_extent_conversion<OtherExtent, Extent>::value && 
            details::is_allowed_element_type_conversion<OtherElementType, element_type>::value
        >
    >
    constexpr span(const span<OtherElementType, OtherExtent>& other)
        : storage_(reinterpret_cast<pointer>(other.data()), extent_type<OtherExtent>(other.size()))
    {}

    template <class OtherElementType, std::ptrdiff_t OtherExtent,
        class = std::enable_if_t<
            details::is_allowed_extent_conversion<OtherExtent, Extent>::value &&
            details::is_allowed_element_type_conversion<OtherElementType, element_type>::value
        >
    >
    constexpr span(span<OtherElementType, OtherExtent>&& other)
        : storage_(reinterpret_cast<pointer>(other.data()), extent_type<OtherExtent>(other.size()))
    {}

    ~span() noexcept = default;
    constexpr span& operator=(const span& other) noexcept = default;
    constexpr span& operator=(span&& other) noexcept = default;

    // [span.sub], span subviews  
    template <ptrdiff_t Count>
    constexpr span<element_type, Count> first() const
    {
        Expects(Count >= 0 && Count <= size());
        return { data(), Count };
    }

    template <ptrdiff_t Count>
    constexpr span<element_type, Count> last() const
    {
        Expects(Count >= 0 && Count <= size());
        return{ Count == 0 ? data() : data() + (size() - Count), Count };
    }

    template <ptrdiff_t Offset, ptrdiff_t Count = dynamic_extent>
    constexpr span<element_type, Count> subspan() const
    {
        Expects((Offset == 0 || Offset > 0 && Offset <= size()) &&
            (Count == dynamic_extent || Count >= 0 && Offset + Count <= size()));
        return { data() + Offset, Count == dynamic_extent ? size() - Offset : Count };
    }

    constexpr span<element_type, dynamic_extent> first(index_type count) const
    {
        Expects(count >= 0 && count <= size());
        return { data(), count };
    }

    constexpr span<element_type, dynamic_extent> last(index_type count) const
    {
        Expects(count >= 0 && count <= size());
        return { count == 0 ? data() : data() + (size() - count), count };
    }

    constexpr span<element_type, dynamic_extent> subspan(index_type offset,
        index_type count = dynamic_extent) const
    {
        Expects((offset == 0 || offset > 0 && offset <= size()) &&
            (count == dynamic_extent || count >= 0 && offset + count <= size()));
        return { data() + offset, count == dynamic_extent ? size() - offset : count };
    }

    // [span.obs], span observers 
    constexpr index_type length() const noexcept { return size(); }
    constexpr index_type size() const noexcept { return storage_.size();  }
    constexpr index_type length_bytes() const noexcept { return size_bytes(); }
    constexpr index_type size_bytes() const noexcept { return size() * sizeof(element_type); }
    constexpr bool empty() const noexcept { return size() == 0; }

    // [span.elem], span element access 
    constexpr reference operator[](index_type idx) const
    {
        Expects(idx >= 0 && idx < storage_.size());
        return storage_.data()[idx];
    }
    constexpr reference operator()(index_type idx) const { return this->operator[](idx); }
    constexpr pointer data() const noexcept { return storage_.data(); }

    // [span.iter], span iterator support 
    iterator begin() const noexcept { return {this, 0}; }
    iterator end() const noexcept { return {this, length()}; }
 
    reverse_iterator rbegin() const noexcept { return {this, length()}; }
    reverse_iterator rend() const noexcept { return {this, 0}; }

private:
    constexpr static const bool is_span_type = true;

    template <index_type Extent>
    class extent_type;
    
    template <index_type Extent>
    class extent_type
    {
    public:
        static_assert(Extent >= 0, "A fixed-size span must be >= 0 in size.");

        constexpr extent_type() noexcept {}

        template <index_type Other>
        constexpr extent_type(extent_type<Other> ext) noexcept
        {
            static_assert(Other == Extent || Other == dynamic_extent,
                "Mismatch between fixed-size extent and size of initializing data.");
            Expects(ext.size() == Extent);
        }

        constexpr extent_type(index_type size)
        { Expects(size == Extent); }

        constexpr inline index_type size() const noexcept { return Extent; }
    };

    template <>
    class extent_type<dynamic_extent>
    {
    public:
        template <index_type Other>
        explicit constexpr extent_type(extent_type<Other> ext) : size_(ext.size())
        {}

        explicit constexpr extent_type(index_type size) : size_(size)
        { Expects(size >= 0); } 

        constexpr inline index_type size() const noexcept
        { return size_; }

    private:
        index_type size_;
    };

    // this implementation detail class lets us take advantage of the 
    // empty base class optimization to pay for only storage of a single
    // pointer in the case of fixed-size spans
    template <class ExtentType>
    class storage_type : public ExtentType
    {
    public:
        template <class OtherExtentType>
        storage_type(pointer data, OtherExtentType ext)
            : ExtentType(ext), data_(data) {}

        constexpr inline pointer data() const noexcept
        { return data_; }

    private:
        pointer data_;
    };

    storage_type<extent_type<Extent>> storage_;
};


// [span.comparison], span comparison operators 
template <class ElementType, ptrdiff_t Extent>
constexpr bool operator==(const span<ElementType, Extent>& l, const span<ElementType, Extent>& r) noexcept
{ return std::equal(l.begin(), l.end(), r.begin(), r.end()); }

template <class ElementType, ptrdiff_t Extent>
constexpr bool operator!=(const span<ElementType, Extent>& l, const span<ElementType, Extent>& r) noexcept
{ return !(l == r); }

template <class ElementType, ptrdiff_t Extent>
constexpr bool operator<(const span<ElementType, Extent>& l, const span<ElementType, Extent>& r) noexcept
{ return std::lexicographical_compare(l.begin(), l.end(), r.begin(), r.end()); }

template <class ElementType, ptrdiff_t Extent>
constexpr bool operator<=(const span<ElementType, Extent>& l, const span<ElementType, Extent>& r) noexcept
{ return !(l > r); }

template <class ElementType, ptrdiff_t Extent>
constexpr bool operator>(const span<ElementType, Extent>& l, const span<ElementType, Extent>& r) noexcept
{ return r < l; }

template <class ElementType, ptrdiff_t Extent>
constexpr bool operator>=(const span<ElementType, Extent>& l, const span<ElementType, Extent>& r) noexcept
{ return !(l < r); }


namespace details
{
    // if we only supported compilers with good constexpr support then
    // this pair of classes could collapse down to a constexpr function

    // we should use a narrow_cast<> to go to size_t, but older compilers may not see it as constexpr
    // and so will fail compilation of the template
    template <class ElementType, ptrdiff_t Extent>
    struct calculate_byte_size :
        std::integral_constant<std::ptrdiff_t, static_cast<ptrdiff_t>(sizeof(ElementType) * static_cast<size_t>(Extent))>
    {};

    template <class ElementType>
    struct calculate_byte_size<ElementType, dynamic_extent> :
        std::integral_constant<std::ptrdiff_t, dynamic_extent>
    {};
}


// [span.objectrep], views of object representation 
template <class ElementType, ptrdiff_t Extent>
constexpr span<const byte, details::calculate_byte_size<ElementType, Extent>::value> as_bytes(span<ElementType, Extent> s) noexcept
{ return {reinterpret_cast<const byte*>(s.data()), s.size_bytes()}; }

template <class ElementType, ptrdiff_t Extent, class = std::enable_if_t<!std::is_const<ElementType>::value>>
constexpr span<byte, details::calculate_byte_size<ElementType, Extent>::value> as_writeable_bytes(span<ElementType, Extent> s) noexcept
{ return {reinterpret_cast<byte*>(s.data()), s.size_bytes()}; }

} // namespace gsl

#ifdef _MSC_VER

#undef constexpr
#pragma pop_macro("constexpr")

#if _MSC_VER <= 1800
#pragma warning(pop)

#ifndef GSL_THROW_ON_CONTRACT_VIOLATION
#undef noexcept
#pragma pop_macro("noexcept")
#endif // GSL_THROW_ON_CONTRACT_VIOLATION

#undef GSL_MSVC_HAS_VARIADIC_CTOR_BUG

#endif // _MSC_VER <= 1800

#endif // _MSC_VER

#if defined(GSL_THROW_ON_CONTRACT_VIOLATION)

#undef noexcept

#ifdef _MSC_VER
#pragma warning(pop)
#pragma pop_macro("noexcept")
#endif

#endif // GSL_THROW_ON_CONTRACT_VIOLATION

#endif // GSL_SPAN_H
