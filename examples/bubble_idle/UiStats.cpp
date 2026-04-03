

#include "Achievements.hpp"
#include "Aliases.hpp"
#include "BubbleIdleMain.hpp"
#include "Cat.hpp"
#include "CatType.hpp"
#include "Milestones.hpp"
#include "PurchasableScalingValue.hpp"
#include "ShrineConstants.hpp"
#include "ShrineType.hpp"
#include "Stats.hpp"

#include "SFML/ImGui/IncludeImGui.hpp"

#include "SFML/Base/Algorithm/Count.hpp"
#include "SFML/Base/GetArraySize.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/SizeT.hpp"

void Main::uiTabBarStats()
{
    constexpr TabButtonPalette palette{
        .idle    = ImVec4(0.15f, 0.35f, 0.60f, 1.0f),
        .hovered = ImVec4(0.25f, 0.45f, 0.80f, 1.0f),
        .active  = ImVec4(0.35f, 0.55f, 0.95f, 1.0f),
    };

    const auto displayStats = [&](const Stats& stats)
    {
        ImGui::Spacing();
        ImGui::Spacing();

        const auto [h, m, s] = formatTime(stats.secondsPlayed);
        ImGui::Text("Time played: %lluh %llum %llus", h, m, s);

        ImGui::Spacing();
        ImGui::Spacing();

        const auto bubblesPopped            = stats.getTotalNBubblesPopped();
        const auto bubblesHandPopped        = stats.getTotalNBubblesHandPopped();
        const auto bubblesPoppedRevenue     = stats.getTotalRevenue();
        const auto bubblesHandPoppedRevenue = stats.getTotalRevenueHand();

        ImGui::Text("Bubbles popped: %s", toStringWithSeparators(bubblesPopped));
        ImGui::Indent();
        ImGui::Text("Clicked: %s", toStringWithSeparators(bubblesHandPopped));
        ImGui::Text("By cats: %s", toStringWithSeparators(bubblesPopped - bubblesHandPopped));
        ImGui::Unindent();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Text("Highest $/s: %s", toStringWithSeparators(stats.highestDPS));

        ImGui::Text("Revenue: $%s", toStringWithSeparators(bubblesPoppedRevenue));
        ImGui::Indent();
        ImGui::Text("Clicked: $%s", toStringWithSeparators(bubblesHandPoppedRevenue));
        ImGui::Text("By cats: $%s", toStringWithSeparators(bubblesPoppedRevenue - bubblesHandPoppedRevenue));
        ImGui::Indent();
        ImGui::Text("Bombs:  $%s", toStringWithSeparators(stats.explosionRevenue));
        ImGui::Text("Flights: $%s", toStringWithSeparators(stats.flightRevenue));
        ImGui::Text("Portals: $%s", toStringWithSeparators(stats.hellPortalRevenue));
        ImGui::Unindent();
        ImGui::Unindent();
    };

    static int lastSelectedTabIdx = 0;

    const auto selectedTab = [&](const int idx)
    {
        if (lastSelectedTabIdx != idx)
            playSound(sounds.uitab);

        lastSelectedTabIdx = idx;
    };

    ImGui::Spacing();
    ImGui::Spacing();

    uiSetFontScale(0.75f);

    ImGui::SameLine(0.f, 0.f);
    if (drawTabButton(0.75f, " Tips ##29990", lastSelectedTabIdx == 0, palette))
        selectedTab(0);

    ImGui::SameLine(0.f, 0.f);
    if (drawTabButton(0.75f, " Statistics ##29991", lastSelectedTabIdx == 1, palette))
        selectedTab(1);

    ImGui::SameLine(0.f, 0.f);
    if (drawTabButton(0.75f, " Milestones ##29992", lastSelectedTabIdx == 2, palette))
        selectedTab(2);

    ImGui::SameLine(0.f, 0.f);
    if (drawTabButton(0.75f, " Achievements ##29993", lastSelectedTabIdx == 3, palette))
        selectedTab(3);

    ImGui::Separator();

    switch (lastSelectedTabIdx)
    {
        case 0:
        {
            ImGui::BeginChild("TipsScroll", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y));

            const auto addTip = [&](const char* title, const char* description)
            {
                uiSetFontScale(uiNormalFontScale * 2.f);
                uiCenteredText(title);

                ImGui::PushFont(fontImGuiMouldyCheese);
                uiSetFontScale(uiNormalFontScale);
                ImGui::TextWrapped("%s", description);
                ImGui::PopFont();

                ImGui::Separator();
            };

            addTip("Getting Started",
                   "Click on bubbles to pop them and earn money.\n\nPurchase upgrades and cats to increase "
                   "your "
                   "income and automate your bubble popping journey.");

            if (pt->comboPurchased)
                addTip("Combos",
                       "Popping bubbles in quick succession will increase your combo multiplier, boosting your "
                       "revenue. Keep the combo going for maximum profit!\n\nPopping high-value bubbles such "
                       "as "
                       "star bubbles while your combo multiplier is high will yield even more revenue.");

            if (pt->getCatCountByType(CatType::Normal) > 0)
            {
                addTip("Regular Cats",
                       "Regular cats will automatically pop bubbles for you, even while you are away or the "
                       "game "
                       "is in the background.\n\nThey are the bread and butter of any cat formation!");

                addTip("Cat Dragging",
                       "You can drag cats around the screen to reposition them.\n\nMoving individual cats can "
                       "be "
                       "done by clicking and dragging them.\n\nMultiple cats can be moved at once by holding "
                       "down "
                       "left shift and dragging a selection box around them. After that, either release left "
                       "shift "
                       "or the mouse button and drag them to their intended position. This is a great way to "
                       "move "
                       "an entire formation of cats at once.");
            }

            if (pt->getCatCountByType(CatType::Uni) > 0)
                addTip("Unicats",
                       "Unicats will convert normal bubbles into star bubbles, which are worth x15 the value "
                       "of "
                       "normal bubbles.\n\nPopping star bubbles manually while your combo multiplier is high "
                       "(towards the end of a combo) is a great way of making money early.\n\nAlternatively, "
                       "you "
                       "can place regular cats under unicats to have them pop the star bubbles for you.");

            if (pt->mapPurchased)
            {
                addTip("Map Exploration",
                       "Expand the map to discover shrines containing powerful unique cats and to have more "
                       "real estate for your cat army.\n\nYou can scroll the map with the scroll wheel, "
                       "holding "
                       "right click, by dragging with two fingers, by using the A/D/Left/Right keys.\n\nYou "
                       "can "
                       "jump around the map by clicking on the minimap or using the PgUp/PgDn/Home/End keys.");

                addTip("Shrines",
                       "Shrines contain powerful cats with unique powers and synergies.\n\nIn order to unseal "
                       "the "
                       "cats, the shrine must first be activated by purchasing \"Activate next shrine\" in the "
                       "shop.\n\nAfterwards, the shrine must be completed by popping bubbles in its range "
                       "until "
                       "the required amount of revenue is collected.");
            }

            if (pt->psvBubbleValue.nPurchases > 0 ||
                (pt->getCatCountByType(CatType::Uni) >= 3 && pt->nShrinesCompleted > 0))
                addTip("Prestige",
                       "Prestige to reset your current progress, permanently increasing the value of bubbles "
                       "and "
                       "unlocking powerful permanent upgrades that persist between prestiges.\n\nDo not be "
                       "afraid "
                       "of prestiging, as its benefits will allow you to return to your current state very "
                       "quickly "
                       "and progress much further than it was possible before!");

            if (pt->getCatCountByType(CatType::Devil) > 0)
                addTip("Devilcats",
                       "Devilcats will convert normal bubbles into bombs, which explode when popped. Bubbles "
                       "caught in the explosion are worth x10 their original value. This means that every star "
                       "bubble caught in the explosion will be worth x150 the value of a normal "
                       "bubble.\n\nPosition regular cats beneath Devilcats to automatically pop bombs, and "
                       "Unicats "
                       "nearby to maximize the chance of having star bubbles caught in the explosion.");

            if (pt->getCatCountByType(CatType::Astro) > 0)
                addTip("Astrocats",
                       "Astrocats will periodically fly across the map, looping around when they reach the "
                       "edge of "
                       "the screen.\n\nAny bubble they touch while flying will be popped with a x20 "
                       "multiplier.\n\nUpgrading the total bubble count and expanding the map will indirectly "
                       "increase the effectiveness of Astrocats.");

            if (pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Witch)])
                addTip("Witchcat",
                       "The Witchcat periodically perform voodoo rituals.\n\nDuring a ritual, a random cat in "
                       "range of the Witchcat will be hexed and will become inactive until the ritual "
                       "ends.\n\nAt "
                       "the same time, voodoo dolls will appear throughout the map -- collect all of them to "
                       "end "
                       "the ritual and gain a powerful timed buff depending on the type of cat that was "
                       "hexed.");


            if (pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Wizard)])
                addTip("Wizardcat",
                       "The Wizardcat casts powerful spells using mana that regenerates over time.\n\nIn order "
                       "to "
                       "learn new spells, the Wizardcat must concentrate and absorb wisdom from star bubbles, "
                       "earning \"wisdom points\".\n\nCasting spells or changing the Wizardcat's state can be "
                       "done "
                       "in the \"Magic\" menu. ");

            if (pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Mouse)])
                addTip("Mousecat",
                       "The Mousecat pops nearby bubbles keeping up its own personal combo.\n\nCombo/click "
                       "upgrades you purchased also apply to the Mousecat.\n\nRegular cats in range of the "
                       "Mousecat will gain the same combo multiplier as the Mousecat.\n\nFurthermore, the "
                       "Mousecat "
                       "provides a global click revenue value buff.");


            if (pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Engi)])
                addTip("Engicat",
                       "The Engicat periodically increases the speed of nearby cats, effectively decreasing "
                       "their "
                       "cooldown.\n\nFurthermore, the Mousecat provides a global cat revenue value buff.");

            if (pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Repulso)])
                addTip("Repulsocat",
                       "The Repulsocat blows nearby bubbles away.\n\nRecently blown bubbles are worth x2 their "
                       "value.");

            if (pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Attracto)])
                addTip("Attractocat",
                       "The Attractocat attracts nearby bubbles.\n\nBubbles in range of the Attractocat are "
                       "worth "
                       "x2 their value.");

            if (pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Copy)])
                addTip("Copycat",
                       "The Copycat can mimic the abilities, effects, and properties of any other unique "
                       "cat.\n\nThe mimicked cat can be chosen through the disguise menu near the bottom of "
                       "the "
                       "screen.");

            ImGui::EndChild();
            break;
        }

        case 1:
        {
            uiSetFontScale(1.f);
            uiCenteredText(" ~~ Lifetime ~~ ");

            uiSetFontScale(0.75f);
            displayStats(profile.statsLifetime);

            ImGui::Separator();

            uiSetFontScale(1.f);
            uiCenteredText(" ~~ This playthrough ~~ ");

            uiSetFontScale(0.75f);
            displayStats(pt->statsTotal);

            ImGui::Separator();

            uiSetFontScale(1.f);
            uiCenteredText(" ~~ This prestige ~~ ");

            uiSetFontScale(0.75f);
            displayStats(pt->statsSession);

            ImGui::Spacing();
            ImGui::Spacing();

            // per cat type
            MoneyType revenueByCatType[nCatTypes] = {};

            for (const Cat& cat : pt->cats)
                revenueByCatType[asIdx(cat.type)] += cat.moneyEarned;

            ImGui::Text("Cat revenue: $%s", toStringWithSeparators(revenueByCatType[asIdx(CatType::Normal)]));
            ImGui::Text("Devilcat revenue: $%s", toStringWithSeparators(revenueByCatType[asIdx(CatType::Devil)]));

            ImGui::Text("Astrocat revenue: $%s", toStringWithSeparators(revenueByCatType[asIdx(CatType::Astro)]));
            ImGui::Text("Mousecat revenue: $%s", toStringWithSeparators(revenueByCatType[asIdx(CatType::Mouse)]));

            uiSetFontScale(uiNormalFontScale);
            break;
        }

        case 2:
        {
            uiSetFontScale(0.75f);

            const auto doMilestone = [&](const char* name, const MilestoneTimestamp value)
            {
                if (value == maxMilestone)
                {
                    ImGui::Text("%s: N/A", name);
                    return;
                }

                const auto [h, m, s] = formatTime(value);
                ImGui::Text("%s: %lluh %llum %llus", name, h, m, s);
            };

            doMilestone("1st Cat", pt->milestones.firstCat);
            doMilestone("5th Cat", pt->milestones.fiveCats);
            doMilestone("10th Cat", pt->milestones.tenCats);

            ImGui::Separator();

            doMilestone("1st Unicat", pt->milestones.firstUnicat);
            doMilestone("5th Unicat", pt->milestones.fiveUnicats);
            doMilestone("10th Unicat", pt->milestones.tenUnicats);

            ImGui::Separator();

            doMilestone("1st Devilcat", pt->milestones.firstDevilcat);
            doMilestone("5th Devilcat", pt->milestones.fiveDevilcats);
            doMilestone("10th Devilcat", pt->milestones.tenDevilcats);

            ImGui::Separator();

            doMilestone("1st Astrocat", pt->milestones.firstAstrocat);
            doMilestone("5th Astrocat", pt->milestones.fiveAstrocats);
            doMilestone("10th Astrocat", pt->milestones.tenAstrocats);

            ImGui::Separator();

            doMilestone("Prestige Level 2", pt->milestones.prestigeLevel2);
            doMilestone("Prestige Level 3", pt->milestones.prestigeLevel3);
            doMilestone("Prestige Level 4", pt->milestones.prestigeLevel4);
            doMilestone("Prestige Level 5", pt->milestones.prestigeLevel5);
            doMilestone("Prestige Level 6", pt->milestones.prestigeLevel6);
            doMilestone("Prestige Level 10", pt->milestones.prestigeLevel10);
            doMilestone("Prestige Level 15", pt->milestones.prestigeLevel15);
            doMilestone("Prestige Level 20", pt->milestones.prestigeLevel20);

            ImGui::Separator();

            doMilestone("$10.000 Revenue", pt->milestones.revenue10000);
            doMilestone("$100.000 Revenue", pt->milestones.revenue100000);
            doMilestone("$1.000.000 Revenue", pt->milestones.revenue1000000);
            doMilestone("$10.000.000 Revenue", pt->milestones.revenue10000000);
            doMilestone("$100.000.000 Revenue", pt->milestones.revenue100000000);
            doMilestone("$1.000.000.000 Revenue", pt->milestones.revenue1000000000);

            ImGui::Separator();

            for (SizeT i = 0u; i < nShrineTypes; ++i)
                doMilestone(shrineNames[i], pt->milestones.shrineCompletions[i]);
            break;
        }

        case 3:
        {
            const sf::base::SizeT nAchievementsUnlocked = sf::base::countTruthy(profile.unlockedAchievements,
                                                                                profile.unlockedAchievements + nAchievements);

            uiSetFontScale(uiNormalFontScale);
            ImGui::Text("%zu / %zu achievements unlocked", nAchievementsUnlocked, sf::base::getArraySize(achievementData));

            static bool showCompleted = true;
            uiSetFontScale(0.75f);
            uiCheckbox("Show completed", &showCompleted);

            ImGui::Separator();
            uiSetFontScale(0.75f);

            ImGui::BeginChild("AchScroll", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y));

            sf::base::U64 id = 0u;
            for (const auto& [name, description, secret] : achievementData)
            {
                const bool unlocked = profile.unlockedAchievements[id];

                if (!showCompleted && unlocked)
                {
                    ++id;
                    continue;
                }

                const float opacity = unlocked ? 1.f : 0.5f;

                const ImVec4 textColor{1.f, 1.f, 1.f, opacity};

                uiSetFontScale(uiNormalFontScale * 1.15f);
                ImGui::TextColored(textColor, "%llu - %s", id, (!secret || unlocked) ? name : "???");

                ImGui::PushFont(fontImGuiMouldyCheese);
                uiSetFontScale(0.75f);
                ImGui::TextColored(textColor, "%s", (!secret || unlocked) ? description : "(...secret achievement...)");

                if (!unlocked && achievementProgress[id].hasValue())
                    ImGui::TextColored(textColor,
                                       "(%s / %s)",
                                       toStringWithSeparators<0>(achievementProgress[id]->value),
                                       toStringWithSeparators<1>(achievementProgress[id]->threshold));

                ImGui::PopFont();

                ImGui::Separator();

                ++id;
            }

            if (debugMode)
            {
                uiState.uiButtonHueMod = 120.f;
                uiPushButtonColors();

                uiSetFontScale(uiNormalFontScale);
                if (ImGui::Button("Reset stats and achievements"))
                {
                    withAllStats([](Stats& stats) { stats = {}; });

                    for (bool& b : profile.unlockedAchievements)
                        b = false;
                }

                uiPopButtonColors();
                uiState.uiButtonHueMod = 0.f;
            }

            ImGui::EndChild();
            break;
        }

        default:
            break;
    }

    uiSetFontScale(uiNormalFontScale);
}
