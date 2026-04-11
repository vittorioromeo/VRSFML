#pragma once

#include "Countdown.hpp"
#include "TextShakeEffect.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/Text.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"


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

    sf::Text        comboText;
    TextShakeEffect comboTextShakeEffect;
    sf::Text        buffText;
    sf::Text        cursorComboText;

    ComboState(const sf::Font&  hudFont,
               const sf::Font&  cursorFont,
               const sf::Vec2f  moneyTextPosition,
               const sf::Color& outlineColor) :
        comboText{hudFont,
                  {
                      .position         = moneyTextPosition.addY(35.f),
                      .string           = "x1",
                      .characterSize    = 48u,
                      .fillColor        = sf::Color::White,
                      .outlineColor     = outlineColor,
                      .outlineThickness = 3.f,
                  }},
        buffText{hudFont,
                 {
                     .position         = moneyTextPosition.addY(70.f),
                     .string           = "",
                     .characterSize    = 48u,
                     .fillColor        = sf::Color::White,
                     .outlineColor     = outlineColor,
                     .outlineThickness = 3.f,
                 }},
        cursorComboText{cursorFont,
                        {
                            .origin           = {0.f, 0.f},
                            .string           = "",
                            .characterSize    = 48u,
                            .outlineThickness = 4.f,
                        }}
    {
    }
};
