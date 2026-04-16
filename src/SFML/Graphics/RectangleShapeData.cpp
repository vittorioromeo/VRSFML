// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/RectangleShapeData.hpp"

#include "SFML/Graphics/Transform.hpp"

#include "SFML/System/Rect2.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
Rect2f RectangleShapeData::getGlobalBounds() const noexcept
{
    if (!hasVisibleGeometry()) [[unlikely]]
        return {};

    return Transform::fromPositionScaleOriginRotation(position, scale, origin, rotation).transformRect(getLocalBounds());
}

} // namespace sf
