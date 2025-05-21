#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Priv/ShapeMacros.hpp"
#include "SFML/Graphics/Priv/TransformableMacros.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Defines the geometric properties of a circle shape
///
/// `sf::CircleShapeData` is used to specify the characteristics
/// of a circle, such as its radius and the number of points
/// used to approximate its contour. This data is then typically
/// used by a rendering system or a shape class like `sf::CircleShape`
/// to draw the actual circle.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API CircleShapeData
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    float        radius{0.f};     //!< Radius of the circle
    unsigned int pointCount{30u}; //!< Number of points composing the circle
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::CircleShapeData
/// \ingroup graphics
///
/// TODO P1: docs
///
/// \see `sf::Shape`, `sf::CircleShape`
///
////////////////////////////////////////////////////////////
