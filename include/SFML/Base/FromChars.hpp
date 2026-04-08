#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Trait/IsFloatingPoint.hpp"
#include "SFML/Base/Trait/IsIntegral.hpp"
#include "SFML/Base/Trait/IsUnsigned.hpp"
#include "SFML/Base/Trait/MakeUnsigned.hpp"


namespace sf::base::priv
{
//////////////////////////////////////////////////////////////
[[nodiscard]] inline constexpr bool isDigit(const char c)
{
    return c >= '0' && c <= '9';
}


//////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard]] inline consteval T maxIntegral() noexcept
{
    if constexpr (SFML_BASE_IS_UNSIGNED(T))
    {
        // For unsigned types, max is all bits set to 1.
        return static_cast<T>(~T(0));
    }
    else
    {
        // For signed types, max is the unsigned max shifted right by one bit.
        // This is a portable way to get the max signed value (e.g., 0111...111).
        using UnsignedT = SFML_BASE_MAKE_UNSIGNED(T);
        return static_cast<T>((static_cast<UnsignedT>(~UnsignedT(0))) >> 1);
    }
}

} // namespace sf::base::priv


namespace sf::base
{
//////////////////////////////////////////////////////////////
/// \brief Outcome of a `fromChars` parse attempt
///
//////////////////////////////////////////////////////////////
enum class [[nodiscard]] FromCharsError
{
    None,            //!< Successful parse
    InvalidArgument, //!< No digits found, or sign without digits
    ResultOutOfRange //!< Value would overflow the destination type
};


//////////////////////////////////////////////////////////////
/// \brief Result type returned from `fromChars`, mirroring `std::from_chars_result`
///
//////////////////////////////////////////////////////////////
struct [[nodiscard]] FromCharsResult
{
    const char*    ptr; //!< One past the last character successfully consumed
    FromCharsError ec;  //!< Error code (`None` on success)
};


//////////////////////////////////////////////////////////////
/// \brief Parse an integer from `[first, last)` into `value`
///
/// Stricter than `strtol`: only base-10 digits and an optional leading
/// sign are accepted. Detects overflow before it happens and signals
/// it via `FromCharsError::ResultOutOfRange`.
///
//////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard]] FromCharsResult fromChars(const char* first, const char* const last, T& value)
    requires isIntegral<T>
{
    if (first == last)
        return {first, FromCharsError::InvalidArgument};

    bool isNegative = false;
    if (*first == '-')
    {
        isNegative = true;
        ++first;
    }
    else if (*first == '+')
    {
        ++first;
    }

    if constexpr (SFML_BASE_IS_UNSIGNED(T))
    {
        if (isNegative)
            return {first, FromCharsError::InvalidArgument};
    }

    if (first == last || !priv::isDigit(*first))
    {
        return {first, FromCharsError::InvalidArgument};
    }

    T result = 0;

    constexpr auto limit = priv::maxIntegral<T>();

    while (first != last && priv::isDigit(*first))
    {
        const int digit = *first - '0';

        // Check for overflow before multiplication
        if (result > limit / 10 || (result == limit / 10 && static_cast<T>(digit) > limit % 10))
            return {first, FromCharsError::ResultOutOfRange};

        result = static_cast<T>(result * 10 + static_cast<T>(digit));
        ++first;
    }

    if constexpr (!SFML_BASE_IS_UNSIGNED(T))
    {
        value = isNegative ? -result : result;
    }
    else
    {
        value = result;
    }

    return {first, FromCharsError::None}; // Success
}


//////////////////////////////////////////////////////////////
/// \brief Parse a floating-point number from `[first, last)` into `value`
///
/// Accepts an optional sign, an integer part, and an optional fractional
/// part. Exponents (`e`/`E`) are not currently supported. Uses
/// `long double` internally to retain precision before narrowing the
/// result back into `T`.
///
//////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard]] FromCharsResult fromChars(const char* first, const char* const last, T& value)
    requires isFloatingPoint<T>
{
    if (first == last)
        return {first, FromCharsError::InvalidArgument};

    const char* initialFirst = first;

    bool isNegative = false;

    if (*first == '-')
    {
        isNegative = true;
        ++first;
    }
    else if (*first == '+')
    {
        ++first;
    }

    // Use long double for intermediate calculations to maximize precision.
    long double result          = 0.0l;
    bool        anyDigitsParsed = false;

    // Parse whole number part
    while (first != last && priv::isDigit(*first))
    {
        result = result * 10.0l + (*first - '0');
        ++first;
        anyDigitsParsed = true;
    }

    // Parse fractional part
    if (first != last && *first == '.')
    {
        ++first;
        long double power = 0.1l;
        while (first != last && priv::isDigit(*first))
        {
            result += (*first - '0') * power;
            power /= 10.0l;
            ++first;
            anyDigitsParsed = true;
        }
    }

    // If no digits were parsed at all, it's an error.
    if (!anyDigitsParsed)
        return {initialFirst, FromCharsError::InvalidArgument};

    if (isNegative)
        result = -result;

    value = static_cast<T>(result);

    return {first, FromCharsError::None}; // Success
}

} // namespace sf::base
