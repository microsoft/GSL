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

#include <new>
#include <stdexcept>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <type_traits>
#include <utility>
#include <array>
#include <iterator>
#include <algorithm>
#include <functional>
#include "fail_fast.h"

#ifdef _MSC_VER

// No MSVC does constexpr fully yet
#pragma push_macro("constexpr")
#define constexpr /* nothing */


// VS 2013 workarounds
#if _MSC_VER <= 1800

#pragma push_macro("GSL_MSVC_HAS_VARIADIC_CTOR_BUG") 
#define GSL_MSVC_HAS_VARIADIC_CTOR_BUG 


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


namespace gsl {

/*
** begin definitions of index and bounds
*/
namespace details
{
    template <typename SizeType>
    struct SizeTypeTraits
    {
        static const SizeType max_value = std::numeric_limits<SizeType>::max();
    };


    template<typename... Ts>
    class are_integral : public std::integral_constant<bool, true> {};

    template<typename T, typename... Ts>
    class are_integral<T, Ts...> : public std::integral_constant<bool, std::is_integral<T>::value && are_integral<Ts...>::value> {};
}

template <size_t Rank>
class index final
{
    static_assert(Rank > 0, "Rank must be greater than 0!");

    template <size_t OtherRank>
    friend class index;

public:
    static const size_t rank = Rank;
    using value_type = std::ptrdiff_t;
    using size_type = value_type;
    using reference = std::add_lvalue_reference_t<value_type>;
    using const_reference = std::add_lvalue_reference_t<std::add_const_t<value_type>>;

    constexpr index() noexcept
    {}

    constexpr index(const value_type(&values)[Rank]) noexcept
    {
        std::copy(values, values + Rank, elems);
    }

#ifdef GSL_MSVC_HAS_VARIADIC_CTOR_BUG           
    template<typename T, typename... Ts, 
        typename = std::enable_if_t<((sizeof...(Ts) + 1) == Rank) && 
        std::is_integral<T>::value && 
        details::are_integral<Ts...>::value>> 
    constexpr index(T t, Ts... ds) : index({ static_cast<value_type>(t), static_cast<value_type>(ds)... }) 
    {} 
#else 
    template<typename... Ts, 
        typename = std::enable_if_t<(sizeof...(Ts) == Rank) && details::are_integral<Ts...>::value>> 
    constexpr index(Ts... ds) noexcept : elems{ static_cast<value_type>(ds)... } 
    {} 
#endif 

    constexpr index(const index& other) noexcept = default;

    constexpr index& operator=(const index& rhs) noexcept = default;

    // Preconditions: component_idx < rank
    constexpr reference operator[](size_t component_idx)
    {
        fail_fast_assert(component_idx < Rank, "Component index must be less than rank");
        return elems[component_idx];
    }

    // Preconditions: component_idx < rank
    constexpr const_reference operator[](size_t component_idx) const noexcept
    {
        fail_fast_assert(component_idx < Rank, "Component index must be less than rank");
        return elems[component_idx];
    }

    constexpr bool operator==(const index& rhs) const noexcept
    {
        return std::equal(elems, elems + rank, rhs.elems);
    }

    constexpr bool operator!=(const index& rhs) const noexcept
    {
        return !(this == rhs);
    }

    constexpr index operator+() const noexcept
    {
        return *this;
    }

    constexpr index operator-() const noexcept
    {
        index ret = *this;
        std::transform(ret, ret + rank, ret, std::negate<value_type>{});
        return ret;
    }

    constexpr index operator+(const index& rhs) const noexcept
    {
        index ret = *this;
        ret += rhs;
        return ret;
    }

    constexpr index operator-(const index& rhs) const noexcept
    {
        index ret = *this;
        ret -= rhs;
        return ret;
    }

    constexpr index& operator+=(const index& rhs) noexcept
    {
        std::transform(elems, elems + rank, rhs.elems, elems, std::plus<value_type>{});
        return *this;
    }

    constexpr index& operator-=(const index& rhs) noexcept
    {
        std::transform(elems, elems + rank, rhs.elems, elems, std::minus<value_type>{});
        return *this;
    }

    constexpr index operator*(value_type v) const noexcept
    {
        index ret = *this;
        ret *= v;
        return ret;
    }

    constexpr index operator/(value_type v) const noexcept
    {
        index ret = *this;
        ret /= v;
        return ret;
    }

    friend constexpr index operator*(value_type v, const index& rhs) noexcept
    {
        return rhs * v;
    }

    constexpr index& operator*=(value_type v) noexcept
    {
        std::transform(elems, elems + rank, elems, [v](value_type x) { return std::multiplies<value_type>{}(x, v); });
        return *this;
    }

    constexpr index& operator/=(value_type v) noexcept
    {
        std::transform(elems, elems + rank, elems, [v](value_type x) { return std::divides<value_type>{}(x, v); });
        return *this;
    }

private:
    value_type elems[Rank] = {};
};

#ifndef _MSC_VER

struct static_bounds_dynamic_range_t
{
    template <typename T, typename Dummy = std::enable_if_t<std::is_integral<T>::value>>
    constexpr operator T() const noexcept
    {
        return static_cast<T>(-1);
    }

    template <typename T, typename Dummy = std::enable_if_t<std::is_integral<T>::value>>
    constexpr bool operator ==(T other) const noexcept
    {
        return static_cast<T>(-1) == other;
    }

    template <typename T, typename Dummy = std::enable_if_t<std::is_integral<T>::value>>
    constexpr bool operator !=(T other) const noexcept
    {
        return static_cast<T>(-1) != other;
    }

};

template <typename T, typename Dummy = std::enable_if_t<std::is_integral<T>::value>>
constexpr bool operator ==(T left, static_bounds_dynamic_range_t right) noexcept
{
    return right == left;
}

template <typename T, typename Dummy = std::enable_if_t<std::is_integral<T>::value>>
constexpr bool operator !=(T left, static_bounds_dynamic_range_t right) noexcept
{
    return right != left;
}

constexpr static_bounds_dynamic_range_t dynamic_range{};
#else
const std::ptrdiff_t dynamic_range = -1;
#endif

struct generalized_mapping_tag {};
struct contiguous_mapping_tag : generalized_mapping_tag {};

namespace details
{

    template <std::ptrdiff_t Left, std::ptrdiff_t Right>
    struct LessThan
    {
        static const bool value = Left < Right;
    };

    template <std::ptrdiff_t... Ranges>
    struct BoundsRanges {
            using size_type = std::ptrdiff_t;
        static const size_type Depth = 0;
        static const size_type DynamicNum = 0;
        static const size_type CurrentRange = 1;
        static const size_type TotalSize = 1;

        // TODO : following signature is for work around VS bug
        template <typename OtherRange>
        BoundsRanges(const OtherRange&, bool /* firstLevel */)
        {}
        
        BoundsRanges (const BoundsRanges&) = default;
        BoundsRanges(const std::ptrdiff_t* const) { }
        BoundsRanges() = default;


        template <typename T, size_t Dim>
        void serialize(T&) const
        {}

        template <typename T, size_t Dim>
        size_type linearize(const T&) const
        { 
            return 0;
        }

        template <typename T, size_t Dim>
        bool contains(const T&) const
        {
            return 0;
        }

        size_type totalSize() const noexcept
        {
            return TotalSize;
        }

        bool operator==(const BoundsRanges&) const noexcept
        {
            return true;
        }
    };

    template <std::ptrdiff_t... RestRanges>
    struct BoundsRanges <dynamic_range, RestRanges...> : BoundsRanges<RestRanges...>{
        using Base = BoundsRanges <RestRanges... >;
            using size_type = std::ptrdiff_t;
        static const size_t Depth = Base::Depth + 1;
        static const size_t DynamicNum = Base::DynamicNum + 1;
        static const size_type CurrentRange = dynamic_range;
        static const size_type TotalSize = dynamic_range;
        const size_type m_bound;

        BoundsRanges (const BoundsRanges&) = default;
        
            BoundsRanges(const std::ptrdiff_t* const arr) : Base(arr + 1), m_bound(*arr * this->Base::totalSize())
        {
            fail_fast_assert(0 <= *arr);
        }

        BoundsRanges() : m_bound(0) {}

        template <std::ptrdiff_t OtherRange, std::ptrdiff_t... RestOtherRanges>
            BoundsRanges(const BoundsRanges<OtherRange, RestOtherRanges...>& other, bool /* firstLevel */ = true) :
                Base(static_cast<const BoundsRanges<RestOtherRanges...>&>(other), false), m_bound(other.totalSize())
            {}

