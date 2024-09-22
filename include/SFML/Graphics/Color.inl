#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Color.hpp" // NOLINT(misc-header-include-cycle)


namespace sf
{
////////////////////////////////////////////////////////////
constexpr Color Color::fromRGBA(std::uint32_t color)
{
    return {static_cast<std::uint8_t>((color & 0xff000000) >> 24),
            static_cast<std::uint8_t>((color & 0x00ff0000) >> 16),
            static_cast<std::uint8_t>((color & 0x0000ff00) >> 8),
            static_cast<std::uint8_t>(color & 0x000000ff)};
}


////////////////////////////////////////////////////////////
constexpr std::uint32_t Color::toInteger() const
{
    return static_cast<std::uint32_t>((r << 24) | (g << 16) | (b << 8) | a);
}


////////////////////////////////////////////////////////////
constexpr Color operator+(Color lhs, Color rhs)
{
    const auto clampedAdd = [](std::uint8_t l, std::uint8_t r) -> std::uint8_t
    {
        const int intResult = int{l} + int{r};
        return static_cast<std::uint8_t>(intResult < 255 ? intResult : 255);
    };

    return {clampedAdd(lhs.r, rhs.r), clampedAdd(lhs.g, rhs.g), clampedAdd(lhs.b, rhs.b), clampedAdd(lhs.a, rhs.a)};
}


////////////////////////////////////////////////////////////
constexpr Color operator-(Color lhs, Color rhs)
{
    const auto clampedSub = [](std::uint8_t l, std::uint8_t r) -> std::uint8_t
    {
        const int intResult = int{l} - int{r};
        return static_cast<std::uint8_t>(intResult > 0 ? intResult : 0);
    };

    return {clampedSub(lhs.r, rhs.r), clampedSub(lhs.g, rhs.g), clampedSub(lhs.b, rhs.b), clampedSub(lhs.a, rhs.a)};
}


////////////////////////////////////////////////////////////
constexpr Color operator*(Color lhs, Color rhs)
{
    const auto scaledMul = [](std::uint8_t l, std::uint8_t r) -> std::uint8_t
    {
        const auto uint16Result = static_cast<std::uint16_t>(std::uint16_t{l} * std::uint16_t{r});
        return static_cast<std::uint8_t>(uint16Result / 255u);
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
