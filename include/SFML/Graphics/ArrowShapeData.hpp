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
/// \struct sf::ArrowShapeData
/// \ingroup graphics
///
/// `sf::ArrowShapeData` is a plain struct that holds the geometric
/// definition of an arrow: the length/width of the shaft and the
/// length/width of the head. It also embeds the standard
/// transformable members (`position`, `scale`, `origin`, `rotation`)
/// and the standard shape appearance members (`textureRect`,
/// `outlineTextureRect`, `fillColor`, `outlineColor`,
/// `outlineThickness`, `miterLimit`).
///
/// `ArrowShapeData` is consumed by `sf::ShapeUtils` and by
/// `sf::CPUDrawableBatch::add` to generate the corresponding vertex
/// data. It is not directly drawable on its own.
///
/// Example usage:
/// \code
/// const sf::ArrowShapeData arrowData{
///     .position    = {100.f, 100.f},
///     .fillColor   = sf::Color::White,
///     .shaftLength = 100.f,
///     .shaftWidth  = 5.f,
///     .headLength  = 20.f,
///     .headWidth   = 15.f,
/// };
///
/// drawableBatch.add(arrowData);
/// \endcode
///
/// \see `sf::Shape`, `sf::ShapeUtils`, `sf::CurvedArrowShapeData`, `sf::DrawableBatch`
///
////////////////////////////////////////////////////////////
