// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/CircleShapeData.hpp"
#include "Zancle/Graphics/Priv/EllipticalGlobalBounds.hpp"
#include "Zancle/System/Rect2.hpp"


namespace za
{
////////////////////////////////////////////////////////////
Rect2f CircleShapeData::getGlobalBounds() const noexcept
{
    if (!hasVisibleGeometry()) [[unlikely]]
        return {};

    return priv::getEllipticalGlobalBounds({radius, radius}, {radius, radius}, position, scale, origin, rotation);
}

} // namespace za
