// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/RoundedRectangleShapeData.hpp"

#include "Zancle/Graphics/Priv/ArcBounds.hpp"
#include "Zancle/Graphics/ShapeUtils.hpp"
#include "Zancle/Graphics/Transform.hpp"

#include "Zancle/System/Priv/Vec2Base.hpp"
#include "Zancle/System/Rect2.hpp"


namespace za
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

} // namespace za
