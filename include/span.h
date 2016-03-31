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
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
#include <new>
#include <numeric>
#include <stdexcept>
#include <type_traits>
#include <utility>

#ifdef _MSC_VER

#pragma warning(push)

// turn off some warnings that are noisy about our Expects statements
#pragma warning(disable : 4127) // conditional expression is constant

// blanket turn off warnings from CppCoreCheck for now
// so people aren't annoyed by them when running the tool.
// more targeted suppressions will be added in a future update to the GSL
#pragma warning(disable: 26481 26482 26483 26485 26490 26491 26492 26493 26495)

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

    template <typename... Ts>
    class are_integral : public std::integral_constant<bool, true>
    {
    };

    template <typename T, typename... Ts>
    class are_integral<T, Ts...>
        : public std::integral_constant<bool,
                                        std::is_integral<T>::value && are_integral<Ts...>::value>
    {
    };
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

    constexpr index() noexcept {}

    constexpr index(const value_type (&values)[Rank]) noexcept
    {
        std::copy(values, values + Rank, elems);
    }

#ifdef GSL_MSVC_HAS_VARIADIC_CTOR_BUG
    template <
        typename T, typename... Ts,
        typename = std::enable_if_t<((sizeof...(Ts) + 1) == Rank) && std::is_integral<T>::value &&
                                    details::are_integral<Ts...>::value>>
    constexpr index(T t, Ts... ds)
        : index({narrow_cast<value_type>(t), narrow_cast<value_type>(ds)...})
    {
    }
#else
    template <typename... Ts, typename = std::enable_if_t<(sizeof...(Ts) == Rank) &&
                                                          details::are_integral<Ts...>::value>>
    constexpr index(Ts... ds) noexcept : elems{narrow_cast<value_type>(ds)...}
    {
    }
#endif

    constexpr index(const index& other) noexcept = default;

    constexpr index& operator=(const index& rhs) noexcept = default;

    // Preconditions: component_idx < rank
    constexpr reference operator[](size_t component_idx)
    {
        Expects(component_idx < Rank); // Component index must be less than rank
        return elems[component_idx];
    }

    // Preconditions: component_idx < rank
    constexpr const_reference operator[](size_t component_idx) const noexcept
    {
        Expects(component_idx < Rank); // Component index must be less than rank
        return elems[component_idx];
    }

    constexpr bool operator==(const index& rhs) const noexcept
    {
        return std::equal(elems, elems + rank, rhs.elems);
    }

    constexpr bool operator!=(const index& rhs) const noexcept { return !(this == rhs); }

    constexpr index operator+() const noexcept { return *this; }

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
        std::transform(elems, elems + rank, elems,
                       [v](value_type x) { return std::multiplies<value_type>{}(x, v); });
        return *this;
    }

    constexpr index& operator/=(value_type v) noexcept
    {
        std::transform(elems, elems + rank, elems,
                       [v](value_type x) { return std::divides<value_type>{}(x, v); });
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
        return narrow_cast<T>(-1);
    }

    template <typename T, typename Dummy = std::enable_if_t<std::is_integral<T>::value>>
    constexpr bool operator==(T other) const noexcept
    {
        return narrow_cast<T>(-1) == other;
    }

    template <typename T, typename Dummy = std::enable_if_t<std::is_integral<T>::value>>
    constexpr bool operator!=(T other) const noexcept
    {
        return narrow_cast<T>(-1) != other;
    }
};

template <typename T, typename Dummy = std::enable_if_t<std::is_integral<T>::value>>
constexpr bool operator==(T left, static_bounds_dynamic_range_t right) noexcept
{
    return right == left;
}

template <typename T, typename Dummy = std::enable_if_t<std::is_integral<T>::value>>
constexpr bool operator!=(T left, static_bounds_dynamic_range_t right) noexcept
{
    return right != left;
}

constexpr static_bounds_dynamic_range_t dynamic_range{};
#else
const std::ptrdiff_t dynamic_range = -1;
#endif

struct generalized_mapping_tag
{
};
struct contiguous_mapping_tag : generalized_mapping_tag
{
};

namespace details
{

    template <std::ptrdiff_t Left, std::ptrdiff_t Right>
    struct LessThan
    {
        static const bool value = Left < Right;
    };

    template <std::ptrdiff_t... Ranges>
    struct BoundsRanges
    {
        using size_type = std::ptrdiff_t;
        static const size_type Depth = 0;
        static const size_type DynamicNum = 0;
        static const size_type CurrentRange = 1;
        static const size_type TotalSize = 1;

        // TODO : following signature is for work around VS bug
        template <typename OtherRange>
        BoundsRanges(const OtherRange&, bool /* firstLevel */)
        {
        }

        BoundsRanges(const BoundsRanges&) = default;
        BoundsRanges& operator=(const BoundsRanges&) = default;
        BoundsRanges(const std::ptrdiff_t* const) {}
        BoundsRanges() = default;

        template <typename T, size_t Dim>
        void serialize(T&) const
        {
        }

        template <typename T, size_t Dim>
        size_type linearize(const T&) const
        {
            return 0;
        }

        template <typename T, size_t Dim>
        size_type contains(const T&) const
        {
            return -1;
        }

        size_type elementNum(size_t) const noexcept { return 0; }

        size_type totalSize() const noexcept { return TotalSize; }

        bool operator==(const BoundsRanges&) const noexcept { return true; }
    };

    template <std::ptrdiff_t... RestRanges>
    struct BoundsRanges<dynamic_range, RestRanges...> : BoundsRanges<RestRanges...>
    {
        using Base = BoundsRanges<RestRanges...>;
        using size_type = std::ptrdiff_t;
        static const size_t Depth = Base::Depth + 1;
        static const size_t DynamicNum = Base::DynamicNum + 1;
        static const size_type CurrentRange = dynamic_range;
        static const size_type TotalSize = dynamic_range;
        const size_type m_bound;

        BoundsRanges(const BoundsRanges&) = default;

        BoundsRanges(const std::ptrdiff_t* const arr)
            : Base(arr + 1), m_bound(*arr * this->Base::totalSize())
        {
            Expects(0 <= *arr);
        }

        BoundsRanges() : m_bound(0) {}

        template <std::ptrdiff_t OtherRange, std::ptrdiff_t... RestOtherRanges>
        BoundsRanges(const BoundsRanges<OtherRange, RestOtherRanges...>& other,
                     bool /* firstLevel */ = true)
            : Base(static_cast<const BoundsRanges<RestOtherRanges...>&>(other), false)
            , m_bound(other.totalSize())
        {
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
            const size_type index = this->Base::totalSize() * arr[Dim];
            Expects(index < m_bound);
            return index + this->Base::template linearize<T, Dim + 1>(arr);
        }

        template <typename T, size_t Dim = 0>
        size_type contains(const T& arr) const
        {
            const ptrdiff_t last = this->Base::template contains<T, Dim + 1>(arr);
            if (last == -1) return -1;
            const ptrdiff_t cur = this->Base::totalSize() * arr[Dim];
            return cur < m_bound ? cur + last : -1;
        }

        size_type totalSize() const noexcept { return m_bound; }

        size_type elementNum() const noexcept { return totalSize() / this->Base::totalSize(); }

        size_type elementNum(size_t dim) const noexcept
        {
            if (dim > 0)
                return this->Base::elementNum(dim - 1);
            else
                return elementNum();
        }

        bool operator==(const BoundsRanges& rhs) const noexcept
        {
            return m_bound == rhs.m_bound &&
                   static_cast<const Base&>(*this) == static_cast<const Base&>(rhs);
        }
    };

    template <std::ptrdiff_t CurRange, std::ptrdiff_t... RestRanges>
    struct BoundsRanges<CurRange, RestRanges...> : BoundsRanges<RestRanges...>
    {
        using Base = BoundsRanges<RestRanges...>;
        using size_type = std::ptrdiff_t;
        static const size_t Depth = Base::Depth + 1;
        static const size_t DynamicNum = Base::DynamicNum;
        static const size_type CurrentRange = CurRange;
        static const size_type TotalSize =
            Base::TotalSize == dynamic_range ? dynamic_range : CurrentRange * Base::TotalSize;

        BoundsRanges(const BoundsRanges&) = default;

        BoundsRanges(const std::ptrdiff_t* const arr) : Base(arr) {}
        BoundsRanges() = default;

