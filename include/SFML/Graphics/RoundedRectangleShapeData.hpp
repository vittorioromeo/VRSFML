#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Priv/ShapeMacros.hpp"
#include "SFML/Graphics/Priv/TransformableMacros.hpp"

#include "SFML/System/Vec2.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Data required to draw a rounded rectangle shape
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API RoundedRectangleShapeData
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    Vec2f        size{};                //!< Size of the rounded rectangle
    float        cornerRadius{0.f};     //!< Radius of the corners of the rounded rectangle
    unsigned int cornerPointCount{16u}; //!< Number of points used to draw the corners of the rounded rectangle
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::RoundedRectangleShapeData
/// \ingroup graphics
///
/// TODO P1: docs
///
/// \see `sf::Shape`
///
////////////////////////////////////////////////////////////
