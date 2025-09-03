#include "Serialization.hpp"

#include "Aliases.hpp"
#include "Bubble.hpp"
#include "Cat.hpp"
#include "Countdown.hpp"
#include "HellPortal.hpp"
#include "Milestones.hpp"
#include "Playthrough.hpp"
#include "Profile.hpp"
#include "PurchasableScalingValue.hpp"
#include "Shrine.hpp"
#include "Timer.hpp"
#include "Version.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"

#include "json.hpp"

#pragma GCC diagnostic pop

#include "SFML/System/IO.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Traits/IsArray.hpp"
#include "SFML/Base/Traits/IsSame.hpp"
#include "SFML/Base/Traits/RemoveCVRef.hpp"

#include <filesystem>
#include <string>

// NOLINTBEGIN(readability-identifier-naming, misc-use-internal-linkage)

namespace sf
{
////////////////////////////////////////////////////////////
template <typename T>
void to_json(nlohmann::json& j, const Vec2<T>& p)
{
    j[0] = p.x;
    j[1] = p.y;
}


////////////////////////////////////////////////////////////
template <typename T>
void from_json(const nlohmann::json& j, Vec2<T>& p)
{
    p.x = j[0].get<T>();
    p.y = j[1].get<T>();
}

////////////////////////////////////////////////////////////
void to_json(nlohmann::json& j, const Time& p)
{
    j = p.asMicroseconds();
}


////////////////////////////////////////////////////////////
void from_json(const nlohmann::json& j, Time& p)
{
    p = Time{j.get<base::I64>()};
}

} // namespace sf


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
void to_json(nlohmann::json& j, const Optional<T>& p)
{
    if (p.hasValue())
        j = p.value();
    else
        j = nullptr;
}


////////////////////////////////////////////////////////////
template <typename T>
void from_json(const nlohmann::json& j, Optional<T>& p)
{
    if (j.is_null())
        p.reset();
    else
        p.emplace(j.get<T>());
}


// `to_json` is not needed for `Vector` because `nlohmann::json`
// has a catch-all overload for container-like types.


////////////////////////////////////////////////////////////
template <typename T>
void from_json(const nlohmann::json& j, Vector<T>& p)
{
    p.clear();
    p.reserve(j.size());

    for (const auto& item : j)
        p.emplaceBack(item.get<T>());
}

} // namespace sf::base


namespace
{
////////////////////////////////////////////////////////////
template <typename>
struct getArraySize;

////////////////////////////////////////////////////////////
template <typename T, auto N>
struct getArraySize<T[N]>
{
    enum
    {
        value = N
    };
};

////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline]] inline void atOr(const nlohmann::json& j, T& target, const sf::base::SizeT index)
{
    if (j.is_array() && j.size() > index)
        j[index].get_to<T>(target);
}

////////////////////////////////////////////////////////////
template <typename T>
void serialize(nlohmann::json& j, const sf::base::SizeT index, const T& field)
{
    if constexpr (!SFML_BASE_IS_ARRAY(T))
    {
        j[index] = field;
    }
    else
    {
        constexpr auto arraySize = getArraySize<T>::value;
        j[index]                 = nlohmann::json(arraySize, {});

        for (sf::base::SizeT i = 0u; i < arraySize; ++i)
            serialize(j[index], i, field[i]);
    }
}

////////////////////////////////////////////////////////////
template <typename T>
void deserialize(const nlohmann::json& j, const sf::base::SizeT index, T& field)
{
    if constexpr (!SFML_BASE_IS_ARRAY(T))
    {
        atOr<T>(j, field, index);
    }
    else
    {
        constexpr auto arraySize = getArraySize<T>::value;

        // Initialize with empty array
        auto arr = nlohmann::json::array();

        // Check if j[index] exists and is an array
        if (j.is_array() && j.size() > index && j[index].is_array())
            arr = j[index];

        for (sf::base::SizeT i = 0u; i < arraySize; ++i)
            deserialize(arr, i, field[i]);
    }
}


////////////////////////////////////////////////////////////
template <bool Serialize>
[[gnu::always_inline]] inline sf::base::SizeT twoWay(sf::base::SizeT index, auto&& j, auto&&... fields)
{
    if constexpr (Serialize)
    {
        (..., serialize(j, index++, fields));
    }
    else
    {
        (..., deserialize(j, index++, fields));
    }

    return index;
}

} // namespace