        template <std::ptrdiff_t OtherRange, std::ptrdiff_t... RestOtherRanges>
        BoundsRanges(const BoundsRanges<OtherRange, RestOtherRanges...>& other,
                     bool firstLevel = true)
            : Base(static_cast<const BoundsRanges<RestOtherRanges...>&>(other), false)
        {
            (void) firstLevel;
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
            Expects(arr[Dim] < CurrentRange); // Index is out of range
            return this->Base::totalSize() * arr[Dim] +
                   this->Base::template linearize<T, Dim + 1>(arr);
        }

        template <typename T, size_t Dim = 0>
        size_type contains(const T& arr) const
        {
            if (arr[Dim] >= CurrentRange) return -1;
            const size_type last = this->Base::template contains<T, Dim + 1>(arr);
            if (last == -1) return -1;
            return this->Base::totalSize() * arr[Dim] + last;
        }

        size_type totalSize() const noexcept { return CurrentRange * this->Base::totalSize(); }

        size_type elementNum() const noexcept { return CurrentRange; }

        size_type elementNum(size_t dim) const noexcept
        {
            if (dim > 0)
                return this->Base::elementNum(dim - 1);
            else
                return elementNum();
        }

        bool operator==(const BoundsRanges& rhs) const noexcept
        {
            return static_cast<const Base&>(*this) == static_cast<const Base&>(rhs);
        }
    };

    template <typename SourceType, typename TargetType>
    struct BoundsRangeConvertible
        : public std::integral_constant<bool, (SourceType::TotalSize >= TargetType::TotalSize ||
                                               TargetType::TotalSize == dynamic_range ||
                                               SourceType::TotalSize == dynamic_range ||
                                               TargetType::TotalSize == 0)>
    {
    };

    template <typename TypeChain>
    struct TypeListIndexer
    {
        const TypeChain& obj_;
        TypeListIndexer(const TypeChain& obj) : obj_(obj) {}

        template <size_t N>
        const TypeChain& getObj(std::true_type)
        {
            return obj_;
        }

        template <size_t N, typename MyChain = TypeChain, typename MyBase = typename MyChain::Base>
        auto getObj(std::false_type)
            -> decltype(TypeListIndexer<MyBase>(static_cast<const MyBase&>(obj_)).template get<N>())
        {
            return TypeListIndexer<MyBase>(static_cast<const MyBase&>(obj_)).template get<N>();
        }

        template <size_t N>
        auto get() -> decltype(getObj<N - 1>(std::integral_constant<bool, N == 0>()))
        {
            return getObj<N - 1>(std::integral_constant<bool, N == 0>());
        }
    };

    template <typename TypeChain>
    TypeListIndexer<TypeChain> createTypeListIndexer(const TypeChain& obj)
    {
        return TypeListIndexer<TypeChain>(obj);
    }

    template <size_t Rank, bool Enabled = (Rank > 1),
              typename Ret = std::enable_if_t<Enabled, index<Rank - 1>>>
    constexpr Ret shift_left(const index<Rank>& other) noexcept
    {
        Ret ret{};
        for (size_t i = 0; i < Rank - 1; ++i) {
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
    static_bounds(const details::BoundsRanges<Ranges...>&) {}
};

template <std::ptrdiff_t FirstRange, std::ptrdiff_t... RestRanges>
class static_bounds<FirstRange, RestRanges...>
{
    using MyRanges = details::BoundsRanges<FirstRange, RestRanges...>;

    MyRanges m_ranges;
    constexpr static_bounds(const MyRanges& range) : m_ranges(range) {}

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

    template <typename SourceType, typename TargetType, size_t Rank>
    struct BoundsRangeConvertible2;

    template <size_t Rank, typename SourceType, typename TargetType,
              typename Ret = BoundsRangeConvertible2<typename SourceType::Base,
                                                     typename TargetType::Base, Rank>>
    static auto helpBoundsRangeConvertible(SourceType, TargetType, std::true_type) -> Ret;

    template <size_t Rank, typename SourceType, typename TargetType>
    static auto helpBoundsRangeConvertible(SourceType, TargetType, ...) -> std::false_type;

    template <typename SourceType, typename TargetType, size_t Rank>
    struct BoundsRangeConvertible2
        : decltype(helpBoundsRangeConvertible<Rank - 1>(
              SourceType(), TargetType(),
              std::integral_constant<bool,
                                     SourceType::Depth == TargetType::Depth &&
                                         (SourceType::CurrentRange == TargetType::CurrentRange ||
                                          TargetType::CurrentRange == dynamic_range ||
                                          SourceType::CurrentRange == dynamic_range)>()))
    {
    };

    template <typename SourceType, typename TargetType>
    struct BoundsRangeConvertible2<SourceType, TargetType, 0> : std::true_type
    {
    };

    template <typename SourceType, typename TargetType, std::ptrdiff_t Rank = TargetType::Depth>
    struct BoundsRangeConvertible
        : decltype(helpBoundsRangeConvertible<Rank - 1>(
              SourceType(), TargetType(),
              std::integral_constant<bool,
                                     SourceType::Depth == TargetType::Depth &&
                                         (!details::LessThan<SourceType::CurrentRange,
                                                             TargetType::CurrentRange>::value ||
                                          TargetType::CurrentRange == dynamic_range ||
                                          SourceType::CurrentRange == dynamic_range)>()))
    {
    };

    template <typename SourceType, typename TargetType>
    struct BoundsRangeConvertible<SourceType, TargetType, 0> : std::true_type
    {
    };

    template <std::ptrdiff_t... Ranges,
              typename = std::enable_if_t<details::BoundsRangeConvertible<
                  details::BoundsRanges<Ranges...>,
                  details::BoundsRanges<FirstRange, RestRanges...>>::value>>
    constexpr static_bounds(const static_bounds<Ranges...>& other) : m_ranges(other.m_ranges)
    {
        Expects((MyRanges::DynamicNum == 0 && details::BoundsRanges<Ranges...>::DynamicNum == 0) ||
                MyRanges::DynamicNum > 0 || other.m_ranges.totalSize() >= m_ranges.totalSize());
    }

    constexpr static_bounds(std::initializer_list<size_type> il)
        : m_ranges(static_cast<const std::ptrdiff_t*>(il.begin()))
    {
        // Size of the initializer list must match the rank of the array
        Expects((MyRanges::DynamicNum == 0 && il.size() == 1 && *il.begin() == static_size) ||
                MyRanges::DynamicNum == il.size());
        // Size of the range must be less than the max element of the size type
        Expects(m_ranges.totalSize() <= PTRDIFF_MAX);
    }

    constexpr static_bounds() = default;

    constexpr static_bounds& operator=(const static_bounds& otherBounds)
    {
        new (&m_ranges) MyRanges(otherBounds.m_ranges);
        return *this;
    }

    constexpr sliced_type slice() const noexcept
    {
        return sliced_type{static_cast<const details::BoundsRanges<RestRanges...>&>(m_ranges)};
    }

    constexpr size_type stride() const noexcept { return rank > 1 ? slice().size() : 1; }

    constexpr size_type size() const noexcept { return m_ranges.totalSize(); }

    constexpr size_type total_size() const noexcept { return m_ranges.totalSize(); }

    constexpr size_type linearize(const index_type& idx) const { return m_ranges.linearize(idx); }

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
        static_assert(Dim < rank,
                      "dimension should be less than rank (dimension count starts from 0)");
        return details::createTypeListIndexer(m_ranges).template get<Dim>().elementNum();
    }

    template <typename IntType>
    constexpr size_type extent(IntType dim) const noexcept
    {
        static_assert(std::is_integral<IntType>::value,
                      "Dimension parameter must be supplied as an integral type.");
        auto real_dim = narrow_cast<size_t>(dim);
        Expects(real_dim < rank);

        return m_ranges.elementNum(real_dim);
    }

    constexpr index_type index_bounds() const noexcept
    {
        size_type extents[rank] = {};
        m_ranges.serialize(extents);
        return {extents};
    }

    template <std::ptrdiff_t... Ranges>
    constexpr bool operator==(const static_bounds<Ranges...>& rhs) const noexcept
    {
        return this->size() == rhs.size();
    }

    template <std::ptrdiff_t... Ranges>
    constexpr bool operator!=(const static_bounds<Ranges...>& rhs) const noexcept
    {
        return !(*this == rhs);
    }

    constexpr const_iterator begin() const noexcept { return const_iterator(*this, index_type{}); }

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
    using value_type = std::ptrdiff_t;
    using reference = std::add_lvalue_reference_t<value_type>;
    using const_reference = std::add_const_t<reference>;
    using size_type = value_type;
    using difference_type = value_type;
    using index_type = index<rank>;
    using const_index_type = std::add_const_t<index_type>;
    using iterator = bounds_iterator<const_index_type>;
    using const_iterator = bounds_iterator<const_index_type>;
    static const value_type dynamic_rank = rank;
    static const value_type static_size = dynamic_range;
    using sliced_type = std::conditional_t<rank != 0, strided_bounds<rank - 1>, void>;
    using mapping_type = generalized_mapping_tag;

    constexpr strided_bounds(const strided_bounds&) noexcept = default;

    constexpr strided_bounds& operator=(const strided_bounds&) noexcept = default;

    constexpr strided_bounds(const value_type (&values)[rank], index_type strides)
        : m_extents(values), m_strides(std::move(strides))
    {
    }

    constexpr strided_bounds(const index_type& extents, const index_type& strides) noexcept
        : m_extents(extents),
          m_strides(strides)
    {
    }

    constexpr index_type strides() const noexcept { return m_strides; }

    constexpr size_type total_size() const noexcept
    {
        size_type ret = 0;
        for (size_t i = 0; i < rank; ++i) {
            ret += (m_extents[i] - 1) * m_strides[i];
        }
        return ret + 1;
    }

    constexpr size_type size() const noexcept
    {
        size_type ret = 1;
        for (size_t i = 0; i < rank; ++i) {
            ret *= m_extents[i];
        }
        return ret;
    }

    constexpr bool contains(const index_type& idx) const noexcept
    {
        for (size_t i = 0; i < rank; ++i) {
            if (idx[i] < 0 || idx[i] >= m_extents[i]) return false;
        }
        return true;
    }

    constexpr size_type linearize(const index_type& idx) const noexcept
    {
        size_type ret = 0;
        for (size_t i = 0; i < rank; i++) {
            Expects(idx[i] < m_extents[i]); // index is out of bounds of the array
            ret += idx[i] * m_strides[i];
        }
        return ret;
    }

    constexpr size_type stride() const noexcept { return m_strides[0]; }

    template <bool Enabled = (rank > 1), typename Ret = std::enable_if_t<Enabled, sliced_type>>
    constexpr sliced_type slice() const
    {
        return {details::shift_left(m_extents), details::shift_left(m_strides)};
    }

    template <size_t Dim = 0>
    constexpr size_type extent() const noexcept
    {
        static_assert(Dim < Rank,
                      "dimension should be less than rank (dimension count starts from 0)");
        return m_extents[Dim];
    }

    constexpr index_type index_bounds() const noexcept { return m_extents; }
    constexpr const_iterator begin() const noexcept { return const_iterator{*this, index_type{}}; }

    constexpr const_iterator end() const noexcept { return const_iterator{*this, index_bounds()}; }

private:
    index_type m_extents;
    index_type m_strides;
};

template <typename T>
struct is_bounds : std::integral_constant<bool, false>
{
};
template <std::ptrdiff_t... Ranges>
struct is_bounds<static_bounds<Ranges...>> : std::integral_constant<bool, true>
{
};
template <size_t Rank>
struct is_bounds<strided_bounds<Rank>> : std::integral_constant<bool, true>
{
};

template <typename IndexType>
class bounds_iterator : public std::iterator<std::random_access_iterator_tag, IndexType>
{
private:
    using Base = std::iterator<std::random_access_iterator_tag, IndexType>;

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
        : boundary_(bnd.index_bounds()),
          curr_(std::move(curr))
    {
        static_assert(is_bounds<Bounds>::value, "Bounds type must be provided");
    }

