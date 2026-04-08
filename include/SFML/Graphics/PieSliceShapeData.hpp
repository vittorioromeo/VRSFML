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
/// \struct sf::PieSliceShapeData
/// \ingroup graphics
///
/// `sf::PieSliceShapeData` describes a pie slice (a filled angular
/// sector of a circle) defined by a `radius`, a `startAngle`, and a
/// `sweepAngle`. The contour is approximated by `pointCount` points
/// distributed along the arc.
///
/// Like other `*ShapeData` types, it embeds the standard
/// transformable and shape appearance members and is consumed by
/// `sf::ShapeUtils` and `sf::CPUDrawableBatch::add` -- it is not
/// directly drawable on its own.
///
/// \see `sf::Shape`, `sf::CircleShape`, `sf::RingPieSliceShapeData`, `sf::ShapeUtils`
///
////////////////////////////////////////////////////////////
