

#include "Achievements.hpp"
#include "Aliases.hpp"
#include "BubbleIdleMain.hpp"
#include "BubbleType.hpp"
#include "CatType.hpp"
#include "Countdown.hpp"
#include "ShrineConstants.hpp"

#include "ExampleUtils/Profiler.hpp"

#include "SFML/System/Clock.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Algorithm/Count.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/SizeT.hpp"

////////////////////////////////////////////////////////////
void Main::gameLoopUpdateMilestones()
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    const auto updateMilestone = [&](const char* name, sf::base::U64& milestone)
    {
        const auto oldMilestone = milestone;

        milestone = sf::base::min(milestone, pt->statsTotal.secondsPlayed);

        if (milestone != oldMilestone)
        {
            const auto [h, m, s] = formatTime(milestone);
            pushNotification("Milestone reached!", "'%s' at %lluh %llum %llus", name, h, m, s);
        }
    };

    const auto nCatNormal = pt->getCatCountByType(CatType::Normal);
    const auto nCatUni    = pt->getCatCountByType(CatType::Uni);
    const auto nCatDevil  = pt->getCatCountByType(CatType::Devil);
    const auto nCatAstro  = pt->getCatCountByType(CatType::Astro);

    if (nCatNormal >= 1)
        updateMilestone("1st Cat", pt->milestones.firstCat);

    if (nCatUni >= 1)
        updateMilestone("1st Unicat", pt->milestones.firstUnicat);

    if (nCatDevil >= 1)
        updateMilestone("1st Devilcat", pt->milestones.firstDevilcat);

    if (nCatAstro >= 1)
        updateMilestone("1st Astrocat", pt->milestones.firstAstrocat);

    if (nCatNormal >= 5)
        updateMilestone("5th Cat", pt->milestones.fiveCats);

    if (nCatUni >= 5)
        updateMilestone("5th Unicat", pt->milestones.fiveUnicats);

    if (nCatDevil >= 5)
        updateMilestone("5th Devilcat", pt->milestones.fiveDevilcats);

    if (nCatAstro >= 5)
        updateMilestone("5th Astrocat", pt->milestones.fiveAstrocats);

    if (nCatNormal >= 10)
        updateMilestone("10th Cat", pt->milestones.tenCats);

    if (nCatUni >= 10)
        updateMilestone("10th Unicat", pt->milestones.tenUnicats);

    if (nCatDevil >= 10)
        updateMilestone("10th Devilcat", pt->milestones.tenDevilcats);

    if (nCatAstro >= 10)
        updateMilestone("10th Astrocat", pt->milestones.tenAstrocats);

    if (pt->psvBubbleValue.nPurchases >= 1)
        updateMilestone("Prestige Level 2", pt->milestones.prestigeLevel2);

    if (pt->psvBubbleValue.nPurchases >= 2)
        updateMilestone("Prestige Level 3", pt->milestones.prestigeLevel3);

    if (pt->psvBubbleValue.nPurchases >= 3)
        updateMilestone("Prestige Level 4", pt->milestones.prestigeLevel4);

    if (pt->psvBubbleValue.nPurchases >= 4)
        updateMilestone("Prestige Level 5", pt->milestones.prestigeLevel5);

    if (pt->psvBubbleValue.nPurchases >= 5)
        updateMilestone("Prestige Level 6", pt->milestones.prestigeLevel6);

    if (pt->psvBubbleValue.nPurchases >= 9)
        updateMilestone("Prestige Level 10", pt->milestones.prestigeLevel10);

    if (pt->psvBubbleValue.nPurchases >= 14)
        updateMilestone("Prestige Level 15", pt->milestones.prestigeLevel15);

    if (pt->psvBubbleValue.nPurchases >= 19)
        updateMilestone("Prestige Level 20 (MAX)", pt->milestones.prestigeLevel20);

    const auto totalRevenue = pt->statsTotal.getTotalRevenue();

    if (totalRevenue >= 10'000)
        updateMilestone("$10.000 Revenue", pt->milestones.revenue10000);

    if (totalRevenue >= 100'000)
        updateMilestone("$100.000 Revenue", pt->milestones.revenue100000);

    if (totalRevenue >= 1'000'000)
        updateMilestone("$1.000.000 Revenue", pt->milestones.revenue1000000);

    if (totalRevenue >= 10'000'000)
        updateMilestone("$10.000.000 Revenue", pt->milestones.revenue10000000);

    if (totalRevenue >= 100'000'000)
        updateMilestone("$100.000.000 Revenue", pt->milestones.revenue100000000);

    if (totalRevenue >= 1'000'000'000)
        updateMilestone("$1.000.000.000 Revenue", pt->milestones.revenue1000000000);

    for (SizeT i = 0u; i < pt->nShrinesCompleted; ++i)
    {
        const char* shrineName = i >= pt->getMapLimitIncreases() ? "Shrine Of ???" : shrineNames[i];
        updateMilestone(shrineName, pt->milestones.shrineCompletions[i]);
    }
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateSplits()
{
    if (!inSpeedrunPlaythrough() || !pt->speedrunStartTime.hasValue())
        return;

    const auto updateSplit = [&](const char* name, sf::base::U64& split)
    {
        if (split == 0u)
            return;

        const auto oldSplit    = split;
        const auto splitTimeUs = (sf::Clock::now() - pt->speedrunStartTime.value()).asMicroseconds();

        split = sf::base::min(split, static_cast<sf::base::U64>(splitTimeUs));

        if (split != oldSplit)
        {
            const auto [hours, mins, secs, millis] = formatSpeedrunTime(sf::microseconds(splitTimeUs));
            pushNotification("Split reached!", "'%s' at %02llu:%02llu:%02llu:%03llu", name, hours, mins, secs, millis);
        }
    };

    if (pt->psvBubbleValue.nPurchases >= 1)
        updateSplit("Prestige Lv.2", pt->speedrunSplits.prestigeLevel2);

    if (pt->psvBubbleValue.nPurchases >= 2)
        updateSplit("Prestige Lv.3", pt->speedrunSplits.prestigeLevel3);

    if (pt->psvBubbleValue.nPurchases >= 3)
        updateSplit("Prestige Lv.4", pt->speedrunSplits.prestigeLevel4);

    if (pt->psvBubbleValue.nPurchases >= 4)
        updateSplit("Prestige Lv.5", pt->speedrunSplits.prestigeLevel5);
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateAchievements()
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    [[maybe_unused]] static bool mustGetFromSteam = true; // sync achievements from Steam only once

    SizeT nextId = 0u;

    const auto unlockIf = [&](const bool condition)
    {
        const auto achievementId = nextId++;

#if defined(BUBBLEBYTE_USE_STEAMWORKS) && !defined(BUBBLEBYTE_DEMO)
        if (steamMgr.isInitialized())
        {
            if (condition)
                steamMgr.unlockAchievement(achievementId);

            if (!profile.unlockedAchievements[achievementId] && mustGetFromSteam)
            {
                if (steamMgr.isAchievementUnlocked(achievementId))
                    profile.unlockedAchievements[achievementId] = true;
            }
        }
#endif

        if (profile.unlockedAchievements[achievementId] || !condition)
            return;

        profile.unlockedAchievements[achievementId] = true;

        pushNotification("Achievement unlocked!",
                         "\"%s\"\n- %s",
                         achievementData[achievementId].name,
                         achievementData[achievementId].description);
    };

    const auto unlockIfPrestige = [&](const bool condition)
    {
        if (undoPPPurchaseTimer.value > 0.f) // don't unlock in undo grace period
        {
            ++nextId;
            return;
        }

        unlockIf(condition);
    };

    const auto unlockIfGtEq = [&](const auto& value, const auto& threshold)
    {
        SFML_BASE_ASSERT(value >= 0);
        SFML_BASE_ASSERT(threshold >= 0);

        unlockIf(static_cast<sf::base::SizeT>(value) >= static_cast<sf::base::SizeT>(threshold));

        achievementProgress[nextId - 1u].emplace(static_cast<sf::base::SizeT>(value),
                                                 static_cast<sf::base::SizeT>(threshold));
    };

    const auto unlockIfGtEqPrestige = [&](const auto& value, const auto& threshold)
    {
        if (undoPPPurchaseTimer.value > 0.f) // don't unlock in undo grace period
        {
            ++nextId;
            return;
        }

        unlockIfGtEq(value, threshold);
    };

    const auto bubblesHandPopped = profile.statsLifetime.getTotalNBubblesHandPopped();
    const auto bubblesCatPopped  = profile.statsLifetime.getTotalNBubblesCatPopped();

    unlockIfGtEq(bubblesHandPopped, 1);
    unlockIfGtEq(bubblesHandPopped, 10);
    unlockIfGtEq(bubblesHandPopped, 100);
    unlockIfGtEq(bubblesHandPopped, 1000);
    unlockIfGtEq(bubblesHandPopped, 10'000);
    unlockIfGtEq(bubblesHandPopped, 100'000);
    unlockIfGtEq(bubblesHandPopped, 1'000'000);

    unlockIfGtEq(bubblesCatPopped, 1);
    unlockIfGtEq(bubblesCatPopped, 100);
    unlockIfGtEq(bubblesCatPopped, 1000);
    unlockIfGtEq(bubblesCatPopped, 10'000);
    unlockIfGtEq(bubblesCatPopped, 100'000);
    unlockIfGtEq(bubblesCatPopped, 1'000'000);
    unlockIfGtEq(bubblesCatPopped, 10'000'000);
    unlockIfGtEq(bubblesCatPopped, 100'000'000);

    unlockIf(pt->comboPurchased);

    unlockIfGtEq(pt->psvComboStartTime.nPurchases, 5);
    unlockIfGtEq(pt->psvComboStartTime.nPurchases, 10);
    unlockIfGtEq(pt->psvComboStartTime.nPurchases, 15);
    unlockIfGtEq(pt->psvComboStartTime.nPurchases, 20);

    unlockIf(pt->mapPurchased); //
    unlockIfGtEq(pt->psvMapExtension.nPurchases, 1);
    unlockIfGtEq(pt->psvMapExtension.nPurchases, 3);
    unlockIfGtEq(pt->psvMapExtension.nPurchases, 5);
    unlockIfGtEq(pt->psvMapExtension.nPurchases, 7);

    unlockIfGtEq(pt->psvBubbleCount.nPurchases, 1);
    unlockIfGtEq(pt->psvBubbleCount.nPurchases, 5);
    unlockIfGtEq(pt->psvBubbleCount.nPurchases, 10);
    unlockIfGtEq(pt->psvBubbleCount.nPurchases, 20);
    unlockIfGtEq(pt->psvBubbleCount.nPurchases, 30);

    unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Normal)].nPurchases, 1);
    unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Normal)].nPurchases, 5);
    unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Normal)].nPurchases, 10);
    unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Normal)].nPurchases, 20);
    unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Normal)].nPurchases, 30);
    unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Normal)].nPurchases, 40);

    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Normal)].nPurchases, 1);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Normal)].nPurchases, 3);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Normal)].nPurchases, 6);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Normal)].nPurchases, 9);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Normal)].nPurchases, 12);

    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Normal)].nPurchases, 1);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Normal)].nPurchases, 3);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Normal)].nPurchases, 6);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Normal)].nPurchases, 9);

    unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Uni)].nPurchases, 1);
    unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Uni)].nPurchases, 5);
    unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Uni)].nPurchases, 10);
    unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Uni)].nPurchases, 20);
    unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Uni)].nPurchases, 30);
    unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Uni)].nPurchases, 40);

    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Uni)].nPurchases, 1);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Uni)].nPurchases, 3);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Uni)].nPurchases, 6);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Uni)].nPurchases, 9);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Uni)].nPurchases, 12);

    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Uni)].nPurchases, 1);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Uni)].nPurchases, 3);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Uni)].nPurchases, 6);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Uni)].nPurchases, 9);

    unlockIfPrestige(pt->perm.unicatTranscendencePurchased);
    unlockIfPrestige(pt->perm.unicatTranscendenceAOEPurchased);

    unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Devil)].nPurchases, 1);
    unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Devil)].nPurchases, 5);
    unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Devil)].nPurchases, 10);
    unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Devil)].nPurchases, 20);
    unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Devil)].nPurchases, 30);
    unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Devil)].nPurchases, 40);

    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Devil)].nPurchases, 1);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Devil)].nPurchases, 3);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Devil)].nPurchases, 6);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Devil)].nPurchases, 9);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Devil)].nPurchases, 12);

    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Devil)].nPurchases, 1);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Devil)].nPurchases, 3);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Devil)].nPurchases, 6);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Devil)].nPurchases, 9);

    unlockIfGtEq(pt->psvExplosionRadiusMult.nPurchases, 1);
    unlockIfGtEq(pt->psvExplosionRadiusMult.nPurchases, 5);
    unlockIfGtEq(pt->psvExplosionRadiusMult.nPurchases, 10);

    unlockIfPrestige(pt->perm.devilcatHellsingedPurchased);

    unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Astro)].nPurchases, 1);
    unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Astro)].nPurchases, 5);
    unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Astro)].nPurchases, 10);
    unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Astro)].nPurchases, 20);
    unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Astro)].nPurchases, 25);
    unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Astro)].nPurchases, 30);

    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Astro)].nPurchases, 1);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Astro)].nPurchases, 3);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Astro)].nPurchases, 6);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Astro)].nPurchases, 9);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Astro)].nPurchases, 12);

    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Astro)].nPurchases, 1);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Astro)].nPurchases, 3);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Astro)].nPurchases, 6);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Astro)].nPurchases, 9);

    unlockIfGtEq(pt->psvBubbleValue.nPurchases, 1);
    unlockIfGtEq(pt->psvBubbleValue.nPurchases, 2);
    unlockIfGtEq(pt->psvBubbleValue.nPurchases, 3);
    unlockIfGtEq(pt->psvBubbleValue.nPurchases, 5);
    unlockIfGtEq(pt->psvBubbleValue.nPurchases, 10);
    unlockIfGtEq(pt->psvBubbleValue.nPurchases, 15);
    unlockIfGtEq(pt->psvBubbleValue.nPurchases, 19);

    unlockIfPrestige(pt->perm.starterPackPurchased);

    unlockIfPrestige(pt->perm.multiPopPurchased);
    unlockIfGtEqPrestige(pt->psvPPMultiPopRange.nPurchases, 1);
    unlockIfGtEqPrestige(pt->psvPPMultiPopRange.nPurchases, 2);
    unlockIfGtEqPrestige(pt->psvPPMultiPopRange.nPurchases, 5);
    unlockIfGtEqPrestige(pt->psvPPMultiPopRange.nPurchases, 10);

    unlockIfPrestige(pt->perm.windPurchased);

    unlockIfPrestige(pt->perm.smartCatsPurchased);
    unlockIfPrestige(pt->perm.geniusCatsPurchased);

    unlockIfPrestige(pt->perm.astroCatInspirePurchased);
    unlockIfGtEqPrestige(pt->psvPPInspireDurationMult.nPurchases, 1);
    unlockIfGtEqPrestige(pt->psvPPInspireDurationMult.nPurchases, 4);
    unlockIfGtEqPrestige(pt->psvPPInspireDurationMult.nPurchases, 8);
    unlockIfGtEqPrestige(pt->psvPPInspireDurationMult.nPurchases, 12);
    unlockIfGtEqPrestige(pt->psvPPInspireDurationMult.nPurchases, 16);

    unlockIfGtEq(comboState.combo, 5);
    unlockIfGtEq(comboState.combo, 10);
    unlockIfGtEq(comboState.combo, 15);
    unlockIfGtEq(comboState.combo, 20);
    unlockIfGtEq(comboState.combo, 25);

    unlockIfGtEq(profile.statsLifetime.highestStarBubblePopCombo, 5);
    unlockIfGtEq(profile.statsLifetime.highestStarBubblePopCombo, 10);
    unlockIfGtEq(profile.statsLifetime.highestStarBubblePopCombo, 15);
    unlockIfGtEq(profile.statsLifetime.highestStarBubblePopCombo, 20);
    unlockIfGtEq(profile.statsLifetime.highestStarBubblePopCombo, 25);

    const auto nStarBubblesPoppedByHand = profile.statsLifetime.getNBubblesHandPopped(BubbleType::Star);
    const auto nStarBubblesPoppedByCat  = profile.statsLifetime.getNBubblesCatPopped(BubbleType::Star);

    unlockIfGtEq(nStarBubblesPoppedByHand, 1);
    unlockIfGtEq(nStarBubblesPoppedByHand, 100);
    unlockIfGtEq(nStarBubblesPoppedByHand, 1000);
    unlockIfGtEq(nStarBubblesPoppedByHand, 10'000);
    unlockIfGtEq(nStarBubblesPoppedByHand, 100'000);

    unlockIfGtEq(nStarBubblesPoppedByCat, 1);
    unlockIfGtEq(nStarBubblesPoppedByCat, 100);
    unlockIfGtEq(nStarBubblesPoppedByCat, 1000);
    unlockIfGtEq(nStarBubblesPoppedByCat, 10'000);
    unlockIfGtEq(nStarBubblesPoppedByCat, 100'000);
    unlockIfGtEq(nStarBubblesPoppedByCat, 1'000'000);
    unlockIfGtEq(nStarBubblesPoppedByCat, 10'000'000);

    unlockIfGtEq(profile.statsLifetime.highestNovaBubblePopCombo, 5);
    unlockIfGtEq(profile.statsLifetime.highestNovaBubblePopCombo, 10);
    unlockIfGtEq(profile.statsLifetime.highestNovaBubblePopCombo, 15);
    unlockIfGtEq(profile.statsLifetime.highestNovaBubblePopCombo, 20);
    unlockIfGtEq(profile.statsLifetime.highestNovaBubblePopCombo, 25);

    const auto nNovaBubblesPoppedByHand = profile.statsLifetime.getNBubblesHandPopped(BubbleType::Nova);
    const auto nNovaBubblesPoppedByCat  = profile.statsLifetime.getNBubblesCatPopped(BubbleType::Nova);

    unlockIfGtEq(nNovaBubblesPoppedByHand, 1);
    unlockIfGtEq(nNovaBubblesPoppedByHand, 100);
    unlockIfGtEq(nNovaBubblesPoppedByHand, 1000);
    unlockIfGtEq(nNovaBubblesPoppedByHand, 10'000);
    unlockIfGtEq(nNovaBubblesPoppedByHand, 100'000);

    unlockIfGtEq(nNovaBubblesPoppedByCat, 1);
    unlockIfGtEq(nNovaBubblesPoppedByCat, 100);
    unlockIfGtEq(nNovaBubblesPoppedByCat, 1000);
    unlockIfGtEq(nNovaBubblesPoppedByCat, 10'000);
    unlockIfGtEq(nNovaBubblesPoppedByCat, 100'000);
    unlockIfGtEq(nNovaBubblesPoppedByCat, 1'000'000);
    unlockIfGtEq(nNovaBubblesPoppedByCat, 10'000'000);

    const auto nBombBubblesPoppedByHand = profile.statsLifetime.getNBubblesHandPopped(BubbleType::Bomb);
    const auto nBombBubblesPoppedByCat  = profile.statsLifetime.getNBubblesCatPopped(BubbleType::Bomb);

    unlockIfGtEq(nBombBubblesPoppedByHand, 1);
    unlockIfGtEq(nBombBubblesPoppedByHand, 100);
    unlockIfGtEq(nBombBubblesPoppedByHand, 1000);
    unlockIfGtEq(nBombBubblesPoppedByHand, 10'000);

    unlockIfGtEq(nBombBubblesPoppedByCat, 1);
    unlockIfGtEq(nBombBubblesPoppedByCat, 100);
    unlockIfGtEq(nBombBubblesPoppedByCat, 1000);
    unlockIfGtEq(nBombBubblesPoppedByCat, 10'000);
    unlockIfGtEq(nBombBubblesPoppedByCat, 100'000);

    unlockIf(pt->achAstrocatPopBomb);

    unlockIf(pt->achAstrocatInspireByType[asIdx(CatType::Normal)]);
    unlockIf(pt->achAstrocatInspireByType[asIdx(CatType::Uni)]);
    unlockIf(pt->achAstrocatInspireByType[asIdx(CatType::Devil)]);
    unlockIf(pt->achAstrocatInspireByType[asIdx(CatType::Witch)]);
    unlockIf(pt->achAstrocatInspireByType[asIdx(CatType::Wizard)]);
    unlockIf(pt->achAstrocatInspireByType[asIdx(CatType::Mouse)]);
    unlockIf(pt->achAstrocatInspireByType[asIdx(CatType::Engi)]);
    unlockIf(pt->achAstrocatInspireByType[asIdx(CatType::Repulso)]);
    unlockIf(pt->achAstrocatInspireByType[asIdx(CatType::Attracto)]);
    unlockIf(pt->achAstrocatInspireByType[asIdx(CatType::Copy)]);

    unlockIfGtEq(pt->psvShrineActivation.nPurchases, 1);
    unlockIfGtEq(pt->psvShrineActivation.nPurchases, 2);
    unlockIfGtEq(pt->psvShrineActivation.nPurchases, 3);
    unlockIfGtEq(pt->psvShrineActivation.nPurchases, 4);
    unlockIfGtEq(pt->psvShrineActivation.nPurchases, 5);
    unlockIfGtEq(pt->psvShrineActivation.nPurchases, 6);
    unlockIfGtEq(pt->psvShrineActivation.nPurchases, 7);
    unlockIfGtEq(pt->psvShrineActivation.nPurchases, 8);

    unlockIfGtEq(pt->nShrinesCompleted, 1);
    unlockIfGtEq(pt->nShrinesCompleted, 2);
    unlockIfGtEq(pt->nShrinesCompleted, 3);
    unlockIfGtEq(pt->nShrinesCompleted, 4);
    unlockIfGtEq(pt->nShrinesCompleted, 5);
    unlockIfGtEq(pt->nShrinesCompleted, 6);
    unlockIfGtEq(pt->nShrinesCompleted, 7);
    unlockIfGtEq(pt->nShrinesCompleted, 8);

    unlockIfPrestige(pt->perm.unsealedByType[asIdx(CatType::Witch)]);
    unlockIfPrestige(pt->perm.unsealedByType[asIdx(CatType::Wizard)]);
    unlockIfPrestige(pt->perm.unsealedByType[asIdx(CatType::Mouse)]);
    unlockIfPrestige(pt->perm.unsealedByType[asIdx(CatType::Engi)]);
    unlockIfPrestige(pt->perm.unsealedByType[asIdx(CatType::Repulso)]);
    unlockIfPrestige(pt->perm.unsealedByType[asIdx(CatType::Attracto)]);
    unlockIfPrestige(pt->perm.unsealedByType[asIdx(CatType::Copy)]);

    unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Normal)], 1);
    unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Uni)], 1);
    unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Devil)], 1);
    unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Astro)], 1);
    unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Wizard)], 1);
    unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Mouse)], 1);
    unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Engi)], 1);
    unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Repulso)], 1);
    unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Attracto)], 1);
    unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Copy)], 1);

    unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Normal)], 500);
    unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Uni)], 100);
    unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Devil)], 100);
    unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Astro)], 50);
    unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Wizard)], 10);
    unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Mouse)], 10);
    unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Engi)], 10);
    unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Repulso)], 10);
    unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Attracto)], 10);
    unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Copy)], 10);

    unlockIfGtEq(profile.statsLifetime.nWitchcatDollsCollected, 1);
    unlockIfGtEq(profile.statsLifetime.nWitchcatDollsCollected, 10);
    unlockIfGtEq(profile.statsLifetime.nWitchcatDollsCollected, 100);
    unlockIfGtEq(profile.statsLifetime.nWitchcatDollsCollected, 1000);
    unlockIfGtEq(profile.statsLifetime.nWitchcatDollsCollected, 10'000);

    unlockIfGtEqPrestige(pt->psvPPWitchCatBuffDuration.nPurchases, 1);
    unlockIfGtEqPrestige(pt->psvPPWitchCatBuffDuration.nPurchases, 3);
    unlockIfGtEqPrestige(pt->psvPPWitchCatBuffDuration.nPurchases, 6);
    unlockIfGtEqPrestige(pt->psvPPWitchCatBuffDuration.nPurchases, 9);
    unlockIfGtEqPrestige(pt->psvPPWitchCatBuffDuration.nPurchases, 12);

    unlockIfPrestige(pt->perm.witchCatBuffPowerScalesWithNCats);
    unlockIfPrestige(pt->perm.witchCatBuffPowerScalesWithMapSize);
    unlockIfPrestige(pt->perm.witchCatBuffFewerDolls);
    unlockIfPrestige(pt->perm.witchCatBuffFlammableDolls);
    unlockIfPrestige(pt->perm.witchCatBuffOrbitalDolls);

    unlockIfGtEqPrestige(pt->psvPPUniRitualBuffPercentage.nPurchases, 1);
    unlockIfGtEqPrestige(pt->psvPPUniRitualBuffPercentage.nPurchases, 6);
    unlockIfGtEqPrestige(pt->psvPPUniRitualBuffPercentage.nPurchases, 12);
    unlockIfGtEqPrestige(pt->psvPPUniRitualBuffPercentage.nPurchases, 18);
    unlockIfGtEqPrestige(pt->psvPPUniRitualBuffPercentage.nPurchases, 24);

    unlockIfGtEqPrestige(pt->psvPPDevilRitualBuffPercentage.nPurchases, 1);
    unlockIfGtEqPrestige(pt->psvPPDevilRitualBuffPercentage.nPurchases, 6);
    unlockIfGtEqPrestige(pt->psvPPDevilRitualBuffPercentage.nPurchases, 12);
    unlockIfGtEqPrestige(pt->psvPPDevilRitualBuffPercentage.nPurchases, 18);
    unlockIfGtEqPrestige(pt->psvPPDevilRitualBuffPercentage.nPurchases, 24);

    const auto nActiveBuffs = sf::base::countIf(pt->buffCountdownsPerType,
                                                pt->buffCountdownsPerType + nCatTypes,
                                                [](const Countdown& c) { return c.value > 0.f; });

    unlockIfGtEq(nActiveBuffs, 2);
    unlockIfGtEq(nActiveBuffs, 3);
    unlockIfGtEq(nActiveBuffs, 4);
    unlockIfGtEq(nActiveBuffs, 5);

    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Witch)].nPurchases, 1);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Witch)].nPurchases, 3);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Witch)].nPurchases, 6);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Witch)].nPurchases, 9);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Witch)].nPurchases, 12);

    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Witch)].nPurchases, 1);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Witch)].nPurchases, 3);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Witch)].nPurchases, 6);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Witch)].nPurchases, 9);

    unlockIfGtEq(profile.statsLifetime.nAbsorbedStarBubbles, 1);
    unlockIfGtEq(profile.statsLifetime.nAbsorbedStarBubbles, 100);
    unlockIfGtEq(profile.statsLifetime.nAbsorbedStarBubbles, 1000);
    unlockIfGtEq(profile.statsLifetime.nAbsorbedStarBubbles, 10'000);
    unlockIfGtEq(profile.statsLifetime.nAbsorbedStarBubbles, 100'000);

    unlockIfGtEq(pt->psvSpellCount.nPurchases, 1);
    unlockIfGtEq(pt->psvSpellCount.nPurchases, 2);
    unlockIfGtEq(pt->psvSpellCount.nPurchases, 3);
    unlockIfGtEq(pt->psvSpellCount.nPurchases, 4);

    unlockIfGtEq(pt->psvStarpawPercentage.nPurchases, 1);
    unlockIfGtEq(pt->psvStarpawPercentage.nPurchases, 4);
    unlockIfGtEq(pt->psvStarpawPercentage.nPurchases, 8);

    unlockIfGtEq(pt->psvMewltiplierMult.nPurchases, 1);
    unlockIfGtEq(pt->psvMewltiplierMult.nPurchases, 5);
    unlockIfGtEq(pt->psvMewltiplierMult.nPurchases, 10);
    unlockIfGtEq(pt->psvMewltiplierMult.nPurchases, 15);

    unlockIfGtEq(pt->psvDarkUnionPercentage.nPurchases, 1);
    unlockIfGtEq(pt->psvDarkUnionPercentage.nPurchases, 4);
    unlockIfGtEq(pt->psvDarkUnionPercentage.nPurchases, 8);

    unlockIfGtEq(profile.statsLifetime.nSpellCasts[0], 1);
    unlockIfGtEq(profile.statsLifetime.nSpellCasts[0], 10);
    unlockIfGtEq(profile.statsLifetime.nSpellCasts[0], 100);
    unlockIfGtEq(profile.statsLifetime.nSpellCasts[0], 1000);

    unlockIfGtEq(profile.statsLifetime.nSpellCasts[1], 1);
    unlockIfGtEq(profile.statsLifetime.nSpellCasts[1], 10);
    unlockIfGtEq(profile.statsLifetime.nSpellCasts[1], 100);
    unlockIfGtEq(profile.statsLifetime.nSpellCasts[1], 1000);

    unlockIfGtEq(profile.statsLifetime.nSpellCasts[2], 1);
    unlockIfGtEq(profile.statsLifetime.nSpellCasts[2], 10);
    unlockIfGtEq(profile.statsLifetime.nSpellCasts[2], 100);
    unlockIfGtEq(profile.statsLifetime.nSpellCasts[2], 1000);

    unlockIfGtEq(profile.statsLifetime.nSpellCasts[3], 1);
    unlockIfGtEq(profile.statsLifetime.nSpellCasts[3], 10);
    unlockIfGtEq(profile.statsLifetime.nSpellCasts[3], 100);
    unlockIfGtEq(profile.statsLifetime.nSpellCasts[3], 1000);

    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Wizard)].nPurchases, 1);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Wizard)].nPurchases, 3);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Wizard)].nPurchases, 6);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Wizard)].nPurchases, 9);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Wizard)].nPurchases, 12);

    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Wizard)].nPurchases, 1);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Wizard)].nPurchases, 3);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Wizard)].nPurchases, 6);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Wizard)].nPurchases, 9);

    unlockIfGtEqPrestige(pt->psvPPManaCooldownMult.nPurchases, 1);
    unlockIfGtEqPrestige(pt->psvPPManaCooldownMult.nPurchases, 4);
    unlockIfGtEqPrestige(pt->psvPPManaCooldownMult.nPurchases, 8);
    unlockIfGtEqPrestige(pt->psvPPManaCooldownMult.nPurchases, 12);
    unlockIfGtEqPrestige(pt->psvPPManaCooldownMult.nPurchases, 16);

    unlockIfGtEqPrestige(pt->psvPPManaMaxMult.nPurchases, 1);
    unlockIfGtEqPrestige(pt->psvPPManaMaxMult.nPurchases, 4);
    unlockIfGtEqPrestige(pt->psvPPManaMaxMult.nPurchases, 8);
    unlockIfGtEqPrestige(pt->psvPPManaMaxMult.nPurchases, 12);
    unlockIfGtEqPrestige(pt->psvPPManaMaxMult.nPurchases, 16);
    unlockIfGtEqPrestige(pt->psvPPManaMaxMult.nPurchases, 20);

    unlockIfPrestige(pt->perm.starpawConversionIgnoreBombs);
    unlockIfPrestige(pt->perm.starpawNova);
    unlockIfPrestige(pt->perm.wizardCatDoubleMewltiplierDuration);
    unlockIfPrestige(pt->perm.wizardCatDoubleStasisFieldDuration);

    unlockIfGtEq(pt->mouseCatCombo, 25);
    unlockIfGtEq(pt->mouseCatCombo, 50);
    unlockIfGtEq(pt->mouseCatCombo, 75);
    unlockIfGtEq(pt->mouseCatCombo, 100);
    unlockIfGtEq(pt->mouseCatCombo, 125);
    unlockIfGtEq(pt->mouseCatCombo, 150);
    unlockIfGtEq(pt->mouseCatCombo, 175);
    unlockIfGtEq(pt->mouseCatCombo, 999);

    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Mouse)].nPurchases, 1);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Mouse)].nPurchases, 3);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Mouse)].nPurchases, 6);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Mouse)].nPurchases, 9);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Mouse)].nPurchases, 12);

    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Mouse)].nPurchases, 1);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Mouse)].nPurchases, 3);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Mouse)].nPurchases, 6);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Mouse)].nPurchases, 9);

    unlockIfGtEqPrestige(pt->psvPPMouseCatGlobalBonusMult.nPurchases, 1);
    unlockIfGtEqPrestige(pt->psvPPMouseCatGlobalBonusMult.nPurchases, 2);
    unlockIfGtEqPrestige(pt->psvPPMouseCatGlobalBonusMult.nPurchases, 6);
    unlockIfGtEqPrestige(pt->psvPPMouseCatGlobalBonusMult.nPurchases, 10);
    unlockIfGtEqPrestige(pt->psvPPMouseCatGlobalBonusMult.nPurchases, 14);

    unlockIfGtEq(profile.statsLifetime.nMaintenances, 1);
    unlockIfGtEq(profile.statsLifetime.nMaintenances, 10);
    unlockIfGtEq(profile.statsLifetime.nMaintenances, 100);
    unlockIfGtEq(profile.statsLifetime.nMaintenances, 1000);
    unlockIfGtEq(profile.statsLifetime.nMaintenances, 10'000);
    unlockIfGtEq(profile.statsLifetime.nMaintenances, 100'000);
    unlockIfGtEq(profile.statsLifetime.nMaintenances, 1'000'000);

    unlockIfGtEq(profile.statsLifetime.highestSimultaneousMaintenances, 3);
    unlockIfGtEq(profile.statsLifetime.highestSimultaneousMaintenances, 6);
    unlockIfGtEq(profile.statsLifetime.highestSimultaneousMaintenances, 9);
    unlockIfGtEq(profile.statsLifetime.highestSimultaneousMaintenances, 12);
    unlockIfGtEq(profile.statsLifetime.highestSimultaneousMaintenances, 15);

    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Engi)].nPurchases, 1);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Engi)].nPurchases, 3);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Engi)].nPurchases, 6);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Engi)].nPurchases, 9);
    unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Engi)].nPurchases, 12);

    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Engi)].nPurchases, 1);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Engi)].nPurchases, 3);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Engi)].nPurchases, 6);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Engi)].nPurchases, 9);

    unlockIfGtEqPrestige(pt->psvPPEngiCatGlobalBonusMult.nPurchases, 1);
    unlockIfGtEqPrestige(pt->psvPPEngiCatGlobalBonusMult.nPurchases, 2);
    unlockIfGtEqPrestige(pt->psvPPEngiCatGlobalBonusMult.nPurchases, 6);
    unlockIfGtEqPrestige(pt->psvPPEngiCatGlobalBonusMult.nPurchases, 10);
    unlockIfGtEqPrestige(pt->psvPPEngiCatGlobalBonusMult.nPurchases, 14);

    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Repulso)].nPurchases, 1);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Repulso)].nPurchases, 3);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Repulso)].nPurchases, 6);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Repulso)].nPurchases, 9);

    unlockIfPrestige(pt->perm.repulsoCatFilterPurchased);
    unlockIfPrestige(pt->perm.repulsoCatConverterPurchased);
    unlockIfPrestige(pt->perm.repulsoCatNovaConverterPurchased);

    unlockIfGtEqPrestige(pt->psvPPRepulsoCatConverterChance.nPurchases, 1);
    unlockIfGtEqPrestige(pt->psvPPRepulsoCatConverterChance.nPurchases, 4);
    unlockIfGtEqPrestige(pt->psvPPRepulsoCatConverterChance.nPurchases, 8);
    unlockIfGtEqPrestige(pt->psvPPRepulsoCatConverterChance.nPurchases, 12);
    unlockIfGtEqPrestige(pt->psvPPRepulsoCatConverterChance.nPurchases, 16);

    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Attracto)].nPurchases, 1);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Attracto)].nPurchases, 3);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Attracto)].nPurchases, 6);
    unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Attracto)].nPurchases, 9);

    unlockIfPrestige(pt->perm.attractoCatFilterPurchased);

    unlockIfGtEq(profile.statsLifetime.nDisguises, 1);
    unlockIfGtEq(profile.statsLifetime.nDisguises, 5);
    unlockIfGtEq(profile.statsLifetime.nDisguises, 25);
    unlockIfGtEq(profile.statsLifetime.nDisguises, 100);

    unlockIf(buyReminder >= 5); // Secret
    unlockIf(pt->geniusCatIgnoreBubbles.normal && pt->geniusCatIgnoreBubbles.star && pt->geniusCatIgnoreBubbles.bomb); // Secret
    unlockIf(wastedEffort);

    const auto minutesToMicroseconds = [](const sf::base::I64 nMinutes) -> sf::base::I64
    { return nMinutes * 60 * 1'000'000; };

    const bool inSpeedrunMode = inSpeedrunPlaythrough();

    unlockIf(inSpeedrunMode);

    unlockIf(inSpeedrunMode && pt->speedrunSplits.prestigeLevel2 <= minutesToMicroseconds(9));
    unlockIf(inSpeedrunMode && pt->speedrunSplits.prestigeLevel2 <= minutesToMicroseconds(7));
    unlockIf(inSpeedrunMode && pt->speedrunSplits.prestigeLevel2 <= minutesToMicroseconds(5));
    unlockIf(inSpeedrunMode && pt->speedrunSplits.prestigeLevel2 <= minutesToMicroseconds(4));

    unlockIf(inSpeedrunMode && pt->speedrunSplits.prestigeLevel3 <= minutesToMicroseconds(30));
    unlockIf(inSpeedrunMode && pt->speedrunSplits.prestigeLevel3 <= minutesToMicroseconds(26));
    unlockIf(inSpeedrunMode && pt->speedrunSplits.prestigeLevel3 <= minutesToMicroseconds(22));
    unlockIf(inSpeedrunMode && pt->speedrunSplits.prestigeLevel3 <= minutesToMicroseconds(18));

    mustGetFromSteam = false;
}
