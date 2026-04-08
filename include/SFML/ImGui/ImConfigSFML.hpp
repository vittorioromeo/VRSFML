#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Dear ImGui user configuration for VRSFML
//
// This header is injected into Dear ImGui via the
// `IMGUI_USER_CONFIG` compile definition (set by the
// `sfml-imgui` CMake target). It does three things:
//
//   1. Wires `IM_ASSERT` into VRSFML's assertion macro.
//   2. Defines `IM_VEC2_CLASS_EXTRA` and `IM_VEC4_CLASS_EXTRA`
//      so that `ImVec2`/`ImVec4` interoperate seamlessly with
//      `sf::Vec2<T>` and `sf::Color`.
//   3. Enables Dear ImGui's `ImVec2`/`ImVec4` math operators by
//      defining `IMGUI_DEFINE_MATH_OPERATORS`.
//
// User code does not need to include this file directly: it is
// pulled in transitively by `IncludeImGui.hpp` and
// `IncludeImGuiInternal.hpp`.
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Color.hpp" // IWYU pragma: keep

#include "SFML/System/Vec2Base.hpp" // IWYU pragma: keep

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/IntTypes.hpp" // IWYU pragma: keep


////////////////////////////////////////////////////////////
// Conversions between `ImVec2` and `sf::Vec2<T>`
////////////////////////////////////////////////////////////
#define IM_VEC2_CLASS_EXTRA                                                        \
    template <typename T>                                                          \
    explicit ImVec2(const ::sf::Vec2<T>& v)                                        \
    {                                                                              \
        x = static_cast<float>(v.x);                                               \
        y = static_cast<float>(v.y);                                               \
    }                                                                              \
                                                                                   \
    template <typename T>                                                          \
    [[gnu::always_inline, gnu::pure]] explicit operator ::sf::Vec2<T>() const      \
    {                                                                              \
        return ::sf::Vec2<T>(static_cast<T>(x), static_cast<T>(y));                \
    }                                                                              \
                                                                                   \
    [[gnu::always_inline, gnu::pure]] operator ::sf::Vec2f() const                 \
    {                                                                              \
        return ::sf::Vec2f(x, y);                                                  \
    }                                                                              \
                                                                                   \
    [[gnu::always_inline, gnu::pure]] ImVec2 operator+(const ::sf::Vec2f& v) const \
    {                                                                              \
        return ImVec2(x + v.x, y + v.y);                                           \
    }                                                                              \
                                                                                   \
    [[gnu::always_inline, gnu::pure]] ImVec2 operator-(const ::sf::Vec2f& v) const \
    {                                                                              \
        return ImVec2(x - v.x, y - v.y);                                           \
    }


////////////////////////////////////////////////////////////
// Conversions between `ImVec4` and `sf::Color`
////////////////////////////////////////////////////////////
#define IM_VEC4_CLASS_EXTRA                                                                       \
    ImVec4(const ::sf::Color& c) : x(c.r / 255.f), y(c.g / 255.f), z(c.b / 255.f), w(c.a / 255.f) \
    {                                                                                             \
    }                                                                                             \
                                                                                                  \
    operator ::sf::Color() const                                                                  \
    {                                                                                             \
        return ::sf::Color(static_cast<::sf::base::U8>(x * 255.f + 0.5f),                         \
                           static_cast<::sf::base::U8>(y * 255.f + 0.5f),                         \
                           static_cast<::sf::base::U8>(z * 255.f + 0.5f),                         \
                           static_cast<::sf::base::U8>(w * 255.f + 0.5f));                        \
    }


////////////////////////////////////////////////////////////
// Enable math operators (`+`, `-`, `*`, `/`) on `ImVec2` / `ImVec4`
////////////////////////////////////////////////////////////
#define IMGUI_DEFINE_MATH_OPERATORS


////////////////////////////////////////////////////////////
// Route Dear ImGui assertions through VRSFML's assertion macro
////////////////////////////////////////////////////////////
#define IM_ASSERT SFML_BASE_ASSERT
