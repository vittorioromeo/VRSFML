#include "Aliases.hpp"
#include "BubbleIdleMain.hpp"
#include "Cat.hpp"
#include "CatConstants.hpp"
#include "CatType.hpp"
#include "Countdown.hpp"
#include "GameEvent.hpp"
#include "Profile.hpp"
#include "PurchasableScalingValue.hpp"
#include "Serialization.hpp"
#include "Shrine.hpp"
#include "Version.hpp"

#include "SFML/ImGui/IncludeImGui.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/SizeT.hpp"
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

void drawDebugSectionTitle(const char* label)
{
    ImGui::Separator();
    ImGui::Text("%s", label);
}

template <typename TAction0, typename TAction1>
void drawButtonRow2(const char* label0, TAction0&& action0, const char* label1, TAction1&& action1)
{
    const float spacing = ImGui::GetStyle().ItemSpacing.x;
    const float width   = (ImGui::GetContentRegionAvail().x - spacing) * 0.5f;

    if (ImGui::Button(label0, {width, 0.f}))
        action0();

    ImGui::SameLine();

    if (ImGui::Button(label1, {width, 0.f}))
        action1();
}

void drawDebugQuickTools(Main& main)
{
    static int                catTypeN          = 0;
    static sf::base::I64      speedrunTimerSet  = 0;
    static char               filenameBuf[128]  = "userdata/custom.json";
    constexpr sf::base::I64   speedrunTimerStep = 1;
    constexpr sf::base::SizeT currencyStep      = 1u;

    auto fullWidth = [&]() { return ImGui::GetContentRegionAvail().x - 140.f; };

    ImGui::Checkbox("Hide main UI", &main.uiState.debugHideUI);
    ImGui::Text("Active events: %zu", main.pt->activeEvents.size());

    drawDebugSectionTitle("Runtime");
    ImGui::Text("Time scale: %.2fx", static_cast<double>(main.debugTimeScale));

    const float sliderButtonWidth = 92.f * main.profile.uiScale;
    const float sliderSpacing     = ImGui::GetStyle().ItemSpacing.x;
    ImGui::SetNextItemWidth(fullWidth() - sliderButtonWidth - sliderSpacing);
    ImGui::SliderFloat("##timescale", &main.debugTimeScale, 0.05f, 10.f, "%.2fx", ImGuiSliderFlags_Logarithmic);

    ImGui::SameLine();
    if (ImGui::Button("Reset##timescale", {sliderButtonWidth, 0.f}))
        main.debugTimeScale = 1.f;

    drawButtonRow2("Slide",
                   [&main]
    {
        main.fixedBgSlideTarget += 1.f;

        if (main.fixedBgSlideTarget >= 3.f)
            main.fixedBgSlideTarget = 0.f;
    },
                   "Feed next shrine",
                   [&main]
    {
        for (Shrine& shrine : main.pt->shrines)
        {
            if (!shrine.isActive() || shrine.tcDeath.hasValue())
                continue;

            const auto requiredReward = main.pt->getComputedRequiredRewardByShrineType(shrine.type);
            shrine.collectedReward += requiredReward / 3u;
            break;
        }
    });

    ImGui::SetNextItemWidth(fullWidth());
    if (ImGui::InputScalar("Speedrun timer", ImGuiDataType_S64, &speedrunTimerSet, &speedrunTimerStep, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal))
    {
        main.pt->speedrunStartTime.emplace(sf::microseconds(speedrunTimerSet));
    }

    drawDebugSectionTitle("Events");
    const auto& bfCfg = main.gameConstants.events.bubblefall;

    drawButtonRow2("Bubblefall (random)",
                   [&main, &bfCfg]
    {
        const float halfWidth = bfCfg.regionWidth * 0.5f;
        main.addEventBubblefall(main.rng.getF(halfWidth, main.pt->getMapLimit() - halfWidth));
    },
                   "Bubblefall (view)",
                   [&main, &bfCfg]
    {
        const float viewCenterX = main.gameView.center.x;
        const float halfWidth   = bfCfg.regionWidth * 0.5f;
        const float mapLimit    = main.pt->getMapLimit();

        main.addEventBubblefall(sf::base::clamp(viewCenterX, halfWidth, mapLimit - halfWidth));
    });

    drawButtonRow2("Clear events", [&main] { main.pt->activeEvents.clear(); }, "Invincible bubble", [&main] {
        main.addEventInvincibleBubble();
    });

    if (ImGui::Button("Nap random cat", {fullWidth(), 0.f}))
    {
        sf::base::SizeT eligibleCount = 0u;
        Cat*            selected      = nullptr;

        for (Cat& candidate : main.pt->cats)
        {
            if (!main.canCatNap(candidate))
                continue;

            ++eligibleCount;

            // Reservoir sampling.
            if (main.rng.getI<sf::base::SizeT>(0, eligibleCount - 1) == 0)
                selected = &candidate;
        }

        if (selected != nullptr)
            main.beginCatNap(*selected, /* sleepDurationMs */ 20'000.f);
    }

    drawDebugSectionTitle("Cats and Rituals");
    ImGui::SetNextItemWidth(fullWidth());
    ImGui::Combo("Cat type", &catTypeN, CatConstants::typeNames, nCatTypes);

    drawButtonRow2("Spawn",
                   [&main]
    {
        const auto catType = static_cast<CatType>(catTypeN);

        if (isUniqueCatType(catType))
        {
            const auto pos = main.gameView.screenToWorld(main.getResolution() / 2.f, main.window.getSize().toVec2f());
            main.spawnSpecialCat(pos, catType);
        }
        else
        {
            main.spawnCatCentered(catType, main.getHueByCatType(catType));
        }
    },
                   "Do Ritual",
                   [&main]
    {
        if (auto* wc = main.getWitchCat())
            wc->cooldown.value = 10.f;
    });

    drawButtonRow2("Ritual",
                   [&main]
    {
        if (auto* wc = main.getWitchCat())
            wc->cooldown.value = 12000.f;
    },
                   "Do Copy Ritual",
                   [&main]
    {
        if (auto* wc = main.getCopyCat())
            wc->cooldown.value = 10.f;
    });

    drawButtonRow2("Copy Ritual",
                   [&main]
    {
        if (auto* wc = main.getCopyCat())
            wc->cooldown.value = 12000.f;
    },
                   "Do Letter",
                   [&main]
    {
        main.victoryTC.emplace(TargetedCountdown{.startingValue = 6500.f});
        main.victoryTC->restart();
        main.delayedActions.emplaceBack(Countdown{.value = 7000.f}, [&main] { main.playSound(main.sounds.letterchime); });
    });

    drawButtonRow2("Do Tip", [&main] {
        main.doTip("Hello, I am a tip!\nHello world... How are you doing today?\nTest test test");
    }, "Do Notification", [&main] {
        main.pushNotification("Test notification", "Hello, I am a test notification!\nHow are you doing today?");
    });

    drawButtonRow2("Do Arrow",
                   [&main] { main.uiState.scrollArrowCountdown.value = 2000.f; },
                   "Do Prestige",
                   [&main]
    {
        ++main.pt->psvBubbleValue.nPurchases;
        const auto ppReward = main.pt->calculatePrestigePointReward(1u);
        main.beginPrestigeTransition(ppReward);
    });

    drawDebugSectionTitle("Save Data");
    drawButtonRow2("Save game",
                   [&main]
    {
        main.ptMain.fullVersion = !isDemoVersion;
        savePlaythroughToFile(main.ptMain, "userdata/playthrough.json");
    },
                   "Load game",
                   [&main] { main.loadPlaythroughFromFileAndReseed(); });

    main.uiState.uiButtonHueMod = 120.f;
    main.uiPushButtonColors();
    drawButtonRow2("Reset game", [&main] { main.forceResetGame(); }, "Reset profile", [&main] {
        main.forceResetProfile();
    });
    main.uiPopButtonColors();
    main.uiState.uiButtonHueMod = 0.f;

    ImGui::SetNextItemWidth(fullWidth());
    ImGui::InputText("Custom file", filenameBuf, sizeof(filenameBuf));

    drawButtonRow2("Custom save",
                   [&main]
    {
        main.pt->fullVersion = !isDemoVersion;
        savePlaythroughToFile(*main.pt, filenameBuf);
    },
                   "Custom load",
                   [&main] { (void)loadPlaythroughFromFile(*main.pt, filenameBuf); });

    drawDebugSectionTitle("Economy");

    const float inputSpacing = ImGui::GetStyle().ItemSpacing.x;
    const float inputWidth   = (fullWidth() - inputSpacing) * 0.5f;

    ImGui::SetNextItemWidth(inputWidth);
    ImGui::InputScalar("Money", ImGuiDataType_U64, &main.pt->money, &currencyStep, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal);

    ImGui::SameLine();
    ImGui::SetNextItemWidth(inputWidth);
    ImGui::InputScalar("PPs", ImGuiDataType_U64, &main.pt->prestigePoints, &currencyStep, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal);

    ImGui::SetNextItemWidth(inputWidth);
    ImGui::InputScalar("WPs", ImGuiDataType_U64, &main.pt->wisdom, &currencyStep, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal);

    ImGui::SameLine();
    ImGui::SetNextItemWidth(inputWidth);
    ImGui::InputScalar("Mana", ImGuiDataType_U64, &main.pt->mana, &currencyStep, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal);

    ImGui::Checkbox("Combo purchased", &main.pt->comboPurchased);
    ImGui::SameLine();
    ImGui::Checkbox("Map purchased", &main.pt->mapPurchased);
}

void drawDebugStateEditors(Main& main)
{
    main.uiSetFontScale(main.uiToolTipFontScale);
    ImGui::PushFont(main.fontImGuiMouldyCheese);

    sf::base::SizeT integerStep = 1u;
    float           floatStep   = 1.f;
    sf::base::SizeT counter     = 0u;

    const auto scalarInput = [&](const char* label, float& value)
    {
        sf::base::String lbuf = label;
        lbuf += "##";
        lbuf += sf::base::toString(counter++);

        ImGui::SetNextItemWidth(160.f * main.profile.uiScale);
        if (ImGui::InputScalar(lbuf.cStr(), ImGuiDataType_Float, &value, &floatStep, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal))
        {
            value = sf::base::clamp(value, 0.f, 10'000.f);
        }
    };

    const auto psvScalarInput = [&](const char* label, PurchasableScalingValue& psv)
    {
        if (psv.data->nMaxPurchases == 0u)
            return;

        sf::base::String lbuf = label;
        lbuf += "##";
        lbuf += sf::base::toString(counter++);

        ImGui::SetNextItemWidth(160.f * main.profile.uiScale);
        if (ImGui::InputScalar(lbuf.cStr(), ImGuiDataType_U64, &psv.nPurchases, &integerStep, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal))
        {
            psv.nPurchases = sf::base::clamp(psv.nPurchases, SizeT{0u}, psv.data->nMaxPurchases);
        }
    };

    const auto inputFloat = [&](const char* label, float& value)
    {
        ImGui::SetNextItemWidth(220.f * main.profile.uiScale);
        ImGui::InputFloat(label, &value, 0.f, 0.f, "%.2f");
    };

    const auto inputInt = [&](const char* label, int& value)
    {
        ImGui::SetNextItemWidth(220.f * main.profile.uiScale);
        ImGui::InputInt(label, &value);
    };

    const auto inputVec2 = [&](const char* label, sf::Vec2f& value)
    {
        ImGui::SetNextItemWidth(280.f * main.profile.uiScale);
        ImGui::InputFloat2(label, &value.x, "%.2f");
    };

    if (ImGui::CollapsingHeader("Progression Values", ImGuiTreeNodeFlags_DefaultOpen))
    {
        psvScalarInput("ComboStartTime", main.pt->psvComboStartTime);
        psvScalarInput("MapExtension", main.pt->psvMapExtension);
        psvScalarInput("ShrineActivation", main.pt->psvShrineActivation);
        psvScalarInput("BubbleCount", main.pt->psvBubbleCount);
        psvScalarInput("SpellCount", main.pt->psvSpellCount);
        psvScalarInput("BubbleValue", main.pt->psvBubbleValue);
        psvScalarInput("ExplosionRadiusMult", main.pt->psvExplosionRadiusMult);
        psvScalarInput("PPMultiPopRange", main.pt->psvPPMultiPopRange);
        psvScalarInput("PPInspireDurationMult", main.pt->psvPPInspireDurationMult);
        psvScalarInput("PPManaCooldownMult", main.pt->psvPPManaCooldownMult);
        psvScalarInput("PPManaMaxMult", main.pt->psvPPManaMaxMult);
        psvScalarInput("PPMouseCatGlobalBonusMult", main.pt->psvPPMouseCatGlobalBonusMult);
        psvScalarInput("PPEngiCatGlobalBonusMult", main.pt->psvPPEngiCatGlobalBonusMult);
        psvScalarInput("PPRepulsoCatConverterChance", main.pt->psvPPRepulsoCatConverterChance);
        psvScalarInput("PPWitchCatBuffDuration", main.pt->psvPPWitchCatBuffDuration);
        psvScalarInput("PPUniRitualBuffPercentage", main.pt->psvPPUniRitualBuffPercentage);
        psvScalarInput("PPDevilRitualBuffPercentage", main.pt->psvPPDevilRitualBuffPercentage);
    }

    if (ImGui::CollapsingHeader("Cat Buff Countdown Values"))
    {
        for (SizeT i = 0u; i < nCatTypes; ++i)
            scalarInput((sf::base::toString(i) + "Buff").cStr(), main.pt->buffCountdownsPerType[i].value);
    }

    if (ImGui::CollapsingHeader("Per-cat Upgrade Tables"))
    {
        for (SizeT i = 0u; i < nCatTypes; ++i)
        {
            ImGui::Text("%s", CatConstants::typeNames[i]);
            psvScalarInput("PerCatType", main.pt->psvPerCatType[i]);
            psvScalarInput("CooldownMultsPerCatType", main.pt->psvCooldownMultsPerCatType[i]);
            psvScalarInput("RangeDivsPerCatType", main.pt->psvRangeDivsPerCatType[i]);
            ImGui::Separator();
        }
    }

    if (ImGui::CollapsingHeader("Permanent Unlocks"))
    {
        ImGui::Checkbox("starterPackPurchased", &main.pt->perm.starterPackPurchased);
        ImGui::Checkbox("multiPopPurchased", &main.pt->perm.multiPopPurchased);
        ImGui::Checkbox("smartCatsPurchased", &main.pt->perm.smartCatsPurchased);
        ImGui::Checkbox("geniusCatsPurchased", &main.pt->perm.geniusCatsPurchased);
        ImGui::Checkbox("windPurchased", &main.pt->perm.windPurchased);
        ImGui::Checkbox("astroCatInspirePurchased", &main.pt->perm.astroCatInspirePurchased);
        ImGui::Checkbox("starpawConversionIgnoreBombs", &main.pt->perm.starpawConversionIgnoreBombs);
        ImGui::Checkbox("starpawNova", &main.pt->perm.starpawNova);
        ImGui::Checkbox("repulsoCatFilterPurchased", &main.pt->perm.repulsoCatFilterPurchased);
        ImGui::Checkbox("repulsoCatConverterPurchased", &main.pt->perm.repulsoCatConverterPurchased);
        ImGui::Checkbox("repulsoCatNovaConverterPurchased", &main.pt->perm.repulsoCatNovaConverterPurchased);
        ImGui::Checkbox("attractoCatFilterPurchased", &main.pt->perm.attractoCatFilterPurchased);
        ImGui::Checkbox("witchCatBuffPowerScalesWithNCats", &main.pt->perm.witchCatBuffPowerScalesWithNCats);
        ImGui::Checkbox("witchCatBuffPowerScalesWithMapSize", &main.pt->perm.witchCatBuffPowerScalesWithMapSize);
        ImGui::Checkbox("witchCatBuffFewerDolls", &main.pt->perm.witchCatBuffFewerDolls);
        ImGui::Checkbox("witchCatBuffFlammableDolls", &main.pt->perm.witchCatBuffFlammableDolls);
        ImGui::Checkbox("witchCatBuffOrbitalDolls", &main.pt->perm.witchCatBuffOrbitalDolls);
        ImGui::Checkbox("wizardCatDoubleMewltiplierDuration", &main.pt->perm.wizardCatDoubleMewltiplierDuration);
        ImGui::Checkbox("wizardCatDoubleStasisFieldDuration", &main.pt->perm.wizardCatDoubleStasisFieldDuration);
        ImGui::Checkbox("unicatTranscendencePurchased", &main.pt->perm.unicatTranscendencePurchased);
        ImGui::Checkbox("unicatTranscendenceAOEPurchased", &main.pt->perm.unicatTranscendenceAOEPurchased);
        ImGui::Checkbox("devilcatHellsingedPurchased", &main.pt->perm.devilcatHellsingedPurchased);
        ImGui::Checkbox("unicatTranscendenceEnabled", &main.pt->perm.unicatTranscendenceEnabled);
        ImGui::Checkbox("devilcatHellsingedEnabled", &main.pt->perm.devilcatHellsingedEnabled);
        ImGui::Checkbox("autocastPurchased", &main.pt->perm.autocastPurchased);
    }

    if (ImGui::CollapsingHeader("Shrine Completion Flags"))
    {
        ImGui::Checkbox("shrineCompleted Witch", &main.pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Witch)]);
        ImGui::Checkbox("shrineCompleted Wizard", &main.pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Wizard)]);
        ImGui::Checkbox("shrineCompleted Mouse", &main.pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Mouse)]);
        ImGui::Checkbox("shrineCompleted Engi", &main.pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Engi)]);
        ImGui::Checkbox("shrineCompleted Attracto", &main.pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Attracto)]);
        ImGui::Checkbox("shrineCompleted Repulso", &main.pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Repulso)]);
        ImGui::Checkbox("shrineCompleted Copy", &main.pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Copy)]);
        ImGui::Checkbox("shrineCompleted Duck", &main.pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Duck)]);
    }

    if (ImGui::CollapsingHeader("Game Constants", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Button("Save game constants"))
            saveGameConstantsToFile(main.gameConstants);

        ImGui::Checkbox("Draw cat center marker", &main.gameConstants.debugDrawCatCenterMarker);
        ImGui::Checkbox("Draw cat body bounds", &main.gameConstants.debugDrawCatBodyBounds);

        if (ImGui::TreeNode("Cloud tuning"))
        {
            inputFloat("Opacity", main.gameConstants.catCloudOpacity);
            inputInt("Circle count", main.gameConstants.catCloudCircleCount);
            inputFloat("Scale", main.gameConstants.catCloudScale);
            inputFloat("X extent", main.gameConstants.catCloudXExtent);
            inputFloat("Base Y offset", main.gameConstants.catCloudBaseYOffset);
            inputFloat("Extra Y offset", main.gameConstants.catCloudExtraYOffset);
            inputFloat("Dragged Y offset", main.gameConstants.catCloudDraggedOffset);
            inputFloat("Lobe lift", main.gameConstants.catCloudLobeLift);
            inputFloat("Wobble X", main.gameConstants.catCloudWobbleX);
            inputFloat("Wobble Y", main.gameConstants.catCloudWobbleY);
            inputFloat("Radius base", main.gameConstants.catCloudRadiusBase);
            inputFloat("Radius lobe", main.gameConstants.catCloudRadiusLobe);
            inputFloat("Radius wobble", main.gameConstants.catCloudRadiusWobble);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Per-cat tables"))
        {
            for (SizeT i = 0u; i < nCatTypes; ++i)
            {
                ImGui::PushID(static_cast<int>(i));

                if (ImGui::TreeNode(CatConstants::typeNames[i]))
                {
                    inputVec2("Draw offset", main.gameConstants.catDrawOffsetsByType[i]);
                    inputVec2("Tail offset", main.gameConstants.catTailOffsetsByType[i]);
                    inputVec2("Eye offset", main.gameConstants.catEyeOffsetsByType[i]);
                    inputFloat("Attachment hue", main.gameConstants.catHueByType[i]);
                    inputVec2("Cloud offset", main.gameConstants.cloudModifiers[i].positionOffset);
                    inputFloat("Cloud X mult", main.gameConstants.cloudModifiers[i].xExtentMult);
                    ImGui::TreePop();
                }

                ImGui::PopID();
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Attachment offsets"))
        {
            inputFloat("Dragged attachment Y", main.gameConstants.catAttachmentDraggedOffsetY);
            inputVec2("Brain jar offset", main.gameConstants.brainJarOffset);
            inputVec2("Uni wings offset", main.gameConstants.uniWingsOffset);
            inputVec2("Uni wings origin offset", main.gameConstants.uniWingsOriginOffsetFromCenter);
            inputVec2("Devil book offset", main.gameConstants.devilBookOffset);
            inputVec2("Devil paw idle offset", main.gameConstants.devilPawIdleOffset);
            inputVec2("Devil paw dragged offset", main.gameConstants.devilPawDraggedOffset);
            inputVec2("Smart hat offset", main.gameConstants.smartHatOffset);
            inputVec2("Ear flap offset", main.gameConstants.earFlapOffset);
            inputVec2("Yawn offset", main.gameConstants.yawnOffset);
            inputVec2("Uni tail extra offset", main.gameConstants.uniTailExtraOffset);
            inputVec2("Uni tail origin offset", main.gameConstants.uniTailOriginOffset);
            inputVec2("Eyelid offset", main.gameConstants.eyelidOffset);
            inputVec2("Regular paw idle offset", main.gameConstants.regularPawIdleOffset);
            inputVec2("Regular paw dragged offset", main.gameConstants.regularPawDraggedOffset);
            inputVec2("Copy mask offset", main.gameConstants.copyMaskOffset);
            inputVec2("Copy mask origin", main.gameConstants.copyMaskOrigin);
            inputVec2("Warden guardhouse back offset", main.gameConstants.wardenGuardhouseBackOffset);
            inputVec2("Warden guardhouse front offset", main.gameConstants.wardenGuardhouseFrontOffset);
            inputVec2("Warden cat body offset", main.gameConstants.wardenCatBodyOffset);
            inputVec2("Warden cat paw offset", main.gameConstants.wardenCatPawOffset);
            inputFloat("Warden cat body wobble (rad)", main.gameConstants.wardenCatBodyWobbleRadians);
            inputVec2("Warden eyelid origin offset", main.gameConstants.wardenCatEyelidOriginOffset);
            inputVec2("Warden yawn origin offset", main.gameConstants.wardenCatYawnOriginOffset);
            inputFloat("Warden paw scale", main.gameConstants.wardenCatPawScale);
            drawSpriteAttachmentControls(main.profile.uiScale, "Devil back tail", main.gameConstants.devilBackTail);
            drawSpriteAttachmentControls(main.profile.uiScale, "Duck flag", main.gameConstants.duckFlag);
            drawSpriteAttachmentControls(main.profile.uiScale, "Smart diploma", main.gameConstants.smartDiploma);
            drawSpriteAttachmentControls(main.profile.uiScale, "Astro flag", main.gameConstants.astroFlag);
            drawSpriteAttachmentControls(main.profile.uiScale, "Engi wrench", main.gameConstants.engiWrench);
            drawSpriteAttachmentControls(main.profile.uiScale, "Attracto magnet", main.gameConstants.attractoMagnet);
            drawSpriteAttachmentControls(main.profile.uiScale, "Tail", main.gameConstants.tail);
            drawSpriteAttachmentControls(main.profile.uiScale, "Mouse prop", main.gameConstants.mouseProp);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Text offsets"))
        {
            inputFloat("Cat name Y", main.gameConstants.catNameTextOffsetY);
            inputFloat("Cat status Y", main.gameConstants.catStatusTextOffsetY);
            inputFloat("Cooldown bar Y", main.gameConstants.catCooldownBarOffsetY);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Events tuning"))
        {
            auto& eventsCfg = main.gameConstants.events;

            inputFloat("Min spawn interval (ms)", eventsCfg.minSpawnIntervalMs);
            inputFloat("Max spawn interval (ms)", eventsCfg.maxSpawnIntervalMs);

            if (ImGui::TreeNode("Bubblefall"))
            {
                auto& bf = eventsCfg.bubblefall;

                inputFloat("Duration (ms)", bf.durationMs);
                inputFloat("Region width", bf.regionWidth);
                inputFloat("Spawn interval (ms)", bf.spawnIntervalMs);

                int bubblesPerTick = static_cast<int>(bf.bubblesPerTick);
                ImGui::SetNextItemWidth(220.f * main.profile.uiScale);
                if (ImGui::InputInt("Bubbles per tick", &bubblesPerTick) && bubblesPerTick >= 0)
                    bf.bubblesPerTick = static_cast<sf::base::SizeT>(bubblesPerTick);

                inputFloat("Initial velocity Y", bf.initialVelocityY);
                inputFloat("Velocity jitter Y", bf.velocityJitterY);
                inputFloat("Velocity jitter X", bf.velocityJitterX);
                inputFloat("Attack ratio", bf.attackRatio);
                inputFloat("Release ratio", bf.releaseRatio);
                ImGui::TreePop();
            }

            ImGui::TreePop();
        }
    }

    main.uiSetFontScale(main.uiNormalFontScale);
    ImGui::PopFont();
}
} // namespace

void Main::uiDrawDebugWindow()
{
    if (!isDebugModeEnabled() || !uiState.debugWindowVisible)
        return;

    const sf::Vec2f resolution = getResolution();
    const float     scale      = profile.uiScale;
    const float     margin     = 24.f * scale;

    float maxWindowWidth  = resolution.x - margin * 2.f;
    float maxWindowHeight = resolution.y - margin * 2.f;

    if (maxWindowWidth <= 0.f)
        maxWindowWidth = resolution.x;

    if (maxWindowHeight <= 0.f)
        maxWindowHeight = resolution.y;

    const float minWindowWidth  = maxWindowWidth < 760.f * scale ? maxWindowWidth : 760.f * scale;
    const float minWindowHeight = maxWindowHeight < 520.f * scale ? maxWindowHeight : 520.f * scale;
    const float defaultWidth    = maxWindowWidth < 1120.f * scale ? maxWindowWidth : 1120.f * scale;
    const float defaultHeight   = maxWindowHeight < 820.f * scale ? maxWindowHeight : 820.f * scale;

    ImGui::SetNextWindowPos({margin, margin}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({defaultWidth, defaultHeight}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints({minWindowWidth, minWindowHeight}, {maxWindowWidth, maxWindowHeight});
    ImGui::SetNextWindowBgAlpha(0.f);

    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.f, 0.f, 0.f, 0.f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.f, 0.f, 0.f, 0.f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.f, 0.f, 0.f, 0.f));

    if (!ImGui::Begin("Debug Tools (F8)", &uiState.debugWindowVisible, ImGuiWindowFlags_NoSavedSettings))
    {
        ImGui::PopStyleColor(3);
        ImGui::End();
        return;
    }

    uiDrawCloudWindowBackground();
    uiSettingsDrawDebugTab();
    ImGui::End();
    ImGui::PopStyleColor(3);
}

void Main::uiSettingsDrawDebugTab()
{
    const float spacing      = ImGui::GetStyle().ItemSpacing.x;
    const float contentWidth = ImGui::GetContentRegionAvail().x;
    const float desiredLeft  = 430.f * profile.uiScale;
    const float minLeft      = 460.f * profile.uiScale;
    const float maxLeft      = contentWidth * 0.48f;
    const float leftPaneWidth = maxLeft > minLeft ? sf::base::clamp(desiredLeft, minLeft, maxLeft) : contentWidth * 0.5f;

    ImGui::BeginChild("DebugQuickToolsPane", {leftPaneWidth, 0.f}, true);
    drawDebugQuickTools(*this);
    ImGui::EndChild();

    ImGui::SameLine(0.f, spacing);

    ImGui::BeginChild("DebugStateEditorsPane", {0.f, 0.f}, true);
    drawDebugStateEditors(*this);
    ImGui::EndChild();
}
