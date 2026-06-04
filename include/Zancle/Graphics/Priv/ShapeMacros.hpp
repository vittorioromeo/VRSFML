#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/Color.hpp" // IWYU pragma: keep

#include "Zancle/System/Rect2.hpp" // IWYU pragma: keep


////////////////////////////////////////////////////////////
/// \brief Define the data members of a shape's settings
///
////////////////////////////////////////////////////////////
#define ZA_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE                                                             \
    ::za::Rect2f textureRect{};                    /*!< Area of the source texture to display for the fill */    \
    ::za::Rect2f outlineTextureRect{};             /*!< Area of the source texture to display for the outline */ \
    ::za::Color  fillColor{::za::Color::White};    /*!< Fill color */                                            \
    ::za::Color  outlineColor{::za::Color::White}; /*!< Outline color */                                         \
    float        outlineThickness{};               /*!< Thickness of the shape's outline */                      \
    float        miterLimit{4.f}; /*!< Limit on the ratio between miter length and outline thickness */          \
                                                                                                                 \
    static_assert(true)
