#pragma once

#include "Achievements.hpp"
#include "Stats.hpp"

#include "SFML/System/Vector2.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] Profile
{
    float masterVolume = 100.f;
    float musicVolume  = 100.f;

    bool playAudioInBackground = true;
    bool playComboEndSound     = true;

    float minimapScale = 20.f;
    float hudScale     = 1.f;
    float uiScale      = 1.f;

    bool tipsEnabled = true;

    float backgroundOpacity = 100.f;
    float backgroundHue     = 0.f;

    bool showCatText         = true;
    bool showParticles       = true;
    bool showTextParticles   = true;
    bool accumulatingCombo   = true;
    bool showCursorComboText = true;
    bool useBubbleShader     = true;

    float bsIridescenceStrength = 0.7f;
    float bsEdgeFactorMin       = 0.f;
    float bsEdgeFactorMax       = 0.6f;
    float bsEdgeFactorStrength  = 1.35f;
    float bsDistortionStrength  = 0.085f;
    float bsBubbleLightness     = 0.25f;
    float bsLensDistortion      = 6.5f;

    Stats statsLifetime;

    sf::Vector2u resWidth = {};

    bool windowed = true;
    bool vsync    = true;

    unsigned int frametimeLimit = 144u;

    bool highVisibilityCursor = true;
    bool multicolorCursor     = false;

    float cursorHue   = 0.f;
    float cursorScale = 0.4f;

    bool showCoinParticles = true;
    bool showDpsMeter      = true;

    bool showFullManaNotification = true;

    bool unlockedAchievements[nAchievements] = {};

    bool uiUnlocks[128] = {};
};
