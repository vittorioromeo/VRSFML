#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Color.hpp" // IWYU pragma: keep

#include "SFML/System/Vec2Base.hpp" // IWYU pragma: keep

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/IntTypes.hpp" // IWYU pragma: keep


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
#define IMGUI_DEFINE_MATH_OPERATORS


////////////////////////////////////////////////////////////
#define IM_ASSERT SFML_BASE_ASSERT