        template <typename T, size_t Dim = 0>
        void serialize(T& arr) const
        {
            arr[Dim] = elementNum();
            this->Base::template serialize<T, Dim + 1>(arr);
        }

        template <typename T, size_t Dim = 0>
        size_type linearize(const T& arr) const
        { 
            const size_type index = this->Base::totalSize() * arr[Dim];
            fail_fast_assert(index < m_bound);
            return index + this->Base::template linearize<T, Dim + 1>(arr);
        }
        
        template <typename T, size_t Dim = 0>
        size_type contains(const T & arr) const
        {
            const ptrdiff_t last = this->Base::template contains<T, Dim + 1>(arr);
            if (last == -1)
                return -1;
            const ptrdiff_t cur = this->Base::totalSize() * arr[Dim];
            return cur < m_bound ? cur + last : -1;
        }
        
        size_type totalSize() const noexcept
        {
            return m_bound;
        }
        
        size_type elementNum() const noexcept
        {
            return totalSize() / this->Base::totalSize();
        }
        
        size_type elementNum(size_t dim) const noexcept
        {
            if (dim > 0)
                return this->Base::elementNum(dim - 1);
            else
                return elementNum();
        }

        bool operator == (const BoundsRanges & rhs) const noexcept
        {
            return m_bound == rhs.m_bound && static_cast<const Base&>(*this) == static_cast<const Base&>(rhs);
        }
    };

    template <std::ptrdiff_t CurRange, std::ptrdiff_t... RestRanges>
    struct BoundsRanges <CurRange, RestRanges...> : BoundsRanges<RestRanges...>
    {
        using Base = BoundsRanges <RestRanges... >;
            using size_type = std::ptrdiff_t;
        static const size_t Depth = Base::Depth + 1;
        static const size_t DynamicNum = Base::DynamicNum;
        static const size_type CurrentRange = CurRange;
        static const size_type TotalSize = Base::TotalSize == dynamic_range ? dynamic_range : CurrentRange * Base::TotalSize;

        BoundsRanges (const BoundsRanges&) = default;
        BoundsRanges(const std::ptrdiff_t* const arr) : Base(arr) { }
        BoundsRanges() = default;

        template <std::ptrdiff_t OtherRange, std::ptrdiff_t... RestOtherRanges>
        BoundsRanges(const BoundsRanges<OtherRange, RestOtherRanges...>&other, bool firstLevel = true) : Base(static_cast<const BoundsRanges<RestOtherRanges...>&>(other), false)
        {
            fail_fast_assert((firstLevel && totalSize() <= other.totalSize()) || totalSize() == other.totalSize());
        }

        template <typename T, size_t Dim = 0>
        void serialize(T& arr) const
        {
            arr[Dim] = elementNum();
            this->Base::template serialize<T, Dim + 1>(arr);
        }

        template <typename T, size_t Dim = 0>
        size_type linearize(const T& arr) const
        {  
            fail_fast_assert(arr[Dim] < CurrentRange, "Index is out of range");
            return this->Base::totalSize() * arr[Dim] + this->Base::template linearize<T, Dim + 1>(arr);
        }

        template <typename T, size_t Dim = 0>
        size_type contains(const T& arr) const
        {
            if (arr[Dim] >= CurrentRange)
                return -1;
            const size_type last = this->Base::template contains<T, Dim + 1>(arr);
            if (last == -1)
                return -1;
            return this->Base::totalSize() * arr[Dim] + last;
        }

        size_type totalSize() const noexcept
        {
            return CurrentRange * this->Base::totalSize();
        }

        size_type elementNum() const noexcept
        {
            return CurrentRange;
        }

        size_type elementNum(size_t dim) const noexcept
        {
            if (dim > 0)
                return this->Base::elementNum(dim - 1);
            else
                return elementNum();
        }

        bool operator== (const BoundsRanges& rhs) const noexcept
        {
            return static_cast<const Base &>(*this) == static_cast<const Base &>(rhs);
        }
    };

    template <typename SourceType, typename TargetType, size_t Rank>
    struct BoundsRangeConvertible2;

    template <size_t Rank, typename SourceType, typename TargetType, typename Ret = BoundsRangeConvertible2<typename SourceType::Base, typename TargetType::Base, Rank>>
    auto helpBoundsRangeConvertible(SourceType, TargetType, std::true_type) -> Ret;

    template <size_t Rank, typename SourceType, typename TargetType>
    auto helpBoundsRangeConvertible(SourceType, TargetType, ...) -> std::false_type;
    
    template <typename SourceType, typename TargetType, size_t Rank>
    struct BoundsRangeConvertible2 : decltype(helpBoundsRangeConvertible<Rank - 1>(SourceType(), TargetType(), 
        std::integral_constant<bool, SourceType::Depth == TargetType::Depth 
        && (SourceType::CurrentRange == TargetType::CurrentRange || TargetType::CurrentRange == dynamic_range || SourceType::CurrentRange == dynamic_range)>())) 
    {};

    template <typename SourceType, typename TargetType>
    struct BoundsRangeConvertible2<SourceType, TargetType, 0> : std::true_type {};

    template <typename SourceType, typename TargetType, std::ptrdiff_t Rank = TargetType::Depth>
    struct BoundsRangeConvertible : decltype(helpBoundsRangeConvertible<Rank - 1>(SourceType(), TargetType(), 
        std::integral_constant<bool, SourceType::Depth == TargetType::Depth 
        && (!LessThan<SourceType::CurrentRange, TargetType::CurrentRange>::value || TargetType::CurrentRange == dynamic_range || SourceType::CurrentRange == dynamic_range)>())) 
    {};
    template <typename SourceType, typename TargetType>
    struct BoundsRangeConvertible<SourceType, TargetType, 0> : std::true_type {};

    template <typename TypeChain>
    struct TypeListIndexer
    {
        const TypeChain & obj;
        TypeListIndexer(const TypeChain & obj) :obj(obj){}
        template<size_t N>
        const TypeChain & getObj(std::true_type)
        {
            return obj;
        }
        template<size_t N, typename MyChain = TypeChain, typename MyBase = typename MyChain::Base>
        auto getObj(std::false_type) -> decltype(TypeListIndexer<MyBase>(static_cast<const MyBase &>(obj)).template get<N>())
        {
            return TypeListIndexer<MyBase>(static_cast<const MyBase &>(obj)).template get<N>();
        }
        template <size_t N>
        auto get() -> decltype(getObj<N - 1>(std::integral_constant<bool, true>()))
        {
            return getObj<N - 1>(std::integral_constant<bool, N == 0>());
        }
    };

    template <typename TypeChain>
    TypeListIndexer<TypeChain> createTypeListIndexer(const TypeChain &obj)
    {
        return TypeListIndexer<TypeChain>(obj);
    }

    template <size_t Rank, bool Enabled = (Rank > 1), typename Ret = std::enable_if_t<Enabled, index<Rank - 1>>>
    constexpr Ret shift_left(const index<Rank>& other) noexcept
    {
        Ret ret{};
        for (size_t i = 0; i < Rank - 1; ++i)
        {
            ret[i] = other[i + 1];
        }
        return ret;
    }
}

template <typename IndexType>
class bounds_iterator;

template <std::ptrdiff_t... Ranges>
class static_bounds
{
public:
    static_bounds(const details::BoundsRanges<Ranges...>&) {
    }
};

template <std::ptrdiff_t FirstRange, std::ptrdiff_t... RestRanges>
class static_bounds<FirstRange, RestRanges...>
{
    using MyRanges = details::BoundsRanges<FirstRange, RestRanges... >;

    MyRanges m_ranges;
    constexpr static_bounds(const MyRanges& range) : m_ranges(range)
    {}
    
    template <std::ptrdiff_t... OtherRanges>
    friend class static_bounds;

public:
    static const size_t rank = MyRanges::Depth;
    static const size_t dynamic_rank = MyRanges::DynamicNum;
    static const std::ptrdiff_t static_size = MyRanges::TotalSize;

    using size_type = std::ptrdiff_t;
    using index_type = index<rank>;
    using const_index_type = std::add_const_t<index_type>;
    using iterator = bounds_iterator<const_index_type>;
    using const_iterator = bounds_iterator<const_index_type>;
    using difference_type = std::ptrdiff_t;
    using sliced_type = static_bounds<RestRanges...>;
    using mapping_type = contiguous_mapping_tag;

