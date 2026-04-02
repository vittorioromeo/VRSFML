

#include "Aliases.hpp"
#include "BubbleIdleMain.hpp"
#include "Cat.hpp"
#include "Countdown.hpp"
#include "PurchasableScalingValue.hpp"

#include "SFML/ImGui/IncludeImGui.hpp"

#include <cstdio>

void Main::uiTabBarMagic()
{
    uiSetFontScale(uiNormalFontScale);

    Cat* wizardCat = getWizardCat();

    if (wizardCat == nullptr)
    {
        ImGui::Text("The wizardcat is missing!");
        return;
    }

    Cat* copyCat = getCopyCat();

    ImGui::Spacing();
    ImGui::Spacing();

    uiImgsep(txrMagicSeparator0, "wisdom", /* first */ true);
    ImGui::Columns(1);

    ImGui::Text("Wisdom points: %s WP", toStringWithSeparators(pt->wisdom));

    uiCheckbox("Absorb wisdom from star bubbles", &pt->absorbingWisdom);
    std::sprintf(uiState.uiTooltipBuffer,
                 "The Wizardcat concentrates, absorbing wisdom points from nearby star bubbles. While the "
                 "Wizardcat is concentrating, it cannot cast spells nor be moved around.");
    uiMakeTooltip();

    uiBeginColumns();
    uiState.uiButtonHueMod = 45.f;

    std::sprintf(uiState.uiTooltipBuffer,
                 "The Wizardcat taps into memories of past lives, remembering a powerful spell.\n\nMana "
                 "costs:\n- "
                 "1st spell: 5 mana\n- 2nd spell: 20 mana\n- 3rd spell: 30 mana\n- 4th spell: 40 mana\n\nNote: "
                 "You "
                 "won't be able to cast a spell if the cost exceeds your maximum mana!");
    std::sprintf(uiState.uiLabelBuffer, "%zu/%zu", pt->psvSpellCount.nPurchases, pt->psvSpellCount.data->nMaxPurchases);
    (void)makePSVButtonExByCurrency("Remember spell",
                                    pt->psvSpellCount,
                                    1u,
                                    static_cast<MoneyType>(pt->psvSpellCount.nextCost()),
                                    pt->wisdom,
                                    "%s WP##%u");

    uiState.uiButtonHueMod = 0.f;

    uiImgsep(txrMagicSeparator1, "mana");
    ImGui::Columns(1);

    ImGui::Text("Mana: %llu / %llu", pt->mana, pt->getComputedMaxMana());

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Next mana:");

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(157, 0, 255, 128));
    ImGui::ProgressBar(pt->manaTimer / pt->getComputedManaCooldown());
    ImGui::PopStyleColor();

    ImGui::Text("Wizard cooldown: %.2fs", static_cast<double>(wizardCat->cooldown.value / 1000.f));

    uiImgsep(txrMagicSeparator2, "spells");

    if (pt->psvSpellCount.nPurchases == 0)
        ImGui::Text("No spells revealed yet...");

    ImGui::Columns(1);
    uiSetFontScale(0.8f);

    if (pt->psvSpellCount.nPurchases > 0)
    {
        if (pt->absorbingWisdom)
            uiCenteredText("Cannot cast spells while absorbing wisdom...");
        else if (wizardCat->isHexedOrCopyHexed())
            uiCenteredText("Cannot cast spells while hexed...");
        else if (wizardCat->cooldown.value > 0.f)
            uiCenteredText("Cannot cast spells while on cooldown...");
        else if (isCatBeingDragged(*wizardCat))
            uiCenteredText("Cannot cast spells while being dragged...");
        else
        {
            const bool anySpellCastable = pt->mana >= spellManaCostByIndex[0] && pt->psvSpellCount.nPurchases >= 1;

            if (anySpellCastable)
                uiCenteredText("Ready to cast a spell!");
            else
                uiCenteredText("Not enough mana to cast any spell...");
        }
    }

    ImGui::Separator();
    uiSetFontScale(uiNormalFontScale);

    uiBeginColumns();
    uiState.uiButtonHueMod = 45.f;

    //
    // SPELL 0
    if (checkUiUnlock(32u, pt->psvSpellCount.nPurchases >= 1))
    {
        uiSetUnlockLabelY(32u);
        std::sprintf(uiState.uiTooltipBuffer,
                     "Transforms a percentage of bubbles around the Wizardcat into star bubbles "
                     "immediately.\n\nCan be upgraded to ignore bombs with prestige points.");
        uiState.uiLabelBuffer[0] = '\0';
        bool done                = false;

        ImGui::BeginDisabled(isWizardBusy());
        if (makePurchasableButtonOneTimeByCurrency("Starpaw Conversion",
                                                   done,
                                                   spellManaCostByIndex[0],
                                                   pt->mana,
                                                   "%s mana##%u"))
        {
            castSpellByIndex(0u, wizardCat, copyCat);
            done = false;
        }
        ImGui::EndDisabled();

        const float currentPercentage = pt->psvStarpawPercentage.currentValue();
        const float nextPercentage    = pt->psvStarpawPercentage.nextValue();

        if (!pt->psvStarpawPercentage.isMaxedOut())
        {
            std::sprintf(uiState.uiTooltipBuffer,
                         "Increase the percentage of bubbles converted into star bubbles from %.2f%% to %.2f%%.",
                         static_cast<double>(currentPercentage),
                         static_cast<double>(nextPercentage));
        }
        else
        {
            std::sprintf(uiState.uiTooltipBuffer,
                         "Increase the percentage of bubbles converted into star bubbles (MAX).");
        }
        std::sprintf(uiState.uiLabelBuffer, "%.2f%%", static_cast<double>(currentPercentage));
        (void)makePSVButtonExByCurrency("  higher percentage##starpawperc",
                                        pt->psvStarpawPercentage,
                                        1u,
                                        static_cast<MoneyType>(pt->psvStarpawPercentage.nextCost()),
                                        pt->wisdom,
                                        "%s WP##%u");
    }

    //
    // SPELL 1
    if (checkUiUnlock(33u, pt->psvSpellCount.nPurchases >= 2))
    {
        ImGui::Separator();

        uiSetUnlockLabelY(33u);
        std::sprintf(uiState.uiTooltipBuffer,
                     "Creates a value multiplier aura around the Wizardcat that affects all cats and bubbles. "
                     "Lasts %d seconds.\n\nCasting this spell multiple times will accumulate the aura "
                     "duration.",
                     pt->perm.wizardCatDoubleMewltiplierDuration ? 12 : 6);
        std::sprintf(uiState.uiLabelBuffer, "%.2fs", static_cast<double>(pt->mewltiplierAuraTimer / 1000.f));
        bool done = false;

        ImGui::BeginDisabled(isWizardBusy());
        if (makePurchasableButtonOneTimeByCurrency("Mewltiplier Aura",
                                                   done,
                                                   spellManaCostByIndex[1],
                                                   pt->mana,
                                                   "%s mana##%u"))
        {
            castSpellByIndex(1u, wizardCat, copyCat);
            done = false;
        }
        ImGui::EndDisabled();

        const float currentMultiplier = pt->psvMewltiplierMult.currentValue();
        const float nextMultiplier    = pt->psvMewltiplierMult.nextValue();

        std::sprintf(uiState.uiTooltipBuffer,
                     "Increase the multiplier applied while the aura is active from x%.2f to x%.2f.",
                     static_cast<double>(currentMultiplier),
                     static_cast<double>(nextMultiplier));
        std::sprintf(uiState.uiLabelBuffer, "x%.2f", static_cast<double>(currentMultiplier));
        (void)makePSVButtonExByCurrency("  higher multiplier",
                                        pt->psvMewltiplierMult,
                                        1u,
                                        static_cast<MoneyType>(pt->psvMewltiplierMult.nextCost()),
                                        pt->wisdom,
                                        "%s WP##%u");
    }

    //
    // SPELL 2
    if (checkUiUnlock(34u, pt->psvSpellCount.nPurchases >= 3))
    {
        ImGui::Separator();

        uiSetUnlockLabelY(34u);
        std::sprintf(uiState.uiTooltipBuffer,
                     "The Wizardcat uses their magic to empower a nearby Witchcat, reducing their remaining "
                     "ritual cooldown.\n\nNote: This spell has no effect if there is no Witchcat "
                     "nearby, or if there are voodoo dolls left to collect.");
        uiState.uiLabelBuffer[0] = '\0';

        bool done = false;

        ImGui::BeginDisabled(isWizardBusy());
        if (makePurchasableButtonOneTimeByCurrency("Dark Union", done, spellManaCostByIndex[2], pt->mana, "%s mana##%u"))
        {
            castSpellByIndex(2u, wizardCat, copyCat);
            done = false;
        }
        ImGui::EndDisabled();

        const float currentPercentage = pt->psvDarkUnionPercentage.currentValue();
        const float nextPercentage    = pt->psvDarkUnionPercentage.nextValue();

        if (!pt->psvDarkUnionPercentage.isMaxedOut())
        {
            std::sprintf(uiState.uiTooltipBuffer,
                         "Increase the cooldown reduction percentage from %.2f%% to %.2f%%.",
                         static_cast<double>(currentPercentage),
                         static_cast<double>(nextPercentage));
        }
        else
        {
            std::sprintf(uiState.uiTooltipBuffer, "Increase the cooldown reduction percentage (MAX).");
        }
        std::sprintf(uiState.uiLabelBuffer, "%.2f%%", static_cast<double>(currentPercentage));
        (void)makePSVButtonExByCurrency("  higher reduction##darkunionperc",
                                        pt->psvDarkUnionPercentage,
                                        1u,
                                        static_cast<MoneyType>(pt->psvDarkUnionPercentage.nextCost()),
                                        pt->wisdom,
                                        "%s WP##%u");
    }

    //
    // SPELL 3
    if (checkUiUnlock(35u, pt->psvSpellCount.nPurchases >= 4))
    {
        ImGui::Separator();

        uiSetUnlockLabelY(35u);
        std::sprintf(uiState.uiTooltipBuffer,
                     "The Wizardcat controls time itself, creating a stasis field for %d seconds. All bubbles "
                     "caught in the field become frozen in time, unable to move or be destroyed. However, they "
                     "can "
                     "still be popped, as many times as you want!\n\nCasting this spell multiple times will "
                     "accumulate the field duration.\n\nNote: This spell has no effect if there are no bubbles "
                     "nearby. Bombs are also affected by the stasis field.",
                     pt->perm.wizardCatDoubleStasisFieldDuration ? 12 : 6);
        std::sprintf(uiState.uiLabelBuffer, "%.2fs", static_cast<double>(pt->stasisFieldTimer / 1000.f));

        bool done = false;

        ImGui::BeginDisabled(isWizardBusy());
        if (makePurchasableButtonOneTimeByCurrency("Stasis Field",
                                                   done,
                                                   spellManaCostByIndex[3],
                                                   pt->mana,
                                                   "%s mana##%u"))
        {
            castSpellByIndex(3u, wizardCat, copyCat);
            done = false;
        }
        ImGui::EndDisabled();
    }

    uiState.uiButtonHueMod = 0.f;
    ImGui::Columns(1);

    if (pt->psvSpellCount.nPurchases > 0 && pt->perm.autocastPurchased)
    {
        uiImgsep(txrMagicSeparator3, "autocast");

        ImGui::Columns(1);
        uiState.uiButtonHueMod = 45.f;

        constexpr const char* entries[]{
            "None",
            "Starpaw Conversion",
            "Mewltiplier Aura",
            "Dark Union",
            "Stasis Field",
        };

        if (ImGui::BeginCombo("Spell##autocastspell", entries[pt->perm.autocastIndex]))
        {
            for (SizeT i = 0u; i < pt->psvSpellCount.nPurchases + 1; ++i)
            {
                const bool isSelected = pt->perm.autocastIndex == i;
                if (ImGui::Selectable(entries[i], isSelected))
                {
                    pt->perm.autocastIndex = i;
                    playSound(sounds.uitab);
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        uiState.uiButtonHueMod = 0.f;
    }
}
