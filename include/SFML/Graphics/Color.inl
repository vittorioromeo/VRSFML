#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/Color.hpp> // NOLINT(misc-header-include-cycle)


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
constexpr bool operator==(const Color& left, const Color& right)
{
    return (left.r == right.r) && (left.g == right.g) && (left.b == right.b) && (left.a == right.a);
}


////////////////////////////////////////////////////////////
constexpr bool operator!=(const Color& left, const Color& right)
{
    return !(left == right);
}


////////////////////////////////////////////////////////////
constexpr Color operator+(const Color& left, const Color& right)
{
    const auto clampedAdd = [](std::uint8_t lhs, std::uint8_t rhs) -> std::uint8_t
    {
        const int intResult = int{lhs} + int{rhs};
        return static_cast<std::uint8_t>(intResult < 255 ? intResult : 255);
    };

    return {clampedAdd(left.r, right.r),
            clampedAdd(left.g, right.g),
            clampedAdd(left.b, right.b),
            clampedAdd(left.a, right.a)};
}


////////////////////////////////////////////////////////////
constexpr Color operator-(const Color& left, const Color& right)
{
    const auto clampedSub = [](std::uint8_t lhs, std::uint8_t rhs) -> std::uint8_t
    {
        const int intResult = int{lhs} - int{rhs};
        return static_cast<std::uint8_t>(intResult > 0 ? intResult : 0);
    };

    return {clampedSub(left.r, right.r),
            clampedSub(left.g, right.g),
            clampedSub(left.b, right.b),
            clampedSub(left.a, right.a)};
}


////////////////////////////////////////////////////////////
constexpr Color operator*(const Color& left, const Color& right)
{
    const auto scaledMul = [](std::uint8_t lhs, std::uint8_t rhs) -> std::uint8_t
    {
        const auto uint16Result = static_cast<std::uint16_t>(std::uint16_t{lhs} * std::uint16_t{rhs});
        return static_cast<std::uint8_t>(uint16Result / 255u);
    };

    return {scaledMul(left.r, right.r), scaledMul(left.g, right.g), scaledMul(left.b, right.b), scaledMul(left.a, right.a)};
}


////////////////////////////////////////////////////////////
constexpr Color& operator+=(Color& left, const Color& right)
{
    return left = left + right;
}


////////////////////////////////////////////////////////////
constexpr Color& operator-=(Color& left, const Color& right)
{
    return left = left - right;
}


////////////////////////////////////////////////////////////
constexpr Color& operator*=(Color& left, const Color& right)
{
    return left = left * right;
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