    constexpr static_bounds(const static_bounds&) = default;
    
    template <std::ptrdiff_t... Ranges, typename Dummy = std::enable_if_t<
        details::BoundsRangeConvertible<details::BoundsRanges<Ranges...>, details::BoundsRanges <FirstRange, RestRanges... >>::value>>
    constexpr static_bounds(const static_bounds<Ranges...>& other) : m_ranges(other.m_ranges)
    {}
                                                       
    constexpr static_bounds(std::initializer_list<size_type> il) : m_ranges((const std::ptrdiff_t*)il.begin())
    {
        fail_fast_assert((MyRanges::DynamicNum == 0 && il.size() == 1 && *il.begin() == static_size) || MyRanges::DynamicNum == il.size(), "Size of the initializer list must match the rank of the array");
        fail_fast_assert(m_ranges.totalSize() <= PTRDIFF_MAX, "Size of the range is larger than the max element of the size type");
    }
    
    constexpr static_bounds() = default;

    constexpr static_bounds& operator=(const static_bounds& otherBounds)
    {
        new(&m_ranges) MyRanges (otherBounds.m_ranges);
        return *this;
    }

    constexpr sliced_type slice() const noexcept
    {
        return sliced_type{static_cast<const details::BoundsRanges<RestRanges...> &>(m_ranges)};
    }

    constexpr size_type stride() const noexcept
    {
        return rank > 1 ? slice().size() : 1;
    }
    
    constexpr size_type size() const noexcept
    {
        return m_ranges.totalSize();
    }

    constexpr size_type total_size() const noexcept
    {
        return m_ranges.totalSize();
    }
    
    constexpr size_type linearize(const index_type & idx) const
    {
        return m_ranges.linearize(idx);
    }
    
    constexpr bool contains(const index_type& idx) const noexcept
    {
        return m_ranges.contains(idx) != -1;
    }
    
    constexpr size_type operator[](size_t index) const noexcept
    {
        return m_ranges.elementNum(index);
    }
    
    template <size_t Dim = 0>
    constexpr size_type extent() const noexcept
    {
        static_assert(Dim < rank, "dimension should be less than rank (dimension count starts from 0)");
        return details::createTypeListIndexer(m_ranges).template get<Dim>().elementNum();
    }
    
    constexpr index_type index_bounds() const noexcept
    {
        size_type extents[rank] = {};
        m_ranges.serialize(extents);
        return{ extents };
    }
    
    template <std::ptrdiff_t... Ranges>
    constexpr bool operator == (const static_bounds<Ranges...>& rhs) const noexcept
    {
        return this->size() == rhs.size();
    }
    
    template <std::ptrdiff_t... Ranges>
    constexpr bool operator != (const static_bounds<Ranges...>& rhs) const noexcept
    {
        return !(*this == rhs);
    }
    
    constexpr const_iterator begin() const noexcept
    {
        return const_iterator(*this, index_type{});
    }
    
    constexpr const_iterator end() const noexcept
    {
        return const_iterator(*this, this->index_bounds());
    }
};

template <size_t Rank>
class strided_bounds 
{
    template <size_t OtherRank>
    friend class strided_bounds;

public:
    static const size_t rank = Rank;
    using value_type      = std::ptrdiff_t;
    using reference       = std::add_lvalue_reference_t<value_type>;
    using const_reference = std::add_const_t<reference>;
    using size_type       = value_type;
    using difference_type = value_type;
    using index_type      = index<rank>;
    using const_index_type = std::add_const_t<index_type>;
    using iterator = bounds_iterator<const_index_type>;
    using const_iterator = bounds_iterator<const_index_type>;
    static const value_type dynamic_rank = rank;
    static const value_type static_size = dynamic_range;
    using sliced_type = std::conditional_t<rank != 0, strided_bounds<rank - 1>, void>;
    using mapping_type = generalized_mapping_tag;

    constexpr strided_bounds(const strided_bounds &) noexcept = default;

    constexpr strided_bounds(const value_type(&values)[rank], index_type strides)
        : m_extents(values), m_strides(std::move(strides))
    {}

    constexpr strided_bounds(const index_type &extents, const index_type &strides) noexcept
        : m_extents(extents), m_strides(strides)
    {}

    constexpr index_type strides() const noexcept
    {
        return m_strides;
    }

    constexpr size_type total_size() const noexcept
    {
        size_type ret = 0;
        for (size_t i = 0; i < rank; ++i)
        {
            ret += (m_extents[i] - 1) * m_strides[i];
        }
        return ret + 1;
    }
    
    constexpr size_type size() const noexcept
    {
        size_type ret = 1;
        for (size_t i = 0; i < rank; ++i)
        {
            ret *= m_extents[i];
        }
        return ret;
    }
    
    constexpr bool contains(const index_type& idx) const noexcept
    {
        for (size_t i = 0; i < rank; ++i)
        {
            if (idx[i] < 0 || idx[i] >= m_extents[i])
                return false;
        }
        return true;
    }

    constexpr size_type linearize(const index_type& idx) const noexcept
    {
        size_type ret = 0;
        for (size_t i = 0; i < rank; i++)
        {
            fail_fast_assert(idx[i] < m_extents[i], "index is out of bounds of the array");
            ret += idx[i] * m_strides[i];
        }
        return ret;
    }
    
    constexpr size_type stride() const noexcept
    {
        return m_strides[0];
    }
    
    template <bool Enabled = (rank > 1), typename Ret = std::enable_if_t<Enabled, sliced_type>>
    constexpr sliced_type slice() const
    {
        return{ details::shift_left(m_extents), details::shift_left(m_strides) };
    }
    
    template <size_t Dim = 0>
    constexpr size_type extent() const noexcept
    {
        static_assert(Dim < Rank, "dimension should be less than rank (dimension count starts from 0)");
        return m_extents[Dim];
    }
    
    constexpr index_type index_bounds() const noexcept
    {
        return m_extents;
    }
    constexpr const_iterator begin() const noexcept
    {
        return const_iterator{ *this, index_type{} };
    }
    
    constexpr const_iterator end() const noexcept
    {
        return const_iterator{ *this, index_bounds() };
    }

private:
    index_type m_extents;
    index_type m_strides;
};

template <typename T>
struct is_bounds : std::integral_constant<bool, false> {};
template <std::ptrdiff_t... Ranges>
struct is_bounds<static_bounds<Ranges...>> : std::integral_constant<bool, true> {};
template <size_t Rank>
struct is_bounds<strided_bounds<Rank>> : std::integral_constant<bool, true> {};

template <typename IndexType>
class bounds_iterator: public std::iterator<std::random_access_iterator_tag, IndexType>
{
private:
    using Base = std::iterator <std::random_access_iterator_tag, IndexType>;

public:
    static const size_t rank = IndexType::rank;
    using typename Base::reference;
    using typename Base::pointer;
    using typename Base::difference_type;
    using typename Base::value_type;
    using index_type = value_type;
    using index_size_type = typename IndexType::value_type;
    template <typename Bounds>
    explicit bounds_iterator(const Bounds& bnd, value_type curr) noexcept
        : boundary(bnd.index_bounds()), curr(std::move(curr))
    {
        static_assert(is_bounds<Bounds>::value, "Bounds type must be provided");
    }

    constexpr reference operator*() const noexcept
    {
        return curr;
    }

    constexpr pointer operator->() const noexcept
    {
        return &curr;
    }

    constexpr bounds_iterator& operator++() noexcept
    {
        for (size_t i = rank; i-- > 0;)
        {
            if (curr[i] < boundary[i] - 1)
            {
                curr[i]++;
                return *this;
            }
            curr[i] = 0;
        }
        // If we're here we've wrapped over - set to past-the-end.
        curr = boundary;
        return *this;
    }

    constexpr bounds_iterator operator++(int) noexcept
    {
        auto ret = *this;
        ++(*this);
        return ret;
    }

    constexpr bounds_iterator& operator--() noexcept
    {
        if (!less(curr, boundary))
        {
            // if at the past-the-end, set to last element
            for (size_t i = 0; i < rank; ++i)
            {
                curr[i] = boundary[i] - 1;
            }
            return *this;
        }
        for (size_t i = rank; i-- > 0;)
        {
            if (curr[i] >= 1)
            {
                curr[i]--;
                return *this;
            }
            curr[i] = boundary[i] - 1;
        }
        // If we're here the preconditions were violated
        // "pre: there exists s such that r == ++s"
        fail_fast_assert(false);
        return *this;
    }

