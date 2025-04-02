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

    int selectedBGM = 0;

    float minimapScale = 20.f;
    float hudScale     = 1.f;
    float uiScale      = 1.f;
    float uiOffsetX    = 0.f;

    bool tipsEnabled = true;

    float backgroundOpacity  = 100.f;
    int   selectedBackground = 0; // `[0, nShrineTypes + 1)`
    bool  alwaysShowDrawings = false;

    bool showCatRange        = true;
    bool showCatText         = true;
    bool showParticles       = true;
    bool showTextParticles   = true;
    bool accumulatingCombo   = true;
    bool showCursorComboText = true;
    bool useBubbleShader     = true;

    int   cursorTrailMode  = 0; // 0=combo, 1=always, 2=off
    float cursorTrailScale = 1.f;

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
    float cursorScale = 0.45f;

    bool showCoinParticles = true;
    bool showDpsMeter      = true;

    bool enableNotifications      = true;
    bool showFullManaNotification = true;

    bool unlockedAchievements[nAchievements] = {};

    bool uiUnlocks[128] = {};

    float ppSVibrance   = 0.25f;
    float ppSSaturation = 1.0f;
    float ppSLightness  = 1.0f;
    float ppSSharpness  = 0.10f;

    bool showBubbles            = true;
    bool invertMouseButtons     = false;
    bool showDollParticleBorder = true;

    float catDragPressDuration = 110.f;

    bool playWitchRitualSounds = true;
    bool enableScreenShake     = true;
    bool enableCatBobbing      = true;

    float catRangeOutlineThickness = 1.f;

    bool showCursorComboBar = true;

    float sfxVolume = 100.f;

    bool hideMaxedOutPurchasables = false;
    bool hideCategorySeparators   = false;
};
