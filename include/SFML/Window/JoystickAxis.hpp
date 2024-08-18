#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


namespace sf::Joystick
{
////////////////////////////////////////////////////////////
/// \brief Axes supported by SFML joysticks
///
////////////////////////////////////////////////////////////
enum class Axis
{
    X,    //!< The X axis
    Y,    //!< The Y axis
    Z,    //!< The Z axis
    R,    //!< The R axis
    U,    //!< The U axis
    V,    //!< The V axis
    PovX, //!< The X axis of the point-of-view hat
    PovY  //!< The Y axis of the point-of-view hat
};

} // namespace sf::Joystick


////////////////////////////////////////////////////////////
/// \namespace sf::Joystick
/// \ingroup window
///
/// Definition of the `sf::Joystic::Axis` enumeration
///
////////////////////////////////////////////////////////////