    constexpr reference operator*() const noexcept { return curr_; }

    constexpr pointer operator->() const noexcept { return &curr_; }

    constexpr bounds_iterator& operator++() noexcept
    {
        for (size_t i = rank; i-- > 0;) {
            if (curr_[i] < boundary_[i] - 1) {
                curr_[i]++;
                return *this;
            }
            curr_[i] = 0;
        }
        // If we're here we've wrapped over - set to past-the-end.
        curr_ = boundary_;
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
        if (!less(curr_, boundary_)) {
            // if at the past-the-end, set to last element
            for (size_t i = 0; i < rank; ++i) {
                curr_[i] = boundary_[i] - 1;
            }
            return *this;
        }
        for (size_t i = rank; i-- > 0;) {
            if (curr_[i] >= 1) {
                curr_[i]--;
                return *this;
            }
            curr_[i] = boundary_[i] - 1;
        }
        // If we're here the preconditions were violated
        // "pre: there exists s such that r == ++s"
        Expects(false);
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
        bounds_iterator ret{*this};
        return ret += n;
    }

    constexpr bounds_iterator& operator+=(difference_type n) noexcept
    {
        auto linear_idx = linearize(curr_) + n;
        std::remove_const_t<value_type> stride = 0;
        stride[rank - 1] = 1;
        for (size_t i = rank - 1; i-- > 0;) {
            stride[i] = stride[i + 1] * boundary_[i + 1];
        }
        for (size_t i = 0; i < rank; ++i) {
            curr_[i] = linear_idx / stride[i];
            linear_idx = linear_idx % stride[i];
        }
        // index is out of bounds of the array
        Expects(!less(curr_, index_type{}) && !less(boundary_, curr_));
        return *this;
    }

    constexpr bounds_iterator operator-(difference_type n) const noexcept
    {
        bounds_iterator ret{*this};
        return ret -= n;
    }

    constexpr bounds_iterator& operator-=(difference_type n) noexcept { return * this += -n; }

    constexpr difference_type operator-(const bounds_iterator& rhs) const noexcept
    {
        return linearize(curr_) - linearize(rhs.curr_);
    }

    constexpr value_type operator[](difference_type n) const noexcept { return *(*this + n); }

    constexpr bool operator==(const bounds_iterator& rhs) const noexcept
    {
        return curr_ == rhs.curr_;
    }

    constexpr bool operator!=(const bounds_iterator& rhs) const noexcept { return !(*this == rhs); }

    constexpr bool operator<(const bounds_iterator& rhs) const noexcept
    {
        return less(curr_, rhs.curr_);
    }

    constexpr bool operator<=(const bounds_iterator& rhs) const noexcept { return !(rhs < *this); }

    constexpr bool operator>(const bounds_iterator& rhs) const noexcept { return rhs < *this; }

    constexpr bool operator>=(const bounds_iterator& rhs) const noexcept { return !(rhs > *this); }

    void swap(bounds_iterator& rhs) noexcept
    {
        std::swap(boundary_, rhs.boundary_);
        std::swap(curr_, rhs.curr_);
    }

private:
    constexpr bool less(index_type& one, index_type& other) const noexcept
    {
        for (size_t i = 0; i < rank; ++i) {
            if (one[i] < other[i]) return true;
        }
        return false;
    }

    constexpr index_size_type linearize(const value_type& idx) const noexcept
    {
        // TODO: Smarter impl.
        // Check if past-the-end
        index_size_type multiplier = 1;
        index_size_type res = 0;
        if (!less(idx, boundary_)) {
            res = 1;
            for (size_t i = rank; i-- > 0;) {
                res += (idx[i] - 1) * multiplier;
                multiplier *= boundary_[i];
            }
        }
        else
        {
            for (size_t i = rank; i-- > 0;) {
                res += idx[i] * multiplier;
                multiplier *= boundary_[i];
            }
        }
        return res;
    }

    value_type boundary_;
    std::remove_const_t<value_type> curr_;
};

template <typename IndexType>
bounds_iterator<IndexType> operator+(typename bounds_iterator<IndexType>::difference_type n,
                                     const bounds_iterator<IndexType>& rhs) noexcept
{
    return rhs + n;
}

namespace details
{
    template <typename Bounds>
    constexpr std::enable_if_t<
        std::is_same<typename Bounds::mapping_type, generalized_mapping_tag>::value,
        typename Bounds::index_type>
    make_stride(const Bounds& bnd) noexcept
    {
        return bnd.strides();
    }

    // Make a stride vector from bounds, assuming contiguous memory.
    template <typename Bounds>
    constexpr std::enable_if_t<
        std::is_same<typename Bounds::mapping_type, contiguous_mapping_tag>::value,
        typename Bounds::index_type>
    make_stride(const Bounds& bnd) noexcept
    {
        auto extents = bnd.index_bounds();
        typename Bounds::size_type stride[Bounds::rank] = {};

        stride[Bounds::rank - 1] = 1;
        for (size_t i = 1; i < Bounds::rank; ++i) {
            stride[Bounds::rank - i - 1] = stride[Bounds::rank - i] * extents[Bounds::rank - i];
        }
        return {stride};
    }

