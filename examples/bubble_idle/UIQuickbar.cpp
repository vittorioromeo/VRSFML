#include "Aliases.hpp"
#include "BubbleIdleMain.hpp"
#include "Cat.hpp"
#include "CatConstants.hpp"
#include "CatType.hpp"
#include "ParticleData.hpp"
#include "ParticleType.hpp"

#include "SFML/ImGui/IncludeImGui.hpp"

#include "SFML/Graphics/Color.hpp"

#include "SFML/System/Vec2Base.hpp"

#include "SFML/Base/Constants.hpp"
#include "SFML/Base/FloatMax.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/SizeT.hpp"

#include <cstdio>

////////////////////////////////////////////////////////////
void Main::uiDrawQuickbarCopyCat(const sf::Vec2f quickBarPos, Cat& copyCat)
{
    const bool asWitchAndBusy = pt->copycatCopiedCatType == CatType::Witch && (anyCatCopyHexed() || !pt->copyDolls.empty());

    const bool asWizardAndBusy = pt->copycatCopiedCatType == CatType::Wizard && isWizardBusy();

    const bool mustDisable = asWitchAndBusy || asWizardAndBusy;

    ImGui::BeginDisabled(mustDisable);

    constexpr const char* popupLabel = "CopyCatSelectorPopup";
    static sf::base::U8   opacity    = 168u;

    uiImageFromAtlas(txrIconCopyCat,
                     {.scale = {0.65f, 0.65f},
                      .color = (mustDisable ? sf::Color::Gray : sf::Color::White).withAlpha(opacity)});

    opacity = ImGui::IsItemHovered() || ImGui::IsPopupOpen(popupLabel) ? 255u : 168u;


    std::sprintf(uiState.uiTooltipBuffer, "Select Copycat mask");
    uiMakeTooltip(/* small */ true);

    ImGui::SameLine();

    if (ImGui::IsItemClicked())
    {
        ImGui::OpenPopup(popupLabel);
        playSound(sounds.uitab);
    }

    ImGui::SetNextWindowPos(ImVec2(quickBarPos) - ImVec2{0.f, 64.f}, 0, {1.f, 1.f});
    if (ImGui::BeginPopup(popupLabel))
    {
        ImGui::SetNextItemWidth(210.f * profile.uiScale);

        if (ImGui::BeginCombo("##copycatsel", CatConstants::typeNamesLong[asIdx(pt->copycatCopiedCatType)]))
        {
            for (SizeT i = asIdx(CatType::Normal); i < nCatTypes; ++i)
            {
                if (static_cast<CatType>(i) == CatType::Duck)
                    continue;

                if (!isUniqueCatType(static_cast<CatType>(i)))
                    continue;

                if (i == asIdx(CatType::Copy))
                    continue;

                if (findFirstCatByType(static_cast<CatType>(i)) == nullptr)
                    continue;

                const bool isSelected = pt->copycatCopiedCatType == static_cast<CatType>(i);
                if (ImGui::Selectable(CatConstants::typeNamesLong[i], isSelected))
                {
                    statDisguise();
                    pt->copycatCopiedCatType = static_cast<CatType>(i);

                    copyCat.cooldown.value = pt->getComputedCooldownByCatType(pt->copycatCopiedCatType);
                    copyCat.hits           = 0u;

                    sounds.smokebomb.settings.position = {copyCat.position.x, copyCat.position.y};
                    sounds.smokebomb.settings.position = {copyCat.position.x, copyCat.position.y};
                    playSound(sounds.smokebomb);

                    for (sf::base::SizeT iP = 0u; iP < 8u; ++iP)
                        spawnParticle(ParticleData{.position   = copyCat.position,
                                                   .velocity   = {rngFast.getF(-0.15f, 0.15f), rngFast.getF(0.f, 0.1f)},
                                                   .scale      = rngFast.getF(0.75f, 1.f),
                                                   .scaleDecay = -0.0005f,
                                                   .accelerationY = -0.00017f,
                                                   .opacity       = 1.f,
                                                   .opacityDecay  = rngFast.getF(0.00065f, 0.00075f),
                                                   .rotation      = rngFast.getF(0.f, sf::base::tau),
                                                   .torque        = rngFast.getF(-0.002f, 0.002f)},
                                      0.f,
                                      ParticleType::Smoke);

                    playSound(sounds.uitab);
                    ImGui::CloseCurrentPopup();
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        ImGui::EndPopup();
    }

    ImGui::EndDisabled();
}

////////////////////////////////////////////////////////////
void Main::uiDrawQuickbarBackgroundSelector(const sf::Vec2f quickBarPos)
{
    constexpr const char* popupLabel = "BackgroundSelectorPopup";
    static sf::base::U8   opacity    = 168u;

    uiImageFromAtlas(txrIconBg, {.scale = {0.65f, 0.65f}, .color = sf::Color::whiteWithAlpha(opacity)});

    opacity = ImGui::IsItemHovered() || ImGui::IsPopupOpen(popupLabel) ? 255u : 168u;

    std::sprintf(uiState.uiTooltipBuffer, "Select background");
    uiMakeTooltip(/* small */ true);

    ImGui::SameLine();

    if (ImGui::IsItemClicked())
    {
        ImGui::OpenPopup(popupLabel);
        playSound(sounds.uitab);
    }

    ImGui::SetNextWindowPos(ImVec2(quickBarPos) - ImVec2{0.f, 64.f}, 0, {1.f, 1.f});
    if (ImGui::BeginPopup(popupLabel))
    {
        auto& [entries, selectedIndex] = getBackgroundSelectorData();

        ImGui::SetNextItemWidth(210.f * profile.uiScale);

        if (ImGui::BeginCombo("##backgroundsel", entries[static_cast<sf::base::SizeT>(selectedIndex)].name))
        {
            for (SizeT i = 0u; i < entries.size(); ++i)
            {
                const bool isSelected = selectedIndex == static_cast<int>(i);
                if (ImGui::Selectable(entries[i].name, isSelected))
                {
                    selectedIndex = static_cast<int>(i);

                    selectBackground(entries, static_cast<int>(i));

                    playSound(sounds.uitab);
                    ImGui::CloseCurrentPopup();
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        ImGui::EndPopup();
    }
}

////////////////////////////////////////////////////////////
void Main::uiDrawQuickbarBGMSelector(const sf::Vec2f quickBarPos)
{
    constexpr const char* popupLabel = "MusicSelectorPopup";
    static sf::base::U8   opacity    = 168u;

    uiImageFromAtlas(txrIconBGM, {.scale = {0.65f, 0.65f}, .color = sf::Color::whiteWithAlpha(opacity)});

    opacity = ImGui::IsItemHovered() || ImGui::IsPopupOpen(popupLabel) ? 255u : 168u;

    std::sprintf(uiState.uiTooltipBuffer, "Select music");
    uiMakeTooltip(/* small */ true);

    ImGui::SameLine();

    if (ImGui::IsItemClicked())
    {
        ImGui::OpenPopup(popupLabel);
        playSound(sounds.uitab);
    }

    ImGui::SetNextWindowPos(ImVec2(quickBarPos) - ImVec2{0.f, 64.f}, 0, {1.f, 1.f});
    if (ImGui::BeginPopup(popupLabel))
    {
        auto& [entries, selectedIndex] = getBGMSelectorData();

        ImGui::SetNextItemWidth(210.f * profile.uiScale);

        if (ImGui::BeginCombo("##musicsel", entries[static_cast<sf::base::SizeT>(selectedIndex)].name))
        {
            for (SizeT i = 0u; i < entries.size(); ++i)
            {
                const bool isSelected = selectedIndex == static_cast<int>(i);
                if (ImGui::Selectable(entries[i].name, isSelected))
                {
                    selectedIndex = static_cast<int>(i);

                    selectBGM(entries, static_cast<int>(i));
                    switchToBGM(static_cast<sf::base::SizeT>(profile.selectedBGM), /* force */ false);

                    playSound(sounds.uitab);
                    ImGui::CloseCurrentPopup();
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        ImGui::EndPopup();
    }
}

////////////////////////////////////////////////////////////
void Main::uiDrawQuickbarQuickSettings(const sf::Vec2f quickBarPos)
{
    constexpr const char* popupLabel = "QuickSettingsPopup";
    static sf::base::U8   opacity    = 168u;

    uiImageFromAtlas(txrIconCfg, {.scale = {0.65f, 0.65f}, .color = sf::Color::whiteWithAlpha(opacity)});

    opacity = ImGui::IsItemHovered() || ImGui::IsPopupOpen(popupLabel) ? 255u : 168u;

    std::sprintf(uiState.uiTooltipBuffer, "Quick settings");
    uiMakeTooltip(/* small */ true);

    ImGui::SameLine();

    if (ImGui::IsItemClicked())
    {
        ImGui::OpenPopup(popupLabel);
        playSound(sounds.uitab);
    }

    ImGui::SetNextWindowPos(ImVec2(quickBarPos) - ImVec2{0.f, 64.f}, 0, {1.f, 1.f});
    if (ImGui::BeginPopup(popupLabel))
    {
        uiCheckbox("Enable tips", &profile.tipsEnabled);
        uiCheckbox("Enable notifications", &profile.enableNotifications);

        ImGui::Separator();

        uiCheckbox("Enable $/s meter", &profile.showDpsMeter);

        ImGui::Separator();

        uiCheckbox("Show cat range", &profile.showCatRange);
        uiCheckbox("Show ranges only on hover", &profile.showRangesOnlyOnHover);
        uiCheckbox("Show cat text", &profile.showCatText);

        ImGui::Separator();

        uiCheckbox("Show particles", &profile.showParticles);

        ImGui::BeginDisabled(!profile.showParticles);
        uiCheckbox("Show coin particles", &profile.showCoinParticles);
        ImGui::EndDisabled();

        uiCheckbox("Show text particles", &profile.showTextParticles);

        ImGui::Separator();

        uiCheckbox("Enable screen shake", &profile.enableScreenShake);

        ImGui::Separator();

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Background Opacity", &profile.backgroundOpacity, 0.f, 100.f, "%.f%%");

        uiCheckbox("Always show drawings", &profile.alwaysShowDrawings);

        ImGui::Separator();

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Minimap Scale", &profile.minimapScale, 5.f, 40.f, "%.2f");

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("HUD Scale", &profile.hudScale, 0.5f, 2.f, "%.2f");

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

        uiCheckbox("Bubble shader", &profile.useBubbleShader);

        ImGui::BeginDisabled(!profile.useBubbleShader);
        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Bubble Lightness", &profile.bsBubbleLightness, -1.f, 1.f, "%.2f");
        ImGui::EndDisabled();

        ImGui::Separator();

        if (uiCheckbox("VSync", &profile.vsync))
            window.setVerticalSyncEnabled(profile.vsync);

        ImGui::EndPopup();
    }
}

////////////////////////////////////////////////////////////
void Main::uiDrawQuickbarVolumeControls(const sf::Vec2f quickBarPos)
{
    constexpr const char* popupLabel = "VolumeSelectorPopup";
    static sf::base::U8   opacity    = 168u;

    uiImageFromAtlas(txrIconVolume, {.scale = {0.65f, 0.65f}, .color = sf::Color::whiteWithAlpha(opacity)});

    opacity = ImGui::IsItemHovered() || ImGui::IsPopupOpen(popupLabel) ? 255u : 168u;

    std::sprintf(uiState.uiTooltipBuffer, "Volume settings");
    uiMakeTooltip(/* small */ true);

    ImGui::SameLine();

    if (ImGui::IsItemClicked())
    {
        ImGui::OpenPopup(popupLabel);
        playSound(sounds.uitab);
    }

    ImGui::SetNextWindowPos(ImVec2(quickBarPos) - ImVec2{0.f, 64.f}, 0, {1.f, 1.f});
    if (ImGui::BeginPopup(popupLabel))
    {
        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Master##popupmastervolume", &profile.masterVolume, 0.f, 100.f, "%.f%%");

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        if (ImGui::SliderFloat("SFX##popupsfxvolume", &profile.sfxVolume, 0.f, 100.f, "%.f%%"))
            sounds.setupSounds(/* volumeOnly */ true, profile.sfxVolume / 100.f);

        ImGui::SetNextItemWidth(210.f * profile.uiScale);
        ImGui::SliderFloat("Music##popupmusicvolume", &profile.musicVolume, 0.f, 100.f, "%.f%%");

        ImGui::EndPopup();
    }
}

////////////////////////////////////////////////////////////
void Main::uiDrawQuickbar()
{
    const float xStart = uiState.lastUiSelectedTabIdx == 0
                             ? getResolution().x
                             : gameView.worldToScreen({getLeftMostUsefulX(), 0.f}, getResolution()).x;

    const sf::Vec2f quickBarPos{xStart - 15.f, getResolution().y - 15.f};

    ImGui::SetNextWindowPos({quickBarPos.x, quickBarPos.y}, 0, {1.f, 1.f});
    ImGui::SetNextWindowSizeConstraints(ImVec2(0.f, 0.f), ImVec2(SFML_BASE_FLOAT_MAX, SFML_BASE_FLOAT_MAX));

    ImGui::Begin("##quickmenu",
                 nullptr,
                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);

    if (Cat* copyCat = getCopyCat(); copyCat != nullptr)
        uiDrawQuickbarCopyCat(quickBarPos, *copyCat);

    if (getBackgroundSelectorData().entries.size() > 1u)
        uiDrawQuickbarBackgroundSelector(quickBarPos);

    if (getBGMSelectorData().entries.size() > 1u)
        uiDrawQuickbarBGMSelector(quickBarPos);

    uiDrawQuickbarVolumeControls(quickBarPos);

    uiDrawQuickbarQuickSettings(quickBarPos);

    ImGui::End();
}
