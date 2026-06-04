#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ParticleType.hpp"

#include "Zancle/System/Rect2.hpp"

#include "ZancleBase/GetArraySize.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] MainAtlasRects
{
    ////////////////////////////////////////////////////////////
    // Quick toolbar icons
    za::Rect2f txrIconVolume;
    za::Rect2f txrIconBGM;
    za::Rect2f txrIconBg;
    za::Rect2f txrIconCfg;
    za::Rect2f txrIconCopyCat;

    ////////////////////////////////////////////////////////////
    // Shop menu separator textures
    za::Rect2f txrMenuSeparator0;
    za::Rect2f txrMenuSeparator1;
    za::Rect2f txrMenuSeparator2;
    za::Rect2f txrMenuSeparator3;
    za::Rect2f txrMenuSeparator4;
    za::Rect2f txrMenuSeparator5;
    za::Rect2f txrMenuSeparator6;
    za::Rect2f txrMenuSeparator7;
    za::Rect2f txrMenuSeparator8;

    ////////////////////////////////////////////////////////////
    // Prestige menu separator textures
    za::Rect2f txrPrestigeSeparator0;
    za::Rect2f txrPrestigeSeparator1;
    za::Rect2f txrPrestigeSeparator2;
    za::Rect2f txrPrestigeSeparator3;
    za::Rect2f txrPrestigeSeparator4;
    za::Rect2f txrPrestigeSeparator5;
    za::Rect2f txrPrestigeSeparator6;
    za::Rect2f txrPrestigeSeparator7;
    za::Rect2f txrPrestigeSeparator8;
    za::Rect2f txrPrestigeSeparator9;
    za::Rect2f txrPrestigeSeparator10;
    za::Rect2f txrPrestigeSeparator11;
    za::Rect2f txrPrestigeSeparator12;
    za::Rect2f txrPrestigeSeparator13;
    za::Rect2f txrPrestigeSeparator14;
    za::Rect2f txrPrestigeSeparator15;

    ////////////////////////////////////////////////////////////
    // Magic menu separator textures
    za::Rect2f txrMagicSeparator0;
    za::Rect2f txrMagicSeparator1;
    za::Rect2f txrMagicSeparator2;
    za::Rect2f txrMagicSeparator3;

    ////////////////////////////////////////////////////////////
    // Texture atlas rects
    za::Rect2f txrWhiteDot;
    za::Rect2f txrBubble;
    za::Rect2f txrBubbleStar;
    za::Rect2f txrBubbleNova;
    za::Rect2f txrBubbleGlass;
    za::Rect2f txrCat;

    // Wardencat composite: drawn back-to-front as guardhouse_back, wardencat
    // (with a tail-like body wobble), guardhouse_front, wardencatpaw.
    za::Rect2f txrGuardhouseBack;
    za::Rect2f txrWardenCat;
    za::Rect2f txrGuardhouseFront;
    za::Rect2f txrWardencatPaw;
    za::Rect2f txrUniCat;
    za::Rect2f txrUniCat2;
    za::Rect2f txrUniCatWings;
    za::Rect2f txrDevilCat2;
    za::Rect2f txrDevilCat3;
    za::Rect2f txrDevilCat3Arm;
    za::Rect2f txrDevilCat3Book;
    za::Rect2f txrDevilCat3Tail;
    za::Rect2f txrDevilCat2Book;
    za::Rect2f txrCatPaw;
    za::Rect2f txrCatTail;
    za::Rect2f txrSmartCatHat;
    za::Rect2f txrSmartCatDiploma;
    za::Rect2f txrBrainBack;
    za::Rect2f txrBrainFront;
    za::Rect2f txrUniCatTail;
    za::Rect2f txrUniCat2Tail;
    za::Rect2f txrDevilCatTail2;
    za::Rect2f txrAstroCatTail;
    za::Rect2f txrAstroCatFlag;
    za::Rect2f txrWitchCatTail;
    za::Rect2f txrWizardCatTail;
    za::Rect2f txrMouseCatTail;
    za::Rect2f txrMouseCatMouse;
    za::Rect2f txrEngiCatTail;
    za::Rect2f txrEngiCatWrench;
    za::Rect2f txrRepulsoCatTail;
    za::Rect2f txrAttractoCatTail;
    za::Rect2f txrCopyCatTail;
    za::Rect2f txrAttractoCatMagnet;
    za::Rect2f txrUniCatPaw;
    za::Rect2f txrDevilCatPaw;
    za::Rect2f txrDevilCatPaw2;
    za::Rect2f txrParticle;
    za::Rect2f txrStarParticle;
    za::Rect2f txrFireParticle;
    za::Rect2f txrFireParticle2;
    za::Rect2f txrSmokeParticle;
    za::Rect2f txrExplosionParticle;
    za::Rect2f txrTrailParticle;
    za::Rect2f txrHexParticle;
    za::Rect2f txrShrineParticle;
    za::Rect2f txrCogParticle;
    za::Rect2f txrGlassParticle;
    za::Rect2f txrWitchCat;
    za::Rect2f txrWitchCatPaw;
    za::Rect2f txrAstroCat;
    za::Rect2f txrBomb;
    za::Rect2f txrShrine;
    za::Rect2f txrWizardCat;
    za::Rect2f txrWizardCatPaw;
    za::Rect2f txrMouseCat;
    za::Rect2f txrMouseCatPaw;
    za::Rect2f txrEngiCat;
    za::Rect2f txrEngiCatPaw;
    za::Rect2f txrRepulsoCat;
    za::Rect2f txrRepulsoCatPaw;
    za::Rect2f txrAttractoCat;
    za::Rect2f txrCopyCat;
    za::Rect2f txrDuckCat;
    za::Rect2f txrDuckFlag;
    za::Rect2f txrAttractoCatPaw;
    za::Rect2f txrCopyCatPaw;
    za::Rect2f txrDollNormal;
    za::Rect2f txrDollUni;
    za::Rect2f txrDollDevil;
    za::Rect2f txrDollAstro;
    za::Rect2f txrDollWizard;
    za::Rect2f txrDollMouse;
    za::Rect2f txrDollEngi;
    za::Rect2f txrDollRepulso;
    za::Rect2f txrDollAttracto;
    za::Rect2f txrCoin;
    za::Rect2f txrCatSoul;
    za::Rect2f txrHellPortal;
    za::Rect2f txrCatEyeLid0;
    za::Rect2f txrCatEyeLid1;
    za::Rect2f txrCatEyeLid2;
    za::Rect2f txrCatWhiteEyeLid0;
    za::Rect2f txrCatWhiteEyeLid1;
    za::Rect2f txrCatWhiteEyeLid2;
    za::Rect2f txrCatDarkEyeLid0;
    za::Rect2f txrCatDarkEyeLid1;
    za::Rect2f txrCatDarkEyeLid2;
    za::Rect2f txrCatGrayEyeLid0;
    za::Rect2f txrCatGrayEyeLid1;
    za::Rect2f txrCatGrayEyeLid2;
    za::Rect2f txrCatEars0;
    za::Rect2f txrCatEars1;
    za::Rect2f txrCatEars2;
    za::Rect2f txrCatYawn0;
    za::Rect2f txrCatYawn1;
    za::Rect2f txrCatYawn2;
    za::Rect2f txrCatYawn3;
    za::Rect2f txrCatYawn4;
    za::Rect2f txrCCMaskWitch;
    za::Rect2f txrCCMaskWizard;
    za::Rect2f txrCCMaskMouse;
    za::Rect2f txrCCMaskEngi;
    za::Rect2f txrCCMaskRepulso;
    za::Rect2f txrCCMaskAttracto;
    za::Rect2f txrMMNormal;
    za::Rect2f txrMMUni;
    za::Rect2f txrMMDevil;
    za::Rect2f txrMMAstro;
    za::Rect2f txrMMWitch;
    za::Rect2f txrMMWizard;
    za::Rect2f txrMMMouse;
    za::Rect2f txrMMEngi;
    za::Rect2f txrMMRepulso;
    za::Rect2f txrMMAttracto;
    za::Rect2f txrMMCopy;
    za::Rect2f txrMMDuck;
    za::Rect2f txrMMShrine;
    za::Rect2f txrCloud;

    ////////////////////////////////////////////////////////////
    // Cat animation rects: eye blinking
    const za::Rect2f* eyeLidRects[8]{
        &txrCatEyeLid2,
        &txrCatEyeLid1,
        &txrCatEyeLid0,
        &txrCatEyeLid0,
        &txrCatEyeLid0,
        &txrCatEyeLid0,
        &txrCatEyeLid1,
        &txrCatEyeLid2,
    };

    static constexpr auto nEyeLidRects = zb::getArraySize(&MainAtlasRects::eyeLidRects);

    ////////////////////////////////////////////////////////////
    // Cat animation rects: eye blinking (white)
    const za::Rect2f* whiteEyeLidRects[8]{
        &txrCatWhiteEyeLid2,
        &txrCatWhiteEyeLid1,
        &txrCatWhiteEyeLid0,
        &txrCatWhiteEyeLid0,
        &txrCatWhiteEyeLid0,
        &txrCatWhiteEyeLid0,
        &txrCatWhiteEyeLid1,
        &txrCatWhiteEyeLid2,
    };

    static constexpr auto nWhiteEyeLidRects = zb::getArraySize(&MainAtlasRects::whiteEyeLidRects);

    ////////////////////////////////////////////////////////////
    // Cat animation rects: eye blinking (dark)
    const za::Rect2f* darkEyeLidRects[8]{
        &txrCatDarkEyeLid2,
        &txrCatDarkEyeLid1,
        &txrCatDarkEyeLid0,
        &txrCatDarkEyeLid0,
        &txrCatDarkEyeLid0,
        &txrCatDarkEyeLid0,
        &txrCatDarkEyeLid1,
        &txrCatDarkEyeLid2,
    };

    static constexpr auto nDarkEyeLidRects = zb::getArraySize(&MainAtlasRects::darkEyeLidRects);

    ////////////////////////////////////////////////////////////
    // Cat animation rects: eye blinking (gray)
    const za::Rect2f* grayEyeLidRects[8]{
        &txrCatGrayEyeLid2,
        &txrCatGrayEyeLid1,
        &txrCatGrayEyeLid0,
        &txrCatGrayEyeLid0,
        &txrCatGrayEyeLid0,
        &txrCatGrayEyeLid0,
        &txrCatGrayEyeLid1,
        &txrCatGrayEyeLid2,
    };

    static constexpr auto nGrayEyeLidRects = zb::getArraySize(&MainAtlasRects::grayEyeLidRects);

    ////////////////////////////////////////////////////////////
    // Cat animation rects: ear flapping
    const za::Rect2f* earRects[8]{
        &txrCatEars0,
        &txrCatEars1,
        &txrCatEars2,
        &txrCatEars2,
        &txrCatEars2,
        &txrCatEars2,
        &txrCatEars1,
        &txrCatEars0,
    };

    static constexpr auto nEarRects = zb::getArraySize(&MainAtlasRects::earRects);

    ////////////////////////////////////////////////////////////
    // Cat animation rects: yawning
    const za::Rect2f* catYawnRects[14]{
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

    static constexpr auto nYawnRects = zb::getArraySize(&MainAtlasRects::catYawnRects);

    ///////////////////////////////////////////////////////////
    const za::Rect2f particleRects[nParticleTypes] = {
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