    template <typename BoundsSrc, typename BoundsDest>
    void verifyBoundsReshape(const BoundsSrc& src, const BoundsDest& dest)
    {
        static_assert(is_bounds<BoundsSrc>::value && is_bounds<BoundsDest>::value,
                      "The src type and dest type must be bounds");
        static_assert(std::is_same<typename BoundsSrc::mapping_type, contiguous_mapping_tag>::value,
                      "The source type must be a contiguous bounds");
        static_assert(BoundsDest::static_size == dynamic_range ||
                          BoundsSrc::static_size == dynamic_range ||
                          BoundsDest::static_size == BoundsSrc::static_size,
                      "The source bounds must have same size as dest bounds");
        Expects(src.size() == dest.size());
    }

} // namespace details

template <typename Span>
class contiguous_span_iterator;
template <typename Span>
class general_span_iterator;
enum class byte : std::uint8_t
{
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

template <typename ValueType, std::ptrdiff_t FirstDimension = dynamic_range,
          std::ptrdiff_t... RestDimensions>
class span;

template <typename ValueType, size_t Rank>
class strided_span;

namespace details
{
    template <typename T, typename = std::true_type>
    struct SpanTypeTraits
    {
        using value_type = T;
        using size_type = size_t;
    };

    template <typename Traits>
    struct SpanTypeTraits<Traits, typename std::is_reference<typename Traits::span_traits&>::type>
    {
        using value_type = typename Traits::span_traits::value_type;
        using size_type = typename Traits::span_traits::size_type;
    };

    template <typename T, std::ptrdiff_t... Ranks>
    struct SpanArrayTraits
    {
        using type = span<T, Ranks...>;
        using value_type = T;
        using bounds_type = static_bounds<Ranks...>;
        using pointer = T*;
        using reference = T&;
    };
    template <typename T, std::ptrdiff_t N, std::ptrdiff_t... Ranks>
    struct SpanArrayTraits<T[N], Ranks...> : SpanArrayTraits<T, Ranks..., N>
    {
    };

    template <typename BoundsType>
    BoundsType newBoundsHelperImpl(std::ptrdiff_t totalSize, std::true_type) // dynamic size
    {
        Expects(totalSize >= 0 && totalSize <= PTRDIFF_MAX);
        return BoundsType{totalSize};
    }
    template <typename BoundsType>
    BoundsType newBoundsHelperImpl(std::ptrdiff_t totalSize, std::false_type) // static size
    {
        Expects(BoundsType::static_size <= totalSize);
        return {};
    }
    template <typename BoundsType>
    BoundsType newBoundsHelper(std::ptrdiff_t totalSize)
    {
        static_assert(BoundsType::dynamic_rank <= 1, "dynamic rank must less or equal to 1");
        return newBoundsHelperImpl<BoundsType>(
            totalSize, std::integral_constant<bool, BoundsType::dynamic_rank == 1>());
    }

    struct Sep
    {
    };

    template <typename T, typename... Args>
    T static_as_span_helper(Sep, Args... args)
    {
        return T{narrow_cast<typename T::size_type>(args)...};
    }
    template <typename T, typename Arg, typename... Args>
    std::enable_if_t<
        !std::is_same<Arg, dim<dynamic_range>>::value && !std::is_same<Arg, Sep>::value, T>
        static_as_span_helper(Arg, Args... args)
    {
        return static_as_span_helper<T>(args...);
    }
    template <typename T, typename... Args>
    T static_as_span_helper(dim<dynamic_range> val, Args... args)
    {
        return static_as_span_helper<T>(args..., val.dvalue);
    }

    template <typename... Dimensions>
    struct static_as_span_static_bounds_helper
    {
        using type = static_bounds<(Dimensions::value)...>;
    };

    template <typename T>
    struct is_span_oracle : std::false_type
    {
    };

    template <typename ValueType, std::ptrdiff_t FirstDimension, std::ptrdiff_t... RestDimensions>
    struct is_span_oracle<span<ValueType, FirstDimension, RestDimensions...>> : std::true_type
    {
    };

    template <typename ValueType, std::ptrdiff_t Rank>
    struct is_span_oracle<strided_span<ValueType, Rank>> : std::true_type
    {
    };

    template <typename T>
    struct is_span : is_span_oracle<std::remove_cv_t<T>>
    {
    };
}

template <typename ValueType, std::ptrdiff_t FirstDimension, std::ptrdiff_t... RestDimensions>
class span
{
    // TODO do we still need this?
    template <typename ValueType2, std::ptrdiff_t FirstDimension2,
              std::ptrdiff_t... RestDimensions2>
    friend class span;

public:
    using bounds_type = static_bounds<FirstDimension, RestDimensions...>;
    static const size_t Rank = bounds_type::rank;
    using size_type = typename bounds_type::size_type;
    using index_type = typename bounds_type::index_type;
    using value_type = ValueType;
    using const_value_type = std::add_const_t<value_type>;
    using pointer = std::add_pointer_t<value_type>;
    using reference = std::add_lvalue_reference_t<value_type>;
    using iterator = contiguous_span_iterator<span>;
    using const_span = span<const_value_type, FirstDimension, RestDimensions...>;
    using const_iterator = contiguous_span_iterator<const_span>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using sliced_type =
        std::conditional_t<Rank == 1, value_type, span<value_type, RestDimensions...>>;

private:
    pointer data_;
    bounds_type bounds_;

    friend iterator;
    friend const_iterator;

public:
    // default constructor - same as constructing from nullptr_t
    constexpr span() noexcept : span(nullptr, bounds_type{})
    {
        static_assert(bounds_type::dynamic_rank != 0 ||
                          (bounds_type::dynamic_rank == 0 && bounds_type::static_size == 0),
                      "Default construction of span<T> only possible "
                      "for dynamic or fixed, zero-length spans.");
    }

    // construct from nullptr - get an empty span
    constexpr span(std::nullptr_t) noexcept : span(nullptr, bounds_type{})
    {
        static_assert(bounds_type::dynamic_rank != 0 ||
                          (bounds_type::dynamic_rank == 0 && bounds_type::static_size == 0),
                      "nullptr_t construction of span<T> only possible "
                      "for dynamic or fixed, zero-length spans.");
    }

    // construct from nullptr with size of 0 (helps with template function calls)
    template <class IntType, typename = std::enable_if_t<std::is_integral<IntType>::value>>
    constexpr span(std::nullptr_t, IntType size) noexcept : span(nullptr, bounds_type{})
    {
        static_assert(bounds_type::dynamic_rank != 0 ||
                          (bounds_type::dynamic_rank == 0 && bounds_type::static_size == 0),
                      "nullptr_t construction of span<T> only possible "
                      "for dynamic or fixed, zero-length spans.");
        Expects(size == 0);
    }

    // construct from a single element
    constexpr span(reference data) noexcept : span(&data, bounds_type{1})
    {
        static_assert(bounds_type::dynamic_rank > 0 || bounds_type::static_size == 0 ||
                          bounds_type::static_size == 1,
                      "Construction from a single element only possible "
                      "for dynamic or fixed spans of length 0 or 1.");
    }

    // prevent constructing from temporaries for single-elements
    constexpr span(value_type&&) = delete;

    // construct from pointer + length
    constexpr span(pointer ptr, size_type size) noexcept : span(ptr, bounds_type{size}) {}

    // construct from pointer + length - multidimensional
    constexpr span(pointer data, bounds_type bounds) noexcept : data_(data),
                                                                bounds_(std::move(bounds))
    {
        Expects((bounds_.size() > 0 && data != nullptr) || bounds_.size() == 0);
    }

    // construct from begin,end pointer pair
    template <typename Ptr,
              typename = std::enable_if_t<std::is_convertible<Ptr, pointer>::value &&
                                          details::LessThan<bounds_type::dynamic_rank, 2>::value>>
    constexpr span(pointer begin, Ptr end)
        : span(begin, details::newBoundsHelper<bounds_type>(static_cast<pointer>(end) - begin))
    {
        Expects(begin != nullptr && end != nullptr && begin <= static_cast<pointer>(end));
    }

    // construct from n-dimensions static array
    template <typename T, size_t N, typename Helper = details::SpanArrayTraits<T, N>>
    constexpr span(T (&arr)[N])
        : span(reinterpret_cast<pointer>(arr), bounds_type{typename Helper::bounds_type{}})
    {
        static_assert(
            std::is_convertible<typename Helper::value_type(*) [], value_type(*) []>::value,
            "Cannot convert from source type to target span type.");
        static_assert(std::is_convertible<typename Helper::bounds_type, bounds_type>::value,
                      "Cannot construct a span from an array with fewer elements.");
    }

