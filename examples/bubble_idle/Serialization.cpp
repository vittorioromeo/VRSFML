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
#include "SFML/System/Vector2.hpp"

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
void to_json(nlohmann::json& j, const Vector2<T>& p)
{
    j[0] = p.x;
    j[1] = p.y;
}


////////////////////////////////////////////////////////////
template <typename T>
void from_json(const nlohmann::json& j, Vector2<T>& p)
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
template <bool Serialize, typename TField>
[[gnu::always_inline]] inline void twoWay(auto&& j, const sf::base::SizeT index, TField&& field)
{
    if constexpr (Serialize)
    {
        serialize<SFML_BASE_REMOVE_CVREF(TField)>(j, index, field);
    }
    else
    {
        deserialize<SFML_BASE_REMOVE_CVREF(TField)>(j, index, field);
    }
}

////////////////////////////////////////////////////////////
template <bool Serialize>
[[gnu::always_inline]] inline sf::base::SizeT twoWayAll(sf::base::SizeT index, auto&& j, auto&&... fields)
{
    (twoWay<Serialize>(j, index++, fields), ...);
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
#define FIELD(...) p.__VA_ARGS__


////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<Bubble> auto&& p)
{
    twoWayAll<Serialize>(0u,
                         j,

                         FIELD(position),
                         FIELD(velocity),

                         FIELD(radius),
                         FIELD(rotation),
                         FIELD(hueMod),

                         FIELD(repelledCountdown),
                         FIELD(attractedCountdown),

                         FIELD(type));
}


////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<Cat::AstroState> auto&& p)
{
    twoWayAll<Serialize>(0u,
                         j,

                         FIELD(startX),

                         FIELD(velocityX),
                         FIELD(particleTimer),

                         FIELD(wrapped));
}


////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<Cat> auto&& p)
{
    twoWayAll<Serialize>(0u,
                         j,

                         FIELD(spawnEffectTimer),

                         FIELD(position),

                         FIELD(wobbleRadians),
                         FIELD(cooldown),

                         // FIELD(pawPosition),
                         // FIELD(pawRotation),

                         // FIELD(pawOpacity),

                         FIELD(hue),

                         FIELD(inspiredCountdown),
                         FIELD(boostCountdown),

                         FIELD(nameIdx),

                         // FIELD(textStatusShakeEffect),
                         // FIELD(textMoneyShakeEffect),

                         FIELD(hits),

                         FIELD(type),

                         FIELD(hexedTimer),
                         FIELD(hexedCopyTimer),

                         FIELD(moneyEarned),

                         FIELD(astroState),

                         FIELD(blinkCountdown),
                         // FIELD(blinkAnimCountdown),

                         FIELD(flapCountdown),
                         // FIELD(flapAnimCountdown),

                         FIELD(yawnCountdown)
                         // FIELD(yawnAnimCountdown)
    );
}


////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<Doll> auto&& p)
{
    twoWayAll<Serialize>(0u,
                         j,

                         FIELD(position),
                         FIELD(buffPower),
                         FIELD(wobbleRadians),
                         FIELD(hue),
                         FIELD(catType),

                         FIELD(tcActivation),
                         FIELD(tcDeath));
}


////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<HellPortal> auto&& p)
{
    twoWayAll<Serialize>(0u,
                         j,

                         FIELD(position),
                         FIELD(life),
                         FIELD(catIdx));
}


////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<Shrine> auto&& p)
{
    twoWayAll<Serialize>(0u,
                         j,

                         FIELD(position),

                         FIELD(wobbleRadians),

                         FIELD(tcActivation),
                         FIELD(tcDeath),

                         // FIELD(textStatusShakeEffect),

                         FIELD(collectedReward),

                         FIELD(type));
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
    twoWayAll<Serialize>(0u,
                         j,

                         FIELD(secondsPlayed),

                         FIELD(nBubblesPoppedByType),
                         FIELD(revenueByType),

                         FIELD(nBubblesHandPoppedByType),
                         FIELD(revenueHandByType),

                         FIELD(explosionRevenue),
                         FIELD(flightRevenue),
                         FIELD(hellPortalRevenue),

                         FIELD(highestStarBubblePopCombo),
                         FIELD(highestNovaBubblePopCombo),

                         FIELD(nAbsorbedStarBubbles),

                         FIELD(nSpellCasts),

                         FIELD(nWitchcatRitualsPerCatType),
                         FIELD(nWitchcatDollsCollected),

                         FIELD(nMaintenances),
                         FIELD(highestSimultaneousMaintenances),

                         FIELD(nDisguises),

                         FIELD(highestDPS));
}


////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<Milestones> auto&& p)
{
    twoWayAll<Serialize>(0u,
                         j,

                         FIELD(firstCat),
                         FIELD(firstUnicat),
                         FIELD(firstDevilcat),
                         FIELD(firstAstrocat),

                         FIELD(fiveCats),
                         FIELD(fiveUnicats),
                         FIELD(fiveDevilcats),
                         FIELD(fiveAstrocats),

                         FIELD(tenCats),
                         FIELD(tenUnicats),
                         FIELD(tenDevilcats),
                         FIELD(tenAstrocats),

                         FIELD(prestigeLevel2),
                         FIELD(prestigeLevel3),
                         FIELD(prestigeLevel4),
                         FIELD(prestigeLevel5),
                         FIELD(prestigeLevel6),
                         FIELD(prestigeLevel10),
                         FIELD(prestigeLevel15),
                         FIELD(prestigeLevel20),

                         FIELD(revenue10000),
                         FIELD(revenue100000),
                         FIELD(revenue1000000),
                         FIELD(revenue10000000),
                         FIELD(revenue100000000),
                         FIELD(revenue1000000000),

                         FIELD(shrineCompletions));
}


////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<SpeedrunningSplits> auto&& p)
{
    twoWayAll<Serialize>(0u,
                         j,

                         FIELD(prestigeLevel2),
                         FIELD(prestigeLevel3),
                         FIELD(prestigeLevel4),
                         FIELD(prestigeLevel5));
}


////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<Profile> auto&& p)
{
    auto version = currentVersion;
    twoWay<Serialize>(j, 0u, version);

    twoWayAll<Serialize>(1u,
                         j,

                         FIELD(masterVolume),
                         FIELD(musicVolume),

                         FIELD(playAudioInBackground),
                         FIELD(playComboEndSound),

                         FIELD(selectedBGM),

                         FIELD(minimapScale),
                         FIELD(hudScale),
                         FIELD(uiScale),

                         FIELD(tipsEnabled),

                         FIELD(backgroundOpacity),
                         FIELD(selectedBackground),
                         FIELD(alwaysShowDrawings),

                         FIELD(showCatText),
                         FIELD(showCatRange),
                         FIELD(showParticles),
                         FIELD(showTextParticles),
                         FIELD(accumulatingCombo),
                         FIELD(showCursorComboText),
                         FIELD(useBubbleShader),

                         FIELD(cursorTrailMode),
                         FIELD(cursorTrailScale),

                         FIELD(bsIridescenceStrength),
                         FIELD(bsEdgeFactorMin),
                         FIELD(bsEdgeFactorMax),
                         FIELD(bsEdgeFactorStrength),
                         FIELD(bsDistortionStrength),
                         FIELD(bsBubbleLightness),
                         FIELD(bsLensDistortion),

                         FIELD(statsLifetime),

                         FIELD(resWidth),

                         FIELD(windowed),
                         FIELD(vsync),

                         FIELD(frametimeLimit),

                         FIELD(highVisibilityCursor),
                         FIELD(multicolorCursor),

                         FIELD(cursorHue),
                         FIELD(cursorScale),

                         FIELD(showCoinParticles),
                         FIELD(showDpsMeter),

                         FIELD(enableNotifications),
                         FIELD(showFullManaNotification),

                         FIELD(unlockedAchievements),

                         FIELD(uiUnlocks),

                         FIELD(ppSVibrance),
                         FIELD(ppSSaturation),
                         FIELD(ppSLightness),
                         FIELD(ppSSharpness),

                         FIELD(showBubbles),
                         FIELD(invertMouseButtons),
                         FIELD(showDollParticleBorder),

                         FIELD(catDragPressDuration),
                         FIELD(playWitchRitualSounds),
                         FIELD(enableScreenShake),
                         FIELD(enableCatBobbing),
                         FIELD(catRangeOutlineThickness),

                         FIELD(showCursorComboBar),
                         FIELD(sfxVolume),

                         FIELD(hideMaxedOutPurchasables),
                         FIELD(hideCategorySeparators));
}

////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<BubbleIgnoreFlags> auto&& p)
{
    twoWayAll<Serialize>(0u,
                         j,

                         FIELD(normal),
                         FIELD(star),
                         FIELD(bomb));
}

////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<Playthrough::Permanent> auto&& p)
{
    twoWayAll<Serialize>(0u,
                         j,

                         FIELD(starterPackPurchased),

                         FIELD(multiPopPurchased),
                         FIELD(smartCatsPurchased),
                         FIELD(geniusCatsPurchased),

                         FIELD(windPurchased),

                         FIELD(astroCatInspirePurchased),

                         FIELD(starpawConversionIgnoreBombs),
                         FIELD(starpawNova),

                         FIELD(repulsoCatFilterPurchased),
                         FIELD(repulsoCatConverterPurchased),
                         FIELD(repulsoCatNovaConverterPurchased),

                         FIELD(attractoCatFilterPurchased),

                         FIELD(witchCatBuffPowerScalesWithNCats),
                         FIELD(witchCatBuffPowerScalesWithMapSize),
                         FIELD(witchCatBuffFewerDolls),
                         FIELD(witchCatBuffFlammableDolls),
                         FIELD(witchCatBuffOrbitalDolls),

                         FIELD(shrineCompletedOnceByCatType),

                         FIELD(unsealedByType),

                         FIELD(wizardCatDoubleMewltiplierDuration),
                         FIELD(wizardCatDoubleStasisFieldDuration),

                         FIELD(unicatTranscendencePurchased),
                         FIELD(unicatTranscendenceAOEPurchased),

                         FIELD(devilcatHellsingedPurchased),

                         FIELD(unicatTranscendenceEnabled),
                         FIELD(devilcatHellsingedEnabled),

                         FIELD(autocastPurchased),
                         FIELD(autocastIndex));
}

////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<Playthrough> auto&& p)
{
    auto version = currentVersion;
    twoWay<Serialize>(j, 0u, version);

    twoWayAll<Serialize>(
        1u,
        j,

        FIELD(seed),
        FIELD(nextCatNamePerType),

        FIELD(psvComboStartTime),
        FIELD(psvMapExtension),
        FIELD(psvShrineActivation),
        FIELD(psvBubbleCount),
        FIELD(psvSpellCount),
        FIELD(psvBubbleValue),
        FIELD(psvExplosionRadiusMult),
        FIELD(psvStarpawPercentage),
        FIELD(psvMewltiplierMult),
        FIELD(psvDarkUnionPercentage),

        FIELD(psvPerCatType),

        FIELD(psvCooldownMultsPerCatType),

        FIELD(psvRangeDivsPerCatType),

        FIELD(psvPPMultiPopRange),
        FIELD(psvPPInspireDurationMult),
        FIELD(psvPPManaCooldownMult),
        FIELD(psvPPManaMaxMult),
        FIELD(psvPPMouseCatGlobalBonusMult),
        FIELD(psvPPEngiCatGlobalBonusMult),
        FIELD(psvPPRepulsoCatConverterChance),
        FIELD(psvPPWitchCatBuffDuration),
        FIELD(psvPPUniRitualBuffPercentage),
        FIELD(psvPPDevilRitualBuffPercentage),

        FIELD(money),

        FIELD(prestigePoints),

        FIELD(comboPurchased),
        FIELD(mapPurchased),

        FIELD(manaTimer),
        FIELD(mana),
        FIELD(absorbingWisdom),
        FIELD(wisdom),
        FIELD(mewltiplierAuraTimer),
        FIELD(stasisFieldTimer),

        FIELD(mouseCatCombo),
        FIELD(mouseCatComboCountdown),

        FIELD(copycatCopiedCatType),

        FIELD(perm),

        FIELD(multiPopEnabled),
        FIELD(multiPopMouseCatEnabled),
        FIELD(windStrength),
        FIELD(geniusCatIgnoreBubbles),
        FIELD(repulsoCatIgnoreBubbles),
        FIELD(attractoCatIgnoreBubbles),
        FIELD(repulsoCatConverterEnabled),

        FIELD(bubbles),
        FIELD(cats),
        FIELD(shrines),
        FIELD(dolls),
        FIELD(copyDolls),
        FIELD(hellPortals),

        FIELD(nShrinesCompleted),

        FIELD(statsTotal),
        FIELD(statsSession),
        FIELD(milestones),

        FIELD(achAstrocatPopBomb),
        FIELD(achAstrocatInspireByType),

        FIELD(buffCountdownsPerType),

        FIELD(prestigeTipShown),
        FIELD(shrineHoverTipShown),
        FIELD(shrineActivateTipShown),
        FIELD(dollTipShown),
        FIELD(spendPPTipShown),
        FIELD(shrinesSpawned),

        FIELD(laserPopEnabled),

        FIELD(disableAstrocatFlight),

        FIELD(speedrunStartTime),
        FIELD(speedrunSplits));
}

////////////////////////////////////////////////////////////
template <bool Serialize>
void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<Version> auto&& p)
{
    twoWayAll<Serialize>(0u,
                         j,

                         FIELD(major),
                         FIELD(minor),
                         FIELD(patch));
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

    const Version parsedVersion{.major = parsed[0][0], .minor = parsed[0][1], .patch = parsed[0][2]};
    return backwardsCompatibilityLoadChecks(parsedVersion, playthrough);

} catch (const std::exception& ex)
{
    sf::cOut() << "Failed to load playthrough from file '" << filename << "' (" << ex.what() << ")\n";
    return "";
}

// NOLINTEND(readability-identifier-naming, misc-use-internal-linkage)