    constexpr bounds_iterator operator--(int) noexcept
    {
        auto ret = *this;
        --(*this);
        return ret;
    }

    constexpr bounds_iterator operator+(difference_type n) const noexcept
    {
        bounds_iterator ret{ *this };
        return ret += n;
    }

    constexpr bounds_iterator& operator+=(difference_type n) noexcept
    {
        auto linear_idx = linearize(curr) + n;
        std::remove_const_t<value_type> stride = 0;
        stride[rank - 1] = 1;
        for (size_t i = rank - 1; i-- > 0;)
        {
            stride[i] = stride[i + 1] * boundary[i + 1];
        }
        for (size_t i = 0; i < rank; ++i)
        {
            curr[i] = linear_idx / stride[i];
            linear_idx = linear_idx % stride[i];
        }
        fail_fast_assert(!less(curr, index_type{}) && !less(boundary, curr), "index is out of bounds of the array");
        return *this;
    }

    constexpr bounds_iterator operator-(difference_type n) const noexcept
    {
        bounds_iterator ret{ *this };
        return ret -= n;
    }

    constexpr bounds_iterator& operator-=(difference_type n) noexcept
    {
        return *this += -n;
    }

    constexpr difference_type operator-(const bounds_iterator& rhs) const noexcept
    {
        return linearize(curr) - linearize(rhs.curr);
    }

    constexpr value_type operator[](difference_type n) const noexcept
    {
        return *(*this + n);
    }

    constexpr bool operator==(const bounds_iterator& rhs) const noexcept
    {
        return curr == rhs.curr;
    }

    constexpr bool operator!=(const bounds_iterator& rhs) const noexcept
    {
        return !(*this == rhs);
    }

    constexpr bool operator<(const bounds_iterator& rhs) const noexcept
    {
        return less(curr, rhs.curr);
    }

    constexpr bool operator<=(const bounds_iterator& rhs) const noexcept
    {
        return !(rhs < *this);
    }

    constexpr bool operator>(const bounds_iterator& rhs) const noexcept
    {
        return rhs < *this;
    }

    constexpr bool operator>=(const bounds_iterator& rhs) const noexcept
    {
        return !(rhs > *this);
    }

    void swap(bounds_iterator& rhs) noexcept
    {
        std::swap(boundary, rhs.boundary);
        std::swap(curr, rhs.curr);
    }
private:
    constexpr bool less(index_type& one, index_type& other) const noexcept
    {
        for (size_t i = 0; i < rank; ++i)
        {
            if (one[i] < other[i])
                return true;
        }
        return false;
    }

    constexpr index_size_type linearize(const value_type& idx) const noexcept
    {
        // TODO: Smarter impl.
        // Check if past-the-end
        index_size_type multiplier = 1;
        index_size_type res = 0;
        if (!less(idx, boundary))
        {
            res = 1;
            for (size_t i = rank; i-- > 0;)
            {
                res += (idx[i] - 1) * multiplier;
                multiplier *= boundary[i];
            }
        }
        else
        {
            for (size_t i = rank; i-- > 0;)
            {
                res += idx[i] * multiplier;
                multiplier *= boundary[i];
            }
        }
        return res;
    }

    value_type boundary;
    std::remove_const_t<value_type> curr;
};

template <typename IndexType>
bounds_iterator<IndexType> operator+(typename bounds_iterator<IndexType>::difference_type n, const bounds_iterator<IndexType>& rhs) noexcept
{
    return rhs + n;
}

//
// begin definitions of basic_span
//
namespace details
{
    template <typename Bounds>
    constexpr std::enable_if_t<std::is_same<typename Bounds::mapping_type, generalized_mapping_tag>::value, typename Bounds::index_type> make_stride(const Bounds& bnd) noexcept
    {
        return bnd.strides();
    }

    // Make a stride vector from bounds, assuming contiguous memory.
    template <typename Bounds>
    constexpr std::enable_if_t<std::is_same<typename Bounds::mapping_type, contiguous_mapping_tag>::value, typename Bounds::index_type> make_stride(const Bounds& bnd) noexcept
    {
        auto extents = bnd.index_bounds();
        typename Bounds::size_type stride[Bounds::rank] = {};

        stride[Bounds::rank - 1] = 1;
        for (size_t i = 1; i < Bounds::rank; ++i)
        {
            stride[Bounds::rank - i - 1] = stride[Bounds::rank - i] * extents[Bounds::rank - i];
        }
        return{ stride };
    }

    template <typename BoundsSrc, typename BoundsDest>
    void verifyBoundsReshape(const BoundsSrc &src, const BoundsDest &dest)
    {
        static_assert(is_bounds<BoundsSrc>::value && is_bounds<BoundsDest>::value, "The src type and dest type must be bounds");
        static_assert(std::is_same<typename BoundsSrc::mapping_type, contiguous_mapping_tag>::value, "The source type must be a contiguous bounds");
        static_assert(BoundsDest::static_size == dynamic_range || BoundsSrc::static_size == dynamic_range || BoundsDest::static_size == BoundsSrc::static_size, "The source bounds must have same size as dest bounds");
        fail_fast_assert(src.size() == dest.size());
    }


} // namespace details

template <typename ArrayView>
class contiguous_span_iterator;
template <typename ArrayView>
class general_span_iterator;
enum class byte : std::uint8_t {};

template <typename ValueType, typename BoundsType>
class basic_span
{
public:
    static const size_t rank = BoundsType::rank;
    using bounds_type = BoundsType;
    using size_type = typename bounds_type::size_type;
    using index_type = typename bounds_type::index_type;
    using value_type = ValueType;
    using const_value_type = std::add_const_t<value_type>;
    using pointer = ValueType*;
    using reference = ValueType&;
    using iterator = std::conditional_t<std::is_same<typename BoundsType::mapping_type, contiguous_mapping_tag>::value, contiguous_span_iterator<basic_span>, general_span_iterator<basic_span>>;
    using const_iterator = std::conditional_t<std::is_same<typename BoundsType::mapping_type, contiguous_mapping_tag>::value, contiguous_span_iterator<basic_span<const_value_type, BoundsType>>, general_span_iterator<basic_span<const_value_type, BoundsType>>>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using sliced_type = std::conditional_t<rank == 1, value_type, basic_span<value_type, typename BoundsType::sliced_type>>;

private:
    pointer m_pdata;
    bounds_type m_bounds;

public:
    constexpr bounds_type bounds() const noexcept
    {
        return m_bounds;
    }
    template <size_t Dim = 0>
    constexpr size_type extent() const noexcept
    {
        static_assert(Dim < rank, "dimension should be less than rank (dimension count starts from 0)");
        return m_bounds.template extent<Dim>();
    }
    constexpr size_type size() const noexcept
    {
        return m_bounds.size();
    }
    constexpr reference operator[](const index_type& idx) const
    {
        return m_pdata[m_bounds.linearize(idx)];
    }
    constexpr pointer data() const noexcept
    {
        return m_pdata;
    }
    template <bool Enabled = (rank > 1), typename Ret = std::enable_if_t<Enabled, sliced_type>>
    constexpr Ret operator[](size_type idx) const
    {
        fail_fast_assert(idx < m_bounds.size(), "index is out of bounds of the array");
        const size_type ridx = idx * m_bounds.stride();

        fail_fast_assert(ridx < m_bounds.total_size(), "index is out of bounds of the underlying data");
        return Ret {m_pdata + ridx, m_bounds.slice()};
    }

    constexpr operator bool () const noexcept
    {
        return m_pdata != nullptr;
    }

    constexpr iterator begin() const
    {
        return iterator {this, true};
    }
    constexpr iterator end() const
    {
        return iterator {this, false};
    }
    constexpr const_iterator cbegin() const
    {
        return const_iterator {reinterpret_cast<const basic_span<const value_type, bounds_type> *>(this), true};
    }
    constexpr const_iterator cend() const
    {
        return const_iterator {reinterpret_cast<const basic_span<const value_type, bounds_type> *>(this), false};
    }