    // construct from n-dimensions dynamic array (e.g. new int[m][4])
    // (precedence will be lower than the 1-dimension pointer)
    template <typename T, typename Helper = details::SpanArrayTraits<T, dynamic_range>>
    constexpr span(T* const& data, size_type size)
        : span(reinterpret_cast<pointer>(data), typename Helper::bounds_type{size})
    {
        static_assert(
            std::is_convertible<typename Helper::value_type(*) [], value_type(*) []>::value,
            "Cannot convert from source type to target span type.");
    }

    // construct from std::array
    template <typename T, size_t N>
    constexpr span(std::array<T, N>& arr) : span(arr.data(), bounds_type{static_bounds<N>{}})
    {
        static_assert(
            std::is_convertible<T(*) [], typename std::remove_const_t<value_type>(*) []>::value,
            "Cannot convert from source type to target span type.");
        static_assert(std::is_convertible<static_bounds<N>, bounds_type>::value,
                      "You cannot construct a span from a std::array of smaller size.");
    }

    // construct from const std::array
    template <typename T, size_t N>
    constexpr span(const std::array<std::remove_const_t<value_type>, N>& arr)
        : span(arr.data(), static_bounds<N>())
    {
        static_assert(std::is_convertible<T(*) [], std::remove_const_t<value_type>>::value,
                      "Cannot convert from source type to target span type.");
        static_assert(std::is_convertible<static_bounds<N>, bounds_type>::value,
                      "You cannot construct a span from a std::array of smaller size.");
    }

    // prevent constructing from temporary std::array
    template <typename T, size_t N>
    constexpr span(std::array<T, N>&& arr) = delete;

    // construct from containers
    // future: could use contiguous_iterator_traits to identify only contiguous containers
    // type-requirements: container must have .size(), operator[] which are value_type compatible
    template <typename Cont, typename DataType = typename Cont::value_type,
              typename = std::enable_if_t<
                  !details::is_span<Cont>::value &&
                  std::is_convertible<DataType (*)[], value_type (*)[]>::value &&
                  std::is_same<std::decay_t<decltype(std::declval<Cont>().size(),
                                                     *std::declval<Cont>().data())>,
                               DataType>::value>>
    constexpr span(Cont& cont)
        : span(static_cast<pointer>(cont.data()),
               details::newBoundsHelper<bounds_type>(narrow_cast<size_type>(cont.size())))
    {
    }

    // prevent constructing from temporary containers
    template <typename Cont, typename DataType = typename Cont::value_type,
              typename = std::enable_if_t<
                  !details::is_span<Cont>::value &&
                  std::is_convertible<DataType (*)[], value_type (*)[]>::value &&
                  std::is_same<std::decay_t<decltype(std::declval<Cont>().size(),
                                                     *std::declval<Cont>().data())>,
                               DataType>::value>>
    explicit constexpr span(Cont&& cont) = delete;

    // construct from a convertible span
    template <typename OtherValueType, std::ptrdiff_t... OtherDimensions,
              typename OtherBounds = static_bounds<OtherDimensions...>,
              typename = std::enable_if_t<std::is_convertible<OtherValueType, ValueType>::value &&
                                          std::is_convertible<OtherBounds, bounds_type>::value>>
    constexpr span(span<OtherValueType, OtherDimensions...> other) noexcept : data_(other.data_),
                                                                              bounds_(other.bounds_)
    {
    }

// trivial copy and move
#ifndef GSL_MSVC_NO_SUPPORT_FOR_MOVE_CTOR_DEFAULT
    constexpr span(span&&) = default;
#endif
    constexpr span(const span&) = default;

// trivial assignment
#ifndef GSL_MSVC_NO_SUPPORT_FOR_MOVE_CTOR_DEFAULT
    constexpr span& operator=(span&&) = default;
#endif
    constexpr span& operator=(const span&) = default;

    // first() - extract the first Count elements into a new span
    template <std::ptrdiff_t Count>
    constexpr span<ValueType, Count> first() const noexcept
    {
        static_assert(Count >= 0, "Count must be >= 0.");
        static_assert(bounds_type::static_size == dynamic_range ||
                          Count <= bounds_type::static_size,
                      "Count is out of bounds.");

        Expects(bounds_type::static_size != dynamic_range || Count <= this->size());
        return {this->data(), Count};
    }

    // first() - extract the first count elements into a new span
    constexpr span<ValueType, dynamic_range> first(size_type count) const noexcept
    {
        Expects(count >= 0 && count <= this->size());
        return {this->data(), count};
    }

    // last() - extract the last Count elements into a new span
    template <std::ptrdiff_t Count>
    constexpr span<ValueType, Count> last() const noexcept
    {
        static_assert(Count >= 0, "Count must be >= 0.");
        static_assert(bounds_type::static_size == dynamic_range ||
                          Count <= bounds_type::static_size,
                      "Count is out of bounds.");

        Expects(bounds_type::static_size != dynamic_range || Count <= this->size());
        return {this->data() + this->size() - Count, Count};
    }

    // last() - extract the last count elements into a new span
    constexpr span<ValueType, dynamic_range> last(size_type count) const noexcept
    {
        Expects(count >= 0 && count <= this->size());
        return {this->data() + this->size() - count, count};
    }

    // subspan() - create a subview of Count elements starting at Offset
    template <std::ptrdiff_t Offset, std::ptrdiff_t Count>
    constexpr span<ValueType, Count> subspan() const noexcept
    {
        static_assert(Count >= 0, "Count must be >= 0.");
        static_assert(Offset >= 0, "Offset must be >= 0.");
        static_assert(bounds_type::static_size == dynamic_range ||
                          ((Offset <= bounds_type::static_size) &&
                           Count <= bounds_type::static_size - Offset),
                      "You must describe a sub-range within bounds of the span.");

        Expects(bounds_type::static_size != dynamic_range ||
                (Offset <= this->size() && Count <= this->size() - Offset));
        return {this->data() + Offset, Count};
    }

    // subspan() - create a subview of count elements starting at offset
    // supplying dynamic_range for count will consume all available elements from offset
    constexpr span<ValueType, dynamic_range> subspan(size_type offset,
                                                     size_type count = dynamic_range) const noexcept
    {
        Expects((offset >= 0 && offset <= this->size()) &&
                (count == dynamic_range || (count <= this->size() - offset)));
        return {this->data() + offset, count == dynamic_range ? this->length() - offset : count};
    }

    // section - creates a non-contiguous, strided span from a contiguous one
    constexpr strided_span<ValueType, Rank> section(index_type origin, index_type extents) const
        noexcept
    {
        size_type size = this->bounds().total_size() - this->bounds().linearize(origin);
        return {&this->operator[](origin), size,
                strided_bounds<Rank>{extents, details::make_stride(bounds())}};
    }

    // length of the span in elements
    constexpr size_type size() const noexcept { return bounds_.size(); }

    // length of the span in elements
    constexpr size_type length() const noexcept { return this->size(); }

    // length of the span in bytes
    constexpr size_type size_bytes() const noexcept { return sizeof(value_type) * this->size(); }

    // length of the span in bytes
    constexpr size_type length_bytes() const noexcept { return this->size_bytes(); }

    constexpr bool empty() const noexcept { return this->size() == 0; }

    static constexpr std::size_t rank() { return Rank; }

    template <size_t Dim = 0>
    constexpr size_type extent() const noexcept
    {
        static_assert(Dim < Rank,
                      "Dimension should be less than rank (dimension count starts from 0).");
        return bounds_.template extent<Dim>();
    }

    template <typename IntType>
    constexpr size_type extent(IntType dim) const noexcept
    {
        return bounds_.extent(dim);
    }

    constexpr bounds_type bounds() const noexcept { return bounds_; }

    constexpr pointer data() const noexcept { return data_; }

    template <typename FirstIndex>
    constexpr reference operator()(FirstIndex index)
    {
        return this->operator[](narrow_cast<std::ptrdiff_t>(index));
    }

    template <typename FirstIndex, typename... OtherIndices>
    constexpr reference operator()(FirstIndex index, OtherIndices... indices)
    {
        index_type idx = {narrow_cast<std::ptrdiff_t>(index),
                          narrow_cast<std::ptrdiff_t>(indices...)};
        return this->operator[](idx);
    }

    constexpr reference operator[](const index_type& idx) const noexcept
    {
        return data_[bounds_.linearize(idx)];
    }

