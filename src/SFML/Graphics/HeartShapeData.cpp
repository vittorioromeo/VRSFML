// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/HeartShapeData.hpp"

#include "SFML/Graphics/Priv/ArcBounds.hpp"
#include "SFML/Graphics/ShapeUtils.hpp"
#include "SFML/Graphics/Transform.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"
#include "SFML/System/Rect2.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
Rect2f HeartShapeData::getGlobalBounds() const noexcept
{
    if (!hasVisibleGeometry()) [[unlikely]]
        return {};

    const auto transform = Transform::fromPositionScaleOriginRotation(position, scale, origin, rotation);

    return priv::computePolygonBounds(pointCount, [this](const unsigned int i) noexcept {
        return ShapeUtils::computeHeartPoint(i, pointCount, size);
    }, [&](const Vec2f p) noexcept { return transform.transformPoint(p); });
}


////////////////////////////////////////////////////////////
Vec2f HeartShapeData::getCentroid() const noexcept
{
    const Vec2f bboxCenter{size.x * 0.5f, size.y * 0.5f};

    if (pointCount < 4u) [[unlikely]]
        return bboxCenter;

    float signedArea2 = 0.f;
    float cxAcc       = 0.f;
    float cyAcc       = 0.f;

    Vec2f prev = ShapeUtils::computeHeartPoint(0u, pointCount, size);
    for (unsigned int i = 0u; i < pointCount; ++i)
    {
        const Vec2f curr  = ShapeUtils::computeHeartPoint((i + 1u) % pointCount, pointCount, size);
        const float cross = prev.x * curr.y - curr.x * prev.y;
        signedArea2 += cross;
        cxAcc += (prev.x + curr.x) * cross;
        cyAcc += (prev.y + curr.y) * cross;
        prev = curr;
    }

    if (signedArea2 == 0.f) [[unlikely]]
        return bboxCenter;

    return {cxAcc / (3.f * signedArea2), cyAcc / (3.f * signedArea2)};
}

} // namespace sf