////////////////////////////////////////////////////////////
template <typename T, typename U>
concept isSameDecayed = sf::base::isSame<SFML_BASE_REMOVE_CVREF(T), SFML_BASE_REMOVE_CVREF(U)>;


////////////////////////////////////////////////////////////
template <typename T>
void to_json(nlohmann::json& j, const T& p)
    requires(requires { twoWaySerializer<true>(j, p); })
{
    twoWaySerializer<true>(j, p);
}


////////////////////////////////////////////////////////////
template <typename T>
void from_json(const nlohmann::json& j, T& p)
    requires(requires { twoWaySerializer<false>(j, p); })
{
    twoWaySerializer<false>(j, p);
}


////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<Bubble> auto&& p)
{
    twoWay<Serialize>(0u,
                      j,

                      p.position,
                      p.velocity,

                      p.radius,
                      p.rotation,
                      p.hueMod,

                      p.repelledCountdown,
                      p.attractedCountdown,

                      p.type);
}


////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<Cat::AstroState> auto&& p)
{
    twoWay<Serialize>(0u,
                      j,

                      p.startX,

                      p.velocityX,
                      p.particleTimer,

                      p.wrapped);
}


////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<Cat> auto&& p)
{
    twoWay<Serialize>(0u,
                      j,

                      p.spawnEffectTimer,

                      p.position,

                      p.wobbleRadians,
                      p.cooldown,

                      // p.pawPosition,
                      // p.pawRotation,

                      // p.pawOpacity,

                      p.hue,

                      p.inspiredCountdown,
                      p.boostCountdown,

                      p.nameIdx,

                      // p.textStatusShakeEffect,
                      // p.textMoneyShakeEffect,

                      p.hits,

                      p.type,

                      p.hexedTimer,
                      p.hexedCopyTimer,

                      p.moneyEarned,

                      p.astroState,

                      p.blinkCountdown,
                      // p.blinkAnimCountdown,

                      p.flapCountdown,
                      // p.flapAnimCountdown,

                      p.yawnCountdown
                      // p.yawnAnimCountdown
    );
}


////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<Doll> auto&& p)
{
    twoWay<Serialize>(0u,
                      j,

                      p.position,
                      p.buffPower,
                      p.wobbleRadians,
                      p.hue,
                      p.catType,

                      p.tcActivation,
                      p.tcDeath);
}


////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<HellPortal> auto&& p)
{
    twoWay<Serialize>(0u,
                      j,

                      p.position,
                      p.life,
                      p.catIdx);
}


////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<Shrine> auto&& p)
{
    twoWay<Serialize>(0u,
                      j,

                      p.position,

                      p.wobbleRadians,

                      p.tcActivation,
                      p.tcDeath,

                      // p.textStatusShakeEffect,

                      p.collectedReward,

                      p.type);
}


////////////////////////////////////////////////////////////
void to_json(nlohmann::json& j, const Timer& p)
{
    j = p.value;
}


////////////////////////////////////////////////////////////
void from_json(const nlohmann::json& j, Timer& p)
{
    p.value = j;
}


////////////////////////////////////////////////////////////
void to_json(nlohmann::json& j, const BidirectionalTimer& p)
{
    j[0] = p.value;
    j[1] = static_cast<bool>(p.direction);
}


