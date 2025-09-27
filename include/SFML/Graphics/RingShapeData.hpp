#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Priv/ShapeMacros.hpp"
#include "SFML/Graphics/Priv/TransformableMacros.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Data required to draw a ring shape
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API RingShapeData
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    float        outerRadius{0.f}; //!< Distance from the center to the outer points
    float        innerRadius{0.f}; //!< Distance from the center to the inner points
    Angle        startAngle{};     //!< Starting angle of the ring point generation
    unsigned int pointCount{30u};  //!< Number of points composing the ring
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::RingShapeData
/// \ingroup graphics
///
/// TODO P1: docs
///
/// \see `sf::Shape`, `sf::CircleShape`
///
////////////////////////////////////////////////////////////
