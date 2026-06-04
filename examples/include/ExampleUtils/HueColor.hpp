#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/Color.hpp"

#include "ZancleBase/AssertAndAssume.hpp"
#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/Remainder.hpp"


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float wrapHue(const float hue) noexcept
{
    return zb::positiveRemainder(hue, 360.f);
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr zb::U8 hueToByte(const float hue) noexcept
{
    ZB_ASSERT_AND_ASSUME(hue >= 0.f && hue <= 360.f);
    return static_cast<zb::U8>(hue / 360.f * 255.f);
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float byteToHue(const zb::U8 byte) noexcept
{
    return static_cast<float>(byte) / 255.f * 360.f;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr bool isHueColor(const za::Color color) noexcept
{
    return color.r == 1 && color.g == 1;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr za::Color hueColor(const float hue,
                                                                                                const zb::U8 alpha) noexcept
{
    // R=1 and G=1 is used in the shader as a signal to make the color hue-based
    return za::Color{1, 1, hueToByte(wrapHue(hue)), alpha};
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr za::Color hueByteColor(
    const zb::U8 hueByte,
    const zb::U8 alpha) noexcept
{
    // R=1 and G=1 is used in the shader as a signal to make the color hue-based
    return za::Color{1, 1, hueByte, alpha};
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float extractHue(const za::Color hueColor) noexcept
{
    return byteToHue(hueColor.g);
}
