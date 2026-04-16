#include "Aliases.hpp"
#include "Bubble.hpp"
#include "Cat.hpp"
#include "Countdown.hpp"
#include "Doll.hpp"
#include "GameConstants.hpp"
#include "GameEvent.hpp"
#include "HellPortal.hpp"
#include "HexSession.hpp"
#include "Milestones.hpp"
#include "Playthrough.hpp"
#include "Profile.hpp"
#include "PurchasableScalingValue.hpp"
#include "Serialization.hpp"
#include "Shrine.hpp"
#include "Stats.hpp"
#include "Version.hpp"

#include "ExampleUtils/Timer.hpp"

#include "SFML/Base/Array.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"

#undef __cpp_lib_formatters
#undef __glibcxx_want_formatters

#define JSON_NO_IO
#include "json.hpp"

#pragma GCC diagnostic pop

#include "SFML/System/IO.hpp"
#include "SFML/System/Priv/Vec2Base.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Trait/IsArray.hpp"
#include "SFML/Base/Trait/IsSame.hpp"
#include "SFML/Base/Trait/RemoveCVRef.hpp"

#include <exception>
#include <filesystem>
#include <stdexcept>
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
    p = microseconds(j.get<base::I64>());
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


////////////////////////////////////////////////////////////
// `sf::base::Array<T, N>` round-trips as a JSON array of `T`. ADL picks
// these up because the type lives in `sf::base`.
template <typename T, SizeT N>
void to_json(nlohmann::json& j, const Array<T, N>& p)
{
    j = nlohmann::json::array();
    for (SizeT i = 0u; i < N; ++i)
        j.push_back(p.elements[i]);
}


////////////////////////////////////////////////////////////
template <typename T, SizeT N>
void from_json(const nlohmann::json& j, Array<T, N>& p)
{
    if (!j.is_array())
        return;

    const auto n = N < j.size() ? N : j.size();
    for (SizeT i = 0u; i < n; ++i)
        j[i].get_to(p.elements[i]);
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
// Forward declarations for recursive C-array handling.
template <typename T>
void writeArrayElement(nlohmann::json& arr, const T& value);

template <typename T>
void readArrayElement(const nlohmann::json& j, T& out);


////////////////////////////////////////////////////////////
template <typename T>
void writeField(nlohmann::json& j, const char* name, const T& field)
{
    if constexpr (!SFML_BASE_IS_ARRAY(T))
    {
        j[name] = field;
    }
    else
    {
        constexpr auto arraySize = getArraySize<T>::value;

        auto& arr = j[name];
        arr       = nlohmann::json::array();

        for (sf::base::SizeT i = 0u; i < arraySize; ++i)
            writeArrayElement(arr, field[i]);
    }
}


////////////////////////////////////////////////////////////
template <typename T>
void writeArrayElement(nlohmann::json& arr, const T& value)
{
    if constexpr (!SFML_BASE_IS_ARRAY(T))
    {
        arr.push_back(value);
    }
    else
    {
        constexpr auto arraySize = getArraySize<T>::value;

        nlohmann::json sub = nlohmann::json::array();
        for (sf::base::SizeT i = 0u; i < arraySize; ++i)
            writeArrayElement(sub, value[i]);

        arr.push_back(sub);
    }
}


////////////////////////////////////////////////////////////
template <typename T>
void readField(const nlohmann::json& j, const char* name, T& field)
{
    if (!j.is_object())
        return;

    const auto it = j.find(name);
    if (it == j.end())
        return;

    if constexpr (!SFML_BASE_IS_ARRAY(T))
    {
        it->template get_to<T>(field);
    }
    else
    {
        if (!it->is_array())
            return;

        constexpr auto arraySize = getArraySize<T>::value;
        const auto     n         = arraySize < it->size() ? arraySize : it->size();

        for (sf::base::SizeT i = 0u; i < n; ++i)
            readArrayElement((*it)[i], field[i]);
    }
}


////////////////////////////////////////////////////////////
template <typename T>
void readArrayElement(const nlohmann::json& j, T& out)
{
    if constexpr (!SFML_BASE_IS_ARRAY(T))
    {
        j.get_to(out);
    }
    else
    {
        if (!j.is_array())
            return;

        constexpr auto arraySize = getArraySize<T>::value;
        const auto     n         = arraySize < j.size() ? arraySize : j.size();

        for (sf::base::SizeT i = 0u; i < n; ++i)
            readArrayElement(j[i], out[i]);
    }
}


} // namespace


