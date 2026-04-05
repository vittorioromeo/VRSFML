#include "BubbleIdleMain.hpp"
#include "Cat.hpp"
#include "CatConstants.hpp"
#include "CatType.hpp"
#include "Countdown.hpp"
#include "Profile.hpp"
#include "PurchasableScalingValue.hpp"
#include "Serialization.hpp"
#include "Shrine.hpp"
#include "Version.hpp"

#include "SFML/ImGui/IncludeImGui.hpp"

#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/ToString.hpp"
#include "SFML/Base/Vector.hpp"

namespace
{
void drawSpriteAttachmentControls(const float uiScale, const char* label, GameConstants::SpriteAttachment& attachment)
{
    if (!ImGui::TreeNode(label))
        return;

    ImGui::SetNextItemWidth(280.f * uiScale);
    ImGui::InputFloat2("Offset", &attachment.positionOffset.x, "%.2f");

    ImGui::SetNextItemWidth(280.f * uiScale);
    ImGui::InputFloat2("Origin", &attachment.origin.x, "%.2f");

    ImGui::TreePop();
}
} // namespace

void Main::uiSettingsDrawDebugTab()
{
    if (ImGui::Button("Slide"))
    {
        fixedBgSlideTarget += 1.f;

        if (fixedBgSlideTarget >= 3.f)
            fixedBgSlideTarget = 0.f;
    }

    ImGui::Separator();

    constexpr sf::base::I64 iStep            = 1;
    static sf::base::I64    speedrunTimerSet = 0;

    ImGui::SetNextItemWidth(240.f * profile.uiScale);
    if (ImGui::InputScalar("Speedrun timer", ImGuiDataType_S64, &speedrunTimerSet, &iStep, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal))
        pt->speedrunStartTime.emplace(speedrunTimerSet);

    ImGui::Separator();

    if (ImGui::Button("Save game"))
    {
        ptMain.fullVersion = !isDemoVersion;
        savePlaythroughToFile(ptMain, "userdata/playthrough.json");
    }

    ImGui::SameLine();

    if (ImGui::Button("Load game"))
        loadPlaythroughFromFileAndReseed();

    ImGui::SameLine();

    uiState.uiButtonHueMod = 120.f;
    uiPushButtonColors();

    if (ImGui::Button("Reset game"))
        forceResetGame();

    ImGui::SameLine();

    if (ImGui::Button("Reset profile"))
        forceResetProfile();

    uiPopButtonColors();
    uiState.uiButtonHueMod = 0.f;

    ImGui::Separator();

    static int catTypeN = 0;
    ImGui::SetNextItemWidth(320.f * profile.uiScale);
    ImGui::Combo("typeN", &catTypeN, CatConstants::typeNames, nCatTypes);

    if (ImGui::Button("Spawn"))
    {
        const auto catType = static_cast<CatType>(catTypeN);

        if (isUniqueCatType(catType))
        {
            const auto pos = gameView.screenToWorld(getResolution() / 2.f, window.getSize().toVec2f());
            spawnSpecialCat(pos, catType);
        }
        else
        {
            spawnCatCentered(catType, getHueByCatType(catType));
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Do Ritual"))
        if (auto* wc = getWitchCat())
            wc->cooldown.value = 10.f;

    ImGui::SameLine();

    if (ImGui::Button("Ritual"))
        if (auto* wc = getWitchCat())
            wc->cooldown.value = 12000.f;

    ImGui::SameLine();

    if (ImGui::Button("Do Copy Ritual"))
        if (auto* wc = getCopyCat())
            wc->cooldown.value = 10.f;

    ImGui::SameLine();

    if (ImGui::Button("Copy Ritual"))
        if (auto* wc = getCopyCat())
            wc->cooldown.value = 12000.f;

    ImGui::SameLine();

    if (ImGui::Button("Do Letter"))
    {
        victoryTC.emplace(TargetedCountdown{.startingValue = 6500.f});
        victoryTC->restart();
        delayedActions.emplaceBack(Countdown{.value = 7000.f}, [this] { playSound(sounds.letterchime); });
    }

    if (ImGui::Button("Do Tip"))
        doTip("Hello, I am a tip!\nHello world... How are you doing today?\nTest test test");

    ImGui::SameLine();

    if (ImGui::Button("Do Arrow"))
        uiState.scrollArrowCountdown.value = 2000.f;

    ImGui::SameLine();

    if (ImGui::Button("Do Prestige"))
    {
        ++pt->psvBubbleValue.nPurchases;
        const auto ppReward = pt->calculatePrestigePointReward(1u);
        beginPrestigeTransition(ppReward);
    }

    ImGui::Separator();

    ImGui::Checkbox("hide ui", &uiState.debugHideUI);

    ImGui::Separator();

    ImGui::PushFont(fontImGuiMouldyCheese);
    uiSetFontScale(uiToolTipFontScale);

    SizeT step    = 1u;
    SizeT counter = 0u;

    static char filenameBuf[128] = "userdata/custom.json";

    ImGui::SetNextItemWidth(320.f * profile.uiScale);
    ImGui::InputText("##Filename", filenameBuf, sizeof(filenameBuf));

    if (ImGui::Button("Custom save"))
    {
        pt->fullVersion = !isDemoVersion;
        savePlaythroughToFile(*pt, filenameBuf);
    }

    ImGui::SameLine();

    if (ImGui::Button("Custom load"))
        (void)loadPlaythroughFromFile(*pt, filenameBuf);

    ImGui::Separator();

    if (ImGui::Button("Feed next shrine"))
    {
        for (Shrine& shrine : pt->shrines)
        {
            if (!shrine.isActive() || shrine.tcDeath.hasValue())
                continue;

            const auto requiredReward = pt->getComputedRequiredRewardByShrineType(shrine.type);
            shrine.collectedReward += requiredReward / 3u;
            break;
        }
    }

    ImGui::Separator();

    ImGui::SetNextItemWidth(240.f * profile.uiScale);
    ImGui::InputScalar("Money", ImGuiDataType_U64, &pt->money, &step, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal);

    ImGui::SetNextItemWidth(240.f * profile.uiScale);
    ImGui::InputScalar("PPs", ImGuiDataType_U64, &pt->prestigePoints, &step, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal);

    ImGui::SetNextItemWidth(240.f * profile.uiScale);
    ImGui::InputScalar("WPs", ImGuiDataType_U64, &pt->wisdom, &step, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal);

    ImGui::SetNextItemWidth(240.f * profile.uiScale);
    ImGui::InputScalar("Mana", ImGuiDataType_U64, &pt->mana, &step, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal);

    ImGui::Separator();

    const auto scalarInput = [&](const char* label, float& value)
    {
        sf::base::String lbuf = label;
        lbuf += "##";
        lbuf += sf::base::toString(counter++);

        ImGui::SetNextItemWidth(140.f * profile.uiScale);
        if (ImGui::InputScalar(lbuf.cStr(), ImGuiDataType_Float, &value, &step, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal))
            value = sf::base::clamp(value, 0.f, 10'000.f);
    };

    const auto psvScalarInput = [&](const char* label, PurchasableScalingValue& psv)
    {
        if (psv.data->nMaxPurchases == 0u)
            return;

        sf::base::String lbuf = label;
        lbuf += "##";
        lbuf += sf::base::toString(counter++);

        ImGui::SetNextItemWidth(140.f * profile.uiScale);
        if (ImGui::InputScalar(lbuf.cStr(), ImGuiDataType_U64, &psv.nPurchases, &step, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal))
            psv.nPurchases = sf::base::clamp(psv.nPurchases, SizeT{0u}, psv.data->nMaxPurchases);
    };

    ImGui::Checkbox("ComboPurchased", &pt->comboPurchased);
    ImGui::Checkbox("MapPurchased", &pt->mapPurchased);

    psvScalarInput("ComboStartTime", pt->psvComboStartTime);
    psvScalarInput("MapExtension", pt->psvMapExtension);
    psvScalarInput("ShrineActivation", pt->psvShrineActivation);
    psvScalarInput("BubbleCount", pt->psvBubbleCount);
    psvScalarInput("SpellCount", pt->psvSpellCount);
    psvScalarInput("BubbleValue", pt->psvBubbleValue);
    psvScalarInput("ExplosionRadiusMult", pt->psvExplosionRadiusMult);

    ImGui::Separator();

    for (SizeT i = 0u; i < nCatTypes; ++i)
        scalarInput((sf::base::toString(i) + "Buff").cStr(), pt->buffCountdownsPerType[i].value);

    ImGui::Separator();

    for (SizeT i = 0u; i < nCatTypes; ++i)
    {
        ImGui::Text("%s", CatConstants::typeNames[i]);
        psvScalarInput("PerCatType", pt->psvPerCatType[i]);
        psvScalarInput("CooldownMultsPerCatType", pt->psvCooldownMultsPerCatType[i]);
        psvScalarInput("RangeDivsPerCatType", pt->psvRangeDivsPerCatType[i]);

        ImGui::Separator();
    }

    psvScalarInput("PPMultiPopRange", pt->psvPPMultiPopRange);
    psvScalarInput("PPInspireDurationMult", pt->psvPPInspireDurationMult);
    psvScalarInput("PPManaCooldownMult", pt->psvPPManaCooldownMult);
    psvScalarInput("PPManaMaxMult", pt->psvPPManaMaxMult);
    psvScalarInput("PPMouseCatGlobalBonusMult", pt->psvPPMouseCatGlobalBonusMult);
    psvScalarInput("PPEngiCatGlobalBonusMult", pt->psvPPEngiCatGlobalBonusMult);
    psvScalarInput("PPRepulsoCatConverterChance", pt->psvPPRepulsoCatConverterChance);
    psvScalarInput("PPWitchCatBuffDuration", pt->psvPPWitchCatBuffDuration);
    psvScalarInput("PPUniRitualBuffPercentage", pt->psvPPUniRitualBuffPercentage);
    psvScalarInput("PPDevilRitualBuffPercentage", pt->psvPPDevilRitualBuffPercentage);

    ImGui::Separator();

    ImGui::Checkbox("starterPackPurchased", &pt->perm.starterPackPurchased);
    ImGui::Checkbox("multiPopPurchased", &pt->perm.multiPopPurchased);
    ImGui::Checkbox("smartCatsPurchased", &pt->perm.smartCatsPurchased);
    ImGui::Checkbox("geniusCatsPurchased", &pt->perm.geniusCatsPurchased);
    ImGui::Checkbox("windPurchased", &pt->perm.windPurchased);
    ImGui::Checkbox("astroCatInspirePurchased", &pt->perm.astroCatInspirePurchased);
    ImGui::Checkbox("starpawConversionIgnoreBombs", &pt->perm.starpawConversionIgnoreBombs);
    ImGui::Checkbox("starpawNova", &pt->perm.starpawNova);
    ImGui::Checkbox("repulsoCatFilterPurchased", &pt->perm.repulsoCatFilterPurchased);
    ImGui::Checkbox("repulsoCatConverterPurchased", &pt->perm.repulsoCatConverterPurchased);
    ImGui::Checkbox("repulsoCatNovaConverterPurchased", &pt->perm.repulsoCatNovaConverterPurchased);
    ImGui::Checkbox("attractoCatFilterPurchased", &pt->perm.attractoCatFilterPurchased);
    ImGui::Checkbox("witchCatBuffPowerScalesWithNCats", &pt->perm.witchCatBuffPowerScalesWithNCats);
    ImGui::Checkbox("witchCatBuffPowerScalesWithMapSize", &pt->perm.witchCatBuffPowerScalesWithMapSize);
    ImGui::Checkbox("witchCatBuffFewerDolls", &pt->perm.witchCatBuffFewerDolls);
    ImGui::Checkbox("witchCatBuffFlammableDolls", &pt->perm.witchCatBuffFlammableDolls);
    ImGui::Checkbox("witchCatBuffOrbitalDolls", &pt->perm.witchCatBuffOrbitalDolls);
    ImGui::Checkbox("wizardCatDoubleMewltiplierDuration", &pt->perm.wizardCatDoubleMewltiplierDuration);
    ImGui::Checkbox("wizardCatDoubleStasisFieldDuration", &pt->perm.wizardCatDoubleStasisFieldDuration);
    ImGui::Checkbox("unicatTranscendencePurchased", &pt->perm.unicatTranscendencePurchased);
    ImGui::Checkbox("unicatTranscendenceAOEPurchased", &pt->perm.unicatTranscendenceAOEPurchased);
    ImGui::Checkbox("devilcatHellsingedPurchased", &pt->perm.devilcatHellsingedPurchased);
    ImGui::Checkbox("unicatTranscendenceEnabled", &pt->perm.unicatTranscendenceEnabled);
    ImGui::Checkbox("devilcatHellsingedEnabled", &pt->perm.devilcatHellsingedEnabled);
    ImGui::Checkbox("autocastPurchased", &pt->perm.autocastPurchased);

    ImGui::Separator();

    ImGui::Checkbox("shrineCompleted Witch", &pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Witch)]);
    ImGui::Checkbox("shrineCompleted Wizard", &pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Wizard)]);
    ImGui::Checkbox("shrineCompleted Mouse", &pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Mouse)]);
    ImGui::Checkbox("shrineCompleted Engi", &pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Engi)]);
    ImGui::Checkbox("shrineCompleted Attracto", &pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Attracto)]);
    ImGui::Checkbox("shrineCompleted Repulso", &pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Repulso)]);
    ImGui::Checkbox("shrineCompleted Copy", &pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Copy)]);
    ImGui::Checkbox("shrineCompleted Duck", &pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Duck)]);

    ImGui::Separator();
    ImGui::Text("Game constants");
    ImGui::SameLine();
    if (ImGui::Button("Save game constants"))
        saveGameConstantsToFile(gameConstants);

    ImGui::Checkbox("Draw cat center marker", &gameConstants.debugDrawCatCenterMarker);
    ImGui::Checkbox("Draw cat body bounds", &gameConstants.debugDrawCatBodyBounds);

    const auto inputFloat = [&](const char* label, float& value)
    {
        ImGui::SetNextItemWidth(220.f * profile.uiScale);
        ImGui::InputFloat(label, &value, 0.f, 0.f, "%.2f");
    };

    const auto inputInt = [&](const char* label, int& value)
    {
        ImGui::SetNextItemWidth(220.f * profile.uiScale);
        ImGui::InputInt(label, &value);
    };

    const auto inputVec2 = [&](const char* label, sf::Vec2f& value)
    {
        ImGui::SetNextItemWidth(280.f * profile.uiScale);
        ImGui::InputFloat2(label, &value.x, "%.2f");
    };

    if (ImGui::TreeNode("Cloud tuning"))
    {
        inputFloat("Opacity", gameConstants.catCloudOpacity);
        inputInt("Circle count", gameConstants.catCloudCircleCount);
        inputFloat("Scale", gameConstants.catCloudScale);
        inputFloat("X extent", gameConstants.catCloudXExtent);
        inputFloat("Base Y offset", gameConstants.catCloudBaseYOffset);
        inputFloat("Extra Y offset", gameConstants.catCloudExtraYOffset);
        inputFloat("Dragged Y offset", gameConstants.catCloudDraggedOffset);
        inputFloat("Lobe lift", gameConstants.catCloudLobeLift);
        inputFloat("Wobble X", gameConstants.catCloudWobbleX);
        inputFloat("Wobble Y", gameConstants.catCloudWobbleY);
        inputFloat("Radius base", gameConstants.catCloudRadiusBase);
        inputFloat("Radius lobe", gameConstants.catCloudRadiusLobe);
        inputFloat("Radius wobble", gameConstants.catCloudRadiusWobble);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Per-cat tables"))
    {
        for (SizeT i = 0u; i < nCatTypes; ++i)
        {
            ImGui::PushID(static_cast<int>(i));

            if (ImGui::TreeNode(CatConstants::typeNames[i]))
            {
                inputVec2("Draw offset", gameConstants.catDrawOffsetsByType[i]);
                inputVec2("Tail offset", gameConstants.catTailOffsetsByType[i]);
                inputVec2("Eye offset", gameConstants.catEyeOffsetsByType[i]);
                inputFloat("Attachment hue", gameConstants.catHueByType[i]);
                inputVec2("Cloud offset", gameConstants.cloudModifiers[i].positionOffset);
                inputFloat("Cloud X mult", gameConstants.cloudModifiers[i].xExtentMult);
                ImGui::TreePop();
            }

            ImGui::PopID();
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Attachment offsets"))
    {
        inputFloat("Dragged attachment Y", gameConstants.catAttachmentDraggedOffsetY);
        inputVec2("Brain jar offset", gameConstants.brainJarOffset);
        inputVec2("Uni wings offset", gameConstants.uniWingsOffset);
        inputVec2("Uni wings origin offset", gameConstants.uniWingsOriginOffsetFromCenter);
        inputVec2("Devil book offset", gameConstants.devilBookOffset);
        inputVec2("Devil paw idle offset", gameConstants.devilPawIdleOffset);
        inputVec2("Devil paw dragged offset", gameConstants.devilPawDraggedOffset);
        inputVec2("Smart hat offset", gameConstants.smartHatOffset);
        inputVec2("Ear flap offset", gameConstants.earFlapOffset);
        inputVec2("Yawn offset", gameConstants.yawnOffset);
        inputVec2("Uni tail extra offset", gameConstants.uniTailExtraOffset);
        inputVec2("Uni tail origin offset", gameConstants.uniTailOriginOffset);
        inputVec2("Eyelid offset", gameConstants.eyelidOffset);
        inputVec2("Regular paw idle offset", gameConstants.regularPawIdleOffset);
        inputVec2("Regular paw dragged offset", gameConstants.regularPawDraggedOffset);
        inputVec2("Copy mask offset", gameConstants.copyMaskOffset);
        inputVec2("Copy mask origin", gameConstants.copyMaskOrigin);
        drawSpriteAttachmentControls(profile.uiScale, "Devil back tail", gameConstants.devilBackTail);
        drawSpriteAttachmentControls(profile.uiScale, "Duck flag", gameConstants.duckFlag);
        drawSpriteAttachmentControls(profile.uiScale, "Smart diploma", gameConstants.smartDiploma);
        drawSpriteAttachmentControls(profile.uiScale, "Astro flag", gameConstants.astroFlag);
        drawSpriteAttachmentControls(profile.uiScale, "Engi wrench", gameConstants.engiWrench);
        drawSpriteAttachmentControls(profile.uiScale, "Attracto magnet", gameConstants.attractoMagnet);
        drawSpriteAttachmentControls(profile.uiScale, "Tail", gameConstants.tail);
        drawSpriteAttachmentControls(profile.uiScale, "Mouse prop", gameConstants.mouseProp);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Text offsets"))
    {
        inputFloat("Cat name Y", gameConstants.catNameTextOffsetY);
        inputFloat("Cat status Y", gameConstants.catStatusTextOffsetY);
        inputFloat("Cooldown bar Y", gameConstants.catCooldownBarOffsetY);
        ImGui::TreePop();
    }

    uiSetFontScale(uiNormalFontScale);
    ImGui::PopFont();
}
