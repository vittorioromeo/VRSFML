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
/// \brief Data required to draw a star shape
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API StarShapeData
{
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
    SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

    float        outerRadius{0.f}; //!< Distance from the center to the outer points
    float        innerRadius{0.f}; //!< Distance from the center to the inner points
    unsigned int pointCount{5u};   //!< Number of points of the star (e.g., 5 for a 5-pointed star)
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::StarShapeData
/// \ingroup graphics
///
/// `sf::StarShapeData` describes a regular star polygon. It has
/// `pointCount` outer "points" and `pointCount` inner concave
/// vertices. `outerRadius` is the distance from the center to the
/// tips, and `innerRadius` is the distance from the center to the
/// concave vertices between them. The ratio between the two
/// determines how "spiky" the star looks.
///
/// Like other `*ShapeData` types, it embeds the standard
/// transformable and shape appearance members and is consumed by
/// `sf::ShapeUtils` and `sf::CPUDrawableBatch::add` -- it is not
/// directly drawable on its own.
///
/// \see `sf::Shape`, `sf::ShapeUtils`
///
////////////////////////////////////////////////////////////
