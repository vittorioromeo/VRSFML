// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/RectangleShapeData.hpp"
#include "Zancle/Graphics/Transform.hpp"
#include "Zancle/System/Rect2.hpp"


namespace za
{
////////////////////////////////////////////////////////////
Rect2f RectangleShapeData::getGlobalBounds() const noexcept
{
    if (!hasVisibleGeometry()) [[unlikely]]
        return {};

    return Transform::fromPositionScaleOriginRotation(position, scale, origin, rotation).transformRect(getLocalBounds());
}

} // namespace za
