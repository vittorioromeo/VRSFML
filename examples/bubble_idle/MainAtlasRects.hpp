#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ParticleType.hpp"

#include "SFML/System/Rect2.hpp"

#include "SFML/Base/GetArraySize.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] MainAtlasRects
{
    ////////////////////////////////////////////////////////////
    // Quick toolbar icons
    sf::Rect2f txrIconVolume;
    sf::Rect2f txrIconBGM;
    sf::Rect2f txrIconBg;
    sf::Rect2f txrIconCfg;
    sf::Rect2f txrIconCopyCat;

    ////////////////////////////////////////////////////////////
    // Shop menu separator textures
    sf::Rect2f txrMenuSeparator0;
    sf::Rect2f txrMenuSeparator1;
    sf::Rect2f txrMenuSeparator2;
    sf::Rect2f txrMenuSeparator3;
    sf::Rect2f txrMenuSeparator4;
    sf::Rect2f txrMenuSeparator5;
    sf::Rect2f txrMenuSeparator6;
    sf::Rect2f txrMenuSeparator7;
    sf::Rect2f txrMenuSeparator8;

    ////////////////////////////////////////////////////////////
    // Prestige menu separator textures
    sf::Rect2f txrPrestigeSeparator0;
    sf::Rect2f txrPrestigeSeparator1;
    sf::Rect2f txrPrestigeSeparator2;
    sf::Rect2f txrPrestigeSeparator3;
    sf::Rect2f txrPrestigeSeparator4;
    sf::Rect2f txrPrestigeSeparator5;
    sf::Rect2f txrPrestigeSeparator6;
    sf::Rect2f txrPrestigeSeparator7;
    sf::Rect2f txrPrestigeSeparator8;
    sf::Rect2f txrPrestigeSeparator9;
    sf::Rect2f txrPrestigeSeparator10;
    sf::Rect2f txrPrestigeSeparator11;
    sf::Rect2f txrPrestigeSeparator12;
    sf::Rect2f txrPrestigeSeparator13;
    sf::Rect2f txrPrestigeSeparator14;
    sf::Rect2f txrPrestigeSeparator15;

    ////////////////////////////////////////////////////////////
    // Magic menu separator textures
    sf::Rect2f txrMagicSeparator0;
    sf::Rect2f txrMagicSeparator1;
    sf::Rect2f txrMagicSeparator2;
    sf::Rect2f txrMagicSeparator3;

    ////////////////////////////////////////////////////////////
    // Texture atlas rects
    sf::Rect2f txrWhiteDot;
    sf::Rect2f txrBubble;
    sf::Rect2f txrBubbleStar;
    sf::Rect2f txrBubbleNova;
    sf::Rect2f txrBubbleGlass;
    sf::Rect2f txrCat;

    // Wardencat composite: drawn back-to-front as guardhouse_back, wardencat
    // (with a tail-like body wobble), guardhouse_front, wardencatpaw.
    sf::Rect2f txrGuardhouseBack;
    sf::Rect2f txrWardenCat;
    sf::Rect2f txrGuardhouseFront;
    sf::Rect2f txrWardencatPaw;
    sf::Rect2f txrUniCat;
    sf::Rect2f txrUniCat2;
    sf::Rect2f txrUniCatWings;
    sf::Rect2f txrDevilCat2;
    sf::Rect2f txrDevilCat3;
    sf::Rect2f txrDevilCat3Arm;
    sf::Rect2f txrDevilCat3Book;
    sf::Rect2f txrDevilCat3Tail;
    sf::Rect2f txrDevilCat2Book;
    sf::Rect2f txrCatPaw;
    sf::Rect2f txrCatTail;
    sf::Rect2f txrSmartCatHat;
    sf::Rect2f txrSmartCatDiploma;
    sf::Rect2f txrBrainBack;
    sf::Rect2f txrBrainFront;
    sf::Rect2f txrUniCatTail;
    sf::Rect2f txrUniCat2Tail;
    sf::Rect2f txrDevilCatTail2;
    sf::Rect2f txrAstroCatTail;
    sf::Rect2f txrAstroCatFlag;
    sf::Rect2f txrWitchCatTail;
    sf::Rect2f txrWizardCatTail;
    sf::Rect2f txrMouseCatTail;
    sf::Rect2f txrMouseCatMouse;
    sf::Rect2f txrEngiCatTail;
    sf::Rect2f txrEngiCatWrench;
    sf::Rect2f txrRepulsoCatTail;
    sf::Rect2f txrAttractoCatTail;
    sf::Rect2f txrCopyCatTail;
    sf::Rect2f txrAttractoCatMagnet;
    sf::Rect2f txrUniCatPaw;
    sf::Rect2f txrDevilCatPaw;
    sf::Rect2f txrDevilCatPaw2;
    sf::Rect2f txrParticle;
    sf::Rect2f txrStarParticle;
    sf::Rect2f txrFireParticle;
    sf::Rect2f txrFireParticle2;
    sf::Rect2f txrSmokeParticle;
    sf::Rect2f txrExplosionParticle;
    sf::Rect2f txrTrailParticle;
    sf::Rect2f txrHexParticle;
    sf::Rect2f txrShrineParticle;
    sf::Rect2f txrCogParticle;
    sf::Rect2f txrGlassParticle;
    sf::Rect2f txrWitchCat;
    sf::Rect2f txrWitchCatPaw;
    sf::Rect2f txrAstroCat;
    sf::Rect2f txrBomb;
    sf::Rect2f txrShrine;
    sf::Rect2f txrWizardCat;
    sf::Rect2f txrWizardCatPaw;
    sf::Rect2f txrMouseCat;
    sf::Rect2f txrMouseCatPaw;
    sf::Rect2f txrEngiCat;
    sf::Rect2f txrEngiCatPaw;
    sf::Rect2f txrRepulsoCat;
    sf::Rect2f txrRepulsoCatPaw;
    sf::Rect2f txrAttractoCat;
    sf::Rect2f txrCopyCat;
    sf::Rect2f txrDuckCat;
    sf::Rect2f txrDuckFlag;
    sf::Rect2f txrAttractoCatPaw;
    sf::Rect2f txrCopyCatPaw;
    sf::Rect2f txrDollNormal;
    sf::Rect2f txrDollUni;
    sf::Rect2f txrDollDevil;
    sf::Rect2f txrDollAstro;
    sf::Rect2f txrDollWizard;
    sf::Rect2f txrDollMouse;
    sf::Rect2f txrDollEngi;
    sf::Rect2f txrDollRepulso;
    sf::Rect2f txrDollAttracto;
    sf::Rect2f txrCoin;
    sf::Rect2f txrCatSoul;
    sf::Rect2f txrHellPortal;
    sf::Rect2f txrCatEyeLid0;
    sf::Rect2f txrCatEyeLid1;
    sf::Rect2f txrCatEyeLid2;
    sf::Rect2f txrCatWhiteEyeLid0;
    sf::Rect2f txrCatWhiteEyeLid1;
    sf::Rect2f txrCatWhiteEyeLid2;
    sf::Rect2f txrCatDarkEyeLid0;
    sf::Rect2f txrCatDarkEyeLid1;
    sf::Rect2f txrCatDarkEyeLid2;
    sf::Rect2f txrCatGrayEyeLid0;
    sf::Rect2f txrCatGrayEyeLid1;
    sf::Rect2f txrCatGrayEyeLid2;
    sf::Rect2f txrCatEars0;
    sf::Rect2f txrCatEars1;
    sf::Rect2f txrCatEars2;
    sf::Rect2f txrCatYawn0;
    sf::Rect2f txrCatYawn1;
    sf::Rect2f txrCatYawn2;
    sf::Rect2f txrCatYawn3;
    sf::Rect2f txrCatYawn4;
    sf::Rect2f txrCCMaskWitch;
    sf::Rect2f txrCCMaskWizard;
    sf::Rect2f txrCCMaskMouse;
    sf::Rect2f txrCCMaskEngi;
    sf::Rect2f txrCCMaskRepulso;
    sf::Rect2f txrCCMaskAttracto;
    sf::Rect2f txrMMNormal;
    sf::Rect2f txrMMUni;
    sf::Rect2f txrMMDevil;
    sf::Rect2f txrMMAstro;
    sf::Rect2f txrMMWitch;
    sf::Rect2f txrMMWizard;
    sf::Rect2f txrMMMouse;
    sf::Rect2f txrMMEngi;
    sf::Rect2f txrMMRepulso;
    sf::Rect2f txrMMAttracto;
    sf::Rect2f txrMMCopy;
    sf::Rect2f txrMMDuck;
    sf::Rect2f txrMMShrine;
    sf::Rect2f txrCloud;

    ////////////////////////////////////////////////////////////
    // Cat animation rects: eye blinking
    const sf::Rect2f* eyeLidRects[8]{
        &txrCatEyeLid2,
        &txrCatEyeLid1,
        &txrCatEyeLid0,
        &txrCatEyeLid0,
        &txrCatEyeLid0,
        &txrCatEyeLid0,
        &txrCatEyeLid1,
        &txrCatEyeLid2,
    };

    static constexpr auto nEyeLidRects = sf::base::getArraySize(&MainAtlasRects::eyeLidRects);

    ////////////////////////////////////////////////////////////
    // Cat animation rects: eye blinking (white)
    const sf::Rect2f* whiteEyeLidRects[8]{
        &txrCatWhiteEyeLid2,
        &txrCatWhiteEyeLid1,
        &txrCatWhiteEyeLid0,
        &txrCatWhiteEyeLid0,
        &txrCatWhiteEyeLid0,
        &txrCatWhiteEyeLid0,
        &txrCatWhiteEyeLid1,
        &txrCatWhiteEyeLid2,
    };

    static constexpr auto nWhiteEyeLidRects = sf::base::getArraySize(&MainAtlasRects::whiteEyeLidRects);

    ////////////////////////////////////////////////////////////
    // Cat animation rects: eye blinking (dark)
    const sf::Rect2f* darkEyeLidRects[8]{
        &txrCatDarkEyeLid2,
        &txrCatDarkEyeLid1,
        &txrCatDarkEyeLid0,
        &txrCatDarkEyeLid0,
        &txrCatDarkEyeLid0,
        &txrCatDarkEyeLid0,
        &txrCatDarkEyeLid1,
        &txrCatDarkEyeLid2,
    };

    static constexpr auto nDarkEyeLidRects = sf::base::getArraySize(&MainAtlasRects::darkEyeLidRects);

    ////////////////////////////////////////////////////////////
    // Cat animation rects: eye blinking (gray)
    const sf::Rect2f* grayEyeLidRects[8]{
        &txrCatGrayEyeLid2,
        &txrCatGrayEyeLid1,
        &txrCatGrayEyeLid0,
        &txrCatGrayEyeLid0,
        &txrCatGrayEyeLid0,
        &txrCatGrayEyeLid0,
        &txrCatGrayEyeLid1,
        &txrCatGrayEyeLid2,
    };

    static constexpr auto nGrayEyeLidRects = sf::base::getArraySize(&MainAtlasRects::grayEyeLidRects);

    ////////////////////////////////////////////////////////////
    // Cat animation rects: ear flapping
    const sf::Rect2f* earRects[8]{
        &txrCatEars0,
        &txrCatEars1,
        &txrCatEars2,
        &txrCatEars2,
        &txrCatEars2,
        &txrCatEars2,
        &txrCatEars1,
        &txrCatEars0,
    };

    static constexpr auto nEarRects = sf::base::getArraySize(&MainAtlasRects::earRects);

    ////////////////////////////////////////////////////////////
    // Cat animation rects: yawning
    const sf::Rect2f* catYawnRects[14]{
        &txrCatYawn0,
        &txrCatYawn1,
        &txrCatYawn2,
        &txrCatYawn3,
        &txrCatYawn4,
        &txrCatYawn4,
        &txrCatYawn4,
        &txrCatYawn4,
        &txrCatYawn4,
        &txrCatYawn4,
        &txrCatYawn3,
        &txrCatYawn2,
        &txrCatYawn1,
        &txrCatYawn0,
    };

    static constexpr auto nYawnRects = sf::base::getArraySize(&MainAtlasRects::catYawnRects);

    ///////////////////////////////////////////////////////////
    const sf::Rect2f particleRects[nParticleTypes] = {
        txrParticle,
        txrStarParticle,
        txrFireParticle,
        txrHexParticle,
        txrShrineParticle,
        txrMouseCatPaw,
        txrCogParticle,
        txrCoin,
        txrCatSoul,
        txrFireParticle2,
        txrSmokeParticle,
        txrExplosionParticle,
        txrTrailParticle,
        txrGlassParticle,
    };
};
