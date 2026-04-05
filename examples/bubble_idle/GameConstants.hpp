#pragma once

#include "CatType.hpp"

#include "SFML/System/Vec2Base.hpp"

#include "SFML/Base/Array.hpp"


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

    float catNameTextOffsetY    = 52.f;
    float catStatusTextOffsetY  = 72.f;
    float catCooldownBarOffsetY = 4.f;

    bool debugDrawCatCenterMarker = false;
    bool debugDrawCatBodyBounds   = false;
};