////////////////////////////////////////////////////////////
void from_json(const nlohmann::json& j, BidirectionalTimer& p)
{
    p.value     = j[0];
    p.direction = static_cast<TimerDirection>(j[1].get<bool>());
}


////////////////////////////////////////////////////////////
void to_json(nlohmann::json& j, const Countdown& p)
{
    j = p.value;
}


////////////////////////////////////////////////////////////
void from_json(const nlohmann::json& j, Countdown& p)
{
    p.value = j;
}


////////////////////////////////////////////////////////////
void to_json(nlohmann::json& j, const TargetedCountdown& p)
{
    j[0] = p.value;
    j[1] = p.startingValue;
}


////////////////////////////////////////////////////////////
void from_json(const nlohmann::json& j, TargetedCountdown& p)
{
    p.value         = j[0];
    p.startingValue = j[1];
}


////////////////////////////////////////////////////////////
void to_json(nlohmann::json& j, const OptionalTargetedCountdown& p)
{
    to_json(j, p.asBase());
}


////////////////////////////////////////////////////////////
void from_json(const nlohmann::json& j, OptionalTargetedCountdown& p)
{
    from_json(j, p.asBase());
}


////////////////////////////////////////////////////////////
void to_json(nlohmann::json& j, const PurchasableScalingValue& p)
{
    j = p.nPurchases;
}


////////////////////////////////////////////////////////////
void from_json(const nlohmann::json& j, PurchasableScalingValue& p)
{
    p.nPurchases = j;
}


////////////////////////////////////////////////////////////
template <SizeT N>
void to_json(nlohmann::json& j, const PurchasableScalingValue (&p)[N])
{
    j = nlohmann::json::array();

    for (SizeT i = 0u; i < N; ++i)
        j[i] = p[i].nPurchases;
}


////////////////////////////////////////////////////////////
template <SizeT N>
void from_json(const nlohmann::json& j, PurchasableScalingValue (&p)[N])
{
    for (SizeT i = 0u; i < N; ++i)
        p[i].nPurchases = j.at(i);
}


////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<Stats> auto&& p)
{
    twoWay<Serialize>(0u,
                      j,

                      p.secondsPlayed,

                      p.nBubblesPoppedByType,
                      p.revenueByType,

                      p.nBubblesHandPoppedByType,
                      p.revenueHandByType,

                      p.explosionRevenue,
                      p.flightRevenue,
                      p.hellPortalRevenue,

                      p.highestStarBubblePopCombo,
                      p.highestNovaBubblePopCombo,

                      p.nAbsorbedStarBubbles,

                      p.nSpellCasts,

                      p.nWitchcatRitualsPerCatType,
                      p.nWitchcatDollsCollected,

                      p.nMaintenances,
                      p.highestSimultaneousMaintenances,

                      p.nDisguises,

                      p.highestDPS);
}


////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<Milestones> auto&& p)
{
    twoWay<Serialize>(0u,
                      j,

                      p.firstCat,
                      p.firstUnicat,
                      p.firstDevilcat,
                      p.firstAstrocat,

                      p.fiveCats,
                      p.fiveUnicats,
                      p.fiveDevilcats,
                      p.fiveAstrocats,

                      p.tenCats,
                      p.tenUnicats,
                      p.tenDevilcats,
                      p.tenAstrocats,

                      p.prestigeLevel2,
                      p.prestigeLevel3,
                      p.prestigeLevel4,
                      p.prestigeLevel5,
                      p.prestigeLevel6,
                      p.prestigeLevel10,
                      p.prestigeLevel15,
                      p.prestigeLevel20,

                      p.revenue10000,
                      p.revenue100000,
                      p.revenue1000000,
                      p.revenue10000000,
                      p.revenue100000000,
                      p.revenue1000000000,

                      p.shrineCompletions);
}


////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<SpeedrunningSplits> auto&& p)
{
    twoWay<Serialize>(0u,
                      j,

                      p.prestigeLevel2,
                      p.prestigeLevel3,
                      p.prestigeLevel4,
                      p.prestigeLevel5);
}


