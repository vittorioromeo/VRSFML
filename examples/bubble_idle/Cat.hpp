#pragma once

#include "Aliases.hpp"
#include "CatType.hpp"
#include "Countdown.hpp"
#include "TextShakeEffect.hpp"
#include "Timer.hpp"

#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Constants.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Math/Sin.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Remainder.hpp"
#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] Cat
{
    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] AstroState
    {
        float startX;

        float velocityX     = 0.f;
        float particleTimer = 0.f;

        bool wrapped = false;
    };

    ////////////////////////////////////////////////////////////
    Timer spawnEffectTimer{};

    sf::Vec2f position;

    float     wobbleRadians{0.f};
    Countdown cooldown;

    sf::Vec2f pawPosition;
    sf::Angle pawRotation{sf::Angle::Zero};

    float pawOpacity = 255.f;

    float hue = 0.f;

    Countdown inspiredCountdown{};
    Countdown boostCountdown{};

    sf::base::SizeT nameIdx;

    TextShakeEffect textStatusShakeEffect{};
    TextShakeEffect textMoneyShakeEffect{};

    sf::base::U32 hits = 0u;

    CatType type;

    sf::base::Optional<BidirectionalTimer> hexedTimer{sf::base::nullOpt};
    sf::base::Optional<BidirectionalTimer> hexedCopyTimer{sf::base::nullOpt};

    MoneyType moneyEarned = 0u;

    sf::base::Optional<AstroState> astroState{sf::base::nullOpt};

    Countdown blinkCountdown{};
    Countdown blinkAnimCountdown{};

    Countdown flapCountdown{};
    Countdown flapAnimCountdown{};

    Countdown yawnCountdown{};
    Countdown yawnAnimCountdown{};

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] inline void update(const float deltaTime)
    {
        textStatusShakeEffect.update(deltaTime);
        textMoneyShakeEffect.update(deltaTime);
        wobbleRadians = sf::base::remainder(wobbleRadians + deltaTime * 0.002f, sf::base::tau);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] inline bool isHexedOrCopyHexed() const
    {
        return hexedTimer.hasValue() || hexedCopyTimer.hasValue();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] inline sf::base::Optional<BidirectionalTimer>& getHexedTimer()
    {
        SFML_BASE_ASSERT(isHexedOrCopyHexed());
        return hexedTimer.hasValue() ? hexedTimer : hexedCopyTimer;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] inline const sf::base::Optional<BidirectionalTimer>& getHexedTimer() const
    {
        SFML_BASE_ASSERT(isHexedOrCopyHexed());
        return hexedTimer.hasValue() ? hexedTimer : hexedCopyTimer;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] inline sf::Vec2f getDrawPosition(const bool enableBobbing) const
    {
        return enableBobbing ? position + sf::Vec2f{0.f, sf::base::sin(wobbleRadians * 2.f) * 7.5f} : position;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline bool updateCooldown(const float deltaTime)
    {
        const float ispiredMult   = inspiredCountdown.updateAndIsActive(deltaTime) ? 2.f : 1.f;
        const float boostMult     = boostCountdown.updateAndIsActive(deltaTime) ? 2.f : 1.f;
        const float cooldownSpeed = ispiredMult * boostMult;

        return cooldown.updateAndStop(deltaTime * cooldownSpeed) == CountdownStatusStop::AlreadyFinished;
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

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] inline bool isCloseToStartX() const noexcept
    {
        SFML_BASE_ASSERT(type == CatType::Astro);
        SFML_BASE_ASSERT(astroState.hasValue());

        return astroState->wrapped && sf::base::fabs(position.x - astroState->startX) < 400.f;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] inline bool isAstroAndInFlight() const noexcept
    {
        return type == CatType::Astro && astroState.hasValue();
    }
};
