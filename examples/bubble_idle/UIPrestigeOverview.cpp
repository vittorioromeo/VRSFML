#include "BubbleIdleMain.hpp"
#include "BubbleType.hpp"
#include "Playthrough.hpp"
#include "Version.hpp"

#include "SFML/ImGui/IncludeImGui.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/SizeT.hpp"

#include <cstdio>

void Main::uiPrestigeDrawOverview()
{
    ImGui::Spacing();
    ImGui::Spacing();

    uiImgsep(txrPrestigeSeparator0, "prestige", /* first */ true);
    ImGui::Columns(1);
    uiSetFontScale(uiNormalFontScale);

    if constexpr (isDemoVersion)
    {
        uiCenteredTextColored({229u, 63u, 63u, 255u}, "(!) Limited to 1 prestige in demo (!)");
    }

    uiSetTooltip(
        "WARNING: this will reset your progress!\n\nPrestige to increase bubble value permanently and "
        "obtain prestige points. Prestige points can be used to unlock powerful permanent "
        "upgrades.\n\nYou will sacrifice all your cats, bubbles, and money, but you will keep your "
        "prestige points and permanent upgrades, and the value of bubbles will be permanently "
        "increased.\n\nDo not be afraid to prestige -- it is what enables you to progress further!");
    uiSetLabel("current bubble value x%llu", pt->getComputedRewardByBubbleType(BubbleType::Normal));

    const auto currentPrestigeLevel    = pt->psvBubbleValue.nPurchases;
    const auto currentCompletedShrines = pt->nShrinesCompleted;
    const auto maxPrestigeLevel        = sf::base::SizeT{20u};

    sf::base::SizeT maxPurchaseablePrestigeLevel = currentPrestigeLevel;
    MoneyType       maxCost                      = 0u;

    for (sf::base::SizeT iPrestige = currentPrestigeLevel + 1u; iPrestige < maxPrestigeLevel; ++iPrestige)
    {
        const auto requiredMoney = static_cast<MoneyType>(
            pt->psvBubbleValue.cumulativeCostBetween(currentPrestigeLevel, iPrestige));

        const auto requiredCompletedShrines = Playthrough::getShrinesCompletedNeededForPrestigeLevel(iPrestige);

        if (pt->money >= requiredMoney && currentCompletedShrines >= requiredCompletedShrines)
        {
            maxPurchaseablePrestigeLevel = iPrestige;
            maxCost                      = requiredMoney;
            continue;
        }

        break;
    }

    const auto prestigeTimes = maxPurchaseablePrestigeLevel - currentPrestigeLevel;
    const auto ppReward      = pt->calculatePrestigePointReward(prestigeTimes);

    const auto printNextPrestigeRequirements = [&](const sf::base::SizeT level)
    {
        const auto nextCost = static_cast<MoneyType>(pt->psvBubbleValue.cumulativeCostBetween(currentPrestigeLevel, level));
        const auto nextRequiredShrines = Playthrough::getShrinesCompletedNeededForPrestigeLevel(level);

        ImGui::Text("    (level %zu -> %zu): $%s, %zu completed shrines",
                    currentPrestigeLevel + 1u,
                    level + 1u,
                    toStringWithSeparators(nextCost),
                    nextRequiredShrines);
    };

    uiSetFontScale(0.75f);

    const bool canPrestigePlus1 = maxPurchaseablePrestigeLevel + 1u < maxPrestigeLevel;
    const bool canPrestigePlus2 = maxPurchaseablePrestigeLevel + 2u < maxPrestigeLevel;

    if (canPrestigePlus1 || canPrestigePlus2)
    {
        ImGui::Text("  next prestige requirements:");

        if (canPrestigePlus1)
            printNextPrestigeRequirements(maxPurchaseablePrestigeLevel + 1u);

        if (canPrestigePlus2)
            printNextPrestigeRequirements(maxPurchaseablePrestigeLevel + 2u);

        ImGui::Spacing();
        ImGui::Spacing();
    }

    uiSetFontScale(uiNormalFontScale);

    uiBeginColumns();

    uiState.uiButtonHueMod = 120.f;

    ImGui::BeginDisabled(prestigeTimes == 0u);
    if (makePSVButtonEx("Prestige", pt->psvBubbleValue, prestigeTimes, maxCost))
        beginPrestigeTransition(ppReward);
    ImGui::EndDisabled();

    ImGui::Columns(1);

    uiState.uiButtonHueMod = 0.f;
    uiSetFontScale(0.75f);

    const auto currentMult = static_cast<sf::base::SizeT>(pt->psvBubbleValue.currentValue()) + 1u;

    if (prestigeTimes > 0u)
    {
        ImGui::Text(
            "  prestige %zu time(s) at once\n  - increase bubble value from x%zu to x%zu\n  - obtain %llu prestige "
            "point(s)",
            prestigeTimes,
            currentMult,
            currentMult + prestigeTimes,
            ppReward);
    }
    else if (pt->psvBubbleValue.nPurchases == 19u)
    {
        ImGui::Text("  max prestige level reached!");

        uiSetFontScale(uiSubBulletFontScale);
        uiBeginColumns();

        uiSetTooltipOnly("Spend money to immediately get prestige points.");

        static int buyPPTimes = 1;

        char buf[256];
        std::snprintf(buf, sizeof(buf), "  Buy %d PPs", buyPPTimes * 100);

        bool done = false;
        if (makePurchasableButtonOneTime(buf, 1'000'000'000'000u * static_cast<unsigned int>(buyPPTimes), done))
            pt->prestigePoints += 100u * static_cast<unsigned int>(buyPPTimes);

        ImGui::NextColumn();

        uiSetFontScale(uiSubBulletFontScale);
        ImGui::InputInt("times##buypptimes", &buyPPTimes);
        buyPPTimes = sf::base::clamp(buyPPTimes, 1, 100);
        uiSetFontScale(uiNormalFontScale);

        ImGui::Columns(1);
    }
    else
    {
        if (maxCost == 0u)
            ImGui::Text("  not enough money to prestige");

        const sf::base::SizeT shrinesNeeded = pt->getShrinesCompletedNeededForNextPrestige();

        if (pt->nShrinesCompleted < shrinesNeeded)
            ImGui::Text("  must complete %zu more shrine(s)", shrinesNeeded - pt->nShrinesCompleted);
    }

    uiSetFontScale(uiNormalFontScale);

    uiImgsep(txrPrestigeSeparator1, "permanent upgrades");
    ImGui::Columns(1);

    uiSetFontScale(uiNormalFontScale * 1.1f);
    char ppBuf[256];
    std::snprintf(ppBuf, sizeof(ppBuf), "%llu PPs available", pt->prestigePoints);
    uiCenteredText(ppBuf);
    uiSetFontScale(uiNormalFontScale);

    ImGui::BeginDisabled(undoPPPurchase.empty());
    {
        ImGui::Spacing();

        uiBeginColumns();

        bool done = false;
        uiSetTooltipOnly("Undo your last PP purchase, refunding you the prestige points.");

        if (uiMakePrestigeOneTimeButton("  Undo last purchase",
                                        0u,
                                        done,
                                        "Undo your last PP purchase, refunding you the prestige points."))
        {
            SFML_BASE_ASSERT(!undoPPPurchase.empty());

            undoPPPurchase.back()();
            undoPPPurchase.popBack();
            done = false;
        }

        uiSetFontScale(uiToolTipFontScale);

        if (undoPPPurchase.empty())
            ImGui::Text("%s", "");
        else
            ImGui::Text("Undo time left: %.2fs", static_cast<double>(undoPPPurchaseTimer.value / 1000.f));

        uiSetFontScale(uiNormalFontScale);

        ImGui::Columns(1);
    }
    ImGui::EndDisabled();

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
}
