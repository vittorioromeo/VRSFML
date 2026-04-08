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
/// \struct sf::RingShapeData
/// \ingroup graphics
///
/// `sf::RingShapeData` describes a ring (an annulus): a closed band
/// between two concentric circles defined by `outerRadius` and
/// `innerRadius`. The contour is approximated by `pointCount` points
/// per circle, and `startAngle` shifts where on the circle the
/// tessellation begins.
///
/// Like other `*ShapeData` types, it embeds the standard
/// transformable and shape appearance members and is consumed by
/// `sf::ShapeUtils` and `sf::CPUDrawableBatch::add` -- it is not
/// directly drawable on its own.
///
/// \see `sf::Shape`, `sf::CircleShape`, `sf::RingPieSliceShapeData`, `sf::ShapeUtils`
///
////////////////////////////////////////////////////////////
