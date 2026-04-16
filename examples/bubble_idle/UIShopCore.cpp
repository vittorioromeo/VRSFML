#include "BubbleIdleMain.hpp"
#include "Cat.hpp"
#include "CatType.hpp"
#include "Version.hpp"

#include "SFML/ImGui/IncludeImGui.hpp"

#include "SFML/Base/SizeT.hpp"

#include <cstdio>

void Main::uiShopDrawCoreUpgrades()
{
    const auto nCatNormal = pt->getCatCountByType(CatType::Normal);
    const auto nCatUni    = pt->getCatCountByType(CatType::Uni);

    Cat* catMouse = getMouseCat();

    uiImgsep(txrMenuSeparator0, "click upgrades", /* first */ true);

    uiSetTooltipOnly(
        "Build your combo by popping bubbles quickly, increasing the value of each subsequent "
        "one.\n\nCombos expire on misclicks and over time, but can be upgraded to last "
        "longer.\n\nStar bubbles are affected -- pop them while your multiplier is high!");
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
            uiSetTooltip("Increase combo duration from %.2fs to %.2fs. We are in it for the long haul!%s",
                         static_cast<double>(currentComboStartTime),
                         static_cast<double>(nextComboStartTime),
                         mouseNote);
        }
        else
        {
            uiSetTooltip("Increase combo duration (MAX). We are in it for the long haul!%s", mouseNote);
        }

        uiSetLabel("%.2fs", static_cast<double>(currentComboStartTime));
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
        uiSetTooltipOnly(
            "Extend the map and enable scrolling.\n\nExtending the map will increase the total number "
            "of bubbles you can work with, and will also reveal magical shrines that grant unique cats "
            "upon completion.\n\nYou can scroll the map with the scroll wheel, holding right click, by "
            "dragging with two fingers, by using the A/D/Left/Right keys.\n\nYou can jump around the "
            "map by clicking on the minimap or using the PgUp/PgDn/Home/End keys.");
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
            uiSetTooltip("Extend the map further by one screen.");
            uiSetLabel("%.2f%%", static_cast<double>(pt->getMapLimit() / boundaries.x * 100.f));
            makePSVButton("  Extend map", pt->psvMapExtension);

            ImGui::BeginDisabled(pt->psvShrineActivation.nPurchases > pt->psvMapExtension.nPurchases);
            uiSetTooltip(
                "Activates the next shrine, enabling it to absorb nearby popped bubbles. Once enough "
                "bubbles are absorbed by a shrine, it will grant a unique cat.");
            uiSetLabel("%zu/9", pt->psvShrineActivation.nPurchases);
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
        uiSetTooltip(
            "Increase the total number of bubbles. Scales with map size.\n\nMore bubbles, "
            "more money, fewer FPS!");
        uiSetLabel("%zu bubbles", static_cast<sf::base::SizeT>(pt->psvBubbleCount.currentValue()));
        makePSVButton("More bubbles", pt->psvBubbleCount);
    }

    if (checkUiUnlock(4u, pt->comboPurchased && pt->psvComboStartTime.nPurchases > 0))
    {
        uiImgsep(txrMenuSeparator3, "cats");

        uiSetUnlockLabelY(4u);
        uiSetTooltip(
            "Cats pop nearby bubbles or bombs. Their cooldown and range can be upgraded. Their "
            "behavior can be permanently upgraded with prestige points.\n\nCats can be dragged around "
            "to position them strategically.\n\nNo, you can't get rid of a cat once purchased, you "
            "monster.");
        uiSetLabel("%zu cats", nCatNormal);
        if (makePSVButton("Cat", pt->psvPerCatType[asIdx(CatType::Normal)]))
        {
            spawnCatCentered(CatType::Normal, getHueByCatType(CatType::Normal), /* placeInHand */ !onSteamDeck);

            if (nCatNormal == 0)
                doTip("Cats periodically pop bubbles for you!\nYou can drag them around to position them.");

            if (nCatNormal == 4)
                doTip(
                    "Multiple cats can be dragged at once by\nholding shift while clicking the mouse.\nRelease "
                    "either button to drop them!");
        }
    }

    const bool catUpgradesUnlocked = pt->psvBubbleCount.nPurchases > 0 && nCatNormal >= 2 && nCatUni >= 1;
    if (checkUiUnlock(5u, catUpgradesUnlocked))
    {
        uiSetUnlockLabelY(5u);
        uiShopCooldownButton("  cooldown##Normal", CatType::Normal);
        uiShopRangeButton("  range##Normal", CatType::Normal);
    }

    // TODO: gate Wardencat behind a prestige-point unlock once the related
    // permanent upgrade is wired up. For now they're always purchasable in
    // the same section as Normal cats so the mechanic can be playtested.
    if (checkUiUnlock(4u, pt->comboPurchased && pt->psvComboStartTime.nPurchases > 0))
    {
        const auto nCatWarden = pt->getCatCountByType(CatType::Warden);

        uiSetTooltip(
            "Wardencats patrol their range and shake any napping cat awake.\n\nWardencats can also "
            "fall asleep — and yes, they will wake each other up.");
        uiSetLabel("%zu wardencats", nCatWarden);
        if (makePSVButton("Wardencat", pt->psvPerCatType[asIdx(CatType::Warden)]))
        {
            spawnCatCentered(CatType::Warden, getHueByCatType(CatType::Warden), /* placeInHand */ !onSteamDeck);

            if (nCatWarden == 0)
                doTip("Wardencats wake up sleeping cats nearby!\nThey can also nap themselves.");
        }

        if (catUpgradesUnlocked)
        {
            uiShopCooldownButton("  cooldown##Warden", CatType::Warden);
            uiShopRangeButton("  range##Warden", CatType::Warden);
        }
    }
}
