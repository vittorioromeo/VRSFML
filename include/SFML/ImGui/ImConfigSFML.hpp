#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/ImGui/Export.hpp"

#include "SFML/Graphics/Color.hpp"

#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/IntTypes.hpp"


////////////////////////////////////////////////////////////
#define IM_VEC2_CLASS_EXTRA                                         \
    template <typename T>                                           \
    explicit ImVec2(const ::sf::Vec2<T>& v)                         \
    {                                                               \
        x = static_cast<float>(v.x);                                \
        y = static_cast<float>(v.y);                                \
    }                                                               \
                                                                    \
    template <typename T>                                           \
    explicit operator ::sf::Vec2<T>() const                         \
    {                                                               \
        return ::sf::Vec2<T>(static_cast<T>(x), static_cast<T>(y)); \
    }


////////////////////////////////////////////////////////////
#define IM_VEC4_CLASS_EXTRA                                                                                \
    explicit ImVec4(const ::sf::Color& c) : x(c.r / 255.f), y(c.g / 255.f), z(c.b / 255.f), w(c.a / 255.f) \
    {                                                                                                      \
    }                                                                                                      \
                                                                                                           \
    operator ::sf::Color() const                                                                           \
    {                                                                                                      \
        return ::sf::Color(static_cast<::sf::base::U8>(x * 255.f),                                         \
                           static_cast<::sf::base::U8>(y * 255.f),                                         \
                           static_cast<::sf::base::U8>(z * 255.f),                                         \
                           static_cast<::sf::base::U8>(w * 255.f));                                        \
    }


////////////////////////////////////////////////////////////
#define IMGUI_DEFINE_MATH_OPERATORS


////////////////////////////////////////////////////////////
#define IM_ASSERT SFML_BASE_ASSERT
