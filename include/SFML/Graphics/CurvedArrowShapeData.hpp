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
/// \brief Defines the geometric properties of a curved arrow shape
///
/// `sf::CurvedArrowShapeData` is used to specify the characteristics
/// of an arrow that follows a curved path, such as its path radius,
/// thickness, angular span, and arrowhead dimensions.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API CurvedArrowShapeData
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    float        outerRadius{0.f};              //!< Radius of the outer circle defining the boundary
    float        innerRadius{0.f};              //!< Radius of the inner circle defining the hole
    Angle        startAngle{};                  //!< Starting angle of the curved path
    Angle        sweepAngle{sf::degrees(90.f)}; //!< Angular extent of the curved path
    float        headLength{15.f};              //!< Length of the arrowhead (from base to tip)
    float        headWidth{20.f};               //!< Width of the arrowhead at its base
    unsigned int pointCount{30u}; //!< Number of points to approximate the curved body (per 90 degrees of sweep)
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::CurvedArrowShapeData
/// \ingroup graphics
///
/// `sf::CurvedArrowShapeData` describes an arrow that bends along
/// a circular arc, useful for diagrams, UI affordances, and game
/// indicators.
///
/// Key properties:
/// - `outerRadius` -- radius of the outer edge of the arc body.
/// - `innerRadius` -- radius of the inner edge of the arc body
///   (the difference with `outerRadius` defines its thickness).
/// - `startAngle`, `sweepAngle` -- define the angular segment of
///   the arc that the arrow occupies.
/// - `headLength`, `headWidth` -- dimensions of the triangular
///   arrowhead at the end of the arc.
/// - `pointCount` -- controls the smoothness of the curved body.
///   It is interpreted as the number of points per 90 degrees of
///   sweep, so that arcs of different lengths render with comparable
///   smoothness.
///
/// Like other `*ShapeData` types, this struct also embeds the
/// standard transformable and shape appearance members.
///
/// `CurvedArrowShapeData` is consumed by `sf::ShapeUtils` and by
/// `sf::CPUDrawableBatch::add` -- it is not directly drawable.
///
/// Example usage:
/// \code
/// const sf::CurvedArrowShapeData curvedArrowData{
///     .position    = {200.f, 200.f},
///     .fillColor   = sf::Color::Red,
///     .outerRadius = 100.f,
///     .innerRadius = 50.f,
///     .startAngle  = sf::degrees(0.f),
///     .sweepAngle  = sf::degrees(120.f),
///     .headLength  = 20.f,
///     .headWidth   = 25.f,
///     .pointCount  = 20,
/// };
///
/// drawableBatch.add(curvedArrowData);
/// \endcode
///
/// \see `sf::ArrowShapeData`, `sf::RingPieSliceShapeData`, `sf::ShapeUtils`, `sf::DrawableBatch`
///
////////////////////////////////////////////////////////////
