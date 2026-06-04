// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/Priv/EllipticalGlobalBounds.hpp"
#include "Zancle/Graphics/RingShapeData.hpp"
#include "Zancle/System/Rect2.hpp"


namespace za
{
////////////////////////////////////////////////////////////
Rect2f RingShapeData::getGlobalBounds() const noexcept
{
    if (!hasVisibleGeometry()) [[unlikely]]
        return {};

    return priv::getEllipticalGlobalBounds({outerRadius, outerRadius}, {outerRadius, outerRadius}, position, scale, origin, rotation);
}

} // namespace za
