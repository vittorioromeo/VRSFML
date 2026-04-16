#pragma once

#include "BubbleType.hpp"
#include "Countdown.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"

#include "SFML/Base/IntTypes.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] Bubble
{
    sf::Vec2f position;
    sf::Vec2f velocity;

    float radius;
    float rotation;
    float torque;
    float hueMod;

    Countdown repelledCountdown;
    Countdown attractedCountdown;

    BubbleType type;

    // Event-spawned bubbles (e.g. Bubblefall) despawn when they fall off the
    // bottom instead of being recycled at the top.
    bool ephemeral = false;

    // Stable per-bubble counter used to derive the Normal-bubble hue. Assigned
    // once at spawn so the color stays put even when earlier bubbles in the
    // vector get erased (which would otherwise shift every other bubble's
    // index-based hue).
    sf::base::U32 hueSeed = 0u;

    // Combo (invincible) bubble state. After the first click, `comboTimerMs`
    // becomes the bespoke combo window: each subsequent click resets it; if it
    // reaches 0 the bubble pops with payout. `comboClickCount` is the running
    // tally of clicks on this bubble — drives the payout magnitude.
    float         comboTimerMs    = 0.f;
    sf::base::U32 comboClickCount = 0u;

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline float getRadiusSquared() const
    {
        return radius * radius;
    }
};
