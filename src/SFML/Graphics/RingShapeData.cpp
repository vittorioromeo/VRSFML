// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/RingShapeData.hpp"

#include "SFML/Graphics/Priv/EllipticalGlobalBounds.hpp"

#include "SFML/System/Rect2.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
Rect2f RingShapeData::getGlobalBounds() const noexcept
{
    if (!hasVisibleGeometry()) [[unlikely]]
        return {};

    return priv::getEllipticalGlobalBounds({outerRadius, outerRadius},
                                           {outerRadius, outerRadius},
                                           position,
                                           scale,
                                           origin,
                                           rotation);
}

} // namespace sf
