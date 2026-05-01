// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/RoundedRectangleShapeData.hpp"

#include "SFML/Graphics/Priv/ArcBounds.hpp"
#include "SFML/Graphics/ShapeUtils.hpp"
#include "SFML/Graphics/Transform.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"
#include "SFML/System/Rect2.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
Rect2f RoundedRectangleShapeData::getGlobalBounds() const noexcept
{
    if (!hasVisibleGeometry()) [[unlikely]]
        return {};

    const auto transform = Transform::fromPositionScaleOriginRotation(position, scale, origin, rotation);

    return priv::computePolygonBounds(4u * cornerPointCount, [this](const unsigned int i) noexcept {
        return ShapeUtils::computeRoundedRectanglePoint(i, size, cornerRadius, cornerPointCount);
    }, [&](const Vec2f p) noexcept { return transform.transformPoint(p); });
}

} // namespace sf
