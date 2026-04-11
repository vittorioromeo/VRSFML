#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Priv/ShapeMacros.hpp"
#include "SFML/Graphics/Priv/TransformableMacros.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Data required to draw a rectangle shape
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API RectangleShapeData
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    Vec2f size{}; //!< Size of the rectangle
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::RectangleShapeData
/// \ingroup graphics
///
/// `sf::RectangleShapeData` is the plain-data description of an
/// axis-aligned rectangle. It holds the rectangle's `size` together
/// with the standard transformable members (`position`, `scale`,
/// `origin`, `rotation`) and the standard shape appearance members
/// (`textureRect`, `outlineTextureRect`, `fillColor`,
/// `outlineColor`, `outlineThickness`, `miterLimit`).
///
/// Use it as input for `sf::ShapeUtils`, `sf::RectangleShape`, or a
/// drawable batch when you want to render a rectangle without
/// instantiating a full shape object.
///
/// \see `sf::Shape`, `sf::RectangleShape`, `sf::ShapeUtils`
///
////////////////////////////////////////////////////////////
