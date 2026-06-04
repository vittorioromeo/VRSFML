#include "Aliases.hpp"
#include "Bubble.hpp"
#include "Cat.hpp"
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
#include "cJSON.h"

#include "ExampleUtils/Progress.hpp"

#include "SFML/System/Fmt/FmtPath.hpp"
#include "SFML/System/IO.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/PathStreamOp.hpp" // IWYU pragma: keep
#include "SFML/System/Priv/Vec2Base.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Array.hpp"
#include "SFML/Base/Fmt/Fmt.hpp"
#include "SFML/Base/Fmt/FmtNumeric.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/OverloadSet.hpp"
#include "SFML/Base/ScopeGuard.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Trait/IsEnum.hpp"
#include "SFML/Base/Trait/IsFloatingPoint.hpp"
#include "SFML/Base/Trait/IsIntegral.hpp"
#include "SFML/Base/Trait/IsSame.hpp"
#include "SFML/Base/Trait/RemoveCVRef.hpp"
#include "SFML/Base/Trait/UnderlyingType.hpp"
#include "SFML/Base/Vector.hpp"

// NOLINTBEGIN(readability-identifier-naming, misc-use-internal-linkage)


////////////////////////////////////////////////////////////
// Map `Serialize == true` (writing) to `cJSON*` and `Serialize == false`
// (reading) to `const cJSON*`. Hand-rolled to avoid `<type_traits>`.
////////////////////////////////////////////////////////////
namespace
{
template <bool>
struct JsonNodeImpl;
template <>
struct JsonNodeImpl<true>
{
    using type = cJSON*;
};
template <>
struct JsonNodeImpl<false>
{
    using type = const cJSON*;
};
template <bool Serialize>
using JsonNode = typename JsonNodeImpl<Serialize>::type;
} // namespace


////////////////////////////////////////////////////////////
// Forward declaration of the per-type two-way serializer. Each
// `DEFINE_TWO_WAY_SERIALIZER(T)` block (below) defines a concrete
// overload for a specific `T`. The `requires(false)` here ensures
// this declaration never wins overload resolution against a real
// DEFINE block -- its only job is to make the primary `toJsonValue`
// template's `if constexpr (requires { twoWaySerializer<...> })`
// branch parseable before the DEFINE blocks come into view.
////////////////////////////////////////////////////////////
template <bool Serialize, typename T>
    requires(false)
void twoWaySerializer(JsonNode<Serialize> j, T&& p);


////////////////////////////////////////////////////////////
// `toJsonValue` / `fromJsonValue` form the uniform read/write
// dispatch table. All complex types (Vec2, Time, Optional, Vector,
// Array, C-arrays, user structs) decay through these.
////////////////////////////////////////////////////////////
namespace
{
////////////////////////////////////////////////////////////
// Primary catch-all templates. These are the fall-through
// for any type that doesn't match a more-specialized overload
// (primitives, enums, Vec2, Time, Optional, Vector, Array,
// C-arrays, or the explicit non-template overloads further
// down). At instantiation, the `if constexpr` branch dispatches
// via `twoWaySerializer<...>` when a `DEFINE_TWO_WAY_SERIALIZER`
// for `T` exists; otherwise the compile fails with a clear
// `static_assert`. The expression `requires { ... }` is
// re-evaluated at instantiation when all `DEFINE` blocks are
// visible via ADL on the argument types.
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard]] cJSON* toJsonValue(const T& v)
{
    if constexpr (requires(cJSON* obj) { twoWaySerializer<true>(obj, v); })
    {
        cJSON* obj = cJSON_CreateObject();
        twoWaySerializer<true>(obj, v);
        return obj;
    }
    else
    {
        static_assert(false, "No toJsonValue overload for T and no DEFINE_TWO_WAY_SERIALIZER block.");
        return nullptr;
    }
}

template <typename T>
void fromJsonValue(const cJSON* j, T& out)
{
    if constexpr (requires { twoWaySerializer<false>(j, out); })
    {
        if (cJSON_IsObject(j))
            twoWaySerializer<false>(j, out);
    }
    else
    {
        static_assert(false, "No fromJsonValue overload for T and no DEFINE_TWO_WAY_SERIALIZER block.");
    }
}


