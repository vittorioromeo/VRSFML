

#include "Aliases.hpp"
#include "BubbleIdleMain.hpp"
#include "BubbleType.hpp"
#include "CatType.hpp"
#include "Countdown.hpp"
#include "Playthrough.hpp"
#include "PurchasableScalingValue.hpp"
#include "Version.hpp"

#include "SFML/ImGui/IncludeImGui.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"

#include <climits>
#include <cstdio>

void Main::uiTabBarPrestige()
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

    std::sprintf(uiState.uiTooltipBuffer,
                 "WARNING: this will reset your progress!\n\nPrestige to increase bubble value permanently and "
                 "obtain prestige points. Prestige points can be used to unlock powerful permanent "
                 "upgrades.\n\nYou will sacrifice all your cats, bubbles, and money, but you will keep your "
                 "prestige points and permanent upgrades, and the value of bubbles will be permanently "
                 "increased.\n\nDo not be afraid to prestige -- it is what enables you to progress further!");
    std::sprintf(uiState.uiLabelBuffer, "current bubble value x%llu", pt->getComputedRewardByBubbleType(BubbleType::Normal));

    // Figure out how many times we can prestige in a row

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
    {
        beginPrestigeTransition(ppReward);
    }
    ImGui::EndDisabled();

    ImGui::Columns(1);

    uiState.uiButtonHueMod = 0.f;
    uiSetFontScale(0.75f);

    const auto currentMult = static_cast<SizeT>(pt->psvBubbleValue.currentValue()) + 1u;

    if (prestigeTimes > 0u)
    {
        ImGui::Text(
            "  prestige %zu time(s) at once\n  - increase bubble value from x%zu to x%zu\n  - obtain %llu "
            "prestige "
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

        std::sprintf(uiState.uiTooltipBuffer, "Spend money to immediately get prestige points.");
        uiState.uiLabelBuffer[0] = '\0';

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

        const SizeT shrinesNeeded = pt->getShrinesCompletedNeededForNextPrestige();

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

        uiState.uiLabelBuffer[0] = '\0';

        std::sprintf(uiState.uiTooltipBuffer, "Undo your last PP purchase, refunding you the prestige points.");

        if (makePurchasablePPButtonOneTime("  Undo last purchase", 0u, done))
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

    uiBeginColumns();

    uiState.uiButtonHueMod = 190.f;

    if (checkUiUnlock(47u, pt->psvBubbleValue.nPurchases >= 3u))
    {
        uiImgsep2(txrPrestigeSeparator4, "faster beginning");

        uiSetUnlockLabelY(47u);
        std::sprintf(uiState.uiTooltipBuffer, "Begin your next prestige with $1000.");
        uiState.uiLabelBuffer[0] = '\0';
        (void)makePurchasablePPButtonOneTime("Starter pack", 1u, pt->perm.starterPackPurchased);
    }

    if (checkUiUnlock(48u, pt->psvBubbleValue.nPurchases >= 1u))
    {
        uiImgsep2(txrPrestigeSeparator2, "clicking tools");

        uiSetUnlockLabelY(48u);
        std::sprintf(uiState.uiTooltipBuffer,
                     "Manually popping a bubble now also pops nearby bubbles automatically!\n\n(Note: combo "
                     "multiplier still only increases once per successful click.)\n\n(Note: this effect can be "
                     "toggled "
                     "at will.)");
        uiState.uiLabelBuffer[0] = '\0';
        if (makePurchasablePPButtonOneTime("Multipop click", 0u, pt->perm.multiPopPurchased))
            doTip("Popping a bubble now also pops\nnearby bubbles automatically!",
                  /* maxPrestigeLevel */ UINT_MAX);

        if (checkUiUnlock(49u, pt->perm.multiPopPurchased))
        {
            uiSetUnlockLabelY(49u);

            if (pt->psvBubbleValue.nPurchases >= 2u)
            {
                const float currentRange = pt->psvPPMultiPopRange.currentValue();
                const float nextRange    = pt->psvPPMultiPopRange.nextValue();

                if (!pt->psvPPMultiPopRange.isMaxedOut())
                {
                    std::sprintf(uiState.uiTooltipBuffer,
                                 "Increase the range of the multipop effect from %.2fpx to %.2fpx.",
                                 static_cast<double>(currentRange),
                                 static_cast<double>(nextRange));
                }
                else
                {
                    std::sprintf(uiState.uiTooltipBuffer, "Increase the range of the multipop effect (MAX).");
                }

                std::sprintf(uiState.uiLabelBuffer, "%.2fpx", static_cast<double>(currentRange));
                makePrestigePurchasablePPButtonPSV("  range", pt->psvPPMultiPopRange);
            }

            uiSetFontScale(uiSubBulletFontScale);
            uiCheckbox("enable ##multipop", &pt->multiPopEnabled);
            if (getMouseCat() != nullptr)
            {
                ImGui::SameLine();
                uiCheckbox("mousecat##multipopmousecat", &pt->multiPopMouseCatEnabled);
            }
            uiSetFontScale(uiNormalFontScale);
            ImGui::NextColumn();
            ImGui::NextColumn();
        }
    }

    uiImgsep2(txrPrestigeSeparator3, "wind effects");

    std::sprintf(uiState.uiTooltipBuffer,
                 "A giant fan (off-screen) will produce an intense wind, making bubbles move and "
                 "flow much faster.\n\n(Note: this effect can be toggled at will.)");
    uiState.uiLabelBuffer[0] = '\0';
    if (makePurchasablePPButtonOneTime("Giant fan", 6u, pt->perm.windPurchased))
        doTip("Hold onto something!", /* maxPrestigeLevel */ UINT_MAX);

    if (checkUiUnlock(50u, pt->perm.windPurchased))
    {
        uiSetFontScale(uiSubBulletFontScale);
        ImGui::Columns(1);

        uiSetUnlockLabelY(50u);
        uiRadio("off##windOff", &pt->windStrength, 0);
        ImGui::SameLine();
        uiRadio("slow##windSlow", &pt->windStrength, 1);
        ImGui::SameLine();
        uiRadio("medium##windMed", &pt->windStrength, 2);
        ImGui::SameLine();
        uiRadio("fast##windFast", &pt->windStrength, 3);

        uiBeginColumns();
        uiSetFontScale(uiNormalFontScale);
        ImGui::NextColumn();
        ImGui::NextColumn();
    }

    uiImgsep2(txrPrestigeSeparator5, "cats");

    uiBeginColumns();

    std::sprintf(uiState.uiTooltipBuffer,
                 "Cats have graduated!\n\nThey still cannot resist their popping insticts, but they will go "
                 "for star bubbles and bombs first, ensuring they are not wasted!");
    uiState.uiLabelBuffer[0] = '\0';
    if (makePurchasablePPButtonOneTime("Smart cats", 1u, pt->perm.smartCatsPurchased))
        doTip("Cats will now prioritize popping\nspecial bubbles over basic ones!",
              /* maxPrestigeLevel */ UINT_MAX);

    if (checkUiUnlock(51u, pt->perm.smartCatsPurchased))
    {
        uiSetUnlockLabelY(51u);
        std::sprintf(uiState.uiTooltipBuffer,
                     "Embrace the glorious evolution!\n\nCats have ascended beyond their primal "
                     "insticts and will now prioritize bombs, then star bubbles, then normal "
                     "bubbles!\n\nThey will also ignore any bubble type of your choosing.\n\nThrough the sheer "
                     "power of their intellect, they also get a x2 multiplier on all bubble values.\n\n(Note: "
                     "this effect can be toggled at will.)");
        uiState.uiLabelBuffer[0] = '\0';
        if (makePurchasablePPButtonOneTime("genius cats", 8u, pt->perm.geniusCatsPurchased))
            doTip("Genius cats prioritize bombs and\ncan be instructed to ignore certain bubbles!",
                  /* maxPrestigeLevel */ UINT_MAX);
    }

    if (checkUiUnlock(52u, pt->perm.geniusCatsPurchased))
    {
        ImGui::Columns(1);
        uiSetFontScale(uiSubBulletFontScale);

        uiSetUnlockLabelY(52u);
        ImGui::AlignTextToFramePadding();
        ImGui::Text("  ignore: ");
        ImGui::SameLine();

        auto& [ignoreNormal, ignoreStar, ignoreBomb] = pt->geniusCatIgnoreBubbles;

        uiCheckbox("normal##genius", &ignoreNormal);
        ImGui::SameLine();

        uiCheckbox("star##genius", &ignoreStar);
        ImGui::SameLine();

        uiCheckbox("bombs##genius", &ignoreBomb);

        uiSetFontScale(uiNormalFontScale);
        uiBeginColumns();
    }

    if (checkUiUnlock(53u, pt->psvBubbleValue.nPurchases >= 3))
    {
        uiImgsep2(txrPrestigeSeparator6, "unicats");

        uiBeginColumns();

        uiSetUnlockLabelY(53u);
        std::sprintf(uiState.uiTooltipBuffer,
                     "Unicats transcend their physical form, becoming a higher entity that transforms bubbles "
                     "into "
                     "nova bubbles, worth x50.");
        uiState.uiLabelBuffer[0] = '\0';

        if (makePurchasablePPButtonOneTime("transcendence", 96u, pt->perm.unicatTranscendencePurchased))
            doTip("Are you ready for that sweet x50?", /* maxPrestigeLevel */ UINT_MAX);

        if (checkUiUnlock(54u, pt->perm.unicatTranscendencePurchased))
        {
            uiSetUnlockLabelY(54u);
            std::sprintf(uiState.uiTooltipBuffer,
                         "Unicats can now transform all bubbles in range at once. Also unlocks Unicat range "
                         "upgrades.");
            uiState.uiLabelBuffer[0] = '\0';

            if (makePurchasablePPButtonOneTime("nova expanse", 128u, pt->perm.unicatTranscendenceAOEPurchased))
                doTip("It's about to get crazy...", /* maxPrestigeLevel */ UINT_MAX);

            if (pt->perm.unicatTranscendencePurchased)
            {
                ImGui::Columns(1);
                uiCheckbox("enable transcendent unicats", &pt->perm.unicatTranscendenceEnabled);
                uiBeginColumns();
            }
        }
    }

    if (checkUiUnlock(55u, pt->psvBubbleValue.nPurchases >= 3))
    {
        uiImgsep2(txrPrestigeSeparator7, "devilcats");

        uiBeginColumns();

        uiSetUnlockLabelY(55u);
        std::sprintf(uiState.uiTooltipBuffer,
                     "Devilcats become touched by the flames of hell, opening stationary portals that teleport "
                     "bubbles into the abyss, with a x50 multiplier. Also unlocks Devilcat range upgrades.");
        uiState.uiLabelBuffer[0] = '\0';

        if (makePurchasablePPButtonOneTime("hellsinged", 192u, pt->perm.devilcatHellsingedPurchased))
            doTip("I'm starting to get a bit scared...", /* maxPrestigeLevel */ UINT_MAX);

        if (pt->perm.devilcatHellsingedPurchased)
        {
            ImGui::Columns(1);
            uiCheckbox("enable hellsinged devilcats", &pt->perm.devilcatHellsingedEnabled);
            uiBeginColumns();
        }
    }

    if (checkUiUnlock(56u, pt->getCatCountByType(CatType::Astro) >= 1u || pt->psvBubbleValue.nPurchases >= 3))
    {
        uiImgsep2(txrPrestigeSeparator8, "astrocats");

        uiBeginColumns();

        uiCheckbox("disable flight", &pt->disableAstrocatFlight);

        uiSetUnlockLabelY(56u);
        std::sprintf(uiState.uiTooltipBuffer,
                     "Astrocats are now equipped with fancy patriotic flags, inspiring cats watching "
                     "them fly by to work faster!");
        uiState.uiLabelBuffer[0] = '\0';

        if (makePurchasablePPButtonOneTime("Space propaganda", 16u, pt->perm.astroCatInspirePurchased))
            doTip("Astrocats will inspire other cats\nto work faster when flying by!",
                  /* maxPrestigeLevel */ UINT_MAX);

        if (checkUiUnlock(57u, pt->perm.astroCatInspirePurchased))
        {
            const float currentDuration = pt->getComputedInspirationDuration();
            const float nextDuration    = pt->getComputedNextInspirationDuration();

            uiSetUnlockLabelY(57u);
            if (!pt->psvPPInspireDurationMult.isMaxedOut())
            {
                std::sprintf(uiState.uiTooltipBuffer,
                             "Increase the duration of the inspiration effect from %.2fs to %.2fs.",
                             static_cast<double>(currentDuration / 1000.f),
                             static_cast<double>(nextDuration / 1000.f));
            }
            else
            {
                std::sprintf(uiState.uiTooltipBuffer, "Increase the duration of the inspiration effect (MAX).");
            }
            std::sprintf(uiState.uiLabelBuffer, "%.2fs", static_cast<double>(currentDuration / 1000.f));

            makePrestigePurchasablePPButtonPSV("inspire duration", pt->psvPPInspireDurationMult);
        }
    }

    const auto makeUnsealButton = [&](const PrestigePointsType ppCost, const char* catName, const CatType type)
    {
        if (!pt->perm.shrineCompletedOnceByCatType[asIdx(type)])
            return;

        std::sprintf(uiState.uiTooltipBuffer,
                     "Permanently release the %s from their shrine. They will be waiting for you right "
                     "outside when the shrine is activated.\n\n(Note: completing the shrine will now grant "
                     "1.5x the money it absorbed.)",
                     catName);
        uiState.uiLabelBuffer[0] = '\0';

        char buf[256];
        std::snprintf(buf, sizeof(buf), "%s##%s", "Break the seal", catName);

        (void)makePurchasablePPButtonOneTime(buf, ppCost, pt->perm.unsealedByType[asIdx(type)]);
    };

    if (checkUiUnlock(58u, pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Witch)]))
    {
        uiImgsep2(txrPrestigeSeparator9, "witchcat");

        uiBeginColumns();

        uiSetUnlockLabelY(58u);
        makeUnsealButton(4u, "Witchcat", CatType::Witch);
        ImGui::Separator();

        const float currentDuration = pt->psvPPWitchCatBuffDuration.currentValue();
        const float nextDuration    = pt->psvPPWitchCatBuffDuration.nextValue();

        if (!pt->psvPPWitchCatBuffDuration.isMaxedOut())
        {
            std::sprintf(uiState.uiTooltipBuffer,
                         "Increase the base duration of Witchcat buffs from %.2fs to %.2fs.",
                         static_cast<double>(currentDuration),
                         static_cast<double>(nextDuration));
        }
        else
        {
            std::sprintf(uiState.uiTooltipBuffer, "Increase the base duration of Witchcat buffs (MAX).");
        }
        std::sprintf(uiState.uiLabelBuffer, "%.2fs", static_cast<double>(currentDuration));
        makePrestigePurchasablePPButtonPSV("Buff duration", pt->psvPPWitchCatBuffDuration);

        ImGui::Separator();

        std::sprintf(uiState.uiTooltipBuffer,
                     "The duration of Witchcat buffs scales with the number of cats in range of the ritual.");
        uiState.uiLabelBuffer[0] = '\0';
        (void)makePurchasablePPButtonOneTime("Group ritual", 4u, pt->perm.witchCatBuffPowerScalesWithNCats);

        std::sprintf(uiState.uiTooltipBuffer,
                     "The duration of Witchcat buffs scales with the size of the explored map.");
        uiState.uiLabelBuffer[0] = '\0';
        (void)makePurchasablePPButtonOneTime("Worldwide cult", 4u, pt->perm.witchCatBuffPowerScalesWithMapSize);

        ImGui::Separator();

        std::sprintf(uiState.uiTooltipBuffer, "Half as many voodoo dolls will appear per ritual.");
        uiState.uiLabelBuffer[0] = '\0';
        (void)makePurchasablePPButtonOneTime("Material shortage", 8u, pt->perm.witchCatBuffFewerDolls);

        ImGui::Separator();

        std::sprintf(uiState.uiTooltipBuffer, "Dolls are automatically collected by Devilcat bomb explosions.");
        uiState.uiLabelBuffer[0] = '\0';
        (void)makePurchasablePPButtonOneTime("Flammable dolls", 8u, pt->perm.witchCatBuffFlammableDolls);

        std::sprintf(uiState.uiTooltipBuffer, "Dolls are automatically collected by Astrocats during their flyby.");
        uiState.uiLabelBuffer[0] = '\0';
        (void)makePurchasablePPButtonOneTime("Orbital dolls", 16u, pt->perm.witchCatBuffOrbitalDolls);

        ImGui::Separator();

        const float currentUniPercentage = pt->psvPPUniRitualBuffPercentage.currentValue();
        const float nextUniPercentage    = pt->psvPPUniRitualBuffPercentage.nextValue();

        if (!pt->psvPPUniRitualBuffPercentage.isMaxedOut())
        {
            std::sprintf(uiState.uiTooltipBuffer,
                         "Increase the star bubble spawn chance during the Unicat vododoo ritual buff from %.2f%% "
                         "to %.2f%%.",
                         static_cast<double>(currentUniPercentage),
                         static_cast<double>(nextUniPercentage));
        }
        else
        {
            std::sprintf(uiState.uiTooltipBuffer,
                         "Increase the star bubble spawn chance during the Unicat vododoo ritual buff "
                         "(MAX).");
        }
        std::sprintf(uiState.uiLabelBuffer, "%.2f%%", static_cast<double>(currentUniPercentage));
        makePrestigePurchasablePPButtonPSV("Star Spawn %", pt->psvPPUniRitualBuffPercentage);

        const float currentDevilPercentage = pt->psvPPDevilRitualBuffPercentage.currentValue();
        const float nextDevilPercentage    = pt->psvPPDevilRitualBuffPercentage.nextValue();

        if (!pt->psvPPDevilRitualBuffPercentage.isMaxedOut())
        {
            std::sprintf(uiState.uiTooltipBuffer,
                         "Increase the bomb spawn chance during the Devil vododoo ritual buff from %.2f%% to "
                         "%.2f%%.",
                         static_cast<double>(currentDevilPercentage),
                         static_cast<double>(nextDevilPercentage));
        }
        else
        {
            std::sprintf(uiState.uiTooltipBuffer,
                         "Increase the bomb spawn chance during the Devil vododoo ritual buff (MAX).");
        }
        std::sprintf(uiState.uiLabelBuffer, "%.2f%%", static_cast<double>(currentDevilPercentage));
        makePrestigePurchasablePPButtonPSV("Bomb Spawn %", pt->psvPPDevilRitualBuffPercentage);
    }

    if (checkUiUnlock(59u, pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Wizard)]))
    {
        uiImgsep2(txrPrestigeSeparator10, "wizardcat");

        uiBeginColumns();

        uiSetUnlockLabelY(59u);
        makeUnsealButton(8u, "Wizardcat", CatType::Wizard);
        ImGui::Separator();

        const float currentManaCooldown = pt->getComputedManaCooldown();
        const float nextManaCooldown    = pt->getComputedManaCooldownNext();

        if (!pt->psvPPManaCooldownMult.isMaxedOut())
        {
            std::sprintf(uiState.uiTooltipBuffer,
                         "Decrease mana generation cooldown from %.2fs to %.2fs.",
                         static_cast<double>(currentManaCooldown / 1000.f),
                         static_cast<double>(nextManaCooldown / 1000.f));
        }
        else
        {
            std::sprintf(uiState.uiTooltipBuffer, "Decrease mana generation cooldown (MAX).");
        }
        std::sprintf(uiState.uiLabelBuffer, "%.2fs", static_cast<double>(currentManaCooldown / 1000.f));
        makePrestigePurchasablePPButtonPSV("Mana cooldown", pt->psvPPManaCooldownMult);

        const ManaType currentMaxMana = pt->getComputedMaxMana();
        const ManaType nextMaxMana    = pt->getComputedMaxManaNext();

        if (!pt->psvPPManaMaxMult.isMaxedOut())
        {
            std::sprintf(uiState.uiTooltipBuffer, "Increase the maximum mana from %llu to %llu.", currentMaxMana, nextMaxMana);
        }
        else
        {
            std::sprintf(uiState.uiTooltipBuffer, "Increase the maximum mana (MAX).");
        }
        std::sprintf(uiState.uiLabelBuffer, "%llu mana", currentMaxMana);
        makePrestigePurchasablePPButtonPSV("Mana limit", pt->psvPPManaMaxMult);

        ImGui::Separator();

        std::sprintf(uiState.uiTooltipBuffer,
                     "Allow the Wizardcat to automatically cast spells when enough mana is available. Can be "
                     "enabled and configured from the \"Magic\" tab.");
        uiState.uiLabelBuffer[0] = '\0';
        (void)makePurchasablePPButtonOneTime("Autocast", 4u, pt->perm.autocastPurchased);

        ImGui::Separator();

        std::sprintf(uiState.uiTooltipBuffer,
                     "Starpaw conversion ignores bombs, transforming only normal bubbles around the wizard "
                     "into star bubbles.");
        uiState.uiLabelBuffer[0] = '\0';
        (void)makePurchasablePPButtonOneTime("Selective starpaw", 4u, pt->perm.starpawConversionIgnoreBombs);

        if (pt->perm.unicatTranscendencePurchased && pt->perm.starpawConversionIgnoreBombs)
        {
            std::sprintf(uiState.uiTooltipBuffer, "Starpaw conversion now turns all normal bubbles into nova bubbles.");
            uiState.uiLabelBuffer[0] = '\0';
            (void)makePurchasablePPButtonOneTime("Nova starpaw", 64u, pt->perm.starpawNova);
        }

        ImGui::Separator();

        std::sprintf(uiState.uiTooltipBuffer, "The duration of Mewltiplier Aura is extended from 6s to 12s.");
        uiState.uiLabelBuffer[0] = '\0';
        (void)makePurchasablePPButtonOneTime("Meeeeeewltiplier", 64u, pt->perm.wizardCatDoubleMewltiplierDuration);

        ImGui::Separator();

        std::sprintf(uiState.uiTooltipBuffer, "The duration of Stasis Field is extended from 6s to 12s.");
        uiState.uiLabelBuffer[0] = '\0';
        (void)makePurchasablePPButtonOneTime("Pop Stuck In Time", 256u, pt->perm.wizardCatDoubleStasisFieldDuration);
    }

    if (checkUiUnlock(60u, pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Mouse)]))
    {
        uiImgsep2(txrPrestigeSeparator11, "mousecat");

        uiBeginColumns();

        uiSetUnlockLabelY(60u);
        makeUnsealButton(8u, "Mousecat", CatType::Mouse);
        ImGui::Separator();

        const float currentReward = pt->psvPPMouseCatGlobalBonusMult.currentValue();
        const float nextReward    = pt->psvPPMouseCatGlobalBonusMult.nextValue();

        if (!pt->psvPPMouseCatGlobalBonusMult.isMaxedOut())
        {
            std::sprintf(uiState.uiTooltipBuffer,
                         "Increase the global click reward value multiplier from x%.2f to x%.2f.",
                         static_cast<double>(currentReward),
                         static_cast<double>(nextReward));
        }
        else
        {
            std::sprintf(uiState.uiTooltipBuffer, "Increase the global click reward value multiplier (MAX).");
        }

        std::sprintf(uiState.uiLabelBuffer, "x%.2f", static_cast<double>(currentReward));
        makePrestigePurchasablePPButtonPSV("Global click mult", pt->psvPPMouseCatGlobalBonusMult);
    }

    if (checkUiUnlock(61u, pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Engi)]))
    {
        uiImgsep2(txrPrestigeSeparator12, "engicat");

        uiBeginColumns();

        uiSetUnlockLabelY(61u);
        makeUnsealButton(16u, "Engicat", CatType::Engi);
        ImGui::Separator();

        const float currentReward = pt->psvPPEngiCatGlobalBonusMult.currentValue();
        const float nextReward    = pt->psvPPEngiCatGlobalBonusMult.nextValue();

        if (!pt->psvPPEngiCatGlobalBonusMult.isMaxedOut())
        {
            std::sprintf(uiState.uiTooltipBuffer,
                         "Increase the global cat reward value multiplierfrom x%.2f to x%.2f.",
                         static_cast<double>(currentReward),
                         static_cast<double>(nextReward));
        }
        else
        {
            std::sprintf(uiState.uiTooltipBuffer, "Increase the global cat reward value multiplier (MAX).");
        }

        std::sprintf(uiState.uiLabelBuffer, "x%.2f", static_cast<double>(currentReward));
        makePrestigePurchasablePPButtonPSV("Global cat mult", pt->psvPPEngiCatGlobalBonusMult);
    }

    if (checkUiUnlock(62u, pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Repulso)]))
    {
        uiImgsep2(txrPrestigeSeparator13, "repulsocat");

        uiBeginColumns();

        uiSetUnlockLabelY(62u);
        makeUnsealButton(128u, "Repulsocat", CatType::Repulso);
        ImGui::Separator();

        std::sprintf(uiState.uiTooltipBuffer,
                     "The Repulsocat cordially asks their fan to filter repelled bubbles by type.");
        uiState.uiLabelBuffer[0] = '\0';
        (void)makePurchasablePPButtonOneTime("Repulsion filter", 16u, pt->perm.repulsoCatFilterPurchased);

        if (checkUiUnlock(63u, pt->perm.repulsoCatFilterPurchased))
        {
            ImGui::Columns(1);
            uiSetFontScale(uiSubBulletFontScale);

            uiSetUnlockLabelY(63u);
            ImGui::Text("  ignore: ");
            ImGui::SameLine();

            auto& [ignoreNormal, ignoreStar, ignoreBomb] = pt->repulsoCatIgnoreBubbles;

            uiCheckbox("normal##repulso", &ignoreNormal);
            ImGui::SameLine();

            uiCheckbox("star##repulso", &ignoreStar);
            ImGui::SameLine();

            uiCheckbox("bombs##repulso", &ignoreBomb);

            uiSetFontScale(uiNormalFontScale);
            uiBeginColumns();
        }

        std::sprintf(uiState.uiTooltipBuffer,
                     "The Repulsocat coats the fan blades with star powder, giving it a chance to convert "
                     "repelled bubbles to star bubbles.");
        uiState.uiLabelBuffer[0] = '\0';
        (void)makePurchasablePPButtonOneTime("Conversion field", 32u, pt->perm.repulsoCatConverterPurchased);

        if (checkUiUnlock(64u, pt->perm.repulsoCatConverterPurchased))
        {
            uiSetFontScale(uiSubBulletFontScale);
            uiSetUnlockLabelY(64u);
            uiCheckbox("enable ##repulsoconv", &pt->repulsoCatConverterEnabled);
            uiSetFontScale(uiNormalFontScale);
            ImGui::NextColumn();
            ImGui::NextColumn();

            const float currentChance = pt->psvPPRepulsoCatConverterChance.currentValue();
            const float nextChance    = pt->psvPPRepulsoCatConverterChance.nextValue();

            if (!pt->psvPPRepulsoCatConverterChance.isMaxedOut())
            {
                std::sprintf(uiState.uiTooltipBuffer,
                             "Increase the repelled bubble conversion chance from %.2f%% to %.2f%%.",
                             static_cast<double>(currentChance),
                             static_cast<double>(nextChance));
            }
            else
            {
                std::sprintf(uiState.uiTooltipBuffer, "Increase the repelled bubble conversion chance (MAX).");
            }
            std::sprintf(uiState.uiLabelBuffer, "%.2f%%", static_cast<double>(currentChance));
            makePrestigePurchasablePPButtonPSV("Conversion chance", pt->psvPPRepulsoCatConverterChance);

            std::sprintf(uiState.uiTooltipBuffer, "Bubbles are converted into nova bubbles instead of star bubbles.");
            uiState.uiLabelBuffer[0] = '\0';
            (void)makePurchasablePPButtonOneTime("Nova conversion", 96u, pt->perm.repulsoCatNovaConverterPurchased);
        }
    }

    if (checkUiUnlock(65u, pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Attracto)]))
    {
        uiImgsep2(txrPrestigeSeparator14, "attractocat");

        uiBeginColumns();

        uiSetUnlockLabelY(65u);
        makeUnsealButton(256u, "Attractocat", CatType::Attracto);
        ImGui::Separator();

        std::sprintf(uiState.uiTooltipBuffer,
                     "The Attractocat does some quantum science stuff to its magnet to allow filtering of "
                     "attracted bubbles by type.");
        uiState.uiLabelBuffer[0] = '\0';
        (void)makePurchasablePPButtonOneTime("Attraction filter", 96u, pt->perm.attractoCatFilterPurchased);

        if (checkUiUnlock(66u, pt->perm.attractoCatFilterPurchased))
        {
            ImGui::Columns(1);
            uiSetFontScale(uiSubBulletFontScale);

            uiSetUnlockLabelY(66u);
            ImGui::Text("  ignore: ");
            ImGui::SameLine();

            auto& [ignoreNormal, ignoreStar, ignoreBomb] = pt->attractoCatIgnoreBubbles;

            uiCheckbox("normal##attracto", &ignoreNormal);
            ImGui::SameLine();

            uiCheckbox("star##attracto", &ignoreStar);
            ImGui::SameLine();

            uiCheckbox("bombs##attracto", &ignoreBomb);

            uiSetFontScale(uiNormalFontScale);
            uiBeginColumns();
        }
    }

    if (checkUiUnlock(67u, pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Copy)]))
    {
        uiImgsep2(txrPrestigeSeparator15, "copycat");

        uiBeginColumns();

        uiSetUnlockLabelY(67u);
        makeUnsealButton(512u, "Copycat", CatType::Copy);
        ImGui::Separator();

        // TODO P1: something?
    }

    uiState.uiButtonHueMod = 0.f;

    ImGui::Columns(1);
}
