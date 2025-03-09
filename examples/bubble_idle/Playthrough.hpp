#pragma once

#include "Aliases.hpp"
#include "Bubble.hpp"
#include "Cat.hpp"
#include "CatConstants.hpp"
#include "CatType.hpp"
#include "Constants.hpp"
#include "Doll.hpp"
#include "ExactArray.hpp"
#include "HellPortal.hpp"
#include "Milestones.hpp"
#include "PSVDataConstants.hpp"
#include "PurchasableScalingValue.hpp"
#include "RNG.hpp"
#include "Shrine.hpp"
#include "ShrineType.hpp"
#include "Stats.hpp"

#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Math/Pow.hpp"

#include <vector>


////////////////////////////////////////////////////////////
struct BubbleIgnoreFlags
{
    bool normal = false;
    bool star   = false;
    bool bomb   = false;
};

////////////////////////////////////////////////////////////
struct Playthrough
{
    //
    // RNG stuff
    RNG::SeedType seed;
    SizeT         nextCatNamePerType[nCatTypes] = {};

    //
    // PSV instances
    PurchasableScalingValue psvComboStartTime{&PSVDataConstants::comboStartTime};
    PurchasableScalingValue psvMapExtension{&PSVDataConstants::mapExtension};
    PurchasableScalingValue psvShrineActivation{&PSVDataConstants::shrineActivation};
    PurchasableScalingValue psvBubbleCount{&PSVDataConstants::bubbleCount};
    PurchasableScalingValue psvSpellCount{&PSVDataConstants::spellCount};
    PurchasableScalingValue psvBubbleValue{&PSVDataConstants::bubbleValue}; // also tracks prestige level
    PurchasableScalingValue psvExplosionRadiusMult{&PSVDataConstants::explosionRadiusMult};
    PurchasableScalingValue psvStarpawPercentage{&PSVDataConstants::starpawPercentage};     // uses WPs
    PurchasableScalingValue psvMewltiplierMult{&PSVDataConstants::mewltiplierMult};         // uses WPs
    PurchasableScalingValue psvDarkUnionPercentage{&PSVDataConstants::darkUnionPercentage}; // uses WPs

    EXACT_ARRAY(PurchasableScalingValue,
                psvPerCatType,
                nCatTypes,
                {
                    {&PSVDataConstants::catNormal},
                    {&PSVDataConstants::catUni},
                    {&PSVDataConstants::catDevil},
                    {&PSVDataConstants::catAstro},

                    {&PSVDataConstants::catWitch},    // unused
                    {&PSVDataConstants::catWizard},   // unused
                    {&PSVDataConstants::catMouse},    // unused
                    {&PSVDataConstants::catEngi},     // unused
                    {&PSVDataConstants::catRepulso},  // unused
                    {&PSVDataConstants::catAttracto}, // unused
                    {&PSVDataConstants::catCopy},     // unused
                });

    EXACT_ARRAY(PurchasableScalingValue,
                psvCooldownMultsPerCatType,
                nCatTypes,
                {
                    {&PSVDataConstants::catNormalCooldownMult},
                    {&PSVDataConstants::catUniCooldownMult},
                    {&PSVDataConstants::catDevilCooldownMult},
                    {&PSVDataConstants::catAstroCooldownMult},

                    {&PSVDataConstants::catWitchCooldownMult},
                    {&PSVDataConstants::catWizardCooldownMult},
                    {&PSVDataConstants::catMouseCooldownMult},
                    {&PSVDataConstants::catEngiCooldownMult},
                    {&PSVDataConstants::catRepulsoCooldownMult},
                    {&PSVDataConstants::catAttractoCooldownMult},
                    {&PSVDataConstants::catCopyCooldownMult}, // unused
                });

