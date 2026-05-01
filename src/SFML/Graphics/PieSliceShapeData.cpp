// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/PieSliceShapeData.hpp"

#include "SFML/Graphics/Priv/ArcBounds.hpp"
#include "SFML/Graphics/ShapeUtils.hpp"
#include "SFML/Graphics/Transform.hpp"

#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/Remainder.hpp"
#include "SFML/Base/SinCosLookup.hpp"


namespace sf
{
namespace
{
////////////////////////////////////////////////////////////
// Arc position using the pie-slice convention `(sin, cos)`.
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline Vec2f pieSlicePointAtAngle(const float radius,
                                                                                             const float radians) noexcept
{
    const auto [sine, cosine] = base::sinCosLookup(base::positiveRemainder(radians, base::tau));
    // Match the CW-visual convention used by `computePieSlicePointFromArcAngleStep`.
    return {radius - radius * sine, radius + radius * cosine};
}

} // namespace


////////////////////////////////////////////////////////////
Rect2f PieSliceShapeData::getLocalBounds() const noexcept
{
    if (!hasVisibleGeometry()) [[unlikely]]
        return {};

    // Normalize for `computeArcBounds`, which expects a non-negative sweep starting at `startRad`.
    // A negative sweep sweeps backwards from `startAngle`, so the equivalent forward-sweep sector
    // starts at `startAngle + sweepAngle` and has sweep `|sweepAngle|`.
    const float sweepRad = sweepAngle.asRadians();
    const float absSweep = SFML_BASE_MATH_FABSF(sweepRad);
    const float startRad = (sweepRad < 0.f) ? startAngle.asRadians() + sweepRad : startAngle.asRadians();

    const Vec2f hub = {radius, radius};

    return priv::computeArcBounds(startRad,
                                  absSweep,
                                  /* includeHub */ true,
                                  hub,
                                  [this](const float a) noexcept { return pieSlicePointAtAngle(radius, a); },
                                  [](const Vec2f p) noexcept { return p; });
}


////////////////////////////////////////////////////////////
Rect2f PieSliceShapeData::getGlobalBounds() const noexcept
{
    if (!hasVisibleGeometry()) [[unlikely]]
        return {};

    const auto transform = Transform::fromPositionScaleOriginRotation(position, scale, origin, rotation);

    const float sweepRad = sweepAngle.asRadians();

    // `arcAngleStep` can be negative (negative sweep); sampling via `startRad + i * arcAngleStep`
    // still walks the sector, just in the opposite direction.
    const float startRad     = startAngle.asRadians();
    const float arcAngleStep = ShapeUtils::computePieSliceArcAngleStep(sweepRad, pointCount);

    return priv::computePolygonBounds(pointCount,
                                      [&](const unsigned int i) noexcept
    {
        if (i == 0u)
            return Vec2f{radius, radius};

        const float angle = startRad + static_cast<float>(i - 1u) * arcAngleStep;
        return pieSlicePointAtAngle(radius, angle);
    },
                                      [&](const Vec2f p) noexcept { return transform.transformPoint(p); });
}


////////////////////////////////////////////////////////////
Vec2f PieSliceShapeData::getCentroid() const noexcept
{
    const Vec2f hub{radius, radius};

    if (!hasVisibleGeometry()) [[unlikely]]
        return hub;

    const float sweepRad = sweepAngle.asRadians();
    const float absSweep = SFML_BASE_MATH_FABSF(sweepRad);
    SFML_BASE_ASSERT(absSweep > 0.f);

    const float halfSwp = absSweep * 0.5f;

    const auto [sinHalf, cosHalf] = base::sinCosLookup(base::positiveRemainder(halfSwp, base::tau));

    // Centroid distance from hub along the bisector: d = 4*R*sin(alpha) / (3 * sweep), alpha = sweep/2.
    // Use |sweep| so the distance is always non-negative; the bisector direction (which depends on
    // the sign of `sweepRad`) carries the orientation.
    const float d = (4.f * radius * sinHalf) / (3.f * absSweep);

    // Bisector direction in pie-slice convention: (-sin, cos), evaluated at startAngle + sweep/2.
    // Works for negative sweep too: the bisector is always the angular midpoint of the sector.
    const float bisector = startAngle.asRadians() + sweepRad * 0.5f;

    const auto [sinB, cosB] = base::sinCosLookup(base::positiveRemainder(bisector, base::tau));

    return {hub.x - d * sinB, hub.y + d * cosB};
}

} // namespace sf
