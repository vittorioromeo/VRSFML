#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


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
    unsigned int pointCount{30u};       //!< Number of points composing the ellipse
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::EllipseShapeData
/// \ingroup graphics
///
/// `sf::EllipseShapeData` is a structure that holds the geometric
/// definition of an ellipse. It defines properties like the
/// horizontal and vertical radii of the ellipse, and the number
/// of points used to approximate its curved shape.
///
/// This structure is typically used in conjunction with a generic
/// shape drawing mechanism or a batching system that can take this
/// data to produce a visual representation of an ellipse.
///
/// Example usage:
/// \code
/// sf::EllipseShapeData ellipseData{
///     horizontalRadius = 50.f,
///     verticalRadius = 30.f,
///     pointCount = 40, // More points for a smoother ellipse
///     fillColor = sf::Color::Cyan,
///     outlineColor = sf::Color::Blue,
///     outlineThickness = 2.f,
/// };
///
/// // ... (pass `ellipseData` to a render target or batch)
/// \endcode
///
/// The `horizontalRadius` and `verticalRadius` members control the
/// dimensions of the ellipse along the X and Y axes, respectively.
/// The `pointCount` member determines how many line segments are
/// used to approximate the ellipse's curve; a higher count results
/// in a smoother appearance.
///
/// Like other shape data structures, it also includes common
/// properties for transform (position, rotation, scale via inherited macros)
/// and appearance (fill color, outline color, texture, etc. via inherited macros).
///
/// \see sf::Shape, sf::CircleShapeData, sf::RectangleShapeData
///
////////////////////////////////////////////////////////////