    EXACT_ARRAY(PurchasableScalingValue,
                psvRangeDivsPerCatType,
                nCatTypes,
                {
                    {&PSVDataConstants::catNormalRangeDiv},
                    {&PSVDataConstants::catUniRangeDiv},
                    {&PSVDataConstants::catDevilRangeDiv},
                    {&PSVDataConstants::catAstroRangeDiv},

                    {&PSVDataConstants::catWitchRangeDiv},
                    {&PSVDataConstants::catWizardRangeDiv},
                    {&PSVDataConstants::catMouseRangeDiv},
                    {&PSVDataConstants::catEngiRangeDiv},
                    {&PSVDataConstants::catRepulsoRangeDiv},
                    {&PSVDataConstants::catAttractoRangeDiv},
                    {&PSVDataConstants::catCopyRangeDiv}, // unused
                });

    //
    // Permanent PSV instances
    PurchasableScalingValue psvPPMultiPopRange{&PSVDataConstants::multiPopRange};
    PurchasableScalingValue psvPPInspireDurationMult{&PSVDataConstants::inspireDurationMult};
    PurchasableScalingValue psvPPManaCooldownMult{&PSVDataConstants::manaCooldownMult};
    PurchasableScalingValue psvPPManaMaxMult{&PSVDataConstants::manaMaxMult};
    PurchasableScalingValue psvPPMouseCatGlobalBonusMult{&PSVDataConstants::mouseCatGlobalBonusMult};
    PurchasableScalingValue psvPPEngiCatGlobalBonusMult{&PSVDataConstants::engiCatGlobalBonusMult};
    PurchasableScalingValue psvPPRepulsoCatConverterChance{&PSVDataConstants::repulsoCatConverterChance};
    PurchasableScalingValue psvPPWitchCatBuffDuration{&PSVDataConstants::witchCatBuffDuration};
    PurchasableScalingValue psvPPUniRitualBuffPercentage{&PSVDataConstants::uniRitualBuffPercentage};
    PurchasableScalingValue psvPPDevilRitualBuffPercentage{&PSVDataConstants::devilRitualBuffPercentage};

    //
    // Currencies
    MoneyType money = 0u;

    //
    // Permanent currencies
    PrestigePointsType prestigePoints = 0u;

    //
    // Purchases
    bool comboPurchased = false;
    bool mapPurchased   = false;

    //
    // Magic
    float     manaTimer            = 0.f;
    ManaType  mana                 = 0u;
    bool      absorbingWisdom      = false;
    MoneyType wisdom               = 0u;
    float     mewltiplierAuraTimer = 0.f;

    //
    // Mousecat
    int       mouseCatCombo = 0u;
    Countdown mouseCatComboCountdown;

    // Copycat
    CatType copycatCopiedCatType{CatType::Witch};

    //
    // Permanent purchases
    struct Permanent
    {
        bool starterPackPurchased = false;

        bool multiPopPurchased   = false;
        bool smartCatsPurchased  = false;
        bool geniusCatsPurchased = false; // if true, `smartCatsPurchased` must be true

        bool windPurchased = false;

        bool astroCatInspirePurchased = false;

        bool starpawConversionIgnoreBombs = false;
        bool starpawNova                  = false;

        bool repulsoCatFilterPurchased        = false;
        bool repulsoCatConverterPurchased     = false;
        bool repulsoCatNovaConverterPurchased = false;

        bool attractoCatFilterPurchased = false;

        bool witchCatBuffPowerScalesWithNCats   = false;
        bool witchCatBuffPowerScalesWithMapSize = false;
        bool witchCatBuffFewerDolls             = false;
        bool witchCatBuffFlammableDolls         = false;
        bool witchCatBuffOrbitalDolls           = false;

        bool shrineCompletedOnceByCatType[nCatTypes] = {};
        bool unsealedByType[nCatTypes]               = {};

        bool wizardCatDoubleMewltiplierDuration = false;

        bool unicatTranscendencePurchased    = false;
        bool unicatTranscendenceAOEPurchased = false;

        bool devilcatHellsingedPurchased = false;
    };

    Permanent perm = {};

    //
    // Permanent purchases settings
    bool              multiPopEnabled            = false;
    bool              multiPopMouseCatEnabled    = false;
    int               windStrength               = 0;
    BubbleIgnoreFlags geniusCatIgnoreBubbles     = {};
    BubbleIgnoreFlags repulsoCatIgnoreBubbles    = {};
    BubbleIgnoreFlags attractoCatIgnoreBubbles   = {};
    bool              repulsoCatConverterEnabled = false;

