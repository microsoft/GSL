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

// [views.constants], constants  
constexpr const std::ptrdiff_t dynamic_extent = -1;


// [span], class template span 
template <class ElementType, std::ptrdiff_t Extent = dynamic_extent>
class span {
public:
    // constants and types 
    using element_type = ElementType;
    using index_type = std::ptrdiff_t;
    using pointer = element_type*;
    using reference = element_type&;
#if 0  // TODO
    using iterator = /*implementation-defined */;
    using const_iterator = /* implementation-defined */;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
#endif 
    constexpr static const index_type extent = Extent;

    // [span.cons], span constructors, copy, assignment, and destructor 
    constexpr span() noexcept : data_(nullptr), size_(0)
    { static_assert(extent == dynamic_extent || extent == 0, "Cannot default initialize a fixed-length span."); }
    constexpr span(nullptr_t) noexcept : span() {}
    constexpr span(pointer ptr, index_type count) : data_(ptr), size_(count)
    { Expects((!ptr && count == 0) || (ptr && count >= 0)); }
#if 0 // TODO
    constexpr span(pointer firstElem, pointer lastElem);
    template <size_t N>
    constexpr span(element_type(&arr)[N]);
    template <size_t N>
    constexpr span(array<remove_const_t<element_type>, N>& arr);
    template <size_t N>
    constexpr span(const array<remove_const_t<element_type>, N>& arr);
    template <class Container>
    constexpr span(Container& cont);
    template <class Container>
    span(const Container&&) = delete;
    constexpr span(const span& other) noexcept = default;
    constexpr span(span&& other) noexcept = default;
    template <class OtherElementType, ptrdiff_t OtherExtent>
    constexpr span(const span<OtherElementType, OtherExtent>& other);
    template <class OtherElementType, ptrdiff_t OtherExtent>
    constexpr span(span<OtherElementType, OtherExtent>&& other);
    ~span() noexcept = default;
    constexpr span& operator=(const span& other) noexcept = default;
    constexpr span& operator=(span&& other) noexcept = default;

    // [span.sub], span subviews  
    template <ptrdiff_t Count>
    constexpr span<element_type, Count> first() const;
    template <ptrdiff_t Count>
    constexpr span<element_type, Count> last() const;
    template <ptrdiff_t Offset, ptrdiff_t Count = dynamic_extent>
    constexpr span<element_type, Count> subspan() const;
    constexpr span<element_type, dynamic_extent> first(index_type count) const;
    constexpr span<element_type, dynamic_extent> last(index_type count) const;
    constexpr span<element_type, dynamic_extent> subspan(index_type offset, index_type count = dynamic_extent) const;
#endif
    // [span.obs], span observers 
    constexpr index_type length() const noexcept { return size(); }
    constexpr index_type size() const noexcept { return size_;  }
    constexpr index_type length_bytes() const noexcept { return size_bytes(); }
    constexpr index_type size_bytes() const noexcept { return size() * sizeof(element_type); }
    constexpr bool empty() const noexcept { return size() == 0; }

#if 0 // TODO
    // [span.elem], span element access 
    constexpr reference operator[](index_type idx) const;
    constexpr reference operator()(index_type idx) const;
#endif 
    constexpr pointer data() const noexcept { return data_; }
#if 0 // TODO
    // [span.iter], span iterator support 
    iterator begin() const noexcept;
    iterator end() const noexcept;
 
    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;

    reverse_iterator rbegin() const noexcept;
    reverse_iterator rend() const noexcept;

    const_reverse_iterator crbegin() const noexcept;
    const_reverse_iterator crend() const noexcept;
#endif
private:
    pointer data_;
    index_type size_;
};


#if 0 // TODO
// [span.comparison], span comparison operators 
template <class ElementType, ptrdiff_t Extent>
constexpr bool operator==(const span<ElementType, Extent>& l, const span<ElementType, Extent>& r) const noexcept;

template <class ElementType, ptrdiff_t Extent>
constexpr bool operator!=(const span<ElementType, Extent>& l, const span<ElementType, Extent>& r) const noexcept;

template <class ElementType, ptrdiff_t Extent>
constexpr bool operator<(const span<ElementType, Extent>& l, const span<ElementType, Extent>& r) const noexcept;

template <class ElementType, ptrdiff_t Extent>
constexpr bool operator<=(const span<ElementType, Extent>& l, const span<ElementType, Extent>& r) const noexcept;

template <class ElementType, ptrdiff_t Extent>
constexpr bool operator>(const span<ElementType, Extent>& l, const span<ElementType, Extent>& r) const noexcept;

template <class ElementType, ptrdiff_t Extent>
constexpr bool operator>=(const span<ElementType, Extent>& l, const span<ElementType, Extent>& r) const noexcept;
#endif


#if 0 // TODO
// [span.objectrep], views of object representation 
template <class ElementType, ptrdiff_t Extent>
constexpr span<const char, ((Extent == dynamic_extent) ? dynamic_extent : (sizeof(ElementType) * Extent))> as_bytes(span<ElementType, Extent> s) noexcept;

template <class ElementType, ptrdiff_t Extent>
constexpr span<char, ((Extent == dynamic_extent) ? dynamic_extent : (sizeof(ElementType) * Extent))> as_writeable_bytes(span<ElementType, Extent>) noexcept;
#endif

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
