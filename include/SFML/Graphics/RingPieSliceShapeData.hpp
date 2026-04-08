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
/// \brief Data required to draw a ring pie slice shape
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API RingPieSliceShapeData
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    float        outerRadius{0.f}; //!< Radius of the outer circle defining the boundary
    float        innerRadius{0.f}; //!< Radius of the inner circle defining the hole
    Angle        startAngle{};     //!< Starting angle of the pie slice
    Angle        sweepAngle{};     //!< Sweep angle of the pie slice
    unsigned int pointCount{30u};  //!< Number of points composing the pie slice
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::RingPieSliceShapeData
/// \ingroup graphics
///
/// `sf::RingPieSliceShapeData` describes a slice of a ring -- the
/// intersection of a `sf::RingShapeData` and a `sf::PieSliceShapeData`.
/// It is defined by an `outerRadius`, an `innerRadius`, a `startAngle`,
/// and a `sweepAngle`. `pointCount` controls the smoothness of the
/// arcs.
///
/// Like other `*ShapeData` types, it embeds the standard
/// transformable and shape appearance members and is consumed by
/// `sf::ShapeUtils` and `sf::CPUDrawableBatch::add` -- it is not
/// directly drawable on its own.
///
/// \see `sf::Shape`, `sf::RingShapeData`, `sf::PieSliceShapeData`, `sf::CurvedArrowShapeData`
///
////////////////////////////////////////////////////////////