    //
    // Object state
    std::vector<Bubble>     bubbles;
    std::vector<Cat>        cats;
    std::vector<Shrine>     shrines;
    std::vector<Doll>       dolls;
    std::vector<Doll>       copyDolls;
    std::vector<HellPortal> hellPortals;

    //
    // Shrines
    SizeT nShrinesCompleted = 0u;

    //
    // Statistics
    Stats      statsTotal;
    Stats      statsSession;
    Milestones milestones;

    //
    // Achievement tracking
    bool achAstrocatPopBomb                  = false;
    bool achAstrocatInspireByType[nCatTypes] = {};

    //
    // Witchcat buffs
    Countdown buffCountdownsPerType[nCatTypes] = {};

    //
    // Other flags
    bool prestigeTipShown       = false;
    bool shrineHoverTipShown    = false;
    bool shrineActivateTipShown = false;
    bool dollTipShown           = false;
    bool spendPPTipShown        = false;
    bool shrinesSpawned         = false;

    ////////////////////////////////////////////////////////////
    void spawnAllShrinesIfNeeded()
    {
        if (shrinesSpawned)
            return;

        shrinesSpawned = true;
        shrines.reserve(nShrineTypes);

        for (SizeT i = 0u; i < nShrineTypes; ++i)
            shrines.push_back({
                .position              = (gameScreenSize / 2.f).addX(gameScreenSize.x * static_cast<float>(i + 1u)),
                .tcActivation          = {},
                .tcDeath               = {},
                .textStatusShakeEffect = {},
                .type                  = static_cast<ShrineType>(i),
            });
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] PrestigePointsType calculatePrestigePointReward(const PrestigePointsType prestigeCount) const
    {
        const auto currentPrestigeLevel = static_cast<PrestigePointsType>(psvBubbleValue.currentValue());
        const auto targetPrestigeLevel  = currentPrestigeLevel + prestigeCount;

        PrestigePointsType pointsToAdd = 0u;

        for (auto i = currentPrestigeLevel; i < targetPrestigeLevel; ++i)
            pointsToAdd += static_cast<PrestigePointsType>(sf::base::pow(2.f, static_cast<float>(i)));

        return pointsToAdd;
    }