////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<Profile> auto&& p)
{
    auto version = currentVersion;
    twoWay<Serialize>(0u, j, version);

    twoWay<Serialize>(1u,
                      j,

                      p.masterVolume,
                      p.musicVolume,

                      p.playAudioInBackground,
                      p.playComboEndSound,

                      p.selectedBGM,

                      p.minimapScale,
                      p.hudScale,
                      p.uiScale,

                      p.tipsEnabled,

                      p.backgroundOpacity,
                      p.selectedBackground,
                      p.alwaysShowDrawings,

                      p.showCatText,
                      p.showCatRange,
                      p.showParticles,
                      p.showTextParticles,
                      p.accumulatingCombo,
                      p.showCursorComboText,
                      p.useBubbleShader,

                      p.cursorTrailMode,
                      p.cursorTrailScale,

                      p.bsIridescenceStrength,
                      p.bsEdgeFactorMin,
                      p.bsEdgeFactorMax,
                      p.bsEdgeFactorStrength,
                      p.bsDistortionStrength,
                      p.bsBubbleLightness,
                      p.bsLensDistortion,

                      p.statsLifetime,

                      p.resWidth,

                      p.windowed,
                      p.vsync,

                      p.frametimeLimit,

                      p.highVisibilityCursor,
                      p.multicolorCursor,

                      p.cursorHue,
                      p.cursorScale,

                      p.showCoinParticles,
                      p.showDpsMeter,

                      p.enableNotifications,
                      p.showFullManaNotification,

                      p.unlockedAchievements,

                      p.uiUnlocks,

                      p.ppSVibrance,
                      p.ppSSaturation,
                      p.ppSLightness,
                      p.ppSSharpness,

                      p.showBubbles,
                      p.invertMouseButtons,
                      p.showDollParticleBorder,

                      p.catDragPressDuration,
                      p.playWitchRitualSounds,
                      p.enableScreenShake,
                      p.enableCatBobbing,
                      p.catRangeOutlineThickness,

                      p.showCursorComboBar,
                      p.sfxVolume,

                      p.hideMaxedOutPurchasables,
                      p.hideCategorySeparators);
}

////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<BubbleIgnoreFlags> auto&& p)
{
    twoWay<Serialize>(0u,
                      j,

                      p.normal,
                      p.star,
                      p.bomb);
}

////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<Playthrough::Permanent> auto&& p)
{
    twoWay<Serialize>(0u,
                      j,

                      p.starterPackPurchased,

                      p.multiPopPurchased,
                      p.smartCatsPurchased,
                      p.geniusCatsPurchased,

                      p.windPurchased,

                      p.astroCatInspirePurchased,

                      p.starpawConversionIgnoreBombs,
                      p.starpawNova,

                      p.repulsoCatFilterPurchased,
                      p.repulsoCatConverterPurchased,
                      p.repulsoCatNovaConverterPurchased,

                      p.attractoCatFilterPurchased,

                      p.witchCatBuffPowerScalesWithNCats,
                      p.witchCatBuffPowerScalesWithMapSize,
                      p.witchCatBuffFewerDolls,
                      p.witchCatBuffFlammableDolls,
                      p.witchCatBuffOrbitalDolls,

                      p.shrineCompletedOnceByCatType,

                      p.unsealedByType,

                      p.wizardCatDoubleMewltiplierDuration,
                      p.wizardCatDoubleStasisFieldDuration,

                      p.unicatTranscendencePurchased,
                      p.unicatTranscendenceAOEPurchased,

                      p.devilcatHellsingedPurchased,

                      p.unicatTranscendenceEnabled,
                      p.devilcatHellsingedEnabled,

                      p.autocastPurchased,
                      p.autocastIndex);
}

