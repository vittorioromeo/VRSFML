#pragma once

#include "TextShakeEffect.hpp"
#include "Zancle/Graphics/Color.hpp"
#include "Zancle/System/Priv/Vec2Base.hpp"

#include "ExampleUtils/Progress.hpp"


////////////////////////////////////////////////////////////
struct ComboState
{
    int       combo{0u};
    Countdown comboCountdown;
    int       laserCursorCombo{0};

    int       comboNStars{0};        // Number of stars clicked in current combo
    int       comboNOthers{0};       // Number of non-stars clicked in current combo
    int       comboAccReward{0};     // Accumulated combo reward effect for non-stars
    int       comboAccStarReward{0}; // Accumulated combo reward effect for stars
    Countdown comboFailCountdown;    // Countdown for combo failure effect (red text)

    Countdown accComboDelay;      // Combo reward coin spawns rate
    int       iComboAccReward{0}; // Index of spawned coin in combo reward (used for pitch)

    Countdown accComboStarDelay;      // Combo reward star spawns rate
    int       iComboAccStarReward{0}; // Index of spawned star in combo reward (used for pitch)

    za::Vec2f       baseTextPosition; // Anchor for combo/buff HUD text (mirrors money text origin)
    TextShakeEffect comboTextShakeEffect;

    int       cursorComboLastShown{0};   // Last value rendered by cursor combo text (preserved while it fades out)
    float     cursorComboAlpha{0.f};     // Alpha used by cursor combo text/bar across the same frame
    za::Color cursorComboOutlineColor{}; // Outline color computed by cursor combo text, reused by the bar

    explicit ComboState(const za::Vec2f moneyTextPosition) : baseTextPosition{moneyTextPosition}
    {
    }
};
