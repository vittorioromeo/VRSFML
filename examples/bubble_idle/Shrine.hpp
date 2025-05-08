#pragma once

#include "Aliases.hpp"
#include "Countdown.hpp"
#include "MathUtils.hpp"
#include "ShrineConstants.hpp"
#include "ShrineType.hpp"
#include "TextShakeEffect.hpp"

#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Constants.hpp"
#include "SFML/Base/Math/Cos.hpp"
#include "SFML/Base/Math/Sin.hpp"
#include "SFML/Base/Remainder.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] Shrine
{
    sf::Vec2f position;

    float wobbleRadians = 0.f;

    OptionalTargetedCountdown tcActivation;
    OptionalTargetedCountdown tcDeath;

    TextShakeEffect textStatusShakeEffect;

    MoneyType collectedReward = 0u;

    ShrineType type;

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline bool isActive() const
    {
        return tcActivation.isDone();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline float getActivationProgress() const
    {
        return tcActivation.getProgress();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline float getDeathProgress() const
    {
        return tcDeath.getProgress();
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] inline void update(const float deltaTime)
    {
        textStatusShakeEffect.update(deltaTime);
        wobbleRadians = sf::base::remainder(wobbleRadians + deltaTime * 0.002f + getDeathProgress() * 0.2f, sf::base::tau);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline sf::Vec2f getDrawPosition() const
    {
        const sf::Vec2f wobbleOffset{sf::base::cos(wobbleRadians) * (7.5f + getDeathProgress() * 128.f),
                                     sf::base::sin(wobbleRadians) * (14.f + getDeathProgress() * 128.f)};

        return position + getActivationProgress() * wobbleOffset;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] inline float getRange() const
    {
        if (!isActive())
            return 0.f;

        return 256.f * (1.f - getDeathProgress());
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] inline float getRangeSquared() const
    {
        const float range = getRange();
        return range * range;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] inline bool isInRange(const sf::Vec2f point) const
    {
        return isActive() && (point - position).lengthSquared() < getRangeSquared();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] inline float getHue() const
    {
        return wrapHue(shrineHues[static_cast<U8>(type)]);
    }

    ////////////////////////////////////////////////////////////
    static inline constexpr float radius = 64.f;

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