    template <bool Enabled = (Rank > 1), typename Ret = std::enable_if_t<Enabled, sliced_type>>
    constexpr Ret operator[](size_type idx) const noexcept
    {
        Expects(idx < bounds_.size()); // index is out of bounds of the array
        const size_type ridx = idx * bounds_.stride();

        // index is out of bounds of the underlying data
        Expects(ridx < bounds_.total_size());
        return Ret{data_ + ridx, bounds_.slice()};
    }

    constexpr iterator begin() const noexcept { return iterator{this, true}; }

    constexpr iterator end() const noexcept { return iterator{this, false}; }

    constexpr const_iterator cbegin() const noexcept
    {
        return const_iterator{reinterpret_cast<const const_span*>(this), true};
    }

    constexpr const_iterator cend() const noexcept
    {
        return const_iterator{reinterpret_cast<const const_span*>(this), false};
    }

    constexpr reverse_iterator rbegin() const noexcept { return reverse_iterator{end()}; }

    constexpr reverse_iterator rend() const noexcept { return reverse_iterator{begin()}; }

    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return const_reverse_iterator{cend()};
    }

    constexpr const_reverse_iterator crend() const noexcept
    {
        return const_reverse_iterator{cbegin()};
    }

    template <typename OtherValueType, std::ptrdiff_t... OtherDimensions,
              typename Dummy = std::enable_if_t<std::is_same<
                  std::remove_cv_t<value_type>, std::remove_cv_t<OtherValueType>>::value>>
    constexpr bool operator==(const span<OtherValueType, OtherDimensions...>& other) const noexcept
    {
        return bounds_.size() == other.bounds_.size() &&
               (data_ == other.data_ || std::equal(this->begin(), this->end(), other.begin()));
    }

    template <typename OtherValueType, std::ptrdiff_t... OtherDimensions,
              typename Dummy = std::enable_if_t<std::is_same<
                  std::remove_cv_t<value_type>, std::remove_cv_t<OtherValueType>>::value>>
    constexpr bool operator!=(const span<OtherValueType, OtherDimensions...>& other) const noexcept
    {
        return !(*this == other);
    }

    template <typename OtherValueType, std::ptrdiff_t... OtherDimensions,
              typename Dummy = std::enable_if_t<std::is_same<
                  std::remove_cv_t<value_type>, std::remove_cv_t<OtherValueType>>::value>>
    constexpr bool operator<(const span<OtherValueType, OtherDimensions...>& other) const noexcept
    {
        return std::lexicographical_compare(this->begin(), this->end(), other.begin(), other.end());
    }

    template <typename OtherValueType, std::ptrdiff_t... OtherDimensions,
              typename Dummy = std::enable_if_t<std::is_same<
                  std::remove_cv_t<value_type>, std::remove_cv_t<OtherValueType>>::value>>
    constexpr bool operator<=(const span<OtherValueType, OtherDimensions...>& other) const noexcept
    {
        return !(other < *this);
    }

    template <typename OtherValueType, std::ptrdiff_t... OtherDimensions,
              typename Dummy = std::enable_if_t<std::is_same<
                  std::remove_cv_t<value_type>, std::remove_cv_t<OtherValueType>>::value>>
    constexpr bool operator>(const span<OtherValueType, OtherDimensions...>& other) const noexcept
    {
        return (other < *this);
    }

    template <typename OtherValueType, std::ptrdiff_t... OtherDimensions,
              typename Dummy = std::enable_if_t<std::is_same<
                  std::remove_cv_t<value_type>, std::remove_cv_t<OtherValueType>>::value>>
    constexpr bool operator>=(const span<OtherValueType, OtherDimensions...>& other) const noexcept
    {
        return !(*this < other);
    }
};

//
// Free functions for manipulating spans
//

// reshape a span into a different dimensionality
// DimCount and Enabled here are workarounds for a bug in MSVC 2015
template <typename SpanType, typename... Dimensions2, size_t DimCount = sizeof...(Dimensions2),
          bool Enabled = (DimCount > 0), typename = std::enable_if_t<Enabled>>
constexpr span<typename SpanType::value_type, Dimensions2::value...> as_span(SpanType s,
                                                                             Dimensions2... dims)
{
    static_assert(details::is_span<SpanType>::value,
                  "Variadic as_span() is for reshaping existing spans.");
    using BoundsType =
        typename span<typename SpanType::value_type, (Dimensions2::value)...>::bounds_type;
    auto tobounds = details::static_as_span_helper<BoundsType>(dims..., details::Sep{});
    details::verifyBoundsReshape(s.bounds(), tobounds);
    return {s.data(), tobounds};
}

// convert a span<T> to a span<const byte>
template <typename U, std::ptrdiff_t... Dimensions>
span<const byte, dynamic_range> as_bytes(span<U, Dimensions...> s) noexcept
{
    static_assert(std::is_trivial<std::decay_t<U>>::value,
                  "The value_type of span must be a trivial type.");
    return {reinterpret_cast<const byte*>(s.data()), s.size_bytes()};
}

// convert a span<T> to a span<byte> (a writeable byte span)
// this is not currently a portable function that can be relied upon to work
// on all implementations. It should be considered an experimental extension
// to the standard GSL interface.
template <typename U, std::ptrdiff_t... Dimensions>
span<byte> as_writeable_bytes(span<U, Dimensions...> s) noexcept
{
    static_assert(std::is_trivial<std::decay_t<U>>::value,
                  "The value_type of span must be a trivial type.");
    return {reinterpret_cast<byte*>(s.data()), s.size_bytes()};
}

// convert a span<const byte> to a span<const T>
// this is not currently a portable function that can be relied upon to work
// on all implementations. It should be considered an experimental extension
// to the standard GSL interface.
template <typename U, std::ptrdiff_t... Dimensions>
constexpr auto as_span(span<const byte, Dimensions...> s) noexcept
    -> span<const U, static_cast<std::ptrdiff_t>(
                         span<const byte, Dimensions...>::bounds_type::static_size != dynamic_range
                             ? (static_cast<size_t>(
                                    span<const byte, Dimensions...>::bounds_type::static_size) /
                                sizeof(U))
                             : dynamic_range)>
{
    using ConstByteSpan = span<const byte, Dimensions...>;
    static_assert(
        std::is_trivial<std::decay_t<U>>::value &&
            (ConstByteSpan::bounds_type::static_size == dynamic_range ||
             ConstByteSpan::bounds_type::static_size % narrow_cast<std::ptrdiff_t>(sizeof(U)) == 0),
        "Target type must be a trivial type and its size must match the byte array size");

    Expects((s.size_bytes() % sizeof(U)) == 0 && (s.size_bytes() / sizeof(U)) < PTRDIFF_MAX);
    return {reinterpret_cast<const U*>(s.data()),
            s.size_bytes() / narrow_cast<std::ptrdiff_t>(sizeof(U))};
}

// convert a span<byte> to a span<T>
// this is not currently a portable function that can be relied upon to work
// on all implementations. It should be considered an experimental extension
// to the standard GSL interface.
template <typename U, std::ptrdiff_t... Dimensions>
constexpr auto as_span(span<byte, Dimensions...> s) noexcept -> span<
    U, narrow_cast<std::ptrdiff_t>(
           span<byte, Dimensions...>::bounds_type::static_size != dynamic_range
               ? static_cast<std::size_t>(span<byte, Dimensions...>::bounds_type::static_size) /
                     sizeof(U)
               : dynamic_range)>
{
    using ByteSpan = span<byte, Dimensions...>;
    static_assert(
        std::is_trivial<std::decay_t<U>>::value &&
            (ByteSpan::bounds_type::static_size == dynamic_range ||
             ByteSpan::bounds_type::static_size % static_cast<std::size_t>(sizeof(U)) == 0),
        "Target type must be a trivial type and its size must match the byte array size");

    Expects((s.size_bytes() % sizeof(U)) == 0);
    return {reinterpret_cast<U*>(s.data()),
            s.size_bytes() / narrow_cast<std::ptrdiff_t>(sizeof(U))};
}

template <typename T, std::ptrdiff_t... Dimensions>
constexpr auto as_span(T* const& ptr, dim<Dimensions>... args)
    -> span<std::remove_all_extents_t<T>, Dimensions...>
{
    return {reinterpret_cast<std::remove_all_extents_t<T>*>(ptr),
            details::static_as_span_helper<static_bounds<Dimensions...>>(args..., details::Sep{})};
}

template <typename T>
constexpr auto as_span(T* arr, std::ptrdiff_t len) ->
    typename details::SpanArrayTraits<T, dynamic_range>::type
{
    return {reinterpret_cast<std::remove_all_extents_t<T>*>(arr), len};
}

template <typename T, size_t N>
constexpr auto as_span(T (&arr)[N]) -> typename details::SpanArrayTraits<T, N>::type
{
    return {arr};
}

template <typename T, size_t N>
constexpr span<const T, N> as_span(const std::array<T, N>& arr)
{
    return {arr};
}

template <typename T, size_t N>
constexpr span<const T, N> as_span(const std::array<T, N>&&) = delete;

template <typename T, size_t N>
constexpr span<T, N> as_span(std::array<T, N>& arr)
{
    return {arr};
}

template <typename T>
constexpr span<T, dynamic_range> as_span(T* begin, T* end)
{
    return {begin, end};
}

template <typename Cont>
constexpr auto as_span(Cont& arr) -> std::enable_if_t<
    !details::is_span<std::decay_t<Cont>>::value,
    span<std::remove_reference_t<decltype(arr.size(), *arr.data())>, dynamic_range>>
{
    Expects(arr.size() < PTRDIFF_MAX);
    return {arr.data(), narrow_cast<std::ptrdiff_t>(arr.size())};
}

template <typename Cont>
constexpr auto as_span(Cont&& arr) -> std::enable_if_t<
    !details::is_span<std::decay_t<Cont>>::value,
    span<std::remove_reference_t<decltype(arr.size(), *arr.data())>, dynamic_range>> = delete;

// from basic_string which doesn't have nonconst .data() member like other contiguous containers
template <typename CharT, typename Traits, typename Allocator>
constexpr auto as_span(std::basic_string<CharT, Traits, Allocator>& str)
    -> span<CharT, dynamic_range>
{
    Expects(str.size() < PTRDIFF_MAX);
    return {&str[0], narrow_cast<std::ptrdiff_t>(str.size())};
}

// strided_span is an extension that is not strictly part of the GSL at this time.
// It is kept here while the multidimensional interface is still being defined.
template <typename ValueType, size_t Rank>
class strided_span
{
public:
    using bounds_type = strided_bounds<Rank>;
    using size_type = typename bounds_type::size_type;
    using index_type = typename bounds_type::index_type;
    using value_type = ValueType;
    using const_value_type = std::add_const_t<value_type>;
    using pointer = std::add_pointer_t<value_type>;
    using reference = std::add_lvalue_reference_t<value_type>;
    using iterator = general_span_iterator<strided_span>;
    using const_strided_span = strided_span<const_value_type, Rank>;
    using const_iterator = general_span_iterator<const_strided_span>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using sliced_type =
        std::conditional_t<Rank == 1, value_type, strided_span<value_type, Rank - 1>>;

private:
    pointer data_;
    bounds_type bounds_;

