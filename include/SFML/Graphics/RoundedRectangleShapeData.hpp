#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Priv/ShapeMacros.hpp"
#include "SFML/Graphics/Priv/TransformableMacros.hpp"

#include "SFML/System/Vec2Base.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Data required to draw a rounded rectangle shape
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API RoundedRectangleShapeData
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    Vec2f        size{};                //!< Size of the rounded rectangle
    float        cornerRadius{0.f};     //!< Radius of the corners of the rounded rectangle
    unsigned int cornerPointCount{16u}; //!< Number of points used to draw the corners of the rounded rectangle
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::RoundedRectangleShapeData
/// \ingroup graphics
///
/// `sf::RoundedRectangleShapeData` describes an axis-aligned
/// rectangle whose corners are rounded with a quarter-circle of
/// radius `cornerRadius`. The smoothness of each rounded corner is
/// controlled by `cornerPointCount`.
///
/// Like other `*ShapeData` types, it embeds the standard
/// transformable and shape appearance members and is consumed by
/// `sf::ShapeUtils` and `sf::CPUDrawableBatch::add` -- it is not
/// directly drawable on its own.
///
/// \see `sf::Shape`, `sf::RectangleShapeData`, `sf::ShapeUtils`
///
////////////////////////////////////////////////////////////
