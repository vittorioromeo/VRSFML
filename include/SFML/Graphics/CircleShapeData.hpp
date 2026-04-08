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
/// \brief Defines the geometric properties of a circle shape
///
/// `sf::CircleShapeData` is used to specify the characteristics
/// of a circle, such as its radius and the number of points
/// used to approximate its contour. This data is consumed by
/// `sf::ShapeUtils`, `sf::Shape`, `sf::CircleShape`, and the
/// drawable batching system.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API CircleShapeData
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    float        radius{0.f};     //!< Radius of the circle
    Angle        startAngle{};    //!< Starting angle of the first generated point on the contour
    unsigned int pointCount{30u}; //!< Number of points composing the circle (higher = smoother contour)
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::CircleShapeData
/// \ingroup graphics
///
/// `sf::CircleShapeData` is the plain-data description of a circle.
/// It holds the radius and tessellation parameters together with
/// the standard transformable members (`position`, `scale`,
/// `origin`, `rotation`) and the standard shape appearance members
/// (`textureRect`, `outlineTextureRect`, `fillColor`,
/// `outlineColor`, `outlineThickness`, `miterLimit`).
///
/// It is intentionally a flat struct so that it can be passed by
/// value, stored in arrays, and consumed by free functions.
///
/// Example usage:
/// \code
/// const sf::CircleShapeData data{
///     .position  = {100.f, 100.f},
///     .fillColor = sf::Color::Yellow,
///     .radius    = 32.f,
/// };
/// \endcode
///
/// \see `sf::Shape`, `sf::CircleShape`, `sf::ShapeUtils`
///
////////////////////////////////////////////////////////////
