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
/// \brief Defines the geometric properties of an ellipse shape
///
/// `sf::EllipseShapeData` is used to specify the characteristics
/// of an ellipse, such as its horizontal and vertical radii,
/// and the number of points used to approximate its contour.
/// This data is then typically used by a rendering system or a
/// shape class to draw the actual ellipse.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API EllipseShapeData
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    float        horizontalRadius{0.f}; //!< Horizontal radius of the ellipse
    float        verticalRadius{0.f};   //!< Vertical radius of the ellipse
    Angle        startAngle{};          //!< Starting angle of ellipse point generation
    unsigned int pointCount{30u};       //!< Number of points composing the ellipse
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::EllipseShapeData
/// \ingroup graphics
///
/// `sf::EllipseShapeData` is a plain struct that holds the geometric
/// definition of an ellipse: horizontal and vertical radii, the
/// starting angle of the contour, and the number of points used to
/// approximate the curve.
///
/// This struct is typically used together with `sf::ShapeUtils` to
/// produce vertex data, with `sf::Shape` for a drawable wrapper, or
/// directly with a batching system that consumes shape data.
///
/// Like all `*ShapeData` types, it also embeds the common
/// transformable members (`position`, `scale`, `origin`, `rotation`)
/// and the common shape appearance members (`textureRect`,
/// `outlineTextureRect`, `fillColor`, `outlineColor`,
/// `outlineThickness`, `miterLimit`).
///
/// Example usage:
/// \code
/// const sf::EllipseShapeData ellipseData{
///     .fillColor        = sf::Color::Cyan,
///     .outlineColor     = sf::Color::Blue,
///     .outlineThickness = 2.f,
///     .horizontalRadius = 50.f,
///     .verticalRadius   = 30.f,
///     .pointCount       = 40, // more points = smoother ellipse
/// };
/// \endcode
///
/// \see `sf::Shape`, `sf::ShapeUtils`, `sf::CircleShapeData`, `sf::RectangleShapeData`
///
////////////////////////////////////////////////////////////