////////////////////////////////////////////////////////////
// Forward declarations of custom non-template overloads. The
// definitions live further down in the same anonymous namespace.
// Forward-declaring them here makes them findable by Phase-1
// unqualified lookup inside the recursive templates `Vector<T>`,
// `Optional<T>`, etc. that come below the primary template.
////////////////////////////////////////////////////////////
[[nodiscard]] cJSON* toJsonValue(const Progress& p);
void                 fromJsonValue(const cJSON* j, Progress& p);

[[nodiscard]] cJSON* toJsonValue(const Countdown& p);
void                 fromJsonValue(const cJSON* j, Countdown& p);

[[nodiscard]] cJSON* toJsonValue(const PurchasableScalingValue& p);
void                 fromJsonValue(const cJSON* j, PurchasableScalingValue& p);

[[nodiscard]] cJSON* toJsonValue(const GameEvent& p);
void                 fromJsonValue(const cJSON* j, GameEvent& p);


////////////////////////////////////////////////////////////
// Primitive overloads. `bool` is special-cased so it round-trips
// as a JSON boolean. All other arithmetic types collapse into a
// single number-typed pair via the `__is_arithmetic` builtin --
// keeping the dispatch in one place avoids ~12 per-type overloads,
// most of which would otherwise be unused in any given TU and trip
// `-Wunused-function`.
////////////////////////////////////////////////////////////
inline cJSON* toJsonValue(bool v)
{
    return cJSON_CreateBool(v);
}

template <typename T>
    requires((SFML_BASE_IS_INTEGRAL(T) || SFML_BASE_IS_FLOATING_POINT(T)) && !SFML_BASE_IS_SAME(T, bool))
cJSON* toJsonValue(const T& v)
{
    return cJSON_CreateNumber(static_cast<double>(v));
}

inline void fromJsonValue(const cJSON* j, bool& out)
{
    if (cJSON_IsBool(j))
        out = cJSON_IsTrue(j);
    else if (cJSON_IsNumber(j))
        out = cJSON_GetNumberValue(j) != 0.0;
}

template <typename T>
    requires((SFML_BASE_IS_INTEGRAL(T) || SFML_BASE_IS_FLOATING_POINT(T)) && !SFML_BASE_IS_SAME(T, bool))
void fromJsonValue(const cJSON* j, T& out)
{
    if (cJSON_IsNumber(j))
        out = static_cast<T>(cJSON_GetNumberValue(j));
}


////////////////////////////////////////////////////////////
// Enum dispatch via compiler builtins (no `<type_traits>` include).
////////////////////////////////////////////////////////////
template <typename T>
    requires(SFML_BASE_IS_ENUM(T))
cJSON* toJsonValue(const T& v)
{
    return cJSON_CreateNumber(static_cast<double>(static_cast<SFML_BASE_UNDERLYING_TYPE(T)>(v)));
}

template <typename T>
    requires(SFML_BASE_IS_ENUM(T))
void fromJsonValue(const cJSON* j, T& out)
{
    if (cJSON_IsNumber(j))
        out = static_cast<T>(static_cast<SFML_BASE_UNDERLYING_TYPE(T)>(cJSON_GetNumberValue(j)));
}


////////////////////////////////////////////////////////////
// `sf::Vec2<T>` round-trips as a JSON array `[x, y]`.
////////////////////////////////////////////////////////////
template <typename T>
cJSON* toJsonValue(const sf::Vec2<T>& p)
{
    cJSON* arr = cJSON_CreateArray();
    cJSON_AddItemToArray(arr, toJsonValue(p.x));
    cJSON_AddItemToArray(arr, toJsonValue(p.y));
    return arr;
}

template <typename T>
void fromJsonValue(const cJSON* j, sf::Vec2<T>& p)
{
    if (!cJSON_IsArray(j) || cJSON_GetArraySize(j) < 2)
        return;
    fromJsonValue(cJSON_GetArrayItem(j, 0), p.x);
    fromJsonValue(cJSON_GetArrayItem(j, 1), p.y);
}


////////////////////////////////////////////////////////////
// `sf::Time` stored as integer microseconds.
////////////////////////////////////////////////////////////
inline cJSON* toJsonValue(const sf::Time& t)
{
    return cJSON_CreateNumber(static_cast<double>(t.asMicroseconds()));
}

inline void fromJsonValue(const cJSON* j, sf::Time& t)
{
    if (cJSON_IsNumber(j))
        t = sf::microseconds(static_cast<sf::base::I64>(cJSON_GetNumberValue(j)));
}


