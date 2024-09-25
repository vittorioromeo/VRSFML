#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Color.hpp" // NOLINT(misc-header-include-cycle)


namespace sf
{
////////////////////////////////////////////////////////////
constexpr Color Color::fromRGBA(base::U32 color)
{
    return {static_cast<base::U8>((color & 0xff000000) >> 24),
            static_cast<base::U8>((color & 0x00ff0000) >> 16),
            static_cast<base::U8>((color & 0x0000ff00) >> 8),
            static_cast<base::U8>(color & 0x000000ff)};
}


////////////////////////////////////////////////////////////
constexpr base::U32 Color::toInteger() const
{
    return static_cast<base::U32>((r << 24) | (g << 16) | (b << 8) | a);
}


////////////////////////////////////////////////////////////
constexpr Color operator+(Color lhs, Color rhs)
{
    const auto clampedAdd = [](base::U8 l, base::U8 r) -> base::U8
    {
        const int intResult = int{l} + int{r};
        return static_cast<base::U8>(intResult < 255 ? intResult : 255);
    };

    return {clampedAdd(lhs.r, rhs.r), clampedAdd(lhs.g, rhs.g), clampedAdd(lhs.b, rhs.b), clampedAdd(lhs.a, rhs.a)};
}


////////////////////////////////////////////////////////////
constexpr Color operator-(Color lhs, Color rhs)
{
    const auto clampedSub = [](base::U8 l, base::U8 r) -> base::U8
    {
        const int intResult = int{l} - int{r};
        return static_cast<base::U8>(intResult > 0 ? intResult : 0);
    };

    return {clampedSub(lhs.r, rhs.r), clampedSub(lhs.g, rhs.g), clampedSub(lhs.b, rhs.b), clampedSub(lhs.a, rhs.a)};
}


////////////////////////////////////////////////////////////
constexpr Color operator*(Color lhs, Color rhs)
{
    const auto scaledMul = [](base::U8 l, base::U8 r) -> base::U8
    {
        const auto uint16Result = static_cast<base::U16>(base::U16{l} * base::U16{r});
        return static_cast<base::U8>(uint16Result / 255u);
    };

    return {scaledMul(lhs.r, rhs.r), scaledMul(lhs.g, rhs.g), scaledMul(lhs.b, rhs.b), scaledMul(lhs.a, rhs.a)};
}


////////////////////////////////////////////////////////////
constexpr Color& operator+=(Color& lhs, Color rhs)
{
    return lhs = lhs + rhs;
}


////////////////////////////////////////////////////////////
constexpr Color& operator-=(Color& lhs, Color rhs)
{
    return lhs = lhs - rhs;
}


////////////////////////////////////////////////////////////
constexpr Color& operator*=(Color& lhs, Color rhs)
{
    return lhs = lhs * rhs;
}


////////////////////////////////////////////////////////////
// Static member data
////////////////////////////////////////////////////////////

// Note: the 'inline' keyword here is technically not required, but VS2019 fails
// to compile with a bogus "multiple definition" error if not explicitly used.
inline constexpr Color Color::Black(0, 0, 0);
inline constexpr Color Color::White(255, 255, 255);
inline constexpr Color Color::Red(255, 0, 0);
inline constexpr Color Color::Green(0, 255, 0);
inline constexpr Color Color::Blue(0, 0, 255);
inline constexpr Color Color::Yellow(255, 255, 0);
inline constexpr Color Color::Magenta(255, 0, 255);
inline constexpr Color Color::Cyan(0, 255, 255);
inline constexpr Color Color::Transparent(0, 0, 0, 0);

} // namespace sf
