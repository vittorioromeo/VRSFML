#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/Graphics/Color.hpp" // used

#include "SFML/System/Rect.hpp" // used


////////////////////////////////////////////////////////////
/// \brief Define the data members of a shape's settings
///
////////////////////////////////////////////////////////////
#define SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE                                                                \
    ::sf::FloatRect textureRect{};                    /*!< Area of the source texture to display for the fill */    \
    ::sf::FloatRect outlineTextureRect{};             /*!< Area of the source texture to display for the outline */ \
    ::sf::Color     fillColor{::sf::Color::White};    /*!< Fill color */                                            \
    ::sf::Color     outlineColor{::sf::Color::White}; /*!< Outline color */                                         \
    float           outlineThickness{};               /*!< Thickness of the shape's outline */                      \
                                                                                                                    \
    static_assert(true)
