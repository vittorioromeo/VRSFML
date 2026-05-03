#pragma once

#include "CatType.hpp"

#include "ExampleUtils/Progress.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"

#include "SFML/Base/Constants.hpp"
#include "SFML/Base/Math/Cos.hpp"
#include "SFML/Base/Math/Sin.hpp"
#include "SFML/Base/Remainder.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] Doll
{
    sf::Vec2f position;
    float     wobbleRadians;
    float     buffPower;
    float     hue = 0.f;
    CatType   catType;

    TimedCountdown                     tcActivation;
    sf::base::Optional<TimedCountdown> tcDeath;

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
        wobbleRadians = sf::base::remainder(wobbleRadians + deltaTime * 0.002f, sf::base::tau);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] inline sf::Vec2f getDrawPosition() const
    {
        const sf::Vec2f wobbleOffset{sf::base::cos(wobbleRadians) * (7.5f + getDeathProgress() * 128.f),
                                     sf::base::sin(wobbleRadians) * (14.f + getDeathProgress() * 128.f)};

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
