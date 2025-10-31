#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/Graphics/Color.hpp" // used

#include "SFML/System/Rect2.hpp" // used


////////////////////////////////////////////////////////////
/// \brief Define the data members of a shape's settings
///
////////////////////////////////////////////////////////////
#define SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE                                                             \
    ::sf::Rect2f textureRect{};                    /*!< Area of the source texture to display for the fill */    \
    ::sf::Rect2f outlineTextureRect{};             /*!< Area of the source texture to display for the outline */ \
    ::sf::Color  fillColor{::sf::Color::White};    /*!< Fill color */                                            \
    ::sf::Color  outlineColor{::sf::Color::White}; /*!< Outline color */                                         \
    float        outlineThickness{};               /*!< Thickness of the shape's outline */                      \
    float        miterLimit{4.f}; /*!< Limit on the ratio between miter length and outline thickness */          \
                                                                                                                 \
    static_assert(true)
