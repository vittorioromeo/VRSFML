#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/Color.hpp"

#include "Zancle/Math/Remainder.hpp"

#include "Zancle/Base/AssertAndAssume.hpp"
#include "Zancle/Base/IntTypes.hpp"


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float wrapHue(const float hue) noexcept
{
    return za::positiveRemainder(hue, 360.f);
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr za::U8 hueToByte(const float hue) noexcept
{
    ZA_ASSERT_AND_ASSUME(hue >= 0.f && hue <= 360.f);
    return static_cast<za::U8>(hue / 360.f * 255.f);
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float byteToHue(const za::U8 byte) noexcept
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
                                                                                                const za::U8 alpha) noexcept
{
    // R=1 and G=1 is used in the shader as a signal to make the color hue-based
    return za::Color{1, 1, hueToByte(wrapHue(hue)), alpha};
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr za::Color hueByteColor(const za::U8 hueByte,
                                                                                                    const za::U8 alpha) noexcept
{
    // R=1 and G=1 is used in the shader as a signal to make the color hue-based
    return za::Color{1, 1, hueByte, alpha};
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float extractHue(const za::Color hueColor) noexcept
{
    return byteToHue(hueColor.g);
}