////////////////////////////////////////////////////////////
// `sf::base::Optional<T>` round-trips as `null` or its contents.
////////////////////////////////////////////////////////////
template <typename T>
cJSON* toJsonValue(const sf::base::Optional<T>& o)
{
    if (!o.hasValue())
        return cJSON_CreateNull();
    return toJsonValue(*o);
}

template <typename T>
void fromJsonValue(const cJSON* j, sf::base::Optional<T>& o)
{
    if (cJSON_IsNull(j))
    {
        o.reset();
        return;
    }

    o.emplace();
    fromJsonValue(j, *o);
}


////////////////////////////////////////////////////////////
// `sf::base::Vector<T>` as a JSON array.
////////////////////////////////////////////////////////////
template <typename T>
cJSON* toJsonValue(const sf::base::Vector<T>& v)
{
    cJSON* arr = cJSON_CreateArray();
    for (const auto& item : v)
        cJSON_AddItemToArray(arr, toJsonValue(item));
    return arr;
}

template <typename T>
void fromJsonValue(const cJSON* j, sf::base::Vector<T>& v)
{
    v.clear();
    if (!cJSON_IsArray(j))
        return;

    v.reserve(static_cast<sf::base::SizeT>(cJSON_GetArraySize(j)));

    const cJSON* child{};
    cJSON_ArrayForEach(child, j)
    {
        T item{};
        fromJsonValue(child, item);
        v.emplaceBack(SFML_BASE_MOVE(item));
    }
}


////////////////////////////////////////////////////////////
// `sf::base::Array<T, N>` as a JSON array.
////////////////////////////////////////////////////////////
template <typename T, sf::base::SizeT N>
cJSON* toJsonValue(const sf::base::Array<T, N>& a)
{
    cJSON* arr = cJSON_CreateArray();
    for (sf::base::SizeT i = 0u; i < N; ++i)
        cJSON_AddItemToArray(arr, toJsonValue(a.elements[i]));
    return arr;
}

template <typename T, sf::base::SizeT N>
void fromJsonValue(const cJSON* j, sf::base::Array<T, N>& a)
{
    if (!cJSON_IsArray(j))
        return;

    const auto n     = static_cast<sf::base::SizeT>(cJSON_GetArraySize(j));
    const auto count = N < n ? N : n;
    for (sf::base::SizeT i = 0u; i < count; ++i)
        fromJsonValue(cJSON_GetArrayItem(j, static_cast<int>(i)), a.elements[i]);
}


////////////////////////////////////////////////////////////
// Native C-arrays `T[N]` as JSON arrays (recurses for `T[N][M]`).
////////////////////////////////////////////////////////////
template <typename T, sf::base::SizeT N>
cJSON* toJsonValue(const T (&arr)[N])
{
    cJSON* a = cJSON_CreateArray();
    for (sf::base::SizeT i = 0u; i < N; ++i)
        cJSON_AddItemToArray(a, toJsonValue(arr[i]));
    return a;
}

template <typename T, sf::base::SizeT N>
void fromJsonValue(const cJSON* j, T (&arr)[N])
{
    if (!cJSON_IsArray(j))
        return;

    const auto n     = static_cast<sf::base::SizeT>(cJSON_GetArraySize(j));
    const auto count = N < n ? N : n;
    for (sf::base::SizeT i = 0u; i < count; ++i)
        fromJsonValue(cJSON_GetArrayItem(j, static_cast<int>(i)), arr[i]);
}


////////////////////////////////////////////////////////////
// Single-field user types: round-trip as the underlying value
// rather than as a JSON object, keeping the on-disk format compact.
////////////////////////////////////////////////////////////
#define DEFINE_SINGLE_FIELD_SERIALIZER(T, field)    \
    inline cJSON* toJsonValue(const T& p)           \
    {                                               \
        return toJsonValue((p).field);              \
    }                                               \
    inline void fromJsonValue(const cJSON* j, T& p) \
    {                                               \
        fromJsonValue(j, (p).field);                \
    }                                               \
    static_assert(true)

DEFINE_SINGLE_FIELD_SERIALIZER(Progress, value);
DEFINE_SINGLE_FIELD_SERIALIZER(Countdown, time);
DEFINE_SINGLE_FIELD_SERIALIZER(PurchasableScalingValue, nPurchases);

#undef DEFINE_SINGLE_FIELD_SERIALIZER


