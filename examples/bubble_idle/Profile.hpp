#pragma once

#include "Achievements.hpp"
#include "Stats.hpp"

#include "SFML/System/Vec2Base.hpp"


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

    bool showCatRange          = true;
    bool showRangesOnlyOnHover = true;
    bool showCatText           = true;
    bool showParticles         = true;
    bool showTextParticles     = true;
    bool accumulatingCombo     = true;
    bool showCursorComboText   = true;
    bool useBubbleShader       = true;

    int   cursorTrailMode  = 0; // 0=combo, 1=always, 2=off
    float cursorTrailScale = 1.f;

    float bsIridescenceStrength = 0.7f;
    float bsEdgeFactorMin       = 0.f;
    float bsEdgeFactorMax       = 0.6f;
    float bsEdgeFactorStrength  = 1.35f;
    float bsDistortionStrength  = 0.085f;
    float bsBubbleLightness     = 0.15f;
    float bsLensDistortion      = 7.5f;

    float catCloudOpacity       = 1.f;
    int   catCloudCircleCount   = 12;
    float catCloudScale         = 1.4f;
    float catCloudXExtent       = 20.f;
    float catCloudBaseYOffset   = 15.f;
    float catCloudExtraYOffset  = 25.f;
    float catCloudDraggedOffset = 8.f;
    float catCloudLobeLift      = 4.f;
    float catCloudWobbleX       = 2.75f;
    float catCloudWobbleY       = 1.75f;
    float catCloudRadiusBase    = 8.5f;
    float catCloudRadiusLobe    = 5.5f;
    float catCloudRadiusWobble  = 1.25f;

    Stats statsLifetime;

    sf::Vec2u resWidth = {};

    bool windowed = true;
    bool vsync    = true;

    unsigned int frametimeLimit = 144u;

    bool highVisibilityCursor = true;
    bool multicolorCursor     = false;

    float cursorHue   = 0.f;
    float cursorScale = 0.5f;

    bool showCoinParticles = true;
    bool showDpsMeter      = true;

    bool enableNotifications      = true;
    bool showFullManaNotification = true;

    bool unlockedAchievements[nAchievements] = {};

    bool uiUnlocks[128] = {};

    float ppSVibrance   = 0.15f;
    float ppSSaturation = 1.f;
    float ppSLightness  = 1.f;
    float ppSSharpness  = 0.10f;
    float ppSBlur       = 0.f;

    float ppBGVibrance   = 0.1f;
    float ppBGSaturation = 0.65f;
    float ppBGLightness  = 0.95f;
    float ppBGSharpness  = 0.f;
    float ppBGBlur       = 1.f;

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

    int autobatchMode = 2; // 0=off 1=cpu 2=gpu
};