    ////////////////////////////////////////////////////////////
    void onPrestige(const PrestigePointsType prestigePointReward)
    {
        psvComboStartTime.nPurchases = 0u;
        // map extension is cleaned in the game loop (prestige transition)
        psvShrineActivation.nPurchases    = 0u;
        psvBubbleCount.nPurchases         = 0u;
        psvSpellCount.nPurchases          = 0u;
        psvExplosionRadiusMult.nPurchases = 0u;
        psvStarpawPercentage.nPurchases   = 0u;
        psvMewltiplierMult.nPurchases     = 0u;
        psvDarkUnionPercentage.nPurchases = 0u;

        for (auto& psv : psvPerCatType)
            psv.nPurchases = 0u;

        for (auto& psv : psvCooldownMultsPerCatType)
            psv.nPurchases = 0u;

        for (auto& psv : psvRangeDivsPerCatType)
            psv.nPurchases = 0u;

        money = perm.starterPackPurchased ? 1000u : 0u;
        prestigePoints += prestigePointReward;

        comboPurchased = false;
        // map purchased is cleaned in the game loop (prestige transition)

        manaTimer            = 0.f;
        mana                 = 0u;
        absorbingWisdom      = false;
        wisdom               = 0u;
        mewltiplierAuraTimer = 0.f;

        mouseCatCombo                = 0.f;
        mouseCatComboCountdown.value = 0.f;

        multiPopEnabled         = false;
        multiPopMouseCatEnabled = false;
        windStrength            = 0;

        // bubbles, cats, dolls, copydolls, hell portals, and shrines are cleaned in the game loop (prestige transition)

        nShrinesCompleted = 0u;

        statsSession = {};

        for (Countdown& buffCountdown : buffCountdownsPerType)
            buffCountdown = {};

        shrinesSpawned = false;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline SizeT getMapLimitIncreases() const
    {
        return static_cast<SizeT>(mapPurchased) + psvMapExtension.nPurchases;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline float getMapLimit() const
    {
        return gameScreenSize.x * static_cast<float>(getMapLimitIncreases() + 1u);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] constexpr MoneyType getComputedRewardByBubbleType(const BubbleType type) const
    {
        constexpr EXACT_ARRAY(MoneyType,
                              baseRewards,
                              nBubbleTypes,
                              {
                                  1u,  // Normal
                                  15u, // Star
                                  1u,  // Bomb
                                  50u, // Nova
                              });

        return baseRewards[asIdx(type)] * static_cast<MoneyType>(psvBubbleValue.currentValue() + 1.f);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline MoneyType getComputedRequiredRewardByShrineType(const ShrineType type) const
    {
        constexpr EXACT_ARRAY(MoneyType,
                              baseRequiredRewards,
                              nShrineTypes,
                              {
                                  1000,           // Voodoo
                                  75'000,         // Magic
                                  150'000,        // Clicking
                                  2'500'000,      // Automation
                                  25'000'000,     // Repulsion
                                  500'000'000,    // Attraction
                                  2'500'000'000,  // Camouflage
                                  10'000'000'000, // Victory
                              });

        return static_cast<MoneyType>(static_cast<float>(baseRequiredRewards[asIdx(type)]));
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline float getComputedCooldownByCatType(const CatType catType) const
    {
        return CatConstants::baseCooldowns[asIdx(catType)] * psvCooldownMultsPerCatType[asIdx(catType)].currentValue();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline float getComputedRangeByCatType(const CatType catType) const
    {
        const auto result = CatConstants::baseRanges[asIdx(catType)] /
                            psvRangeDivsPerCatType[asIdx(catType)].currentValue();

        if (catType == CatType::Wizard && absorbingWisdom)
            return result / 2.f;

        return result;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline float getComputedSquaredRangeByCatType(const CatType catType) const
    {
        const float range = getComputedRangeByCatType(catType);
        return range * range;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline float getComputedBombExplosionRadius() const
    {
        return 200.f * psvExplosionRadiusMult.currentValue();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline float getComputedInspirationDuration() const
    {
        return 1500.f * psvPPInspireDurationMult.currentValue();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline float getComputedManaCooldown() const
    {
        return 10'000.f * psvPPManaCooldownMult.currentValue();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline ManaType getComputedMaxMana() const
    {
        return static_cast<ManaType>(20.f * psvPPManaMaxMult.currentValue());
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getComputedMultiPopRange() const
    {
        return psvPPMultiPopRange.currentValue();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] SizeT getCatCountByType(const CatType type) const
    {
        return psvPerCatType[asIdx(type)].nPurchases;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isBubbleValueUnlocked() const // also unlocks prestige
    {
        const auto nCatUni = getCatCountByType(CatType::Uni);
        return psvBubbleValue.nPurchases > 0 || (psvBubbleCount.nPurchases > 0 && nCatUni >= 3 && nShrinesCompleted >= 1);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::const]] static constexpr SizeT getShrinesCompletedNeededForPrestigeLevel(const SizeT prestigeLevel)
    {
        if (prestigeLevel <= 0u)
            return 1u;

        if (prestigeLevel <= 1u)
            return 1u;

        if (prestigeLevel <= 2u)
            return 2u;

        if (prestigeLevel <= 4u)
            return 3u;

        if (prestigeLevel <= 8u)
            return 4u;

        if (prestigeLevel <= 11u)
            return 5u;

        if (prestigeLevel <= 14u)
            return 6u;

        if (prestigeLevel <= 17u)
            return 7u;

        return 8u;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] SizeT getShrinesCompletedNeededForNextPrestige() const
    {
        return getShrinesCompletedNeededForPrestigeLevel(psvBubbleValue.nPurchases + 1u);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool canBuyNextPrestige() const
    {
        const SizeT shrinesNeeded = getShrinesCompletedNeededForNextPrestige();
        return psvBubbleValue.nextCost() <= static_cast<float>(money) && nShrinesCompleted >= shrinesNeeded;
    }
};
