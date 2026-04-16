#pragma once

#include "CatType.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"

#include "SFML/Base/Array.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] GameConstants
{
    struct [[nodiscard]] CloudModifier
    {
        sf::Vec2f positionOffset{};
        float     xExtentMult = 1.f;
    };

    struct [[nodiscard]] SpriteAttachment
    {
        sf::Vec2f positionOffset{};
        sf::Vec2f origin{};
    };

    sf::base::Array<sf::Vec2f, nCatTypes> catTailOffsetsByType{{
        {-40.f, 0.f},    // Normal
        {-35.f, -222.f}, // Uni
        {-8.f, 2.f},     // Devil
        {56.f, -80.f},   // Astro
        {-40.f, 0.f},    // Warden -- TODO: dedicated assets (reusing Normal)
        {37.f, 165.f},   // Witch
        {-90.f, 120.f},  // Wizard
        {0.f, 0.f},      // Mouse
        {2.f, 43.f},     // Engi
        {4.f, -29.f},    // Repulso
        {0.f, 0.f},      // Attracto
        {0.f, 0.f},      // Copy
        {0.f, 0.f},      // Duck
    }};

    sf::base::Array<sf::Vec2f, nCatTypes> catDrawOffsetsByType{{
        {0.f, 0.f}, // Normal
        {0.f, 0.f}, // Uni
        {0.f, 0.f}, // Devil
        {0.f, 0.f}, // Astro
        {0.f, 0.f}, // Warden -- TODO: dedicated assets
        {0.f, 0.f}, // Witch
        {0.f, 0.f}, // Wizard
        {0.f, 0.f}, // Mouse
        {0.f, 0.f}, // Engi
        {0.f, 0.f}, // Repulso
        {0.f, 0.f}, // Attracto
        {0.f, 0.f}, // Copy
        {0.f, 0.f}, // Duck
    }};

    sf::base::Array<sf::Vec2f, nCatTypes> catEyeOffsetsByType{{
        {-40.f, 0.f},    // Normal
        {-35.f, -222.f}, // Uni
        {-8.f, 2.f},     // Devil
        {56.f, -80.f},   // Astro
        {-40.f, 0.f},    // Warden -- TODO: dedicated assets (reusing Normal)
        {37.f, 165.f},   // Witch
        {-25.f, 65.f},   // Wizard
        {0.f, 0.f},      // Mouse
        {2.f, 43.f},     // Engi
        {4.f, -29.f},    // Repulso
        {0.f, 0.f},      // Attracto
        {0.f, 0.f},      // Copy
        {0.f, 0.f},      // Duck
    }};

    sf::base::Array<float, nCatTypes> catHueByType{{
        0.f,    // Normal
        160.f,  // Uni
        -25.f,  // Devil
        0.f,    // Astro
        45.f,   // Warden -- TODO: dedicated assets
        80.f,   // Witch
        -135.f, // Wizard
        0.f,    // Mouse
        -10.f,  // Engi
        -40.f,  // Repulso
        0.f,    // Attracto
        0.f,    // Copy
        0.f,    // Duck
    }};

    sf::base::Array<CloudModifier, nCatTypes> cloudModifiers{{
        CloudModifier{{0.f, 0.f}, 1.f},    // Normal
        CloudModifier{{0.f, -10.f}, 1.5f}, // Uni
        CloudModifier{{0.f, 0.f}, 1.5f},   // Devil
        CloudModifier{{0.f, -17.f}, 2.f},  // Astro
        CloudModifier{{0.f, 0.f}, 1.f},    // Warden -- TODO: dedicated assets
        CloudModifier{{0.f, -5.f}, 1.2f},  // Witch
        CloudModifier{{0.f, -5.f}, 1.5f},  // Wizard
        CloudModifier{{0.f, 0.f}, 1.f},    // Mouse
        CloudModifier{{0.f, 0.f}, 1.f},    // Engi
        CloudModifier{{0.f, 0.f}, 1.f},    // Repulso
        CloudModifier{{0.f, 0.f}, 1.f},    // Attracto
        CloudModifier{{0.f, 0.f}, 1.f},    // Copy
        CloudModifier{{0.f, 0.f}, 1.f},    // Duck
    }};

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

    float catAttachmentDraggedOffsetY = 75.f;

    SpriteAttachment devilBackTail{{905.f, 10.f}, {320.f, 32.f}};
    sf::Vec2f        brainJarOffset{210.f, -235.f};
    sf::Vec2f        uniWingsOffset{250.f, -175.f};
    sf::Vec2f        uniWingsOriginOffsetFromCenter{35.f, 10.f};
    sf::Vec2f        devilBookOffset{10.f, 20.f};
    sf::Vec2f        devilPawIdleOffset{4.f, 2.f};
    sf::Vec2f        devilPawDraggedOffset{-6.f, 6.f};
    SpriteAttachment duckFlag{{335.f, -65.f}, {98.f, 330.f}};
    sf::Vec2f        smartHatOffset{-150.f, -535.f};
    sf::Vec2f        earFlapOffset{-131.f, -365.f};
    sf::Vec2f        yawnOffset{-221.f, 25.f};
    SpriteAttachment smartDiploma{{295.f, 355.f}, {23.f, 150.f}};
    SpriteAttachment astroFlag{{395.f, 225.f}, {98.f, 330.f}};
    SpriteAttachment engiWrench{{295.f, 385.f}, {36.f, 167.f}};
    SpriteAttachment attractoMagnet{{190.f, 315.f}, {142.f, 254.f}};
    SpriteAttachment tail{{475.f, 240.f}, {320.f, 32.f}};
    sf::Vec2f        uniTailExtraOffset{-130.f, 405.f};
    sf::Vec2f        uniTailOriginOffset{250.f, 0.f};
    SpriteAttachment mouseProp{{-275.f, -15.f}, {53.f, 77.f}};
    sf::Vec2f        eyelidOffset{-185.f, -185.f};
    sf::Vec2f        regularPawIdleOffset{0.f, 0.f};
    sf::Vec2f        regularPawDraggedOffset{-12.f, 12.f};
    sf::Vec2f        copyMaskOffset{265.f, 115.f};
    sf::Vec2f        copyMaskOrigin{353.f, 295.f};

    // Wardencat composite layout. Offsets are applied on top of the cat's
    // `visualCatAnchor` and inherit the usual cat scaling. The guardhouse
    // parts are drawn without rotation so they don't visibly swing while the
    // cat body wobbles; only the cat adds an extra tail-like rock.
    sf::Vec2f wardenGuardhouseBackOffset{0.f, 0.f};
    sf::Vec2f wardenGuardhouseFrontOffset{0.f, 0.f};
    sf::Vec2f wardenCatBodyOffset{0.f, 0.f};
    sf::Vec2f wardenCatPawOffset{0.f, 0.f};
    float     wardenCatBodyWobbleRadians = 0.12f;

    // Rotation-pivot offsets added to the face sprites' origins so the
    // eyelids / yawn rotate around a point that actually lines up with the
    // wardencat body rather than their own texture centers.
    sf::Vec2f wardenCatEyelidOriginOffset{0.f, 0.f};
    sf::Vec2f wardenCatYawnOriginOffset{0.f, 0.f};

    // Scale multiplier applied on top of the shared cat scale for the Warden
    // paw specifically. Handy because the paw is always visible for Warden
    // and its art may not match the default cat-paw proportions.
    float wardenCatPawScale = 1.f;

    float catNameTextOffsetY    = 52.f;
    float catStatusTextOffsetY  = 72.f;
    float catCooldownBarOffsetY = 4.f;

    bool debugDrawCatCenterMarker = false;
    bool debugDrawCatBodyBounds   = false;

    ////////////////////////////////////////////////////////////
    // Random event system tuning. Lives here so it round-trips through
    // `game_constants.json` and can be tweaked live from the debug menu.
    struct [[nodiscard]] BubblefallTuning
    {
        float           durationMs       = 12'000.f;
        float           regionWidth      = 240.f;
        float           spawnIntervalMs  = 10.f;
        sf::base::SizeT bubblesPerTick   = 3u;
        float           initialVelocityY = 0.55f;
        float           velocityJitterY  = 0.25f;
        float           velocityJitterX  = 0.05f;

        // Fraction of `durationMs` spent ramping the spawn rate up from zero
        // at the start (`attackRatio`) and down to zero at the end
        // (`releaseRatio`). The middle `1 - attack - release` is at full rate.
        // Clamped ∈ [0, 0.5] in aggregate at runtime.
        float attackRatio  = 0.15f;
        float releaseRatio = 0.15f;
    };

    struct [[nodiscard]] InvincibleBubbleTuning
    {
        float minRadius          = 0.05f * 256.f; // smaller than normal bubbles
        float maxRadius          = 0.08f * 256.f;
        float initialVelocityY   = 0.04f; // gentle initial drop
        float velocityJitterY    = 0.015f;
        float velocityJitterX    = 0.04f;
        float maxVelocityY       = 0.06f; // hard cap on fall speed
        float spawnYOffsetTopMin = 1.f;
        float spawnYOffsetTopMax = 3.f;

        // Combo mechanic tuning.
        float         comboTimerMaxMs      = 1000.f;  // window after each click before the bubble pops
        sf::base::U32 maxClicks            = 25u;     // auto-pop after this many clicks
        float         rewardScalePerClick  = 5.f;     // base coins per click
        float         rewardClickExponent  = 1.5f;    // reward = base * pow(clicks, exp)
        float         ambientRepelRadius   = 128.f;   // gentle "stand-out" push while alive
        float         ambientRepelStrength = 0.0018f; // per-ms velocity push factor
        float         popRepelRadius       = 360.f;   // burst on pop
        float         popRepelImpulse      = 0.95f;   // one-shot velocity kick

        // Per-click visual feedback: coins-of-energy fly into the bubble and
        // it puffs up slightly to telegraph progress.
        float clickAbsorbRadius    = 64.f;  // ring around the bubble where particles spawn
        float clickAbsorbSpeed     = 0.45f; // px/ms inward
        float radiusGrowthPerClick = 0.4f;  // px added per click
        float radiusGrowthMax      = 18.f;  // total px added cap

        // Spawn position safety margin so the bubble never appears flush
        // against either horizontal edge of the map.
        float         spawnEdgeMarginPx   = 300.f;
        float         payoutCoinDelayMs   = 35.f; // ms between rising-pitch coin collections
        sf::base::U32 payoutCoinsPerClick = 2u;   // coins spewed per click
        sf::base::U32 payoutMaxCoins      = 80u;  // hard cap on the spew

        // Burst phase: coins explode outward from the bubble, damp to a stop,
        // then transition into the rising-pitch collection sequence.
        float burstSpeedMin      = 0.55f; // initial outward speed (px/ms)
        float burstSpeedMax      = 0.95f;
        float burstDampingPerSec = 0.005f; // fraction of velocity left per second (smaller = more damping)
        float burstSettleDelayMs = 350.f;  // wait between burst start and collection start
    };

    struct [[nodiscard]] EventsTuning
    {
        float minSpawnIntervalMs = 45'000.f;
        float maxSpawnIntervalMs = 120'000.f;

        BubblefallTuning       bubblefall{};
        InvincibleBubbleTuning invincibleBubble{};
    };

    EventsTuning events{};
};
