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

#ifndef GSL_BYTE_H
#define GSL_BYTE_H

#include <type_traits>

#ifdef _MSC_VER
 #pragma warning(push)

 // don't warn about function style casts in byte related operators
 #pragma warning(disable : 26493)

 // MSVC 2013 workarounds
 #if _MSC_VER <= 1800
  // constexpr is not understood
  #pragma push_macro("constexpr")
  #define constexpr /*constexpr*/

  // noexcept is not understood
  #pragma push_macro("noexcept")
  #define noexcept /*noexcept*/
 #endif // _MSC_VER <= 1800
#endif // _MSC_VER

namespace gsl
{
// This is a simple definition for now that allows
// use of byte within span<> to be standards-compliant
enum class byte : unsigned char
{
};

template <class IntegerType, class = std::enable_if_t<std::is_integral<IntegerType>::value>>
inline constexpr byte& operator<<=(byte& b, IntegerType shift) noexcept
{
    return b = byte(static_cast<unsigned char>(b) << shift);
}

template <class IntegerType, class = std::enable_if_t<std::is_integral<IntegerType>::value>>
inline constexpr byte operator<<(byte b, IntegerType shift) noexcept
{
    return byte(static_cast<unsigned char>(b) << shift);
}

template <class IntegerType, class = std::enable_if_t<std::is_integral<IntegerType>::value>>
inline constexpr byte& operator>>=(byte& b, IntegerType shift) noexcept
{
    return b = byte(static_cast<unsigned char>(b) >> shift);
}

template <class IntegerType, class = std::enable_if_t<std::is_integral<IntegerType>::value>>
inline constexpr byte operator>>(byte b, IntegerType shift) noexcept
{
    return byte(static_cast<unsigned char>(b) >> shift);
}

inline constexpr byte& operator|=(byte& l, byte r) noexcept
{
    return l = byte(static_cast<unsigned char>(l) | static_cast<unsigned char>(r));
}

inline constexpr byte operator|(byte l, byte r) noexcept
{
    return byte(static_cast<unsigned char>(l) | static_cast<unsigned char>(r));
}

inline constexpr byte& operator&=(byte& l, byte r) noexcept
{
    return l = byte(static_cast<unsigned char>(l) & static_cast<unsigned char>(r));
}

inline constexpr byte operator&(byte l, byte r) noexcept
{
    return byte(static_cast<unsigned char>(l) & static_cast<unsigned char>(r));
}

inline constexpr byte& operator^=(byte& l, byte r) noexcept
{
    return l = byte(static_cast<unsigned char>(l) ^ static_cast<unsigned char>(r));
}

inline constexpr byte operator^(byte l, byte r) noexcept
{
    return byte(static_cast<unsigned char>(l) ^ static_cast<unsigned char>(r));
}

inline constexpr byte operator~(byte b) noexcept { return byte(~static_cast<unsigned char>(b)); }

template <class IntegerType, class = std::enable_if_t<std::is_integral<IntegerType>::value>>
inline constexpr IntegerType to_integer(byte b) noexcept
{
    return static_cast<IntegerType>(b);
}

template<bool E, typename T>
inline constexpr byte to_byte_impl(T t) noexcept
{
    static_assert(
        E,
        "gsl::to_byte(t) must be provided an unsigned char, otherwise data loss may occur. "
        "If you are calling to_byte with an integer contant use: gsl::to_byte<t>() version."
    );
    return static_cast<byte>(t);
}
template<>
inline constexpr byte to_byte_impl<true, unsigned char>(unsigned char t) noexcept
{
     return byte(t);
}

template<typename T>
inline constexpr byte to_byte(T t) noexcept
{
     return to_byte_impl<std::is_same<T, unsigned char>::value, T>(t);
}

template <int I>
inline constexpr byte to_byte() noexcept
{
    static_assert(I >= 0 && I <= 255, "gsl::byte only has 8 bits of storage, values must be in range 0-255");
    return static_cast<byte>(I);
}

} // namespace gsl

#ifdef _MSC_VER
 #if _MSC_VER <= 1800
  #undef constexpr
  #pragma pop_macro("constexpr")

  #undef noexcept
  #pragma pop_macro("noexcept")
 #endif // _MSC_VER <= 1800

 #pragma warning(pop)
#endif // _MSC_VER

#endif // GSL_BYTE_H