////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<Playthrough> auto&& p)
{
    auto version = currentVersion;
    twoWay<Serialize>(0u, j, version);

    twoWay<Serialize>(1u,
                      j,

                      p.seed,
                      p.nextCatNamePerType,

                      p.psvComboStartTime,
                      p.psvMapExtension,
                      p.psvShrineActivation,
                      p.psvBubbleCount,
                      p.psvSpellCount,
                      p.psvBubbleValue,
                      p.psvExplosionRadiusMult,
                      p.psvStarpawPercentage,
                      p.psvMewltiplierMult,
                      p.psvDarkUnionPercentage,

                      p.psvPerCatType,

                      p.psvCooldownMultsPerCatType,

                      p.psvRangeDivsPerCatType,

                      p.psvPPMultiPopRange,
                      p.psvPPInspireDurationMult,
                      p.psvPPManaCooldownMult,
                      p.psvPPManaMaxMult,
                      p.psvPPMouseCatGlobalBonusMult,
                      p.psvPPEngiCatGlobalBonusMult,
                      p.psvPPRepulsoCatConverterChance,
                      p.psvPPWitchCatBuffDuration,
                      p.psvPPUniRitualBuffPercentage,
                      p.psvPPDevilRitualBuffPercentage,

                      p.money,

                      p.prestigePoints,

                      p.comboPurchased,
                      p.mapPurchased,

                      p.manaTimer,
                      p.mana,
                      p.absorbingWisdom,
                      p.wisdom,
                      p.mewltiplierAuraTimer,
                      p.stasisFieldTimer,

                      p.mouseCatCombo,
                      p.mouseCatComboCountdown,

                      p.copycatCopiedCatType,

                      p.perm,

                      p.multiPopEnabled,
                      p.multiPopMouseCatEnabled,
                      p.windStrength,
                      p.geniusCatIgnoreBubbles,
                      p.repulsoCatIgnoreBubbles,
                      p.attractoCatIgnoreBubbles,
                      p.repulsoCatConverterEnabled,

                      p.bubbles,
                      p.cats,
                      p.shrines,
                      p.dolls,
                      p.copyDolls,
                      p.hellPortals,

                      p.nShrinesCompleted,

                      p.statsTotal,
                      p.statsSession,
                      p.milestones,

                      p.achAstrocatPopBomb,
                      p.achAstrocatInspireByType,

                      p.buffCountdownsPerType,

                      p.prestigeTipShown,
                      p.shrineHoverTipShown,
                      p.shrineActivateTipShown,
                      p.dollTipShown,
                      p.spendPPTipShown,
                      p.shrinesSpawned,

                      p.laserPopEnabled,

                      p.disableAstrocatFlight,

                      p.speedrunStartTime,
                      p.speedrunSplits,

                      p.fullVersion);
}

////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<Version> auto&& p)
{
    twoWay<Serialize>(0u,
                      j,

                      p.major,
                      p.minor,
                      p.patch);
}

namespace
{
////////////////////////////////////////////////////////////
void forceCopyFile(const std::filesystem::path& from, const std::filesystem::path& to)
try
{
    std::filesystem::remove(to);
    std::filesystem::copy_file(from, to);
} catch (...)
{
}


////////////////////////////////////////////////////////////
void doRotatingBackup(const std::string& filename)
try
{
    forceCopyFile(filename + ".bak2", filename + ".bak3");
    forceCopyFile(filename + ".bak1", filename + ".bak2");
    forceCopyFile(filename + ".bak0", filename + ".bak1");
    forceCopyFile(filename, filename + ".bak0");

} catch (const std::exception& ex)
{
    sf::cOut() << "Failed to backup '" << filename << "' (" << ex.what() << ")\n";
}

} // namespace

