#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Math/Round.hpp"
#include "SFML/Base/Trait/IsFloatingPoint.hpp"
#include "SFML/Base/Trait/IsIntegral.hpp"
#include "SFML/Base/Trait/IsUnsigned.hpp"
#include "SFML/Base/Trait/MakeUnsigned.hpp"


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
/// \brief Precomputed powers of 10 for performance
///
/// Supports up to 10 decimal places.
///
////////////////////////////////////////////////////////////
inline constexpr const long long powersOf10[] = {
    1ll,
    10ll,
    100ll,
    1000ll,
    10'000ll,
    100'000ll,
    1'000'000ll,
    10'000'000ll,
    100'000'000ll,
    1'000'000'000ll,
    10'000'000'000ll,
};


////////////////////////////////////////////////////////////
/// \brief Reverses the character sequence in the given range.
///
////////////////////////////////////////////////////////////
[[gnu::always_inline]] inline constexpr void reverseChars(char* first, char* last)
{
    // `last` is one-past-the-end, so decrement it to point to the last character.
    --last;

    while (first < last)
    {
        const char tmp = *first;
        *first++       = *last;
        *last--        = tmp;
    }
}


////////////////////////////////////////////////////////////
/// \brief Converts an unsigned integral value to a character string.
///
/// \param first Pointer to the beginning of the character buffer.
/// \param value The unsigned value to convert.
///
/// \return Pointer to one past the last character written.
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten]] constexpr char* unsignedToChars(char* const first, T value)
{
    char* p = first;

    if (value == T{0})
    {
        *p++ = '0';
        return p;
    }

    while (value > T{0})
    {
        *p++ = '0' + (value % 10);
        value /= 10;
    }

    reverseChars(first, p);
    return p;
}

} // namespace sf::base::priv


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Converts an integral value to a character string.
///
/// This is an internal helper that mimics std::to_chars.
///
/// \param first Pointer to the beginning of the character buffer.
/// \param last  Pointer to the end of the character buffer (unused, kept for API consistency).
/// \param value The integral value to convert.
/// \return Pointer to one past the last character written.
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard]] constexpr char* toChars(char* first, [[maybe_unused]] const char* const last, const T value)
    requires isIntegral<T>
{
    if constexpr (isUnsigned<T>)
    {
        return priv::unsignedToChars(first, value);
    }
    else
    {
        // Use the unsigned counterpart for calculations to correctly handle T_MIN
        const auto uValue = static_cast<MakeUnsigned<T>>(value);

        if (value < 0)
        {
            *first++ = '-';

            // Unsigned arithmetic wraps around, so `-uValue` is well-defined
            // and gives the correct positive magnitude for all negative values, including T_MIN.
            return priv::unsignedToChars(first, -uValue);
        }

        return priv::unsignedToChars(first, uValue);
    }
}


////////////////////////////////////////////////////////////
/// \brief Converts a floating-point value to a character string.
///
/// This is a simple implementation with fixed precision, similar to std::to_string.
///
/// \param first Pointer to the beginning of the character buffer.
/// \param last  Pointer to the end of the character buffer (unused, kept for API consistency).
/// \param value The floating-point value to convert.
/// \param precision The number of digits after the decimal point.
/// \return Pointer to one past the last character written.
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard]] constexpr char* toChars(char* first, [[maybe_unused]] const char* const last, T value, const int precision = 6)
    requires isFloatingPoint<T>
{
    char* p = first;

    if (value < 0)
    {
        *p++  = '-';
        value = -value;
    }

    // Convert integer part (note: subject to long long's range limitations)
    const auto intPart = static_cast<long long>(value);

    p    = priv::unsignedToChars(p, intPart);
    *p++ = '.';

    // Calculate fractional part as an integer
    const T         fracPart       = value - static_cast<T>(intPart);
    const long long fracMultiplier = (precision > 0 && precision < 11) ? priv::powersOf10[precision] : 1;
    const auto      fracAsInt      = static_cast<long long>(round(fracPart * static_cast<T>(fracMultiplier)));

    char fracBuffer[20]; // A buffer for the fractional part (`long long` max is 19 digits)

    const char* const fracEnd    = priv::unsignedToChars(fracBuffer, fracAsInt);
    const int         fracLength = static_cast<int>(fracEnd - fracBuffer);

    // 1. Add leading zeros to the main buffer if needed. This is much faster than memmove.
    for (int i = 0; i < precision - fracLength; ++i)
        *p++ = '0';

    // 2. Copy the generated fractional digits from the temp buffer.
    for (char* c = fracBuffer; c < fracEnd; ++c)
        *p++ = *c;

    return p;
}

} // namespace sf::base
