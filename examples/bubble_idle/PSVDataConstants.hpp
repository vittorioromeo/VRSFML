#pragma once

#include "PSVData.hpp"
#include "Version.hpp"


////////////////////////////////////////////////////////////
namespace PSVDataConstants
{

inline constexpr PSVData comboStartTime //
    {.nMaxPurchases = 20u,
     .cost          = {.initial = 250.f, .linear = 125.f, .exponential = 1.7f, .flat = -200.f},
     .value         = {.initial = 0.55f, .linear = 0.04f, .exponential = 1.02f}};

inline constexpr PSVData mapExtension //
    {.nMaxPurchases = isDemoVersion ? 1u : 7u,
     .cost          = {.initial = 15000.f, .exponential = 8.f, .flat = -5000.f},
     .value         = {}};

inline constexpr PSVData shrineActivation //
    {.nMaxPurchases = isDemoVersion ? 2u : 8u,
     .cost          = {.initial = 15000.f, .linear = -56'500.f, .exponential = 7.5f, .flat = -13'500.f},
     .value         = {}};

inline constexpr PSVData bubbleCount //
    {.nMaxPurchases = 30u,
     .cost          = {.initial = 250.f, .linear = 1500.f, .exponential = 2.65f, .flat = -100.f},
     .value         = {.initial = 500.f, .linear = 325.f, .exponential = 1.01f}};

inline constexpr PSVData bubbleValue //
    {.nMaxPurchases = isDemoVersion ? 1u : 19u,
     .cost  = {.initial = 19000.f, .linear = -620.f, .multiplicative = -800.f, .exponential = 4.1f, .flat = -14000.f},
     .value = {.initial = 0.f, .linear = 1.f}};

inline constexpr PSVData explosionRadiusMult //
    {.nMaxPurchases = 10u,
     .cost          = {.initial = 15000.f, .linear = -15000.f, .exponential = 3.f, .flat = -5000.f},
     .value         = {.initial = 1.f, .linear = 0.1f}};

inline constexpr PSVData catNormal //
    {.nMaxPurchases = 64u, .cost = {.initial = 75.f, .linear = 100.f, .exponential = 1.7f, .flat = -40.f}, .value = {}};

inline constexpr PSVData catNormalCooldownMult //
    {.nMaxPurchases = 12u,
     .cost          = {.initial = 2'500.f, .exponential = 1.85f, .flat = -1500.f},
     .value         = {.initial = 1.f, .linear = 0.015f, .multiplicative = 0.05f, .exponential = 0.8f}};

inline constexpr PSVData catNormalRangeDiv //
    {.nMaxPurchases = 9u,
     .cost          = {.initial = 7000.f, .exponential = 1.85f, .flat = -3000.f},
     .value         = {.initial = 0.6f, .multiplicative = -0.045f, .exponential = 0.85f, .flat = 0.4f}};

inline constexpr PSVData catUni //
    {.nMaxPurchases = 64u, .cost = {.initial = 350.f, .linear = 300.f, .exponential = 1.75f}, .value = {}};

inline constexpr PSVData catUniCooldownMult //
    {.nMaxPurchases = 12u,
     .cost          = {.initial = 8'500.f, .exponential = 1.85f, .flat = -4'500.f},
     .value         = {.initial = 1.f, .linear = 0.015f, .multiplicative = 0.05f, .exponential = 0.8f}};

inline constexpr PSVData catUniRangeDiv //
    {.nMaxPurchases = 9u,
     .cost          = {.initial = 10'000'000.f, .exponential = 1.85f},
     .value         = {.initial = 0.6f, .multiplicative = -0.05f, .exponential = 0.8f, .flat = 0.4f}};

inline constexpr PSVData catDevil //
    {.nMaxPurchases = 64u, .cost = {.initial = 15000.f, .exponential = 1.7f, .flat = -7000.f}, .value = {}};

inline constexpr PSVData catDevilCooldownMult //
    {.nMaxPurchases = 12u,
     .cost          = {.initial = 1'000'000.f, .exponential = 1.75f},
     .value         = {.initial = 1.f, .linear = 0.01f, .multiplicative = 0.045f, .exponential = 0.85f}};

inline constexpr PSVData catDevilRangeDiv //
    {.nMaxPurchases = 9u,
     .cost          = {.initial = 50'000'000.f, .exponential = 1.75f},
     .value         = {.initial = 0.6f, .multiplicative = -0.045f, .exponential = 0.85f, .flat = 0.4f}};

inline constexpr PSVData catWitch //
    {.nMaxPurchases = 0u, .cost = {}, .value = {}};

inline constexpr PSVData catWitchCooldownMult //
    {.nMaxPurchases = 12u,
     .cost          = {.initial = 500'000.f, .exponential = 2.5f},
     .value         = {.initial = 1.f, .linear = 0.0085f, .multiplicative = 0.02f, .exponential = 0.875f}};

inline constexpr PSVData catWitchRangeDiv //
    {.nMaxPurchases = 9u,
     .cost          = {.initial = 200'000.f, .exponential = 2.5f},
     .value         = {.initial = 0.6f, .multiplicative = -0.05f, .exponential = 0.8f, .flat = 0.4f}};

inline constexpr PSVData catWizard //
    {.nMaxPurchases = 0u, .cost = {}, .value = {}};

inline constexpr PSVData catWizardCooldownMult //
    {.nMaxPurchases = 12u,
     .cost          = {.initial = 750'000.f, .exponential = 1.75f},
     .value         = {.initial = 1.f, .linear = 0.015f, .multiplicative = 0.05f, .exponential = 0.8f}};

inline constexpr PSVData catWizardRangeDiv //
    {.nMaxPurchases = 9u,
     .cost          = {.initial = 1'000'000.f, .exponential = 1.85f},
     .value         = {.initial = 0.6f, .multiplicative = -0.05f, .exponential = 0.8f, .flat = 0.4f}};

inline constexpr PSVData catMouse //
    {.nMaxPurchases = 0u, .cost = {}, .value = {}};

inline constexpr PSVData catMouseCooldownMult //
    {.nMaxPurchases = 12u,
     .cost          = {.initial = 125000.f, .exponential = 1.75f},
     .value         = {.initial = 1.f, .linear = 0.015f, .multiplicative = 0.05f, .exponential = 0.8f}};

inline constexpr PSVData catMouseRangeDiv //
    {.nMaxPurchases = 9u,
     .cost          = {.initial = 100'000.f, .exponential = 1.85f},
     .value         = {.initial = 0.6f, .multiplicative = -0.045f, .exponential = 0.85f, .flat = 0.4f}};

inline constexpr PSVData catEngi //
    {.nMaxPurchases = 0u, .cost = {}, .value = {}};

inline constexpr PSVData catEngiCooldownMult //
    {.nMaxPurchases = 12u,
     .cost          = {.initial = 1'250'000.f, .exponential = 1.75f},
     .value         = {.initial = 1.f, .linear = 0.015f, .multiplicative = 0.05f, .exponential = 0.8f}};

inline constexpr PSVData catEngiRangeDiv //
    {.nMaxPurchases = 9u,
     .cost          = {.initial = 1'000'000.f, .exponential = 1.85f},
     .value         = {.initial = 0.6f, .multiplicative = -0.045f, .exponential = 0.85f, .flat = 0.4f}};

inline constexpr PSVData catRepulso //
    {.nMaxPurchases = 0u, .cost = {}, .value = {}};

inline constexpr PSVData catRepulsoCooldownMult //
    {.nMaxPurchases = 12u,
     .cost          = {.initial = 250'000.f, .exponential = 1.75f},
     .value         = {.initial = 1.f, .linear = 0.015f, .multiplicative = 0.05f, .exponential = 0.8f}};

inline constexpr PSVData catRepulsoRangeDiv //
    {.nMaxPurchases = 9u,
     .cost          = {.initial = 10'000'000.f, .exponential = 1.85f},
     .value         = {.initial = 0.7f, .multiplicative = -0.025f, .exponential = 0.95f, .flat = 0.3f}};

inline constexpr PSVData catAttracto //
    {.nMaxPurchases = 0u, .cost = {}, .value = {}};

inline constexpr PSVData catAttractoCooldownMult //
    {.nMaxPurchases = 12u,
     .cost          = {.initial = 300'000.f, .exponential = 1.75f},
     .value         = {.initial = 1.f, .linear = 0.015f, .multiplicative = 0.05f, .exponential = 0.8f}};

inline constexpr PSVData catAttractoRangeDiv //
    {.nMaxPurchases = 9u,
     .cost          = {.initial = 50'000'000.f, .exponential = 1.85f},
     .value         = {.initial = 0.7f, .multiplicative = -0.025f, .exponential = 0.95f, .flat = 0.3f}};

inline constexpr PSVData catCopy //
    {.nMaxPurchases = 0u, .cost = {}, .value = {}};

inline constexpr PSVData catCopyCooldownMult //
    {.nMaxPurchases = 0u, .cost = {}, .value = {}};

inline constexpr PSVData catCopyRangeDiv //
    {.nMaxPurchases = 0u, .cost = {}, .value = {}};

inline constexpr PSVData catDuck //
    {.nMaxPurchases = 0u, .cost = {}, .value = {}};

inline constexpr PSVData catDuckCooldownMult //
    {.nMaxPurchases = 0u, .cost = {}, .value = {}};

inline constexpr PSVData catDuckRangeDiv //
    {.nMaxPurchases = 0u, .cost = {}, .value = {}};

inline constexpr PSVData catAstro //
    {.nMaxPurchases = 64u, .cost = {.initial = 1'000'000.f, .exponential = 1.85f}, .value = {}};

inline constexpr PSVData catAstroCooldownMult //
    {.nMaxPurchases = 12u,
     .cost          = {.initial = 3'500'000.f, .exponential = 1.85f},
     .value         = {.initial = 1.f, .linear = 0.01f, .multiplicative = 0.04f, .exponential = 0.85f}};

inline constexpr PSVData catAstroRangeDiv //
    {.nMaxPurchases = 9u,
     .cost          = {.initial = 5'500'000.f, .exponential = 1.85f},
     .value         = {.initial = 0.6f, .multiplicative = -0.045f, .exponential = 0.85f, .flat = 0.4f}};

inline constexpr PSVData multiPopRange //
    {.nMaxPurchases = 24u,
     .cost          = {.initial = 1.f, .linear = 2.f, .exponential = 1.75f},
     .value         = {.initial = 64.f, .linear = 8.f}};

inline constexpr PSVData inspireDurationMult //
    {.nMaxPurchases = 16u,
     .cost          = {.initial = 2.f, .linear = 2.f, .exponential = 1.8f},
     .value         = {.initial = 1.f, .linear = 0.2f}};

inline constexpr PSVData manaCooldownMult //
    {.nMaxPurchases = 16u,
     .cost          = {.initial = 1.f, .linear = 2.f, .exponential = 1.75f},
     .value         = {.initial = 1.f, .exponential = 0.875f}};

inline constexpr PSVData manaMaxMult //
    {.nMaxPurchases = 20u,
     .cost          = {.initial = 1.f, .linear = 2.f, .exponential = 1.75f},
     .value         = {.initial = 1.f, .linear = 0.2f}};

inline constexpr PSVData spellCount //
    {.nMaxPurchases = 4u,
     .cost          = {.initial = 3'500.f, .linear = -24000.f, .exponential = 8.f, .flat = -3250.f},
     .value         = {.initial = 0.f, .linear = 1.f}};

inline constexpr PSVData mouseCatGlobalBonusMult //
    {.nMaxPurchases = 14u,
     .cost          = {.initial = 2.f, .linear = 4.f, .exponential = 1.5f},
     .value         = {.initial = 1.25f, .linear = 0.125f}};

inline constexpr PSVData engiCatGlobalBonusMult //
    {.nMaxPurchases = 14u,
     .cost          = {.initial = 4.f, .linear = 8.f, .exponential = 1.5f},
     .value         = {.initial = 1.25f, .linear = 0.125f}};

inline constexpr PSVData repulsoCatConverterChance //
    {.nMaxPurchases = 16u,
     .cost          = {.initial = 128.f, .linear = 32.f, .exponential = 1.35f},
     .value         = {.initial = 6.f, .linear = 5.875f}};

inline constexpr PSVData witchCatBuffDuration //
    {.nMaxPurchases = 12u,
     .cost          = {.initial = 1.f, .linear = 8.f, .exponential = 1.5f},
     .value         = {.initial = 8.f, .linear = 1.f}};

inline constexpr PSVData starpawPercentage //
    {.nMaxPurchases = 8u, .cost = {.initial = 1000.f, .exponential = 1.25f}, .value = {.initial = 40.f, .linear = 7.5f}};

inline constexpr PSVData mewltiplierMult //
    {.nMaxPurchases = 15u, .cost = {.initial = 2000.f, .exponential = 1.75f}, .value = {.initial = 2.5f, .linear = 0.5f}};

inline constexpr PSVData darkUnionPercentage //
    {.nMaxPurchases = 8u, .cost = {.initial = 50'000.f, .exponential = 1.25f}, .value = {.initial = 50.f, .linear = 5.f}};

inline constexpr PSVData uniRitualBuffPercentage //
    {.nMaxPurchases = 24u, .cost = {.initial = 4.f, .exponential = 1.25f}, .value = {.initial = 15.f, .linear = 2.5f}};

inline constexpr PSVData devilRitualBuffPercentage //
    {.nMaxPurchases = 24u, .cost = {.initial = 8.f, .exponential = 1.25f}, .value = {.initial = 1.f, .linear = 1.f}};

} // namespace PSVDataConstants
