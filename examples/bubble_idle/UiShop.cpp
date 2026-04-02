

#include "Aliases.hpp"
#include "BubbleIdleMain.hpp"
#include "Cat.hpp"
#include "CatConstants.hpp"
#include "CatType.hpp"
#include "Constants.hpp"
#include "Countdown.hpp"
#include "PurchasableScalingValue.hpp"
#include "Version.hpp"

#include "SFML/ImGui/IncludeImGui.hpp"

#include "SFML/System/Vec2Base.hpp"

#include "SFML/Base/String.hpp"
#include "SFML/Base/ToString.hpp"

#include <cstdio>

void Main::uiTabBarShop()
{
    const auto nCatNormal = pt->getCatCountByType(CatType::Normal);
    const auto nCatUni    = pt->getCatCountByType(CatType::Uni);
    const auto nCatDevil  = pt->getCatCountByType(CatType::Devil);
    const auto nCatAstro  = pt->getCatCountByType(CatType::Astro);

    Cat* catWitch    = getWitchCat();
    Cat* catWizard   = getWizardCat();
    Cat* catMouse    = getMouseCat();
    Cat* catEngi     = getEngiCat();
    Cat* catRepulso  = getRepulsoCat();
    Cat* catAttracto = getAttractoCat();

    const bool anyUniqueCat = catWitch != nullptr || catWizard != nullptr || catMouse != nullptr ||
                              catEngi != nullptr || catRepulso != nullptr || catAttracto != nullptr;

    ImGui::Spacing();
    ImGui::Spacing();

    uiImgsep(txrMenuSeparator0, "click upgrades", /* first */ true);

    std::sprintf(uiState.uiTooltipBuffer,
                 "Build your combo by popping bubbles quickly, increasing the value of each subsequent "
                 "one.\n\nCombos expire on misclicks and over time, but can be upgraded to last "
                 "longer.\n\nStar bubbles are affected -- pop them while your multiplier is high!");
    uiState.uiLabelBuffer[0] = '\0';
    if (makePurchasableButtonOneTime("Combo", 20u, pt->comboPurchased))
    {
        comboState.combo = 0;
        doTip("Pop bubbles quickly to keep\nyour combo up and make more money!");
    }

    if (checkUiUnlock(0u, pt->comboPurchased))
    {
        const char* mouseNote = catMouse == nullptr ? "" : "\n\n(Note: this also applies to the Mousecat!)";

        const float currentComboStartTime = pt->psvComboStartTime.currentValue();
        const float nextComboStartTime    = pt->psvComboStartTime.nextValue();

        uiSetUnlockLabelY(0u);
        if (!pt->psvComboStartTime.isMaxedOut())
        {
            std::sprintf(uiState.uiTooltipBuffer,
                         "Increase combo duration from %.2fs to %.2fs. We are in it for the long haul!%s",
                         static_cast<double>(currentComboStartTime),
                         static_cast<double>(nextComboStartTime),
                         mouseNote);
        }
        else
        {
            std::sprintf(uiState.uiTooltipBuffer, "Increase combo duration (MAX). We are in it for the long haul!%s", mouseNote);
        }

        std::sprintf(uiState.uiLabelBuffer, "%.2fs", static_cast<double>(currentComboStartTime));
        makePSVButton("  Longer combo", pt->psvComboStartTime);
    }

    if (checkUiUnlock(1u, nCatNormal > 0 && pt->psvComboStartTime.nPurchases > 0))
    {
        uiImgsep(txrMenuSeparator1, "exploration");

        if constexpr (isDemoVersion)
        {
            ImGui::Columns(1);
            uiSetFontScale(uiNormalFontScale);
            uiCenteredTextColored({229u, 63u, 63u, 255u}, "(!) Limited to 2 shrines in demo (!)");
            uiBeginColumns();
        }

        uiSetUnlockLabelY(1u);
        std::sprintf(uiState.uiTooltipBuffer,
                     "Extend the map and enable scrolling.\n\nExtending the map will increase the total number "
                     "of "
                     "bubbles you can work with, and will also reveal magical shrines that grant unique cats "
                     "upon "
                     "completion.\n\nYou can scroll the map with the scroll wheel, holding right click, by "
                     "dragging with two fingers, by using the A/D/Left/Right keys.\n\nYou can jump around the "
                     "map "
                     "by clicking on the minimap or using the PgUp/PgDn/Home/End keys.");
        uiState.uiLabelBuffer[0] = '\0';
        if (makePurchasableButtonOneTime("Map scrolling", 1000u, pt->mapPurchased))
        {
            playerInputState.scroll = 0.f;
            doTip(
                "Explore the map by using the mouse wheel,\ndragging via right click, or with your "
                "keyboard.\nYou can also click on the minimap!");

            if (pt->psvBubbleValue.nPurchases == 0u)
                uiState.scrollArrowCountdown.value = 2000.f;
        }

        if (checkUiUnlock(2u, pt->mapPurchased))
        {
            uiSetUnlockLabelY(2u);
            std::sprintf(uiState.uiTooltipBuffer, "Extend the map further by one screen.");
            std::sprintf(uiState.uiLabelBuffer, "%.2f%%", static_cast<double>(pt->getMapLimit() / boundaries.x * 100.f));
            makePSVButton("  Extend map", pt->psvMapExtension);

            ImGui::BeginDisabled(pt->psvShrineActivation.nPurchases > pt->psvMapExtension.nPurchases);
            std::sprintf(uiState.uiTooltipBuffer,
                         "Activates the next shrine, enabling it to absorb nearby popped bubbles. Once enough "
                         "bubbles are absorbed by a shrine, it will grant a unique cat.");
            std::sprintf(uiState.uiLabelBuffer, "%zu/9", pt->psvShrineActivation.nPurchases);
            if (makePSVButton("  Activate next shrine", pt->psvShrineActivation))
            {
                if (!pt->shrineActivateTipShown)
                {
                    pt->shrineActivateTipShown = true;

                    if (pt->psvBubbleValue.nPurchases == 0u)
                    {
                        doTip(
                            "Move your cats near the shrine to help\n it absorb bubbles and unlock a unique "
                            "cat!\nWill there be consequences?");
                    }
                }
            }
            ImGui::EndDisabled();
        }
    }

    if (checkUiUnlock(3u, nCatNormal > 0 && pt->psvComboStartTime.nPurchases > 0))
    {
        uiImgsep(txrMenuSeparator2, "bubble upgrades");

        uiSetUnlockLabelY(3u);
        std::sprintf(uiState.uiTooltipBuffer,
                     "Increase the total number of bubbles. Scales with map size.\n\nMore bubbles, "
                     "more money, fewer FPS!");
        std::sprintf(uiState.uiLabelBuffer, "%zu bubbles", static_cast<SizeT>(pt->psvBubbleCount.currentValue()));
        makePSVButton("More bubbles", pt->psvBubbleCount);
    }

    if (checkUiUnlock(4u, pt->comboPurchased && pt->psvComboStartTime.nPurchases > 0))
    {
        uiImgsep(txrMenuSeparator3, "cats");

        uiSetUnlockLabelY(4u);
        std::sprintf(uiState.uiTooltipBuffer,
                     "Cats pop nearby bubbles or bombs. Their cooldown and range can be upgraded. Their "
                     "behavior can be permanently upgraded with prestige points.\n\nCats can be dragged around "
                     "to "
                     "position them strategically.\n\nNo, you can't get rid of a cat once purchased, you "
                     "monster.");
        std::sprintf(uiState.uiLabelBuffer, "%zu cats", nCatNormal);
        if (makePSVButton("Cat", pt->psvPerCatType[asIdx(CatType::Normal)]))
        {
            spawnCatCentered(CatType::Normal, getHueByCatType(CatType::Normal), /* placeInHand */ !onSteamDeck);

            if (nCatNormal == 0)
                doTip("Cats periodically pop bubbles for you!\nYou can drag them around to position them.");

            if (nCatNormal == 2)
                doTip(
                    "Multiple cats can be dragged at once by\nholding shift while clicking the mouse.\nRelease "
                    "either button to drop them!");
        }
    }

    const auto makeCooldownButton = [this](const char* label, const CatType catType, const char* additionalInfo = "")
    {
        auto& psv = pt->psvCooldownMultsPerCatType[asIdx(catType)];

        const float currentCooldown = CatConstants::baseCooldowns[asIdx(catType)] * psv.currentValue();
        const float nextCooldown    = CatConstants::baseCooldowns[asIdx(catType)] * psv.nextValue();

        if (!psv.isMaxedOut())
        {
            std::sprintf(uiState.uiTooltipBuffer,
                         "Decrease cooldown from %.2fs to %.2fs.%s\n\n(Note: can be reverted by right-clicking, "
                         "but no refunds!)",
                         static_cast<double>(currentCooldown / 1000.f),
                         static_cast<double>(nextCooldown / 1000.f),
                         additionalInfo);
        }
        else
        {
            std::sprintf(uiState.uiTooltipBuffer,
                         "Decrease cooldown (MAX).%s\n\n(Note: can be reverted by right-clicking, but "
                         "no refunds!)",
                         additionalInfo);
        }

        std::sprintf(uiState.uiLabelBuffer, "%.2fs", static_cast<double>(pt->getComputedCooldownByCatType(catType) / 1000.f));

        makePSVButton(label, psv);

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Right) && psv.nPurchases > 0u)
        {
            --psv.nPurchases;
            playSound(sounds.buy);
        }
    };

    const auto makeRangeButton = [this](const char* label, const CatType catType, const char* additionalInfo = "")
    {
        auto& psv = pt->psvRangeDivsPerCatType[asIdx(catType)];

        const float currentRange = CatConstants::baseRanges[asIdx(catType)] / psv.currentValue();
        const float nextRange    = CatConstants::baseRanges[asIdx(catType)] / psv.nextValue();

        if (!psv.isMaxedOut())
        {
            std::sprintf(uiState.uiTooltipBuffer,
                         "Increase range from %.2fpx to %.2fpx.%s\n\n(Note: can be reverted by right-clicking, but "
                         "no refunds!)",
                         static_cast<double>(currentRange),
                         static_cast<double>(nextRange),
                         additionalInfo);
        }
        else
        {
            std::sprintf(uiState.uiTooltipBuffer,
                         "Increase range (MAX).%s\n\n(Note: can be reverted by right-clicking, but "
                         "no refunds!)",
                         additionalInfo);
        }

        std::sprintf(uiState.uiLabelBuffer, "%.2fpx", static_cast<double>(pt->getComputedRangeByCatType(catType)));
        makePSVButton(label, psv);

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Right) && psv.nPurchases > 0u)
        {
            --psv.nPurchases;
            playSound(sounds.buy);
        }
    };

    const bool catUpgradesUnlocked = pt->psvBubbleCount.nPurchases > 0 && nCatNormal >= 2 && nCatUni >= 1;
    if (checkUiUnlock(5u, catUpgradesUnlocked))
    {
        uiSetUnlockLabelY(5u);
        makeCooldownButton("  cooldown##Normal", CatType::Normal);
        makeRangeButton("  range##Normal", CatType::Normal);
    }

    // UNICAT
    const bool catUnicornUnlocked         = pt->psvBubbleCount.nPurchases > 0 && nCatNormal >= 3;
    const bool catUnicornUpgradesUnlocked = catUnicornUnlocked && nCatUni >= 2 && nCatDevil >= 1;
    if (checkUiUnlock(6u, catUnicornUnlocked))
    {
        uiImgsep(txrMenuSeparator4, "unicats");

        uiSetUnlockLabelY(6u);
        std::sprintf(uiState.uiTooltipBuffer,
                     "Unicats transform bubbles into star bubbles, which are worth x15 more!\n\nHave "
                     "your cats pop them for you, or pop them towards the end of a combo for huge rewards!");
        std::sprintf(uiState.uiLabelBuffer, "%zu unicats", nCatUni);
        if (makePSVButton("Unicat", pt->psvPerCatType[asIdx(CatType::Uni)]))
        {
            spawnCatCentered(CatType::Uni, getHueByCatType(CatType::Uni), /* placeInHand */ !onSteamDeck);

            if (nCatUni == 0)
                doTip("Unicats transform bubbles in star bubbles,\nworth x15! Pop them at the end of a combo!");
        }

        if (checkUiUnlock(7u, catUnicornUpgradesUnlocked))
        {
            uiSetUnlockLabelY(7u);
            makeCooldownButton("  cooldown##Uni", CatType::Uni);

            if (pt->perm.unicatTranscendencePurchased && pt->perm.unicatTranscendenceAOEPurchased)
                makeRangeButton("  range##Uni", CatType::Uni);
        }
    }

    // DEVILCAT
    const bool catDevilUnlocked         = pt->psvBubbleValue.nPurchases > 0 && nCatNormal >= 6 && nCatUni >= 4 &&
                                          pt->nShrinesCompleted >= 1;
    const bool catDevilUpgradesUnlocked = catDevilUnlocked && nCatDevil >= 2 && nCatAstro >= 1;
    if (checkUiUnlock(8u, catDevilUnlocked))
    {
        uiImgsep(txrMenuSeparator5, "devilcats");

        uiSetUnlockLabelY(8u);
        std::sprintf(uiState.uiTooltipBuffer,
                     "Devilcats transform bubbles into bombs that explode when popped. Bubbles affected by the "
                     "explosion are worth x10 more! Bomb explosion range can be upgraded.");
        std::sprintf(uiState.uiLabelBuffer, "%zu devilcats", nCatDevil);
        if (makePSVButton("Devilcat", pt->psvPerCatType[asIdx(CatType::Devil)]))
        {
            spawnCatCentered(CatType::Devil, getHueByCatType(CatType::Devil), /* placeInHand */ !onSteamDeck);

            if (nCatDevil == 0)
                doTip(
                    "Devilcats transform bubbles in bombs!\nExplode them to pop nearby "
                    "bubbles\nwith a x10 money multiplier!",
                    /* maxPrestigeLevel */ 1);
        }

        if (checkUiUnlock(9u, nCatDevil >= 1) && !isDevilcatHellsingedActive())
        {
            const float currentExplosionRadius = pt->psvExplosionRadiusMult.currentValue();
            const float nextExplosionRadius    = pt->psvExplosionRadiusMult.nextValue();

            uiSetUnlockLabelY(9u);
            std::sprintf(uiState.uiTooltipBuffer,
                         "Increase bomb explosion radius from x%.2f to x%.2f.",
                         static_cast<double>(currentExplosionRadius),
                         static_cast<double>(nextExplosionRadius));
            std::sprintf(uiState.uiLabelBuffer, "x%.2f", static_cast<double>(currentExplosionRadius));
            makePSVButton("  Explosion radius", pt->psvExplosionRadiusMult);
        }

        if (checkUiUnlock(10u, catDevilUpgradesUnlocked))
        {
            uiSetUnlockLabelY(10u);
            makeCooldownButton("  cooldown##Devil", CatType::Devil);

            if (pt->perm.devilcatHellsingedPurchased)
                makeRangeButton("  range##Devil", CatType::Devil);
        }
    }

    // ASTROCAT
    const bool astroCatUnlocked = nCatNormal >= 10 && nCatUni >= 5 && nCatDevil >= 2 && pt->nShrinesCompleted >= 2;
    const bool astroCatUpgradesUnlocked = astroCatUnlocked && nCatDevil >= 9 && nCatAstro >= 5;
    if (checkUiUnlock(11u, astroCatUnlocked))
    {
        uiImgsep(txrMenuSeparator6, "astrocats");

        uiSetUnlockLabelY(11u);
        std::sprintf(uiState.uiTooltipBuffer,
                     "Astrocats periodically fly across the map, popping bubbles they hit with a huge x20 "
                     "money "
                     "multiplier!\n\nThey can be permanently upgraded with prestige points to inspire cats "
                     "watching them fly past to pop bubbles faster.");
        std::sprintf(uiState.uiLabelBuffer, "%zu astrocats", nCatAstro);
        if (makePSVButton("Astrocat", pt->psvPerCatType[asIdx(CatType::Astro)]))
        {
            spawnCatCentered(CatType::Astro, getHueByCatType(CatType::Astro), /* placeInHand */ !onSteamDeck);

            if (nCatAstro == 0)
                doTip(
                    "Astrocats periodically fly across\nthe entire map, with a huge\nx20 "
                    "money multiplier!",
                    /* maxPrestigeLevel */ 1);
        }

        if (checkUiUnlock(12u, astroCatUpgradesUnlocked))
        {
            uiSetUnlockLabelY(12u);
            makeCooldownButton("  cooldown##Astro", CatType::Astro);
            makeRangeButton("  range##Astro", CatType::Astro);
        }
    }

    // UNIQUE CAT BONUSES
    if (anyUniqueCat)
    {
        uiImgsep(txrMenuSeparator7, "unique cats");

        if (checkUiUnlock(13u, catWitch != nullptr))
        {
            uiSetUnlockLabelY(13u);
            makeCooldownButton("  witchcat cooldown",
                               CatType::Witch,
                               "\n\nEffectively increases the frequency of rituals.");

            if (checkUiUnlock(14u, pt->perm.witchCatBuffPowerScalesWithNCats))
            {
                uiSetUnlockLabelY(14u);
                makeRangeButton("  witchcat range",
                                CatType::Witch,
                                "\n\nAllows more cats to participate in group rituals, increasing the duration "
                                "of "
                                "buffs.");
            }
        }

        if (checkUiUnlock(15u, catWizard != nullptr))
        {
            ImGui::Separator();

            uiSetUnlockLabelY(15u);
            makeCooldownButton("  wizardcat cooldown",
                               CatType::Wizard,
                               "\n\nDoes *not* increase mana generation rate, but increases star bubble "
                               "absorption "
                               "rate and decreases cooldown between spell casts.");

            makeRangeButton("  wizardcat range",
                            CatType::Wizard,
                            "\n\nEffectively increases the area of effect of most spells, and star bubble "
                            "absorption range.");
        }

        if (checkUiUnlock(16u, catMouse != nullptr))
        {
            ImGui::Separator();

            uiSetUnlockLabelY(16u);
            makeCooldownButton("  mousecat cooldown##Mouse", CatType::Mouse);
            makeRangeButton("  mousecat range##Mouse", CatType::Mouse);
        }

        if (checkUiUnlock(17u, catEngi != nullptr))
        {
            ImGui::Separator();

            uiSetUnlockLabelY(17u);
            makeCooldownButton("  engicat cooldown",
                               CatType::Engi,
                               "\n\nEffectively increases the frequency of maintenances.");

            makeRangeButton("  engicat range",
                            CatType::Engi,
                            "\n\nAllows more cats to be boosted by maintenance at once.");
        }

        if (checkUiUnlock(18u, catRepulso != nullptr))
        {
            ImGui::Separator();

            uiSetUnlockLabelY(18u);
            // makeCooldownButton("  repulsocat cooldown##Repulso", CatType::Repulso);
            makeRangeButton("  repulsocat range##Repulso", CatType::Repulso);
        }

        if (checkUiUnlock(19u, catAttracto != nullptr))
        {
            ImGui::Separator();

            uiSetUnlockLabelY(19u);
            // makeCooldownButton("  attractocat cooldown##Attracto", CatType::Attracto);
            makeRangeButton("  attractocat range##Attracto", CatType::Attracto);
        }
    }

    const auto nextGoalsText = [&]() -> sf::base::String
    {
        sf::base::String result; // Use a single local variable for NRVO

        if (!pt->comboPurchased)
        {
            result = "- buy combo to earn money faster";
            return result;
        }

        if (pt->psvComboStartTime.nPurchases == 0)
        {
            result = "- buy longer combo to unlock cats";
            return result;
        }

        if (nCatNormal == 0)
        {
            result = "- buy a cat";
            return result;
        }

        const auto startList = [&](const char* s)
        {
            result += result.empty() ? "" : "\n\n";
            result += s;
        };

        const auto needNCats = [&](const SizeT& count, const SizeT needed)
        {
            const char* name = "";

            // clang-format off
                if      (&count == &nCatNormal) name = "cat";
                else if (&count == &nCatUni)    name = "unicat";
                else if (&count == &nCatDevil)  name = "devilcat";
                else if (&count == &nCatAstro)  name = "astrocat";
            // clang-format on

            if (count < needed)
                result += "\n    - buy " + sf::base::toString(needed - count) + " more " + name + "(s)";
        };

        if (!pt->mapPurchased)
        {
            startList("- to extend playing area:");
            result += "\n    - buy map scrolling";
        }

        if (!catUnicornUnlocked)
        {
            startList("- to unlock unicats:");

            if (pt->psvBubbleCount.nPurchases == 0)
                result += "\n    - buy more bubbles";

            needNCats(nCatNormal, 3);
        }

        if (!catUpgradesUnlocked && catUnicornUnlocked)
        {
            startList("- to unlock cat upgrades:");

            if (pt->psvBubbleCount.nPurchases == 0)
                result += "\n    - buy more bubbles";

            needNCats(nCatNormal, 2);
            needNCats(nCatUni, 1);
        }

        // TODO P2: change dynamically
        if (catUnicornUnlocked && !pt->isBubbleValueUnlocked())
        {
            startList("- to unlock prestige:");

            if (pt->psvBubbleCount.nPurchases == 0)
                result += "\n    - buy more bubbles";

            if (pt->nShrinesCompleted < 1)
                result += "\n    - complete at least one shrine";

            needNCats(nCatUni, 3);
        }

        if (catUnicornUnlocked && pt->isBubbleValueUnlocked() && !catDevilUnlocked)
        {
            startList("- to unlock devilcats:");

            if (pt->psvBubbleValue.nPurchases == 0)
                result += "\n    - prestige at least once";

            if (pt->nShrinesCompleted < 1)
                result += "\n    - complete at least one shrine";

            if (pt->psvBubbleValue.nPurchases > 0u)
            {
                needNCats(nCatNormal, 6);
                needNCats(nCatUni, 4);
            }
        }

        if (catUnicornUnlocked && catDevilUnlocked && !catUnicornUpgradesUnlocked)
        {
            startList("- to unlock unicat upgrades:");
            needNCats(nCatUni, 2);
            needNCats(nCatDevil, 1);
        }

        if (catUnicornUnlocked && catDevilUnlocked && !astroCatUnlocked)
        {
            startList("- to unlock astrocats:");

            if (pt->nShrinesCompleted < 2)
                result += "\n    - complete at least two shrines";

            needNCats(nCatNormal, 10);
            needNCats(nCatUni, 5);
            needNCats(nCatDevil, 2);
        }

        if (catUnicornUnlocked && catDevilUnlocked && astroCatUnlocked && !catDevilUpgradesUnlocked)
        {
            startList("- to unlock devilcat upgrades:");
            needNCats(nCatDevil, 2);
            needNCats(nCatAstro, 1);
        }

        if (catUnicornUnlocked && catDevilUnlocked && astroCatUnlocked && !astroCatUpgradesUnlocked)
        {
            startList("- to unlock astrocat upgrades:");
            needNCats(nCatDevil, 9);
            needNCats(nCatAstro, 5);
        }

        return result;
    }();

    ImGui::Columns(1);

    if (nextGoalsText != "")
    {
        uiImgsep(txrMenuSeparator8, "next goals");
        ImGui::Columns(1);

        uiSetFontScale(uiSubBulletFontScale);
        ImGui::Text("%s", nextGoalsText.cStr());
        uiSetFontScale(uiNormalFontScale);
    }

    ImGui::Columns(1);
}

////////////////////////////////////////////////////////////
bool Main::uiCheckbox(const char* label, bool* b)
{
    if (!ImGui::Checkbox(label, b))
        return false;

    playSound(sounds.btnswitch);
    return true;
}

////////////////////////////////////////////////////////////
bool Main::uiRadio(const char* label, int* i, const int value)
{
    if (!ImGui::RadioButton(label, i, value))
        return false;

    playSound(sounds.btnswitch);
    return true;
}