////////////////////////////////////////////////////////////
// Event kinds are tagged by a string `kind` so the variant can
// round-trip without positional coupling. Keep tags stable when
// renaming types.
////////////////////////////////////////////////////////////
// Both overloads are reached through `GameEvent::linearVisit`'s generic
// lambda; static analysis can't always prove every variant arm is hit
// in this TU, so silence `-Wunused-function`.
[[maybe_unused]] inline constexpr const char* eventKindTag(const EBubblefall&)
{
    return "bubblefall";
}

[[maybe_unused]] inline constexpr const char* eventKindTag(const EInvincibleBubble&)
{
    return "invincible_bubble";
}


////////////////////////////////////////////////////////////
// `GameEvent` is a variant tagged by `kind`. Keep tags stable.
////////////////////////////////////////////////////////////
inline cJSON* toJsonValue(const GameEvent& p)
{
    cJSON* obj = cJSON_CreateObject();
    p.linearVisit(sf::base::OverloadSet{
        [&](const auto& e)
    {
        cJSON_AddStringToObject(obj, "kind", eventKindTag(e));
        cJSON_AddItemToObject(obj, "data", toJsonValue(e));
    },
    });
    return obj;
}

inline void fromJsonValue(const cJSON* j, GameEvent& p)
{
    if (!cJSON_IsObject(j))
        return;

    const cJSON* kindItem = cJSON_GetObjectItemCaseSensitive(j, "kind");
    const cJSON* dataItem = cJSON_GetObjectItemCaseSensitive(j, "data");
    if (kindItem == nullptr || dataItem == nullptr || !cJSON_IsString(kindItem))
        return;

    const char* const kind = cJSON_GetStringValue(kindItem);
    if (kind == nullptr)
        return;

    const sf::base::StringView kindView{kind};
    if (kindView == "bubblefall")
    {
        EBubblefall e{};
        fromJsonValue(dataItem, e);
        p = GameEvent{e};
    }
    else if (kindView == "invincible_bubble")
    {
        EInvincibleBubble e{};
        fromJsonValue(dataItem, e);
        p = GameEvent{e};
    }
}


////////////////////////////////////////////////////////////
// Object field helpers used directly by the `FIELD(...)` macro
// inside every `DEFINE_TWO_WAY_SERIALIZER(T)` block.
////////////////////////////////////////////////////////////
template <typename T>
void writeField(cJSON* obj, const char* name, const T& field)
{
    cJSON_AddItemToObject(obj, name, toJsonValue(field));
}


////////////////////////////////////////////////////////////
template <typename T>
void readField(const cJSON* obj, const char* name, T& field)
{
    const cJSON* item = cJSON_GetObjectItemCaseSensitive(obj, name);
    if (item != nullptr)
        fromJsonValue(item, field);
}

} // namespace


////////////////////////////////////////////////////////////
template <typename T, typename U>
concept isSameDecayed = sf::base::isSame<SFML_BASE_REMOVE_CVREF(T), SFML_BASE_REMOVE_CVREF(U)>;


