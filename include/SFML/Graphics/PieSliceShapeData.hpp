#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Priv/ShapeMacros.hpp"
#include "SFML/Graphics/Priv/TransformableMacros.hpp"

#include "SFML/System/Angle.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Data required to draw a pie slice shape
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API PieSliceShapeData
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    float        radius{0.f};     //!< Radius of the pie slice
    Angle        startAngle{};    //!< Starting angle of the pie slice
    Angle        sweepAngle{};    //!< Sweep angle of the pie slice
    unsigned int pointCount{30u}; //!< Number of points composing the pie slice
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::PieSliceShapeData
/// \ingroup graphics
///
/// TODO P1: docs
///
/// \see `sf::Shape`, `sf::CircleShape`
///
////////////////////////////////////////////////////////////
