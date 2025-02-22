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
/// \brief Data required to draw an ellipse shape
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
/// TODO P1: docs
///
/// \see `sf::Shape`
///
////////////////////////////////////////////////////////////
