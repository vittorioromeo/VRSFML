#pragma once

#include "Aliases.hpp"
#include "CatType.hpp"
#include "Countdown.hpp"
#include "TextShakeEffect.hpp"

#include "ExampleUtils/Timer.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Math/Fabs.hpp"
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

    // While > 0 the per-frame paw lerp is suspended, leaving the paw frozen
    // at whatever pose the current action set it to. Lets the warden's
    // baton-bonk visibly linger on the target cat instead of snapping back
    // the moment the action fires. Ticked down each frame; transient.
    float pawHoldMs = 0.f;

    // Outgoing-swing animation toward `pawPosition`. `pawBonkTravelMs` counts
    // down from `pawBonkTravelDurationMs` so the render can interpolate from
    // the snapshotted start pose over the travel window instead of snapping.
    sf::Vec2f pawBonkStartPos{};
    sf::Angle pawBonkStartRotation{sf::Angle::Zero};
    float     pawBonkTravelMs         = 0.f;
    float     pawBonkTravelDurationMs = 0.f;

    // Return phase: after the hold expires the paw eases back from its last
    // struck pose toward the idle windowsill target over this window. The
    // target is dynamic (it follows the cat), so only the START is cached.
    sf::Vec2f pawBonkReturnStartPos{};
    sf::Angle pawBonkReturnStartRotation{sf::Angle::Zero};
    float     pawBonkReturnMs         = 0.f;
    float     pawBonkReturnDurationMs = 0.f;

    // While > 0 the cat rocks side-to-side like a struck pendulum -- used by
    // the Warden's bonk to give the target a visible reaction. Ticked down
    // each frame; transient.
    float bonkImpactMs = 0.f;

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

    // Napping state. `napTransition` is a 0..1 fade that eases eyes closed on
    // entry and open on exit. `napSleepCountdown` is the remaining sleep time
    // (only counted down once the fade has reached 1). The player can wake
    // the cat early by grabbing and shaking it: `napShakeProgress` (0..1)
    // accumulates drag motion and triggers wake-up on reaching 1.
    // `napWakeWobble` is a transient feedback rotation that decays to zero.
    sf::base::Optional<BidirectionalTimer> napTransition{sf::base::nullOpt};
    sf::base::Optional<Countdown>          napSleepCountdown{sf::base::nullOpt};
    float                                  napShakeProgress{0.f};
    float                                  napWakeWobble{0.f};

    // Transient per-frame kinematics used to derive a velocity-based tilt
    // while being dragged (and also the delta for `napShakeProgress`).
    sf::Vec2f lastFramePosition{};
    float     dragTiltRadians{0.f};

    // Per-cat retrigger countdown for the looping `sleep` sound. Reset to
    // the sound's natural length each time it's (re)started so each napping
    // cat plays its own independent instance.
    float sleepSoundCountdownMs{0.f};

    // Per-cat nap-schedule countdown. Once the sleep mechanic has been
    // introduced (post-tutorial or post-prestige), this ticks down and the
    // cat takes a nap when it reaches zero, then resets to a fresh random
    // value (~2-5 minutes). Pre-introduction it's not ticked.
    float napScheduleCountdownMs{0.f};

    float dragTime{};

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
    [[nodiscard, gnu::always_inline, gnu::pure]] inline bool isNapping() const
    {
        return napTransition.hasValue();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] inline float getNapTransitionValue() const
    {
        return napTransition.hasValue() ? napTransition->value : 0.f;
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
        return enableBobbing ? position + sf::Vec2f{0.f, sf::base::sin(wobbleRadians) * 7.f} : position;
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
