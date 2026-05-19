#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Numeric base used by `toCharsRadix` / `fromCharsRadix`.
///
/// The underlying values are the bases themselves, so a `Radix` value can
/// be used directly as an unsigned divisor / multiplier inside conversion
/// loops -- no switch table needed.
///
/// Decimal is intentionally absent: the existing `toChars` / `fromChars`
/// overloads are signed-aware and emit / accept a leading `'-'`. The radix
/// variants treat the integer as a raw bit pattern instead (no sign).
///
////////////////////////////////////////////////////////////
enum class Radix : unsigned int
{
    Bin = 2u,
    Oct = 8u,
    Hex = 16u,
};

} // namespace sf::base