    friend iterator;
    friend const_iterator;
    template <typename OtherValueType, size_t OtherRank>
    friend class strided_span;

public:
    // from raw data
    constexpr strided_span(pointer ptr, size_type size, bounds_type bounds)
        : data_(ptr), bounds_(std::move(bounds))
    {
        Expects((bounds_.size() > 0 && ptr != nullptr) || bounds_.size() == 0);
        // Bounds cross data boundaries
        Expects(this->bounds().total_size() <= size);
        (void) size;
    }

    // from static array of size N
    template <size_type N>
    constexpr strided_span(value_type (&values)[N], bounds_type bounds)
        : strided_span(values, N, std::move(bounds))
    {
    }

    // from array view
    template <typename OtherValueType, std::ptrdiff_t... Dimensions,
              bool Enabled1 = (sizeof...(Dimensions) == Rank),
              bool Enabled2 = std::is_convertible<OtherValueType*, ValueType*>::value,
              typename Dummy = std::enable_if_t<Enabled1 && Enabled2>>
    constexpr strided_span(span<OtherValueType, Dimensions...> av, bounds_type bounds)
        : strided_span(av.data(), av.bounds().total_size(), std::move(bounds))
    {
    }

    // convertible
    template <typename OtherValueType, typename Dummy = std::enable_if_t<std::is_convertible<
                                           OtherValueType (*)[], value_type (*)[]>::value>>
    constexpr strided_span(const strided_span<OtherValueType, Rank>& other)
        : data_(other.data_), bounds_(other.bounds_)
    {
    }

    // convert from bytes
    template <typename OtherValueType>
    constexpr strided_span<
        typename std::enable_if<std::is_same<value_type, const byte>::value, OtherValueType>::type,
        Rank>
    as_strided_span() const
    {
        static_assert((sizeof(OtherValueType) >= sizeof(value_type)) &&
                          (sizeof(OtherValueType) % sizeof(value_type) == 0),
                      "OtherValueType should have a size to contain a multiple of ValueTypes");
        auto d = narrow_cast<size_type>(sizeof(OtherValueType) / sizeof(value_type));

        size_type size = this->bounds().total_size() / d;
        return {const_cast<OtherValueType*>(reinterpret_cast<const OtherValueType*>(this->data())), size,
                bounds_type{resize_extent(this->bounds().index_bounds(), d),
                            resize_stride(this->bounds().strides(), d)}};
    }

    constexpr strided_span section(index_type origin, index_type extents) const
    {
        size_type size = this->bounds().total_size() - this->bounds().linearize(origin);
        return {&this->operator[](origin), size,
                bounds_type{extents, details::make_stride(bounds())}};
    }

    constexpr reference operator[](const index_type& idx) const
    {
        return data_[bounds_.linearize(idx)];
    }

    template <bool Enabled = (Rank > 1), typename Ret = std::enable_if_t<Enabled, sliced_type>>
    constexpr Ret operator[](size_type idx) const
    {
        Expects(idx < bounds_.size()); // index is out of bounds of the array
        const size_type ridx = idx * bounds_.stride();

        // index is out of bounds of the underlying data
        Expects(ridx < bounds_.total_size());
        return {data_ + ridx, bounds_.slice().total_size(), bounds_.slice()};
    }

    constexpr bounds_type bounds() const noexcept { return bounds_; }

    template <size_t Dim = 0>
    constexpr size_type extent() const noexcept
    {
        static_assert(Dim < Rank,
                      "dimension should be less than Rank (dimension count starts from 0)");
        return bounds_.template extent<Dim>();
    }

    constexpr size_type size() const noexcept { return bounds_.size(); }

    constexpr pointer data() const noexcept { return data_; }

    constexpr explicit operator bool() const noexcept { return data_ != nullptr; }

    constexpr iterator begin() const { return iterator{this, true}; }

    constexpr iterator end() const { return iterator{this, false}; }

    constexpr const_iterator cbegin() const
    {
        return const_iterator{reinterpret_cast<const const_strided_span*>(this), true};
    }

    constexpr const_iterator cend() const
    {
        return const_iterator{reinterpret_cast<const const_strided_span*>(this), false};
    }

    constexpr reverse_iterator rbegin() const { return reverse_iterator{end()}; }

    constexpr reverse_iterator rend() const { return reverse_iterator{begin()}; }

    constexpr const_reverse_iterator crbegin() const { return const_reverse_iterator{cend()}; }

    constexpr const_reverse_iterator crend() const { return const_reverse_iterator{cbegin()}; }

    template <typename OtherValueType, std::ptrdiff_t OtherRank,
              typename Dummy = std::enable_if_t<std::is_same<
                  std::remove_cv_t<value_type>, std::remove_cv_t<OtherValueType>>::value>>
    constexpr bool operator==(const strided_span<OtherValueType, OtherRank>& other) const noexcept
    {
        return bounds_.size() == other.bounds_.size() &&
               (data_ == other.data_ || std::equal(this->begin(), this->end(), other.begin()));
    }

    template <typename OtherValueType, std::ptrdiff_t OtherRank,
              typename Dummy = std::enable_if_t<std::is_same<
                  std::remove_cv_t<value_type>, std::remove_cv_t<OtherValueType>>::value>>
    constexpr bool operator!=(const strided_span<OtherValueType, OtherRank>& other) const noexcept
    {
        return !(*this == other);
    }

    template <typename OtherValueType, std::ptrdiff_t OtherRank,
              typename Dummy = std::enable_if_t<std::is_same<
                  std::remove_cv_t<value_type>, std::remove_cv_t<OtherValueType>>::value>>
    constexpr bool operator<(const strided_span<OtherValueType, OtherRank>& other) const noexcept
    {
        return std::lexicographical_compare(this->begin(), this->end(), other.begin(), other.end());
    }

    template <typename OtherValueType, std::ptrdiff_t OtherRank,
              typename Dummy = std::enable_if_t<std::is_same<
                  std::remove_cv_t<value_type>, std::remove_cv_t<OtherValueType>>::value>>
    constexpr bool operator<=(const strided_span<OtherValueType, OtherRank>& other) const noexcept
    {
        return !(other < *this);
    }