    constexpr reverse_iterator rbegin() const
    {
        return reverse_iterator {end()};
    }
    constexpr reverse_iterator rend() const
    {
        return reverse_iterator {begin()};
    }
    constexpr const_reverse_iterator crbegin() const
    {
        return const_reverse_iterator {cend()};
    }
    constexpr const_reverse_iterator crend() const
    {
        return const_reverse_iterator {cbegin()};
    }

    template <typename OtherValueType, typename OtherBoundsType, typename Dummy = std::enable_if_t<std::is_same<std::remove_cv_t<value_type>, std::remove_cv_t<OtherValueType>>::value>>
    constexpr bool operator== (const basic_span<OtherValueType, OtherBoundsType> & other) const noexcept
    {
        return m_bounds.size() == other.m_bounds.size() &&
            (m_pdata == other.m_pdata || std::equal(this->begin(), this->end(), other.begin()));
    }

    template <typename OtherValueType, typename OtherBoundsType, typename Dummy = std::enable_if_t<std::is_same<std::remove_cv_t<value_type>, std::remove_cv_t<OtherValueType>>::value>>
    constexpr bool operator!= (const basic_span<OtherValueType, OtherBoundsType> & other) const noexcept
    {
        return !(*this == other);
    }

    template <typename OtherValueType, typename OtherBoundsType, typename Dummy = std::enable_if_t<std::is_same<std::remove_cv_t<value_type>, std::remove_cv_t<OtherValueType>>::value>>
    constexpr bool operator< (const basic_span<OtherValueType, OtherBoundsType> & other) const noexcept
    {
        return std::lexicographical_compare(this->begin(), this->end(), other.begin(), other.end());
    }

    template <typename OtherValueType, typename OtherBoundsType, typename Dummy = std::enable_if_t<std::is_same<std::remove_cv_t<value_type>, std::remove_cv_t<OtherValueType>>::value>>
    constexpr bool operator<= (const basic_span<OtherValueType, OtherBoundsType> & other) const noexcept
    {
        return !(other < *this);
    }

    template <typename OtherValueType, typename OtherBoundsType, typename Dummy = std::enable_if_t<std::is_same<std::remove_cv_t<value_type>, std::remove_cv_t<OtherValueType>>::value>>
    constexpr bool operator> (const basic_span<OtherValueType, OtherBoundsType> & other) const noexcept
    {
        return (other < *this);
    }

    template <typename OtherValueType, typename OtherBoundsType, typename Dummy = std::enable_if_t<std::is_same<std::remove_cv_t<value_type>, std::remove_cv_t<OtherValueType>>::value>>
    constexpr bool operator>= (const basic_span<OtherValueType, OtherBoundsType> & other) const noexcept
    {
        return !(*this < other);
    }

public:
    template <typename OtherValueType, typename OtherBounds,
        typename Dummy = std::enable_if_t<std::is_convertible<OtherValueType(*)[], value_type(*)[]>::value
            && std::is_convertible<OtherBounds, bounds_type>::value>>
    constexpr basic_span(const basic_span<OtherValueType, OtherBounds> & other ) noexcept
        : m_pdata(other.m_pdata), m_bounds(other.m_bounds)
    {
    }
protected:

    constexpr basic_span(pointer data, bounds_type bound) noexcept
        : m_pdata(data)
        , m_bounds(std::move(bound))
    {
        fail_fast_assert((m_bounds.size() > 0 && data != nullptr) || m_bounds.size() == 0);
    }
    template <typename T>
    constexpr basic_span(T *data, std::enable_if_t<std::is_same<value_type, std::remove_all_extents_t<T>>::value, bounds_type> bound) noexcept
        : m_pdata(reinterpret_cast<pointer>(data))
        , m_bounds(std::move(bound))
    {
        fail_fast_assert((m_bounds.size() > 0 && data != nullptr) || m_bounds.size() == 0);
    }
    template <typename DestBounds>
    constexpr basic_span<value_type, DestBounds> as_span(const DestBounds &bounds)
    {
        details::verifyBoundsReshape(m_bounds, bounds);
        return {m_pdata, bounds};
    }
private:

    friend iterator;
    friend const_iterator;
    template <typename ValueType2, typename BoundsType2>
    friend class basic_span;
};

template <std::ptrdiff_t DimSize = dynamic_range>
struct dim
{
    static const std::ptrdiff_t value = DimSize;
};
template <>
struct dim<dynamic_range>
{
    static const std::ptrdiff_t value = dynamic_range;
    const std::ptrdiff_t dvalue;
    dim(std::ptrdiff_t size) : dvalue(size) {}
};

template <typename ValueType, std::ptrdiff_t FirstDimension = dynamic_range, std::ptrdiff_t... RestDimensions>
class span;

template <typename ValueType, size_t Rank>
class strided_span;

namespace details
{
    template <typename T, typename = std::true_type>
    struct ArrayViewTypeTraits
    {
        using value_type = T;
        using size_type = size_t;
    };

    template <typename Traits>
    struct ArrayViewTypeTraits<Traits, typename std::is_reference<typename Traits::span_traits &>::type>
    {
        using value_type = typename Traits::span_traits::value_type;
        using size_type = typename Traits::span_traits::size_type;
    };

    template <typename T, std::ptrdiff_t... Ranks>
    struct ArrayViewArrayTraits {
        using type = span<T, Ranks...>;
        using value_type = T;
        using bounds_type = static_bounds<Ranks...>;
        using pointer = T*;
        using reference = T&;
    };
    template <typename T, std::ptrdiff_t N, std::ptrdiff_t... Ranks>
    struct ArrayViewArrayTraits<T[N], Ranks...> : ArrayViewArrayTraits<T, Ranks..., N> {};

    template <typename BoundsType>
    BoundsType newBoundsHelperImpl(std::ptrdiff_t totalSize, std::true_type) // dynamic size
    {
        fail_fast_assert(totalSize <= PTRDIFF_MAX);
        return BoundsType{totalSize};
    }
    template <typename BoundsType>
    BoundsType newBoundsHelperImpl(std::ptrdiff_t totalSize, std::false_type) // static size
    {
        fail_fast_assert(BoundsType::static_size == totalSize);
        return {};
    }
    template <typename BoundsType>
    BoundsType newBoundsHelper(std::ptrdiff_t totalSize)
    {
        static_assert(BoundsType::dynamic_rank <= 1, "dynamic rank must less or equal to 1");
        return newBoundsHelperImpl<BoundsType>(totalSize, std::integral_constant<bool, BoundsType::dynamic_rank == 1>());
    }
    
    struct Sep{};
    
    template <typename T, typename... Args>
    T static_as_span_helper(Sep, Args... args)
    {
        return T{static_cast<typename T::size_type>(args)...};
    }
    template <typename T, typename Arg, typename... Args>
    std::enable_if_t<!std::is_same<Arg, dim<dynamic_range>>::value && !std::is_same<Arg, Sep>::value, T> static_as_span_helper(Arg, Args... args)
    {
        return static_as_span_helper<T>(args...);
    }
    template <typename T, typename... Args>
    T static_as_span_helper(dim<dynamic_range> val, Args ... args)
    {
        return static_as_span_helper<T>(args..., val.dvalue);
    }

    template <typename ...Dimensions>
    struct static_as_span_static_bounds_helper
    {
        using type = static_bounds<(Dimensions::value)...>;
    };

    template <typename T>
    struct is_span_oracle : std::false_type
    {};

    template <typename ValueType, std::ptrdiff_t FirstDimension, std::ptrdiff_t... RestDimensions>
    struct is_span_oracle<span<ValueType, FirstDimension, RestDimensions...>> : std::true_type
    {};
    
    template <typename ValueType, std::ptrdiff_t Rank>
    struct is_span_oracle<strided_span<ValueType, Rank>> : std::true_type
    {};
    
    template <typename T>
    struct is_span : is_span_oracle<std::remove_cv_t<T>>
    {};

}


template <typename ValueType, std::ptrdiff_t FirstDimension, std::ptrdiff_t... RestDimensions>
class span : public basic_span <ValueType, static_bounds <FirstDimension, RestDimensions...>>
{
    template <typename ValueType2, std::ptrdiff_t FirstDimension2,
              std::ptrdiff_t... RestDimensions2>
    friend class span;

    using Base = basic_span<ValueType, static_bounds<FirstDimension, RestDimensions...>>;

public:
    using typename Base::bounds_type;
    using typename Base::size_type;
    using typename Base::pointer;
    using typename Base::value_type;
    using typename Base::index_type;
    using typename Base::iterator;
    using typename Base::const_iterator;
    using typename Base::reference;
    using Base::rank;

public:
    // basic
    constexpr span(pointer ptr, size_type size) : Base(ptr, bounds_type{ size })
    {}

