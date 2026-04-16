// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/RingPieSliceShapeData.hpp"

#include "SFML/Graphics/Priv/ArcBounds.hpp"
#include "SFML/Graphics/ShapeUtils.hpp"
#include "SFML/Graphics/Transform.hpp"

#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Constants.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/Remainder.hpp"
#include "SFML/Base/SinCosLookup.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
Rect2f RingPieSliceShapeData::getLocalBounds() const noexcept
{
    if (!hasVisibleGeometry()) [[unlikely]]
        return {};

    const float absSweep = SFML_BASE_MATH_FABSF(sweepAngle.asRadians());
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
    const float absSweep = SFML_BASE_MATH_FABSF(sweepRad);
    const float halfSwp  = absSweep * 0.5f;

    const auto [sinHalf, cosHalf] = base::sinCosLookup(base::positiveRemainder(halfSwp, base::tau));

    const float d = priv::annulusSectorCentroidDistance(outerRadius, innerRadius, absSweep, sinHalf);

    if (d == 0.f) [[unlikely]]
        return center;

    // Bisector direction in the ring convention `(cos, sin)`, evaluated at startAngle + sweep/2.
    // (Works for negative sweep too: the bisector is always the angular midpoint of the sector.)
    const float bisector = startAngle.asRadians() + sweepRad * 0.5f;

    const auto [sinB, cosB] = base::sinCosLookup(base::positiveRemainder(bisector, base::tau));

    return {center.x + d * cosB, center.y + d * sinB};
}

} // namespace sf
