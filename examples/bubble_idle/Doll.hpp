#pragma once

#include "CatType.hpp"

#include "ExampleUtils/Progress.hpp"

#include "Zancle/System/Priv/Vec2Base.hpp"

#include "ZancleBase/Constants.hpp"
#include "ZancleBase/Math/Cos.hpp"
#include "ZancleBase/Math/Sin.hpp"
#include "ZancleBase/Remainder.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] Doll
{
    za::Vec2f position;
    float     wobbleRadians;
    float     buffPower;
    float     hue = 0.f;
    CatType   catType;

    TimedCountdown                     tcActivation;
    zb::Optional<TimedCountdown> tcDeath;

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline bool isActive() const
    {
        return tcActivation.isDone();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline float getActivationProgress() const
    {
        return tcActivation.asProgress().getElapsed();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline float getDeathProgress() const
    {
        return getElapsedOr(tcDeath, 0.f);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] inline void update(const float deltaTime)
    {
        wobbleRadians = zb::remainder(wobbleRadians + deltaTime * 0.002f, zb::tau);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] inline za::Vec2f getDrawPosition() const
    {
        const za::Vec2f wobbleOffset{zb::cos(wobbleRadians) * (7.5f + getDeathProgress() * 128.f),
                                     zb::sin(wobbleRadians) * (14.f + getDeathProgress() * 128.f)};

        return position + getActivationProgress() * wobbleOffset;
    }

    ////////////////////////////////////////////////////////////
    static inline constexpr float radius = 32.f;

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::const]] inline constexpr float getRadius() const noexcept
    {
        return radius;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr float getRadiusSquared() const noexcept
    {
        return radius * radius;
    }
};
