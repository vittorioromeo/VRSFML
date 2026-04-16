#include "BubbleIdleMain.hpp"
#include "Cat.hpp"
#include "CatConstants.hpp"
#include "CatType.hpp"
#include "Version.hpp"

#include "SFML/ImGui/IncludeImGui.hpp"

#include <cstdio>

void Main::uiShopCooldownButton(const char* label, const CatType catType, const char* additionalInfo)
{
    auto& psv = pt->psvCooldownMultsPerCatType[asIdx(catType)];

    const float currentCooldown = CatConstants::baseCooldowns[asIdx(catType)] * psv.currentValue();
    const float nextCooldown    = CatConstants::baseCooldowns[asIdx(catType)] * psv.nextValue();

    if (!psv.isMaxedOut())
    {
        uiSetTooltip(
            "Decrease cooldown from %.2fs to %.2fs.%s\n\n(Note: can be reverted by right-clicking, "
            "but no refunds!)",
            static_cast<double>(currentCooldown / 1000.f),
            static_cast<double>(nextCooldown / 1000.f),
            additionalInfo);
    }
    else
    {
        uiSetTooltip("Decrease cooldown (MAX).%s\n\n(Note: can be reverted by right-clicking, but no refunds!)",
                     additionalInfo);
    }

    uiSetLabel("%.2fs", static_cast<double>(pt->getComputedCooldownByCatType(catType) / 1000.f));

    makePSVButton(label, psv);

    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && ImGui::IsMouseClicked(ImGuiMouseButton_Right) &&
        psv.nPurchases > 0u)
    {
        --psv.nPurchases;
        playSound(sounds.buy);
    }
}

void Main::uiShopRangeButton(const char* label, const CatType catType, const char* additionalInfo)
{
    auto& psv = pt->psvRangeDivsPerCatType[asIdx(catType)];

    const float currentRange = CatConstants::baseRanges[asIdx(catType)] / psv.currentValue();
    const float nextRange    = CatConstants::baseRanges[asIdx(catType)] / psv.nextValue();

    if (!psv.isMaxedOut())
    {
        uiSetTooltip(
            "Increase range from %.2fpx to %.2fpx.%s\n\n(Note: can be reverted by right-clicking, but "
            "no refunds!)",
            static_cast<double>(currentRange),
            static_cast<double>(nextRange),
            additionalInfo);
    }
    else
    {
        uiSetTooltip("Increase range (MAX).%s\n\n(Note: can be reverted by right-clicking, but no refunds!)",
                     additionalInfo);
    }

    uiSetLabel("%.2fpx", static_cast<double>(pt->getComputedRangeByCatType(catType)));
    makePSVButton(label, psv);

    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && ImGui::IsMouseClicked(ImGuiMouseButton_Right) &&
        psv.nPurchases > 0u)
    {
        --psv.nPurchases;
        playSound(sounds.buy);
    }
}

void Main::uiShopDrawSpecialCats()
{
    const auto nCatNormal = pt->getCatCountByType(CatType::Normal);
    const auto nCatUni    = pt->getCatCountByType(CatType::Uni);
    const auto nCatDevil  = pt->getCatCountByType(CatType::Devil);
    const auto nCatAstro  = pt->getCatCountByType(CatType::Astro);

    const bool prestigedBefore    = pt->psvBubbleValue.nPurchases > 0u;
    const bool catUnicornUnlocked = pt->psvBubbleCount.nPurchases > 0 && nCatNormal >= 3 &&
                                    (prestigedBefore || pt->anyCatEverWokenFromNap);
    const bool catUnicornUpgradesUnlocked = catUnicornUnlocked && nCatUni >= 2 && nCatDevil >= 1;
    if (checkUiUnlock(6u, catUnicornUnlocked))
    {
        uiImgsep(txrMenuSeparator4, "unicats");

        uiSetUnlockLabelY(6u);
        uiSetTooltip(
            "Unicats transform bubbles into star bubbles, which are worth x15 more!\n\nHave "
            "your cats pop them for you, or pop them towards the end of a combo for huge rewards!");
        uiSetLabel("%zu unicats", nCatUni);
        if (makePSVButton("Unicat", pt->psvPerCatType[asIdx(CatType::Uni)]))
        {
            spawnCatCentered(CatType::Uni, getHueByCatType(CatType::Uni), /* placeInHand */ !onSteamDeck);

            if (nCatUni == 0)
                doTip("Unicats transform bubbles in star bubbles,\nworth x15! Pop them at the end of a combo!");
        }

        if (checkUiUnlock(7u, catUnicornUpgradesUnlocked))
        {
            uiSetUnlockLabelY(7u);
            uiShopCooldownButton("  cooldown##Uni", CatType::Uni);

            if (pt->perm.unicatTranscendencePurchased && pt->perm.unicatTranscendenceAOEPurchased)
                uiShopRangeButton("  range##Uni", CatType::Uni);
        }
    }

    const bool catDevilUnlocked         = pt->psvBubbleValue.nPurchases > 0 && nCatNormal >= 6 && nCatUni >= 4 &&
                                          pt->nShrinesCompleted >= 1;
    const bool catDevilUpgradesUnlocked = catDevilUnlocked && nCatDevil >= 2 && nCatAstro >= 1;
    if (checkUiUnlock(8u, catDevilUnlocked))
    {
        uiImgsep(txrMenuSeparator5, "devilcats");

        uiSetUnlockLabelY(8u);
        uiSetTooltip(
            "Devilcats transform bubbles into bombs that explode when popped. Bubbles affected by the "
            "explosion are worth x10 more! Bomb explosion range can be upgraded.");
        uiSetLabel("%zu devilcats", nCatDevil);
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
            uiSetTooltip("Increase bomb explosion radius from x%.2f to x%.2f.",
                         static_cast<double>(currentExplosionRadius),
                         static_cast<double>(nextExplosionRadius));
            uiSetLabel("x%.2f", static_cast<double>(currentExplosionRadius));
            makePSVButton("  Explosion radius", pt->psvExplosionRadiusMult);
        }

        if (checkUiUnlock(10u, catDevilUpgradesUnlocked))
        {
            uiSetUnlockLabelY(10u);
            uiShopCooldownButton("  cooldown##Devil", CatType::Devil);

            if (pt->perm.devilcatHellsingedPurchased)
                uiShopRangeButton("  range##Devil", CatType::Devil);
        }
    }

    const bool astroCatUnlocked = nCatNormal >= 10 && nCatUni >= 5 && nCatDevil >= 2 && pt->nShrinesCompleted >= 2;
    const bool astroCatUpgradesUnlocked = astroCatUnlocked && nCatDevil >= 9 && nCatAstro >= 5;
    if (checkUiUnlock(11u, astroCatUnlocked))
    {
        uiImgsep(txrMenuSeparator6, "astrocats");

        uiSetUnlockLabelY(11u);
        uiSetTooltip(
            "Astrocats periodically fly across the map, popping bubbles they hit with a huge x20 "
            "money multiplier!\n\nThey can be permanently upgraded with prestige points to inspire cats "
            "watching them fly past to pop bubbles faster.");
        uiSetLabel("%zu astrocats", nCatAstro);
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
            uiShopCooldownButton("  cooldown##Astro", CatType::Astro);
            uiShopRangeButton("  range##Astro", CatType::Astro);
        }
    }
}