////////////////////////////////////////////////////////////
// Dispatch a named field to the write or read helper depending on direction.
// All `twoWaySerializer` bodies use `j` (the cJSON node) and `p` (the object) as
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
// `toJsonValue` / `fromJsonValue` dispatchers above.
#define DEFINE_TWO_WAY_SERIALIZER(T) \
    template <bool Serialize>        \
    void twoWaySerializer(JsonNode<Serialize> j, isSameDecayed<T> auto&& p)


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
    FIELD(pendingTransformMs);
    FIELD(pendingTransformTargetType);
    FIELD(pendingTransformCatIdx);
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
DEFINE_TWO_WAY_SERIALIZER(Cat::WardenBonkState)
{
    FIELD(phase);
    FIELD(phaseMs);
    FIELD(pendingTargetIdx);
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
    FIELD(napBoostCountdown);
    FIELD(napBoostMultiplier);

    FIELD(wardenBonk);

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
DEFINE_TWO_WAY_SERIALIZER(Transition)
{
    FIELD(value);
    FIELD(reversed);
}


////////////////////////////////////////////////////////////
DEFINE_TWO_WAY_SERIALIZER(TimedCountdown)
{
    FIELD(time);
    FIELD(duration);
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

    FIELD(bsRimShineStrength);
    FIELD(bsRimShineFallRate);
    FIELD(bsRimShineTimeRate);
    FIELD(bsRimShineArc);

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
    FIELD(wardenGuardhouseBackOffset);
    FIELD(wardenGuardhouseFrontOffset);
    FIELD(wardenCatBodyOffset);
    FIELD(wardenCatPawOffset);
    FIELD(wardenCatBodyWobbleRadians);
    FIELD(wardenCatEyelidOriginOffset);
    FIELD(wardenCatYawnOriginOffset);
    FIELD(wardenCatPawScale);
    FIELD(wardenCatBatonWindupOffset);
    FIELD(wardenCatBatonWindupRotationDeg);
    FIELD(wardenCatBatonWindupMs);
    FIELD(wardenCatBatonTravelMs);
    FIELD(wardenCatBatonHoldMs);
    FIELD(wardenCatBatonReturnMs);
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

    FIELD(powerNapPurchased);
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
    FIELD(psvPPPowerNapDuration);
    FIELD(psvPPPowerNapStrength);

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
template <typename T>
[[nodiscard]] sf::base::String serializeToJsonString(const T& value, bool pretty)
{
    cJSON* const root = toJsonValue(value);
    SFML_BASE_SCOPE_GUARD({ cJSON_Delete(root); });

    char* const buf = pretty ? cJSON_Print(root) : cJSON_PrintUnformatted(root);
    if (buf == nullptr)
        return {};
    SFML_BASE_SCOPE_GUARD({ cJSON_free(buf); });

    return sf::base::String{buf};
}


////////////////////////////////////////////////////////////
bool forceCopyFile(const sf::Path& from, const sf::Path& to)
{
    (void)to.removeFromDisk();

    if (!from.copyFileTo(to))
    {
        sf::base::printLn("Failed to copy file from '{}' to '{}'", from, to);
        return false;
    }

    return true;
}


////////////////////////////////////////////////////////////
void doRotatingBackup(const sf::base::String& filename)
{
    (void)forceCopyFile(filename + ".bak2", filename + ".bak3");
    (void)forceCopyFile(filename + ".bak1", filename + ".bak2");
    (void)forceCopyFile(filename + ".bak0", filename + ".bak1");
    (void)forceCopyFile(filename, filename + ".bak0");
}

} // namespace

////////////////////////////////////////////////////////////
bool saveProfileToFile(const Profile& profile, const char* filename)
{
    if (!sf::Path{"userdata"}.createDirectoryTree())
    {
        sf::base::printLn("Failed to save profile to file '{}' (createDirectoryTree failed)", filename);
        return false;
    }

    doRotatingBackup(filename);

    if (!sf::writeToFile(sf::base::StringView{filename}, serializeToJsonString(profile, /* pretty */ false)))
    {
        sf::base::printLn("Failed to save profile to file '{}' (writeToFile failed)", filename);
        return false;
    }

    return true;
}


////////////////////////////////////////////////////////////
bool loadProfileFromFile(Profile& profile, const char* filename)
{
    const auto fail = [&](const char* reason)
    {
        sf::base::printLn("Failed to load profile from file '{}' ({})", filename, reason);
        return false;
    };

    sf::base::String contents;

    if (!sf::readFromFile(sf::base::StringView{filename}, contents))
        return fail("readFromFile failed");

    cJSON* const parsed = cJSON_Parse(contents.cStr());

    if (parsed == nullptr)
        return fail("cJSON_Parse failed");

    SFML_BASE_SCOPE_GUARD({ cJSON_Delete(parsed); });

    // Old saves used a JSON array at the root; new saves are objects.
    if (!cJSON_IsObject(parsed))
    {
        sf::base::printLn("Profile '{}' is in the legacy array format and cannot be loaded. Resetting to defaults.",
                          filename);

        profile = Profile{};
        return true;
    }

    fromJsonValue(parsed, profile);
    return true;
}

////////////////////////////////////////////////////////////
bool saveGameConstantsToFile(const GameConstants& gameConstants, const char* filename)
{
    const sf::Path path{filename};

    if (path.hasParent() && !path.getParent().createDirectoryTree())
    {
        sf::base::printLn("Failed to save game constants to file '{}' (createDirectoryTree failed)", filename);
        return false;
    }

    doRotatingBackup(filename);

    if (!sf::writeToFile(sf::base::StringView{filename}, serializeToJsonString(gameConstants, /* pretty */ true)))
    {
        sf::base::printLn("Failed to save game constants to file '{}' (writeToFile failed)", filename);
        return false;
    }

    return true;
}


////////////////////////////////////////////////////////////
bool loadGameConstantsFromFile(GameConstants& gameConstants, const char* filename)
{
    const auto fail = [&](const char* reason)
    {
        sf::base::printLn("Failed to load game constants from file '{}' ({})", filename, reason);
        return false;
    };

    sf::base::String contents;

    if (!sf::readFromFile(sf::base::StringView{filename}, contents))
        return fail("readFromFile failed");

    cJSON* const parsed = cJSON_Parse(contents.cStr());

    if (parsed == nullptr)
        return fail("cJSON_Parse failed");

    SFML_BASE_SCOPE_GUARD({ cJSON_Delete(parsed); });

    fromJsonValue(parsed, gameConstants);
    return true;
}


////////////////////////////////////////////////////////////
bool savePlaythroughToFile(const Playthrough& playthrough, const char* filename)
{
    if (!sf::Path{"userdata"}.createDirectoryTree())
    {
        sf::base::printLn("Failed to save playthrough to file '{}' (createDirectoryTree failed)", filename);
        return false;
    }

    doRotatingBackup(filename);

    if (!sf::writeToFile(sf::base::StringView{filename}, serializeToJsonString(playthrough, /* pretty */ false)))
    {
        sf::base::printLn("Failed to save playthrough to file '{}' (writeToFile failed)", filename);
        return false;
    }

    return true;
}


////////////////////////////////////////////////////////////
sf::base::StringView backwardsCompatibilityLoadChecks(const Version& parsedVersion, Playthrough& playthrough)
{
    if (parsedVersion == currentVersion)
        return "";

    sf::base::printLn("Loaded playthrough version {}.{}.{} does not match current version {}.{}.{}",
                      parsedVersion.major,
                      parsedVersion.minor,
                      parsedVersion.patch,
                      currentVersion.major,
                      currentVersion.minor,
                      currentVersion.patch);

    // Prestige point scaling buff compensation
    if (parsedVersion.major == 1 && parsedVersion.minor <= 4)
    {
        const auto loadedPrestigeLevel = static_cast<PrestigePointsType>(playthrough.psvBubbleValue.currentValue());

        if (loadedPrestigeLevel > 0u)
        {
            sf::base::printLn("Adding missing prestige points...");

            const auto oldAccumulatedPPs = Playthrough::calculatePrestigePointReward(0u,
                                                                                     loadedPrestigeLevel,
                                                                                     /* levelBias */ 0u);
            const auto newAccumulatedPPs = Playthrough::calculatePrestigePointReward(0u,
                                                                                     loadedPrestigeLevel,
                                                                                     /* levelBias */ 1u);

            sf::base::printLn("Old accumulated pps: {}{}New accumulated pps: {}{}Adding {} prestige points",
                              oldAccumulatedPPs,
                              '\n',
                              newAccumulatedPPs,
                              '\n',
                              newAccumulatedPPs - oldAccumulatedPPs);

            playthrough.prestigePoints += newAccumulatedPPs - oldAccumulatedPPs;

            return "Awarded compensation prestige points!";
        }
    }

    return "";
}


////////////////////////////////////////////////////////////
sf::base::StringView loadPlaythroughFromFile(Playthrough& playthrough, const char* filename)
{
    const auto fail = [&](const char* reason)
    {
        sf::base::printLn("Failed to load playthrough from file '{}' ({})", filename, reason);
        return "";
    };

    sf::base::String contents;

    if (!sf::readFromFile(sf::base::StringView{filename}, contents))
        return fail("readFromFile failed");

    cJSON* const parsed = cJSON_Parse(contents.cStr());

    if (parsed == nullptr)
        return fail("cJSON_Parse failed");

    SFML_BASE_SCOPE_GUARD({ cJSON_Delete(parsed); });

    // Old saves used a JSON array at the root; new saves are objects.
    if (!cJSON_IsObject(parsed))
    {
        sf::base::printLn("Playthrough '{}' is in the legacy array format and cannot be loaded.", filename);
        playthrough = Playthrough{};
        return "Your save is from an older version with an incompatible\n"
               "format and could not be loaded. A fresh playthrough has been started.";
    }

    fromJsonValue(parsed, playthrough);

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

    Version            parsedVersion{};
    const cJSON* const versionItem = cJSON_GetObjectItemCaseSensitive(parsed, "version");
    if (versionItem != nullptr)
        fromJsonValue(versionItem, parsedVersion);

    return backwardsCompatibilityLoadChecks(parsedVersion, playthrough);
}

// NOLINTEND(readability-identifier-naming, misc-use-internal-linkage)
