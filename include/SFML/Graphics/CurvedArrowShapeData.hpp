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
/// \class sf::CurvedArrowShapeData
/// \ingroup graphics
///
/// `sf::CurvedArrowShapeData` allows for drawing arrows that bend along
/// a circular arc.
///
/// Key properties:
/// - `outerRadius`: Radius of the outer circle defining the boundary.
/// - `innerRadius`: Radius of the inner circle defining the hole.
/// - `startAngle`, `sweepAngle`: Define the segment of the circle that the arrow occupies.
/// - `headLength`, `headWidth`: Dimensions of the arrowhead.
/// - `pointCount`: Controls the smoothness of the curved body. This is interpreted as
///   points per 90 degrees of sweep angle to maintain similar smoothness for different sweep angles.
///
/// Example usage:
/// \code
/// sf::CurvedArrowShapeData curvedArrowData{
///     .position = {200, 200},
///     .outerRadius = 100.f,
///     .innerRadius = 50.f,
///     .startAngle = sf::degrees(0.f),
///     .sweepAngle = sf::degrees(120.f),
///     .headLength = 20.f,
///     .headWidth = 25.f,
///     .pointCount = 20,
/// };
///
/// window.draw(curvedArrowData);
/// \endcode
///
/// \see sf::ArrowShapeData, sf::RingPieSliceShapeData
///
////////////////////////////////////////////////////////////
