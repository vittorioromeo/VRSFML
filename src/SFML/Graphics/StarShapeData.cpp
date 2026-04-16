// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/StarShapeData.hpp"

#include "SFML/Graphics/Priv/ArcBounds.hpp"
#include "SFML/Graphics/ShapeUtils.hpp"
#include "SFML/Graphics/Transform.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"
#include "SFML/System/Rect2.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
Rect2f StarShapeData::getGlobalBounds() const noexcept
{
    if (!hasVisibleGeometry()) [[unlikely]]
        return {};

    const auto transform   = Transform::fromPositionScaleOriginRotation(position, scale, origin, rotation);
    const auto transformFn = [&](const Vec2f p) noexcept { return transform.transformPoint(p); };

    // In the typical case (`innerRadius <= outerRadius`) the inner-tip vertices lie strictly
    // inside the outer-tip envelope, so only outer tips (even indices) can push the AABB.
    // Stride by 2 to halve the number of transforms. The pathological `innerRadius > outerRadius`
    // case folds every vertex.
    if (innerRadius <= outerRadius) [[likely]]
        return priv::computePolygonBounds(pointCount, [this](const unsigned int i) noexcept {
            return ShapeUtils::computeStarPoint(2u * i, pointCount, outerRadius, innerRadius);
        }, transformFn);

    return priv::computePolygonBounds(2u * pointCount, [this](const unsigned int i) noexcept {
        return ShapeUtils::computeStarPoint(i, pointCount, outerRadius, innerRadius);
    }, transformFn);
}

} // namespace sf