    constexpr span(pointer ptr, bounds_type bounds) : Base(ptr, std::move(bounds))
    {}

    constexpr span(std::nullptr_t) : Base(nullptr, bounds_type{})
    {}

    constexpr span(std::nullptr_t, size_type size) : Base(nullptr, bounds_type{})
    {
        fail_fast_assert(size == 0);
    }

    // default
    template <std::ptrdiff_t DynamicRank = bounds_type::dynamic_rank, typename = std::enable_if_t<DynamicRank != 0>>
    constexpr span() : Base(nullptr, bounds_type())
    {}

    // from n-dimensions dynamic array (e.g. new int[m][4]) (precedence will be lower than the 1-dimension pointer)
    template <typename T, typename Helper = details::ArrayViewArrayTraits<T, dynamic_range>
        /*typename Dummy = std::enable_if_t<std::is_convertible<Helper::value_type (*)[], typename Base::value_type (*)[]>::value>*/>
    constexpr span(T* const& data, size_type size) : Base(data, typename Helper::bounds_type{size})
    {}

    // from n-dimensions static array
    template <typename T, size_t N, typename Helper = details::ArrayViewArrayTraits<T, N>,
        typename = std::enable_if_t<std::is_convertible<typename Helper::value_type(*)[], typename Base::value_type(*)[]>::value>>
    constexpr span (T (&arr)[N]) : Base(arr, typename Helper::bounds_type())
    {}

    // from n-dimensions static array with size
    template <typename T, size_t N, typename Helper = details::ArrayViewArrayTraits<T, N>,
        typename = std::enable_if_t<std::is_convertible<typename Helper::value_type(*)[], typename Base::value_type(*)[]>::value>
    >
    constexpr span(T(&arr)[N], size_type size) : Base(arr, typename Helper::bounds_type{size})
    {
        fail_fast_assert(size <= N);
    }

    // from std array
    template <size_t N, 
       typename Dummy = std::enable_if_t<std::is_convertible<static_bounds<N>, typename Base::bounds_type>::value>
    >
    constexpr span (std::array<std::remove_const_t<value_type>, N> & arr) : Base(arr.data(), static_bounds<N>())
    {}

    template <size_t N,
        typename Dummy = std::enable_if_t<std::is_convertible<static_bounds<N>, typename Base::bounds_type>::value
        && std::is_const<value_type>::value>
    >
    constexpr span (const std::array<std::remove_const_t<value_type>, N> & arr) : Base(arr.data(), static_bounds<N>())
    {}
    
    // from begin, end pointers. We don't provide iterator pair since no way to guarantee the contiguity 
    template <typename Ptr,
        typename Dummy = std::enable_if_t<std::is_convertible<Ptr, pointer>::value
        && details::LessThan<Base::bounds_type::dynamic_rank, 2>::value>
    > // remove literal 0 case
    constexpr span (pointer begin, Ptr end) : Base(begin, details::newBoundsHelper<typename Base::bounds_type>(static_cast<pointer>(end) - begin))
    {}

    // from containers. It must has .size() and .data() two function signatures
    template <typename Cont, typename DataType = typename Cont::value_type,
        typename Dummy = std::enable_if_t<!details::is_span<Cont>::value
        && std::is_convertible<DataType (*)[], typename Base::value_type (*)[]>::value
        && std::is_same<std::decay_t<decltype(std::declval<Cont>().size(), *std::declval<Cont>().data())>, DataType>::value>
    >
    constexpr span (Cont& cont) : Base(static_cast<pointer>(cont.data()), details::newBoundsHelper<typename Base::bounds_type>(cont.size()))
    {}

    constexpr span(const span &) = default;

    // convertible
    template <typename OtherValueType, std::ptrdiff_t... OtherDimensions,
        typename BaseType = basic_span<ValueType, static_bounds<FirstDimension, RestDimensions...>>,
        typename OtherBaseType = basic_span<OtherValueType, static_bounds<OtherDimensions...>>,
        typename Dummy = std::enable_if_t<std::is_convertible<OtherBaseType, BaseType>::value>
    >
    constexpr span(const span<OtherValueType, OtherDimensions...> &av)
        : Base(static_cast<const typename span<OtherValueType, OtherDimensions...>::Base&>(av))
    {}

    // reshape
    // DimCount here is a workaround for a bug in MSVC 2015
    template <typename... Dimensions2, size_t DimCount = sizeof...(Dimensions2), typename = std::enable_if_t<(DimCount > 0)>>
    constexpr span<ValueType, Dimensions2::value...> as_span(Dimensions2... dims)
    {
        using BoundsType = typename span<ValueType, (Dimensions2::value)...>::bounds_type;
        auto tobounds = details::static_as_span_helper<BoundsType>(dims..., details::Sep{});
        details::verifyBoundsReshape(this->bounds(), tobounds);
        return {this->data(), tobounds};
    }

    // to bytes array
    template <bool Enabled = std::is_standard_layout<std::decay_t<ValueType>>::value>
    auto as_bytes() const noexcept -> span<const byte>
    {
        static_assert(Enabled, "The value_type of span must be standarded layout");
        return { reinterpret_cast<const byte*>(this->data()), this->bytes() };
    }

    template <bool Enabled = std::is_standard_layout<std::decay_t<ValueType>>::value>
    auto as_writeable_bytes() const noexcept -> span<byte>
    {
        static_assert(Enabled, "The value_type of span must be standarded layout");
        return { reinterpret_cast<byte*>(this->data()), this->bytes() };
    }

    // from bytes array
    template<typename U, bool IsByte = std::is_same<value_type, const byte>::value, typename = std::enable_if_t<IsByte && sizeof...(RestDimensions) == 0>>
    constexpr auto as_span() const noexcept -> span<const U, (Base::bounds_type::static_size != dynamic_range ? static_cast<std::ptrdiff_t>(static_cast<size_t>(Base::bounds_type::static_size) / sizeof(U)) : dynamic_range)>
    {
        static_assert(std::is_standard_layout<U>::value && (Base::bounds_type::static_size == dynamic_range || Base::bounds_type::static_size % static_cast<size_type>(sizeof(U)) == 0),
            "Target type must be standard layout and its size must match the byte array size");
        fail_fast_assert((this->bytes() % sizeof(U)) == 0 && (this->bytes() / sizeof(U)) < PTRDIFF_MAX);
        return { reinterpret_cast<const U*>(this->data()), this->bytes() / static_cast<size_type>(sizeof(U)) };
    }

    template<typename U, bool IsByte = std::is_same<value_type, byte>::value, typename = std::enable_if_t<IsByte && sizeof...(RestDimensions) == 0>>
    constexpr auto as_span() const noexcept -> span<U, (Base::bounds_type::static_size != dynamic_range ? static_cast<ptrdiff_t>(static_cast<size_t>(Base::bounds_type::static_size) / sizeof(U)) : dynamic_range)>
    {
        static_assert(std::is_standard_layout<U>::value && (Base::bounds_type::static_size == dynamic_range || Base::bounds_type::static_size % static_cast<size_t>(sizeof(U)) == 0),
            "Target type must be standard layout and its size must match the byte array size");
        fail_fast_assert((this->bytes() % sizeof(U)) == 0);
        return { reinterpret_cast<U*>(this->data()), this->bytes() / static_cast<size_type>(sizeof(U)) };
    }

    // section on linear space
    template<std::ptrdiff_t Count>
    constexpr span<ValueType, Count> first() const noexcept
    {
        static_assert(bounds_type::static_size == dynamic_range || Count <= bounds_type::static_size, "Index is out of bound");
        fail_fast_assert(bounds_type::static_size != dynamic_range || Count <= this->size()); // ensures we only check condition when needed
        return { this->data(), Count };
    }

    constexpr span<ValueType, dynamic_range> first(size_type count) const noexcept
    {
        fail_fast_assert(count <= this->size());
        return { this->data(), count };
    }

    template<std::ptrdiff_t Count>
    constexpr span<ValueType, Count> last() const noexcept
    {
        static_assert(bounds_type::static_size == dynamic_range || Count <= bounds_type::static_size, "Index is out of bound");
        fail_fast_assert(bounds_type::static_size != dynamic_range || Count <= this->size());
        return { this->data() + this->size() - Count, Count };
    }

