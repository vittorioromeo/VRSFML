// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/Priv/ArcBounds.hpp"
#include "Zancle/Graphics/RingPieSliceShapeData.hpp"
#include "Zancle/Graphics/ShapeUtils.hpp"
#include "Zancle/Graphics/Transform.hpp"
#include "Zancle/System/Rect2.hpp"
#include "Zancle/System/Vec2.hpp"
#include "ZancleBase/Constants.hpp"
#include "ZancleBase/Math/Fabs.hpp"
#include "ZancleBase/Remainder.hpp"
#include "ZancleBase/SinCosLookup.hpp"


namespace za
{
////////////////////////////////////////////////////////////
Rect2f RingPieSliceShapeData::getLocalBounds() const noexcept
{
    if (!hasVisibleGeometry()) [[unlikely]]
        return {};

    const float absSweep = ZB_MATH_FABSF(sweepAngle.asRadians());
    const float startRad = (sweepAngle.asRadians() < 0.f) ? startAngle.asRadians() + sweepAngle.asRadians()
                                                          : startAngle.asRadians();

    const auto idFn = [](const Vec2f p) noexcept { return p; };

    const auto outerAabb = priv::computeArcBounds(startRad,
                                                  absSweep,
                                                  /* includeHub */ false,
                                                  {},
                                                  [this](const float a) noexcept
    { return priv::ringArcPointAtAngle(outerRadius, outerRadius, a); },
                                                  idFn);

    const auto innerAabb = priv::computeArcBounds(startRad,
                                                  absSweep,
                                                  /* includeHub */ false,
                                                  {},
                                                  [this](const float a) noexcept
    { return priv::ringArcPointAtAngle(outerRadius, innerRadius, a); },
                                                  idFn);

    return priv::mergeAabb(outerAabb, innerAabb);
}


////////////////////////////////////////////////////////////
Rect2f RingPieSliceShapeData::getGlobalBounds() const noexcept
{
    if (!hasVisibleGeometry()) [[unlikely]]
        return {};

    const auto transform = Transform::fromPositionScaleOriginRotation(position, scale, origin, rotation);

    const float        sweepRad   = sweepAngle.asRadians();
    const float        startRad   = startAngle.asRadians();
    const unsigned int arcSamples = pointCount;
    const float        arcStep    = ShapeUtils::computeArcAngleStep(sweepRad, arcSamples);

    return priv::computePolygonBounds(2u * arcSamples,
                                      [&](const unsigned int i) noexcept
    {
        const unsigned int local  = i % arcSamples;
        const float        radius = (i < arcSamples) ? outerRadius : innerRadius;
        return priv::ringArcPointAtAngle(outerRadius, radius, startRad + static_cast<float>(local) * arcStep);
    },
                                      [&](const Vec2f p) noexcept { return transform.transformPoint(p); });
}


////////////////////////////////////////////////////////////
Vec2f RingPieSliceShapeData::getCentroid() const noexcept
{
    const Vec2f center{outerRadius, outerRadius};

    if (!hasVisibleGeometry()) [[unlikely]]
        return center;

    const float sweepRad = sweepAngle.asRadians();
    const float absSweep = ZB_MATH_FABSF(sweepRad);
    const float halfSwp  = absSweep * 0.5f;

    const auto [sinHalf, cosHalf] = zb::sinCosLookup(zb::positiveRemainder(halfSwp, zb::tau));

    const float d = priv::annulusSectorCentroidDistance(outerRadius, innerRadius, absSweep, sinHalf);

    if (d == 0.f) [[unlikely]]
        return center;

    // Bisector direction in the ring convention `(cos, sin)`, evaluated at startAngle + sweep/2.
    // (Works for negative sweep too: the bisector is always the angular midpoint of the sector.)
    const float bisector = startAngle.asRadians() + sweepRad * 0.5f;

    const auto [sinB, cosB] = zb::sinCosLookup(zb::positiveRemainder(bisector, zb::tau));

    return {center.x + d * cosB, center.y + d * sinB};
}

} // namespace za
