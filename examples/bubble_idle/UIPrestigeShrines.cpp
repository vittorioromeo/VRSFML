#include "BubbleIdleMain.hpp"

#include "CatType.hpp"

#include "SFML/ImGui/IncludeImGui.hpp"

#include <cstdio>

void Main::uiPrestigeUnsealButton(const PrestigePointsType ppCost, const char* catName, const CatType type)
{
    if (!pt->perm.shrineCompletedOnceByCatType[asIdx(type)])
        return;

    uiSetTooltipOnly("Permanently release the %s from their shrine. They will be waiting for you right outside "
                     "when the shrine is activated.\n\n(Note: completing the shrine will now grant 1.5x the money "
                     "it absorbed.)",
                     catName);

    char buf[256];
    std::snprintf(buf, sizeof(buf), "%s##%s", "Break the seal", catName);

    (void)makePurchasablePPButtonOneTime(buf, ppCost, pt->perm.unsealedByType[asIdx(type)]);
}

void Main::uiPrestigeDrawShrineCatUpgrades()
{
    if (checkUiUnlock(58u, pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Witch)]))
    {
        uiImgsep2(txrPrestigeSeparator9, "witchcat");

        uiBeginColumns();

        uiSetUnlockLabelY(58u);
        uiPrestigeUnsealButton(4u, "Witchcat", CatType::Witch);
        ImGui::Separator();

        const float currentDuration = pt->psvPPWitchCatBuffDuration.currentValue();
        const float nextDuration    = pt->psvPPWitchCatBuffDuration.nextValue();

        uiMakePrestigePsvButtonValue("Buff duration",
                                     pt->psvPPWitchCatBuffDuration,
                                     "%.2fs",
                                     static_cast<double>(currentDuration),
                                     "Increase the base duration of Witchcat buffs from %.2fs to %.2fs.",
                                     static_cast<double>(currentDuration),
                                     static_cast<double>(nextDuration),
                                     "Increase the base duration of Witchcat buffs (MAX).");

        ImGui::Separator();

        (void)uiMakePrestigeOneTimeButton("Group ritual",
                                          4u,
                                          pt->perm.witchCatBuffPowerScalesWithNCats,
                                          "The duration of Witchcat buffs scales with the number of cats in range of the ritual.");

        (void)uiMakePrestigeOneTimeButton("Worldwide cult",
                                          4u,
                                          pt->perm.witchCatBuffPowerScalesWithMapSize,
                                          "The duration of Witchcat buffs scales with the size of the explored map.");

        ImGui::Separator();

        (void)uiMakePrestigeOneTimeButton("Material shortage",
                                          8u,
                                          pt->perm.witchCatBuffFewerDolls,
                                          "Half as many voodoo dolls will appear per ritual.");

        ImGui::Separator();

        (void)uiMakePrestigeOneTimeButton("Flammable dolls",
                                          8u,
                                          pt->perm.witchCatBuffFlammableDolls,
                                          "Dolls are automatically collected by Devilcat bomb explosions.");

        (void)uiMakePrestigeOneTimeButton("Orbital dolls",
                                          16u,
                                          pt->perm.witchCatBuffOrbitalDolls,
                                          "Dolls are automatically collected by Astrocats during their flyby.");

        ImGui::Separator();

        const float currentUniPercentage = pt->psvPPUniRitualBuffPercentage.currentValue();
        const float nextUniPercentage    = pt->psvPPUniRitualBuffPercentage.nextValue();

        uiMakePrestigePsvButtonValue("Star Spawn %",
                                     pt->psvPPUniRitualBuffPercentage,
                                     "%.2f%%",
                                     static_cast<double>(currentUniPercentage),
                                     "Increase the star bubble spawn chance during the Unicat vododoo ritual buff from %.2f%% to %.2f%%.",
                                     static_cast<double>(currentUniPercentage),
                                     static_cast<double>(nextUniPercentage),
                                     "Increase the star bubble spawn chance during the Unicat vododoo ritual buff (MAX).");

        const float currentDevilPercentage = pt->psvPPDevilRitualBuffPercentage.currentValue();
        const float nextDevilPercentage    = pt->psvPPDevilRitualBuffPercentage.nextValue();

        uiMakePrestigePsvButtonValue("Bomb Spawn %",
                                     pt->psvPPDevilRitualBuffPercentage,
                                     "%.2f%%",
                                     static_cast<double>(currentDevilPercentage),
                                     "Increase the bomb spawn chance during the Devil vododoo ritual buff from %.2f%% to %.2f%%.",
                                     static_cast<double>(currentDevilPercentage),
                                     static_cast<double>(nextDevilPercentage),
                                     "Increase the bomb spawn chance during the Devil vododoo ritual buff (MAX).");
    }

    if (checkUiUnlock(59u, pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Wizard)]))
    {
        uiImgsep2(txrPrestigeSeparator10, "wizardcat");

        uiBeginColumns();

        uiSetUnlockLabelY(59u);
        uiPrestigeUnsealButton(8u, "Wizardcat", CatType::Wizard);
        ImGui::Separator();

        const float currentManaCooldown = pt->getComputedManaCooldown();
        const float nextManaCooldown    = pt->getComputedManaCooldownNext();

        uiMakePrestigePsvButtonValue("Mana cooldown",
                                     pt->psvPPManaCooldownMult,
                                     "%.2fs",
                                     static_cast<double>(currentManaCooldown / 1000.f),
                                     "Decrease mana generation cooldown from %.2fs to %.2fs.",
                                     static_cast<double>(currentManaCooldown / 1000.f),
                                     static_cast<double>(nextManaCooldown / 1000.f),
                                     "Decrease mana generation cooldown (MAX).");

        const ManaType currentMaxMana = pt->getComputedMaxMana();
        const ManaType nextMaxMana    = pt->getComputedMaxManaNext();

        uiMakePrestigePsvButtonValue("Mana limit",
                                     pt->psvPPManaMaxMult,
                                     "%llu mana",
                                     currentMaxMana,
                                     "Increase the maximum mana from %llu to %llu.",
                                     currentMaxMana,
                                     nextMaxMana,
                                     "Increase the maximum mana (MAX).");

        ImGui::Separator();

        (void)uiMakePrestigeOneTimeButton("Autocast",
                                          4u,
                                          pt->perm.autocastPurchased,
                                          "Allow the Wizardcat to automatically cast spells when enough mana is available. Can be enabled and configured from the \"Magic\" tab.");

        ImGui::Separator();

        (void)uiMakePrestigeOneTimeButton("Selective starpaw",
                                          4u,
                                          pt->perm.starpawConversionIgnoreBombs,
                                          "Starpaw conversion ignores bombs, transforming only normal bubbles around the wizard into star bubbles.");

        if (pt->perm.unicatTranscendencePurchased && pt->perm.starpawConversionIgnoreBombs)
        {
            (void)uiMakePrestigeOneTimeButton("Nova starpaw",
                                              64u,
                                              pt->perm.starpawNova,
                                              "Starpaw conversion now turns all normal bubbles into nova bubbles.");
        }

        ImGui::Separator();

        (void)uiMakePrestigeOneTimeButton("Meeeeeewltiplier",
                                          64u,
                                          pt->perm.wizardCatDoubleMewltiplierDuration,
                                          "The duration of Mewltiplier Aura is extended from 6s to 12s.");

        ImGui::Separator();

        (void)uiMakePrestigeOneTimeButton("Pop Stuck In Time",
                                          256u,
                                          pt->perm.wizardCatDoubleStasisFieldDuration,
                                          "The duration of Stasis Field is extended from 6s to 12s.");
    }

    if (checkUiUnlock(60u, pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Mouse)]))
    {
        uiImgsep2(txrPrestigeSeparator11, "mousecat");

        uiBeginColumns();

        uiSetUnlockLabelY(60u);
        uiPrestigeUnsealButton(8u, "Mousecat", CatType::Mouse);
        ImGui::Separator();

        const float currentReward = pt->psvPPMouseCatGlobalBonusMult.currentValue();
        const float nextReward    = pt->psvPPMouseCatGlobalBonusMult.nextValue();

        uiMakePrestigePsvButtonValue("Global click mult",
                                     pt->psvPPMouseCatGlobalBonusMult,
                                     "x%.2f",
                                     static_cast<double>(currentReward),
                                     "Increase the global click reward value multiplier from x%.2f to x%.2f.",
                                     static_cast<double>(currentReward),
                                     static_cast<double>(nextReward),
                                     "Increase the global click reward value multiplier (MAX).");
    }

    if (checkUiUnlock(61u, pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Engi)]))
    {
        uiImgsep2(txrPrestigeSeparator12, "engicat");

        uiBeginColumns();

        uiSetUnlockLabelY(61u);
        uiPrestigeUnsealButton(16u, "Engicat", CatType::Engi);
        ImGui::Separator();

        const float currentReward = pt->psvPPEngiCatGlobalBonusMult.currentValue();
        const float nextReward    = pt->psvPPEngiCatGlobalBonusMult.nextValue();

        uiMakePrestigePsvButtonValue("Global cat mult",
                                     pt->psvPPEngiCatGlobalBonusMult,
                                     "x%.2f",
                                     static_cast<double>(currentReward),
                                     "Increase the global cat reward value multiplierfrom x%.2f to x%.2f.",
                                     static_cast<double>(currentReward),
                                     static_cast<double>(nextReward),
                                     "Increase the global cat reward value multiplier (MAX).");
    }

    if (checkUiUnlock(62u, pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Repulso)]))
    {
        uiImgsep2(txrPrestigeSeparator13, "repulsocat");

        uiBeginColumns();

        uiSetUnlockLabelY(62u);
        uiPrestigeUnsealButton(128u, "Repulsocat", CatType::Repulso);
        ImGui::Separator();

        (void)uiMakePrestigeOneTimeButton("Repulsion filter",
                                          16u,
                                          pt->perm.repulsoCatFilterPurchased,
                                          "The Repulsocat cordially asks their fan to filter repelled bubbles by type.");

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

        (void)uiMakePrestigeOneTimeButton("Conversion field",
                                          32u,
                                          pt->perm.repulsoCatConverterPurchased,
                                          "The Repulsocat coats the fan blades with star powder, giving it a chance to convert repelled bubbles to star bubbles.");

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

            uiMakePrestigePsvButtonValue("Conversion chance",
                                         pt->psvPPRepulsoCatConverterChance,
                                         "%.2f%%",
                                         static_cast<double>(currentChance),
                                         "Increase the repelled bubble conversion chance from %.2f%% to %.2f%%.",
                                         static_cast<double>(currentChance),
                                         static_cast<double>(nextChance),
                                         "Increase the repelled bubble conversion chance (MAX).");

            (void)uiMakePrestigeOneTimeButton("Nova conversion",
                                              96u,
                                              pt->perm.repulsoCatNovaConverterPurchased,
                                              "Bubbles are converted into nova bubbles instead of star bubbles.");
        }
    }

    if (checkUiUnlock(65u, pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Attracto)]))
    {
        uiImgsep2(txrPrestigeSeparator14, "attractocat");

        uiBeginColumns();

        uiSetUnlockLabelY(65u);
        uiPrestigeUnsealButton(256u, "Attractocat", CatType::Attracto);
        ImGui::Separator();

        (void)uiMakePrestigeOneTimeButton("Attraction filter",
                                          96u,
                                          pt->perm.attractoCatFilterPurchased,
                                          "The Attractocat does some quantum science stuff to its magnet to allow filtering of attracted bubbles by type.");

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
        uiPrestigeUnsealButton(512u, "Copycat", CatType::Copy);
        ImGui::Separator();

        // TODO P1: something?
    }
}
