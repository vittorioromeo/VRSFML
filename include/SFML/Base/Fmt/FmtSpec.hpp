#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Per-placeholder format spec.
///
/// Grammar (subset of `std::format`):
///     spec ::= [[fill]align]? [width]? [.precision]? [type]?
///     fill      = any single character (defaults to space)
///     align     = '<' (left), '>' (right), '^' (center)
///     width     = decimal digits (0..65535)
///     precision = '.' followed by decimal digits (0..10, floats only)
///     type      = 'd' (decimal int), 'x' / 'X' (hex), 'o' (octal),
///                 'b' (binary), 'c' (char glyph), 'f' (float fixed)
///
/// Type tag validation is strict: anything outside the set above is
/// rejected at consteval. Non-decimal integer tags ('x', 'X', 'o', 'b')
/// interpret the argument as its raw bit pattern -- no sign is emitted,
/// matching `printf("%x" / "%o")`.
///
/// Defaults: numeric args right-align, everything else left-aligns.
////////////////////////////////////////////////////////////
struct FmtSpec
{
    int  width     = 0;
    int  precision = -1;
    char align     = '\0';
    char fill      = ' ';
    char type      = '\0';
};

} // namespace sf::base