////////////////////////////////////////////////////////////
void saveProfileToFile(const Profile& profile, const char* filename)
try
{
    std::filesystem::create_directories("userdata");
    doRotatingBackup(filename);

    sf::writeToFile(filename, nlohmann::json(profile).dump());
} catch (const std::exception& ex)
{
    sf::cOut() << "Failed to save profile to file '" << filename << "' (" << ex.what() << ")\n";
}


////////////////////////////////////////////////////////////
void loadProfileFromFile(Profile& profile, const char* filename)
try
{
    std::string contents;
    sf::readFromFile(filename, contents);

    nlohmann::json::parse(contents).get_to(profile);
} catch (const std::exception& ex)
{
    sf::cOut() << "Failed to load profile from file '" << filename << "' (" << ex.what() << ")\n";
}


////////////////////////////////////////////////////////////
void savePlaythroughToFile(const Playthrough& playthrough, const char* filename)
try
{
    std::filesystem::create_directories("userdata");
    doRotatingBackup(filename);

    sf::writeToFile(filename, nlohmann::json(playthrough).dump());
} catch (const std::exception& ex)
{
    sf::cOut() << "Failed to save playthrough to file '" << filename << "' (" << ex.what() << ")\n";
}


////////////////////////////////////////////////////////////
sf::base::StringView backwardsCompatibilityLoadChecks(const Version& parsedVersion, Playthrough& playthrough)
{
    if (parsedVersion == currentVersion)
        return "";

    sf::cOut() << "Loaded playthrough version " << parsedVersion.major << "." << parsedVersion.minor << "."
               << parsedVersion.patch << " does not match current version " << currentVersion.major << "."
               << currentVersion.minor << "." << currentVersion.patch << '\n';

    // Prestige point scaling buff compensation
    if (parsedVersion.major == 1 && parsedVersion.minor <= 4)
    {
        const auto loadedPrestigeLevel = static_cast<PrestigePointsType>(playthrough.psvBubbleValue.currentValue());

        if (loadedPrestigeLevel > 0u)
        {
            sf::cOut() << "Adding missing prestige points...\n";

            const auto oldAccumulatedPPs = Playthrough::calculatePrestigePointReward(0u,
                                                                                     loadedPrestigeLevel,
                                                                                     /* levelBias */ 0u);
            const auto newAccumulatedPPs = Playthrough::calculatePrestigePointReward(0u,
                                                                                     loadedPrestigeLevel,
                                                                                     /* levelBias */ 1u);

            sf::cOut() << "Old accumulated pps: " << oldAccumulatedPPs << '\n'
                       << "New accumulated pps: " << newAccumulatedPPs << '\n'
                       << "Adding " << newAccumulatedPPs - oldAccumulatedPPs << " prestige points\n";

            playthrough.prestigePoints += newAccumulatedPPs - oldAccumulatedPPs;

            return "Awarded compensation prestige points!";
        }
    }

    return "";
}


////////////////////////////////////////////////////////////
sf::base::StringView loadPlaythroughFromFile(Playthrough& playthrough, const char* filename)
try
{
    std::string contents;
    sf::readFromFile(filename, contents);

    const auto parsed = nlohmann::json::parse(contents);
    parsed.get_to(playthrough);

    if constexpr (isDemoVersion)
    {
        if (playthrough.fullVersion)
        {
            playthrough             = {};
            playthrough.fullVersion = true;
            return "Cannot load non-demo playthrough in demo version!";
        }
    }
    else
    {
        if (!playthrough.fullVersion)
        {
            return "Thank you for purchasing BubbleByte!\nDemo limitations have been lifted.";
        }
    }

    const Version parsedVersion{.major = parsed[0][0], .minor = parsed[0][1], .patch = parsed[0][2]};
    return backwardsCompatibilityLoadChecks(parsedVersion, playthrough);

} catch (const std::exception& ex)
{
    sf::cOut() << "Failed to load playthrough from file '" << filename << "' (" << ex.what() << ")\n";
    return "";
}

// NOLINTEND(readability-identifier-naming, misc-use-internal-linkage)