    constexpr span<ValueType, dynamic_range> last(size_type count) const noexcept
    {
        fail_fast_assert(count <= this->size());
        return { this->data() + this->size() - count, count };
    }

    template<std::ptrdiff_t Offset, std::ptrdiff_t Count>
    constexpr span<ValueType, Count> sub() const noexcept
    {
        static_assert(bounds_type::static_size == dynamic_range || ((Offset == 0 || Offset <= bounds_type::static_size) && Offset + Count <= bounds_type::static_size), "Index is out of bound");
        fail_fast_assert(bounds_type::static_size != dynamic_range || ((Offset == 0 || Offset <= this->size()) && Offset + Count <= this->size()));
        return { this->data() + Offset, Count };
    }

    constexpr span<ValueType, dynamic_range> sub(size_type offset, size_type count = dynamic_range) const noexcept
    {
        fail_fast_assert((offset == 0 || offset <= this->size()) && (count == dynamic_range || (offset + count) <= this->size()));
        return { this->data() + offset, count == dynamic_range ? this->length() - offset : count };
    }

    // size
    constexpr size_type length() const noexcept
    {
        return this->size();
    }

    constexpr size_type used_length() const noexcept
    {
        return length();
    }

    constexpr size_type bytes() const noexcept
    {
        return sizeof(value_type) * this->size();
    }

    constexpr size_type used_bytes() const noexcept
    {
        return bytes();
    }

    // section
    constexpr strided_span<ValueType, rank> section(index_type origin, index_type extents) const
    {
        size_type size = this->bounds().total_size() - this->bounds().linearize(origin);
        return{ &this->operator[](origin), size, strided_bounds<rank> {extents, details::make_stride(Base::bounds())} };
    }
    
        constexpr reference operator[](const index_type& idx) const
    {
        return Base::operator[](idx);
    }
    
        template <bool Enabled = (rank > 1), typename Dummy = std::enable_if_t<Enabled>>
    constexpr span<ValueType, RestDimensions...> operator[](size_type idx) const
    {
        auto ret = Base::operator[](idx);
        return{ ret.data(), ret.bounds() };
    }

    using Base::operator==;
    using Base::operator!=;
    using Base::operator<;
    using Base::operator<=;
    using Base::operator>;
    using Base::operator>=;
};

template <typename T, std::ptrdiff_t... Dimensions>
constexpr auto as_span(T* const& ptr, dim<Dimensions>... args) -> span<std::remove_all_extents_t<T>, Dimensions...>
{
    return {reinterpret_cast<std::remove_all_extents_t<T>*>(ptr), details::static_as_span_helper<static_bounds<Dimensions...>>(args..., details::Sep{})};
}

template <typename T>
constexpr auto as_span (T* arr, std::ptrdiff_t len) -> typename details::ArrayViewArrayTraits<T, dynamic_range>::type
{
    return {reinterpret_cast<std::remove_all_extents_t<T>*>(arr), len};
}

template <typename T, size_t N>
constexpr auto as_span (T (&arr)[N]) -> typename details::ArrayViewArrayTraits<T, N>::type
{
    return {arr};
}

template <typename T, size_t N>
constexpr span<const T, N> as_span(const std::array<T, N> &arr)
{
    return {arr};
}

template <typename T, size_t N>
constexpr span<const T, N> as_span(const std::array<T, N> &&) = delete;

template <typename T, size_t N>
constexpr span<T, N> as_span(std::array<T, N> &arr)
{
    return {arr};
}

template <typename T>
constexpr span<T, dynamic_range> as_span(T *begin, T *end)
{
    return {begin, end};
}

template <typename Cont>
constexpr auto as_span(Cont &arr) -> std::enable_if_t<!details::is_span<std::decay_t<Cont>>::value,
    span<std::remove_reference_t<decltype(arr.size(), *arr.data())>, dynamic_range>>
{
    fail_fast_assert(arr.size() < PTRDIFF_MAX);
    return {arr.data(), static_cast<std::ptrdiff_t>(arr.size())};
}

template <typename Cont>
constexpr auto as_span(Cont &&arr) -> std::enable_if_t<!details::is_span<std::decay_t<Cont>>::value,
    span<std::remove_reference_t<decltype(arr.size(), *arr.data())>, dynamic_range>> = delete;

template <typename ValueType, size_t Rank>
class strided_span : public basic_span<ValueType, strided_bounds<Rank>>
{
    using Base = basic_span<ValueType, strided_bounds<Rank>>;

    template<typename OtherValue, size_t OtherRank>
    friend class strided_span;

public:
    using Base::rank;
    using typename Base::bounds_type;
    using typename Base::size_type;
    using typename Base::pointer;
    using typename Base::value_type;
    using typename Base::index_type;
    using typename Base::iterator;
    using typename Base::const_iterator;
    using typename Base::reference;
    
    // from static array of size N
    template<size_type N>
    strided_span(value_type(&values)[N], bounds_type bounds) : Base(values, std::move(bounds))
    {
        fail_fast_assert(this->bounds().total_size() <= N, "Bounds cross data boundaries");
    }

    // from raw data
    strided_span(pointer ptr, size_type size, bounds_type bounds): Base(ptr, std::move(bounds))
    {
        fail_fast_assert(this->bounds().total_size() <= size, "Bounds cross data boundaries");
    }

    // from array view
    template <std::ptrdiff_t... Dimensions, typename Dummy = std::enable_if<sizeof...(Dimensions) == Rank>>
    strided_span(span<ValueType, Dimensions...> av, bounds_type bounds) : Base(av.data(), std::move(bounds))
    {
        fail_fast_assert(this->bounds().total_size() <= av.bounds().total_size(), "Bounds cross data boundaries");
    }
    
    // convertible
    template <typename OtherValueType,
        typename BaseType = basic_span<ValueType, strided_bounds<Rank>>,
        typename OtherBaseType = basic_span<OtherValueType, strided_bounds<Rank>>,
        typename Dummy = std::enable_if_t<std::is_convertible<OtherBaseType, BaseType>::value>
    >
    constexpr strided_span(const strided_span<OtherValueType, Rank> &av) : Base(static_cast<const typename strided_span<OtherValueType, Rank>::Base &>(av)) // static_cast is required
    {}

    // convert from bytes
        template <typename OtherValueType>
        strided_span<typename std::enable_if<std::is_same<value_type, const byte>::value, OtherValueType>::type, rank> as_strided_span() const 
    {
        static_assert((sizeof(OtherValueType) >= sizeof(value_type)) && (sizeof(OtherValueType) % sizeof(value_type) == 0), "OtherValueType should have a size to contain a multiple of ValueTypes");
        auto d = static_cast<size_type>(sizeof(OtherValueType) / sizeof(value_type));

        size_type size = this->bounds().total_size() / d;
        return{ (OtherValueType*)this->data(), size, bounds_type{ resize_extent(this->bounds().index_bounds(), d), resize_stride(this->bounds().strides(), d)} };
    }

    strided_span section(index_type origin, index_type extents) const
    {
        size_type size = this->bounds().total_size() - this->bounds().linearize(origin);
        return { &this->operator[](origin), size, bounds_type {extents, details::make_stride(Base::bounds())}};
    }

    constexpr reference operator[](const index_type& idx) const
    {
        return Base::operator[](idx);
    }

    template <bool Enabled = (rank > 1), typename Dummy = std::enable_if_t<Enabled>>
    constexpr strided_span<value_type, rank-1> operator[](size_type idx) const
    {
        auto ret = Base::operator[](idx);
        return{ ret.data(), ret.bounds().total_size(), ret.bounds() };
    }

private:
    static index_type resize_extent(const index_type& extent, std::ptrdiff_t d)
    {
        fail_fast_assert(extent[rank - 1] >= d && (extent[rank-1] % d == 0), "The last dimension of the array needs to contain a multiple of new type elements");

        index_type ret = extent;
        ret[rank - 1] /= d;

        return ret;
    }

    template <bool Enabled = (rank == 1), typename Dummy = std::enable_if_t<Enabled>>
    static index_type resize_stride(const index_type& strides, std::ptrdiff_t , void * = 0)
    {
        fail_fast_assert(strides[rank - 1] == 1, "Only strided arrays with regular strides can be resized");

        return strides;
    }