////////////////////////////////////////////////////////////
// Dispatch a named field to the write or read helper depending on direction.
// All `twoWaySerializer` bodies use `j` (the json) and `p` (the object) as
// convention, and this macro wraps the common lookup pattern.
#define FIELD(x)                    \
    do                              \
    {                               \
        if constexpr (Serialize)    \
            writeField(j, #x, p.x); \
        else                        \
            readField(j, #x, p.x);  \
    } while (0)


////////////////////////////////////////////////////////////
// Boilerplate-killer for the two-way serializer signature. `T` is the type
// being serialized; the body is a sequence of `FIELD(x);` calls (one per
// field). Each declared serializer is automatically picked up by the templated
// `to_json` / `from_json` dispatchers below.
#define DEFINE_TWO_WAY_SERIALIZER(T) \
    template <bool Serialize>        \
    void twoWaySerializer(isSameDecayed<nlohmann::json> auto&& j, isSameDecayed<T> auto&& p)


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
DEFINE_TWO_WAY_SERIALIZER(Bubble)
{
    FIELD(position);
    FIELD(velocity);

    FIELD(radius);
    FIELD(rotation);
    FIELD(hueMod);

    FIELD(repelledCountdown);
    FIELD(attractedCountdown);

    FIELD(type);
    FIELD(ephemeral);
    FIELD(hueSeed);
    FIELD(comboTimerMs);
    FIELD(comboClickCount);
}


////////////////////////////////////////////////////////////
DEFINE_TWO_WAY_SERIALIZER(Cat::AstroState)
{
    FIELD(startX);

    FIELD(velocityX);
    FIELD(particleTimer);

    FIELD(wrapped);
}


////////////////////////////////////////////////////////////
DEFINE_TWO_WAY_SERIALIZER(Cat)
{
    FIELD(spawnEffectTimer);

    FIELD(position);

    FIELD(wobbleRadians);
    FIELD(cooldown);

    FIELD(hue);

    FIELD(inspiredCountdown);
    FIELD(boostCountdown);

    FIELD(nameIdx);

    FIELD(hits);

    FIELD(type);

    FIELD(hexedTimer);
    FIELD(hexedCopyTimer);

    FIELD(moneyEarned);

    FIELD(astroState);

    FIELD(blinkCountdown);
    FIELD(flapCountdown);
    FIELD(yawnCountdown);

    FIELD(napTransition);
    FIELD(napSleepCountdown);
    FIELD(napShakeProgress);
    FIELD(napScheduleCountdownMs);
}


////////////////////////////////////////////////////////////
DEFINE_TWO_WAY_SERIALIZER(Doll)
{
    FIELD(position);
    FIELD(buffPower);
    FIELD(wobbleRadians);
    FIELD(hue);
    FIELD(catType);

    FIELD(tcActivation);
    FIELD(tcDeath);
}


////////////////////////////////////////////////////////////
DEFINE_TWO_WAY_SERIALIZER(HexSession)
{
    FIELD(catIdx);
    FIELD(dolls);
}


////////////////////////////////////////////////////////////
DEFINE_TWO_WAY_SERIALIZER(HellPortal)
{
    FIELD(position);
    FIELD(life);
    FIELD(catIdx);
}


////////////////////////////////////////////////////////////
DEFINE_TWO_WAY_SERIALIZER(EBubblefall)
{
    FIELD(regionCenterX);
    FIELD(regionWidth);
    FIELD(remainingMs);
    FIELD(subTickMs);
}


////////////////////////////////////////////////////////////
DEFINE_TWO_WAY_SERIALIZER(EInvincibleBubble)
{
    FIELD(remainingMs);
}


////////////////////////////////////////////////////////////
// Event kinds are tagged by a string `kind` so the variant can round-trip
// without positional coupling. Keep tags stable when renaming types.
inline constexpr const char* eventKindTag(const EBubblefall&)
{
    return "bubblefall";
}

inline constexpr const char* eventKindTag(const EInvincibleBubble&)
{
    return "invincible_bubble";
}


////////////////////////////////////////////////////////////
void to_json(nlohmann::json& j, const GameEvent& p)
{
    p.linearVisit(sf::base::OverloadSet{
        [&](const auto& e)
    {
        j["kind"] = eventKindTag(e);
        j["data"] = e;
    },
    });
}


////////////////////////////////////////////////////////////
void from_json(const nlohmann::json& j, GameEvent& p)
{
    if (!j.is_object())
        return;

    const auto kindIt = j.find("kind");
    const auto dataIt = j.find("data");
    if (kindIt == j.end() || dataIt == j.end())
        return;

    const auto kind = kindIt->get<std::string>();

    if (kind == "bubblefall")
    {
        EBubblefall e{};
        dataIt->get_to(e);
        p = GameEvent{e};
    }
    else if (kind == "invincible_bubble")
    {
        EInvincibleBubble e{};
        dataIt->get_to(e);
        p = GameEvent{e};
    }
}


////////////////////////////////////////////////////////////
DEFINE_TWO_WAY_SERIALIZER(Shrine)
{
    FIELD(position);
    FIELD(wobbleRadians);

    FIELD(tcActivation);
    FIELD(tcDeath);

    FIELD(collectedReward);
    FIELD(type);
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
DEFINE_TWO_WAY_SERIALIZER(BidirectionalTimer)
{
    FIELD(value);

    if constexpr (Serialize)
    {
        const bool dir = static_cast<bool>(p.direction);
        writeField(j, "direction", dir);
    }
    else
    {
        bool dir{};
        readField(j, "direction", dir);
        p.direction = static_cast<TimerDirection>(dir);
    }
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
DEFINE_TWO_WAY_SERIALIZER(TargetedCountdown)
{
    FIELD(value);
    FIELD(startingValue);
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
DEFINE_TWO_WAY_SERIALIZER(Stats)
{
    FIELD(secondsPlayed);

    FIELD(nBubblesPoppedByType);
    FIELD(revenueByType);

    FIELD(nBubblesHandPoppedByType);
    FIELD(revenueHandByType);

    FIELD(explosionRevenue);
    FIELD(flightRevenue);
    FIELD(hellPortalRevenue);

    FIELD(highestStarBubblePopCombo);
    FIELD(highestNovaBubblePopCombo);

    FIELD(nAbsorbedStarBubbles);

    FIELD(nSpellCasts);

    FIELD(nWitchcatRitualsPerCatType);
    FIELD(nWitchcatDollsCollected);

    FIELD(nMaintenances);
    FIELD(highestSimultaneousMaintenances);

    FIELD(nDisguises);

    FIELD(highestDPS);
}


////////////////////////////////////////////////////////////
DEFINE_TWO_WAY_SERIALIZER(Milestones)
{
    FIELD(firstCat);
    FIELD(firstUnicat);
    FIELD(firstDevilcat);
    FIELD(firstAstrocat);

    FIELD(fiveCats);
    FIELD(fiveUnicats);
    FIELD(fiveDevilcats);
    FIELD(fiveAstrocats);

    FIELD(tenCats);
    FIELD(tenUnicats);
    FIELD(tenDevilcats);
    FIELD(tenAstrocats);

    FIELD(prestigeLevel2);
    FIELD(prestigeLevel3);
    FIELD(prestigeLevel4);
    FIELD(prestigeLevel5);
    FIELD(prestigeLevel6);
    FIELD(prestigeLevel10);
    FIELD(prestigeLevel15);
    FIELD(prestigeLevel20);

    FIELD(revenue10000);
    FIELD(revenue100000);
    FIELD(revenue1000000);
    FIELD(revenue10000000);
    FIELD(revenue100000000);
    FIELD(revenue1000000000);

    FIELD(shrineCompletions);
}


////////////////////////////////////////////////////////////
DEFINE_TWO_WAY_SERIALIZER(SpeedrunningSplits)
{
    FIELD(prestigeLevel2);
    FIELD(prestigeLevel3);
    FIELD(prestigeLevel4);
    FIELD(prestigeLevel5);
}


////////////////////////////////////////////////////////////
DEFINE_TWO_WAY_SERIALIZER(Profile)
{
    if constexpr (Serialize)
    {
        auto version = currentVersion;
        writeField(j, "version", version);
    }
    else
    {
        Version version{};
        readField(j, "version", version);
    }

    FIELD(masterVolume);
    FIELD(musicVolume);

    FIELD(playAudioInBackground);
    FIELD(playComboEndSound);

    FIELD(selectedBGM);

    FIELD(minimapScale);
    FIELD(hudScale);
    FIELD(uiScale);

    FIELD(tipsEnabled);

    FIELD(backgroundOpacity);
    FIELD(selectedBackground);
    FIELD(alwaysShowDrawings);

    FIELD(showCatRange);
    FIELD(showRangesOnlyOnHover);
    FIELD(showCatText);
    FIELD(showParticles);
    FIELD(showTextParticles);
    FIELD(accumulatingCombo);
    FIELD(showCursorComboText);
    FIELD(useBubbleShader);

    FIELD(cursorTrailMode);
    FIELD(cursorTrailScale);

    FIELD(bsIridescenceStrength);
    FIELD(bsEdgeFactorMin);
    FIELD(bsEdgeFactorMax);
    FIELD(bsEdgeFactorStrength);
    FIELD(bsDistortionStrength);
    FIELD(bsBubbleLightness);
    FIELD(bsLensDistortion);

    FIELD(statsLifetime);

    FIELD(resWidth);

    FIELD(windowed);
    FIELD(vsync);

    FIELD(frametimeLimit);

    FIELD(highVisibilityCursor);
    FIELD(multicolorCursor);

    FIELD(cursorHue);
    FIELD(cursorScale);

    FIELD(showCoinParticles);
    FIELD(showDpsMeter);

    FIELD(enableNotifications);
    FIELD(showFullManaNotification);

    FIELD(unlockedAchievements);

    FIELD(uiUnlocks);

    FIELD(ppSVibrance);
    FIELD(ppSSaturation);
    FIELD(ppSLightness);
    FIELD(ppSSharpness);

    FIELD(showBubbles);
    FIELD(invertMouseButtons);
    FIELD(showDollParticleBorder);

    FIELD(catDragPressDuration);
    FIELD(playWitchRitualSounds);
    FIELD(enableScreenShake);
    FIELD(enableCatBobbing);
    FIELD(catRangeOutlineThickness);

    FIELD(showCursorComboBar);
    FIELD(sfxVolume);

    FIELD(hideMaxedOutPurchasables);
    FIELD(hideCategorySeparators);

    FIELD(autobatchMode);

    FIELD(ppSBlur);
    FIELD(ppBGVibrance);
    FIELD(ppBGSaturation);
    FIELD(ppBGLightness);
    FIELD(ppBGSharpness);
    FIELD(ppBGBlur);
}

////////////////////////////////////////////////////////////
DEFINE_TWO_WAY_SERIALIZER(GameConstants::CloudModifier)
{
    FIELD(positionOffset);
    FIELD(xExtentMult);
}

////////////////////////////////////////////////////////////
DEFINE_TWO_WAY_SERIALIZER(GameConstants::SpriteAttachment)
{
    FIELD(positionOffset);
    FIELD(origin);
}

////////////////////////////////////////////////////////////
DEFINE_TWO_WAY_SERIALIZER(GameConstants::BubblefallTuning)
{
    FIELD(durationMs);
    FIELD(regionWidth);
    FIELD(spawnIntervalMs);
    FIELD(bubblesPerTick);
    FIELD(initialVelocityY);
    FIELD(velocityJitterY);
    FIELD(velocityJitterX);
    FIELD(attackRatio);
    FIELD(releaseRatio);
}

////////////////////////////////////////////////////////////
DEFINE_TWO_WAY_SERIALIZER(GameConstants::InvincibleBubbleTuning)
{
    FIELD(minRadius);
    FIELD(maxRadius);
    FIELD(initialVelocityY);
    FIELD(velocityJitterY);
    FIELD(velocityJitterX);
    FIELD(maxVelocityY);
    FIELD(spawnYOffsetTopMin);
    FIELD(spawnYOffsetTopMax);
    FIELD(comboTimerMaxMs);
    FIELD(maxClicks);
    FIELD(rewardScalePerClick);
    FIELD(rewardClickExponent);
    FIELD(ambientRepelRadius);
    FIELD(ambientRepelStrength);
    FIELD(popRepelRadius);
    FIELD(popRepelImpulse);
    FIELD(clickAbsorbRadius);
    FIELD(clickAbsorbSpeed);
    FIELD(radiusGrowthPerClick);
    FIELD(radiusGrowthMax);
    FIELD(spawnEdgeMarginPx);
    FIELD(payoutCoinDelayMs);
    FIELD(payoutCoinsPerClick);
    FIELD(payoutMaxCoins);
    FIELD(burstSpeedMin);
    FIELD(burstSpeedMax);
    FIELD(burstDampingPerSec);
    FIELD(burstSettleDelayMs);
}

DEFINE_TWO_WAY_SERIALIZER(GameConstants::EventsTuning)
{
    FIELD(minSpawnIntervalMs);
    FIELD(maxSpawnIntervalMs);
    FIELD(bubblefall);
    FIELD(invincibleBubble);
}

////////////////////////////////////////////////////////////
DEFINE_TWO_WAY_SERIALIZER(GameConstants)
{
    FIELD(catTailOffsetsByType);
    FIELD(catDrawOffsetsByType);
    FIELD(catEyeOffsetsByType);
    FIELD(catHueByType);
    FIELD(cloudModifiers);
    FIELD(catCloudOpacity);
    FIELD(catCloudCircleCount);
    FIELD(catCloudScale);
    FIELD(catCloudXExtent);
    FIELD(catCloudBaseYOffset);
    FIELD(catCloudExtraYOffset);
    FIELD(catCloudDraggedOffset);
    FIELD(catCloudLobeLift);
    FIELD(catCloudWobbleX);
    FIELD(catCloudWobbleY);
    FIELD(catCloudRadiusBase);
    FIELD(catCloudRadiusLobe);
    FIELD(catCloudRadiusWobble);
    FIELD(catAttachmentDraggedOffsetY);
    FIELD(devilBackTail);
    FIELD(brainJarOffset);
    FIELD(uniWingsOffset);
    FIELD(uniWingsOriginOffsetFromCenter);
    FIELD(devilBookOffset);
    FIELD(devilPawIdleOffset);
    FIELD(devilPawDraggedOffset);
    FIELD(duckFlag);
    FIELD(smartHatOffset);
    FIELD(earFlapOffset);
    FIELD(yawnOffset);
    FIELD(smartDiploma);
    FIELD(astroFlag);
    FIELD(engiWrench);
    FIELD(attractoMagnet);
    FIELD(tail);
    FIELD(uniTailExtraOffset);
    FIELD(uniTailOriginOffset);
    FIELD(mouseProp);
    FIELD(eyelidOffset);
    FIELD(regularPawIdleOffset);
    FIELD(regularPawDraggedOffset);
    FIELD(copyMaskOffset);
    FIELD(copyMaskOrigin);
    FIELD(catNameTextOffsetY);
    FIELD(catStatusTextOffsetY);
    FIELD(catCooldownBarOffsetY);
    FIELD(debugDrawCatCenterMarker);
    FIELD(debugDrawCatBodyBounds);
    FIELD(events);
}

////////////////////////////////////////////////////////////
DEFINE_TWO_WAY_SERIALIZER(BubbleIgnoreFlags)
{
    FIELD(normal);
    FIELD(star);
    FIELD(bomb);
}

////////////////////////////////////////////////////////////
DEFINE_TWO_WAY_SERIALIZER(Playthrough::Permanent)
{
    FIELD(starterPackPurchased);

    FIELD(multiPopPurchased);
    FIELD(smartCatsPurchased);
    FIELD(geniusCatsPurchased);

    FIELD(windPurchased);

    FIELD(astroCatInspirePurchased);

    FIELD(starpawConversionIgnoreBombs);
    FIELD(starpawNova);

    FIELD(repulsoCatFilterPurchased);
    FIELD(repulsoCatConverterPurchased);
    FIELD(repulsoCatNovaConverterPurchased);

    FIELD(attractoCatFilterPurchased);

    FIELD(witchCatBuffPowerScalesWithNCats);
    FIELD(witchCatBuffPowerScalesWithMapSize);
    FIELD(witchCatBuffFewerDolls);
    FIELD(witchCatBuffFlammableDolls);
    FIELD(witchCatBuffOrbitalDolls);

    FIELD(shrineCompletedOnceByCatType);

    FIELD(unsealedByType);

    FIELD(wizardCatDoubleMewltiplierDuration);
    FIELD(wizardCatDoubleStasisFieldDuration);

    FIELD(unicatTranscendencePurchased);
    FIELD(unicatTranscendenceAOEPurchased);

    FIELD(devilcatHellsingedPurchased);

    FIELD(unicatTranscendenceEnabled);
    FIELD(devilcatHellsingedEnabled);

    FIELD(autocastPurchased);
    FIELD(autocastIndex);
}

////////////////////////////////////////////////////////////
DEFINE_TWO_WAY_SERIALIZER(Playthrough)
{
    if constexpr (Serialize)
    {
        auto version = currentVersion;
        writeField(j, "version", version);
    }
    else
    {
        Version version{};
        readField(j, "version", version);
    }

    FIELD(seed);
    FIELD(nextCatNamePerType);

    FIELD(psvComboStartTime);
    FIELD(psvMapExtension);
    FIELD(psvShrineActivation);
    FIELD(psvBubbleCount);
    FIELD(psvSpellCount);
    FIELD(psvBubbleValue);
    FIELD(psvExplosionRadiusMult);
    FIELD(psvStarpawPercentage);
    FIELD(psvMewltiplierMult);
    FIELD(psvDarkUnionPercentage);

    FIELD(psvPerCatType);

    FIELD(psvCooldownMultsPerCatType);

    FIELD(psvRangeDivsPerCatType);

    FIELD(psvPPMultiPopRange);
    FIELD(psvPPInspireDurationMult);
    FIELD(psvPPManaCooldownMult);
    FIELD(psvPPManaMaxMult);
    FIELD(psvPPMouseCatGlobalBonusMult);
    FIELD(psvPPEngiCatGlobalBonusMult);
    FIELD(psvPPRepulsoCatConverterChance);
    FIELD(psvPPWitchCatBuffDuration);
    FIELD(psvPPUniRitualBuffPercentage);
    FIELD(psvPPDevilRitualBuffPercentage);

    FIELD(money);

    FIELD(prestigePoints);

    FIELD(comboPurchased);
    FIELD(mapPurchased);

    FIELD(manaTimer);
    FIELD(mana);
    FIELD(absorbingWisdom);
    FIELD(wisdom);
    FIELD(mewltiplierAuraTimer);
    FIELD(stasisFieldTimer);

    FIELD(mouseCatCombo);
    FIELD(mouseCatComboCountdown);

    FIELD(copycatCopiedCatType);

    FIELD(perm);

    FIELD(multiPopEnabled);
    FIELD(multiPopMouseCatEnabled);
    FIELD(windStrength);
    FIELD(geniusCatIgnoreBubbles);
    FIELD(repulsoCatIgnoreBubbles);
    FIELD(attractoCatIgnoreBubbles);
    FIELD(repulsoCatConverterEnabled);

    FIELD(bubbles);
    FIELD(cats);
    FIELD(shrines);
    FIELD(hexSessions);
    FIELD(copyHexSessions);
    FIELD(hellPortals);

    FIELD(activeEvents);
    FIELD(nextEventSpawnMs);
    FIELD(nextBubbleHueSeed);

    FIELD(nShrinesCompleted);

    FIELD(statsTotal);
    FIELD(statsSession);
    FIELD(milestones);

    FIELD(achAstrocatPopBomb);
    FIELD(achAstrocatInspireByType);

    FIELD(buffCountdownsPerType);

    FIELD(prestigeTipShown);
    FIELD(shrineHoverTipShown);
    FIELD(shrineActivateTipShown);
    FIELD(dollTipShown);
    FIELD(spendPPTipShown);
    FIELD(napTipShown);

    FIELD(scriptedNapDone);
    FIELD(scriptedNapPendingCountdown);
    FIELD(anyCatEverWokenFromNap);
    FIELD(shrinesSpawned);

    FIELD(laserPopEnabled);

    FIELD(disableAstrocatFlight);

    FIELD(speedrunStartTime);
    FIELD(speedrunSplits);

    FIELD(fullVersion);
}

////////////////////////////////////////////////////////////
DEFINE_TWO_WAY_SERIALIZER(Version)
{
    FIELD(major);
    FIELD(minor);
    FIELD(patch);
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
    sf::cOut() << "Failed to copy file from '" << from.string() << "' to '" << to.string() << "'\n";
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

    if (!sf::writeToFile(filename, nlohmann::json(profile).dump()))
        throw std::runtime_error("writeToFile failed");
} catch (const std::exception& ex)
{
    sf::cOut() << "Failed to save profile to file '" << filename << "' (" << ex.what() << ")\n";
}


////////////////////////////////////////////////////////////
void loadProfileFromFile(Profile& profile, const char* filename)
try
{
    sf::base::String contents;

    if (!sf::readFromFile(filename, contents))
        throw std::runtime_error("readFromFile failed");

    const auto parsed = nlohmann::json::parse(contents);

    // Old saves used a JSON array at the root; new saves are objects.
    if (!parsed.is_object())
    {
        sf::cOut() << "Profile '" << filename
                   << "' is in the legacy array format and cannot be loaded. Resetting to defaults.\n";
        profile = Profile{};
        return;
    }

    parsed.get_to(profile);
} catch (const std::exception& ex)
{
    sf::cOut() << "Failed to load profile from file '" << filename << "' (" << ex.what() << ")\n";
}

////////////////////////////////////////////////////////////
void saveGameConstantsToFile(const GameConstants& gameConstants, const char* filename)
try
{
    const std::filesystem::path path{filename};

    if (path.has_parent_path())
        std::filesystem::create_directories(path.parent_path());

    doRotatingBackup(filename);

    if (!sf::writeToFile(filename, nlohmann::json(gameConstants).dump(2)))
        throw std::runtime_error("writeToFile failed");
} catch (const std::exception& ex)
{
    sf::cOut() << "Failed to save game constants to file '" << filename << "' (" << ex.what() << ")\n";
}


////////////////////////////////////////////////////////////
void loadGameConstantsFromFile(GameConstants& gameConstants, const char* filename)
try
{
    sf::base::String contents;

    if (!sf::readFromFile(filename, contents))
        throw std::runtime_error("readFromFile failed");

    nlohmann::json::parse(contents).get_to(gameConstants);
} catch (const std::exception& ex)
{
    sf::cOut() << "Failed to load game constants from file '" << filename << "' (" << ex.what() << ")\n";
}


////////////////////////////////////////////////////////////
void savePlaythroughToFile(const Playthrough& playthrough, const char* filename)
try
{
    std::filesystem::create_directories("userdata");
    doRotatingBackup(filename);

    if (!sf::writeToFile(filename, nlohmann::json(playthrough).dump()))
        throw std::runtime_error("writeToFile failed");
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

    if (!sf::readFromFile(filename, contents))
        throw std::runtime_error("readFromFile failed");

    const auto parsed = nlohmann::json::parse(contents);

    // Old saves used a JSON array at the root; new saves are objects.
    if (!parsed.is_object())
    {
        sf::cOut() << "Playthrough '" << filename << "' is in the legacy array format and cannot be loaded.\n";
        playthrough = Playthrough{};
        return "Your save is from an older version with an incompatible\n"
               "format and could not be loaded. A fresh playthrough has been started.";
    }

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

    Version parsedVersion{};
    if (const auto it = parsed.find("version"); it != parsed.end())
        it->get_to(parsedVersion);

    return backwardsCompatibilityLoadChecks(parsedVersion, playthrough);

} catch (const std::exception& ex)
{
    sf::cOut() << "Failed to load playthrough from file '" << filename << "' (" << ex.what() << ")\n";
    return "";
}

// NOLINTEND(readability-identifier-naming, misc-use-internal-linkage)
