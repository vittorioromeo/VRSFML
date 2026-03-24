#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Color.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2Base.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
struct DrawTextureSettings
{
    Vec2f position{};      //!< Position of the object in the 2D world
    Vec2f scale{1.f, 1.f}; //!< Scale of the object
    Vec2f origin{};        //!< Origin of translation/rotation/scaling of the object

    // NOLINTNEXTLINE(readability-redundant-member-init)
    Angle rotation{}; //!< Orientation of the object

    Rect2f textureRect{};       //!< Rectangle defining the area of the source texture to display
    Color  color{Color::White}; //!< Color of the sprite
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::RenderTarget
/// \ingroup graphics
///
/// TODO P1: docs
///
/// \see TODO P1: docs
///
////////////////////////////////////////////////////////////