    template <bool Enabled = (rank > 1), typename Dummy = std::enable_if_t<Enabled>>
    static index_type resize_stride(const index_type& strides, std::ptrdiff_t d)
    {
        fail_fast_assert(strides[rank - 1] == 1, "Only strided arrays with regular strides can be resized");
        fail_fast_assert(strides[rank - 2] >= d && (strides[rank - 2] % d == 0), "The strides must have contiguous chunks of memory that can contain a multiple of new type elements");

        for (size_t i = rank - 1; i > 0; --i)
            fail_fast_assert((strides[i-1] >= strides[i]) && (strides[i-1] % strides[i] == 0), "Only strided arrays with regular strides can be resized");

        index_type ret = strides / d;
        ret[rank - 1] = 1;

        return ret;
    }
};

template <typename ArrayView>
class contiguous_span_iterator : public std::iterator<std::random_access_iterator_tag, typename ArrayView::value_type>
{
    using Base = std::iterator<std::random_access_iterator_tag, typename ArrayView::value_type>;
public:
    using typename Base::reference;
    using typename Base::pointer;
    using typename Base::difference_type;

private:
    template <typename ValueType, typename Bounds>
    friend class basic_span;

    pointer m_pdata;
    const ArrayView * m_validator;
    void validateThis() const
    {
        fail_fast_assert(m_pdata >= m_validator->m_pdata && m_pdata < m_validator->m_pdata + m_validator->size(), "iterator is out of range of the array");
    }
    contiguous_span_iterator (const ArrayView *container, bool isbegin) :
        m_pdata(isbegin ? container->m_pdata : container->m_pdata + container->size()), m_validator(container) {}
public:
    reference operator*() const noexcept
    {
        validateThis();
        return *m_pdata;
    }
    pointer operator->() const noexcept
    {
        validateThis();
        return m_pdata;
    }
    contiguous_span_iterator& operator++() noexcept
    {
        ++m_pdata;
        return *this;
    }
    contiguous_span_iterator operator++(int)noexcept
    {
        auto ret = *this;
        ++(*this);
        return ret;
    }
    contiguous_span_iterator& operator--() noexcept
    {
        --m_pdata;
        return *this;
    }
    contiguous_span_iterator operator--(int)noexcept
    {
        auto ret = *this;
        --(*this);
        return ret;
    }
    contiguous_span_iterator operator+(difference_type n) const noexcept
    {
        contiguous_span_iterator ret{ *this };
        return ret += n;
    }
    contiguous_span_iterator& operator+=(difference_type n) noexcept
    {
        m_pdata += n;
        return *this;
    }
    contiguous_span_iterator operator-(difference_type n) const noexcept
    {
        contiguous_span_iterator ret{ *this };
        return ret -= n;
    }
    contiguous_span_iterator& operator-=(difference_type n) noexcept
    {
        return *this += -n;
    }
    difference_type operator-(const contiguous_span_iterator& rhs) const noexcept
    {
        fail_fast_assert(m_validator == rhs.m_validator);
        return m_pdata - rhs.m_pdata;
    }
    reference operator[](difference_type n) const noexcept
    {
        return *(*this + n);
    }
    bool operator==(const contiguous_span_iterator& rhs) const noexcept
    {
        fail_fast_assert(m_validator == rhs.m_validator);
        return m_pdata == rhs.m_pdata;
    }
    bool operator!=(const contiguous_span_iterator& rhs) const noexcept
    {
        return !(*this == rhs);
    }
    bool operator<(const contiguous_span_iterator& rhs) const noexcept
    {
        fail_fast_assert(m_validator == rhs.m_validator);
        return m_pdata < rhs.m_pdata;
    }
    bool operator<=(const contiguous_span_iterator& rhs) const noexcept
    {
        return !(rhs < *this);
    }
    bool operator>(const contiguous_span_iterator& rhs) const noexcept
    {
        return rhs < *this;
    }
    bool operator>=(const contiguous_span_iterator& rhs) const noexcept
    {
        return !(rhs > *this);
    }
    void swap(contiguous_span_iterator& rhs) noexcept
    {
        std::swap(m_pdata, rhs.m_pdata);
        std::swap(m_validator, rhs.m_validator);
    }
};

template <typename ArrayView>
contiguous_span_iterator<ArrayView> operator+(typename contiguous_span_iterator<ArrayView>::difference_type n, const contiguous_span_iterator<ArrayView>& rhs) noexcept
{
    return rhs + n;
}

template <typename ArrayView>
class general_span_iterator : public std::iterator<std::random_access_iterator_tag, typename ArrayView::value_type>
{
    using Base = std::iterator<std::random_access_iterator_tag, typename ArrayView::value_type>;
public:
    using typename Base::reference;
    using typename Base::pointer;
    using typename Base::difference_type;
    using typename Base::value_type;
private:
    template <typename ValueType, typename Bounds>
    friend class basic_span;
    
    const ArrayView * m_container;
    typename ArrayView::bounds_type::iterator m_itr;
    general_span_iterator(const ArrayView *container, bool isbegin) :
        m_container(container), m_itr(isbegin ? m_container->bounds().begin() : m_container->bounds().end())
    {}
public:
    reference operator*() noexcept
    {
        return (*m_container)[*m_itr];
    }
    pointer operator->() noexcept
    {
        return &(*m_container)[*m_itr];
    }
    general_span_iterator& operator++() noexcept
    {
        ++m_itr;
        return *this;
    }
    general_span_iterator operator++(int)noexcept
    {
        auto ret = *this;
        ++(*this);
        return ret;
    }
    general_span_iterator& operator--() noexcept
    {
        --m_itr;
        return *this;
    }
    general_span_iterator operator--(int)noexcept
    {
        auto ret = *this;
        --(*this);
        return ret;
    }
    general_span_iterator operator+(difference_type n) const noexcept
    {
        general_span_iterator ret{ *this };
        return ret += n;
    }
    general_span_iterator& operator+=(difference_type n) noexcept
    {
        m_itr += n;
        return *this;
    }
    general_span_iterator operator-(difference_type n) const noexcept
    {
        general_span_iterator ret{ *this };
        return ret -= n;
    }
    general_span_iterator& operator-=(difference_type n) noexcept
    {
        return *this += -n;
    }
    difference_type operator-(const general_span_iterator& rhs) const noexcept
    {
        fail_fast_assert(m_container == rhs.m_container);
        return m_itr - rhs.m_itr;
    }
    value_type operator[](difference_type n) const noexcept
    {
        return (*m_container)[m_itr[n]];;
    }
    bool operator==(const general_span_iterator& rhs) const noexcept
    {
        fail_fast_assert(m_container == rhs.m_container);
        return m_itr == rhs.m_itr;
    }
    bool operator !=(const general_span_iterator& rhs) const noexcept
    {
        return !(*this == rhs);
    }
    bool operator<(const general_span_iterator& rhs) const noexcept
    {
        fail_fast_assert(m_container == rhs.m_container);
        return m_itr < rhs.m_itr;
    }
    bool operator<=(const general_span_iterator& rhs) const noexcept
    {
        return !(rhs < *this);
    }
    bool operator>(const general_span_iterator& rhs) const noexcept
    {
        return rhs < *this;
    }
    bool operator>=(const general_span_iterator& rhs) const noexcept
    {
        return !(rhs > *this);
    }
    void swap(general_span_iterator& rhs) noexcept
    {
        std::swap(m_itr, rhs.m_itr);
        std::swap(m_container, rhs.m_container);
    }
};

template <typename ArrayView>
general_span_iterator<ArrayView> operator+(typename general_span_iterator<ArrayView>::difference_type n, const general_span_iterator<ArrayView>& rhs) noexcept
{
    return rhs + n;
}

} // namespace gsl

#ifdef _MSC_VER

#undef constexpr
#pragma pop_macro("constexpr")

#if _MSC_VER <= 1800
#pragma warning(pop)

#ifndef GSL_THROWS_FOR_TESTING
#pragma undef noexcept
#endif // GSL_THROWS_FOR_TESTING

#undef GSL_MSVC_HAS_VARIADIC_CTOR_BUG 
#pragma pop_macro("GSL_MSVC_HAS_VARIADIC_CTOR_BUG") 


#endif // _MSC_VER <= 1800

#endif // _MSC_VER

#if defined(GSL_THROWS_FOR_TESTING) 
#undef noexcept 
#endif // GSL_THROWS_FOR_TESTING 


#endif // GSL_SPAN_H
