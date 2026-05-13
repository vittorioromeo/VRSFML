#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtin/IsInf.hpp"
#include "SFML/Base/Builtin/IsNan.hpp"
#include "SFML/Base/Builtin/Signbit.hpp"
#include "SFML/Base/Math/Rint.hpp"
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
/// \brief Write the decimal representation of an unsigned `value` into `[first, last)`
///
/// \return Pointer one past the last written character, or `nullptr` if the buffer is too small
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten]] constexpr char* unsignedToChars(char* const first, const char* const last, T value)
{
    // `do-while` covers `value == 0` without a separate branch: we always emit
    // at least one digit, then keep dividing while bits remain.
    char* p = first;

    do
    {
        if (p >= last)
            return nullptr; // Buffer too small

        *p++ = '0' + static_cast<char>(value % 10);
        value /= 10;
    } while (value > T{0});

    reverseChars(first, p);
    return p;
}

} // namespace sf::base::priv


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Write the decimal representation of an integral `value` into `[first, last)` (mimics `std::to_chars`)
///
/// \return Pointer one past the last written character, or `nullptr` if the buffer is too small
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard]] constexpr char* toChars(char* first, const char* const last, const T value)
    requires isIntegral<T>
{
    if constexpr (isUnsigned<T>)
    {
        return priv::unsignedToChars(first, last, value);
    }
    else
    {
        // Use the unsigned counterpart for calculations to correctly handle T_MIN
        using UT          = MakeUnsigned<T>;
        const auto uValue = static_cast<UT>(value);

        if (value < 0)
        {
            if (first >= last)
                return nullptr; // Buffer too small

            *first++ = '-';

            // The cast back to `UT` after `-uValue` is required for types narrower
            // than `int`: integer promotion turns `-uValue` into a (signed) `int`,
            // defeating the wrap-around trick and producing a negative value that
            // `unsignedToChars` would silently print as nothing.
            return priv::unsignedToChars(first, last, static_cast<UT>(-uValue));
        }

        return priv::unsignedToChars(first, last, uValue);
    }
}


////////////////////////////////////////////////////////////
/// \brief Write a floating-point `value` into `[first, last)` with fixed `precision`.
///
/// Matches `std::to_chars(..., chars_format::fixed, precision)` for finite values
/// in the supported precision range. Special-value handling: `NaN` is written as
/// `"nan"` (no sign), infinities as `"inf"` / `"-inf"`, negative zero preserves
/// its sign (`"-0.00"`). Rounding uses the active FPU rounding mode (default:
/// round-half-to-even, matching IEEE-754 and `std::to_chars`).
///
/// \pre `precision >= 0 && precision <= 10`.
/// \pre `value` is finite when compiled with `-ffinite-math-only` (or
///      `-ffast-math`, which implies it). Under that flag the compiler may
///      legally fold `__builtin_isnan` / `__builtin_isinf` to `false`, so
///      NaN/inf inputs would silently produce garbage output.
///
/// \return Pointer one past the last written character, or `nullptr` on:
///         - buffer too small,
///         - `value` outside the `long long`-representable range
///           (≈ `±9.2e18`, scaled by `10^precision`), in which case the caller
///           should pre-scale or use a different formatter.
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard]] constexpr char* toChars(char* first, const char* const last, T value, const int precision = 6)
    requires isFloatingPoint<T>
{
    SFML_BASE_ASSERT(precision >= 0 && precision <= 10);

    // Promote narrow floats to `double` so the internal `value * 10^precision`
    // multiplication preserves all the input's bits. `float`'s 24-bit mantissa
    // fits losslessly into `double`'s 53-bit mantissa, so the cast is exact.
    // Without this, e.g. `0.1f` at precision 10 would print "0.1000000000"
    // instead of the true "0.1000000015".
    if constexpr (sizeof(T) < sizeof(double))
        return toChars(first, last, static_cast<double>(value), precision);

    char* p = first;

    // NaN: emit "nan" with no sign (matches `std::to_chars` and IEEE-754).
    // Under `-ffinite-math-only`, this branch folds to dead code: that's
    // intentional -- passing NaN under that flag is out of contract (see
    // precondition in the doc).
    if (SFML_BASE_ISNAN(value)) [[unlikely]]
    {
        if (last - p < 3)
            return nullptr;

        *p++ = 'n';
        *p++ = 'a';
        *p++ = 'n';
        return p;
    }

    // Sign via signbit so `-0.0` keeps its sign (matches `std::to_chars`).
    if (SFML_BASE_SIGNBIT(value))
    {
        if (p >= last)
            return nullptr;

        *p++  = '-';
        value = -value;
    }

    // Infinity: emit "inf" after any sign already written. Same out-of-contract
    // status as NaN under `-ffinite-math-only`.
    if (SFML_BASE_ISINF(value)) [[unlikely]]
    {
        if (last - p < 3)
            return nullptr;

        *p++ = 'i';
        *p++ = 'n';
        *p++ = 'f';
        return p;
    }

    // Out-of-range guard: the integer-part conversion casts to `long long`,
    // which is UB for values outside its range. Pick a threshold safely
    // below `LLONG_MAX` (= 9.223e18) so that rounding doesn't push us over.
    constexpr T safeLLongUpper = static_cast<T>(9'000'000'000'000'000'000LL);

    if (precision == 0)
    {
        if (value > safeLLongUpper) [[unlikely]]
            return nullptr;

        const auto roundedAsInt = static_cast<long long>(base::rint(value));
        return priv::unsignedToChars(p, last, roundedAsInt);
    }

    const long long multiplier = priv::powersOf10[precision];
    const T         scaled     = value * static_cast<T>(multiplier);

    if (scaled > safeLLongUpper) [[unlikely]]
        return nullptr;

    // `value` is non-negative at this point (signbit branch already negated it)
    // and `scaled <= 9e18 < LLONG_MAX`, so signed cast is well-defined.
    const auto roundedScaledValue = static_cast<long long>(base::rint(scaled));
    const auto finalIntPart       = roundedScaledValue / multiplier;
    auto       finalFracPart      = roundedScaledValue % multiplier;

    p = priv::unsignedToChars(p, last, finalIntPart);

    if (p == nullptr)
        return nullptr;

    if (last - p < precision + 1) // '.' + `precision` digits
        return nullptr;

    *p++ = '.';

    // Write the fractional digits directly into [p, p + precision), backward.
    // The output is always exactly `precision` chars wide; any shortfall vs. the
    // natural digit count is filled with leading zeros on the left.
    char* const fracStart  = p;
    char* const fracOutEnd = p + precision;
    char*       c          = fracOutEnd - 1;

    while (finalFracPart > 0)
    {
        *c-- = '0' + static_cast<char>(finalFracPart % 10);
        finalFracPart /= 10;
    }

    while (c >= fracStart)
        *c-- = '0';

    return fracOutEnd;
}

} // namespace sf::base
