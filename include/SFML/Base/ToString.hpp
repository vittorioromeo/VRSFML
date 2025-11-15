#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/String.hpp"
#include "SFML/Base/ToChars.hpp"
#include "SFML/Base/Trait/IsFloatingPoint.hpp"
#include "SFML/Base/Trait/IsIntegral.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Converts a numeric value to a string.
///
/// This function serves as a replacement for `std::to_string`,
/// using the SFML/Base ecosystem.
///
/// \param value The numeric value to convert.
///
/// \return A string representing the value.
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard]] String toString(const T value)
    requires(isIntegral<T> || isFloatingPoint<T>)
{
    // A buffer large enough for 64-bit integers and floating point numbers.
    // long long: -9,223,372,036,854,775,808 (19 digits + sign)
    // double: sign + integer part + '.' + fractional part (e.g., 9 digits) + null

    char buffer[64];

    const char* const end = toChars(buffer, buffer + 64, value);
    return String{buffer, static_cast<SizeT>(end - buffer)};
}

} // namespace sf::base
