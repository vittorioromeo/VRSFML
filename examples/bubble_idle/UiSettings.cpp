

#include "Aliases.hpp"
#include "BubbleIdleMain.hpp"
#include "Cat.hpp"
#include "CatConstants.hpp"
#include "CatType.hpp"
#include "Countdown.hpp"
#include "IconsFontAwesome6.h"
#include "Profile.hpp"
#include "PurchasableScalingValue.hpp"
#include "Serialization.hpp"
#include "Shrine.hpp"
#include "Version.hpp"

#include "SFML/ImGui/IncludeImGui.hpp"

#include "SFML/Window/VideoModeUtils.hpp"

#include "SFML/System/Vec2Base.hpp"

#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/ScopeGuard.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/ToString.hpp"
#include "SFML/Base/Vector.hpp"

void Main::uiTabBarSettings()
{
    bool sgActive = false;
    SFML_BASE_SCOPE_GUARD({
        if (sgActive)
            ImGui::EndTabBar();
    });
    sgActive = ImGui::BeginTabBar("TabBarSettings", ImGuiTabBarFlags_DrawSelectedOverline);

    static int lastSelectedTabIdx = 0;

    const auto selectedTab = [&](int idx)
    {
        if (lastSelectedTabIdx != idx)
            playSound(sounds.uitab);

        lastSelectedTabIdx = idx;
    };

    uiSetFontScale(0.75f);
    if (ImGui::BeginTabItem(ICON_FA_VOLUME_HIGH " Audio "))
    {
        selectedTab(0);

        uiSetFontScale(uiNormalFontScale);

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Master volume", &profile.masterVolume, 0.f, 100.f, "%.f%%");

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        if (ImGui::SliderFloat("SFX volume", &profile.sfxVolume, 0.f, 100.f, "%.f%%"))
            sounds.setupSounds(/* volumeOnly */ true, profile.sfxVolume / 100.f);

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Music volume", &profile.musicVolume, 0.f, 100.f, "%.f%%");

        uiCheckbox("Play audio in background", &profile.playAudioInBackground);
        uiCheckbox("Enable combo scratch sound", &profile.playComboEndSound);
        uiCheckbox("Enable ritual sounds", &profile.playWitchRitualSounds);

        ImGui::EndTabItem();
    }

    uiSetFontScale(0.75f);
    if (ImGui::BeginTabItem(ICON_FA_WINDOW_MAXIMIZE " UI "))
    {
        selectedTab(1);

        uiSetFontScale(uiNormalFontScale);

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Minimap Scale", &profile.minimapScale, 5.f, 40.f, "%.2f");

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("HUD Scale", &profile.hudScale, 0.5f, 2.f, "%.2f");

        ImGui::Separator();

        ImGui::AlignTextToFramePadding();
        ImGui::Text("UI Scale");

        const auto makeUIScaleButton = [&](const char* label, const float scaleFactor)
        {
            ImGui::SameLine();
            if (ImGui::Button(label, ImVec2{46.f * profile.uiScale, 0.f}))
            {
                playSound(sounds.buy);
                profile.uiScale = scaleFactor;
            }
        };

        makeUIScaleButton("XXL", 1.75f);
        makeUIScaleButton("XL", 1.5f);
        makeUIScaleButton("L", 1.25f);
        makeUIScaleButton("M", 1.f);
        makeUIScaleButton("S", 0.75f);
        makeUIScaleButton("XS", 0.5f);

        ImGui::Separator();

        uiCheckbox("Hide maxed-out upgrades", &profile.hideMaxedOutPurchasables);
        uiCheckbox("Hide category separators", &profile.hideCategorySeparators);

        ImGui::Separator();

        uiCheckbox("Enable tips", &profile.tipsEnabled);

        ImGui::Separator();

        uiCheckbox("Enable notifications", &profile.enableNotifications);

        ImGui::BeginDisabled(!profile.enableNotifications);
        uiCheckbox("Enable full mana notification", &profile.showFullManaNotification);
        ImGui::EndDisabled();

        ImGui::Separator();

        uiCheckbox("Enable $/s meter", &profile.showDpsMeter);

        ImGui::Separator();

        constexpr const char* trailModeNames[3]{"Combo only", "Always", "Never"};

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::Combo("Cursor trail mode", &profile.cursorTrailMode, trailModeNames, 3);

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Cursor trail scale", &profile.cursorTrailScale, 0.25f, 5.f, "%.2f");

        ImGui::Separator();

        uiCheckbox("High-visibility cursor", &profile.highVisibilityCursor);

        ImGui::BeginDisabled(!profile.highVisibilityCursor);
        {
            uiSetFontScale(0.75f);

            uiCheckbox("Multicolor", &profile.multicolorCursor);

            ImGui::BeginDisabled(profile.multicolorCursor);
            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Hue", &profile.cursorHue, 0.f, 360.f, "%.2f");
            ImGui::EndDisabled();

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Scale", &profile.cursorScale, 0.3f, 1.5f, "%.2f");

            uiSetFontScale(uiNormalFontScale);
        }
        ImGui::EndDisabled();

        ImGui::Separator();

        uiCheckbox("Accumulating combo effect", &profile.accumulatingCombo);
        uiCheckbox("Show cursor combo text", &profile.showCursorComboText);
        uiCheckbox("Show cursor combo bar", &profile.showCursorComboBar);

        ImGui::Separator();

        uiCheckbox("Invert mouse buttons", &profile.invertMouseButtons);

        ImGui::Separator();

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Cat drag timer", &profile.catDragPressDuration, 50.f, 500.f, "%.2fms");

        ImGui::EndTabItem();
    }

    uiSetFontScale(0.75f);
    if (ImGui::BeginTabItem(ICON_FA_IMAGE " Graphics "))
    {
        selectedTab(2);

        uiSetFontScale(uiNormalFontScale);

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Background Opacity", &profile.backgroundOpacity, 0.f, 100.f, "%.f%%");

        uiCheckbox("Always show drawings", &profile.alwaysShowDrawings);

        ImGui::Separator();

        uiCheckbox("Show cat range", &profile.showCatRange);
        uiCheckbox("Show cat text", &profile.showCatText);
        uiCheckbox("Enable cat bobbing", &profile.enableCatBobbing);

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Cat range thickness", &profile.catRangeOutlineThickness, 1.f, 4.f, "%.2fpx");

        ImGui::Separator();

        ImGui::Text("Cat clouds");

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Cloud opacity", &profile.catCloudOpacity, 0.f, 1.f, "%.2f");

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderInt("Cloud circles", &profile.catCloudCircleCount, 3, 24);

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Cloud scale", &profile.catCloudScale, 0.5f, 3.f, "%.2f");

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Cloud X extent", &profile.catCloudXExtent, 4.f, 40.f, "%.2f");

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Cloud base Y", &profile.catCloudBaseYOffset, -10.f, 35.f, "%.2f");

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Cloud extra Y", &profile.catCloudExtraYOffset, -10.f, 45.f, "%.2f");

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Cloud dragged Y", &profile.catCloudDraggedOffset, 0.f, 20.f, "%.2f");

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Cloud lobe lift", &profile.catCloudLobeLift, 0.f, 10.f, "%.2f");

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Cloud wobble X", &profile.catCloudWobbleX, 0.f, 8.f, "%.2f");

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Cloud wobble Y", &profile.catCloudWobbleY, 0.f, 8.f, "%.2f");

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Cloud base radius", &profile.catCloudRadiusBase, 2.f, 20.f, "%.2f");

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Cloud lobe radius", &profile.catCloudRadiusLobe, 0.f, 16.f, "%.2f");

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Cloud radius wobble", &profile.catCloudRadiusWobble, 0.f, 5.f, "%.2f");

        if (ImGui::Button("Reset clouds to default"))
        {
            Profile defaultProfile{};

            profile.catCloudOpacity       = defaultProfile.catCloudOpacity;
            profile.catCloudCircleCount   = defaultProfile.catCloudCircleCount;
            profile.catCloudScale         = defaultProfile.catCloudScale;
            profile.catCloudXExtent       = defaultProfile.catCloudXExtent;
            profile.catCloudBaseYOffset   = defaultProfile.catCloudBaseYOffset;
            profile.catCloudExtraYOffset  = defaultProfile.catCloudExtraYOffset;
            profile.catCloudDraggedOffset = defaultProfile.catCloudDraggedOffset;
            profile.catCloudLobeLift      = defaultProfile.catCloudLobeLift;
            profile.catCloudWobbleX       = defaultProfile.catCloudWobbleX;
            profile.catCloudWobbleY       = defaultProfile.catCloudWobbleY;
            profile.catCloudRadiusBase    = defaultProfile.catCloudRadiusBase;
            profile.catCloudRadiusLobe    = defaultProfile.catCloudRadiusLobe;
            profile.catCloudRadiusWobble  = defaultProfile.catCloudRadiusWobble;
        }

        ImGui::Separator();

        uiCheckbox("Show particles", &profile.showParticles);

        ImGui::BeginDisabled(!profile.showParticles);
        uiCheckbox("Show coin particles", &profile.showCoinParticles);
        ImGui::EndDisabled();

        uiCheckbox("Show text particles", &profile.showTextParticles);

        ImGui::Separator();

        uiCheckbox("Enable screen shake", &profile.enableScreenShake);

        ImGui::Separator();

        uiCheckbox("Show bubbles", &profile.showBubbles);

        ImGui::Separator();

        uiCheckbox("Show doll particle border", &profile.showDollParticleBorder);

        ImGui::Separator();

        uiCheckbox("Bubble shader", &profile.useBubbleShader);

        ImGui::BeginDisabled(!profile.useBubbleShader);
        {
            uiSetFontScale(0.75f);

            if (isDebugModeEnabled())
            {
                ImGui::SetNextItemWidth(210.f * profile.uiScale);
                ImGui::SliderFloat("Iridescence", &profile.bsIridescenceStrength, 0.f, 1.f, "%.2f");

                ImGui::SetNextItemWidth(210.f * profile.uiScale);
                ImGui::SliderFloat("Edge Factor Min", &profile.bsEdgeFactorMin, 0.f, 1.f, "%.2f");

                ImGui::SetNextItemWidth(210.f * profile.uiScale);
                ImGui::SliderFloat("Edge Factor Max", &profile.bsEdgeFactorMax, 0.f, 1.f, "%.2f");

                ImGui::SetNextItemWidth(210.f * profile.uiScale);
                ImGui::SliderFloat("Edge Factor Strength", &profile.bsEdgeFactorStrength, 0.f, 10.f, "%.2f");

                ImGui::SetNextItemWidth(210.f * profile.uiScale);
                ImGui::SliderFloat("Distortion Strength", &profile.bsDistortionStrength, 0.f, 1.f, "%.2f");

                ImGui::SetNextItemWidth(210.f * profile.uiScale);
                ImGui::SliderFloat("Lens Distortion", &profile.bsLensDistortion, 0.f, 10.f, "%.2f");
            }

            ImGui::BeginDisabled(!profile.useBubbleShader);
            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Bubble Lightness", &profile.bsBubbleLightness, -1.f, 1.f, "%.2f");
            ImGui::EndDisabled();

            uiSetFontScale(uiNormalFontScale);
        }
        ImGui::EndDisabled();

        ImGui::Separator();

        const auto makePPUi = [&](auto vibrance, auto saturation, auto lightness, auto sharpness, auto blur)
        {
            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Vibrance", &(profile.*vibrance), 0.f, 2.f, "%.2f");

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Saturation", &(profile.*saturation), 0.f, 2.f, "%.2f");

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Lightness", &(profile.*lightness), 0.5f, 1.5f, "%.2f");

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Sharpness", &(profile.*sharpness), 0.f, 1.f, "%.2f");

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Blur", &(profile.*blur), 0.f, 1.f, "%.2f");

            if (ImGui::Button("Reset to default"))
            {
                Profile defaultProfile{};

                profile.*vibrance   = defaultProfile.*vibrance;
                profile.*saturation = defaultProfile.*saturation;
                profile.*lightness  = defaultProfile.*lightness;
                profile.*sharpness  = defaultProfile.*sharpness;
                profile.*blur       = defaultProfile.*blur;
            }
        };

        ImGui::Text("Foreground postprocess");
        ImGui::PushID("foregroundPostProcess");

        makePPUi(&Profile::ppSVibrance, //
                 &Profile::ppSSaturation,
                 &Profile::ppSLightness,
                 &Profile::ppSSharpness,
                 &Profile::ppSBlur);

        ImGui::PopID();

        ImGui::Separator();

        ImGui::Text("Background postprocess");
        ImGui::PushID("backgroundPostProcess");

        makePPUi(&Profile::ppBGVibrance,
                 &Profile::ppBGSaturation,
                 &Profile::ppBGLightness,
                 &Profile::ppBGSharpness,
                 &Profile::ppBGBlur);

        ImGui::PopID();

        ImGui::Separator();

        // TODO P1: check if this solves flickering
        ImGui::Text("Advanced options");

        constexpr const char* autobatchModes[3]{"Off", "CPU", "GPU"};

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        if (ImGui::Combo("Batching mode", &profile.autobatchMode, autobatchModes, 3))
            refreshWindowAutoBatchModeFromProfile();

        uiCheckbox("Flush after every batch", &flushAfterEveryBatch);
        uiCheckbox("Finish after every batch", &finishAfterEveryBatch);
        uiCheckbox("Flush before display", &flushBeforeDisplay);
        uiCheckbox("Finish before display", &finishBeforeDisplay);
        uiCheckbox("Flush after display", &flushAfterDisplay);
        uiCheckbox("Finish after display", &finishAfterDisplay);

        ImGui::EndTabItem();
    }

    uiSetFontScale(0.75f);
    if (ImGui::BeginTabItem(ICON_FA_DESKTOP " Display "))
    {
        selectedTab(3);

        uiSetFontScale(uiNormalFontScale);

        ImGui::Text("Auto resolution");

        uiSetFontScale(0.85f);
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Windowed");

        ImGui::SameLine();

        if (ImGui::Button("Large"))
        {
            playSound(sounds.buy);

            profile.resWidth = getReasonableWindowSize(1.f);
            profile.windowed = true;

            if (window.isFullscreen())
                recreateWindow();
            else
                resizeWindow();
        }

        ImGui::SameLine();

        if (ImGui::Button("Medium"))
        {
            playSound(sounds.buy);

            profile.resWidth = getReasonableWindowSize(0.9f);
            profile.windowed = true;

            if (window.isFullscreen())
                recreateWindow();
            else
                resizeWindow();
        }

        ImGui::SameLine();

        if (ImGui::Button("Small"))
        {
            playSound(sounds.buy);

            profile.resWidth = getReasonableWindowSize(0.8f);
            profile.windowed = true;

            if (window.isFullscreen())
                recreateWindow();
            else
                resizeWindow();
        }

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Fullscreen");

        ImGui::SameLine();

        if (ImGui::Button("Borderless"))
        {
            playSound(sounds.buy);

            profile.resWidth = sf::VideoModeUtils::getDesktopMode().size;
            profile.windowed = true;

            if (window.isFullscreen())
                recreateWindow();
            else
                resizeWindow();
        }

        ImGui::SameLine();

        if (ImGui::Button("Exclusive"))
        {
            playSound(sounds.buy);

            profile.resWidth = sf::VideoModeUtils::getDesktopMode().size;
            profile.windowed = false;

            recreateWindow();
        }

        ImGui::Separator();

        if (uiCheckbox("VSync", &profile.vsync))
            window.setVerticalSyncEnabled(profile.vsync);

        static auto fpsLimit = static_cast<float>(profile.frametimeLimit);
        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        if (ImGui::DragFloat("FPS Limit", &fpsLimit, 1.f, 60.f, 144.f, "%.f", ImGuiSliderFlags_AlwaysClamp))
        {
            profile.frametimeLimit = static_cast<unsigned int>(fpsLimit);
            window.setFramerateLimit(profile.frametimeLimit);
        }

        uiSetFontScale(uiNormalFontScale);

        ImGui::EndTabItem();
    }

    uiSetFontScale(0.75f);
    if (ImGui::BeginTabItem(ICON_FA_FILE " Data "))
    {
        selectedTab(4);

        uiSetFontScale(uiNormalFontScale);

        ImGui::Text("!!! Danger Zone !!!");

        uiSetFontScale(0.75f);
        ImGui::Text("Be careful with these dangerous settings!\nYour progress might be lost forever!");
        uiSetFontScale(uiNormalFontScale);

        static bool dangerZone = false;
        uiCheckbox("Enable Danger Zone (!)", &dangerZone);

        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::BeginDisabled(!dangerZone);

        uiButtonHueMod = 120.f;
        uiPushButtonColors();

        if (ImGui::Button("Reset *current* prestige##dangerzoneforceprestige"))
        {
            dangerZone = false;
            beginPrestigeTransition(0u);
        }

        ImGui::Spacing();
        ImGui::Spacing();

        if (ImGui::Button("Reset game##dangerzoneresetgame"))
        {
            dangerZone = false;
            forceResetGame();
        }

        ImGui::SameLine();

        if (ImGui::Button("Reset profile##dangerzoneresetprofile"))
        {
            dangerZone = false;
            forceResetProfile();
        }

        uiPopButtonColors();
        uiButtonHueMod = 0.f;

        ImGui::EndDisabled();

        ImGui::Separator();

        if (inSpeedrunPlaythrough())
        {
            ImGui::Text("Speedrun mode enabled!");
            uiSetFontScale(0.75f);
            ImGui::Text("Currently in a speedrun playthrough");
            uiSetFontScale(uiNormalFontScale);

            if (ImGui::Button("End Speedrun"))
            {
                pt = &ptMain;

                reseedRNGs(pt->seed);
                shuffledCatNamesPerType = makeShuffledCatNames(rng);
            }
        }
        else
        {
            ImGui::Text("Speedrun mode disabled!");
            uiSetFontScale(0.75f);
            ImGui::Text("Currently in the normal playthrough");
            uiSetFontScale(uiNormalFontScale);

            if (ImGui::Button("Start New Speedrun"))
            {
                pt = &ptSpeedrun;
                forceResetGame(/* goToShopTab */ false);
            }
        }

        ImGui::EndTabItem();
    }

    uiSetFontScale(0.75f);
    if (isDebugModeEnabled() && ImGui::BeginTabItem(ICON_FA_BUG " Debug "))
    {
        selectedTab(5);

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

        uiButtonHueMod = 120.f;
        uiPushButtonColors();

        if (ImGui::Button("Reset game"))
            forceResetGame();

        ImGui::SameLine();

        if (ImGui::Button("Reset profile"))
            forceResetProfile();

        uiPopButtonColors();
        uiButtonHueMod = 0.f;

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
                spawnCatCentered(catType, getHueByCatType(catType));
        }

        ImGui::SameLine();

        if (ImGui::Button("Do Ritual"))
            if (auto* wc = cachedWitchCat)
                wc->cooldown.value = 10.f;

        ImGui::SameLine();

        if (ImGui::Button("Ritual"))
            if (auto* wc = cachedWitchCat)
                wc->cooldown.value = 12000.f;

        ImGui::SameLine();

        if (ImGui::Button("Do Copy Ritual"))
            if (auto* wc = cachedCopyCat)
                wc->cooldown.value = 10.f;

        ImGui::SameLine();

        if (ImGui::Button("Copy Ritual"))
            if (auto* wc = cachedCopyCat)
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
            scrollArrowCountdown.value = 2000.f;

        ImGui::SameLine();

        if (ImGui::Button("Do Prestige"))
        {
            ++pt->psvBubbleValue.nPurchases;
            const auto ppReward = pt->calculatePrestigePointReward(1u);
            beginPrestigeTransition(ppReward);
        }

        ImGui::Separator();

        ImGui::Checkbox("hide ui", &debugHideUI);

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
        {
            scalarInput((sf::base::toString(i) + "Buff").cStr(), pt->buffCountdownsPerType[i].value);
        }

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

        uiSetFontScale(uiNormalFontScale);
        ImGui::PopFont();

        ImGui::EndTabItem();
    }

    ImGui::Separator();
    uiSetFontScale(uiNormalFontScale);

    ImGui::Text("FPS: %.2f", static_cast<double>(fps));
}
