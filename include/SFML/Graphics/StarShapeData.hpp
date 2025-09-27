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
/// \brief Data required to draw a star shape
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API StarShapeData
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    float        outerRadius{0.f}; //!< Distance from the center to the outer points
    float        innerRadius{0.f}; //!< Distance from the center to the inner points
    unsigned int pointCount{5u};   //!< Number of points of the star (e.g., 5 for a 5-pointed star)
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::StarShapeData
/// \ingroup graphics
///
/// TODO P1: docs
///
/// \see `sf::Shape`, `sf::CircleShape`
///
////////////////////////////////////////////////////////////
