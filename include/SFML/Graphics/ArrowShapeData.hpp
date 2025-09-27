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
/// \brief Defines the geometric properties of an arrow shape
///
/// `sf::ArrowShapeData` is used to specify the dimensions and
/// characteristics of an arrow, such as the length and width
/// of its shaft and head. This data is then typically used by
/// a rendering system or a shape class to draw the actual arrow.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API ArrowShapeData
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    float shaftLength{50.f}; //!< Length of the shaft of the arrow
    float shaftWidth{10.f};  //!< Width of the shaft of the arrow
    float headLength{20.f};  //!< Length of the head of the arrow
    float headWidth{30.f};   //!< Width of the head of the arrow
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::ArrowShapeData
/// \ingroup graphics
///
/// `sf::ArrowShapeData` is a structure that holds the geometric
/// definition of an arrow. It defines properties like the length
/// and width of the arrow's shaft, and the length and width of
/// its head.
///
/// This structure is typically used in conjunction with a generic
/// shape class or a rendering function that can take this data
/// to produce a visual representation of an arrow.
///
/// Example usage:
/// \code
/// sf::ArrowShapeData arrowData{
///     .position = {100.f, 100.f},
///     .shaftLength = 100.f,
///     .shaftWidth = 5.f,
///     .headLength = 20.f,
///     .headWidth = 15.f,
/// };
///
/// window.draw(arrowData);
/// \endcode
///
/// The individual members `shaftLength`, `shaftWidth`, `headLength`,
/// and `headWidth` control the respective dimensions of the arrow.
///
/// \see sf::Shape, sf::RectangleShape, sf::ConvexShape
///
////////////////////////////////////////////////////////////