void Main::uiShopDrawUniqueCatBonuses()
{
    Cat* catWitch    = getWitchCat();
    Cat* catWizard   = getWizardCat();
    Cat* catMouse    = getMouseCat();
    Cat* catEngi     = getEngiCat();
    Cat* catRepulso  = getRepulsoCat();
    Cat* catAttracto = getAttractoCat();

    const bool anyUniqueCat = catWitch != nullptr || catWizard != nullptr || catMouse != nullptr ||
                              catEngi != nullptr || catRepulso != nullptr || catAttracto != nullptr;

    if (!anyUniqueCat)
        return;

    uiImgsep(txrMenuSeparator7, "unique cats");

    if (checkUiUnlock(13u, catWitch != nullptr))
    {
        uiSetUnlockLabelY(13u);
        uiShopCooldownButton("  witchcat cooldown",
                             CatType::Witch,
                             "\n\nEffectively increases the frequency of rituals.");

        if (checkUiUnlock(14u, pt->perm.witchCatBuffPowerScalesWithNCats))
        {
            uiSetUnlockLabelY(14u);
            uiShopRangeButton("  witchcat range",
                              CatType::Witch,
                              "\n\nAllows more cats to participate in group rituals, increasing the duration of "
                              "buffs.");
        }
    }

    if (checkUiUnlock(15u, catWizard != nullptr))
    {
        ImGui::Separator();

        uiSetUnlockLabelY(15u);
        uiShopCooldownButton("  wizardcat cooldown",
                             CatType::Wizard,
                             "\n\nDoes *not* increase mana generation rate, but increases star bubble absorption "
                             "rate and decreases cooldown between spell casts.");

        uiShopRangeButton("  wizardcat range",
                          CatType::Wizard,
                          "\n\nEffectively increases the area of effect of most spells, and star bubble "
                          "absorption range.");
    }

    if (checkUiUnlock(16u, catMouse != nullptr))
    {
        ImGui::Separator();

        uiSetUnlockLabelY(16u);
        uiShopCooldownButton("  mousecat cooldown##Mouse", CatType::Mouse);
        uiShopRangeButton("  mousecat range##Mouse", CatType::Mouse);
    }

    if (checkUiUnlock(17u, catEngi != nullptr))
    {
        ImGui::Separator();

        uiSetUnlockLabelY(17u);
        uiShopCooldownButton("  engicat cooldown",
                             CatType::Engi,
                             "\n\nEffectively increases the frequency of maintenances.");

        uiShopRangeButton("  engicat range",
                          CatType::Engi,
                          "\n\nAllows more cats to be boosted by maintenance at once.");
    }

    if (checkUiUnlock(18u, catRepulso != nullptr))
    {
        ImGui::Separator();

        uiSetUnlockLabelY(18u);
        uiShopRangeButton("  repulsocat range##Repulso", CatType::Repulso);
    }

    if (checkUiUnlock(19u, catAttracto != nullptr))
    {
        ImGui::Separator();

        uiSetUnlockLabelY(19u);
        uiShopRangeButton("  attractocat range##Attracto", CatType::Attracto);
    }
}
