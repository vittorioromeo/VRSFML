#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/FromCharsResult.hpp" // IWYU pragma: export
#include "SFML/Base/Radix.hpp"
#include "SFML/Base/Trait/IsIntegral.hpp"
#include "SFML/Base/Trait/IsUnsigned.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Parse an unsigned integer from `[first, last)` in the given `radix`.
///
/// Symmetric counterpart to `toCharsRadix`: rejects a leading sign (a hex /
/// oct / binary literal has no sign in C-protocol contexts), stops at the
/// first character that is not a valid digit for the chosen radix, and
/// signals overflow via `FromCharsError::ResultOutOfRange`.
///
/// Hex parsing accepts both lowercase (`a`..`f`) and uppercase (`A`..`F`)
/// digits; the choice is irrelevant for octal and binary.
///
/// Constrained to **unsigned** types so the "treat as bit pattern" model is
/// unambiguous. To parse a signed value as decimal use the existing
/// `fromChars` overload.
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard]] constexpr FromCharsResult fromCharsRadix(const char* first, const char* const last, T& value, const Radix radix)
    requires(isIntegral<T> && isUnsigned<T>)
{
    if (first == last)
        return {first, FromCharsError::InvalidArgument};

    const auto base = static_cast<T>(radix);

    // Largest value of T (all bits set, since T is unsigned).
    constexpr auto limit = static_cast<T>(~T(0));

    T    result   = 0;
    bool anyDigit = false;

    while (first != last)
    {
        const char c = *first;

        T digit = 0;
        if (c >= '0' && c <= '9')
            digit = static_cast<T>(c - '0');
        else if (c >= 'a' && c <= 'f')
            digit = static_cast<T>(c - 'a' + 10);
        else if (c >= 'A' && c <= 'F')
            digit = static_cast<T>(c - 'A' + 10);
        else
            break; // First non-digit character ends the parse.

        // Reject digits outside the chosen radix (e.g. '8' under `Radix::Oct`,
        // 'a' under `Radix::Bin`). This is a parse stop, not an error: matches
        // `fromChars` decimal semantics on the first non-digit byte.
        if (digit >= base)
            break;

        // Overflow check, mirroring the decimal `fromChars`.
        if (result > limit / base || (result == limit / base && digit > limit % base))
            return {first, FromCharsError::ResultOutOfRange};

        result = static_cast<T>(result * base + digit);
        ++first;
        anyDigit = true;
    }

    if (!anyDigit)
        return {first, FromCharsError::InvalidArgument};

    value = result;
    return {first, FromCharsError::None};
}

} // namespace sf::base