    template <typename OtherValueType, std::ptrdiff_t OtherRank,
              typename Dummy = std::enable_if_t<std::is_same<
                  std::remove_cv_t<value_type>, std::remove_cv_t<OtherValueType>>::value>>
    constexpr bool operator>(const strided_span<OtherValueType, OtherRank>& other) const noexcept
    {
        return (other < *this);
    }

    template <typename OtherValueType, std::ptrdiff_t OtherRank,
              typename Dummy = std::enable_if_t<std::is_same<
                  std::remove_cv_t<value_type>, std::remove_cv_t<OtherValueType>>::value>>
    constexpr bool operator>=(const strided_span<OtherValueType, OtherRank>& other) const noexcept
    {
        return !(*this < other);
    }

private:
    static index_type resize_extent(const index_type& extent, std::ptrdiff_t d)
    {
        // The last dimension of the array needs to contain a multiple of new type elements
        Expects(extent[Rank - 1] >= d && (extent[Rank - 1] % d == 0));

        index_type ret = extent;
        ret[Rank - 1] /= d;

        return ret;
    }

    template <bool Enabled = (Rank == 1), typename Dummy = std::enable_if_t<Enabled>>
    static index_type resize_stride(const index_type& strides, std::ptrdiff_t, void* = 0)
    {
        // Only strided arrays with regular strides can be resized
        Expects(strides[Rank - 1] == 1);

        return strides;
    }

    template <bool Enabled = (Rank > 1), typename Dummy = std::enable_if_t<Enabled>>
    static index_type resize_stride(const index_type& strides, std::ptrdiff_t d)
    {
        // Only strided arrays with regular strides can be resized
        Expects(strides[Rank - 1] == 1);
        // The strides must have contiguous chunks of
        // memory that can contain a multiple of new type elements
        Expects(strides[Rank - 2] >= d && (strides[Rank - 2] % d == 0));

        for (size_t i = Rank - 1; i > 0; --i) {
            // Only strided arrays with regular strides can be resized
            Expects((strides[i - 1] >= strides[i]) && (strides[i - 1] % strides[i] == 0));
        }

        index_type ret = strides / d;
        ret[Rank - 1] = 1;

        return ret;
    }
};

template <class Span>
class contiguous_span_iterator
    : public std::iterator<std::random_access_iterator_tag, typename Span::value_type>
{
    using Base = std::iterator<std::random_access_iterator_tag, typename Span::value_type>;

public:
    using typename Base::reference;
    using typename Base::pointer;
    using typename Base::difference_type;

private:
    template <typename ValueType, std::ptrdiff_t FirstDimension, std::ptrdiff_t... RestDimensions>
    friend class span;

    pointer data_;
    const Span* m_validator;
    void validateThis() const
    {
        // iterator is out of range of the array
        Expects(data_ >= m_validator->data_ && data_ < m_validator->data_ + m_validator->size());
    }
    contiguous_span_iterator(const Span* container, bool isbegin)
        : data_(isbegin ? container->data_ : container->data_ + container->size())
        , m_validator(container)
    {
    }

public:
    reference operator*() const noexcept
    {
        validateThis();
        return *data_;
    }
    pointer operator->() const noexcept
    {
        validateThis();
        return data_;
    }
    contiguous_span_iterator& operator++() noexcept
    {
        ++data_;
        return *this;
    }
    contiguous_span_iterator operator++(int) noexcept
    {
        auto ret = *this;
        ++(*this);
        return ret;
    }
    contiguous_span_iterator& operator--() noexcept
    {
        --data_;
        return *this;
    }
    contiguous_span_iterator operator--(int) noexcept
    {
        auto ret = *this;
        --(*this);
        return ret;
    }
    contiguous_span_iterator operator+(difference_type n) const noexcept
    {
        contiguous_span_iterator ret{*this};
        return ret += n;
    }
    contiguous_span_iterator& operator+=(difference_type n) noexcept
    {
        data_ += n;
        return *this;
    }
    contiguous_span_iterator operator-(difference_type n) const noexcept
    {
        contiguous_span_iterator ret{*this};
        return ret -= n;
    }
    contiguous_span_iterator& operator-=(difference_type n) noexcept { return * this += -n; }
    difference_type operator-(const contiguous_span_iterator& rhs) const noexcept
    {
        Expects(m_validator == rhs.m_validator);
        return data_ - rhs.data_;
    }
    reference operator[](difference_type n) const noexcept { return *(*this + n); }
    bool operator==(const contiguous_span_iterator& rhs) const noexcept
    {
        Expects(m_validator == rhs.m_validator);
        return data_ == rhs.data_;
    }
    bool operator!=(const contiguous_span_iterator& rhs) const noexcept { return !(*this == rhs); }
    bool operator<(const contiguous_span_iterator& rhs) const noexcept
    {
        Expects(m_validator == rhs.m_validator);
        return data_ < rhs.data_;
    }
    bool operator<=(const contiguous_span_iterator& rhs) const noexcept { return !(rhs < *this); }
    bool operator>(const contiguous_span_iterator& rhs) const noexcept { return rhs < *this; }
    bool operator>=(const contiguous_span_iterator& rhs) const noexcept { return !(rhs > *this); }
    void swap(contiguous_span_iterator& rhs) noexcept
    {
        std::swap(data_, rhs.data_);
        std::swap(m_validator, rhs.m_validator);
    }
};

template <typename Span>
contiguous_span_iterator<Span> operator+(typename contiguous_span_iterator<Span>::difference_type n,
                                         const contiguous_span_iterator<Span>& rhs) noexcept
{
    return rhs + n;
}

template <typename Span>
class general_span_iterator
    : public std::iterator<std::random_access_iterator_tag, typename Span::value_type>
{
    using Base = std::iterator<std::random_access_iterator_tag, typename Span::value_type>;

public:
    using typename Base::reference;
    using typename Base::pointer;
    using typename Base::difference_type;
    using typename Base::value_type;

private:
    template <typename ValueType, size_t Rank>
    friend class strided_span;

    const Span* m_container;
    typename Span::bounds_type::iterator m_itr;
    general_span_iterator(const Span* container, bool isbegin)
        : m_container(container)
        , m_itr(isbegin ? m_container->bounds().begin() : m_container->bounds().end())
    {
    }

public:
    reference operator*() noexcept { return (*m_container)[*m_itr]; }
    pointer operator->() noexcept { return &(*m_container)[*m_itr]; }
    general_span_iterator& operator++() noexcept
    {
        ++m_itr;
        return *this;
    }
    general_span_iterator operator++(int) noexcept
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
    general_span_iterator operator--(int) noexcept
    {
        auto ret = *this;
        --(*this);
        return ret;
    }
    general_span_iterator operator+(difference_type n) const noexcept
    {
        general_span_iterator ret{*this};
        return ret += n;
    }
    general_span_iterator& operator+=(difference_type n) noexcept
    {
        m_itr += n;
        return *this;
    }
    general_span_iterator operator-(difference_type n) const noexcept
    {
        general_span_iterator ret{*this};
        return ret -= n;
    }
    general_span_iterator& operator-=(difference_type n) noexcept { return * this += -n; }
    difference_type operator-(const general_span_iterator& rhs) const noexcept
    {
        Expects(m_container == rhs.m_container);
        return m_itr - rhs.m_itr;
    }
    value_type operator[](difference_type n) const noexcept
    {
        return (*m_container)[m_itr[n]];
        ;
    }
    bool operator==(const general_span_iterator& rhs) const noexcept
    {
        Expects(m_container == rhs.m_container);
        return m_itr == rhs.m_itr;
    }
    bool operator!=(const general_span_iterator& rhs) const noexcept { return !(*this == rhs); }
    bool operator<(const general_span_iterator& rhs) const noexcept
    {
        Expects(m_container == rhs.m_container);
        return m_itr < rhs.m_itr;
    }
    bool operator<=(const general_span_iterator& rhs) const noexcept { return !(rhs < *this); }
    bool operator>(const general_span_iterator& rhs) const noexcept { return rhs < *this; }
    bool operator>=(const general_span_iterator& rhs) const noexcept { return !(rhs > *this); }
    void swap(general_span_iterator& rhs) noexcept
    {
        std::swap(m_itr, rhs.m_itr);
        std::swap(m_container, rhs.m_container);
    }
};

template <typename Span>
general_span_iterator<Span> operator+(typename general_span_iterator<Span>::difference_type n,
                                      const general_span_iterator<Span>& rhs) noexcept
{
    return rhs + n;
}

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
