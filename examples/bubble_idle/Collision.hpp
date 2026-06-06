#pragma once

#include "Zancle/Geometry/Priv/Vec2Base.hpp"

#include "Zancle/Math/Sqrt.hpp"
#include "Zancle/Vocabulary/Optional.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] CollisionResolution
{
    za::Vec2f iDisplacement;
    za::Vec2f jDisplacement;
    za::Vec2f iVelocityChange;
    za::Vec2f jVelocityChange;
};

////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] inline bool detectCollision(const za::Vec2f iPosition,
                                                      const za::Vec2f jPosition,
                                                      const float     iRadius,
                                                      const float     jRadius)
{
    const za::Vec2f diff            = jPosition - iPosition;
    const float     squaredDistance = diff.lengthSquared();
    const float     sumRadii        = iRadius + jRadius;

    return squaredDistance < sumRadii * sumRadii;
}

////////////////////////////////////////////////////////////
[[nodiscard, gnu::pure]] inline za::Optional<CollisionResolution> handleCollision(
    const float     deltaTimeMs,
    const za::Vec2f iPosition,
    const za::Vec2f jPosition,
    const za::Vec2f iVelocity,
    const za::Vec2f jVelocity,
    const float     iRadius,
    const float     jRadius,
    const float     iMassMult,
    const float     jMassMult)
{
    const za::Vec2f diff            = jPosition - iPosition;
    const float     squaredDistance = diff.lengthSquared();
    const float     sumRadii        = iRadius + jRadius;

    if (squaredDistance >= sumRadii * sumRadii)
        return za::nullOpt;

    const float distance = za::sqrt(squaredDistance);

    const za::Vec2f normal = (distance > 0.f) ? (diff / distance) : za::Vec2f{1.f, 0.f};

    // Move the bubbles apart based on their masses (heavier bubbles move less)
    const float m1           = iRadius * iRadius * iMassMult; // Mass of bubble i (quadratic scaling)
    const float m2           = jRadius * jRadius * jMassMult; // Mass of bubble j (quadratic scaling)
    const float totalMassInv = 1.f / (m1 + m2);
    const float invM1        = 1.f / m1;
    const float invM2        = 1.f / m2;

    // Velocity resolution calculations
    const float vRelDotNormal = (iVelocity - jVelocity).dot(normal);

    za::Vec2f velocityChangeI;
    za::Vec2f velocityChangeJ;

    // Only apply impulse if bubbles are moving towards each other
    if (vRelDotNormal > 0.f)
    {
        constexpr float e = 0.65f; // Coefficient of restitution (1.0 = perfectly elastic)
        const float     j = -(1.f + e) * vRelDotNormal / (invM1 + invM2);

        const za::Vec2f impulse = normal * j;

        velocityChangeI = impulse * invM1;
        velocityChangeJ = -impulse * invM2;
    }

    // Define a "softness" factor to control how quickly the overlap is resolved
    const float softnessFactor = 0.0075f * deltaTimeMs;

    const float     overlap      = sumRadii - distance; // Amount of overlap
    const za::Vec2f displacement = normal * overlap * softnessFactor;

    return za::makeOptional<CollisionResolution>( //
        /* iDisplacement */ -displacement * (m2 * totalMassInv),
        /* jDisplacement */ displacement * (m1 * totalMassInv),
        /* iVelocityChange */ velocityChangeI,
        /* jVelocityChange */ velocityChangeJ);
}
